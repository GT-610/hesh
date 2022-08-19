#include "lib/default.h"
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
void child_handler(int sig){
    int olderrno = errno;
    /* char tmp[FILENAME]; */
    sigset_t mask_all, prev_all;

    pid_t pid;
    while ((pid = waitpid(-1, NULL, 0)) > 0) {
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
        /* deletejob(pid); */
        printf_clr("recycling\r\n", "c");
        
        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }
    if (errno != ECHILD)
        Sio_error("waitpid error");
    errno = olderrno;
    /* read(0, tmp, FILENAME); */
}

void chld_handler(int sig){
    while (waitpid(-1, NULL, 0) > 0) {
        printf_clr("recycling\r\n", "c");
    }
}
int main(void){
    char buf[1024];
    int pid;
    memset(buf, 0, 1024);
    signal(SIGCHLD, child_handler);    
    /* pid = fork(); */
    /* if (pid == 0) { */
    /*     sleep(2); */
    /*     printf("\r"); */
    /*     printf_clr("child process\r\n", "y"); */
    /*     sleep(2); */
    /*     exit(0); */
    /* } */
    while (1) {
        printf_clr("Linux\n > ", "r");
        /* fflush(stdout); */
        memset(buf, 0, 1024);
        /* if (fgets(buf, 1024, stdin) == NULL){ */
        /*     printf("fgets return a null pointer\n"); */
        /*     printf("%s\n", buf); */
        /*     return 1; */
        /* } */
        fgets(buf, 1024, stdin);
        if (feof(stdin))
            break;

        printf("%s\n", buf);
    }
    return 0;

}
