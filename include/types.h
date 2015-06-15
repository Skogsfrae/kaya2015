#ifndef TYPES_H
#define TYPES_H

#include <listx.h>
#include <uARMtypes.h>
#include <const.h>

/* typedef struct time_t{ */
/*   cputime_t upper_time; */
/*   cputime_t lower_time; */
/* } time_t; */

typedef struct pcb_t {
  struct list_head p_list;
  struct list_head p_children;
  struct list_head p_siblings;
  struct pcb_t *p_parent;
  struct semd_t *p_cursem;
  state_t p_s;
  /* Added fields */
  pid_t pid;

  /* Time */
  cputime_t start_time;
  cputime_t elapsed_time;  /* Time task ticked last */
  cputime_t kernel_time;
  cputime_t global_time;

  state_t excvector[6];
  int bool_excvector;
  
  int state;
  int sem_wait;
} pcb_t;

#endif
