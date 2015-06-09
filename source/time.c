#include <types.h>
#include <libuarm.h>

void reset_timer(struct time_t *timer){
  timer->upper_time = 0;
  timer->lower_time = 0;
}

void timer_add(struct time_t *first, struct time_t *second,
	       struct time_t *result){
  int i = 1;

  i <<= 31;
  result->lower_time = first->lower_time + second->lower_time;

  /* C'è il riporto da sommare all'upper_time */
  if( (first->lower_time & i) && (second->lower_time & i) )
    result->upper_time ^= 1;

  result->upper_time = first->upper_time + second->upper_time;
}

void timer_sub(struct time_t *first, struct time_t *second,
	       struct time_t *result){
  
}

void gettimeofday(struct time_t *timer){
  timer->lower_time = getTODLO();
  timer->upper_time = getTODHI();
}

void timecpy(struct time_t *dest, struct time_t *source){
  dest->upper_time = source->upper_time;
  dest->lower_time = source->lower_time;
}
