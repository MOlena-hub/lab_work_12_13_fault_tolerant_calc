#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>

#define NUM_THREADS 2
#define TARGET_COUNT 10

// Thread-Local Storage: кожен потік має власну точку відновлення та лічильник
__thread sigjmp_buf thread_env;
__thread int current_iteration = 0;
__thread int thread_id_internal;

void recovery_handler(int sig) {
    (void)sig; // Позбуваємося попередження  про невикористаний параметр

    // Використовуємо безпечний write замість printf
    write(1, "\n [!] SIGSEGV detected. Skipping failed step and recovering... \n", 65);
    
    // Пропускаємо ітерацію
    current_iteration++; 

    // Повертаємо потік до точки sigsetjmp
    siglongjmp(thread_env, 1); 
}

void* compute_task(void* arg) {
    thread_id_internal = *((int*)arg);
    
    // Встановлюємо контрольну точку (checkpoint)
    sigsetjmp(thread_env, 1);

    while (current_iteration < TARGET_COUNT) {
        printf("Thread %d: Working on iteration %d\n", thread_id_internal, current_iteration);
        usleep(500000); 

        // Штучна критична помилка
        if (thread_id_internal == 0 && current_iteration == 4) {
            printf("Thread %d: Intentional crash (NULL pointer) at iter %d!\n", thread_id_internal, current_iteration);
            int *ptr = NULL;
            *ptr = 0xDEAD; // Тут виникає SIGSEGV
        }

        current_iteration++;
    }

    printf("Thread %d: DONE!\n", thread_id_internal);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    // Налаштування обробника сигналів через sigaction
    struct sigaction sa;
    sa.sa_handler = recovery_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER; 
    sigaction(SIGSEGV, &sa, NULL);

    printf("Starting fault-tolerant computation (Lab 12-13)...\n");

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, compute_task, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Main: Finished.\n");
    return 0;
}
