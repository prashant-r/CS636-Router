/* net.c - net_init, netin, rawin, eth_hton, eth_ntoh, getport */

#include <xinu.h>
#include <stdio.h>
#include <string.h>

#include "mactable.incl"

struct	network	NetData;

uint64	netportseed;			/* Used to generate a random	*/
					/*  port number for UDP or TCP	*/

int32	packetdump = 0;			/* 0 => off, 1 => print each	*/
					/*  incoming packet that is	*/
					/*  accepted, 2 => print each	*/
					/*  incoming packet		*/

struct	ifentry	if_tab[NIFACES];	/* The network interfaces	*/

bool8	host;				/* Are we operating as a host	*/
					/*  or a NAT box?		*/

int32	ifprime;			/* Primary interface: for a	*/
					/*  host, the only active	*/
					/*  interface, and for a NAT	*/
					/*  box, the Ethernet interface	*/

bpid32	netbufpool;			/* IP of network buffer pool	*/

/*------------------------------------------------------------------------
 * net_init  -  Initialize network data structures and processes
 *------------------------------------------------------------------------
 */

void	net_init (void)
{
	memset((char *)&NetData, NULLCH, sizeof(struct network));

	control(ETHER0, ETH_CTRL_GET_MAC, (int32)NetData.ethucast, 0);

	memset((char *)NetData.ethbcast, 0xFF, ETH_ADDR_LEN);
	int32	nbufs;			/* Total no of buffers		*/
	int32	iface;			/* Index into interface table	*/
	struct	ifentry	*ifptr;		/* Ptr to interface table entry	*/
	bool8	found;			/* Controls user input loop	*/
	char	buffer[40];		/* Buffer to hold input		*/
	int32	bingid;				
	/* User's Bing ID		*/
	int32	nchars;			/* Number of characters read	*/
	char	ch;			/* One character of input	*/
	int32	uid;			/* Unique ID for this node	*/
	byte	*mptr;			/* Ptr to hardware MAC addr	*/
	int32	tindex;			/* Index into mactable		*/
	byte	*tptr;			/* Ptr to MAC addr. in mactable	*/
	int32	i;			/* Counter used when scanning	*/
					/*	user's input		*/
	char	pname[20];		/* String used for process name	*/

	/* Initialize the random port seed */

	netportseed = getticks();

	/* Allocate the global buffer pool shared by all interfaces */

	nbufs = NIFACES * IF_QUEUESIZE + 1;
	netbufpool = mkbufpool(PACKLEN, nbufs);

	if (netbufpool == SYSERR) {
		kprintf("net_init cannot allocate network buffer pool\n");
		kill(getpid());
	}

	/* Ask the user for a Bing ID */

	found = TRUE;
	while (!found) {
		printf("\nEnter a bing ID between 0 and 255: ");
		nchars = read(CONSOLE, buffer, 5);
		if ((nchars == SYSERR) || (nchars == EOF) ||
							(nchars < 2)) {
			continue;
		}
		nchars--;	/* Eliminate the NEWLINE */
		bingid = 0;
		for (i=0; i<nchars; i++) {
			ch = buffer[i];
			if ( (ch<'0') || (ch>'9')) {
				break;
			}
			bingid = 10*bingid + (ch-'0');
		}
		if ((i<nchars) || (bingid > 254)) {
			continue;
		}
		found = TRUE;
	}

	bingid = 59;
	printf("Bing ID is set to %d.\n", bingid);
	/* Ask the user what to run */
	found = FALSE;
	while (!found) {
			#if UIP_CONF_ROUTER
				host = FALSE;
				ifprime = 0;
				found = TRUE;
				break;
		    #else
				ifprime = HOST;
				host = TRUE;
				found = TRUE;
				break;
		    #endif
	}

	/* Initialize three network interfaces: one for the the		*/
	/*	Ethernet, and two for Othernets				*/


	/* Initialize all fields to zero */

	memset((char *)if_tab, NULLCH, sizeof(if_tab));


	for (iface=0; iface<NIFACES; iface++) {
		ifptr = &if_tab[iface];

		/* Set the device ID of the underlying hardware	device	*/
		/*	used to send and receive packets (all three	*/
		/*	interfaces use the Ethernet device)		*/

		ifptr->if_dev = ETHER0;

		/* Create a sempahore to handle access to the queue	*/
		/*	of incoming packets.  The initial count is	*/
		/*	zero because the queue is empty.		*/

		ifptr->if_sem = semcreate(0);
		if (ifptr->if_sem == SYSERR) {
			panic("net_init: cannot create semaphore");
		}

		/* Set the state to "down" */

		ifptr->if_state = IF_DOWN;

		if (iface == 0) {

			/* Handle name and MAC addresses for Ethernet */

			/* Set the interface name */

			strncpy(ifptr->if_name, "eth0", IF_NLEN);


		} else {

			/* Handle name and MAC addresses for Othernets */

			/* Set the interface name */

			char	tmp[10];
			sprintf(tmp,"oth%d", iface);
			strncpy(ifptr->if_name, tmp, IF_NLEN);
		}
	}

	/* For Ethernet, obtain the MAC address from the hardware */

	ifptr = &if_tab[0];

	control(ETHER0, ETH_CTRL_GET_MAC, (int32) ifptr->if_macucast, 0);

	control(ETHER0, ETH_CTRL_GET_MAC, (int32) &uip_lladdr, 0);

	uip_ds6_init();


	//timer_set(&print_network_status, 30, 1);

	/* Set the broadcast address to all 1's */

	memset(ifptr->if_macbcast, 0xff, ETH_ADDR_LEN);

	/* Generate emulated unicast and broadcast MAC addresses for	*/
	/*	the Othernet interfaces					*/

	mptr = (byte *) &if_tab[0].if_macucast;

	/* Search mactable for the MAC address of this node */

	for (tindex=0; tindex<NMACADDRS; tindex++) {
		tptr = mactable[tindex].mtmac;
		if (memcmp(mptr,tptr,ETH_ADDR_LEN) == 0) {
			/* Found a match */
			uid = mactable[tindex].mtid;
			break;
		}
	}
	if (tindex >= NMACADDRS) {
		panic("net_init: no MAC address match\n");
	}

	/* Othernet 1 */
	
	ifptr = &if_tab[1];
	ifptr->if_macucast[0] = 0x11;
	ifptr->if_macucast[1] = 0xff & (uid >> 24);
	ifptr->if_macucast[2] = 0xff & (uid >> 16);
	ifptr->if_macucast[3] = 0xff & (uid >>  8);
	ifptr->if_macucast[4] = 0xff & (uid >>  0);
	ifptr->if_macucast[5] = 0xff & bingid;

	ifptr->if_macbcast[0] = 0x11;
	ifptr->if_macbcast[1] = 0xff;
	ifptr->if_macbcast[2] = 0xff;
	ifptr->if_macbcast[3] = 0xff;
	ifptr->if_macbcast[4] = 0xff;
	ifptr->if_macbcast[5] = 0xff & bingid;

	control(ETHER0, ETH_CTRL_ADD_MCAST, (int32)ifptr->if_macucast, 0);
	control(ETHER0, ETH_CTRL_ADD_MCAST, (int32)ifptr->if_macbcast, 0);


	/* Othernet 2 */
	
	ifptr = &if_tab[2];
	ifptr->if_macucast[0] = 0x21;
	ifptr->if_macucast[1] = 0xff & (uid >> 24);
	ifptr->if_macucast[2] = 0xff & (uid >> 16);
	ifptr->if_macucast[3] = 0xff & (uid >>  8);
	ifptr->if_macucast[4] = 0xff & (uid >>  0);
	ifptr->if_macucast[5] = 0xff & bingid;

	ifptr->if_macbcast[0] = 0x21;
	ifptr->if_macbcast[1] = 0xff;
	ifptr->if_macbcast[2] = 0xff;
	ifptr->if_macbcast[3] = 0xff;
	ifptr->if_macbcast[4] = 0xff;
	ifptr->if_macbcast[5] = 0xff & bingid;

	control(ETHER0, ETH_CTRL_ADD_MCAST, (int32)ifptr->if_macucast, 0);
	control(ETHER0, ETH_CTRL_ADD_MCAST, (int32)ifptr->if_macbcast, 0);

	

	if (host) {
		printf("\nRunning host on %s\n", if_tab[ifprime].if_name);
		if_tab[ifprime].if_state = IF_UP;
		memcpy(&macbcast,if_tab[ifprime].if_macbcast, UIP_LLADDR_LEN);
	} else {
		kprintf("Running a nat box\n");
		for (i=0; i<NIFACES; i++) {
			ifptr = &if_tab[i];
			ifptr->if_state = IF_UP;
		}
	}


	/* Create a low-level input process that reads raw frames and	*/
	/*	demultiplexes them to the correct interface		*/

	resume(create(rawin, 4096, 8000, "raw_input", 0));


	resume(create(periodic_process, 4096, 8000, "periodic", 0));
	
	for (iface=0; iface<NIFACES; iface++) {
		sprintf(pname, "net%d_input", iface);
		resume(create(netin, 4196, 5000, pname, 1, iface));
	}

	return;
}



