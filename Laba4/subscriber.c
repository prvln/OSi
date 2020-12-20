#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <time.h>

int main(){
    const int ARRAY_SIZE = 50;
    clock_t start, stop, delay;
    sem_t* sem;
    int shm;

    char timeChar[26] = {0};
    char shmName[7] = "SEM_SHM";
    char semName[7] = "SEM_QUE";
    char format[128] = "";
    char *ARRAY_POINTER;

    memset(format, 0, 128);
    memcpy(format, "\nHi, I'm [%i], now is %i here is message that I found:\n\t", 73);

    time_t seconds = time(NULL);   

    if ( (shm = shm_open(shmName, O_RDWR, 0777)) == -1 ) {
        perror("shm_open");
        return 1;
    }

    if ( (sem = sem_open(semName, 0)) == SEM_FAILED ) {
        perror("sem_open error");
        return 1;
    }

    ARRAY_POINTER = mmap(0, ARRAY_SIZE, PROT_READ, MAP_SHARED, shm, 0);

    while(1){
        seconds = time(NULL);
        memcpy(timeChar, asctime(localtime(&seconds)), 26);      
        timeChar[24] = '\0';

        start = clock();
        if(sem_wait(sem) != -1){
            stop = clock();
            printf("\nTime elapsed: %ims", (stop - start));
            memcpy(&format[56], ARRAY_POINTER, ARRAY_SIZE);
            printf(format, getpid(), seconds);
            fflush(stdout);
            sem_post(sem);
        }else{
            perror("Semaphore broken");
            return 1;
        }
        //sleep(1);
    }
    return 0;
}