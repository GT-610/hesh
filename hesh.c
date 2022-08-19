#include "hesh.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXJOBS 64 // max jobs
#define MAXBGPROG 32 // max bg program
#define MAXARGS 256 // max parameters

Config usercfg ,* Usercfg; // User's config of hesh

volatile int job_exist; // number of job
JobList job[MAXJOBS]; // job chart
ssize_t joblen = sizeof(JobList);


/* control var */
char isroot = '$'; 
int connected = 0;
pid_t prefix_process_group; //gpid of runing process
pid_t prefix_process; // pid of runing process 
sigset_t mask_all, mask_one, prev_one;

struct sigaction act_chld, act_int, act_tsip;

/* Server */
int serverfd;

/* base data */
static char cmd[MAXLINE];
static char pwd[MAXLINE];

/* base functions */
void execute(char *argv[], int control); // execute program
void exec_bg(char *argv[]); // execute program in background
void Process(char * arg); // get args
void show_terminal(); // get command
void inithesh(); // init to hesh

/* job functions */
void initjob();
void addjob(pid_t job_pid, char command[FILENAME]);
void deletejob(pid_t job_pid);

/* ======== end ======== */

/* sig_handler */
void chld_handler(int sig){
    int olderrno = errno;
    /* char tmp[FILENAME]; */
    sigset_t mask_all, prev_all;

    pid_t pid;
    while ((pid = waitpid(-1, NULL, 0)) > 0) {
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
        deletejob(pid);
        sigprocmask(SIG_SETMASK, &prev_all, NULL);
        if (!waitpid(-1, NULL, WNOHANG)) // 如果后台进程组还有其他进程未执行完毕，则立马返回
            return;
    }
    if (errno != ECHILD)
        Sio_error("waitpid error");
    errno = olderrno;
}

void tsip_handler(int sig){
    write(1, "\n", 2);
    kill(-prefix_process_group, SIGTSTP);
}

void sigint_handler(int sig){
    write(1, "\n", 2);
    if (prefix_process == 0) {
        return;
    }
    kill(-prefix_process_group, SIGINT);
}

/* ================= MAIN ================= */
int main(int argc, char *argv[], char *envp[]){

    inithesh();
    initjob();

    while (1) {
        show_terminal();
        if (fgets(cmd, MAXLINE, stdin) == NULL){
            if (feof(stdin))
                break;
            continue;
        }
        if (!strcmp("quit\n", cmd)) {
            break;
        }
        Process(cmd);
    }

    puts("exit");
    exit(0);
}

