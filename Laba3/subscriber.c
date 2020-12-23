#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 46
#define FILE_PATH "./broker.c"

int main(){
    char timeChar[24] = {0};
    char format[102]  = {0};
    char *ARRAY_POINTER;
    time_t seconds;
    key_t shmKey;
    int shm;

    memcpy(format, "\nHi, I'm [%i], now is %s here is message that I found:\n\t", 57);

    if ( (shmKey = ftok(FILE_PATH, 42)) == -1){
            perror("Ftok failed");
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

    
    while(1){
        seconds = time(NULL);
        memcpy(timeChar, asctime(localtime(&seconds)), 24);
        memcpy(&format[56], ARRAY_POINTER, ARRAY_SIZE);

        printf(format, getpid(), timeChar);
        fflush(stdout);

        sleep(1);
    }
    return 0;
}