#include "hesh.h"
#include <sys/types.h>
#include <unistd.h>


void sbuf_init(sbuf_t *sp, int n){
    /* 为buf（一个指向int的指针）分配空间 */
    sp->buf = calloc(n, sizeof(int)); 
    sp->n = n;
    sp->front = sp->rear = 0;
    sem_init(&sp->mutex, 0, 1);
    sem_init(&sp->slots, 0, n);     // 将slots赋值为n，此时共有n个空槽
    sem_init(&sp->items, 0, 1);
}

/* 清除buffer sp */
void sbuf_deinit(sbuf_t *sp){
    free(sp->buf);
}

void sbuf_insert(sbuf_t *sp, int item){
    sem_wait(&sp->slots);   // 将slots减1, 减少了一个空槽 
    sem_wait(&sp->mutex);
    /*
     * 将sp->rear的值加一，与n进行%运算
     * 保证将item添加到buf的最后一个项中
     * 与n进行%运算的意思是不断更新这个共享缓冲区
     * 如果缓冲区满了就从第一个开始覆盖，周而复始
     * 来达到重复利用的目的 */
    sp->buf[(++sp->rear)%(sp->n)] = item;   // 添加一个项目
    sem_post(&sp->mutex);
    sem_post(&sp->items);   // 将item加1，表示增添了一个可用的项目
}

int sbuf_remove(sbuf_t *sp){
    int item;
    sem_wait(&sp->items);
    sem_wait(&sp->mutex);
    /*
     * 将sp->front的这个项加一，与n进行%运算
     * 不断更替第一个项的索引*/
    item = sp->buf[(++sp->front)%(sp->n)]; // 移除这个项目
    sem_post(&sp->mutex);                                           
    sem_post(&sp->slots);
    return item;
}


/* ================= Socket ================= */
/* ================= Socket ================= */
/* ================= Socket ================= */
int open_clientfd(char *hostname, char *port){

    int clientfd , temp;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG;
    temp = getaddrinfo(hostname, port, &hints, &listp); 
    
    for (p = listp; p; p = p->ai_next) {
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
                < 0) continue;    

        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) 
            break;
        close(clientfd);
    }

    freeaddrinfo(listp);
    if (!p)
        return -1;
    else
        return clientfd;
}

int open_listenfd(char *port){
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1, temp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    /* AI_PASSIVE
     *   *  如果设置此标志，且nodename(主机名)是NULL，那么返回的
     * socket地址可以用于bind，返回的地址是通配符地址(IPv4, INADDR_ANY
     * IPv6, IN6ADDR_ANY_INIT)。一般用于Server中，就可以使用这个通配符地址
     * 用来接收任何请求主机地址的连接。
     *
     *   *  如果nodename(主机名)不是NULL, 那么AI_PASSIVE将被忽略；
     *   *  如果未设置AI_PASSIVE标志，返回的网络地址会被设置为
     * */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    temp = getaddrinfo(NULL, port, &hints, &listp);

    for (p = listp; p; p = p->ai_next) {
        /* Creat a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
                < 0) continue;

        /* 配置服务器
         * int setsockopt( int socket, int level, int option_name,
         * const void *option_value, size_t ，ption_len);
         * SO_REUSEADDR 启用地址复用，使不同的服务器转换成同一个外部
         * 地址，但因为端口号不同，不会导致冲突。
         * 当option_value不等于0时打开，否则关闭。
         * int level 一般设置为SOL_SOCKET
         * */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                (const void *)&optval, sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) 
            break; /* Success to exit */
        close(listenfd);
    }

    /* Clear up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;
    if (listen(listenfd, LISTENQ) < 0) {
       close(listenfd);
       return -1;
    }

    return listenfd;
}
/* ================= Socket ================= */
/* ================= Socket ================= */
/* ================= Socket ================= */

void printf_clr(const char *s, const char *color){

    switch (*color) {
        case 'r': // red
            printf("\033[0m\033[1;31m%s\033[0m", s);
            break;
        case 'g': // green
            printf("\033[0m\033[1;32m%s\033[0m", s);
            break;
        case 'y': // yellow
            printf("\033[0m\033[1;33m%s\033[0m", s);
            break;
        case 'b': // blue
            printf("\033[0m\033[1;34m%s\033[0m", s);
            break; 
        case 'p': // pink
            printf("\033[0m\033[1;35m%s\033[0m", s);
            break;
        case 'c': // cyan
            printf("\033[0m\033[1;36m%s\033[0m", s);
            break;
        default:
            printf("%s", s);
    }

}

void initArgs(char **args, int n){
    for (int i = 0; i < n; i++) 
        args[i] = NULL;
}
/* void call_execArgsPiped(char **args){ */
/*             char *Args[2][MAXARGS]; */
/*             initArgs(Args[0], MAXARGS); */
/*             initArgs(Args[1], MAXARGS); */
/*             for (int j; j < i; j++)  // copy args to Args[0] */
/*                 Args[0][j] = args[j]; */
/* } */

