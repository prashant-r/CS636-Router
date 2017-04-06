#include <xinu.h>


void ipv6_in(unsigned char * packet)
{
  uip_buf = (unsigned char *) (packet);
  uip_next_hdr = &UIP_IP_BUF->nexthdr;
  printf("Obtained ipv6 packet\n");
  ip_ntoh();

  ip_debug_print();

  uip_len = UIP_IP_BUF->len + UIP_IPH_LEN;
  uip_ext_len = 0;
  if (UIP_IP_BUF->v != 6) {
    //ip_debug_print(p);
    PRINTF("Version is not correct %d \n", UIP_IP_BUF->v );
    return;
  }
  switch(*uip_next_hdr)
  {
    case UIP_PROTO_ICMP6:
      /* ICMPv6 */
      icmp6_input(packet);
      break;

    case UIP_PROTO_FRAG:
      PRINTF("Processing frag header\n");
      break;

    default:
      PRINTF("Undefined uip next header\n");
      goto discard;

    discard:
      uip_clear_buf();
      uip_len = 0;
      return;

  }
  ip_hton();
}

/*------------------------------------------------------------------------
 * ip_ntoh  -  Convert IP header fields to host byte order
 *------------------------------------------------------------------------
 */
void 	ip_ntoh(
	)
{
	UIP_IP_BUF->flow2 = UIP_NTOHS(UIP_IP_BUF->flow2);
	UIP_IP_BUF->len =  UIP_NTOHS(UIP_IP_BUF->len);
}


/*------------------------------------------------------------------------
 * ip_hton  -  Convert IP header fields to network byte order
 *------------------------------------------------------------------------
 */
void 	ip_hton(

	)
{
	UIP_IP_BUF->flow2 = UIP_HTONS(UIP_IP_BUF->flow2);
	UIP_IP_BUF->len =  UIP_HTONS(UIP_IP_BUF->len);
}
