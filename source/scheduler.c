#include <pcb.h>
#include <const.h>
#include <types.h>
#include <libuarm.h>
#include <uARMconst.h>
#include <scheduler.h>

struct cputime_t temp, tick, tod;
struct list_head p_low, p_norm, p_high, p_idle;
pcb_t *current;
int pc_count;
int sb_count;

void scheduler(void){

  while(1){
    /* 
     * 1 aggiornare il timer di current
     * 2 modificare current->state
     * 3 selezionare il prossimo
     * 4 aggiornare il cputimer
     * 5 lanciare il prossimo processo (ldst)
     */

    tod = 0;
    temp = 0;
    tod = getTODLO();
    current->global_time += tod - current->elapsed_time;

    if(current->state != WAIT){
      current->state = READY;
      switch(current->prio){
      case PRIO_LOW:
	insertProcQ(&p_low, current);
	break;
      case PRIO_NORM:
	insertProcQ(&p_norm, current);
	break;
      case PRIO_HIGH:
	insertProcQ(&p_high, current);
	break;
      }
    }

    if( (current = headProcQ(&p_high))
	!= NULL && (current->state != WAIT)){
      outProcQ(&p_high, current);
      current->elapsed_time = tod;
      current->state = RUNNING;
    }
    else{
      if( (current = headProcQ(&p_norm))
	  != NULL && (current->state != WAIT)){
	outProcQ(&p_norm, current);
	current->elapsed_time = tod;
	current->state = RUNNING;
      }
      else{
	if( (current = headProcQ(&p_low))
	    != NULL && (current->state != WAIT)){
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
	    if(pc_count > 0 && sb_count == 0)
	      PANIC();
	    /* idle */
	    else{
	      current = headProcQ(&p_idle);
	      current->elapsed_time = tod;
	      current->state = RUNNING;
	    }
	  }
	}
      }
    }

    setTIMER(SCHED_TIME_SLICE);
    LDST(&current->p_s)
  }
}
