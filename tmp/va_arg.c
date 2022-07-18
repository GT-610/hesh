#include <stdarg.h>
#include <stdio.h>

void mysum(char * s, ...){
    va_list ap;
    va_start(ap, s);
    /* for (int j = 0; j < va_arg(ap, int); j++) { */
    /*       */

    /* } */
    printf("%d\n", va_arg(ap, int));
}
int main(void){
    char *args[128];
    mysum((char *)args, 2);
    return 0;
}
