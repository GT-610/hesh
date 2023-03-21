#include "lib/default.h"
#define JOBSTATUS(status) status ? printf("running") : printf("stopped")

/* Process status 
 * ===Cannot Be Changed=== */
#define RUNNING 0x0001  
#define STOPPED 0x0000

/* Process mode */
#define BACKGROUND 0x0001
#define PREFIX 0x0000

/* Special sign */
#define AS_OUT_W 0x0010 // >
#define AS_OUT_A 0x0020 // >> 
#define AS_IN 0x0040  // <

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

/* special sign struct */
struct redirect_sign {
    int post[2];
    int type[2];
};
