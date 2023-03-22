#include "lib/hesh.h"
#include "lib/job.h"
#include "lib/config.h"

Config usercfg ,* Usercfg; // User's config of hesh

//job
volatile int job_exist; // number of job
JobList job[MAXJOBS]; // job data
ssize_t joblen = sizeof(JobList); // len of job struct


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
static char cwd[PATH_MAX];

/* base functions */
void f_redirection(char *args[MAXARGS], struct redirect_sign * sign, int * redirect);
void execute(char *argv[], int control); // execute program
void process(char * arg); // get args
void show_terminal(); // get command
void inithesh(); // init to hesh

/* ======== end ======== */

/* sig_handler */

void chld_handler(int sig){
    int olderrno = errno;
    sigset_t mask_all, prev_mask;
    sigfillset(&mask_all);

    pid_t pid;
    while ((pid = waitpid(-1, NULL, 0)) > 0) {
        sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);
        deletejob(job, joblen, pid, job_exist);
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
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
    sigset_t mask_all, prev_mask;
    sigfillset(&mask_all);
    
    sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);
    kill(-prefix_process_group, SIGINT);
    wait(NULL); //回收子进程，而不是通过信号处理函数
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);

}

/* ================= MAIN ================= */
int main(int argc, char *argv[], char *envp[]){

    inithesh();
    initjob(job, joblen*MAXJOBS, job_exist);

    while (1) {
        show_terminal();

        if (fgets(cmd, MAXLINE, stdin) == NULL){
            if (feof(stdin))
                break;
           show_terminal();
           fgets(cmd, MAXLINE, stdin);
        }

        if (feof(stdin))
            break;

        if (!strcmp("quit\n", cmd) | !strcmp("exit\n", cmd)) {
            break;
        }

        process(cmd);
    }

    puts("exit");
    exit(0);
}

