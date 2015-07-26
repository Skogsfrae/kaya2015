#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <uARMconst.h>

extern void interrupt_handler(void);

extern unsigned int status_word[MAX_DEVICES];

#endif
