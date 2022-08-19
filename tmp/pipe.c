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

/* cat tmp.txt | grep "C" */
/* size_t unsigned ssize_t long*/

/* stdin 0 stdout 1 stderr 2 */
void chld_handler(int sig){
    int pid;
    while ((pid = waitpid(-1, NULL, 0)) > 0) {
        ;
    }
}
int main(void){

    struct sigaction act_chld;
    act_chld.sa_handler = chld_handler;
    sigaction(SIGCHLD, &act_chld, 0);

    int fd[2]; // fd[0] read fd[1] write
    int ret = pipe(fd);
    if (ret == -1) {
        perror("pipe error\n");
        return 0;
    } 

    pid_t cat = fork(), grep = fork();
    if (cat == 0) {
        close(fd[0]);
        char *args[3];
        args[0] = "cat";
        args[1] = "tmp.txt";
        args[2] = (char*)0;
        dup2(STDIN_FILENO, fd[0]);
        dup2(fd[1], 1);
        if(execvp(args[0], args) < 0){
            printf("command not found...\n");
            kill(getpid(), SIGTERM);
        }
    }
    if (grep == 0) {
        close(fd[1]);
        size_t sn;
        char msg[1024];
        char *args[3];
        args[0] = "grep";
        args[1] = "\"C\"";
        args[2] = (char*)0;

        dup2(fd[0], 0);
        dup2(STDOUT_FILENO, fd[1]);
        read(0, msg, 1024);
        write(1, msg, sn);
        /* if(execvp(args[0], args) < 0){ */
        /*     printf("command not found...\n"); */
            kill(getpid(), SIGTERM);
        /* } */
    }

    while (1) {
        ;
    }
    return 0;

}













