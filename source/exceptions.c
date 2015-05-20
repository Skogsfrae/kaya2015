#include <pcb.h>

/* Soluzione adottata nella versione 0.01 di linux */
static pid_t lastpid = -1; /* -1 no process executing */

int SYSCALL(CREATEPROCESS, state_t *statep, priority_enum *prio)
{
	pcb_t *newp;

	/* Controllo risorse/priorità */
	if( ((newp = allocPcb()) == NULL) && (*prio == PRIO_IDLE) )
		/* No pcb avaleable */
		return -1;

	/* Copy statep */
	newp->p_s.a1 = statep->a1;
	newp->p_s.a2 = statep->a2;
	newp->p_s.a3 = statep->a3;
	newp->p_s.a4 = statep->a4;
	newp->p_s.v1 = statep->v1;
	newp->p_s.v2 = statep->v2;
	newp->p_s.v3 = statep->v3;
	newp->p_s.v4 = statep->v4;
	newp->p_s.v5 = statep->v5;
	newp->p_s.v6 = statep->v6;
	newp->p_s.sl = statep->sl;
	newp->p_s.fp = statep->fp;
	newp->p_s.ip = statep->ip;
	newp->p_s.sp = statep->sp;
	newp->p_s.lr = statep->lr;
	newp->p_s.pc = statep->ps;
	newp->p_s.cpsr = statep->cpsr;
	newp->p_s.CP15_Control = statep->CP15_Control;
	newp->p_s.CP15_EntryHi = statep->CP15_EntryHi;
	newp->p_s.CP15_Cause = statep->CP15_Cause;
	newp->p_s.TOD_Hi = statep->TOD_Hi;
	newp->p_s.TOD_Low = statep->TOD_Low;

	lastpid++;
	newp->pid = lastpid;
	return newp->pid;
}
