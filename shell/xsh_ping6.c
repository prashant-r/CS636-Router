/* xsh_ping.c - xsh_ping */

#include <xinu.h>
#include <stdio.h>
#include <string.h>
/*------------------------------------------------------------------------
 * xsh_ping - shell command to ping a remote host
 *------------------------------------------------------------------------
 */
shellcmd xsh_ping6(int nargs, char *args[])
{
	int retval;
	byte	mac1[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
	char	buf[1500];
	struct	netpacket pkt;
	int32	i;
	bool8	done;

	kprintf("\n\n###########################################################\n\n");

	done = FALSE;
	while (!done) {
		printf("Enter s for sender or r for receiver: ");
		retval = read(CONSOLE, buf, 20);
		if ( retval != 2 ) {
			continue;
		}
		if ( (buf[0] != 'r') && (buf[0] != 's') ) {
			continue;
		} else {
			done = TRUE;
		}
	}
	control(ETHER0, ETH_CTRL_ADD_MCAST, (int32)mac1, 4);
	if ( buf[0] == 'r' ) {
		while (TRUE) {
			retval = read(ETHER0, buf, 1500);
			if (retval < 0) {
				continue;
			}
			if (buf[0] == mac1[0]) {
				kprintf("success!\n");
			}
		}
	} else {
		for (i=0; i<1500; i++) {
			*(i + (char *)&pkt) = 0xff&i;
		}
		memcpy(pkt.net_ethdst, mac1, 6);
		control(ETHER0, ETH_CTRL_GET_MAC,
				(int32)&pkt.net_ethsrc, 0);
		pkt.net_ethtype = ntohs(0x0800);
		while (TRUE) {
			retval = read(CONSOLE, buf, 100);
			write(ETHER0, (char *)&pkt, 1200);
		}
	}	
	return 0;
}