void inithesh(){
    FILE *heshrc;
    char *value; // config's value
    char configbuf[MAXLINE]; // get config of hesh

    Usercfg = &usercfg;
    memset(cwd, 0, PATH_MAX);
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

void f_redirection(char *args[MAXARGS], struct redirect_sign * sign, int * redirect){

    int n_redirect = 0;
    int index_redirect;

    for (index_redirect = 0; args[index_redirect]; index_redirect++) {

        if (n_redirect > 1) {
            printf_clr("too many redirection symbols...\n", "r");
            return;
        }
        if (!strcmp(args[index_redirect], ">")) {
            sign->post[n_redirect] = index_redirect;
            sign->type[n_redirect] = AS_OUT_W;
            n_redirect++;
        }
        else if (!strcmp(args[index_redirect], ">>")) {
            sign->post[n_redirect] = index_redirect;
            sign->type[n_redirect] = AS_OUT_A;
            n_redirect++;
        }
        else if (!strcmp(args[index_redirect], "<")) {
            sign->post[n_redirect] = index_redirect;
            sign->type[n_redirect] = AS_IN;
            n_redirect++;
        }

    }




    if (n_redirect) {
        if (sign->type[0] == AS_OUT_W) {
            redirect[0] = open(args[sign->post[0]+1], O_WRONLY | O_CREAT, 0644);
        }
    }

}

void execute(char *args[MAXARGS], int control){

    // 存储命令
    char JOBCMD[PATH_MAX];
    memset(JOBCMD, 0, PATH_MAX);
    for (int i = 0; args[i]; i++){
        strcat(JOBCMD, args[i]);
        strcat(JOBCMD, " ");
    }

    /* struct redirect_sign sign_localtion;    */
    /* int redirect[2]; */
    /* f_redirection(args, &sign_localtion, redirect); */

    /* ===built-in 内置命令 === */
    if (!strcmp(args[0], "cd")) {
        int ret = chdir(args[1]);
        if (ret == 0) {
            if (getcwd(cwd, sizeof(cwd)) != NULL) 
                setenv("PWD", cwd, 1);
            else 
                perror("getcwd() error");
        }
        else
            perror("chdir() error");
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
    /* ==== END ==== */

    /* [> 判断特殊字符 <] */
    /* for (int i = 0; args[i];i++) { */
    /*  */
    /*     if (!strcmp(args[i], "|")) { */
    /*  */
    /*     } else if (!strcmp(args[i], ">>")) { */
    /*  */
    /*     } else if (!strcmp(args[i], "<")) { */
    /*  */
    /*     } else if (!strcmp(args[i], ">")) { */
    /*  */
    /*     } */
    /*  */
    /*  */
    /* } */



    // block SIGCHLD
    sigprocmask(SIG_BLOCK, &mask_one, &prev_one);

    if ((prefix_process = fork()) == -1){
        printf("Child process could not be created\n"); 
        return;
    }
    if (prefix_process == 0) {

        // unblocking SIGCHLD in child
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
    sigprocmask(SIG_SETMASK, &prev_one, NULL); // unblocking SIGCHLD

    prefix_process_group = getppid();
    if (control == PREFIX) {
        waitpid(prefix_process, NULL, 0);
    }

    else {
        printf("Process created with PID: %d\n", prefix_process);
        addjob(job, prefix_process, JOBCMD, job_exist);
    }


}

void process(char *cmd){

    int isstr = 0;
    int n_arg = 0;
    int parameter = 0;

    char stored_cmd[MAXARGS][MAXLEN];
    memset(stored_cmd, 0, MAXARGS*MAXLEN);
    
    while (*cmd == ' ')  // ignore leader space 
        cmd++;
    if (*cmd == '\n')
        return;
    cmd[strlen(cmd)-1] = '\0';
    for (int i = 0; i < strlen(cmd); cmd++) {

        /* 忽略引号中的特殊字符，仅当做字符串存储 */
        if (*cmd == '"') {
            stored_cmd[n_arg][parameter++] = *cmd++;
            while (*cmd != '"') {
                stored_cmd[n_arg][parameter++] = *cmd++;
            }
            stored_cmd[n_arg][parameter++] = *cmd;
            continue;
        }
        if (*cmd == '\'') {
            stored_cmd[n_arg][parameter++] = *cmd++;
            while (*cmd != '\'') {
                stored_cmd[n_arg][parameter++] = *cmd++;
            }
            stored_cmd[n_arg][parameter++] = *cmd;
            continue;
        }

        /*跳到下一个参数*/
        if(*cmd == ' '){
            stored_cmd[n_arg++][parameter] = '\0';
            parameter = 0;
            continue;
        }
        /*===========================================*/

        stored_cmd[n_arg][parameter++] = *cmd;

    }

    /*====init_args======*/
    char * args[MAXARGS];
    initArgs(args, MAXARGS);
    /*====init_args======*/

    int i_cmd = 0, i_args = 0;
    for (; i_args <= n_arg; i_args++) {

        if (!strcmp(stored_cmd[i_args], "&&")) {
            if (args[i_cmd] != NULL)
                /* execute(&args[i_cmd], i_args-i_cmd, PREFIX); */
                execute(&args[i_cmd],  PREFIX);
            else{
                printf("Syntax Error:\n");
                return;
            }
            if (i_args == n_arg) 
                return;
            else
                i_cmd = i_args+1;

        }
        else if (!strcmp(stored_cmd[i_args], "&")) {
            if (i_args+1 <= n_arg && stored_cmd[i_args+1]){
                printf_clr("Syntax Error: `&` must be placed the end of command...\n", "r");
                return;
            }
            if (args[i_cmd] != NULL) 
                /* execute(args, i_args-i_cmd, BACKGROUND); */
                execute(args,  BACKGROUND);
            else{
                printf("Syntax Error:\n");
                return;
            }
            if (i_args == n_arg) 
                return;
            else
                i_cmd = i_args+1;
        }
        else {
            args[i_args] = stored_cmd[i_args]; 
        }
    }


    if (args[i_cmd] != NULL)
        /* execute(&args[i_cmd], i_args-i_cmd-1, PREFIX); */
        execute(&args[i_cmd], PREFIX);
    else
        printf("Syntax Error:\n");

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

