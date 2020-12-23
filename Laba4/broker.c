#include <stdio.h>
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
    shmctl(shm, IPC_RMID, NULL);
    semctl(sem, 0, IPC_RMID, 0);
}

void signalHandler(int signum) {
   printf("\n[%x] Caught signal: %i\n", getpid(), signum);
   exit(signum);
}

int main(){
    int pid_file = open("/tmp/U_R_THE_ONLY_ONE.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc) {
        if(EWOULDBLOCK == errno){
            perror("Another instance of broker is running!\nExit");
            return 0;
        }   
    }
    else {
        union semun {
            int val;              
            struct semid_ds *buf; 
            unsigned short *array;        
        } arg;

        struct sembuf semLock = {0, -1, 0};
        struct sembuf semUnlock = {0, 1, 0};

        key_t shmKey, semKey;
        clock_t start, stop;
        time_t seconds;

        int errorHandler = 0;

        char message[ARRAY_SIZE];
        char timeCharArray[24] = {0};
        char *ARRAY_POINTER;
        char format[22] = "[%i] Hi, today is %24s";

        if ( (errorHandler = atexit(shutdown)) == -1){
            perror("Atexit failed to initialize shutdown function");
            return 1;
        } 
        ;
        if ( (errorHandler = (uintptr_t)signal(SIGINT, signalHandler)) == -1){
            perror("Signal failed to initialize signalHandler function");
            return 1;
        }

        if ( (shmKey = ftok(FILE_PATH, 10)) == -1){
            perror("Shared memory ftok failed");
            return 1;
        }
        if ( (shm = shmget(shmKey, ARRAY_SIZE, IPC_CREAT | 0666)) == -1 ) {
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

        if ((sem = semget(semKey, 1, IPC_CREAT | 0666)) == -1) {
            perror("Semget failed");
            return 1;
        }

        arg.val = 1;
        if (semctl(sem, 0, SETVAL, arg) == -1) {
            perror("Semctl failed");
            return 1;
        }

        while(1){
            start = clock();

            if (semop(sem, &semLock, 1) == -1) {
                perror("semop");
                return 1;
            }
            stop = clock();
            printf("\nWaited for: %f s", (double) (stop - start) / CLOCKS_PER_SEC);

            seconds = time(NULL);
            memcpy(timeCharArray, asctime(localtime(&seconds)), 24);

            sprintf(message, format, getpid(), timeCharArray);
            
            memcpy(ARRAY_POINTER, message, ARRAY_SIZE);
            printf("\nMessage sent: %46s", ARRAY_POINTER);
            fflush(stdout);

            if (semop(sem, &semUnlock, 1) == -1) {
                perror("semop");
                return 1;
            }
            sleep(1);
        }
        return 0;
    }
}