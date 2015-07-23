#include <const.h>
#include <bitmap.h>
#include <initial.h>
#include <syscall.h>
#include <scheduler.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <exceptions.h>

//#define DEBUG

state_t *state = (state_t *)INT_OLDAREA;
/* Numero di interrupt lines +1 (2 per i terminali) */
unsigned int status_word[DEV_USED_INTS+1][DEV_PER_INT];
unsigned int *devAddrBase;

void interrupt_handler(void)
{
  cputime_t kernel_time1, kernel_time2, tod;
  unsigned int *dev_bitmap;
  unsigned int cause, dnum;

  kernel_time1 = getTODLO();

  copy_state(&current->p_s, state);
  current->p_s.pc = current->p_s.lr - WS;

  cause = getCAUSE();
#ifdef DEBUG
  tprint("Interrupt\n");
#endif
  
  if(CAUSE_IP_GET(cause, INT_TIMER)){
#ifdef DEBUG
    tprint("Interrupt: gestione timer\n");
#endif
    setSTATUS(STATUS_DISABLE_TIMER(getSTATUS()));
    scheduler();
  }
  else{
      if(CAUSE_IP_GET(cause, INT_DISK)){
#ifdef DEBUG
    tprint("Interrupt: gestione disk\n");
#endif
	dev_bitmap = (memaddr)0x6FE0;
	dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	/* -3 perché negli array dei device/semafori si parte da **
	** disk (INT_DISK = 3)  */
	devAddrBase =  (memaddr) (0x1000.0050p0 + ((INT_DISK - 3) * 0x80) + (dnum * 0x10)); //Corretto, errore di esponente
	status_word[INT_DISK-3][dnum] = *devAddrBase;
	devAddrBase += 0x4;
	*devAddrBase + = DEV_C_ACK;
	verhogen(&dev_sem[(INT_DISK-3)*DEV_PER_INT + dnum], 1);
      }
    else{
      if(CAUSE_IP_GET(cause, INT_TAPE)){
#ifdef DEBUG
	tprint("Interrupt: gestione tape\n");
#endif
	dev_bitmap = (memaddr)0x6FE4;
	dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	devAddrBase = (memaddr) (0x1000.0050p0 + ((INT_TAPE - 3) * 0x80) + (dnum * 0x10)); //Corretto
	status_word[INT_TAPE-3][dnum] = *devAddrBase;
	devAddrBase += 0x4;
	*devAddrBase = DEV_C_ACK;
	verhogen(&dev_sem[(INT_TAPE-3)*DEV_PER_INT + dnum], 1);
      }
      else{
	if(CAUSE_IP_GET(cause, INT_UNUSED)){
#ifdef DEBUG
	  tprint("Interrupt: gestione unused\n");
#endif
	  dev_bitmap = (memaddr)0x6FE8;
	  dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	  devAddrBase = (memaddr) (0x1000.0050p0 + ((INT_UNUSED - 3) * 0x80) + (dnum * 0x10)); //Corretto
	  status_word[INT_UNUSED-3][dnum] = *devAddrBase;
	  devAddrBase += 0x4;
	  *devAddrBase = DEV_C_ACK;
	  verhogen(&dev_sem[(INT_UNUSED-3)*DEV_PER_INT + dnum], 1);
	}
	else{
	  if(CAUSE_IP_GET(cause, INT_PRINTER)){
#ifdef DEBUG
	    tprint("Interrupt: gestione printer\n");
#endif
	    dev_bitmap = (memaddr)0x6FEC;
	    dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	    devAddrBase =  (memaddr) (0x1000.0050p0 + ((INT_PRINTER - 3) * 0x80) + (dnum * 0x10)); //Corretto
	    status_word[INT_PRINTER-3][dnum] = *devAddrBase;
	    devAddrBase += 0x4;
	    *devAddrBase = DEV_C_ACK;
	    verhogen(&dev_sem[(INT_PRINTER-3)*DEV_PER_INT + dnum], 1);
	  }
	  else{
	    /* Gestione dei terminali */
	    if(CAUSE_IP_GET(cause, INT_TERMINAL)){
#ifdef DEBUG
	      tprint("Interrupt: gestione terminal \n");
#endif
	      dev_bitmap = (memaddr)0x6FF0;
	      /* DEBUG INFO le funzioni di gestione delle bitmap sono */
	      /* corrette, altrimenti la verhogen dovrebbe sbloccare  */
	      /* un altro semaforo e il processo restare bloccato     */
	      dnum = get_bit_num(find_dev_mask(*dev_bitmap));
	      devAddrBase = (memaddr) (0x1000.0050p0 + ((INT_TERMINAL - 3) * 0x80) + (dnum * 0x10)); //Corretto
	      /* read */
	      if((*devAddrBase &  DEV_TRCV_S_CHARRECV)
		 == DEV_TRCV_S_CHARRECV){
#ifdef DEBUG
		tprint("Interrupt: terminal char recv\n");
#endif
		status_word[INT_TERMINAL-2][dnum] =
		  *devAddrBase;
		devAddrBase += 0x4;
		*devAddrBase = DEV_C_ACK;
		verhogen(&dev_sem[(INT_TERMINAL-3)*DEV_PER_INT +
				  DEV_PER_INT + dnum], 1);
	      }
	      /* transmit */
	      devAddrBase += 0x8; //Punto a RecvStatus
	      if((*devAddrBase & DEV_TTRS_S_CHARTRSM)
		 == DEV_TTRS_S_CHARTRSM){
#ifdef DEBUG
		tprint("Interrupt: terminal char trsm\n");
#endif
		status_word[INT_TERMINAL-3][dnum] =
		  *devAddrBase;
		devAddrBase += 0x4;
		*devAddrBase = DEV_C_ACK;
		verhogen(&dev_sem[(INT_TERMINAL-3)*DEV_PER_INT + dnum], 1);
	      }
	    }
	  }
	}
      }
    }
  }

  kernel_time2 = getTODLO();
  current->kernel_time += kernel_time2 - kernel_time1;

  scheduler();
}
