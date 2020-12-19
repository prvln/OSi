#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

int main(){
    const int ARRAY_SIZE = 55;
    int shm;

    char timeChar[26] = {0};
    char shmName[13] = "OUR_NOT_YOUR";
    char format[111] = "";
    memset(format, 0, 111);
    memcpy(format, "\nHi, I'm [%i], now is %s here is message that I found:\n\t", 56);
    char *ARRAY_POINTER;

    time_t seconds = time(NULL);   

    if ( (shm = shm_open(shmName, O_RDONLY, 0777)) == -1 ) {
        perror("shm_open");
        return 1;
    }
    ARRAY_POINTER = mmap(0, ARRAY_SIZE, PROT_READ, MAP_SHARED, shm, 0);

    
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