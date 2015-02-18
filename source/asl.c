#include "types.h"
#include "asl.h"
#include "pcb.h"
#include "libuarm.h"
#include "listx.h"
#include <uARMconst.h>

typedef struct semd_t{
	int *s_semAdd;
	struct list_head s_link;
	struct list_head s_procq;
}semd_t;

static struct list_head aslh = LIST_HEAD_INIT(aslh); 
static struct list_head semdFree = LIST_HEAD_INIT(semdFree);
static struct semd_t semdTable[MAXPROC];

void initASL()
{
	int i;
	struct semd_t *tmp;
	
	for(i=0; i<MAXPROC; i++)
	{
		tmp = &semdTable[i];
		list_add_tail(&tmp->s_link, &semdFree);
	}
}

struct semd_t *lookForSemaphore(int *semAdd)
{
	struct semd_t *tmp;
	
	list_for_each_entry(tmp, &aslh, s_link)
		if(tmp->s_semAdd == semAdd)
			break;
			
	if(tmp->s_semAdd != semAdd)
		return NULL;
	else
		return tmp;
}

//int insertBlocked(int *semAdd, struct pcb_t *p)
//{
	//struct semd_t *tmp;
	//struct semd_t *s_tmp;
	
	//if(list_empty(&aslh))
	//{
		//tprint("Aslh vuota\n");
		//s_tmp = container_of(semdFree.next, typeof(*s_tmp), s_link);
		//list_del(&s_tmp->s_link);
		
		//s_tmp->s_semAdd = semAdd;
		
		//list_add(&s_tmp->s_link, &aslh);
		
		//INIT_LIST_HEAD(&s_tmp->s_procq);
		
		//list_add_tail(&p->p_list, &s_tmp->s_procq);
		//p->p_cursem->s_semAdd = semAdd;
		
		//return FALSE;
	//}
	
	//list_for_each_entry(tmp, &aslh, s_link)
		//if(tmp->s_semAdd == semAdd)
		//{
			//tprint("Trovato\n");
			//list_add_tail(&p->p_list, &tmp->s_procq);
			//p->p_cursem->s_semAdd = semAdd;
			//return FALSE;
		//}		
		//else
		//{
			//tprint("Sono nell'else\n");
			//if(tmp->s_semAdd > semAdd)
			//{
				//if(list_empty(&semdFree))
					//return TRUE;
				//else
				//{
					//s_tmp = container_of(semdFree.next, typeof(*s_tmp), s_link);
					//list_del(&s_tmp->s_link);
					
					//s_tmp->s_semAdd = semAdd;
					
					//list_add(&s_tmp->s_link, tmp->s_link.prev);
		
					//INIT_LIST_HEAD(&s_tmp->s_procq);
					
					//list_add_tail(&p->p_list, &s_tmp->s_procq);
					//p->p_cursem->s_semAdd = semAdd;
					
					//return FALSE;
				//}	
			//}
		//}
//}

