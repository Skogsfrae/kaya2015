#include <uARMconst.h>
#include <uARMtypes.h>
#include <scheduler.h>
#include <syscall.h>
#include <libuarm.h>
#include <types.h>
#include <const.h>

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
  unsigned int sys_num, arg1, arg2, arg3, ret_value;
  cputime_t kernel_time1, kernel_time2;
  state_t *state = (state_t *)SYSBK_OLDAREA;

  #ifdef DEBUG
  tprint("Syshandler: ciao\n");
  #endif

  /* Used to compute kernel time */
  kernel_time1 = getTODLO();

  /* If not excvector and no SYS5, genocidio again */
  if(!current->bool_excvector && (sys_num != SPECTRAPVEC)){
    terminate_process(current->pid);
    scheduler();
  }

  copy_state(current->excvector[EXCP_SYS_OLD], state);

  /* Breakpoint or syscall?? */
  switch(getCAUSE()){
  case EXC_BREAKPOINT:
    break;
  case EXC_SYSCALL:
    if(!(current->p_s.cpsr & STATUS_SYS_MODE)){
      copy_state(current->excvector[EXCP_PGMT_OLD],
		 current->excvector[EXCP_SYS_OLD]);
      CAUSE_EXCCODE_SET(current->excvector[EXCP_PGMT_OLD]->CP15_Cause,
			EXC_RESERVEDINSTR);
      kernel_time2 = getTODLO();
      current->kernel_time += kernel_time2 - kernel_time1;
      pgmtrap_handler();
    }
    
    /* Take sys_num and args */
    sys_num = current->p_s.a1;
    arg1 = current->p_s.a2;
    arg2 = current->p_s.a3;
    arg3 = current->p_s.a4;

    if(sys_num > SYSCALL_MAX){
#ifdef DEBUG
      tprint("Non existing syscall\n");
#endif
      PANIC();
    }

    /* Syscall dispatch */
    switch(sys_num){
    case CREATEPROCESS:
      current->p_s.a1 = create_process((state_t*)arg1, (priority_enum)arg2);
      break;
    case TERMINATEPROCESS:
      terminate_process(arg1);
      break;
    case VERHOGEN:
      verhogen((int*)arg1, arg2);
      break;
    case PASSEREN:
      passeren((int*)arg1, arg2);
      break;
    case SPECTRAPVEC:
      specify_exception_state_vector((state_t**)arg1);
      break;
    case GETCPUTIME:
      get_cpu_time((cputime_t*)arg1, (cputime_t*)arg2);
      break;
    case GETPID:
      current->p_s.a1 = get_pid();
      break;
    case GETPPID:
      current->p_s.a1 = get_ppid();
      break;
    }
  }

  kernel_time2 = getTODLO();
  current->kernel_time += kernel_time2 - kernel_time1;
  current->excvector[EXCP_SYS_NEW]->pc++;
  LDST(current->excvector[EXCP_SYS_NEW]);
}

void pgmtrap_handler(void){
  cputime_t kernel_time1, kernel_time2;
  state_t *state = (state_t *)PGMTRAP_OLDAREA;

  kernel_time1 = getTODLO();
  /* Olocausto again */
  if(!current->bool_excvector){
    terminate_process(current->pid);
    scheduler();
  }
  
  copy_state(current->excvector[EXCP_PGMT_OLD], state);
  kernel_time2 = getTODLO();
  current->kernel_time += kernel_time2 - kernel_time1;
  LDST(current->excvector[EXCP_PGMT_NEW]);
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
  
  copy_state(current->excvector[EXCP_TLB_OLD], state);
  kernel_time2 = getTODLO();
  current->kernel_time += kernel_time2 - kernel_time1;
  LDST(current->excvector[EXCP_TLB_NEW]);
}
