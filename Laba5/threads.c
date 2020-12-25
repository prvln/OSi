#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_OF_THREADS 11
#define SHARED_ARRAY_SIZE 10
#define SLEEP_DELAY_IN_NANOSECONDS 10000000
 
char sharedArray[SHARED_ARRAY_SIZE] = {0};
pthread_t threads[NUM_OF_THREADS];
pthread_mutex_t mutex;

void signalHandler(int signum) {
    printf("\n[%x] Caught signal: %i\n", getpid(), signum);
    for(int i = 0; i < NUM_OF_THREADS; i++){
        pthread_cancel(threads[i]);
    }
    pthread_mutex_destroy(&mutex);

    
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
    int tid = gettid();

    char filename[SHARED_ARRAY_SIZE] = {0};
    toArray(tid, &filename[0]);
    sprintf(filename, "%4.6s.txt", filename);

    FILE *filePTR;
    if ( (filePTR = fopen(filename,"w")) == NULL){
        char * errorMessage = "[%i] failed to create file: \"%8.10s\"";
        filePTR = stdout;
    }
    
    while(1){
        start = clock();
        pthread_mutex_lock(&mutex);
            stop = clock();
            localInt = atoi(&sharedArray[0]);
            fprintf(filePTR, "[%d] - tid, waited for: %fs array: %i\n", tid,(double) (stop - start) / CLOCKS_PER_SEC , localInt);
            fflush(filePTR);
        pthread_mutex_unlock(&mutex);
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
        pthread_mutex_lock(&mutex);
            memcpy(&localBuffer, &sharedArray[0], SHARED_ARRAY_SIZE);

            local = atoi(&localBuffer[0]);
            printf("write %i\n", ++local);
            toArray(local, &localBuffer[0]);

            memcpy(&sharedArray, &localBuffer[0], SHARED_ARRAY_SIZE);

            nanosleep (&tw, &tr); //wait until release memory
        pthread_mutex_unlock(&mutex);

        //wait for 0.01 sec for readers to read from memory
        tw.tv_nsec = SLEEP_DELAY_IN_NANOSECONDS;
        nanosleep (&tw, &tr);
    }
    return NULL;
}
 
int main() {
    int errorHandler = 0;
    size_t i;

    if ( (errorHandler = (uintptr_t)signal(SIGINT, signalHandler)) == -1){
            perror("Signal failed to initialize signalHandler function");
            return 1;
    }

    printf("\n-->WARNING<--\nThis program will create %i .txt files that will quickly expand their size", NUM_OF_THREADS - 1);
    printf("\nDO NOT let the program live for a long time");
    printf("\nHARDLY recommend not to set SLEEP_DELAY_IN_NANOSECONDS to value less than 1 million");
    printf("\nBy pressing any key you accept your responsibility for any actions from now on");

    getchar();

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&threads[0], NULL, writer, NULL);

    for (i = 1; i < NUM_OF_THREADS; i++) {
        pthread_create(&threads[i], NULL, reader, NULL);
    }
    
    for (i = 0; i < NUM_OF_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    //Уничтожение мьютекса
    pthread_mutex_destroy(&mutex);
    return 0;
}