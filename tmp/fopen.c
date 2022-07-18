#include "lib/default.c"
#include <stdio.h>
int main(void){
    FILE *Ferro;
    /* int erro ,ferro; */
    char filebuf[1024];
    /* printf("erro = %d\n", (erro = open("bfuck", O_RDONLY, 0))); */
    Ferro = fopen("tmp.txt", "r");
    int i = 0;
    while(fgets(filebuf, 1024, (FILE*)Ferro))
    {
        i++;
        printf("i = %d.\n%s", i, filebuf);
        
    }
    /* fopen打开失败返回null*/
    /* if (Ferro == 0) { */
    /*    printf("Ferro = 0\n");  */
    /* } */
    /* close(erro); */

    
    /* while ((filebuf[0] = fgetc(Ferro)) == -1) { */
    /*     putc(filebuf[0], stdout); */
    /* } */
    fclose(Ferro);
    return 0;
}


