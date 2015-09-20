#include <types.h>
#include <uARMconst.h>
#include <listx.h>

static struct list_head pcbFree=LIST_HEAD_INIT(pcbFree);
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

  if(list_empty(&pcbFree))
    return NULL;
  else
    {
      tmp = container_of(pcbFree.next, typeof(*tmp), p_list);
      list_del(&tmp->p_list);

      INIT_LIST_HEAD(&tmp->p_list);
      INIT_LIST_HEAD(&tmp->p_children);
      INIT_LIST_HEAD(&tmp->p_siblings);
      tmp->p_parent = NULL;
      tmp->p_cursem = NULL;
		
      return tmp;
    }
}

void freePcb(pcb_t *p)
{
  list_add(&p->p_list, &pcbFree);
}

void insertProcQ(struct list_head *q, pcb_t *p)
{
  list_add_tail(&p->p_list, q);
}

pcb_t *removeProcQ(struct list_head *q)
{
  pcb_t *tmp;
	
  if(list_empty(q))
    return NULL;
		
  else
    {
      tmp = container_of(q->next, typeof(*tmp), p_list);
      list_del(&tmp->p_list);
      return tmp;
    }
}

pcb_t *outProcQ(struct list_head *q, pcb_t *p)
{
  pcb_t *tmp;

  if(list_empty(q))
    return NULL;

  list_for_each_entry(tmp, q, p_list)
    if(tmp == p)
      {
	list_del(&tmp->p_list);
	return p;
      }

  return NULL;
}

pcb_t *headProcQ(struct list_head *q)
{
  pcb_t *tmp;

  if(list_empty(q))
    return NULL;
  else
    {
      tmp = container_of(q->next, typeof(*tmp), p_list);	
      return tmp;
    }
}

int emptyChild(struct pcb_t *p)
{
  return list_empty(&p->p_children);
}

void insertChild(struct pcb_t *parent, struct pcb_t *p)
{
  list_add(&p->p_siblings, &parent->p_children);
  p->p_parent = parent;
}

struct pcb_t *removeChild(struct pcb_t *p)
{
  struct pcb_t *tmp;

  if (list_empty(&p->p_children))
    return NULL;
  else
    {
      tmp = container_of(p->p_children.next, typeof(*tmp), p_children);
      list_del(p->p_children.next);
      tmp->p_parent = NULL;
      return tmp;
    }
}

struct pcb_t *outChild(struct pcb_t *p)
{
  if(p->p_parent == NULL)
    return NULL;
  else
    {
      p->p_parent = NULL;
      list_del(p->p_siblings.prev->next);
      return p;
    }
}
