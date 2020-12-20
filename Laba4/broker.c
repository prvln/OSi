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
    int pid_file = open("/tmp/U_R_THE_ONLY_ONE.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc) {
        if(EWOULDBLOCK == errno){
            fprintf(stderr, "%s", "Another instance of broker is running!\nExit");
            return 0;
        }   
    }
    else {
        const int ARRAY_SIZE = 50;
        clock_t start, stop;
        time_t seconds;
        sem_t* sem;
        int shm;
        
        char timeChar[26] = {0};
        char *ARRAY_POINTER;
        char shmName[7] = "SEM_SHM";
        char semName[7] = "SEM_QUE";
        char format[25] = "Hi, I'm [%i], today is %i";
        char message[ARRAY_SIZE];

        shm_unlink(shmName);
        if ( (shm = shm_open(shmName, O_CREAT | O_RDWR, 0777)) == -1 ) {
            perror("shm_open error");
            return 1;
        }
        ftruncate(shm, ARRAY_SIZE);
        ARRAY_POINTER = mmap(0, ARRAY_SIZE, PROT_WRITE, MAP_SHARED, shm, 0);

        sem_unlink(semName);
        if ( (sem = sem_open(semName, O_CREAT, 0777, 1)) == SEM_FAILED ) {
            perror("sem_open error");
            return 1;
        }

        

        while(1){
            seconds = time(NULL);
            start = clock();                   
            if(sem_wait(sem) != -1){
                stop = clock();
                memset(message, 0, ARRAY_SIZE);
                sprintf(message, format, getpid(), seconds);
                memcpy(ARRAY_POINTER, message, ARRAY_SIZE);
                printf("\nTime elapsed: %ims", (stop - start));
                printf("\nMessage sent: %s", message);
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
}