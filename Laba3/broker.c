#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <time.h>

int shm;

void shutdown(){
    printf("[%x] Atexit has been started\n", getpid());
    shmctl(shm, IPC_RMID, NULL);
}

void signalHandler(int signum) {
   printf("\n[%x] Caught signal: %i\n", signum);
   exit(signum);
}

int main(){
    int pid_file = open("/tmp/U_R_THE_ONLY_ONE.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc) {
        if(EWOULDBLOCK == errno){
            fprintf(stderr, "%s", "[%x] Another instance of broker is running!\nExit", getpid());
            return 0;
        }   
    }
    else {
        const int ARRAY_SIZE = 50;
        char sharedMemorySeed[23] = "I HAVE THE HIGH GROUND";
        char message[ARRAY_SIZE];
        char *ARRAY_POINTER;
        char format[25] = "[%i] Hi, today is %s";
        time_t seconds;
        key_t shmKey;

        int error = atexit(shutdown);
        if (error) printf("[%x] atexit returned error: %i \n\n", getpid(), error);
        signal(SIGINT, signalHandler);


        shmKey = ftok(sharedMemorySeed, 42);
        if ( (shm = shmget(shmKey, ARRAY_SIZE, IPC_CREAT | 0666)) == -1 ) {
            perror("shm_open");
            return 1;
        }
        ARRAY_POINTER = shmat(shm, NULL, 0);


        while(1){
            seconds = time(NULL);        
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