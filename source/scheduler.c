#include <pcb.h>
#include <types.h>
#include <libuarm.h>
#include <scheduler.h>

void scheduler(void){
  /* deve farlo il nucleo */
  /* p_low = LIST_HEAD_INIT(p_low); */
  /* p_norm = LIST_HEAD_INIT(p_norm); */
  /* p_high = LIST_HEAD_INIT(p_high); */
  /* p_idle = LIST_HEAD_INIT(p_idle); */

  while(1){
    /* 
     * 1 aggiornare il timer di current
     * 2 modificare current->state
     * 3 selezionare il prossimo
     * 4 aggiornare il cputimer
     * 5 lanciare il prossimo processo (ldst)
     */ 
  }
}
