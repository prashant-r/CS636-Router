#include <xinu.h>
#define PING6_NB 5
#define PING6_DATALEN 16

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

static uint8_t count = 0;
static char command[20];
static uint16_t nowaddr[8];
static uip_ipaddr_t in_addr;
uip_ipaddr_t dest_addr;

/*---------------------------------------------------------------------------*/
shellcmd xsh_ping6(int nargs, char *args[])
{

  if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
    printf("Use: ping6  address\n\n");
    printf("Description:\n");
    printf("\tUse ICMPv6 Echo to ping a remote host\n");
    printf("Options:\n");
    printf("\t--help\t display this help and exit\n");
    printf("\taddress\t an IP address in dotted decimal\n");
    return 0;
  }
  if (nargs != 2) {
    fprintf(stderr, "%s: invalid arguments\n", args[0]);
    fprintf(stderr, "Try '%s --help' for more information\n",
        args[0]);
    return 1;
  }

  if(count == 0){

    #if MACDEBUG
    // Setup destination address.
    nowaddr[0] = 0xFE80;
    nowaddr[4] = 0x6466;
    nowaddr[5] = 0x6666;
    nowaddr[6] = 0x6666;
    nowaddr[7] = 0x6666;
    uip_ip6addr(&dest_addr, nowaddr[0], nowaddr[1],nowaddr[2],
                nowaddr[3],nowaddr[4],nowaddr[5],nowaddr[6],nowaddr[7]);

    // Set the command to fool the 'if' below.
    memcpy(command, (void *)"ping6", 5);
    #else
    /* prompt */
    KPRINTF("\n > ");
    /** \note the scanf here is blocking (the all stack is blocked waiting
     *  for user input). This is far from ideal and could be improved
     */
    if(strcmp(args[0],"ping6") != 0){
      KPRINTF("> invalid command\n");
      return 0;
    }
    memcpy(command, (void *)"ping6", 5);
    char buf[strlen(args[1])+1];
    char copytoken[7];
    copytoken[0] = '0';
    copytoken[1] = 'x';
    memcpy(buf,(args[1]), strlen(args[1]));
    char *token;
    char *search = ":";
    token = strtok(buf, search);
    
    if(token != NULL && strlen(token) == 4)
      {
        KPRINTF("%s:", token);
        memcpy(copytoken + 2, token, 4);
        copytoken[6] = '\0';
        nowaddr[0] = strtol(copytoken,NULL,16);
    }
    else{
      KPRINTF("\n-x- invalid address \n");
      return -1;
    }

    token = strtok(NULL, search);
    if(token != NULL && strlen(token) == 4){
        KPRINTF("%s:", token);
        memcpy(copytoken + 2, token, 4);
        copytoken[6] = '\0';
        nowaddr[1] = strtol(copytoken,NULL,16);
    }
    else{

      KPRINTF("\n-x- invalid address \n");
      return -1;
    }

    token = strtok(NULL, search);
    if(token != NULL && strlen(token) == 4){
        KPRINTF("%s:", token);
        memcpy(copytoken + 2, token, 4);
        copytoken[6] = '\0';
        nowaddr[2] = strtol(copytoken,NULL,16);
    }
    else{
      KPRINTF("\n-x- invalid address \n");
      return -1;
    }

    token = strtok(NULL, search);
    if(token != NULL && strlen(token) == 4){
        KPRINTF("%s:", token);
        memcpy(copytoken + 2, token, 4);
        copytoken[6] = '\0';
        nowaddr[3] = strtol(copytoken,NULL,16);
    }
    else{
      KPRINTF("\n-x- invalid address \n");
      return -1;
    }

    token = strtok(NULL, search);
    if(token != NULL && strlen(token) == 4){
        KPRINTF("%s:", token);
        memcpy(copytoken + 2, token, 4);
        copytoken[6] = '\0';
        nowaddr[4] = strtol(copytoken,NULL,16);
    }
    else{
      KPRINTF("\n-x- invalid address \n");
      return -1;
    }

    token = strtok(NULL, search);
    if(token != NULL && strlen(token) == 4){
        KPRINTF("%s:", token);
        memcpy(copytoken + 2, token, 4);
        copytoken[6] = '\0';
        nowaddr[5] = strtol(copytoken,NULL,16);
    }
    else{
      KPRINTF("\n-x- invalid address \n");
      return -1;
    }

    token = strtok(NULL, search);
    if(token != NULL && strlen(token) == 4){
        KPRINTF("%s:", token);
        memcpy(copytoken + 2, token, 4);
        copytoken[6] = '\0';
        nowaddr[6] = strtol(copytoken,NULL,16);
    }
    else{
      KPRINTF("\n-x- invalid address \n");
      return -1;
    }

    token = strtok(NULL, "");
    if(token != NULL && strlen(token) == 4){
        KPRINTF("%s:", token);
        memcpy(copytoken + 2, token, 4);
        copytoken[6] = '\0';
        nowaddr[7] = strtol(copytoken,NULL,16);
    }
    else{
      KPRINTF("\n-x- invalid address \n");
      return -1;
    }
    uip_ip6addr(&dest_addr, nowaddr[0], nowaddr[1],nowaddr[2],
                nowaddr[3],nowaddr[4],nowaddr[5],nowaddr[6],nowaddr[7]);
    #endif
  }
  
  if((strcmp(command,"ping6") == 0) && (count < PING6_NB)){

	struct	netpacket pkt;
	struct  netpacket *packet = &pkt;
	packet->net_type = 0x086DD;
	uip_buf = (unsigned char *) (&pkt);
    UIP_IP_BUF->v = 6;
    UIP_IP_BUF->tclass1 = 0;
    UIP_IP_BUF->flow2 = 0;
    UIP_IP_BUF->nexthdr = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = 254;
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &dest_addr);
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr,  &dest_addr);

    UIP_ICMP_BUF->type = ICMP6_ECHO_REQUEST;
    UIP_ICMP_BUF->icode = 0;
    /* set identifier and sequence number to 0 */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0, 4);
    /* put one byte of data */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN,
           count, PING6_DATALEN);


    uip_len = UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN + UIP_IPH_LEN + PING6_DATALEN;
    UIP_IP_BUF->len = (uint8_t)((uip_len - 40));
    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();


    KPRINTF("Sending Echo Request to");
    KPRINT6ADDR(&UIP_IP_BUF->destipaddr);
    KPRINTF("from");
    KPRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    KPRINTF("\n");
    
    memcpy(&BUF->dest, &macbcast, UIP_LLADDR_LEN);
    memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
    

    ip_hton();

    KPRINTF("Ethernet to : \n");
    KPRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",BUF->dest.addr[0], BUF->dest.addr[1], BUF->dest.addr[2], BUF->dest.addr[3],BUF->dest.addr[4], BUF->dest.addr[5]);
    KPRINTF("Ethernet from : \n");
    KPRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",BUF->src.addr[0], BUF->src.addr[1], BUF->src.addr[2], BUF->src.addr[3],BUF->src.addr[4], BUF->src.addr[5]);
    uip_len += sizeof(struct uip_eth_hdr);
    KPRINTF("UIP len is %d \n", uip_len);
    eth_hton(packet);
    ip_debug_print();
    write(ETHER0, uip_buf, uip_len);
    // etimer_set(&ping6_periodic_timer, 3 * CLOCK_SECOND);
    return 1;
  }
  return 0;
}
