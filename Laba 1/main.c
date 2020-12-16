#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define statusDEAD 0

void atexitResult(){
    pid_t myPID = getpid();
    pid_t parentPID = getppid();
    printf("[%x] Atexit has been started, oh no, i'm dying... \n",myPID);
    printf("[%x] An id of the process (PID): %x \n", myPID, myPID);
    printf("[%x] An id of the parent process (PPID): %x \n\n", myPID, parentPID);
    }

void forkResult(pid_t pid){
    switch(pid){
        case 0:{
            pid_t myPID = getpid();
            pid_t parentPID = getppid();
            printf("[%x] My PID is: %x \n", myPID, myPID);
            printf("[%x] I'm child of process with PID: %x \n\n", myPID, parentPID);
            break;
        }
        case -1:{ 
            printf("[%x] T_T my child state is unknown \n\n", getpid());
            break;
        }
        default:{
            int endOfLifePID = 0;
            printf("[%x] I'm parent of %x \n", getpid(), pid);
            printf("[%x] Starting to wait for my children %x to die\n\n", getpid(), pid);
            endOfLifePID = waitpid(pid,statusDEAD,0);
            printf("[%x] Process with PID (%x) is dead \n\n",getpid(), endOfLifePID);
            break;
        }
    }
}

int main(){
    int error = atexit(atexitResult);
    if (error) { printf("[%x] atexit returned error: %i \n\n", getpid(), error); }
    pid_t forkPID = fork();
    forkResult(forkPID);
    return 0;
}