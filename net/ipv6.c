#include <xinu.h>

uint8_t uip_ext_bitmap = 0;
void ipv6_in(unsigned char * packet)
{
  printf("Got a packet");
  uip_buf = (unsigned char *) (packet);
  uip_next_hdr = &UIP_IP_BUF->nexthdr;
  ip_ntoh();
  ip_debug_print();
  uip_len = UIP_IP_BUF->len + UIP_IPH_LEN;
  uip_ext_len = 0;
  if (UIP_IP_BUF->v != 6) {
    //ip_debug_print(p);
    PRINTF("Version is not correct %d \n", UIP_IP_BUF->v );
    return;
  }

  #if UIP_CONF_ROUTER
  uip_next_hdr = &UIP_IP_BUF->nexthdr;
  uip_ext_len = 0;
  uip_ext_bitmap = 0;
  // if(*uip_next_hdr == UIP_PROTO_HBHO) {
  // #if UIP_CONF_IPV6_CHECKS
  //     uip_ext_bitmap |= UIP_EXT_HDR_BITMAP_HBHO;
  // #endif /* UIP_CONF_IPV6_CHECKS */
  //     switch(ext_hdr_options_process()) {
  //     case 0:
  //       /* continue */
  //       uip_next_hdr = &UIP_EXT_BUF->next;
  //       uip_ext_len += (UIP_EXT_BUF->len << 3) + 8;
  //       break;
  //     case 1:
  //       PRINTF("Dropping packet after extension header processing\n");
  //       /* silently discard */
  //       goto drop;
  //     case 2:
  //       PRINTF("Sending error message after extension header processing\n");
  //       /* send icmp error message (created in ext_hdr_options_process)
  //        * and discard*/
  //       goto send;
  //     }
  //   }

    /*
     * Process Packets with a routable multicast destination:
     * - We invoke the multicast engine and let it do its thing
     *   (cache, forward etc).
     * - We never execute the datagram forwarding logic in this file here. When
     *   the engine returns, forwarding has been handled if and as required.
     * - Depending on the return value, we either discard or deliver up the stack
     *
     * All multicast engines must hook in here. After this function returns, we
     * expect UIP_BUF to be unmodified
     */
  #if UIP_IPV6_MULTICAST
    if(uip_is_addr_mcast_routable(&UIP_IP_BUF->destipaddr)) {
      if(UIP_MCAST6.in() == UIP_MCAST6_ACCEPT) {
        /* Deliver up the stack */
        goto process;
      } else {
        /* Don't deliver up the stack */
        goto drop;
      }
    }
  #endif /* UIP_IPV6_MULTICAST */

    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    /* TBD Some Parameter problem messages */
    if(!uip_ds6_is_my_addr(&UIP_IP_BUF->destipaddr) &&
       !uip_ds6_is_my_maddr(&UIP_IP_BUF->destipaddr)) {
      if(!uip_is_addr_mcast(&UIP_IP_BUF->destipaddr) &&
         !uip_is_addr_linklocal(&UIP_IP_BUF->destipaddr) &&
         !uip_is_addr_linklocal(&UIP_IP_BUF->srcipaddr) &&
         !uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr) &&
         !uip_is_addr_loopback(&UIP_IP_BUF->destipaddr)) {


        /* Check MTU */
        if(uip_len > UIP_LINK_MTU) {
          uip_icmp6_error_output(ICMP6_PACKET_TOO_BIG, 0, UIP_LINK_MTU);
          //UIP_STAT(++uip_stat.ip.drop);
          goto send;
        }
        /* Check Hop Limit */
        if(UIP_IP_BUF->ttl <= 1) {
          uip_icmp6_error_output(ICMP6_TIME_EXCEEDED,
                                 ICMP6_TIME_EXCEED_TRANSIT, 0);
          //UIP_STAT(++uip_stat.ip.drop);
          goto send;
        }

        UIP_IP_BUF->ttl = UIP_IP_BUF->ttl - 1;
        PRINTF("Forwarding packet to ");
        PRINT6ADDR(&UIP_IP_BUF->destipaddr);
        PRINTF("\n");
        //UIP_STAT(++uip_stat.ip.forwarded);
        goto send;
      } else {
        if((uip_is_addr_linklocal(&UIP_IP_BUF->srcipaddr)) &&
           (!uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr)) &&
           (!uip_is_addr_loopback(&UIP_IP_BUF->destipaddr)) &&
           (!uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) &&
           (!uip_ds6_is_addr_onlink((&UIP_IP_BUF->destipaddr)))) {
          PRINTF("LL source address with off link destination, dropping\n");
          uip_icmp6_error_output(ICMP6_DST_UNREACH,
                                 ICMP6_DST_UNREACH_NOTNEIGHBOR, 0);
          goto send;
        }
        PRINTF("Dropping packet, not for me and link local or multicast\n");
        //UIP_STAT(++uip_stat.ip.drop);
        goto drop;
      }
    }
  #else /* UIP_CONF_ROUTER */
    if(!uip_ds6_is_my_addr(&UIP_IP_BUF->destipaddr) &&
       !uip_ds6_is_my_maddr(&UIP_IP_BUF->destipaddr) &&
       !uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
      PRINTF("Dropping packet, not for me\n");
      //UIP_STAT(++uip_stat.ip.drop);
      goto drop;
    }

    /*
     * Next header field processing. In IPv6, we can have extension headers,
     * they are processed here
     */
    uip_next_hdr = &UIP_IP_BUF->nexthdr;
    uip_ext_len = 0;
    uip_ext_bitmap = 0;
  #endif /* UIP_CONF_ROUTER */

  switch(*uip_next_hdr)
  {
    case UIP_PROTO_ICMP6:
      /* ICMPv6 */
      icmp6_input(packet);
      if(uip_len > 0)
        goto send;
      else
        goto discard;
      break;

    case UIP_PROTO_FRAG:
      PRINTF("Processing frag header\n");
      goto discard;
      break;

    default:
      PRINTF("Undefined uip next header\n");
      goto discard;

    discard:
      uip_clear_buf();
      uip_len = 0;
      return;

  }
  send:
  ip_hton();
  PRINTF("Sending packet with length %d (%d)\n", uip_len,
      (UIP_IP_BUF->len));
  PRINTF("Here is the packet \n");
  ip_debug_print();
  PRINTF("");
  //UIP_STAT(++uip_stat.ip.sent);
  /* Return and let the caller do the actual transmission. */
  //uip_flags = 0;
  return;

  drop:
  uip_clear_buf();
  uip_ext_bitmap = 0;
  //uip_flags = 0;
  
  return;

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