void inithesh(){
    FILE *heshrc;
    char *value; // config's value
    char configbuf[MAXLINE]; // get config of hesh

    Usercfg = &usercfg;
    memset(pwd, 0, MAXLINE);
    memset(Usercfg, 0, sizeof(struct config));
    memset(configbuf, 0, MAXLINE);


    act_chld.sa_handler = chld_handler;
    act_int.sa_handler = sigint_handler;
    act_tsip.sa_handler = tsip_handler;
    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);

    sigaction(SIGCHLD, &act_chld, 0);
    sigaction(SIGINT, &act_int, 0);
    sigaction(SIGTSTP, &act_tsip, 0);

    // 判断使用sh的是否是红裤衩外穿的那个人
    if (!strcmp("root", getenv("USER")))
        isroot = '#';

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
                /* printf_clr(Usercfg->server_host, "blue"); */
            }
            else if (!strcmp(configbuf, "server_port")) {
                strcat(Usercfg->server_port, value+1);
                /* printf_clr(Usercfg->server_port, "blue"); */
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
void execute(char *args[MAXARGS], int control){


    if (!strcmp(args[0], "cd")) {
        chdir(args[1]);
        return;
    }

    else if (!strcmp(args[0], "jobs")) {
        printf("ID\t PID\t JOB_STATUS\t CMD\t\n");
        for (int i = 0; i < MAXJOBS; ++i) {
            if (job[i].exist == 1) {
                printf("%2d\t %3d\t ", job[i].id, job[i].pid);        
                JOBSTATUS(job[i].status_code);
                printf("   \t %s\t\n", job[i].command);
            }
        }
        printf("Number Of Jobs: %d\n", job_exist);
        return;
    }

    if ((prefix_process = fork()) == -1){
        printf("Child process could not be created\n"); 
        return;
    }
    if (prefix_process == 0) {

        signal(SIGINT, SIG_IGN);

        sigprocmask(SIG_SETMASK, &prev_one, NULL);
        if (!strncmp(args[0], "./", 2)) { // ignore leader './'
            char *realprog = args[0]+2;
            args[0] = realprog;
            if (execve(args[0], args, NULL) < 0){
                printf_clr("can't execute the program", "red");
                kill(getpid(), SIGTERM);
            }
        }
        else{
            if(execvp(args[0], args) < 0){
                printf_clr("command not found...\n", "red");
                /* kill(getpid(), SIGTERM); */
                exit(8);
            }
        }
    }

    prefix_process_group = getppid();
    if (control == PREFIX) {
        waitpid(prefix_process, NULL, 0);
    }
    else{
        printf("Process created with PID: %d\n", prefix_process);
        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        addjob(prefix_process, args[0]);
        sigprocmask(SIG_SETMASK, &prev_one, NULL);
    }
}

void Process(char * command){

    char * args[MAXARGS];

    // init args
    for (int i = 0; i < MAXARGS; i++) {
        args[i] = (char *)0;
    }

    char * next_parameter; // 下一个参数
    char * prev_parameter; // 上一个参数
    int index_parameter = 0; // args[]参数的索引
    int index_prev_cmd = 0; // 如果args拥有多个命令，此为上一个命令的索引

    while (*command == ' ')  // ignore leader space 
        command++;
    if (*command == '\n')
        return;
    command[strlen(command)-1] = ' '; // 将最后一个字符设置成空格 
    
    prev_parameter = command;  // 开始处理

    /* 
     * 用strchr找到空格，将空格设置为0后，将prev_parameter的值赋值给args
     * 再将prev_parameter指向下一个参数（next_parameter），一直循环，直到
     * 没有参数为止*/
    while ((next_parameter = strchr(prev_parameter, ' '))) {

        for (; *next_parameter == ' '; next_parameter++)
            *next_parameter = '\0'; // 此时next_parameter指向下一个参数的起始位置

        /* 
         * 判断，如果是特殊字符就直接运行跳过赋值，然后重新设置prev_parameter的值*/
        if (!strcmp(prev_parameter, "&&")) {
            if (args[index_prev_cmd] == (char *)0) {
                printf_clr("Syntax Error: No command...\n", "r");
                return;
            }
            execute(&args[index_prev_cmd], PREFIX);
            index_prev_cmd = index_parameter;
            prev_parameter = next_parameter; // ignore special character
            continue;
        }
        if (!strcmp(prev_parameter, "&")) {
            if (args[index_prev_cmd] == (char *)0) {
                printf_clr("Syntax Error: There is no command before the `&`...\n", "r");
                return;
            }
            execute(&args[index_prev_cmd], BACKGROUND);
            index_prev_cmd = index_parameter;
            prev_parameter = next_parameter;
            continue;
        }
        
        args[index_parameter++] = prev_parameter; // 赋值

        prev_parameter = next_parameter; // 指向下一个字符
    }

    if (args[index_prev_cmd] == (char *)0) { 
        return;
    }

    execute(&args[index_prev_cmd], PREFIX);

}


void initjob(){
    memset(&job, 0, joblen * MAXJOBS);
    job_exist = 0;
}

void addjob(pid_t job_pid, char command_msg[FILENAME]){
    int post; // 查找joblist中的空位置
    for (post = 0; job[post].exist != 0 && post < MAXJOBS; ++post)
        ; 
    if (post >= MAXJOBS) {
        sio_puts("joblist is full\n");
        return;
    }

    job[post].id = post;
    job[post].pid = job_pid;
    job[post].status_code = RUNNING;
    job[post].exist = 1;
    strcpy(job[post].command, command_msg);
    job_exist++;
}

void deletejob(pid_t job_pid){
    int i;
    for (i = 0; i < MAXJOBS; ++i) {
       if (job[i].pid == job_pid) 
           break;
    }
    printf("\n[%d]\t +%d\t done\t %s\t\n", job[i].id, job[i].pid, job[i].command);
    memset(&job[i], 0, joblen);
    job_exist--;
}

void show_terminal(){

    time_t t;
    struct tm * lt;
    time(&t);
    lt = localtime(&t);

    printf("\033[0m\033[1;35m{\033[0m\
\033[0m\033[1;31m%s\033[0m\
\033[0m\033[1;35m}^\033[0m\
\033[0m\033[1;35m[\033[0m\
\033[0m\033[1;32m%s\033[0m\
\033[0m\033[1;35m]^\033[0m\
\033[0m\033[1;35m(\033[0m\
\033[0m\033[1;36m%d\033[0m\
\033[0m\033[1;35m:\033[0m\
\033[0m\033[1;36m%d\033[0m\
\033[0m\033[1;35m:\033[0m\
\033[0m\033[1;36m%d\033[0m\
\033[0m\033[1;35m)\n\033[0m\
\033[0m\033[1;32m %c \033[0m"
            ,getenv("USER"), getenv("PWD"),
            lt->tm_hour, lt->tm_min,
            lt->tm_sec, isroot);
    memset(cmd, 0, MAXLINE);
    fflush(stdout);
}

