#include <pcb.h>
#include <asl.h>
#include <const.h>
#include <listx.h>
#include <uARMtypes.h>
#include <scheduler.h>
#include <exceptions.h>

extern void test();

void main(void){
  /* 1 */
  state_t *new_areas[4];
  int i, sem_values[MAX_DEVICES];
  semd_t dev_sem[MAX_DEVICES];
  pcb_t *first, *idle;

  for(i=0; i<4; i++){
    switch(i){
    case 0:
      new_areas[i] = (state_t*)INT_NEWAREA;
      new_areas[i]->pc = (memaddr)&interrupt_handler;
      break;
    case 1:
      new_areas[i] = (state_t*)TLB_NEWAREA;
      new_areas[i]->pc = (memaddr)&tlb_handler;
      break;
    case 2:
      new_areas[i] = (state_t*)PGMTRAP_NEWAREA;
      new_areas[i]->pc = (memaddr)&pgmtrap_handler;
      break;
    case 3:
      new_areas[i] = (state_t*)SYSBK_NEWAREA;
      new_areas[i]->pc = (memaddr)&syscall_handler;
      break;
    }

    new_areas[i]->sp = RAM_TOP;
    new_areas[i]->cpsr = STATUS_NULL;
    new_areas[i]->cpsr = STATUS_SYS_MODE
      | STATUS_ALL_INT_DISABLE(new_areas[i]->cpsr)
      | STATUS_DISABLE_TIMER(new_areas[i]->cpsr);
  }

  /* 2 */
  initPcbs();
  initASL();
  
  /* 3 */
  p_low = LIST_HEAD_INIT(p_low);
  p_norm = LIST_HEAD_INIT(p_norm);
  p_high = LIST_HEAD_INIT(p_high);
  p_idle = LIST_HEAD_INIT(p_idle);

  pc_count = 0;
  sb_count = 0;
  current = NULL;

  /* 4 */
  for(i=0; i<MAX_DEVICES; i++){
    sem_values[i] = 0;
    dev_sem[i]->s_semAdd = sem_values[i];
  }

  /* 5 */
  if( (first = allocPcb()) == NULL)
    PANIC();
  first->cpsr = STATUS_NULL;
  first->cpsr = STATUS_SYS_MODE | STATUS_INT_ENABLE(first->cpsr)
    | STATUS_ENABLE_TIMER(first->cpsr);
  first->sp = RAM_TOP - FRAMESIZE;
  first->pc = (memaddr)test;
  insertProcQ(&p_norm, first);
  pc_count++;
  current = first;

  /* 6 */
  if( (idle = allocPcb()) == NULL)
    PANIC();
  idle->cpsr = STATUS_NULL;
  /* a cosa puntano sp e pc? */

  /* 7 */
  scheduler();
}
