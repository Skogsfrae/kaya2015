#ifndef PCB_H
#define PCB_H

#include "listx.h"
#include "types.h"

extern void initPcbs();
extern pcb_t *allocPcb();
extern void freePcb(pcb_t *p);
extern void insertProcQ(struct list_head *q, pcb_t *p);
extern pcb_t *removeProcQ(struct list_head *q);
extern pcb_t *outProcQ(struct list_head *q, pcb_t *p);
extern pcb_t *headProcQ(struct list_head *q);
extern int emptyChild(struct pcb_t *p);
extern void insertChild(struct pcb_t *parent, struct pcb_t *p);
extern struct pcb_t *removeChild(struct pcb_t *p);
extern struct pcb_t *outChild(struct pcb_t *p);

#endif
