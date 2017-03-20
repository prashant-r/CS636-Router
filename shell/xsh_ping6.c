/* xsh_ping.c - xsh_ping */

#include <xinu.h>
#include <stdio.h>
#include <string.h>
/*------------------------------------------------------------------------
 * xsh_ping - shell command to ping a remote host
 *------------------------------------------------------------------------
 */
static uint8_t count = 0;
static char command[20];
static uint16_t addr[8];
uip_ipaddr_t dest_addr;
void transmit_ping();
#define PING6_NB 5
#define PING6_DATALEN 16
#define MACDEBUG 1
shellcmd xsh_ping6(int nargs, char *args[])
{
	byte	mac1[] = {0x18, 0x03, 0x73, 0x1d, 0x31, 0x7b};
	char	buf[1500];
	struct	netpacket pkt;
	int32	i;
	bool8	done;
  	done = FALSE;
  	int retval;
	while (!done) {
		printf("Enter s for sender or r for receiver: ");
		retval = read(CONSOLE, buf, 20);
		if ( retval != 2 ) {
			printf("Got something");
			continue;
		}
		if ( (buf[0] != 'r') && (buf[0] != 's') ) {
			printf("Got something");
			continue;
		} else {
			done = TRUE;
		}
	}
	//control(ETHER0, ETH_CTRL_ADD_MCAST, (int32)mac1, 4);
	if ( buf[0] == 'r' ) {
		while (TRUE) {
			retval = read(ETHER0, buf, 1500);
			if (retval < 0) {
				continue;
			}
			ipv6_in(buf);
		}
	} else {
		memcpy(&BUF->dest, mac1, 6);
		control(ETHER0, ETH_CTRL_GET_MAC,
				(int32)&BUF->src, 0);
		pkt.net_ethtype = ntohs(0x86DD);
		while (TRUE) {
			transmit_ping();
			write(ETHER0, (char *)uip_buf, uip_len + SIZE_ETHERNET);
		}
	}
  return 0;
}


void transmit_ping()
{
	if(count == 0){
#if MACDEBUG
    // Setup destination address.
    addr[0] = 0xFE80;
    addr[4] = 0x6466;
    addr[5] = 0x6666;
    addr[6] = 0x6666;
    addr[7] = 0x6666;
    uip_ip6addr(&dest_addr, addr[0], addr[1],addr[2],
                addr[3],addr[4],addr[5],addr[6],addr[7]);

    // Set the command to fool the 'if' below.
    memcpy(command, (void *)"ping6", 5);

#else
/* prompt */
    printf("> ");
    /** \note the scanf here is blocking (the all stack is blocked waiting
     *  for user input). This is far from ideal and could be improved
     */
    scanf("%s", command);

    if(strcmp(command,"ping6") != 0){
      PRINTF("> invalid command\n");
      return 0;
    }

    if(scanf(" %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
             &addr[0],&addr[1],&addr[2],&addr[3],
             &addr[4],&addr[5],&addr[6],&addr[7]) == 8){

      uip_ip6addr(&dest_addr, addr[0], addr[1],addr[2],
                  addr[3],addr[4],addr[5],addr[6],addr[7]);
    } else {
      PRINTF("> invalid ipv6 address format\n");
      return 0;
    }
#endif

  }

  if((strcmp(command,"ping6") == 0) && (count < PING6_NB)){

    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 1;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = 1;
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &dest_addr);

    UIP_ICMP_BUF->type = ICMP6_ECHO_REQUEST;
    UIP_ICMP_BUF->icode = 0;
    /* set identifier and sequence number to 0 */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0, 4);
    /* put one byte of data */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN,
           count, PING6_DATALEN);


    uip_len = UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN + UIP_IPH_LEN + PING6_DATALEN;
    UIP_IP_BUF->len[0] = (uint8_t)((uip_len - 40) >> 8);
    UIP_IP_BUF->len[1] = (uint8_t)((uip_len - 40) & 0x00FF);

    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();


    PRINTF("Sending Echo Request to");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    PRINTF("from");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("\n");
    count++;


    return 1;
  }
}