#include <stdio.h>
int main(void){

    char word[] = "hello, world\n";
    printf("\033[0m\033[1;36m%s\033[0m", word);

    return 0;
}
