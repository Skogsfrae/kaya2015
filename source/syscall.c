#include <pcb.h>
#include <asl.h>
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
static int get_pid_num(int pidmask){
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
static int get_pid_mask(int pid){
  int mask = 1;

  return mask <<= pid;
}

/* Bisogna aggiungere la priorita' */
int create_process(state_t *statep, priority_enum *prio)
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
  newp->start_time = getTODLO();
  newp->elapsed_time = 0;
  newp->kernel_time = 0;
  newp->user_time = 0;
  newp->global_time = 0;

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

  for(i=0; i<6; i++)
    newp->excvector[i] = NULL;
  new->bool_excvector = FALSE;

  newp->sem_wait = 0;
  newp->state = READY;
	
  pc_count++;
  return newp->pid;
}

static pcb_t *terminate_children(pcb_t *parent){
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
    verhogen(parent->p_cursem->semaddr, parent->sem_wait)
    sb_count--;
  }

  pc_count--;
  return outChild(parent);
}

/* Bisogna valutare il caso che il processo sia in una coda di un semaforo */
void terminate_process(pid_t pid){
  pcb_t *parent, *child;
  int pidmask = get_pid_mask(pid);

  parent = pidmap[pid];
  outChild(parent);

  /* Genocidio */
  if(!emptyChild(parent))
    freePcb(terminate_children(parent));
  
  if(pidmask == last_pid)
    last_pid = 0;
  pid_bitmap ^= pidmask;
  free_pidmap ^= pidmask;
  last_freed_pid = pidmask;

  /* Aggiusto il valore del semaforo */
  if(parent->sem_wait > 0 && *(parent->p_cursem->semaddr) < 0){
    verhogen(parent->p_cursem->semaddr, parent->sem_wait);
    sb_count--;
  }
  
  freePcb(parent);
  pc_count--;
  pidmap[pid] = NULL;
}

void verhogen(int *semaddr, int weight){
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

void passeren(int *semaddr, int weight){
  *semaddr -= weight;
  insertBlocked(semaddr, current);
  current->state = WAIT;
  current->sem_wait = weight;
  sb_count++;
  scheduler();
}

void specify_exception_state_vector(state_t **state_vector){
  int i;

  if(current->excvector[0] != NULL)
    terminate_process(current->pid);

  for(i=0; i<6; i++){
    current->excvector[i] = state_vector[i];
    /* switch(i){ */
    /* case 0: */
    /*   state_vector[i] = TLB_OLDAREA; */
    /*   break; */
    /* case 1: */
    /*   state_vector[i] = TLB_NEWAREA; */
    /*   break; */
    /* case 2: */
    /*   state_vector[i] = SYSBK_OLDAREA; */
    /*   break; */
    /* case 3: */
    /*   state_vector[i] = SYSBK_NEWAREA; */
    /*   break; */
    /* case 4: */
    /*   state_vector[i] = PGMTRAP_OLDAREA; */
    /*   break; */
    /* case 5: */
    /*   state_vector[i] = PGMTRAP_NEWAREA; */
    /*   break; */
    /* } */
  }

  current->bool_excvector = TRUE;
}

void get_cpu_time(cputime_t *global, cputime_t *user){
  *global = current->global_time;
  *user = current->global_time - current->kernel_time;
}

void wait_for_clock(void){
  passeren(dev_sem[CLOCK_SEM]->semAdd, 1);
}

unsigned int wait_for_io(int intlNo, int dnum, int waitFotTermRead){
  passeren(dev_sem[dnum]->semAdd, 1);
  
}

pid_t get_pid(void){
  return current->pid;
}

pid_t get_ppid(void){
  return current->p_parent->pid;
}
