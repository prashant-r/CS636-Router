/*
 * lab4q3.c
 *
 *  Created on: Feb 13, 2016
 *      Author: prashantravi
 */
/* lab4q3.c - lab4q3_AllTests, testReceiver, testSender , AsynchronousSendTest1, AsynchronousSendTest2, AsynchronousSendTest3, AsynchronousSendTest4*/
#include <xinu.h>
#include <stdio.h>

umsg32 msgglob;
bool8 useSigRecv;
/*------------------------------------------------------------------------
 *  lab4q3_AllTests -  LAB 4Q3 Runs four test cases for lab 3 q4 as required by problem spec.
 *------------------------------------------------------------------------
*/
void lab4q3_AllTests(){
	kprintf("\n // SIGRECV TESTS //\n");

	// LAB 4Q3: V. IMP line, we are setting the useSigRecv to true here
	// so we can test the MYSIGRECV signal functionality
	useSigRecv = TRUE;
	kprintf("\n // XCPUTEST 1 //\n");
	pid32 xcpuTest1 = create(XCputest1,1024,20,"XCputest1",0);
	kprintf("\n // XCPUTEST 2 //\n");
	pid32 xcpuTest2 = create(XCputest2,1024,20,"XCputest2",0);
	resume(xcpuTest1);
	resume(xcpuTest2);
	sleepms(1000);
}

/*------------------------------------------------------------------------
 *  myxcpuhandler -  LAB 4Q3 handler for the MYSIGXCPU signal
 *------------------------------------------------------------------------
*/
void myxcpuhandler()
{
	struct	procent *prptr = &proctab[currpid];
	kprintf(" XCPU handler called on process %d CPU time consumed so far: %d\n", currpid, (prptr->prcpumsec + (clktimemsec- prptr->prctxswintime)));
}
/*------------------------------------------------------------------------
 *  myxcpuhandler_alternate -  LAB 4Q3 another handler for the MYSIGXCPU signal for testing purposes
 *------------------------------------------------------------------------
*/
void myxcpuhandler_alternate()
{
	struct	procent *prptr = &proctab[currpid];
	kprintf(" XCPU handler alternate called on process %d CPU time consumed so far: %d\n", currpid, (prptr->prcpumsec + (clktimemsec- prptr->prctxswintime)));
}


/*------------------------------------------------------------------------
 *  registerSignal -  LAB 4Q3 common method to register a signal
 *------------------------------------------------------------------------
*/
int registerSignal(uint16 asig, int16 time)
{
	switch(asig) {
		   case MYSIGXCPU:{
			   if (registercbsig(MYSIGXCPU, &myxcpuhandler, time) != OK) {
			   			kprintf("xcpu handler registration failed\n");
			   	    	return 1;
			   		}
			   break;
			  }
		   /* you can have any number of case statements */
		   default : /* Optional */
			   kprintf("In registercbsig: Invalid Signal");
		}
		return OK;

}

/*------------------------------------------------------------------------
 *  XCputest1 -  LAB 4Q3 Simple XCputest to test it works, can only verify visually.
 *  			Would recommend subtracting time displayed when call back
 *  			shown from time displayed when started to verify it equals to
 *  			arguments. Not time displayed here is CPU time as apposed to
 *  			wall clock time.
 *------------------------------------------------------------------------
*/
void XCputest1()
{
	registerSignal(MYSIGXCPU, 200);
	while(TRUE)
	{
	}
}

/*------------------------------------------------------------------------
 *  XCputest2 -  LAB 4Q3 Tests that xcputest runs once for each time registered and that
 *  			second time registered for the same process would cause it to
 *  			overwrite the first registration so mycpuxhandler_alternate should be called instead.
 *------------------------------------------------------------------------
*/
void XCputest2()
{
	kprintf("Executing XCputest2 \n");
	registerSignal(MYSIGXCPU, 300);
	if (registercbsig(MYSIGXCPU, &myxcpuhandler_alternate, 120) != OK) {
			kprintf("xcpu handler alternate registration failed\n");
			return;
	}
	while(TRUE)
	{

	}
}