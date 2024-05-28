#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define NUM_MASTERS 6

sem_t ingredient_available;  // Malzeme bulunabilir semaforu
sem_t ingredient_taken;      // Malzeme alındı semaforu
sem_t cake_ready;            // Pasta hazır semaforu

// Malzeme isimleri
char* ingredients[] = {"Yumurta", "Süt", "Un", "Kakao"};
int desired_ingredients[NUM_MASTERS][4] = {
    {1, 0, 1, 0},   // Usta 1: Yumurta = 1, Süt = 0, Un = 1, Kakao = 0 (YU)  usta 1  durum0
    {0, 1, 0, 1},   // Usta 2: Yumurta = 0, Süt = 1, Un = 0, Kakao = 1 (SK)  usta 2  durum1
    {1, 1, 0, 0},   // Usta 3: Yumurta = 1, Süt = 1, Un = 0, Kakao = 0 (YS)  usta 3  durum2
    {0, 1, 1, 0},   // Usta 4: Yumurta = 0, Süt = 1, Un = 1, Kakao = 0 (SU)  usta 4  durum3
    {1, 0, 0, 1},   // Usta 5: Yumurta = 1, Süt = 0, Un = 0, Kakao = 1 (YK)  usta 5  durum4
    {0, 0, 1, 1}    // Usta 6: Yumurta = 0, Süt = 0, Un = 1, Kakao = 1 (UK)  usta 6  durum5
};
int ingredients_available = 0; // Kullanılabilir malzeme sayısı
void* master_thread(void* arg) { //Usta threadi
    int master_id = *(int*)arg;
    free(arg);

    // Usta rastgele bir süre uyur
    int sleep_time = rand() % 5 + 1;
    printf("\033[1;95m	            Usta %d, %d saniye uyuyor.\033[0m\n", master_id, sleep_time);
   
    sleep(sleep_time);

    // Malzeme talep ediyor
    int* desired_ingredients_ptr = desired_ingredients[master_id -1 ];
    printf("Usta %d-->", master_id);
    if (desired_ingredients_ptr[0] == 0)
    printf("\033[1;32m YUMURTA \033[0m");  // printf("\033[1;32m     \033[0m\n"); yesil renk
    
    if (desired_ingredients_ptr[1] == 0)
    printf("\033[1;32m SUT \033[0m");
     
    if (desired_ingredients_ptr[2] == 0)
    printf("\033[1;32m UN \033[0m");
       
    if (desired_ingredients_ptr[3] == 0)
    printf("\033[1;32m KAKAO \033[0m");
      
    printf(" malzemelerini istiyor.\n");  

    // Malzemeleri almak için bekleniyor
    sem_wait(&ingredient_available);
    printf("\033[1;95m*\033[0m");
    printf("Usta %d, malzemeleri aldı.\n", master_id);

    // Malzemeleri kullanıyor
    printf("\033[1;95m**\033[0m");
    printf("Usta %d, malzemeleri kullanıyor.\n", master_id);
    sleep(2); // Malzemelerin kullanılma süresi

    // Malzemeleri bırakıyor
    sem_post(&ingredient_taken);
    printf("\033[1;95m***\033[0m");
    printf("Usta %d, pasta hazırlandı ve usta malzemeleri bıraktı.\n", master_id);

    // Pastayı bekliyor
    sem_wait(&cake_ready);

    pthread_exit(NULL);
}
void* toptanci_thread_func(void* arg) {
   
    while (1) {
        // Malzeme teslimi yapılıyor
        sleep(2);
        printf("\033[1;33mToptancı, malzeme teslim ediyor.\033[0m\n");   //turuncu renk
        sem_post(&ingredient_available);

        // Pasta alınıncaya kadar bekleniyor
        sem_wait(&ingredient_taken);
        printf("\033[1;34mToptancı, pastayı aldı ve satmaya gitti.\033[0m\n");
    }


    // Tüm ustalara pastayı hazırladığını bildir
    for (int i = 0; i < NUM_MASTERS; i++) {
        sem_post(&cake_ready);
    }

    pthread_exit(NULL);
}



int giris() {
  // Her usta için malzemeleri kontrol et
  for (int master_id = 0; master_id < NUM_MASTERS; master_id++) {
    printf("Usta %d:\n", master_id + 1);

    // Her malzemeyi kontrol et
    for (int ingredient_id = 0; ingredient_id < 4; ingredient_id++) {
      if (desired_ingredients[master_id][ingredient_id] == 1) {
        printf("\033[1;32m  - %s var\033[0m\n", ingredients[ingredient_id]);
      } else {
        printf("\033[1;33m  - %s yok\033[0m\n", ingredients[ingredient_id]);
      }
    }
  }

  return 0;
}


int main(int argc, char* argv[]) {
    if (argc != 3 || strcmp(argv[1], "-i") != 0) {
        fprintf(stderr, "Kullanım: %s -i <dosya_adi>\n", argv[0]);
        return 1;
    }
    giris();
    printf("---- Kim daha erken uykudan uyanirsa o ilk siraya girer ----\n");
    // Semaforları başlat
    sem_init(&ingredient_available, 0, 0);
    sem_init(&ingredient_taken, 0, 0);
    sem_init(&cake_ready, 0, 0);

    // Usta thread'lerini oluştur
    pthread_t master_threads[NUM_MASTERS];
    for (int i = 0; i < NUM_MASTERS; i++) {
        int* master_id = malloc(sizeof(int));
        *master_id = i + 1;
        pthread_create(&master_threads[i], NULL, master_thread, master_id);
    }
    

    // Toptancı thread'ini oluştur
    pthread_t toptanci_thread;
    pthread_create(&toptanci_thread, NULL, toptanci_thread_func, argv[2]);

    // Toptancı thread'ini bekleyin
    pthread_join(toptanci_thread, NULL);

    // Usta thread'lerini bekleyin
    for (int i = 0; i < NUM_MASTERS; i++) {
        pthread_join(master_threads[i], NULL);
    }


    // Semaforları temizle
    sem_destroy(&ingredient_available);
    sem_destroy(&ingredient_taken);
    sem_destroy(&cake_ready);

    return 0;
}

