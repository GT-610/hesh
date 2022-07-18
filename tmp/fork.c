#include "lib/default.h"
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

void sigchldhandler(int sig);
int main(void){
    pid_t pid[5];

    signal(SIGCHLD, sigchldhandler);

    for (int i = 0 ; i < 1; i++) {
        if ((pid[i] = fork()) == 0) {
            sleep(3);
            printf("hello\n");
        }
    }


    while (1) {
        ;
    }
    /* write(1, "====nnnn\n", 9); */
    return 0;
}
void sigchldhandler(int sig){
    if (wait(NULL) > 0) { 
        write(1, "nnnn\n", 5);
    }
}