void execArgsPiped(char **parsed, char **toparsed, pid_t pid_group){
    // 0 is read end, 1 is write end;
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized\n");
        return;
    }
    if ((p1 = fork()) == -1){
        printf("Child process could not be created\n"); 
        return;
    }
    if (p1 == 0) {


        setpgid(0, 0); // 将自己的进程号添加到自己的进程组

        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO); //将管道的写端已经复制到标准输出了
        close(pipefd[1]);
        if (!strncmp(parsed[0], "./", 2)) { // ignore leader './'
            char *prog = parsed[0]+2;
            parsed[0] = prog;
            if (execve(parsed[0], parsed, NULL) < 0){
                printf_clr("can't execute the program", "red");
                exit(8);
            }
        }
        else{
            if(execvp(parsed[0], parsed) < 0){
                printf_clr("command not found...\n", "red");
                /* kill(getpid(), SIGTERM); */
                exit(8);
            }
        }
    } else {
        if ((p2 = fork()) == -1){
            printf("Child process could not be created\n"); 
            return;
        }
        if (p2 == 0) {

            setpgid(0, p1); // 将自己的进程号添加到p1的进程组中
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO); //将管道的写端已经复制到标准输出了
                                           //所以不需要原来的文件描述符了
            close(pipefd[0]);
            if (!strncmp(parsed[0], "./", 2)) { // ignore leader './'
                char *prog = parsed[0]+2;
                parsed[0] = prog;
                if (execve(parsed[0], parsed, NULL) < 0){
                    printf_clr("can't execute the program", "red");
                    exit(8);
                }
            } else {
                if(execvp(parsed[0], parsed) < 0){
                    printf_clr("command not found...\n", "red");
                    /* kill(getpid(), SIGTERM); */
                    exit(8);
                }
            }
        
        }

    }
    pid_group = p1; // 返回进程组号
}



/*************************************************************
 * The Sio (Signal-safe I/O) package - simple reentrant output
 * functions that are safe for signal handlers.
 *************************************************************/

/* Private sio functions */

/* $begin sioprivate */
/* sio_reverse - Reverse a string (from K&R) */
static void sio_reverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* sio_ltoa - Convert long to base b string (from K&R) */
static void sio_ltoa(long v, char s[], int b) 
{
    int c, i = 0;
    int neg = v < 0;

    if (neg)
	v = -v;

    do {  
        s[i++] = ((c = (v % b)) < 10)  ?  c + '0' : c - 10 + 'a';
    } while ((v /= b) > 0);

    if (neg)
	s[i++] = '-';

    s[i] = '\0';
    sio_reverse(s);
}

/* sio_strlen - Return length of string (from K&R) */
static size_t sio_strlen(char s[])
{
    int i = 0;

    while (s[i] != '\0')
        ++i;
    return i;
}
/* $end sioprivate */

/* Public Sio functions */
/* $begin siopublic */

ssize_t sio_puts(char s[]) /* Put string */
{
    return write(STDOUT_FILENO, s, sio_strlen(s)); //line:csapp:siostrlen
}

ssize_t sio_putl(long v) /* Put long */
{
    char s[128];
    
    sio_ltoa(v, s, 10); /* Based on K&R itoa() */  //line:csapp:sioltoa
    return sio_puts(s);
}

void sio_error(char s[]) /* Put error message and exit */
{
    sio_puts(s);
    _exit(1);                                      //line:csapp:sioexit
}
/* $end siopublic */

/*******************************
 * Wrappers for the SIO routines
 ******************************/
ssize_t Sio_putl(long v)
{
    ssize_t n;
  
    if ((n = sio_putl(v)) < 0)
	sio_error("Sio_putl error");
    return n;
}

ssize_t Sio_puts(char s[])
{
    ssize_t n;
  
    if ((n = sio_puts(s)) < 0)
	sio_error("Sio_puts error");
    return n;
}

void Sio_error(char s[])
{
    sio_error(s);
}



void unix_error(char *msg){
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

void printf_security(const char *s, size_t n){
  if (write(1, s, n) < 0){
      perror("write: ");
      exit(-1);
  }
}


handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

int sendfile(int sendfd, char *filename){
    int ptfile;
    long rsize;
    char buf[MAXLINE];

    if ((ptfile = open(filename, O_RDONLY, 0)) < 0)
        perror("open");

    while ((rsize = read(ptfile, buf, MAXLINE)) > 0) {
        write(sendfd, buf, rsize); 
    }

    close(ptfile);
    return 0;
}

int recvfile(int recvfd, char *filename){

    long rsize;
    int ptfile;
    char buf[MAXLINE];

    /* O_CREAT: isn't exist, then creat new file.
     * O_WRONLY: Only write. 
     * S_IRUSR: creator can read.
     * S_IWUSR: creator can write.
     * */
    if ((ptfile = open(filename, O_CREAT | O_WRONLY,
                    S_IRUSR | S_IWUSR))< 0){
        perror("open");
    }

    while ((rsize = read(recvfd, buf, MAXLINE)) > 0) {
        write(ptfile, buf, rsize); 
    }

    close(ptfile);
    
    return 0;
}
