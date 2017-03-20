#include <xinu.h>


void ipv6_in(unsigned char * packet)
{
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
  if(uip_icmp6chksum() != 0xffff) {
       PRINTF("icmpv6: bad checksum.\n");
       return;
  }
  PRINTF("icmpv6 good.\n");

  ip_hton();
}

static uint16_t
chksum(uint16_t sum, const uint8_t *data, uint16_t len)
{
  uint16_t t;
  const uint8_t *dataptr;
  const uint8_t *last_byte;

  dataptr = data;
  last_byte = data + len - 1;

  while(dataptr < last_byte) {   /* At least two more bytes */
    t = (dataptr[0] << 8) + dataptr[1];
    sum += t;
    if(sum < t) {
      sum++;      /* carry */
    }
    dataptr += 2;
  }

  if(dataptr == last_byte) {
    t = (dataptr[0] << 8) + 0;
    sum += t;
    if(sum < t) {
      sum++;      /* carry */
    }
  }

  /* Return sum in host byte order. */
  return sum;
}

static uint16_t
upper_layer_chksum(uint8_t proto)
{
  volatile uint16_t upper_layer_len;
  uint16_t sum;

  upper_layer_len = (((uint16_t)(UIP_IP_BUF->len)) - uip_ext_len);

  PRINTF("Upper layer checksum len: %d from: %d\n", upper_layer_len,
         UIP_IPH_LEN + UIP_LLH_LEN + uip_ext_len);

  /* First sum pseudoheader. */
  /* IP protocol and length fields. This addition cannot carry. */
  sum = upper_layer_len + proto;
  /* Sum IP source and destination addresses. */
  sum = chksum(sum, (uint8_t *)&UIP_IP_BUF->srcipaddr, 2 * sizeof(uip_ipaddr_t));

  /* Sum TCP header and data. */
  sum = chksum(sum, &uip_buf[UIP_IPH_LEN + UIP_LLH_LEN + uip_ext_len],
               upper_layer_len);

  return (sum == 0) ? 0xffff : htons(sum);
}

/*---------------------------------------------------------------------------*/
uint16_t
uip_icmp6chksum(void)
{
  return upper_layer_chksum(UIP_PROTO_ICMP6);
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
	int a =0 ;
	// for(a =0; a< 8; a++)
	// {
	// 	UIP_IP_BUF->srcipaddr.u16[a] =  UIP_NTOHS(UIP_IP_BUF->srcipaddr.u16[a]);
	// }
	// a =0 ;
	// for(a =0; a< 8; a++)
	// {
	// 	UIP_IP_BUF->destipaddr.u16[a] =  UIP_NTOHS(UIP_IP_BUF->destipaddr.u16[a]);
	// }
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
	int a =0 ;
	for(a =0; a< 8; a++)
	{
		UIP_IP_BUF->srcipaddr.u16[a] =  UIP_HTONS(UIP_IP_BUF->srcipaddr.u16[a]);
	}
	a =0 ;
	for(a =0; a< 8; a++)
	{
		UIP_IP_BUF->destipaddr.u16[a] =  UIP_HTONS(UIP_IP_BUF->destipaddr.u16[a]);
	}
}
