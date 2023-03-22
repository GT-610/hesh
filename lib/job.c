#include "job.h"
#include <unistd.h>
void initjob(JobList * job, ssize_t joblen, int job_exists){
    memset(job, 0, joblen);
    job_exists = 0;
}

void addjob(JobList * job, pid_t job_pid, char command_msg[FILENAME], int job_exists){
    int post; // 查找joblist中的空位置
    for (post = 0; job[post].exist != 0 && post < MAXJOBS; ++post)
        ;
    if (post >= MAXJOBS) {
        sio_puts("joblist is full\n");
        return;
    }

    job[post].id = post;
    job[post].pid = job_pid;
    job[post].status_code = RUNNING;
    job[post].exist = 1;
    job[post].command = command_msg;
    job_exists++;
}

void deletejob(JobList * job, ssize_t joblen, pid_t job_pid, int job_exists){
    int i;
    for (i = 0; i < MAXJOBS; ++i) {
       if (job[i].pid == job_pid)
           break;
    }
    /* printf("\n[%d]\t +%d\t done\t %s\t\n", job[i].id, job[i].pid, job[i].command); */
    // printf status
    write(1, "\n[", 3);
    sio_putl(job[i].id);
    write(1, "]\t +", 5);
    sio_putl(job[i].pid);
    write(1, "\t done\t ", 9);
    sio_puts(job[i].command);
    write(1, "\n", 2);
    

    memset(&job[i], 0, joblen);
    job_exists--;
}
