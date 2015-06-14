#include <uARMconst.h>
#include <uARMtypes.h>
#include <syscall.h>
#include <libuarm.h>
#include <const.h>
#include <time.h>

void syscall_handler(void){
  struct time_t beginning, ending;
  unsigned int sys_num, arg1, arg2, arg3, ret_value;
  cputime_t kernel_time1, kernel_time2;

  /* Used to compute kernel time */
  kernel_time1 = getTODLO();

  /* If not excvector and no SYS5, genocidio again */
  if(!current->bool_excvector && (sys_num != SPECTRAPVEC))
    terminate_precess(current->pid);

  STST(current->excvector[EXCP_SYS_OLD]);

  /* Breakpoint or syscall?? */
  switch(getCAUSE()){
  case EXC_BREAKPOINT:
    break;
  case EXC_SYSCALL:
    if(!(current->cpsr & STATUS_SYS_MODE)){
      STST(current->excvector[EXCP_PGMT_OLD]);
      CAUSE_EXCCODE_SET(current->excvector[EXCP_PGMT_OLD].CP15_Cause,
			EXC_RESERVEDINSTR);
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
      current->p_s.a1 = create_process(arg1, &arg2);
      break;
    case TERMINATEPROCESS:
      terminate_process(arg1);
      break;
    case VERHOGEN:
      verhogen(&arg1, arg2);
      break;
    case PASSEREN:
      passeren(&arg1, arg2);
      break;
    case SPECTRAPVEC:
      specify_exception_state_vector(arg1);
      break;
    case GETCPUTIME:
      get_cpu_time(&arg1, &arg2);
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
  /* if(current->excvector[0] == NULL) */
  /*   terminate_process(current->pid); */
  LDST(&current->p_s);
}




