#include "bin.h"

void cd(char *pwd, char *topath){
    if (*topath == '/' || !strcmp(topath, "..") || !strcmp(topath, ".")) {
        chdir(topath);
    }
    else{
            getcwd(pwd, MAXLINE);
            strcat(pwd, "/");
            strcat(pwd, topath);
            chdir(pwd);
    }
}
