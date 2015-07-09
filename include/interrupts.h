#ifndef INTERRUPTS_H
#define INTERRUPTS_H

extern void interrupt_handler(void);

extern unsigned int status_word[DEV_USED_INTS+1][DEV_PER_INT];

#endif
