#include <const.h>
#include <bitmap.h>
#include <initial.h>
#include <syscall.h>
#include <scheduler.h>
#include <uARMconst.h>
#include <uARMtypes.h>

#define DEBUG

state_t *state = (state_t *)INT_OLDAREA;
int status_word[DEV_USED_INTS+1][DEV_PER_INT];

void interrupt_handler(void){
  cputime_t kernel_time1, kernel_time2;
  int *dev_bitmap;
  int cause, dnum;

  kernel_time1 = getTODLO();

  cause = state->CP15_Cause;
#ifdef DEBUG
  tprint("Interrupt\n");
#endif
  
  if(CAUSE_IP_GET(cause, INT_TIMER)){
#ifdef DEBUG
    tprint("Interrupt: gestione timer\n");
#endif
    setSTATUS(STATUS_DISABLE_TIMER(getSTATUS()));
  }
  else{
      if(CAUSE_IP_GET(cause, INT_DISK)){
#ifdef DEBUG
    tprint("Interrupt: gestione disk\n");
#endif
	dev_bitmap = (memaddr)0x6FE0;
	dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	/* -3 perché negli array dei device/semafori si parte da **
	** disk (INT_DISK = 3)                                   */
	status_word[INT_DISK-3][dnum] = devices[(INT_DISK-3)+dnum]->status;
	devices[(INT_DISK-3)+dnum]->command = DEV_C_ACK;
	verhogen(&dev_sem[(INT_DISK-3)*DEV_PER_INT + dnum], 1);
      }
    else{
      if(CAUSE_IP_GET(cause, INT_TAPE)){
#ifdef DEBUG
	tprint("Interrupt: gestione tape\n");
#endif
	dev_bitmap = (memaddr)0x6FE4;
	dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	status_word[INT_TAPE-3][dnum] = devices[(INT_TAPE-3)+dnum]->status;
	devices[(INT_TAPE-3)+dnum]->command = DEV_C_ACK;
	verhogen(&dev_sem[(INT_TAPE-3)*DEV_PER_INT + dnum], 1);
      }
      else{
	if(CAUSE_IP_GET(cause, INT_UNUSED)){
#ifdef DEBUG
	  tprint("Interrupt: gestione unused\n");
#endif
	  dev_bitmap = (memaddr)0x6FE8;
	  dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	  status_word[INT_UNUSED-3][dnum] =
	    devices[(INT_UNUSED-3)+dnum]->status;
	  devices[(INT_UNUSED-3)+dnum]->command = DEV_C_ACK;
	  verhogen(&dev_sem[(INT_UNUSED-3)*DEV_PER_INT + dnum], 1);
	}
	else{
	  if(CAUSE_IP_GET(cause, INT_PRINTER)){
#ifdef DEBUG
	    tprint("Interrupt: gestione printer\n");
#endif
	    dev_bitmap = (memaddr)0x6FEC;
	    dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	    status_word[INT_PRINTER-3][dnum] =
	      devices[(INT_PRINTER-3)+dnum]->status;
	    devices[(INT_PRINTER-3)+dnum]->command = DEV_C_ACK;
	    verhogen(&dev_sem[(INT_PRINTER-3)*DEV_PER_INT + dnum], 1);
	  }
	  else{
	    if(CAUSE_IP_GET(cause, INT_TERMINAL)){
#ifdef DEBUG
	      tprint("Interrupt: gestione terminal \n");
#endif
	      dev_bitmap = (memaddr)0x6FF0;
	      dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	      /* read */
	      if(terminals[dnum]->recv_status & DEV_TRCV_S_CHARRECV){
		status_word[INT_TERMINAL-2][dnum] =
		  terminals[dnum]->recv_status;
		terminals[dnum]->recv_command = DEV_C_ACK;
		verhogen(&dev_sem[(INT_TERMINAL-3)*DEV_PER_INT +
				  DEV_PER_INT + dnum], 1);
	      }
	      /* transmit */
	      if(terminals[dnum]->transm_status & DEV_TTRS_S_CHARTRSM){
		status_word[INT_TERMINAL-3][dnum] =
		  terminals[dnum]->transm_status;
		terminals[dnum]->transm_command = DEV_C_ACK;
		verhogen(&dev_sem[(INT_TERMINAL-3)*DEV_PER_INT + dnum], 1);
	      }
	    }
	  }
	}
      }
    }
  }
  
  scheduler();
}
