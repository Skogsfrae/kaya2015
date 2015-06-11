#include <pcb.h>
#include <const.h>
#include <types.h>
#include <libuarm.h>
#include <uARMconst.h>
#include <scheduler.h>

void scheduler(void){
  struct cputime_t temp, tick, tod;

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
    /* timer_sub(&tod, &current->elapsed_time, &tick); */
    /* timer_add(&current->global_time, &tick, &temp); */
    /* timecpy(&current->global_time, &temp); */
    /* timer_add(&current->user_time, &tick, &temp); */
    /* timecpy(&current->user_time, &temp); */
    /* timecpy(&current->elapsed_time, &tod); */

    if(current->state != WAIT)
      current->state = READY;

    if( (current = headProcQ(&p_high))
	!= NULL && (current->state != WAIT)){
      outProcQ(&p_high, current);
      insertProcQ(&p_high, current);
      current->elapsed_time = tod;
      current->state = RUNNING;
    }
    else{
      if( (current = headProcQ(&p_norm))
	  != NULL && (current->state != WAIT)){
	outProcQ(&p_norm, current);
	insertProcQ(&p_norm, current);
	current->elapsed_time = tod;
	current->state = RUNNING;
      }
      else{
	if( (current = headProcQ(&p_low))
	    != NULL && (current->state != WAIT)){
	  outProcQ(&p_low, current);
	  insertProcQ(&p_low, current);
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
