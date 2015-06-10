#include <pcb.h>
#include <const.h>
#include <types.h>
#include <libuarm.h>
#include <scheduler.h>

void scheduler(void){
  struct time_t temp, tick, tod;

  while(1){
    /* 
     * 1 aggiornare il timer di current
     * 2 modificare current->state
     * 3 selezionare il prossimo
     * 4 aggiornare il cputimer
     * 5 lanciare il prossimo processo (ldst)
     */

    reset_timer(&tod);
    reset_timer(&temp);
    gettimeofday(&tod);
    timer_sub(&tod, &current->elapsed_time, &tick);
    timer_add(&current->global_time, &tick, &temp);
    timecpy(&current->global_time, &temp);
    timer_add(&current->user_time, &tick, &temp);
    timecpy(&current->user_time, &temp);
    timecpy(&current->elapsed_time, &tod);

    current->state = READY;

    if( (current = headProcQ(&p_high))
	!= NULL && (current->state != WAIT)){
      outProcQ(&p_high, current);
      insertProcQ(&p_high, current);
      timecpy(&current->elapsed_time, &tod);
      current->state = RUNNING;
    }
    else{
      if( (current = headProcQ(&p_norm))
	  != NULL && (current->state != WAIT)){
	outProcQ(&p_norm, current);
	insertProcQ(&p_norm, current);
	timecpy(&current->elapsed_time, &tod);
	current->state = RUNNING;
      }
      else{
	if( (current = headProcQ(&p_low))
	    != NULL && (current->state != WAIT)){
	  outProcQ(&p_low, current);
	  insertProcQ(&p_low, current);
	  timecpy(&current->elapsed_time, &tod);
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
	      timecpy(&current->elapsed_time, &tod);
	      current->state = RUNNING;
	    }
	  }
	}
      }
    }

    setTIMER(TICK_TIME);
    LDST(&current->p_s)
  }
}
