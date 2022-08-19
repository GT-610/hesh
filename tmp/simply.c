#include "lib/default.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXARGS 128

char cmd[MAXLINE];
pid_t pid;


void child_handler(int sig){
    pid_t recycle;
    int status;
    while (waitpid(-1, NULL, 0) > 0) {
        printf("mainpid = %d ", getpid());
    }
    
}
void exec(char * command){


    char * args[MAXARGS];

    char * next_parameter; // 下一个指令或参数
    char * prev_parameter; // 上一个指令或参数
    int index_parameter = 0; // args[]参数的索引
    int index_prev_cmd = 0; // 如果args拥有多个命令，此为上一个命令的索引

    while (*command == ' ') 
        command++;
    if (*command == '\n')
        return;
    command[strlen(command)-1] = ' ';
    /* vim\n\0*/
    prev_parameter = command;

    while ((next_parameter = strchr(prev_parameter, ' '))) {

        for (; *next_parameter == ' '; next_parameter++)
            *next_parameter = '\0';

        args[index_parameter++] = prev_parameter;


        prev_parameter = next_parameter;
    }
    args[index_parameter] = (char *)0;

    if (args[index_prev_cmd] == (char*)0) {
        return;
    }

    if ((pid = fork()) == 0) {
        int null;
        null = open("/dev/null", O_WRONLY);
        dup2(null, 1);
        if (execvp(args[0], args) < 0)
            printf("command not found...");
        close(null);
        exit(0);
    }
    printf("%d\t%s\n", pid, args[0]);

}

int main(void){
    
    signal(SIGCHLD, child_handler);
    while (1) {
        printf("\n");
        printf("> ");
        memset(cmd, 0, MAXLINE);
        fgets(cmd, MAXLINE, stdin);
        if (!strcmp(cmd, "quit\n")) {
            break;
        }
        exec(cmd);
    }

    printf("exit\n");
    exit(0);
}


