#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <time.h>
int main(){
    const int ARRAY_SIZE = 55;
    int shm;

    char timeChar[26] = {0};
    char format[111] = "";
    char *ARRAY_POINTER;

    memset(format, 0, 111);
    memcpy(format, "\nHi, I'm [%i], now is %s here is message that I found:\n\t", 56);

    time_t seconds = time(NULL);   

    if ( (shm = shmget(1585, 1, IPC_CREAT | 0777)) == -1 ) {
            perror("shm_open");
            return 1;
        }

    ARRAY_POINTER = shmat(shm, NULL, 0);

    
    while(1){
        seconds = time(NULL);
        memcpy(timeChar, asctime(localtime(&seconds)), 26);      
        timeChar[24] = '\0';

        memcpy(&format[56], ARRAY_POINTER, ARRAY_SIZE);
        printf(format, getpid(), timeChar);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}