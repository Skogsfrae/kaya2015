#include <pcb.h>
#include <asl.h>
#include <arch.h>
#include <const.h>
#include <listx.h>
#include <syscall.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <scheduler.h>
#include <exceptions.h>
#include <interrupts.h>

//#define DEBUG

pcb_t *current;
int pc_count;
int sb_count;
struct list_head p_low=LIST_HEAD_INIT(p_low);
struct list_head p_norm=LIST_HEAD_INIT(p_norm);
struct list_head p_high=LIST_HEAD_INIT(p_high);
struct list_head p_idle=LIST_HEAD_INIT(p_idle);
int dev_sem[MAX_DEVICES];
struct dtpreg_t *devices[(DEV_USED_INTS -1)*DEV_PER_INT];
struct termreg_t *terminals[DEV_PER_INT];

extern void test();

void main(void){
  /* 1 */
  state_t *new_areas[4];
  int i, addr = 0x40;
  pcb_t *first, *idle;
  state_t fproc;

#ifdef DEBUG
  tprint("Populating areas\n");
#endif

  for(i=0; i<4; i++){
    switch(i){
    case 0:
      new_areas[i] = (state_t*)INT_NEWAREA;
      new_areas[i]->pc = (memaddr)interrupt_handler;
      break;
    case 1:
      new_areas[i] = (state_t*)TLB_NEWAREA;
      new_areas[i]->pc = (memaddr)tlb_handler;
      break;
    case 2:
      new_areas[i] = (state_t*)PGMTRAP_NEWAREA;
      new_areas[i]->pc = (memaddr)pgmtrap_handler;
      break;
    case 3:
      new_areas[i] = (state_t*)SYSBK_NEWAREA;
      new_areas[i]->pc = (memaddr)syscall_handler;
      break;
    }

    new_areas[i]->sp = RAM_TOP;
    new_areas[i]->cpsr = STATUS_NULL;
    new_areas[i]->cpsr = new_areas[i]->cpsr | STATUS_SYS_MODE;
    new_areas[i]->cpsr = STATUS_ALL_INT_DISABLE(new_areas[i]->cpsr);
    //new_areas[i]->cpsr = STATUS_DISABLE_TIMER(new_areas[i]->cpsr);
  }

  /* 2 */
#ifdef DEBUG
  tprint("Initializing pcbs and asl\n");
#endif
  initPcbs();
  initASL();
  
  /* 3 */
  /* p_low=LIST_HEAD_INIT(p_low); */
  /* p_norm=LIST_HEAD_INIT(p_norm); */
  /* p_high=LIST_HEAD_INIT(p_high); */
  /* p_idle=LIST_HEAD_INIT(p_idle); */

  pc_count = 0;
  sb_count = 0;
  current = NULL;

  /* 4 */
#ifdef DEBUG
  tprint("Initializing dev semaphores\n");
#endif
  for(i=0; i<MAX_DEVICES; i++)
    dev_sem[i] = 0;
  for(i=0; i<(DEV_USED_INTS - 1)*DEV_PER_INT; i++){
    devices[i] = (memaddr)addr;
    addr += 0x10;
  }
  for(i=0; i<DEV_PER_INT; i++){
    terminals[i] = (memaddr)addr;
    addr += 0x10;
  }

  /* 5 */
#ifdef DEBUG
  tprint("Creating first process\n");
#endif

  fproc.cpsr = STATUS_NULL;
  fproc.cpsr = fproc.cpsr | STATUS_SYS_MODE;
  fproc.cpsr = STATUS_ALL_INT_ENABLE(fproc.cpsr);
  //  fproc.cpsr = STATUS_ENABLE_TIMER(fproc.cpsr);
  fproc.sp = RAM_TOP - FRAMESIZE*2;
  fproc.pc = (memaddr)test;
  create_process(&fproc, PRIO_NORM);
  
  /* if( (first = allocPcb()) == NULL) */
  /*   PANIC(); */
  /* first->p_s.cpsr = STATUS_NULL; */
  /* first->p_s.cpsr = first->p_s.cpsr | STATUS_SYS_MODE */
  /*   | STATUS_ENABLE_INT(first->p_s.cpsr) */
  /*   | STATUS_ENABLE_TIMER(first->p_s.cpsr); */
  /* first->p_s.sp = RAM_TOP - FRAMESIZE; */
  /* first->p_s.pc = (memaddr)&test; */
  /* first->prio = PRIO_NORM; */
  /* insertProcQ(&p_norm, first); */
  /* pc_count++; */
  /* current = first; */

  /* 6 */
#ifdef DEBUG
  tprint("Allocating idle process\n");
#endif
  if( (idle = allocPcb()) == NULL)
    PANIC();
  idle->p_s.cpsr = STATUS_NULL;
  /* a cosa puntano sp e pc? */

  /* 7 */
  //  LDST(&fproc);
#ifdef DEBUG
  tprint("Calling scheduler\n");
#endif
  scheduler();
}