/*------------------------------------------------------------------------
 * netin - repeatedly read and process the next incoming packet
 *------------------------------------------------------------------------
 */

process	netin (
	  int32	iface			/* Interface for this process	*/
	)
{
	struct	ifentry	*ifptr;		/* Ptr to interface table entry	*/
	struct	netpacket *pkt;		/* Ptr to current packet	*/

	byte mac_address[6];
	if( iface == 0 ) {
 		control(ETHER0, ETH_CTRL_GET_MAC, (int32) &mac_address, 0);

 		mac_address[0] = 0x33;
 		mac_address[1] = 0x33;
 		mac_address[2] = 0xFF;
 
 		kprintf("\n > Successfully received Local MAC Address \n : %02x:%02x:%02x:%02x:%02x:%02x\n",
  							(unsigned char) mac_address[0],
  							(unsigned char) mac_address[1],
  							(unsigned char) mac_address[2],
  							(unsigned char) mac_address[3],
  							(unsigned char) mac_address[4],
  							(unsigned char) mac_address[5]);
 		control(ETHER0, ETH_CTRL_ADD_MCAST, (int32) &mac_address, 0);
 	}

	/* Do forever: read packet from the network interface and handle*/

	ifptr = &if_tab[iface];

	while(1) {

		/* Obtain next packet arriving on an interface */

		wait(ifptr->if_sem);
		pkt = ifptr->if_queue[ifptr->if_head++];
		if (ifptr->if_head >= IF_QUEUESIZE) {
			ifptr->if_head = 0;
		}

		/* Store interface number in packet buffer */

		pkt->net_iface = iface;
		/* Convert Ethernet Type field to host order */

		eth_ntoh(pkt);

		/* Demultiplex on Ethernet type */

		switch (pkt->net_type) {

		    case ETH_ARP:			/* Handle ARP	*/
			freebuf((char *)pkt);
			continue;

		    case ETH_IP:			/* Handle IPv4	*/
			freebuf((char *)pkt);
			continue;
	
		    case ETH_IPv6:			/* Handle IPv6	*/
		    ipv6_in((unsigned char*) pkt);
		    if(uip_len > 0 )
		    {
		    	eth_hton(pkt);
				int special = 1;
				if(special == 1)
				{
					memcpy(&BUF->dest,ifptr->if_macbcast, UIP_LLADDR_LEN);
					memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
					PRINTF("Ethernet from : \n");
					PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",BUF->dest.addr[0], BUF->dest.addr[1], BUF->dest.addr[2], BUF->dest.addr[3],BUF->dest.addr[4], BUF->dest.addr[5]);
					PRINTF("Ethernet to : \n");
					PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",BUF->src.addr[0], BUF->src.addr[1], BUF->src.addr[2], BUF->src.addr[3],BUF->src.addr[4], BUF->src.addr[5]);
					uip_len += sizeof(struct uip_eth_hdr);
					PRINTF("UIP len is %d \n", uip_len);
					pdump(uip_buf);
					write(ETHER0, uip_buf, uip_len);
					freebuf((char *)pkt);
				}
				else
				{
					test_send_packet(pkt);
				}
			}
			freebuf((char *)pkt);
			continue;

		    default:	/* Ignore all other incoming packets	*/
			freebuf((char *)pkt);
			continue;
		}

		
	}
}

