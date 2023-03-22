#include "hesh.h"
/* User Config */
typedef struct config {
    char PATH[PATHLINE];
    char P_HOME[512];
    char P_LBIN[PATHLINE];
    char server_host[256];
    char server_port[64];
    int autoconnect;
} Config;
