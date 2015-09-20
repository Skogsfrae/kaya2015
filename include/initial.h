#ifndef INITIAL_H
#define INITIAL_H

#include <types.h>

extern struct list_head p_low, p_norm, p_high, p_idle;
extern pcb_t *current;
extern int pc_count;
extern int sb_count;
extern int dev_sem[MAX_DEVICES];

#endif
