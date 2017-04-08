#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  handleCallback  -  LAB 4Q2  LAB 4Q3 this method handles the invocation of callback functions for MYSIGALRM and
 *  				  MYSIGRECV signals, MYSIGXCPU is take care of in the clkhandler.c . If the conditions
 *  				  are met then the corresponding callback function is invoked right after being context
 *  				  switched in .
 *------------------------------------------------------------------------
 */
void handleCallback()
{
	struct procent *ptcurr;
	ptcurr = &proctab[currpid];

	// MYSIGRECV related callback processing
	if(ptcurr->callback != NULL)
	{
		if(ptcurr->prhasmsg == TRUE){ // ensure that a message has arrived
		void (*callbackfn) () = ptcurr->callback;
		callbackfn();
		ptcurr->prhasmsg = FALSE; // RESET the message buffer flag
		}
	}
	// MYSIGALARM related callback processing
	if (ptcurr->alarmfunc!= NULL) {
		if(ptcurr->alarmTimeOut)
		{
			ptcurr->alarmTimeOut = FALSE;
			ptcurr->alarmtime = 0;
			void (*alarmFunction) () = ptcurr->alarmfunc;
			alarmFunction();
			ptcurr->alarmfunc = NULL;
		}
	}

}

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */

	currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	// LAB 4Q3: Invoke the handleCallback
	handleCallback();

	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}