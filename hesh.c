#include "lib/default.h"
#include "lib/bin.h"
#include <stdio.h>
#include <string.h>

#define MAXBGPROG 32
#define MAXARGS 128
#define HOSTLEN 256
#define PORTLEN 64
#define PATHLINE 2048

/* User Config */
typedef struct config {
    char PATH[PATHLINE];
    char P_HOME[512];
    char P_LBIN[PATHLINE];
    char server_host[256];
    char server_port[64];
    int autoconnect;
} Config;
Config usercfg ,* Usercfg;
/* control var */
char isroot = '$';
int connected = 0;
int indexbg[MAXBGPROG];
int pidprog[MAXBGPROG];
int bg = 0;

/* Server */
int serverfd;

/* base data */
static char cmd[MAXLINE];
static char pwd[MAXLINE];

/* ===== functions ===== */

/* sig_handler */
void child_handler(int sig); 
void sigint_handler(int sig);

/* base functions */
void execute(char * arg);
void paserarg(char * arg, char *args[MAXARGS]);
void sendcmd(int serverfd, char *arg);
void input(); // get command
void inithesh(); // init
/* ======== end ======== */


int main(int argc, char *argv[], char *envp[]){

    inithesh();
    // 判断使用sh的是否是红裤衩外穿的那个人
    if (!strcmp("root", getenv("USER")))
        isroot = '#';

    // 初始化signal
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, child_handler);

    do {

        memset(indexbg,0, sizeof(int) * MAXBGPROG); // clear up indexbg
        input();
        if (!strcmp("quit\n", cmd)) {
            break;
        }
        execute(cmd);
    }while (1);

    puts("exit");
    return 0;
}
void inithesh(){
    FILE *heshrc;
    char *value;
    char configbuf[MAXLINE];

    Usercfg = &usercfg;
    memset(pwd, 0, MAXLINE);
    memset(Usercfg, 0, sizeof(struct config));
    memset(configbuf, 0, MAXLINE);
    
    /* 设置配置文件的路径 */
    strcat(configbuf, getenv("HOME"));
    strcat(configbuf, "/.heshrc");

    heshrc = fopen(configbuf, "r");
    if (heshrc) {
        /* 找到配置文件时 */
        memset(configbuf, 0, MAXLINE);
        /* 读取一行，将字符串与各个配置项对比，将值写入到config中 */
        while (fgets(configbuf, MAXLINE, (FILE*)heshrc)) {
            configbuf[strlen(configbuf)-1] = '\0';
            value = strchr(configbuf, '=');
            *value = '\0'; 
            /*配置项
             * server_host
             * server_port
             * autoconnect
             * */
            if (!strcmp(configbuf, "server_host")) {
                strcat(Usercfg->server_host, value+1);
                printf_clr(Usercfg->server_host, "blue");
            }
            else if (!strcmp(configbuf, "server_port")) {
                strcat(Usercfg->server_port, value+1);
                printf_clr(Usercfg->server_port, "blue");
            }
            else if (!strcmp(configbuf, "autoconnect")){
                Usercfg->autoconnect = atoi(value+1);
            }
            else {
                Usercfg->autoconnect = 0;
            }
        }
        fclose(heshrc);
    }
    if (Usercfg->autoconnect) {
        serverfd = open_clientfd(Usercfg->server_host, Usercfg->server_port);
        connected = 1;
    }
}

