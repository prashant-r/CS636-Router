#include <xinu.h>
#define PING6_NB 5
#define PING6_DATALEN 16

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

static uint8_t count = 0;
static char command[20];
static uint16_t address[8];
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
    address[0] = 0xFE80;
    address[4] = 0x6466;
    address[5] = 0x6666;
    address[6] = 0x6666;
    address[7] = 0x6666;
    uip_ip6addr(&dest_addr, address[0], address[1],address[2],
                address[3],address[4],address[5],address[6],address[7]);

    // Set the command to fool the 'if' below.
    memcpy(command, (void *)"ping6", 5);

#else
    /* prompt */
    printf("> ");
    /** \note the scanf here is blocking (the all stack is blocked waiting
     *  for user input). This is far from ideal and could be improved
     */
    if(strcmp(args[0],"ping6") != 0){
      PRINTF("> invalid command\n");
      return 0;
    }

    char buf[40];

    memcpy(buf, &(args[1]), strlen(args[1]));
    buf[strlen(args[1])] = '.';
    buf[strlen(args[1])+1] = NULL;
    char *token;
    char *search = ":";
    token = strtok(buf, search);
    if(token != NULL && strlen(token) == 4)
      {
        address[0] = token;
    }
    else{
      PRINTF("-x- invalid address \n");
      return -1;
    }

    token = strtok(buf, search);
    if(token != NULL && strlen(token) == 4)
      address[1] = token;
    else{
      PRINTF("-x- invalid address \n");
      return -1;
    }
    token = strtok(buf, search);
    if(token != NULL && strlen(token) == 4)
      address[2] = token;
    else{
      PRINTF("-x- invalid address \n");
      return -1;
    }
    token = strtok(buf, search);
    if(token != NULL && strlen(token) == 4)
      address[3] = token;
    else{
      PRINTF("-x- invalid address \n");
      return -1;
    }
    token = strtok(buf, search);
    if(token != NULL && strlen(token) == 4)
      address[4] = token;
    else{
      PRINTF("-x- invalid address \n");
      return -1;
    }
    token = strtok(buf, search);
    if(token != NULL && strlen(token) == 4)
      address[5] = token;
    else{
      PRINTF("-x- invalid address \n");
      return -1;
    }
    token = strtok(buf, search);
    if(token != NULL && strlen(token) == 4)
      address[6] = token;
    else{
      PRINTF("-x- invalid address \n");
      return -1;
    }
    token = strtok(buf, '.');
    if(token != NULL && strlen(token) == 4)
      address[7] = token;
    else{
      PRINTF("-x- invalid address \n");
      return -1;
    }
    uip_ip6addr(&dest_addr, address[0], address[1],address[2],
                  address[3],address[4],address[5],address[6],address[7]);

    PRINTF("address entered is %s \n");
#endif

  }
  if((strcmp(command,"ping6") == 0) && (count < PING6_NB)){


    UIP_IP_BUF->v = 1;
    UIP_IP_BUF->tclass1 = 1;
    UIP_IP_BUF->flow2 = 0;
    UIP_IP_BUF->nexthdr = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = 254;
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &dest_addr);
    uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr,  &tmp_ipaddr);

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


    PRINTF("Sending Echo Request to");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    PRINTF("from");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("\n");
    count++;
    // etimer_set(&ping6_periodic_timer, 3 * CLOCK_SECOND);
    // return 1;
  }
  return 0;
}