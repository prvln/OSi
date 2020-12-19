#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <errno.h>
#include <string.h>
#include <time.h>

int main(){
    int pid_file = open("/tmp/U_R_THE_ONLY_ONE.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc) {
        if(EWOULDBLOCK == errno){
            fprintf(stderr, "%s", "Another instance of broker is running!\nExit");
            return 0;
        }   
    }
    else {
        int shm;
        const int ARRAY_SIZE = 50;

        char *ARRAY_POINTER;
        char shmName[13] = "OUR_NOT_YOUR";
        char format[25] = "Hi, I'm [%i], today is %s";
        char message[ARRAY_SIZE];

        if ( (shm = shm_open(shmName, O_CREAT | O_RDWR, 0777)) == -1 ) {
            perror("shm_open");
            return 1;
        }

        ftruncate(shm, ARRAY_SIZE);
        ARRAY_POINTER = mmap(0, ARRAY_SIZE, PROT_WRITE, MAP_SHARED, shm, 0);

        while(1){
            time_t seconds = time(NULL);        
            memset(message, 0, ARRAY_SIZE);
            sprintf(message, format, getpid(), asctime(localtime(&seconds)));
            
            memcpy(ARRAY_POINTER, message, ARRAY_SIZE);
            printf("\nMessage sent: %s", ARRAY_POINTER);
            fflush(stdout);
            sleep(1);
        }
        return 0;
    }
}