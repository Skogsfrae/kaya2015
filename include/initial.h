#ifndef INITIAL_H
#define INITIAL_H

#include <types.h>

extern int dev_sem[MAX_DEVICES];
extern dtpreg_t *devices[(DEV_USED_INTS -1)*DEV_PER_INT];
extern termreg_t *terminals[DEV_PER_INT*2];

#endif
