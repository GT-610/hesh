#include "fsocket.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int main(void){

    pid_t pid;
    int fileinput;
    if ((fileinput = open("print", O_WRONLY | O_CREAT)) < 0) {
        printf("%d\n", fileinput);
        exit(0);
    };
    if ((pid = fork()) == 0) {
        dup2(fileinput, 1); 
        for (int i = 0; i < 5; i++) {
            write(1, "hello, world\n", 13);
        }
        close(fileinput);
        exit(0);
    }
    waitpid(-1, NULL, 0);
    write(1, "completed\n", 11);
    close(fileinput);
    return 0;
}
