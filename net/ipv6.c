#include <xinu.h>
// /* ethernet headers are always exactly 14 bytes */


void ipv6_in(char * packet)
{
  uip_buf = (unsigned char *) (packet);
  uip_next_hdr = &UIP_IP_BUF->proto;
  PRINTF("Received Echo Request from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");


  uip_len = (UIP_IP_BUF->len[0] << 8) + UIP_IP_BUF->len[1] + UIP_IPH_LEN;
  uip_ext_len = 0;
  


  struct ip_hdr *iphdr;
  iphdr = (struct ip_hdr *)(UIP_IP_BUF);
  
  PRINTF("IN IPV6_IN %d and uip len is %d \n", *uip_next_hdr, uip_len);
  PRINTF("IN IPV6 %d \n", UIP_IP_BUF->vtc);
  PRINTF("IN IPV6 %d %d \n", iphdr->tclass1, iphdr->v);
  PRINTF("IN IPV6 %d \n", iphdr->len);
  ip_debug_print(iphdr);
   /* identify the IP header */
  if (iphdr->v != 6) {
    //ip_debug_print(p);
    PRINTF("Version is not correct %d \n", iphdr->v );
    return;
  }
  PRINTF("Version is correct");
  switch(*uip_next_hdr)
  {
    case UIP_PROTO_ICMP6:
      /* ICMPv6 */
      icmp6_input(packet);
     
  }

}

unsigned short checksum(unsigned short *addr, unsigned short count);
unsigned short checksum(unsigned short *addr, unsigned short count)
{
    unsigned long sum;

    sum = 0;
    while (count > 1) {
            sum += *(unsigned short*)addr++;
            count -= 2;
            //printf("sum is %x\n", sum);
    }

    /*  Add left-over byte, if any */
    if (count){
            sum += *(unsigned char *)addr;
            //printf("in side left-over byte\n");
    }
    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16) {
            sum  = (sum & 0xffff) + (sum >> 16);
            printf("IN while:sum is %x\n", sum);
    }
    printf("sum is %x\n", sum);
    return (unsigned short)(~sum);
}

static uint16_t
upper_layer_chksum(uint8_t proto)
{
  /* gcc 4.4.0 - 4.6.1 (maybe 4.3...) with -Os on 8 bit CPUS incorrectly compiles:
 * int bar (int);
 * int foo (unsigned char a, unsigned char b) {
 *   int len = (a << 8) + b; //len becomes 0xff00&<random>+b
 *   return len + bar (len);
 * }
 * upper_layer_len triggers this bug unless it is declared volatile.
 * See https://sourceforge.net/apps/mantisbt/contiki/view.php?id=3
 */
  uint16_t upper_layer_len;
  uint16_t sum;

  upper_layer_len = (((uint16_t)(UIP_IP_BUF->len[0]) << 8) + UIP_IP_BUF->len[1] - uip_ext_len);

  PRINTF("Upper layer checksum len: %d from: %d with uip_ext_len %d \n", upper_layer_len,
         UIP_IPH_LEN + UIP_LLH_LEN + uip_ext_len, uip_ext_len);

  /* First sum pseudoheader. */
  /* IP protocol and length fields. This addition cannot carry. */
  sum = upper_layer_len + proto;
  /* Sum IP source and destination addresses. */
  sum += checksum((uint8_t *) &UIP_IP_BUF->srcipaddr, 2 * sizeof(uip_ipaddr_t));

  /* Sum TCP header and data. */
  sum += checksum((uint8_t *) &uip_buf[UIP_IPH_LEN + UIP_LLH_LEN + uip_ext_len],
               upper_layer_len);

  return (sum == 0) ? 0xffff : uip_htons(sum);
}

/*---------------------------------------------------------------------------*/
uint16_t
uip_icmp6chksum(void)
{
  return upper_layer_chksum(UIP_PROTO_ICMP6);
}


uint16_t uip_htons(uint16_t val)
{
  return UIP_HTONS(val);
}

void icmp6_input(char * packet)
{

  /* This is IPv6 ICMPv6 processing code. */
  PRINTF("icmp6_input: length %d type: %d \n", uip_len, UIP_ICMP_BUF->type);

  #if UIP_CONF_IPV6_CHECKS
    /* Compute and check the ICMP header checksum */

  if((UIP_IP_BUF->vtc & 0xf0) != 0x60)  { /* IP version and header length. */
    PRINTF("ipv6: invalid version.");
    goto drop;
  }


  if(uip_icmp6chksum() != 0xffff) {
       PRINTF("icmpv6: bad checksum.\n");
      goto drop;
     }
  #endif /*UIP_CONF_IPV6_CHECKS*/
  
    //* Search generic input handlers.
    //* The handler is in charge of setting uip_len to 0
   
   if(uip_icmp6_input(packet, UIP_ICMP_BUF->type,
                      UIP_ICMP_BUF->icode) == UIP_ICMP6_INPUT_ERROR) {
     PRINTF("Unknown ICMPv6 message type/code %d\n", UIP_ICMP_BUF->type);
     // UIP_STAT(++uip_stat.icmp.drop);
     // UIP_STAT(++uip_stat.icmp.typeerr);
     // UIP_LOG("icmp6: unknown ICMPv6 message.");
     uip_clear_buf();
   }
   return;
   drop:
   		uip_clear_buf();
   		return;
}

uint8_t uip_icmp6_input(char * packet, uint8_t type, uint8_t icode)
{
  switch(type)
  {
    case ICMP6_ECHO_REQUEST:
      echo_request(packet);
      PRINTF("Its a request\n");
      break;
    case ICMP6_ECHO_REPLY:
      PRINTF("Its a reply \n");
      break;
    case ICMP6_NS:
      PRINTF("It's a NS \n");
      echo_request(packet);
      break;
    case ICMP6_NA:
      PRINTF("Its a NA \n");
      break;
    case ICMP6_RS:
      PRINTF("Its a RS \n");
      break;
    case ICMP6_RA:
      PRINTF("Its a RA \n");
      break;
    default:
      return UIP_ICMP6_INPUT_ERROR;
      break;
  }
  return UIP_ICMP6_INPUT_SUCCESS;
}

void ns_input(char * packet)
{

}

void echo_request(char * packet)
{
   
    PRINTF("Inet check sum try \n");
    if (uip_icmp6chksum() != 0xffff) {
      PRINTF("icmp_input: checksum failed for received ICMP echo %d \n" , uip_icmp6chksum());
      return;
    }
    PRINTF("inet check sum was fine. \n");
    // PRINTF("icmp: p->len %"S16_F" p->tot_len %"S16_F"\n", p->len, p->tot_len);
    // ip_addr_set(&tmpaddr, &(iphdr->src));
    // ip_addr_set(&(iphdr->src), &(iphdr->dest));
    // ip_addr_set(&(iphdr->dest), &tmpaddr);
    // iecho->type = ICMP6_ER;
    // /* adjust the checksum */
    // if (iecho->chksum >= htons(0xffff - (ICMP6_ECHO << 8))) {
    //   iecho->chksum += htons(ICMP6_ECHO << 8) + 1;
    // } else {
    //   iecho->chksum += htons(ICMP6_ECHO << 8);
    // }
    // PRINTF("icmp_input: checksum failed for received ICMP echo (%"X16_F")\n", inet_chksum_pseudo(p, &(iphdr->src), &(iphdr->dest), IP_PROTO_ICMP, p->tot_len));
    // /*    LWIP_DEBUGF("icmp: p->len %"U16_F" p->tot_len %"U16_F"\n", p->len, p->tot_len);*/
    // PRINTF("Ready to ship out");
}