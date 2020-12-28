#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define __USE_UNIX98 1

#define NUM_OF_THREADS 11
#define SHARED_ARRAY_SIZE 10
#define SLEEP_DELAY_IN_NANOSECONDS 10000000
 
char sharedArray[SHARED_ARRAY_SIZE] = {0};
pthread_t threads[NUM_OF_THREADS];
pthread_rwlock_t  *locker;

void signalHandler(int signum) {
    printf("\n[%x] Caught signal: %i\n", getpid(), signum);
    for(int i = 0; i < NUM_OF_THREADS; i++){
        pthread_cancel(threads[i]);
    }
    pthread_rwlock_destroy(&locker);
    exit(signum);
}

void toArray(int number, char outputArray[])
{
    int n = log10(number) + 1;
    int i;
    for (i = n-1; i >= 0; --i, number /= 10)
    {
        outputArray[i] = (number % 10) + '0';
    }
}

void* reader(void *args) {
    struct timespec tr; struct timespec tw = {0,SLEEP_DELAY_IN_NANOSECONDS};
    clock_t start, stop;
    int localInt = 0;

    pthread_t tid = pthread_self();

    while(1){
        start = clock();
        pthread_rwlock_rdlock(&locker);
            stop = clock();
            localInt = atoi(&sharedArray[0]);
            fprintf(stdout, "[%u] - tid, waited for: %fs array: %i\n", tid, (double) (stop - start) / CLOCKS_PER_SEC , localInt);
            fflush(stdout);
        pthread_rwlock_unlock(&locker);
        nanosleep (&tw, &tr);
    }
    return NULL;
}
 
void* writer(void *args) {
    int local;
    struct timespec tw = {0,SLEEP_DELAY_IN_NANOSECONDS};
    struct timespec tr;
    char localBuffer[SHARED_ARRAY_SIZE] = {0};
    srand(time(NULL));

    while(1){
        tw.tv_nsec = random();
        
        pthread_rwlock_wrlock(&locker);
            memcpy(&localBuffer, &sharedArray[0], SHARED_ARRAY_SIZE);

            local = atoi(&localBuffer[0]);
            printf("write %i\n", ++local);
            toArray(local, &localBuffer[0]);

            memcpy(&sharedArray, &localBuffer[0], SHARED_ARRAY_SIZE);

            nanosleep (&tw, &tr); //wait until release memory
        pthread_rwlock_unlock(&locker);

        //wait for 0.01 sec for readers to read from memory
        tw.tv_nsec = SLEEP_DELAY_IN_NANOSECONDS;
        nanosleep (&tw, &tr);
    }
    return NULL;
}
 
int main() {
    int errorHandler = 0;
    int i;

    if ( (errorHandler = (uintptr_t)signal(SIGINT, signalHandler)) == -1){
            perror("Signal failed to initialize signalHandler function");
            return 1;
    }

    pthread_rwlock_init(&locker, NULL);

    pthread_create(&threads[0], NULL, writer, NULL);

    int *arg = malloc(sizeof(*arg));
    for (i = 1; i < NUM_OF_THREADS; i++) {
        *arg = i;
        pthread_create(&threads[i], NULL, reader, arg);
    }
    
    for (i = 0; i < NUM_OF_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_cond_destroy(&locker);
    return 0;
}