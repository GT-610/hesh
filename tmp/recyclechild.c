#include "lib/default.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
pid_t pid[5];
void child_handler(int sig){
    while(waitpid(-1, NULL, 0)>0){
        printf_clr("recycled\n", "r");
        sleep(1);
    }
}

int main(void){
    /* struct sigaction act, oldact; */
    /* act.sa_handler = child_handler; */
    /* sigaddset(&act.sa_mask, SIGCHLD); */
    /* act.sa_flags = SA_NOCLDWAIT; */
    signal(SIGCHLD, child_handler);
    int i = 0;
    while (i < 5) {
            if ((pid[i] = fork()) == 0) {
                printf("pid[%d] == %d\n", i, pid[i]);
                exit(0);
            }
        i++; 
    }
    
}
