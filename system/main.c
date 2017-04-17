#include <xinu.h>
#include <stdio.h>

/* global variable */
process	main(void)
{
	recvclr();
	resume(create(shell, 8192, 20, "shell", 1, CONSOLE));
	while(TRUE)
	{
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	}
	return OK;
}