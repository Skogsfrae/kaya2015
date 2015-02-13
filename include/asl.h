#ifndef ASL_H
#define ASL_H

#include "listx.h"

extern void initASL();
extern int insertBlocked(int *semAdd, struct pcb_t *p);
extern struct pcb_t *removeBlocked(int *semAdd);

#endif
