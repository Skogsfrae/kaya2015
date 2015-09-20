#include "types.h"
#include "pcb.h"
#include "listx.h"
#include <uARMconst.h>

static struct list_head aslh = LIST_HEAD_INIT(aslh); 
static struct list_head semdFree = LIST_HEAD_INIT(semdFree);
static struct semd_t semdTable[MAXPROC];

void initASL()
{
  int i;
  struct semd_t *tmp;
	
  for(i=0; i<MAXPROC; i++){
      tmp = &semdTable[i];
      list_add_tail(&tmp->s_link, &semdFree);
    }
}

/* Look for the semaphore semAdd
 * returns the right semaphore or
 * the following one */
struct semd_t *lookForSemaphore(int *semAdd)
{
  struct semd_t *tmp;
	
  list_for_each_entry(tmp, &aslh, s_link)
    if(tmp->s_semAdd >= semAdd)
      break;

  return tmp;
}

int insertBlocked(int *semAdd, struct pcb_t *p)
{
  struct semd_t *tmp, *s_tmp;
	
  /* Aslh is empty, so the semaphore is allocated in first position */
  if(list_empty(&aslh)){
      tmp = container_of(semdFree.next, typeof(*tmp), s_link);
      list_del(semdFree.next);
      INIT_LIST_HEAD(&tmp->s_procq);
      tmp->s_semAdd = semAdd;
      p->p_cursem->s_semAdd = semAdd;
      insertProcQ(&tmp->s_procq, p);
      list_add_tail(&tmp->s_link, &aslh);
    }
	
  /* Let's look for the right semaphore */
  else{
      tmp = lookForSemaphore(semAdd);
      if(tmp->s_semAdd == semAdd){
	  p->p_cursem->s_semAdd = semAdd;
	  insertProcQ(&tmp->s_procq, p);
	}
      else{
	  if(list_empty(&semdFree)){
	      return TRUE;
	    }
	  else{
	      /* Alloc a new semaphore */
	      s_tmp = container_of(semdFree.next, typeof(*s_tmp), s_link);
	      list_del(semdFree.next);
	      INIT_LIST_HEAD(&s_tmp->s_procq);
	      s_tmp->s_semAdd = semAdd;
	      p->p_cursem->s_semAdd = semAdd;
	      insertProcQ(&s_tmp->s_procq, p);
	      list_add_tail(&s_tmp->s_link, &tmp->s_link);
	    }
	}

    }
  return FALSE;
}

struct pcb_t *removeBlocked(int *semAdd)
{
  struct semd_t *tmp;
  struct pcb_t *p_tmp;
	
  tmp = lookForSemaphore(semAdd);
  if(tmp->s_semAdd == semAdd){
      p_tmp = removeProcQ(&tmp->s_procq);
      if(list_empty(&tmp->s_procq)){
	  list_del(&tmp->s_link);
	  list_add(&tmp->s_link, &semdFree);
	}
		
      return p_tmp;
    }
  else
    return NULL;
}

struct pcb_t *outBlocked(struct pcb_t *p)
{
  struct semd_t *tmp;
  struct pcb_t *p_tmp;
	
  tmp = lookForSemaphore(p->p_cursem->s_semAdd);
  if(tmp->s_semAdd != p->p_cursem->s_semAdd)
    return NULL;

  return outProcQ(&tmp->s_procq, p);
}

struct pcb_t *headBlocked(int *semAdd)
{
  struct semd_t *tmp;
  struct pcb_t *ptmp;
	
  tmp = lookForSemaphore(semAdd);
  if(tmp->s_semAdd != semAdd)
    return NULL;

  ptmp = headProcQ(&tmp->s_procq);
  if(list_empty(&tmp->s_procq)){
      list_del(&tmp->s_link);
      list_add(&tmp->s_link, &semdFree);
    }
	
  return ptmp;
}
