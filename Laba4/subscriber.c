#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
int shm, sem;

void shutdown(){
    printf("[%x] Atexit has been started\n", getpid());
    char semaphoreSeed[4] = "WOOF";
    struct sembuf semUnlock = {0, 1, 0};
    if (semop(ftok(semaphoreSeed, 21), &semUnlock, 1) == -1) {
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

    const int ARRAY_SIZE = 55;
    key_t shmKey, semKey;
    clock_t start, stop;
    time_t seconds;
    

    char semaphoreSeed[4] = "WOOF";
    char sharedMemorySeed[23] = "WHO LET THE DOGS OUT?";
    char timeChar[26] = {0};
    char semName[7] = "SEM_QUE";    
    char format[111] = "";
    char *ARRAY_POINTER;

    int error = atexit(shutdown);
    if (error) printf("[%x] atexit returned error: %i \n\n", getpid(), error);
    signal(SIGINT, signalHandler);

    shmKey = ftok(sharedMemorySeed, 20);
    if ( (shm = shmget(shmKey, ARRAY_SIZE, IPC_CREAT | 0666)) == -1 ) 
    {
        perror("shm_open");
        return 1;
    }

    ARRAY_POINTER = shmat(shm, NULL, 0);

    semKey = ftok(semaphoreSeed, 21);
    if ((sem = semget(semKey, 1, 0)) == -1) {
            perror("semget");
            return 1;
    }

    while(1){
        
        start = clock();

        if (semop(sem, &semLock, 1) == -1) {
                perror("semop");
                return 1;
        }

        stop = clock();
        printf("\nTime elapsed: %f ", (double) (stop - start) / CLOCKS_PER_SEC);

        seconds = time(NULL);
        memcpy(timeChar, asctime(localtime(&seconds)), 26);      
        timeChar[24] = '\0';

        memcpy(&format[56], ARRAY_POINTER, ARRAY_SIZE);
        printf(format, getpid(), timeChar);
        fflush(stdout);

        if (semop(sem, &semUnlock, 1) == -1) {
                perror("semop");
                return 1;
        }
        sleep(1);
    }
    return 0;
}