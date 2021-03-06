#include <const.h>
#include <bitmap.h>
#include <initial.h>
#include <syscall.h>
#include <scheduler.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <exceptions.h>

state_t *state = (state_t *)INT_OLDAREA;
unsigned int status_word[MAX_DEVICES];

void dev_verhogen(int semaddr, unsigned int s_word)
{
  pcb_t *tmp;
  dev_sem[semaddr]++;
  if((tmp = headBlocked(&dev_sem[semaddr])) != NULL)
    {
      if(tmp->sem_wait >= dev_sem[semaddr])
	{
	  tmp->sem_wait = 0;
	  outBlocked(tmp);
	  tmp->state = READY;
	  /* Adding tmp to proper queue */
	  switch(tmp->prio)
	    {
	    case PRIO_LOW:
	      insertProcQ(&p_low, tmp);
	      break;
	    case PRIO_NORM:
	      insertProcQ(&p_norm, tmp);
	      break;
	    case PRIO_HIGH:
	      insertProcQ(&p_high, tmp);
	      break;
	    }
	  sb_count--;
	  tmp->p_s.a1 = s_word;
	}
    }
  else
    status_word[semaddr] = s_word;
}

void interrupt_handler(void)
{
  cputime_t kernel_time1, kernel_time2, tod;
  unsigned int *dev_bitmap;
  unsigned int cause, dnum;
  dtpreg_t *device;
  termreg_t *terminal;

  kernel_time1 = getTODLO();
  copy_state(&current->p_s, state);
  current->p_s.pc = current->p_s.lr;
  cause = getCAUSE();
  
  if(CAUSE_IP_GET(cause, INT_TIMER))
    {
      setSTATUS(STATUS_DISABLE_TIMER(getSTATUS()));
    }
  else
    {
      if(CAUSE_IP_GET(cause, INT_DISK))
	{
	  dev_bitmap = (memaddr)CDEV_BITMAP_ADDR(INT_DISK);//0x6FE0;
	  dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	  device = (memaddr)DEV_REG_ADDR(INT_DISK, dnum);
	  dev_verhogen(EXT_IL_INDEX(INT_DISK)*DEV_PER_INT + dnum, device->status);
	  device->command = DEV_C_ACK;
	}
      else
	{
	  if(CAUSE_IP_GET(cause, INT_TAPE))
	    {
	      dev_bitmap = (memaddr)CDEV_BITMAP_ADDR(INT_TAPE);//0x6FE4;
	      dnum = get_bit_num(get_bit_mask(*dev_bitmap));
	      device = (memaddr)DEV_REG_ADDR(INT_TAPE, dnum);
	      dev_verhogen(EXT_IL_INDEX(INT_TAPE)*DEV_PER_INT + dnum, device->status);
	      device->command = DEV_C_ACK;
	    }
	  else
	    {
	      if(CAUSE_IP_GET(cause, INT_UNUSED))
		{
		  dev_bitmap = (memaddr)CDEV_BITMAP_ADDR(INT_UNUSED);//0x6FE8;
		  dnum = get_bit_num(get_bit_mask(*dev_bitmap));
		  device = (memaddr)DEV_REG_ADDR(INT_UNUSED, dnum);
		  dev_verhogen(EXT_IL_INDEX(INT_UNUSED)*DEV_PER_INT + dnum, device->status);
		  device->command = DEV_C_ACK;
		}
	      else
		{
		  if(CAUSE_IP_GET(cause, INT_PRINTER))
		    {
		      dev_bitmap = (memaddr)CDEV_BITMAP_ADDR(INT_PRINTER);//0x6FEC;
		      dnum = get_bit_num(get_bit_mask(*dev_bitmap));
		      device = (memaddr)DEV_REG_ADDR(INT_PRINTER, dnum);
		      dev_verhogen(EXT_IL_INDEX(INT_PRINTER)*DEV_PER_INT + dnum, device->status);
		      device->command = DEV_C_ACK;
		    }
		  else
		    {
		      /* Terminal handling */
		      if(CAUSE_IP_GET(cause, INT_TERMINAL))
			{
			  dev_bitmap = (memaddr)CDEV_BITMAP_ADDR(INT_TERMINAL);//0x6FF0;
			  dnum = get_bit_num(find_dev_mask(*dev_bitmap));
			  terminal = (memaddr)DEV_REG_ADDR(INT_TERMINAL, dnum);
			  /* read */
			  if((terminal->recv_status &  DEV_TRCV_S_CHARRECV) == DEV_TRCV_S_CHARRECV)
			    {
			      dev_verhogen(EXT_IL_INDEX(INT_TERMINAL)*DEV_PER_INT + DEV_PER_INT + dnum, terminal->recv_status);
			      terminal->recv_command = DEV_C_ACK;
			    }
			  /* transmit */
			  if((terminal->transm_status & DEV_TTRS_S_CHARTRSM) == DEV_TTRS_S_CHARTRSM)
			    {
			      dev_verhogen(EXT_IL_INDEX(INT_TERMINAL)*DEV_PER_INT + dnum, terminal->transm_status);
			      terminal->transm_command = DEV_C_ACK;
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