void execute(char *arg){

    char *args[MAXARGS];
    /*
     * 如果 connected 是一个非零数则在远程执行
     * 否则在本地执行*/ 
    if (connected) {
        sendcmd(serverfd, arg);
    }
    else {
        paserarg(arg, args); /* get args[] */
        
        /* parse args, if *args[0] == '&' 
         * then skip and print error.*/
        if (*args[0] != '&')
        {
            /* if args[0] in hesh's bins
             * otherwise execute fork()*/
                /* 如果indexbg[0] 不为0，则代表命令中含有 &，则不能执行
                 * connect命令也不能放在后台运行*/

            if (bg)  {


            /*===================   if   ====================*/


                if (!strcmp(args[0], "cd") && indexbg[0] == 0) {
                    if (indexbg[0] == 0) {
                        printf_clr("cd cannot be executed in the background.\n", "red");
                        return;
                    }
                    cd(pwd, args[1]);
                }


                else if (!strcmp(args[0], "connect") && indexbg[0] == 0) {
                    if (indexbg[0] == 0) {
                        printf_clr("connected cannot be executed in the background.\n", "red");
                        return;
                    }
                    if (connected > 0) {
                        printf("Connected: {Host: %s, Prot: %s\n", args[1], args[2]);
                        return;
                    }
                    serverfd = open_clientfd(args[1], args[2]);
                    connected = 1;
                }



                else
                {
                    int n = 0;
                    do {
                            pid_t  pid;
                            if ((pid = fork()) == 0) {
                                if (!strncmp(args[0], "./", 2)) {
                                    /* ignore leader './' */
                                    char *realprog = args[0]+2;
                                    args[0] = realprog;
                                    if (execve(args[0], args, NULL) < 0) 
                                        printf_clr("can't execute the program", "red");
                                }
                                else{
                                    if(execvp(args[0], args) < 0)
                                        printf_clr("command not found...\n", "red");
                                }
                                exit(1);
                            }
                            wait(NULL);
                            n++;
                    } while(args[n]);
                }



            }
            /*===================   if   ====================*/

            else {

            /*===================   else   ====================*/
                if (!strcmp(args[0], "cd")) {
                    cd(pwd, args[1]);
                }
                else if (!strcmp(args[0], "connect")) {
                    if (connected > 0) {
                        printf("Connected: {Host: %s, Prot: %s\n", args[1], args[2]);
                        return;
                    }
                    serverfd = open_clientfd(args[1], args[2]);
                    connected = 1;
                }
                else
                {
                    pid_t  pid;
                    if ((pid = fork()) == 0) {
                        if (!strncmp(args[0], "./", 2)) {
                            /* ignore leader './' */
                            char *realprog = args[0]+2;
                            args[0] = realprog;
                            if (execve(args[0], args, NULL) < 0) 
                                printf_clr("can't execute the program", "red");
                        }
                        else{
                            if(execvp(args[0], args) < 0)
                                printf_clr("command not found...\n", "red");
                        }
                        exit(1);
                    }
                    wait(NULL);
                }
            /*===================   else   ====================*/

            }
        }
    }

}

void paserarg(char * command, char *args[MAXARGS]){
    char * paraments = command;
    char *findspace;
    char *prevarg;
    int indexarg = 0;

    while (*paraments == ' ')  // 忽略领头的空格
        paraments++;
    if (*paraments == '\n')  // 如果遇到换行符直接跳过
       return; 
    paraments[strlen(paraments)-1] = ' '; // 将最后一个字符设置为空

    prevarg = paraments;
    /*
     * 不断的寻找下一个空格，如果找到一个空格就判断下一个字符是否为空格
     * 并把空格替换为0，复制给args数组中。
     * 如果遇到&字符，就把当前的args中的&位置索引赋值到indexbg数组中
     * index_bg控制indexbg的位置*/
    int index_bg = 0;
    while ((findspace = strchr(prevarg, ' '))) {
        for (; *findspace == ' '; findspace++)
            *findspace = '\0';
        args[indexarg++] = prevarg;
        if (!strcmp(args[indexarg-1], "&")) {
            indexbg[index_bg] = indexarg-1;
            index_bg++;
        }
        prevarg = findspace;
    }
    indexbg[0] == 0 ? (bg = 0) : (bg = 1);
    /* set rear to NULL */
    args[indexarg] = (char*)0;


}
void input(){

    memset(cmd, 0, MAXLINE);
    /* printf("{%s}:%s:%c>", getenv("PWD"), getenv("USER"), isroot); */
    printf("\033[0m\033[1;32m%s\033[0m\033[0m\033[1;31m%c\033[0m", getenv("USER"), isroot); 
    fgets(cmd, MAXLINE, stdin);
}

void sigint_handler(int sig){

}

void child_handler(int sig){

}
