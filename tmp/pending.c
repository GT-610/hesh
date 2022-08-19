#include "lib/default.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
static void sig_quit(int signo){
    printf_clr("\nCaught SIGINT\n\n", "r");
    signal(SIGINT, SIG_DFL);
}
int main(void){
    sigset_t newmask, oldmask, pendmask;

    signal(SIGINT, sig_quit);
    
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGINT);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    printf("newmask:%x, oldmask:%x\n", newmask, oldmask);

    sleep(5);

    sigpending(&pendmask);

    printf("pendmask:%x\n", pendmask);

    if (sigismember(&pendmask, SIGINT))
        printf("\nSIGINT pending\n\n");
    sigprocmask(SIG_SETMASK, &oldmask, NULL);

    printf_clr("\nSIGINT unbolock\n", "b");

    sleep(5);
    return 0;
}
