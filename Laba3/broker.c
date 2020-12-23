#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 46
#define FILE_PATH "./broker.c"

int shm;

void shutdown(){
    printf("[%x] Atexit has been started\n", getpid());
    shmctl(shm, IPC_RMID, NULL);
}

void signalHandler(int signum) {
   printf("\n[%x] Caught signal: %i\n", getpid(), signum);
   exit(signum);
}

int main(){

    int pid_file = open("/tmp/U_R_THE_ONLY_ONE.pid", O_CREAT | O_RDWR, 0666);
    if(pid_file == -1){
        perror("Can't open reference file to check another instance of program running\nExit");
    }

    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc) {
        if(EWOULDBLOCK == errno){
            perror("Another instance of broker is running!\nExit");
            return 0;
        }   
    }
    else {
        char message[ARRAY_SIZE];
        char timeCharArray[24] = {0};
        char *ARRAY_POINTER;
        char format[22] = "[%i] Hi, today is %24s";
        int errorHandler = 0;
        time_t seconds;
        key_t shmKey;

        
        if ( (errorHandler = atexit(shutdown)) == -1){
            perror("Atexit failed to initialize shutdown function");
            return 1;
        } 
        ;
        if ( (errorHandler = (uintptr_t)signal(SIGINT, signalHandler)) == -1){
            perror("Signal failed to initialize signalHandler function");
            return 1;
        }

        if ( (shmKey = ftok(FILE_PATH, 42)) == -1){
            perror("Ftok failed");
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


        while(1){
            seconds = time(NULL);        
            memset(message, 0, ARRAY_SIZE);
            memcpy(timeCharArray, asctime(localtime(&seconds)), 24);

            sprintf(message, format, getpid(), timeCharArray);
            
            memcpy(ARRAY_POINTER, message, ARRAY_SIZE);
            printf("\nMessage sent: %46s", ARRAY_POINTER);
            fflush(stdout);
            sleep(1);
        }
        return 0;
    }
}