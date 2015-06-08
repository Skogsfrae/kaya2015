#include <pcb.h>

/* Soluzione adottata nella versione 0.01 di linux
static pid_t lastpid = -1;  -1 no process executing */
unsigned int pid_bitmap = 0;
unsigned int free_pidmap = MAXPROC;
pid_t last_pid = 0, last_freed_pid = 0;
pcb_t *pidmap[MAXPROC];

int get_pid_num(int pid){
  int count;

  if(pid == 0)
    return 0;

  while(pid > 0){
    if(pid&1 == 0)
      count++;
    pid >>=1;
  }

  return count;
}

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
		/* No pcb avaleable */
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
	return newp->pid;
}

pcb_t *removeChildren(pcb_t *parent){
  int pidmask = get_pid_mask(parent->pid);
  while(emptyChild(parent))
    freePcb(removeChildren(parent->p_children));
  
  if(pidmask == last_pid)
    last_pid = 0;
  pid_bitmap ^= pidmask;
  free_pidmap ^= pidmask;
  last_freed_pid = pidmask;
  pidmap[parent->pid] = NULL;
  
  return outChild(parent);
}

/* Bisogna valutare il caso che il processo sia in una coda di un semaforo */
void SYSCALL(TEMINATEPROCESS, pid_t pid){
  pcb_t *parent, *child;
  int pidmask = get_pid_mask(pid);

  parent = pidmap[pid];
  outChild(parent);
  if(emptyChild(parent)){
    if(pidmask == last_pid)
      last_pid = 0;
    pid_bitmap ^= pidmask;
    free_pidmap ^= pidmask;
    last_freed_pid = pidmask;
    freePcb(parent);
  }
  else{
    if(pidmask == last_pid)
      last_pid = 0;
    pid_bitmap ^= pidmask;
    free_pidmap ^= pidmask;
    last_freed_pid = pidmask;
    freePcb(removeChildren(parent));
  }
  pidmap[pid] = NULL;
}

void SYSCALL(VERHOGEN, int *semaddr, int weight){
  *semaddr += weight;
}

void SYSCALL(PASSEREN, int *semaddr, int weight){
  *semaddr -= weight;
}

