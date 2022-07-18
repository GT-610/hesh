#include "default.h"
#include <fcntl.h>
#include <netdb.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
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

void printf_clr(const char *s, const char *color){

    switch (*color) {
        case 'r': // red
            printf("\033[0m\033[1;31m%s\033[0m", s);
            break;
        case 'g': // green
            printf("\033[0m\033[1;31m%s\033[0m", s);
            break;
        case 'y': // yellow
            printf("\033[0m\033[1;31m%s\033[0m", s);
            break;
        case 'b': // blue
            printf("\033[0m\033[1;31m%s\033[0m", s);
            break; 
        case 'p': // pink
            printf("\033[0m\033[1;31m%s\033[0m", s);
            break;
        case 'c': // cyan
            printf("\033[0m\033[1;31m%s\033[0m", s);
            break;
        default:
            printf("%s", s);
    }

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
