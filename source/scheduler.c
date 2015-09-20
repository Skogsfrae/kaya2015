#include <pcb.h>
#include <asl.h>
#include <const.h>
#include <types.h>
#include <syscall.h>
#include <libuarm.h>
#include <initial.h>
#include <uARMtypes.h>
#include <uARMconst.h>

cputime_t tick, tod;
int clock_ticks = 0;

void scheduler(void)
{
  /* Unlock every process locked in the clock semaphore */
  if((clock_ticks++) == 20)
    {
      while(headBlocked(&dev_sem[CLOCK_SEM]) != NULL)
	verhogen(&dev_sem[CLOCK_SEM], 1);
      clock_ticks = 0;
    }

  /* If a process ran out of time, then put it in the proper queue */
  if(current != NULL)
    {    
      tod = getTODLO();
      current->global_time += tod - current->elapsed_time;
      
      if(current->state != WAITING)
	{
	  current->state = READY;
	
	  switch(current->prio)
	    {
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
    }

  /* Take the right process from its queue */
  if( (current = headProcQ(&p_high)) != NULL && (current->state != WAITING))
    {
      outProcQ(&p_high, current);
      current->elapsed_time = tod;
      current->state = RUNNING;
    }
  else
    {
      if( (current = headProcQ(&p_norm)) != NULL && (current->state != WAITING))
	{
	  outProcQ(&p_norm, current);
	  current->elapsed_time = tod;
	  current->state = RUNNING;
	}
      else
	{
	  if( (current = headProcQ(&p_low)) != NULL && (current->state != WAITING))
	    {
	      outProcQ(&p_low, current);
	      current->elapsed_time = tod;
	      current->state = RUNNING;
	    }
	  else
	    {
	      /* shut down */
	      if(pc_count == 0)
		{
		  HALT();
		}
	      else
		{
		  /* deadlock */
		  if(pc_count > 0 && sb_count == 0)
		    {
		      PANIC();
		    }
		  /* idle */
		  else
		    {
		      current = headProcQ(&p_idle);
		      current->elapsed_time = tod;
		      current->state = RUNNING;
		    }
		}
	    }
	}
    }

  setTIMER(SCHED_TIME_SLICE);
  LDST(&(current->p_s));
}
