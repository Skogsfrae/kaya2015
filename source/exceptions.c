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
  unsigned int sys_num, arg1, arg2, arg3, ret_value, cause;
  cputime_t kernel_time1, kernel_time2;
  state_t *state = (state_t *)SYSBK_OLDAREA;

#ifdef DEBUG
  tprint("Syshandler: ciao\n");
#endif

  /* Used to compute kernel time */
#ifdef DEBUG
  tprint("Syshandler: getting TOD\n");
#endif
  kernel_time1 = getTODLO();

#ifdef DEBUG
  tprint("Syshandler: copying state\n");
#endif
  copy_state(&current->excvector[EXCP_SYS_OLD], state);

#ifdef DEBUG
  tprint("Syshandler: getting cause\n");
#endif
  cause = state->CP15_Cause; //getCAUSE();
  
  /* Breakpoint or syscall?? */
  switch(CAUSE_EXCCODE_GET(cause)){
  case EXC_BREAKPOINT:
#ifdef DEBUG
    tprint("Syshandler: this is a breakpoint\n");
#endif
    /* If not excvector and no SYS5, genocidio again */
    if(!current->bool_excvector){
      terminate_process(current->pid);
      scheduler();
    }
    break;
  case EXC_SYSCALL:
#ifdef DEBUG
    tprint("Syshandler: this is a syscall\n");
#endif
    if(!(current->p_s.cpsr & STATUS_SYS_MODE)){
#ifdef DEBUG
      tprint("Syshandler: passing to pgmtrap\n");
#endif
      copy_state((state_t *)PGMTRAP_OLDAREA,
		 &current->excvector[EXCP_SYS_OLD]);
      CAUSE_EXCCODE_SET(current->excvector[EXCP_PGMT_OLD].CP15_Cause,
			EXC_RESERVEDINSTR);
      kernel_time2 = getTODLO();
      current->kernel_time += kernel_time2 - kernel_time1;
      pgmtrap_handler();
    }
    
    /* Take sys_num and args */
    sys_num = state->a1;
    arg1 = state->a2;
    arg2 = state->a3;
    arg3 = state->a4;

    if(sys_num > SYSCALL_MAX || sys_num < 0){
#ifdef DEBUG
      tprint("Syshandler: non existing syscall\n");
#endif
      PANIC();
    }
#ifdef DEBUG
    tprint("Syshandler: dispatch\n");
#endif
    /* Syscall dispatch */
    switch(sys_num){
    case CREATEPROCESS:
#ifdef DEBUG
      tprint("Syshandler: createprocess\n");
#endif
      current->p_s.a1 = create_process((memaddr)arg1, (priority_enum)arg2);
      break;
    case TERMINATEPROCESS:
#ifdef DEBUG
      tprint("Syshandler: terminateprocess\n");
#endif
      terminate_process(arg1);
      break;
    case VERHOGEN:
#ifdef DEBUG
      tprint("Syshandler: verhogen\n");
#endif
      verhogen((int*)arg1, arg2);
      break;
    case PASSEREN:
#ifdef DEBUG
      tprint("Syshandler: passeren\n");
#endif
      passeren(arg1, arg2);
      break;
    case SPECTRAPVEC:
#ifdef DEBUG
      tprint("Syshandler: systrapvect\n");
#endif
      specify_exception_state_vector((memaddr)arg1);
      break;
    case GETCPUTIME:
#ifdef DEBUG
      tprint("Syshandler: cputime\n");
#endif
      get_cpu_time((memaddr)arg1, (memaddr)arg2);
      break;
    case GETPID:
#ifdef DEBUG
      tprint("Syshandler: getpid\n");
#endif
      current->p_s.a1 = get_pid();
      break;
    case GETPPID:
#ifdef DEBUG
      tprint("Syshandler: getppid\n");
#endif
      current->p_s.a1 = get_ppid();
      break;
/*     default: */
/* #ifdef DEBUG */
/*       tprint("Syshandler: wrong syscall\n"); */
/* #endif */
/*       PANIC(); */
/*       break; */
    }
    break;
  /* default: */
  /*   PANIC(); */
  /*   break; */
  }

#ifdef DEBUG
  tprint("Syshandler: fine\n");
#endif
  kernel_time2 = getTODLO();
  current->kernel_time += kernel_time2 - kernel_time1;
  current->p_s.pc += WS;
  //copy_state(&current->p_s, &current->excvector[EXCP_SYS_OLD]);
  LDST(&current->excvector[EXCP_SYS_OLD]);
}

void pgmtrap_handler(void){
  cputime_t kernel_time1, kernel_time2;
  state_t *state = (state_t *)PGMTRAP_OLDAREA;
  unsigned int cause;

  cause = state->CP15_Cause;
  if(CAUSE_EXCCODE_GET(cause) == EXC_BUSINVFETCH)
    HALT();

#ifdef DEBUG
  tprint("Pgmtrap: ciao\n");
#endif
  kernel_time1 = getTODLO();
  /* Olocausto again */
  if(!current->bool_excvector){
    terminate_process(current->pid);
    scheduler();
  }
  
  copy_state(&current->excvector[EXCP_PGMT_OLD], state);
  kernel_time2 = getTODLO();
  current->kernel_time += kernel_time2 - kernel_time1;
  LDST(&current->excvector[EXCP_PGMT_NEW]);
}

void tlb_handler(void){
  cputime_t kernel_time1, kernel_time2;
  state_t *state = (state_t *)TLB_OLDAREA;

#ifdef DEBUG
  tprint("Tlbhandler: ciao\n");
#endif
  kernel_time1 = getTODLO();
  /* Olocausto again */
  if(!current->bool_excvector){
    terminate_process(current->pid);
    scheduler();
  }
  
  copy_state(&current->excvector[EXCP_TLB_OLD], state);
  kernel_time2 = getTODLO();
  current->kernel_time += kernel_time2 - kernel_time1;
  LDST(&current->excvector[EXCP_TLB_NEW]);
}