void test_send_packet(struct netpacket *pkt)
{
    memcpy(&BUF->dest, &(pkt->net_src), UIP_LLADDR_LEN);
	memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
	KPRINTF("Ethernet from : \n");
	KPRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",BUF->dest.addr[0], BUF->dest.addr[1], BUF->dest.addr[2], BUF->dest.addr[3],BUF->dest.addr[4], BUF->dest.addr[5]);
	KPRINTF("Ethernet to : \n");
	KPRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",BUF->src.addr[0], BUF->src.addr[1], BUF->src.addr[2], BUF->src.addr[3],BUF->src.addr[4], BUF->src.addr[5]);
	uip_len += sizeof(struct uip_eth_hdr);
	KPRINTF("UIP len is %d \n", uip_len);
	pdump(uip_buf);
	write(ETHER0, uip_buf, uip_len);
	freebuf((char *)pkt);
}


process periodic_process(void)
{
	while(1)
	{
		if(!UIP_CONF_ROUTER)
 			uip_ds6_send_rs();
      	uip_ds6_periodic();
		sleep(65);
	}
}



void print_network_status()
{
  int i;
  uint8_t state;
  // uip_ds6_defrt_t *default_route;
  // uip_ds6_route_t *route;
  KPRINTF("--- Network status ---\n");
  /* Our IPv6 addresses */
  KPRINTF("- Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      KPRINTF("-- ");
      KPRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      KPRINTF("\n");
    }
  }
  // /* Our default route */
  // KPRINTF("- Default route:\n");
  // default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
  // if(default_route != NULL) {
  //   KPRINTF("-- ");
  //   KPRINT6ADDR(&default_route->ipaddr);;
  //   //PRINTA(" (lifetime: %lu seconds)\n", (unsigned long)default_route->lifetime.interval);
  // } else {
  //   KPRINTF("-- None\n");
  // }
  // /* Our routing entries */
  // KPRINTF("- Routing entries (%u in total):\n", uip_ds6_route_num_routes());
  // route = uip_ds6_route_head();
  // while(route != NULL) {
  //   KPRINTF("-- ");
  //   KPRINT6ADDR(&route->ipaddr);
  //   KPRINTF(" via ");
  //   KPRINT6ADDR(uip_ds6_route_nexthop(route));
  //   KPRINTF(" (lifetime: %lu seconds)\n", (unsigned long)route->state.lifetime);
  //   route = uip_ds6_route_next(route);
  // }
  // KPRINTF("----------------------\n");
}



