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
    int pid_file = open("/tmp/U_R_THE_ONLY_ONE.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc) {
        if(EWOULDBLOCK == errno){
            fprintf(stderr, "%s", "Another instance of broker is running!\nExit");
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

        const int ARRAY_SIZE = 50;
        clock_t start, stop;
        int shm, sem;
        
        char semName[7] = "SEM_QUE";
        char timeChar[26] = {0};
        char *ARRAY_POINTER;
        char format[25] = "Hi, I'm [%i], today is %s";
        char message[ARRAY_SIZE];

        if ( (shm = shmget(9876, 1, IPC_CREAT | 0777)) == -1 ) {
            perror("shm_open");
            return 1;
        }

        ARRAY_POINTER = shmat(shm, NULL, 0);

        if ((sem = semget(1984, 1, IPC_CREAT | 0777)) == -1) {
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

            time_t seconds = time(NULL);
            seconds = time(NULL);
            memcpy(timeChar, asctime(localtime(&seconds)), 26);      
            timeChar[24] = ' ';        
            memset(message, 0, ARRAY_SIZE);
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