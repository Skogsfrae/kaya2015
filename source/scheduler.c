#include <pcb.h>
#include <types.h>

struct list_head *PRIO_LOW, *PRIO_NORM, *PRIO_HIGH, *PRIO_IDLE;
struct pcb_t *curr_p;
int proc_cnt;
int sblock_cnt;

struct task{
  int time;             /* Time in which the task will tick */
  int elapsed_time;     /* Time last task ticked */
  struct pcb_t *pcb;    /* Pcb associated to the task */
};

