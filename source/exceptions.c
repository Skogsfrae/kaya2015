#include <pcb.h>
#include <asl.h>
#include <time.h>
#include <listx.h>
#include <const.h>
#include <scheduler.h>

/* Soluzione adottata nella versione 0.01 di linux
static pid_t lastpid = -1;  -1 no process executing */
unsigned int pid_bitmap = 0;
unsigned int free_pidmap = MAXPROC;
pid_t last_pid = 0, last_freed_pid = 0;
pcb_t *pidmap[MAXPROC];

/* Gets pid number from pid mask */
int get_pid_num(int pidmask){
  int pid;

  if(pidmask == 0)
    return 0;

  while(pidmask > 0){
    if(pidmask&1 == 0)
      pid++;
    pidmask >>=1;
  }

  return pid++;
}

/* Gets pid mask from pid number */
int get_pid_mask(int pid){
  int mask = 1;

  return mask <<= pid;
}

/* Bisogna aggiungere la priorita' */
int SYSCALL(CREATEPROCESS, state_t *statep, priority_enum *prio)
{
	pcb_t *newp;
	unsigned int temp_bitmap, i = 1;

	/* Controllo risorse/priorità */
	if( ((newp = allocPcb()) == NULL) && (*prio == PRIO_IDLE) )
		/* No pcb available */
		return -1;

	/* Copy statep */
	newp->p_s.a1 = statep->a1;
	newp->p_s.a2 = statep->a2;
	newp->p_s.a3 = statep->a3;
	newp->p_s.a4 = statep->a4;
	newp->p_s.v1 = statep->v1;
	newp->p_s.v2 = statep->v2;
	newp->p_s.v3 = statep->v3;
	newp->p_s.v4 = statep->v4;
	newp->p_s.v5 = statep->v5;
	newp->p_s.v6 = statep->v6;
	newp->p_s.sl = statep->sl;
	newp->p_s.fp = statep->fp;
	newp->p_s.ip = statep->ip;
	newp->p_s.sp = statep->sp;
	newp->p_s.lr = statep->lr;
	newp->p_s.pc = statep->ps;
	newp->p_s.cpsr = statep->cpsr;
	newp->p_s.CP15_Control = statep->CP15_Control;
	newp->p_s.CP15_EntryHi = statep->CP15_EntryHi;
	newp->p_s.CP15_Cause = statep->CP15_Cause;
	newp->p_s.TOD_Hi = statep->TOD_Hi;
	newp->p_s.TOD_Low = statep->TOD_Low;

	/* Set timers */
        gettimeofday(&(newp->start_time));
	reset_timer(&(newp->elapsed_time));
	reset_timer(&(newp->user_time));
	reset_timer(&(newp->global_time));

	/* it means there are no pids used (no running programs) */
	/* if(!pid_bitmap){ */
	/*   newp->pid = 1; */
	/*   pid_bitmap &= 1; */
	/*   last_pid = 1; */
	/*   free_pidmap ^= last_pid; */
	/*   pidmap[last_pid] = newp; */
	/* } */

	/* else{ */
	
	/* Vale sia come caso base (primo programma da eseguire),
	** sia come caso in cui incrementalmente vengono creati processi */
	if(last_pid == last_freed_pid){
	  tmp_bitmap = pid_bitmap;
	  pid_bitmap <<= 1;
	  pid_bitmap ^= 1;
	  tmp_bitmap = pid_bitmap ^ tmp_bitmap; /* extracting new pid */
	  free_pidmap ^= tmp_bitmap;
	  last_pid = tmp_bitmap;
	  newp->pid = get_pid_num(tmp_bitmap);
	  pidmap[newp->pid - 1] = newp;
	}

	/* Riciclo pid */
	else{
	  newp->pid = get_pid_num(last_freed_pid);
	  last_pid = last_freed_pid;
	  free_pidmap ^= last_freed_pid;
	  pid_bitmap ^= last_pid;
	  pidmap[newp->pid] = newp;
	  tmp_bitmap = pid_bitmap ^ (pid_bitmap >> 1); /* limito la ricerca*/
	  while(i != tmp_bitmap){                      /* del nuovo freepid*/
	    if(i & free_pidmap)
	      break;
	    else
	      i <<= 1;
	  }
	  last_freed_pid = i;
	}
	/* } */
	
	
	/* lastpid++;
	   newp->pid = lastpid; */

	/* Adding to proper queue */
	switch(*prio){
	case PRIO_LOW:
	  list_add(newp->p_list, p_low);
	  break;
	case PRIO_NORM:
	  list_add(newp->p_list, p_norm);
	  break;
	case PRIO_HIGH:
	  list_add(newp->p_list, p_high);
	  break;
	}

	newp->sem_wait = 0;
	newp->state = READY;
	
	pc_count++;
	current->p_s.a1 = newp->pid;
	return newp->pid;
}

pcb_t *terminate_children(pcb_t *parent){
  int pidmask = get_pid_mask(parent->pid);
  while(emptyChild(parent))
    freePcb(terminate_children(parent->p_children));
  
  if(pidmask == last_pid)
    last_pid = 0;
  pid_bitmap ^= pidmask;
  free_pidmap ^= pidmask;
  last_freed_pid = pidmask;
  pidmap[parent->pid] = NULL;

  /* Aggiusto il valore del semaforo su cui è in attesa facendo una V */
  if(parent->sem_wait > 0 && *(parent->p_cursem->semaddr) < 0){
    *parent->p_cursem->semaddr += parent->sem_wait;
    outBlocked(parent);
    sb_count--;
  }

  pc_count--;
  return outChild(parent);
}

/* Bisogna valutare il caso che il processo sia in una coda di un semaforo */
void SYSCALL(TEMINATEPROCESS, pid_t pid){
  pcb_t *parent, *child;
  int pidmask = get_pid_mask(pid);

  parent = pidmap[pid];
  outChild(parent);

  /* Se ci sono dei figli killali */
  if(!emptyChild(parent))
    freePcb(terminate_children(parent));
  
  if(pidmask == last_pid)
    last_pid = 0;
  pid_bitmap ^= pidmask;
  free_pidmap ^= pidmask;
  last_freed_pid = pidmask;

  /* Aggiusto il valore del semaforo */
  if(parent->sem_wait > 0 && *(parent->p_cursem->semaddr) < 0){
    *parent->p_cursem->semaddr += parent->sem_wait;
    outBlocked(parent);
    sb_count--;
  }
  
  freePcb(parent);
  pc_count--;
  pidmap[pid] = NULL;
}

void SYSCALL(VERHOGEN, int *semaddr, int weight){
  pcb_t *tmp;
  *semaddr += weight;
  if((tmp = headBlocked(semaddr)) == NULL)
    return;
  if(tmp->sem_wait >= *semaddr){
    tmp->state = READY;
    tmp->sem_wait = 0;
    outBlocked(tmp);
    sb_count--;
  }
}

void SYSCALL(PASSEREN, int *semaddr, int weight){
  *semaddr -= weight;
  insertBlocked(semaddr, current);
  current->state = WAIT;
  current->sem_wait = weight;
  sb_count++;
  scheduler();
}

void SYSCALL(GETCPUTIME, cputime_t *global, cputime_t *user){
  *global = current->global_time;
  *user = current->user_time;
}

pid_t SYSCALL(GETPID){
  current->p_s.a1 = current->pid;
  return current->pid;
}

pid_t SYSCALL(GETPPID){
  current->p_s.a1 = current->p_parent->pid;
  return current->p_parent->pid;
}
