#include <types.h>
#include <pcb.h>

struct list_head p_low, p_norm, p_high, p_idle;
pcb_t *current;
int pc_count;
int sb_count;
