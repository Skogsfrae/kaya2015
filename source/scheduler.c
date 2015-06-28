#include <pcb.h>
#include <const.h>
#include <types.h>
#include <syscall.h>
#include <libuarm.h>
#include <initial.h>
#include <uARMtypes.h>
#include <uARMconst.h>

struct list_head p_low=LIST_HEAD_INIT(p_low);
struct list_head p_norm=LIST_HEAD_INIT(p_norm);
struct list_head p_high=LIST_HEAD_INIT(p_high);
struct list_head p_idle=LIST_HEAD_INIT(p_idle);
cputime_t tick, tod;
pcb_t *current;
int pc_count;
int sb_count;
int clock_ticks = 0;

void scheduler(void){

  while(1){
    /* 
     * 1 aggiornare il timer di current
     * 2 modificare current->state
     * 3 selezionare il prossimo
     * 4 aggiornare il cputimer
     * 5 lanciare il prossimo processo (ldst)
     */
    if((clock_ticks++) == 20){
      if((headBlocked(&dev_sem[CLOCK_SEM])) != NULL)
	verhogen(&dev_sem[CLOCK_SEM], 1);
      clock_ticks = 0;
    }
    
    tod = getTODLO();
    current->global_time += tod - current->elapsed_time;
#ifdef DEBUG
    tprint("Scheduler: metto il processo in coda\n");
#endif
    if(current->state != WAITING){
      current->state = READY;
      #ifdef DEBUG
      tprint("Scheduler: burp!\n");
      #endif
      switch(current->prio){
      case PRIO_LOW:
	insertProcQ(&p_low, current);
	break;
      case PRIO_NORM:
	#ifdef DEBUG
	tprint("Scheduler: scassacci la minchia\n");
	#endif
	insertProcQ(&p_norm, current);
	break;
      case PRIO_HIGH:
	insertProcQ(&p_high, current);
	break;
      }
    }

#ifdef DEBUG
    tprint("Scheduler: seleziono il processo dalla coda\n");
#endif
    if( (current = headProcQ(&p_high))
	!= NULL && (current->state != WAITING)){
      outProcQ(&p_high, current);
      current->elapsed_time = tod;
      current->state = RUNNING;
    }
    else{
      if( (current = headProcQ(&p_norm))
	  != NULL && (current->state != WAITING)){
	#ifdef DEBUG
	tprint("Scheduler: corretto\n");
	#endif
	outProcQ(&p_norm, current);
	current->elapsed_time = tod;
	current->state = RUNNING;
      }
      else{
	if( (current = headProcQ(&p_low))
	    != NULL && (current->state != WAITING)){
	  outProcQ(&p_low, current);
	  current->elapsed_time = tod;
	  current->state = RUNNING;
	}
	else{
	  /* shut down */
	  if(pc_count == 0)
	    HALT();
	  else{
	    /* deadlock */
	    if(pc_count > 0 && sb_count == 0){
#ifdef DEBUG
	      tprint("Scheduler: deadlock\n");
#endif
	      PANIC();
	    /* idle */
	    }
	    else{
#ifdef DEBUG
	      tprint("Scheduler: calling idle process\n");
#endif
	      current = headProcQ(&p_idle);
	      current->elapsed_time = tod;
	      current->state = RUNNING;
	    }
	  }
	}
      }
    }

    setTIMER(SCHED_TIME_SLICE);
    LDST(&current->p_s);
  }
}
