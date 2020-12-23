#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 46
#define FILE_PATH "./broker.c"

int shm, sem;

void shutdown(){
    printf("[%x] Atexit has been started\n", getpid());
    struct sembuf semUnlock = {0, 1, 0};
    if (semop(ftok(FILE_PATH, 21), &semUnlock, 1) == -1) {
                perror("semop");
        }
}

void signalHandler(int signum) {
   printf("\n[%x] Caught signal: %i\n", getpid(), signum);
   exit(signum);
}

int main(){
    struct sembuf semLock = {0, -1, 0};
    struct sembuf semUnlock = {0, 1, 0};

    key_t shmKey, semKey;
    clock_t start, stop;
    time_t seconds;

    int errorHandler = 0;

    char timeCharArray[25] = {0};
    char *ARRAY_POINTER;  
    char format[90] = {0};

    memcpy(format, "\n[%x] This message was written in %24s\t", 40);

    if ( (errorHandler = atexit(shutdown)) == -1){
        perror("Atexit failed to initialize shutdown function");
        return 1;
    }
    if ( (errorHandler = (uintptr_t)signal(SIGINT, signalHandler)) == -1){
        perror("Signal failed to initialize signalHandler function");
        return 1;
    }

    if ( (shmKey = ftok(FILE_PATH, 10)) == -1){
        perror("Shared memory ftok failed");
        return 1;
    }
    if ( (shm = shmget(shmKey, ARRAY_SIZE, 0666)) == -1 ){
        perror("Shmget failed");
        return 1;
    }
    if ( (uintptr_t)(ARRAY_POINTER = shmat(shm, NULL, 0)) == -1){
        perror("Shmat failed");
        return 1;
    }


    if ( (semKey = ftok(FILE_PATH, 21)) == -1){
        perror("Semaphore ftok failed");
        return 1;
    }
    if ((sem = semget(semKey, 1, 0)) == -1) {
        perror("Semget failed");
        return 1;
    }

    while(1){
        
        start = clock();

        if (semop(sem, &semLock, 1) == -1) {
                perror("semop");
                return 1;
        }

        stop = clock();
        printf("\nWaited for: %f ", (double) (stop - start) / CLOCKS_PER_SEC);

        seconds = time(NULL);
        memcpy(timeCharArray, asctime(localtime(&seconds)), 25);
        memcpy(&format[39], ARRAY_POINTER, ARRAY_SIZE);
        printf(format, getpid(), timeCharArray);
        fflush(stdout);

        if (semop(sem, &semUnlock, 1) == -1) {
                perror("semop");
                return 1;
        }
        sleep(1);
    }
    return 0;
}