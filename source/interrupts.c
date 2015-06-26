#include <const.h>
#include <bitmap.h>
#include <syscall.h>
#include <uARMconst.h>

static struct state_t *state = (state_t *)INT_OLDAREA;

void interrupt_handler(void){
  cputime_t kernel_time1, kernel_time2;
  int *dev_bitmap;
  int cause, dnum;

  kernel_time1 = getTODLO();

  cause = current->p_s.cause;
  
  if(CAUSE_IP_GET(cause, INT_TIMER)){
  }
  else{
      if(CAUSE_IP_GET(cause, INT_DISK)){
	dev_bitmap = (memaddr)0x6FE0;
	dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	devices[(INT_DISK-3)+dnum]->command = DEV_C_ACK;
	verhogen(
      }
    else{
      if(CAUSE_IP_GET(cause, INT_TAPE)){
	dev_bitmap = (memaddr)0x6FE4;
	dnum = get_bit_num(get_bit_mask(*dev_bitmap));
      }
      else{
	if(CAUSE_IP_GET(cause, INT_UNUSED)){
	  dev_bitmap = (memaddr)0x6FE8;
	  dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	}
	else{
	  if(CAUSE_IP_GET(cause, INT_PRINTER)){
	    dev_bitmap = (memaddr)0x6FEC;
	    dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	  }
	  else{
	    if(CAUSE_IP_GET(cause, INT_TERMINAL)){
	      dev_bitmap = (memaddr)0x6FF0;
	      dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	    }
	  }
	}
      }
    }
  }
  
  scheduler();
}
