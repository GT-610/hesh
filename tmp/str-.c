#include <stdio.h>
#include <string.h>
static char a[1024] = "123456/7890";
char b[1024] = "abcdefghij";
int main(void){
    
    char *tmp;
    tmp = strrchr(a, '/');
    memset(tmp, 0, strlen(tmp));
    printf("%s\n",a);
}
