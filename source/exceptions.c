#include <uARMconst.h>
#include <uARMtypes.h>
#include <scheduler.h>
#include <syscall.h>
#include <libuarm.h>
#include <initial.h>
#include <types.h>
#include <const.h>
#include <arch.h>

//#define DEBUG

void pgmtrap_handler(void);

void copy_state(state_t *dest, state_t *src){
  dest->a1 = src->a1;
  dest->a2 = src->a2;
  dest->a3 = src->a3;
  dest->a4 = src->a4;
  dest->v1 = src->v1;
  dest->v2 = src->v2;
  dest->v3 = src->v3;
  dest->v4 = src->v4;
  dest->v5 = src->v5;
  dest->v6 = src->v6;
  dest->sl = src->sl;
  dest->fp = src->fp;
  dest->ip = src->ip;
  dest->sp = src->sp;
  dest->lr = src->lr;
  dest->pc = src->pc;
  dest->cpsr = src->cpsr;
  dest->CP15_Control = src->CP15_Control;
  dest->CP15_EntryHi = src->CP15_EntryHi;
  dest->CP15_Cause = src->CP15_Cause;
  dest->TOD_Hi = src->TOD_Hi;
  dest->TOD_Low = src->TOD_Low;
}

void syscall_handler(void){
  unsigned int sys_num, arg1, arg2, arg3, cause;
  cputime_t kernel_time1, kernel_time2;
  state_t *state = (state_t *)SYSBK_OLDAREA;

  /* Used to compute kernel time */
  kernel_time1 = getTODLO();
  copy_state(&current->p_s, state);
  cause = state->CP15_Cause; //getCAUSE();
  
  /* Breakpoint or syscall?? */
  switch(CAUSE_EXCCODE_GET(cause)){
  case EXC_BREAKPOINT:
    /* If not excvector and no SYS5, genocidio again */
    if(!current->bool_excvector){
      terminate_process(current->pid);
      scheduler();
    }
    break;
  case EXC_SYSCALL:
    // Passing to program trap
    if(!(current->p_s.cpsr & STATUS_SYS_MODE)){
      CAUSE_EXCCODE_SET(current->p_s.CP15_Cause,
			EXC_RESERVEDINSTR);
      copy_state((state_t *)PGMTRAP_OLDAREA, &current->p_s);
      kernel_time2 = getTODLO();
      current->kernel_time += kernel_time2 - kernel_time1;
      pgmtrap_handler();
    }
    
    /* Take sys_num and args */
    sys_num = state->a1;
    arg1 = state->a2;
    arg2 = state->a3;
    arg3 = state->a4;

    // Non existing syscall
    if(sys_num > SYSCALL_MAX || sys_num < 0)
      PANIC();

    /* Syscall dispatch */
    switch(sys_num){
    case CREATEPROCESS:
      current->p_s.a1 = create_process((memaddr)arg1, (priority_enum)arg2);
      break;
    case TERMINATEPROCESS:
      terminate_process(arg1);
      break;
    case VERHOGEN:
      verhogen((int*)arg1, arg2);
      break;
    case PASSEREN:
      passeren((int *)arg1, arg2);
      break;
    case SPECTRAPVEC:
      specify_exception_state_vector((memaddr)arg1);
      break;
    case GETCPUTIME:
      get_cpu_time((memaddr)arg1, (memaddr)arg2);
      break;
    case WAITCLOCK:
      wait_for_clock();
      break;
    case WAITIO:
      current->p_s.a1 = wait_for_io(arg1, arg2, arg3);
      break;
    case GETPID:
      current->p_s.a1 = get_pid();
      break;
    case GETPPID:
      current->p_s.a1 = get_ppid();
      break;
    default:
      PANIC();
      break;
    }
    break;
  default:
    PANIC();
    break;
  }

  kernel_time2 = getTODLO();
  current->kernel_time += kernel_time2 - kernel_time1;
  current->p_s.pc = current->p_s.lr;
  if(current->state == WAITING)
    scheduler();
  else
    LDST(&current->p_s);
}

void pgmtrap_handler(void){
  cputime_t kernel_time1, kernel_time2;
  state_t *state = (state_t *)PGMTRAP_OLDAREA;
  unsigned int cause;

  cause = state->CP15_Cause;
  if(CAUSE_EXCCODE_GET(cause) == EXC_BUSINVFETCH)
    HALT();

  kernel_time1 = getTODLO();
  /* Olocausto again */
  if(!current->bool_excvector){
    terminate_process(current->pid);
    scheduler();
  }
  
  copy_state(&current->p_s, state);
  kernel_time2 = getTODLO();
  current->kernel_time += kernel_time2 - kernel_time1;
  LDST(&current->p_s);
}

void tlb_handler(void){
  cputime_t kernel_time1, kernel_time2;
  state_t *state = (state_t *)TLB_OLDAREA;

  kernel_time1 = getTODLO();
  /* Olocausto again */
  if(!current->bool_excvector){
    terminate_process(current->pid);
    scheduler();
  }
  
  copy_state(&current->p_s, state);
  kernel_time2 = getTODLO();
  current->kernel_time += kernel_time2 - kernel_time1;
  LDST(&current->p_s);
}
