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


/* Soluzione adottata nella versione 0.01 di linux     */
/* static pid_t lastpid = -1;  -1 no process executing */
unsigned int pid_bitmap = 0;
unsigned int free_pidmap = 0xFFFFF;
pid_t last_pid = 0, last_freed_pid = 0;
pcb_t *pidmap[MAXPROC];

/* Bisogna aggiungere la priorita' */
int create_process(state_t *statep, priority_enum prio)
{
  pcb_t *newp;
  unsigned int tmp_bitmap, i = 1;

  /* Controllo risorse/priorità */
  if( ((newp = allocPcb()) == NULL) || (prio == PRIO_IDLE) )
    /* No pcb available */
    return -1;

  /* Copy statep */
  copy_state(&newp->p_s, statep);

  /* Set timers */
  newp->start_time = getTODLO();
  newp->elapsed_time = 0;
  newp->kernel_time = 0;
  newp->global_time = 0;
	
  /* Vale sia come caso base (primo programma da eseguire),        */
  /* sia come caso in cui incrementalmente vengono creati processi */
  if(last_pid == last_freed_pid){
    tmp_bitmap = pid_bitmap;
    pid_bitmap <<= 1;
    pid_bitmap ^= 1;
    tmp_bitmap = pid_bitmap ^ tmp_bitmap; /* extracting new pid */
    free_pidmap ^= tmp_bitmap;
    last_pid = tmp_bitmap;
    newp->pid = get_bit_num(tmp_bitmap);
    pidmap[newp->pid - 1] = newp;
  }

  /* Riciclo pid */
  else{
    newp->pid = get_bit_num(last_freed_pid);
    last_pid = last_freed_pid;
    free_pidmap ^= last_freed_pid;
    pid_bitmap ^= last_pid;
    pidmap[newp->pid - 1] = newp;
    tmp_bitmap = pid_bitmap ^ (pid_bitmap >> 1); /* limito la ricerca*/
    while(i != tmp_bitmap){                      /* del nuovo freepid*/
      if(i & free_pidmap)
	break;
      else
	i <<= 1;
    }
    /* gestire il caso che non sia trovato */
    last_freed_pid = i;
  }

  /* Adding to proper queue */
  switch(prio){
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

  /* for(i=0; i<6; i++) */
  /*   newp->excvector[i] = NULL; */
  newp->bool_excvector = FALSE;

  newp->sem_wait = 0;
  newp->state = READY;
	
  pc_count++;
  return newp->pid;
}

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

  /* Aggiusto il valore del semaforo su cui è in attesa facendo una V */
  if(parent->sem_wait > 0 && *(parent->p_cursem->s_semAdd) < 0){
    verhogen(parent->p_cursem->s_semAdd, parent->sem_wait);
    sb_count--;
  }

  pc_count--;
  return outChild(parent);
}

/* Bisogna valutare il caso che il processo sia in una coda di un semaforo */
void terminate_process(pid_t pid)
{
  struct pcb_t *parent, *child;
  int pidmask = get_bit_mask(pid);
  
  parent = pidmap[pid - 1];
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
  if(parent->sem_wait > 0 && *(parent->p_cursem->s_semAdd) < 0){
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
  if((tmp = headBlocked(semaddr)) != NULL){
    if(tmp->sem_wait >= *semaddr){
      tmp->sem_wait = 0;
      outBlocked(tmp);
      tmp->state = READY;
      /* Adding to proper queue */
      switch(tmp->prio){
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
  if((*semaddr -= weight) < 0){
    /* Non ci sono semafori liberi */
    if((insertBlocked(semaddr, current)) == TRUE){
      PANIC();
    }
    current->state = WAITING;
    current->sem_wait = weight;
    sb_count++;
    /* Tolta la chiamata allo scheduler. Concettualmente è sbagliato */
    /* farlo chiamare da una system call                             */
    //scheduler();
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

  /* Passa al terminale di lettura */
  if(waitForTermRead)
    read = DEV_PER_INT;
  passeren(&dev_sem[EXT_IL_INDEX(intlNo)*DEV_PER_INT + read + dnum], 1);

  /* La status word viene presa dalla matrice dichiarata */
  /* nel file interrupts.c                               */
  /* if(intlNo == INT_TERMINAL){ */
  /*   if(waitForTermRead) */
  /*     stat_word = status_word[(INT_TERMINAL-2)*DEV_PER_INT + dnum]; */
  /*   else */
  /*     stat_word = status_word[EXT_IL_INDEX(INT_TERMINAL)*DEV_PER_INT + dnum]; */
  /* } */
  /* else{ */
  /*   stat_word = status_word[EXT_IL_INDEX(intlNo)*DEV_PER_INT + dnum]; */
  /* } */

  stat_word = status_word[EXT_IL_INDEX(intlNo)*DEV_PER_INT + read + dnum];

  return stat_word;
}

pid_t get_pid(void)
{
  return current->pid;
}

pid_t get_ppid(void)
{
  return current->p_parent->pid;
}
