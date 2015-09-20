#include <pcb.h>
#include <asl.h>
#include <listx.h>
#include <const.h>
#include <bitmap.h>
#include <syscall.h>
#include <initial.h>
#include <scheduler.h>
#include <interrupts.h>
#include <exceptions.h>
#include <uARMtypes.h>

unsigned int pid_bitmap = 0;
unsigned int free_pidmap = 0xFFFFF;
pid_t last_pid = 0, last_freed_pid = 0;
pcb_t *pidmap[MAXPROC];

/* Alloc a pid from pid bitmap and links it to its assigned pcb */
void pid_alloc(pcb_t *newp)
{
  unsigned int tmp_bitmap, i = 1;
  
  /* Both for the first and incremental process allocation */
  if(last_pid == last_freed_pid)
    {
      tmp_bitmap = pid_bitmap;
      pid_bitmap <<= 1;
      pid_bitmap ^= 1;
      tmp_bitmap = pid_bitmap ^ tmp_bitmap; /* extracting new pid */
      free_pidmap ^= tmp_bitmap;
      last_pid = tmp_bitmap;
      newp->pid = get_bit_num(tmp_bitmap);
      pidmap[newp->pid - 1] = newp;
    }

  /* Pid recycling */
  else
    {
      newp->pid = get_bit_num(last_freed_pid);
      last_pid = last_freed_pid;
      free_pidmap ^= last_freed_pid;
      pid_bitmap ^= last_pid;
      pidmap[newp->pid - 1] = newp;
      tmp_bitmap = pid_bitmap ^ (pid_bitmap >> 1); /* bound for the new freepid research */
      while(i != tmp_bitmap)
	{                      /* new freepid research */
	  if(i & free_pidmap)
	    break;
	  else
	    i <<= 1;
	}
      last_freed_pid = i;
    }
}

int create_process(state_t *statep, priority_enum prio)
{
  pcb_t *newp;

  /* Cheque resources availability */
  if( ((newp = allocPcb()) == NULL) || (prio == PRIO_IDLE) )
    return -1;

  /* Copy statep */
  copy_state(&newp->p_s, statep);

  /* Set timers */
  newp->start_time = getTODLO();
  newp->elapsed_time = 0;
  newp->kernel_time = 0;
  newp->global_time = 0;

  pid_alloc(newp);

  /* Adding to proper queue */
  switch(prio)
    {
    case PRIO_LOW:
      insertProcQ(&p_low, newp);
      break;
    case PRIO_NORM:
      insertProcQ(&p_norm, newp);
      break;
    case PRIO_HIGH:
      insertProcQ(&p_high, newp);
      break;
    }

  newp->prio = prio;
  newp->bool_excvector = FALSE;
  newp->sem_wait = 0;
  newp->state = READY;
  pc_count++;
  
  return newp->pid;
}

/* Recursive children process termination */
static pcb_t *terminate_children(struct pcb_t *parent)
{
  int pidmask = get_bit_mask(parent->pid);
  
  while(emptyChild(parent))
    freePcb(terminate_children(headProcQ(&parent->p_children)));
  
  if(pidmask == last_pid)
    last_pid = 0;
  
  pid_bitmap ^= pidmask;
  free_pidmap ^= pidmask;
  last_freed_pid = pidmask;
  pidmap[parent->pid - 1] = NULL;

  /* In case, fixing semaphore value of the waiting process */
  if(parent->sem_wait > 0 && *(parent->p_cursem->s_semAdd) < 0)
    {
      verhogen(parent->p_cursem->s_semAdd, parent->sem_wait);
      sb_count--;
    }

  pc_count--;
  return outChild(parent);
}

void terminate_process(pid_t pid)
{
  struct pcb_t *parent, *child;
  int pidmask = get_bit_mask(pid);
  
  parent = pidmap[pid - 1];
  outChild(parent);

  /* Killing all */
  if(!emptyChild(parent))
    freePcb(terminate_children(parent));
  
  if(pidmask == last_pid)
    last_pid = 0;
  
  pid_bitmap ^= pidmask;
  free_pidmap ^= pidmask;
  last_freed_pid = pidmask;

  /* Fixing semaphore value */
  if(parent->sem_wait > 0 && *(parent->p_cursem->s_semAdd) < 0)
    {
      verhogen(parent->p_cursem->s_semAdd, parent->sem_wait);
      sb_count--;
    }
  
  freePcb(parent);
  pc_count--;
  pidmap[pid - 1] = NULL;
}

void verhogen(int *semaddr, int weight)
{
  pcb_t *tmp;
  *semaddr += weight;
  if((tmp = headBlocked(semaddr)) != NULL)
    {
      if(tmp->sem_wait >= *semaddr)
	{
	  tmp->sem_wait = 0;
	  outBlocked(tmp);
	  tmp->state = READY;
	  /* Adding to proper queue */
	  switch(tmp->prio)
	    {
	    case PRIO_LOW:
	      insertProcQ(&p_low, tmp);
	      break;
	    case PRIO_NORM:
	      insertProcQ(&p_norm, tmp);
	      break;
	    case PRIO_HIGH:
	      insertProcQ(&p_high, tmp);
	      break;
	    }
	  sb_count--;
	}
    }
}

void passeren(int *semaddr, int weight)
{
  if((*semaddr -= weight) < 0)
    {
      /* No free semaphores */
      if((insertBlocked(semaddr, current)) == TRUE)
	{
	  PANIC();
	}
      current->state = WAITING;
      current->sem_wait = weight;
      sb_count++;
    }
}

void specify_exception_state_vector(state_t **state_vector)
{
  int i;

  if(current->bool_excvector == TRUE)
    terminate_process(current->pid);

  for(i=0; i<6; i++)
    copy_state(&current->excvector[i], state_vector[i]);

  current->bool_excvector = TRUE;
}

void get_cpu_time(cputime_t *global, cputime_t *user)
{
  *global = current->global_time;
  *user = current->global_time - current->kernel_time;
}

void wait_for_clock(void)
{
  passeren(&dev_sem[CLOCK_SEM], 1);
}

unsigned int wait_for_io(int intlNo, int dnum, int waitForTermRead)
{
  int read = 0;
  int stat_word;

  /* Switch to reading terminal */
  if(waitForTermRead)
    read = DEV_PER_INT;
  passeren(&dev_sem[EXT_IL_INDEX(intlNo)*DEV_PER_INT + read + dnum], 1);

  if(current->state != WAITING)
    {
      stat_word = status_word[EXT_IL_INDEX(intlNo)*DEV_PER_INT + read + dnum];
    
      return stat_word;
    }
}

pid_t get_pid(void)
{
  return current->pid;
}

pid_t get_ppid(void)
{
  return current->p_parent->pid;
}
