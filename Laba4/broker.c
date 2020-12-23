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

        const int ARRAY_SIZE = 55;
        key_t shmKey, semKey;
        clock_t start, stop;
        time_t seconds;
        
        char *WHOcontainer;
        char semaphoreSeed[4] = "WOOF";
        char sharedMemorySeed[23] = "WHO LET THE DOGS OUT?";
        WHOcontainer = "WHO?";
        WHOcontainer = "WHO?";
        char timeChar[26] = "WHO? WHO?";
        char *ARRAY_POINTER;
        char message[ARRAY_SIZE];
        char semName[7] = "SEM_QUE";
        char format[30] = "Hi, I'm [%i], today is %26.26s";

        int error = atexit(shutdown);
        if (error) printf("[%x] atexit returned error: %i \n\n", getpid(), error);
        signal(SIGINT, signalHandler);

        shmKey = ftok(sharedMemorySeed, 20);
        if ( (shm = shmget(shmKey, ARRAY_SIZE, IPC_CREAT | 0666)) == -1 ) {
            perror("shm_open");
            return 1;
        }
        ARRAY_POINTER = shmat(shm, NULL, 0);

        semKey = ftok(semaphoreSeed, 21);
        if ((sem = semget(semKey, 1, IPC_CREAT | 0666)) == -1) {
            perror("semget");
            return 1;
        }

        arg.val = 1;
        if (semctl(sem, 0, SETVAL, arg) == -1) {
            perror("semctl");
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
            timeChar[24] = ' '; timeChar[25] = ' ';

            sprintf(message, format, getpid(), timeChar);
            
            memcpy(ARRAY_POINTER, message, ARRAY_SIZE);
            printf("\nMessage sent: %s", message);
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