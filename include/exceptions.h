#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

extern void syscall_handler(void);
extern void interrupt_handler(void);
extern void tlb_handler(void);
extern void pgmtrap_handler(void);

#endif
