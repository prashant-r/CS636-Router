#include <xinu.h>

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

void icmp6_input(char * packet)
{

  /* This is IPv6 ICMPv6 processing code. */
  PRINTF("icmp6_input: length %d type: %d \n", uip_len, UIP_ICMP_BUF->type);

  #if UIP_CONF_IPV6_CHECKS
     if(uip_icmp6chksum() != 0xffff) {
       PRINTF("icmpv6: bad checksum.\n");
       goto drop;
    }
    PRINTF("icmpv6 checksum good.\n");   
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
      PRINTF("Its a request\n");
      echo_request_input();
      break;
    case ICMP6_ECHO_REPLY:
      PRINTF("Its a reply \n");
      break;
    case ICMP6_NS:
      PRINTF("It's a NS \n");
      ns_input();
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

static void
create_llao(uint8_t *llao, uint8_t type) {

  control(ETHER0, ETH_CTRL_GET_MAC, (int32) &uip_lladdr, 0);
  llao[UIP_ND6_OPT_TYPE_OFFSET] = type;
  llao[UIP_ND6_OPT_LEN_OFFSET] = UIP_ND6_OPT_LLAO_LEN >> 3;
  memcpy(&llao[UIP_ND6_OPT_DATA_OFFSET], &uip_lladdr, UIP_LLADDR_LEN);
  /* padding on some */
  memset(&llao[UIP_ND6_OPT_DATA_OFFSET + UIP_LLADDR_LEN], 0,
         UIP_ND6_OPT_LLAO_LEN - 2 - UIP_LLADDR_LEN);
}

static void *memmove(void *v_dst, const void *v_src, int32_t c)
{
  const char *src = v_src;
  char *dst = v_dst;

  if (!c)
    return v_dst;

  /* Use memcpy when source is higher than dest */
  if (v_dst <= v_src)
    return memcpy(v_dst, v_src, c);

  /* copy backwards, from end to beginning */
  src += c;
  dst += c;

  /* Simple, byte oriented memmove. */
  while (c--)
    *--dst = *--src;

  return v_dst;
}

void echo_request_input(void)
{
  /*
   * we send an echo reply. It is trivial if there was no extension
   * headers in the request otherwise we need to remove the extension
   * headers and change a few fields
   */
  PRINTF("Received Echo Request from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");

  /* IP header */

  if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)){
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);
    uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &tmp_ipaddr);
  } else {
    uip_ipaddr_copy(&tmp_ipaddr, &UIP_IP_BUF->srcipaddr);
    uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &tmp_ipaddr);
  }

  if(uip_ext_len > 0) {
    /* Remove extension headers if any */
    UIP_IP_BUF->nexthdr = UIP_PROTO_ICMP6;
    uip_len -= uip_ext_len;
    UIP_IP_BUF->len = ((uip_len - UIP_IPH_LEN));
    /* move the echo request payload (starting after the icmp header)
     * to the new location in the reply.
     * The shift is equal to the length of the extension headers present
     * Note: UIP_ICMP_BUF still points to the echo request at this stage
     */
    memmove((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN - uip_ext_len,
        (uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN,
        (uip_len - UIP_IPH_LEN - UIP_ICMPH_LEN));
    uip_ext_len = 0;
  }

  /* Below is important for the correctness of UIP_ICMP_BUF and the
   * checksum
   */

  /* Note: now UIP_ICMP_BUF points to the beginning of the echo reply */
  UIP_ICMP_BUF->type = ICMP6_ECHO_REPLY;
  UIP_ICMP_BUF->icode = 0;
  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

  PRINTF("Sending Echo Reply to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");
  return;
}

void ns_input()
{
  uint8_t flags;
  PRINTF("Received NS from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" with target address ");
  PRINT6ADDR((uip_ipaddr_t *) (&UIP_ND6_NS_BUF->tgtipaddr));
  uip_ipaddr_copy(&tmp_ipaddr, (&UIP_ND6_NS_BUF->tgtipaddr));
  PRINTF("\n");
  #if UIP_CONF_IPV6_CHECKS
  if((UIP_IP_BUF->hoplim != UIP_ND6_HOP_LIMIT) ||
     (uip_is_addr_mcast(&UIP_ND6_NS_BUF->tgtipaddr)) ||
     (UIP_ICMP_BUF->icode != 0)) {
    PRINTF("NS received is bad\n");
    goto discard;
  }
  #endif /* UIP_CONF_IPV6_CHECKS */

  if(uip_is_addr_solicited_node(&UIP_IP_BUF->destipaddr)) {
      uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);
      uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &UIP_ND6_NS_BUF->tgtipaddr);
      flags = UIP_ND6_NA_FLAG_SOLICITED | UIP_ND6_NA_FLAG_OVERRIDE;
      goto create_na;
  }

  create_na:
     /* If the node is a router it should set R flag in NAs */
    #if UIP_CONF_ROUTER
      flags = flags | UIP_ND6_NA_FLAG_ROUTER;
    #endif
    uip_ext_len = 0;
    UIP_IP_BUF->len = UIP_ICMPH_LEN + UIP_ND6_NA_LEN + UIP_ND6_OPT_LLAO_LEN;
    UIP_IP_BUF->nexthdr = UIP_PROTO_ICMP6;
    UIP_IP_BUF->hoplim = UIP_ND6_HOP_LIMIT;

    UIP_ICMP_BUF->type = ICMP6_NA;
    UIP_ICMP_BUF->icode = 0;

    UIP_ND6_NA_BUF->flagsreserved = flags;
    //memcpy(&UIP_ND6_NA_BUF->tgtipaddr, &addr->ipaddr, sizeof(uip_ipaddr_t));

    create_llao(&uip_buf[uip_l2_l3_icmp_hdr_len + UIP_ND6_NA_LEN],
             UIP_ND6_OPT_TLLAO);

    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

    uip_len =
      UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_NA_LEN + UIP_ND6_OPT_LLAO_LEN;

    PRINTF("Sending NA to ");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    PRINTF(" from ");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF(" with target address ");
    PRINT6ADDR(&UIP_ND6_NA_BUF->tgtipaddr);
    PRINTF("\n");
    return;

  discard:
      uip_clear_buf();
      return;
}

