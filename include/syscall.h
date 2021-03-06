#ifndef SYSCALL_H
#define SYSCALL_H

#include <types.h>
#include <const.h>

extern int create_process(state_t *statep, priority_enum prio);
extern void terminate_process(pid_t pid);
extern void verhogen(int *semaddr, int weight);
extern void passeren(int *semaddr, int weight);
extern void specify_exception_state_vector(state_t **state_vector);
extern void get_cpu_time(cputime_t *global, cputime_t *user);
extern void wait_for_clock(void);
extern unsigned int wait_for_io(int intlNo, int dnum, int waitForTermRead);
extern pid_t get_pid(void);
extern pid_t get_ppid(void);

#endif
