#include <types.h>
#include <const.h>
#include <listx.h>

hidden struct list_head pcbFree=LIST_HEAD_INIT(pcbFree);
static pcb_t procp[MAXPROC];

void initPcbs()
{
	int i;
	pcb_t *tmp;

	for(i=0; i<MAXPROC; i++)
	{
		tmp = &procp[i];
		list_add_tail(&tmp->p_list, &pcbFree);
	}
}

pcb_t *allocPcb()
{
	pcb_t *tmp;

	if(list_empty(&pcbFree) == TRUE)
		return NULL;
	else
	{
		tmp = container_of((pcbFree)->next, typeof(tmp), p_list);
		list_del(tmp->p_list);

		tmp->p_list = NULL;
		tmp->p_children = NULL;
		tmp->psiblings = NULL;
		tmp->p_parent = NULL;
		tmp->p_cursem = NULL;
		tmp->p_s = 0;
		
		return tmp;
	}
}

void freePcb(pcb_t *p)
{
	list_add_tail(&p->p_list, &pcbFree);
}

void insertProcQ(struct list_head *q, pcb_t *p)
{
	list_add_tail(&p->p_list, q);
}

pcb_t *removeProcQ(struct list_head *q)
{
	if(list_empty(q) == TRUE)
		return NULL;
	
	else
	{
		pcb_t *tmp;

		tmp = container_of(q->next, typeof(tmp), p_list);
		list_del(tmp->p_list);
		
		return tmp;
	}
}

pcb_t *outProcQ(struct list_head *q, pcb_t *p)
{
	pcb_t *tmp;

	list_for_each_entry(tmp, q, p_list)
		if(tmp == p)
		{
			list_del(tmp->p_list);
			return p;
		}

	return NULL;
}

pcb_t *headProcQ(struct list_head *q)
{
	if(list_empty(q) == TRUE)
		return NULL;
	else
	{
		pcb_t *tmp;

		tmp = container_of(q->next, typeof(tmp), p_list);
		
		return tmp;
	}
}


