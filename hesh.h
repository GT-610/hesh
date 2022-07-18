#define MAXLINE 1024

typedef struct config {
    char path[MAXLINE];
    char serverhost[64];
    char serverport[8];
    int autoconnect = 0;
} Config;
