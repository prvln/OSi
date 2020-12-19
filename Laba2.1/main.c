#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

int main(){
    int fd;
    char fifoPipa[20] = "/tmp/fifoPipa.txt";
    char format[80] = "I'm [%i], a beloved parent of [%i], today is %s";

    remove(fifoPipa);
    if (mkfifo(fifoPipa, 0777) == -1)
    {
        printf("Error creating FIFO file");
        return 1;
    }
    time_t seconds;
    pid_t childsPID = fork();

    switch(childsPID)
        {
            case 0:{
                sleep (6);
                char receive[80] = "";
                seconds = time(NULL);
                fd = open(fifoPipa, O_RDONLY);
                read(fd, &receive, 80);

                printf("Hi, i'm [%i]. Why dont you read a message from my parent [%i]:\n\n\"\n%s\"\n\n", getpid(), getppid(), receive);
                printf("He is 6 seconds late! Now it's %s", asctime(localtime(&seconds)));
                break; 
            }
            default:{
                char sent[80] = "";
                seconds = time(NULL);
                sprintf(sent, format, getpid(), childsPID ,asctime(localtime(&seconds)));
                fd = open(fifoPipa, O_WRONLY);
                write(fd, sent, 80);
                waitpid(childsPID,0,0);
                break;                
            }
        }
    return 0;
}