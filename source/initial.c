#include <pcb.h>
#include <asl.h>
#include <const.h>
#include <listx.h>
#include <scheduler.h>

void main(void){

  /* 2 */
  initPcbs();
  initASL();
  
  /* 3 */
  p_low = LIST_HEAD_INIT(p_low);
  p_norm = LIST_HEAD_INIT(p_norm);
  p_high = LIST_HEAD_INIT(p_high);
  p_idle = LIST_HEAD_INIT(p_idle);

  pc_count = 0;
  sb_count = 0;
  current = NULL;
}