int insertBlocked(int *semAdd, struct pcb_t *p)
{
	struct semd_t *tmp, *s_tmp;
	
	/* Aslh is empty, so the semaphore is allocated in first position */
	if(list_empty(&aslh))
	{
//		tprint("Aggiunto il primo semaforo\n");
		tmp = container_of(semdFree.next, typeof(*tmp), s_link);
		list_del(semdFree.next);
		INIT_LIST_HEAD(&tmp->s_procq);
		tmp->s_semAdd = semAdd;
		p->p_cursem->s_semAdd = semAdd;
		insertProcQ(&tmp->s_procq, p);
		//list_add_tail(&p->p_list, &tmp->s_procq);
		list_add_tail(&tmp->s_link, &aslh);
		return FALSE;
	}
	
	/* Let's look for the right semaphore */
	else
	{
		/* Se il cursore della lista supera il valore cercato esce, altrimenti continua a girare,
		 * nonostante ciò, s*/
		list_for_each_entry(tmp, &aslh, s_link)
		{	/* Trovato */
//			tprint("Cerco\n");
			if(tmp->s_semAdd == semAdd)
			{
//				tprint("Trovato\n");
				p->p_cursem->s_semAdd = semAdd;
				insertProcQ(&tmp->s_procq, p);
				//list_add_tail(&p->p_list, &tmp->s_procq);
				return FALSE;
			}
			else
			{
//				tprint("Non trovato\n");
				/* Caso semaforo corrente maggiore di quello cercato, esci */
				if(tmp->s_semAdd > semAdd)
				{
					break;
////					tprint("Ci sei?\n");
					///* semdFree vuota */
					
				}
					//if(list_empty(&semdFree))
					//{
////						tprint("Return true\n");
						//return TRUE;
					//}
					//else
					//{
						///* Alloca nuovo semaforo */
						//s_tmp = container_of(semdFree.next, typeof(*s_tmp), s_link);
						//list_del(semdFree.next);
						//INIT_LIST_HEAD(&s_tmp->s_procq);
						//s_tmp->s_semAdd = semAdd;
						//p->p_cursem->s_semAdd = semAdd;
						//insertProcQ(&s_tmp->s_procq, p);
						////list_add_tail(&p->p_list, &s_tmp->s_procq);
						//list_add_tail(&s_tmp->s_link, &tmp->s_link);
////						tprint("Return false\n");
						//return FALSE;
					//}

				/* Caso semAdd maggiore (Dovrebbe essere sbagliato) */
				//if(tmp->s_semAdd < semAdd)
				//{
//					tprint("Ci sei?\n");
					/* semdFree vuota */
					//if(list_empty(&semdFree))
					//{
////						tprint("Return true\n");
						//return TRUE;
					//}
					//else
					//{
						///* Alloca nuovo semaforo */
						//s_tmp = container_of(semdFree.next, typeof(*s_tmp), s_link);
						//list_del(semdFree.next);
						//INIT_LIST_HEAD(&s_tmp->s_procq);
						//s_tmp->s_semAdd = semAdd;
						//p->p_cursem->s_semAdd = s_tmp->s_semAdd;
						//list_add_tail(&p->p_list, &s_tmp->s_procq);
						//list_add(&s_tmp->s_link, tmp->s_link.prev);
////						tprint("Return false\n");
						//return FALSE;
					//}
					
				//}

			}
		}
		if(list_empty(&semdFree))
		{
//			tprint("Return true\n");
			return TRUE;
		}
		else
		{
			/* Alloca nuovo semaforo */
			s_tmp = container_of(semdFree.next, typeof(*s_tmp), s_link);
			list_del(semdFree.next);
			INIT_LIST_HEAD(&s_tmp->s_procq);
			s_tmp->s_semAdd = semAdd;
			p->p_cursem->s_semAdd = semAdd;
			insertProcQ(&s_tmp->s_procq, p);
			//list_add_tail(&p->p_list, &s_tmp->s_procq);
			list_add_tail(&s_tmp->s_link, &tmp->s_link);
//			tprint("Return false\n");
			return FALSE;
		}

	}
	tprint("Che problemi hai?\n");
}

struct pcb_t *removeBlocked(int *semAdd)
{
	struct semd_t *tmp;
	struct pcb_t *p_tmp;
	
	list_for_each_entry(tmp, &aslh, s_link)
	{
		if(tmp->s_semAdd == semAdd)
		{
			//tprint("Semaforo trovato\n");
			p_tmp = removeProcQ(&tmp->s_procq);
			if(list_empty(&tmp->s_procq))
			{
				//tprint("Rimuovo il semaforo\n");
				list_del(&tmp->s_link);
				list_add(&tmp->s_link, &semdFree);
			}
			
			return p_tmp;
		}
		else
		{
			//tprint("Semaforo non trovato\n");
			if(tmp->s_semAdd > semAdd)
				return NULL;
		}
	}
	
	return FALSE;
	tprint("E questo è...?\n");
}

struct pcb_t *outBlocked(struct pcb_t *p)
{
	struct semd_t *tmp;
	struct pcb_t *p_tmp;
	
	tmp = lookForSemaphore(p->p_cursem->s_semAdd);
	if(tmp == NULL)
	{
		tprint("Semaforo non trovato, errore!\n");
		return NULL;
	}

	
	//list_for_each_entry(tmp, &aslh, s_link)
		//if(tmp->s_semAdd == p->p_cursem->s_semAdd)
			//break;
			
	//if(tmp->s_semAdd != p->p_cursem->s_semAdd)
	//{
		//tprint("Semaforo non trovato, errore!\n");
		//return NULL;
	//}

	return outProcQ(&tmp->s_procq, p);
}

struct pcb_t *headBlocked(int *semAdd)
{
	struct semd_t *tmp;
	
	tmp = lookForSemaphore(semAdd);
	
	return removeProcQ(&tmp->s_procq);
}
