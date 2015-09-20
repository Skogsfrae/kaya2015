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

pcb_t *current;
int pc_count;
int sb_count;
struct list_head p_low=LIST_HEAD_INIT(p_low);
struct list_head p_norm=LIST_HEAD_INIT(p_norm);
struct list_head p_high=LIST_HEAD_INIT(p_high);
struct list_head p_idle=LIST_HEAD_INIT(p_idle);
int dev_sem[MAX_DEVICES];

extern void test();

void idlec(void)
{
  while(1);
}

void main(void)
{
  state_t *new_areas[4];
  int i, j, addr = DEV_REG_START;
  pcb_t *first, *idle;
  state_t fproc;

  /* 1 - Populating the four New Areas */
  for(i=0; i<4; i++)
    {
      switch(i)
	{
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
    }

  /* 2 - Initializing Level 2 data structures */
  initPcbs();
  initASL();

  /* 3 - Initializing nucleus maintained variables */
  pc_count = 0;
  sb_count = 0;
  current = NULL;

  /* 4 - Initializing nucleus semaphores */
  for(i=0; i<MAX_DEVICES; i++)
    dev_sem[i] = 0;

  /* 5 - Instantiating the first process */
  fproc.cpsr = STATUS_NULL;
  fproc.cpsr = fproc.cpsr | STATUS_SYS_MODE;
  fproc.cpsr = STATUS_ALL_INT_ENABLE(fproc.cpsr);
  fproc.sp = RAM_TOP - FRAME_SIZE;
  fproc.pc = (memaddr)test;
  create_process(&fproc, PRIO_NORM);

  /* 6 - Setting up idle process */
  if( (idle = allocPcb()) == NULL)
    PANIC();
  idle->p_s.cpsr = STATUS_NULL;
  idle->p_s.cpsr = idle->p_s.cpsr | STATUS_SYS_MODE;
  idle->p_s.cpsr = STATUS_ALL_INT_ENABLE(idle->p_s.cpsr);
  idle->p_s.sp = RAM_TOP - FRAMESIZE;
  idle->p_s.pc = (memaddr)idlec;
  insertProcQ(&p_idle, idle);

  /* 7 - Calling scheduler */
  scheduler();
}
