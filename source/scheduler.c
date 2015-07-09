#include <pcb.h>
#include <asl.h>
#include <const.h>
#include <types.h>
#include <syscall.h>
#include <libuarm.h>
#include <initial.h>
#include <uARMtypes.h>
#include <uARMconst.h>

//#define DEBUG

cputime_t tick, tod;
int clock_ticks = 0;

void scheduler(void){
  // while(1){
    /* 
     * 1 aggiornare il timer di current
     * 2 modificare current->state
     * 3 selezionare il prossimo
     * 4 aggiornare il cputimer
     * 5 lanciare il prossimo processo (ldst)
     */
    if((clock_ticks++) == 20){
      while(headBlocked(&dev_sem[CLOCK_SEM]) != NULL)
	verhogen(&dev_sem[CLOCK_SEM], 1);
      clock_ticks = 0;
    }

    if(current != NULL){    
      tod = getTODLO();
      current->global_time += tod - current->elapsed_time;
#ifdef DEBUG
      tprint("Scheduler: metto il processo in coda\n");
#endif
      if(current->state != WAITING){
	current->state = READY;
	switch(current->prio){
	case PRIO_LOW:
#ifdef DEBUG
	  tprint("Scheduler: prio_low\n");
#endif
	  insertProcQ(&p_low, current);
	  break;
	case PRIO_NORM:
#ifdef DEBUG
	  tprint("Scheduler: prio_norm\n");
#endif
	  insertProcQ(&p_norm, current);
	  break;
	case PRIO_HIGH:
#ifdef DEBUG
	  tprint("Scheduler: prio_high\n");
#endif
	  insertProcQ(&p_high, current);
	  break;
	}
      }
    }

#ifdef DEBUG
    tprint("Scheduler: seleziono il processo dalla coda\n");
#endif
    if( (current = headProcQ(&p_high))
	!= NULL && (current->state != WAITING)){
#ifdef DEBUG
      tprint("Scheduler: coda p_high\n");
#endif
      outProcQ(&p_high, current);
      current->elapsed_time = tod;
      current->state = RUNNING;
    }
    else{
      if( (current = headProcQ(&p_norm))
	  != NULL && (current->state != WAITING)){
#ifdef DEBUG
	tprint("Scheduler: coda p_norm\n");
#endif
	outProcQ(&p_norm, current);
	current->elapsed_time = tod;
	current->state = RUNNING;
      }
      else{
	if( (current = headProcQ(&p_low))
	    != NULL && (current->state != WAITING)){
#ifdef DEBUG
	  tprint("Scheduler: coda p_low\n");
#endif
	  outProcQ(&p_low, current);
	  current->elapsed_time = tod;
	  current->state = RUNNING;
	}
	else{
	  /* shut down */
	  if(pc_count == 0){
	    //#ifdef DEBUG
	    tprint("Scheduler: shutting down\n");
	    //#endif
	    HALT();
	  }
	  else{
	    /* deadlock */
	    if(pc_count > 0 && sb_count == 0){
	      //#ifdef DEBUG
	      tprint("Scheduler: deadlock\n");
	      //#endif
	      PANIC();
	    }
	    /* idle */
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

#ifdef DEBUG
    tprint("Scheduler: setting timer\n");
#endif

    setTIMER(SCHED_TIME_SLICE);
    LDST(&(current->p_s));
    //}
}
