#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <types.h>
#include <pcb.h>

extern struct list_head p_low, p_norm, p_high, p_idle;
extern pcb_t *current;
extern int pc_count;
extern int sb_count;

extern void scheduler(void);

#endif
