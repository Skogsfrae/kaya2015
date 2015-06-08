#ifndef TYPES_H
#define TYPES_H

#include <listx.h>
#include <uARMtypes.h>
#include <const.h>

typedef struct pcb_t {
  struct list_head p_list;
  struct list_head p_children;
  struct list_head p_siblings;
  struct pcb_t *p_parent;
  struct semd_t *p_cursem;
  state_t p_s;
  /* Added fields */
  pid_t pid;
  cputime_t start_time;
  cputime_t elapsed_time;  /* Time last task ticked */
  cputime_t user_time;
  cputime_t global_time;
  int state;
  int semw_wait;
} pcb_t;

#endif