/*------------------------------------------------------------------------
 * rawin - continuously read the next incoming frame, examine the MAC
 *		address, and enqueue on the appropriate interface queue
 *------------------------------------------------------------------------
 */

process	rawin (void) {

	status	retval;			/* return value from function	*/
	struct	netpacket *pkt;		/* packet buffer being used now	*/
	struct	ifentry	*ifptr;		/* ptr to interface table entry	*/
	int32	iface;			/* index into interface table	*/

	pkt = (struct netpacket *)getbuf(netbufpool);
	while(1) {
	    	retval = read(ETHER0, (char *)pkt, PACKLEN);
	    	if (retval == SYSERR) {
			panic("rawin -- Ethernet read error");
	    	}

		/* If source is an othernet, restore the multicast bit */

		if ( (pkt->net_src[1] & 0xff) == 0x80 &&
			(pkt->net_src[2] & 0xff) == 0x0a ) {
				pkt->net_src[0] |= 0x01;
		}

		if (packetdump == 2) {
			pdump(pkt);
		}

		/* Demultiplex on MAC address */

		for (iface=NIFACES-1; iface>0; iface--) {
		    ifptr = &if_tab[iface];
		    if (ifptr->if_state != IF_UP) {
			continue;
		    }
		    if ( (memcmp(ifptr->if_macucast, pkt->net_dst,
				OTH_ADDR_LEN) == 0) ||
			 (memcmp(ifptr->if_macbcast, pkt->net_dst,
				OTH_ADDR_LEN) == 0)) {

			/* packet goes to this interface */

			pkt -> net_iface = iface;
			if (packetdump == 1) {
				pdump(pkt);
			}

			/* Check interface queue; drop packet if full	*/
									
			if (semcount(ifptr->if_sem) >= IF_QUEUESIZE) {
				kprintf("rawin: queue overflow on %s\n",
					if_tab[iface].if_name);
				break;
			}

			/* Enqueue packet and signal semaphore */

			ifptr->if_queue[ifptr->if_tail++] = pkt;
			if (ifptr->if_tail >= IF_QUEUESIZE) {
				ifptr->if_tail = 0;
			}
			signal(ifptr->if_sem);

			/* Obtain a buffer for the next packet */

			pkt = (struct netpacket *)getbuf(netbufpool);
			break;
		    }
		}

		if (iface == 0) {

			/* Not an Othernet - send to Ethernet */

		        ifptr = &if_tab[iface];
			if (ifptr->if_state != IF_UP) {
				continue;
			}

			/* packet goes to the Ethernet */

			pkt -> net_iface = iface;
			if (packetdump == 1) {
				pdump(pkt);
			}

			/* Check interface queue; drop packet if full	*/
									
			if (semcount(ifptr->if_sem) >= IF_QUEUESIZE) {
				kprintf("rawin: queue overflow on %s\n",
					if_tab[iface].if_name);
				break;
			}

			/* Enqueue packet and signal semaphore */

			ifptr->if_queue[ifptr->if_tail++] = pkt;
			if (ifptr->if_tail >= IF_QUEUESIZE) {
				ifptr->if_tail = 0;
			}
			signal(ifptr->if_sem);

			/* Obtain a buffer for the next packet */

			pkt = (struct netpacket *)getbuf(netbufpool);
		}
	}
	return SYSERR;	/* To eliminate a compiler warning */
}

