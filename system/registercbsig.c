#include <xinu.h>

/*------------------------------------------------------------------------
 *  registercbsig - LAB 4Q3 registers the callback for the three type of callback function signals
 *------------------------------------------------------------------------
*/
syscall registercbsig(uint16 asig, int (*func)(void),uint32 optarg){

	intmask mask;
	mask = disable();
	if(func == NULL)
	{
		restore(mask);
		return SYSERR;
	}
	switch(asig) {
	   case MYSIGXCPU:{
		   struct procent *prptr = &proctab[currpid];
		   if(prptr->xcpufunc == NULL)
		   {
			   kprintf(" XCPU Signal for process %d CPU time consumed so far: %d\n", currpid, prptr->prcpumsec);
			   prptr->prcpumsec = 0;

		   }
		   else
		   {
			   kprintf("Overwriting XCPU Signal for process %d CPU time consumed so far: %d\n", currpid, prptr->prcpumsec);
			   prptr->prcpumsec = 0;
		   }
		   prptr->xcpufunc = func; //sets pointer sigxcpu function callback
		   prptr->xcputime = optarg;

		   break;}
	   default : /* Optional */
		   kprintf("In registercbsig: Invalid Signal \n");
	}
	kprintf("Exiting register cbig \n");
	restore(mask);	//restore the mask
	return OK;
}