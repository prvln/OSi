#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <time.h>

int main(){
    struct sembuf semLock = {0, -1, 0};
    struct sembuf semUnlock = {0, 1, 0};

    const int ARRAY_SIZE = 50;
    clock_t start, stop;
    int shm, sem;

    char timeChar[26] = {0};
    char semName[7] = "SEM_QUE";    
    char format[111] = "";
    char *ARRAY_POINTER;

    memset(format, 0, 111);
    memcpy(format, "\nHi, I'm [%i], now is %i here is message that I found:\n\t", 56);

    time_t seconds = time(NULL);   

    if ( (shm = shmget(9876, 1, IPC_CREAT | 0777)) == -1 ) {
            perror("shm_open");
            return 1;
        }

    ARRAY_POINTER = shmat(shm, NULL, 0);

    if ((sem = semget(1984, 1, 0)) == -1) {
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