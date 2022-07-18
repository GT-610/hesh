#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXARGS 128
char cmd[8192] = "    ls   -al   -h  -t   ";

int main(void)
{
    char *arg = cmd;
    {
        while (*arg == ' ') {
            arg++;
        }
        arg[strlen(arg)-1] = ' ';  // 将最后一个换行符替换成 '\0'
    }
    char *args[MAXARGS];

    /* arch$>   ls  -al -h */
    {
        // 设置参数
        char *findspace;
        char *prevarg = arg;
        int indexarg = 0;
        /* int countspace = 1; */
        while ((findspace = strchr(prevarg, ' '))) {

            for (; *findspace == ' '; findspace++)
                *findspace = '\0';

            args[indexarg++] = prevarg;
            prevarg = findspace;
        }
        args[indexarg] = (char*)0;
    }


    for (int i = 0; args[i]; i++) 
        printf("i = %d\n--@&&&@-@%s@-@&&&@--\n",i, args[i]); 
    printf("\n");
}

