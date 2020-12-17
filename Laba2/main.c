#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h> 
int main(){
    int pipa[2];
    char format[80] = "I'm [%i], a beloved child of [%i], today is %s";
    time_t seconds;
    if (pipe(pipa) == -1) return 1;

    pid_t childsPID = fork();

    switch(childsPID)
        {
            case 0:{
                char sent[80] = "";
                seconds = time(NULL);
                sprintf(sent, format, getpid(), getppid() ,asctime(localtime(&seconds)));
                write(pipa[1], sent, 80);
                break;
            }
            default:{
                sleep (6);
                char receive[80] = "";
                seconds = time(NULL);
                read(pipa[0], &receive, 80);

                printf("Hi, i'm [%i], read a message from my child [%i]:\n\n\"\n%s\"\n\n", getpid(), childsPID, receive);
                printf("He is 6 seconds late! Now it's %s", asctime(localtime(&seconds)));
                break;
            }
        }
    return 0;
}