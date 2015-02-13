#ifndef TYPES_H
#define TYPES_H

#include "listx.h"
#include <uARMtypes.h>

typedef struct pcb_t {
        struct list_head p_list;
        struct list_head p_children;
        struct list_head p_siblings;
        struct pcb_t *p_parent;
        struct semd_t *p_cursem;
        state_t p_s;
} pcb_t;

#endif
