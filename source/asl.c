#include "types.h"
#include "asl.h"
#include "pcb.h"
#include <uARMconst.h>

typedef struct semd_t{
	int *s_semAdd;
	struct list_head s_link;
	struct list_head s_procq;
}semd_t;

static struct list_head aslh = LIST_HEAD_INIT(aslh); 
static struct list_head semdFree = LIST_HEAD_INIT(semdFree);

void initASL()
{
	int i;
	static struct semd_t semdTable[MAXPROC];
	
	for(i=0; i<MAXPROC; i++)
		list_add(&semdTable[i].s_link, &semdFree);
}

int insertBlocked(int *semAdd, struct pcb_t *p)
{
	struct semd_t *tmp;
	struct semd_t *s_tmp;
	
	if(list_empty(&aslh))
	{
		s_tmp = container_of(semdFree.next, typeof(*s_tmp), s_link);
		list_del(&s_tmp->s_link);
			
		s_tmp->s_semAdd = semAdd;
		
		list_add(&s_tmp->s_link, tmp->s_link.prev);
		
		INIT_LIST_HEAD(&s_tmp->s_procq);
					
		list_add_tail(&p->p_list, &s_tmp->s_procq);
		p->p_cursem->s_semAdd = semAdd;
					
		return FALSE;
	}
	
	list_for_each_entry(tmp, &aslh, s_link)
		if(tmp->s_semAdd == semAdd)
		{
			list_add_tail(&p->p_list, &tmp->s_procq);
			p->p_cursem->s_semAdd = semAdd;
			return FALSE;
		}		
		else
			if(tmp->s_semAdd > semAdd)
			{
				if(list_empty(&semdFree))
					return TRUE;
				else
				{
					s_tmp = container_of(semdFree.next, typeof(*s_tmp), s_link);
					list_del(&s_tmp->s_link);
					
					s_tmp->s_semAdd = semAdd;
					
					list_add(&s_tmp->s_link, tmp->s_link.prev);
		
					INIT_LIST_HEAD(&s_tmp->s_procq);
					
					list_add_tail(&p->p_list, &s_tmp->s_procq);
					p->p_cursem->s_semAdd = semAdd;
					
					return FALSE;
				}	
			}			
}

struct pcb_t *removeBlocked(int *semAdd)
{
	struct semd_t *tmp;
	struct pcb_t *p_tmp;
	
	list_for_each_entry(tmp, &aslh, s_link)
		if(tmp->s_semAdd == semAdd)
		{
			p_tmp = removeProcQ(&tmp->s_procq);
			if(list_empty(&tmp->s_procq))
			{
				list_del(&tmp->s_link);
				list_add(&tmp->s_link, &semdFree);
			}
			
			return p_tmp;
		}		
		else
			if(tmp->s_semAdd > semAdd)
				return NULL;
}
