#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

extern void copy_state(state_t *dest, state_t *src);
extern void syscall_handler(void);
extern void interrupt_handler(void);
extern void tlb_handler(void);
extern void pgmtrap_handler(void);

#endif
