#include "lib/default.h"
#define JOBSTATUS(status) status ? printf("running") : printf("stopped")
#define RUNNING 0x0001
#define STOPPED 0x0000
#define BACKGROUND 0x0010
#define PREFIX 0x0000

/* User Config */
typedef struct config {
    char PATH[PATHLINE];
    char P_HOME[512];
    char P_LBIN[PATHLINE];
    char server_host[256];
    char server_port[64];
    int autoconnect;
} Config;

/* job struct */
typedef struct jobstatus {
    int exist;
    int id; // minimum unsigned int
    pid_t pid; // PID
    int status_code;   
    char command[FILENAME];
}JobList;