/*------------------------------------------------------------------------
 * eth_hton  -  Convert Ethernet type field to network byte order
 *------------------------------------------------------------------------
 */
void 	eth_hton(
	  struct netpacket *pktptr
	)
{
	pktptr->net_type = htons(pktptr->net_type);
}


/*------------------------------------------------------------------------
 * eth_ntoh  -  Convert Ethernet type field to host byte order
 *------------------------------------------------------------------------
 */
void 	eth_ntoh(
	  struct netpacket *pktptr
	)
{
	pktptr->net_type = ntohs(pktptr->net_type);
}


/*------------------------------------------------------------------------
 * getport  -  Retrieve a random port number 
 *------------------------------------------------------------------------
 */
uint16 	getport(void)
{
	netportseed = 6364136223846793005ULL * netportseed + 1;
	return 50000 + ((uint16)((netportseed >> 48)) % 15535);
}

void timer_set(void (*func)(void), int time, int periodic)
{
	if(periodic == 1)
	{
		pid32 function = create(schedule_periodic,1024,20,"",2,func, time);
		resume(function);
	}
	else
	{
		pid32 function = create(schedule_once, 1024,20, "", 2,func, time);
	}
}

void schedule_periodic(void (*func)(void), int time)
{
	while(1)
	{
		if(time >= 0)
			sleep(time);
		func();
	}
}

void schedule_once(void (*func)(void),int time)
{
	if(time >= 0)
		sleep(time);
	func();
}

/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  KPRINTF("Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      KPRINTF(" ");
      KPRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      KPRINTF("\n");
    }
  }
}