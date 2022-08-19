#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void){
    /* int redirect = open("templates", O_WRONLY | O_CREAT, 0666); */
    int redirect = open("templates", O_APPEND | O_WRONLY | O_CREAT, 0666);
    
    pid_t cat = fork();
    if (cat == 0) {
        char *args[3];
        args[0] = "cat";
        args[1] = "tmp.txt";
        args[2] = (char*)0;
        dup2(redirect, 1);
        dup2(redirect, 2);
        if(execvp(args[0], args) < 0){
            printf("command not found...\n");
            kill(getpid(), SIGTERM);
        }
    }
    wait(NULL);

    return 0;
}
