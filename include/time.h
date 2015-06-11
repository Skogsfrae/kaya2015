#ifndef TIME_H
#define TIME_H

#include <types.h>

extern void reset_timer(struct time_t *timer);
extern void timer_add(struct time_t *first, struct time_t *second,
		      struct time_t *result);
extern void timer_sub(struct time_t *first, struct time_t *second,
		      struct time_t *result);
extern void gettimeofday(struct time_t *timer);
extern void timecpy(struct time_t *dest, struct time_t *source);

#endif
