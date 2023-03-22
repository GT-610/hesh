#include "hesh.h"
#define JOBSTATUS(status) status ? printf("running") : printf("stopped")

//job
/* job struct */
typedef struct jobstatus {
    int exist;
    int id; // minimum unsigned int
    pid_t pid; // PID
    int status_code;
    char *command;
}JobList;


/* job functions */
void initjob(JobList * job, ssize_t joblen, int job_exist);

void addjob(JobList * job, pid_t job_pid, char command_msg[FILENAME], int job_exists);
void deletejob(JobList * job, ssize_t joblen, pid_t job_pid, int job_exists);

