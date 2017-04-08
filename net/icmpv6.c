#include <xinu.h>

#define UIP_ND6_OPT_HDR_BUF  ((uip_nd6_opt_hdr *)&uip_buf[uip_l2_l3_icmp_hdr_len + nd6_opt_offset])
#define UIP_ND6_OPT_PREFIX_BUF ((uip_nd6_opt_prefix_info *)&uip_buf[uip_l2_l3_icmp_hdr_len + nd6_opt_offset])
#define UIP_ND6_OPT_MTU_BUF ((uip_nd6_opt_mtu *)&uip_buf[uip_l2_l3_icmp_hdr_len + nd6_opt_offset])
#define UIP_ND6_OPT_RDNSS_BUF ((uip_nd6_opt_dns *)&uip_buf[uip_l2_l3_icmp_hdr_len + nd6_opt_offset])

#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP6_ERROR_BUF  ((struct uip_icmp6_error *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_EXT_BUF              ((struct uip_ext_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_FIRST_EXT_BUF        ((struct uip_ext_hdr *)&uip_buf[UIP_LLIPH_LEN])

/** @} */

#if UIP_ND6_SEND_NS || UIP_ND6_SEND_NA || UIP_ND6_SEND_RA || !UIP_CONF_ROUTER
static uint8_t nd6_opt_offset;                     /** Offset from the end of the icmpv6 header to the option in uip_buf*/
static uint8_t *nd6_opt_llao;   /**  Pointer to llao option in uip_buf */
static uip_ds6_nbr_t *nbr; /**  Pointer to a nbr cache entry*/
static uip_ds6_addr_t *addr; /**  Pointer to an interface address */
#endif /* UIP_ND6_SEND_NS || UIP_ND6_SEND_NA || UIP_ND6_SEND_RA || !UIP_CONF_ROUTER */

#if UIP_ND6_SEND_NS || UIP_ND6_SEND_RA || !UIP_CONF_ROUTER
static uip_ds6_defrt_t *defrt; /**  Pointer to a router list entry */
#endif /* UIP_ND6_SEND_NS || UIP_ND6_SEND_RA || !UIP_CONF_ROUTER */

#if !UIP_CONF_ROUTER            // TBD see if we move it to ra_input
static uip_nd6_opt_prefix_info *nd6_opt_prefix_info; /**  Pointer to prefix information option in uip_buf */
static uip_ipaddr_t ipaddr;
#endif
#if (!UIP_CONF_ROUTER || UIP_ND6_SEND_RA)
static uip_ds6_prefix_t *prefix; /**  Pointer to a prefix list entry */
#endif


#define UIP_ND6_RA_BUF            ((uip_nd6_ra *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ND6_NS_BUF            ((uip_nd6_ns *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ND6_NA_BUF            ((uip_nd6_na *)&uip_buf[uip_l2_l3_icmp_hdr_len])

static uip_ds6_prefix_t *prefix; /**  Pointer to a prefix list entry */
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

#if UIP_ND6_SEND_NA || UIP_ND6_SEND_RA || !UIP_CONF_ROUTER
/*------------------------------------------------------------------*/
/* Copy link-layer address from LLAO option to a word-aligned uip_lladdr_t */
static int
extract_lladdr_from_llao_aligned(uip_lladdr_t *dest) {
  if(dest != NULL && nd6_opt_llao != NULL) {
    memcpy(dest, &nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET], UIP_LLADDR_LEN);
    return 1;
  }
  return 0;
}

#endif /* UIP_ND6_SEND_NA || UIP_ND6_SEND_RA || !UIP_CONF_ROUTER */

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

void
uip_icmp6_error_output(uint8_t type, uint8_t code, uint32_t param) {
  /* check if originating packet is not an ICMP error */
  if(uip_ext_len) {
    if(UIP_EXT_BUF->next == UIP_PROTO_ICMP6 && UIP_ICMP_BUF->type < 128) {
      uip_clear_buf();
      return;
    }
  } else {
    if(UIP_IP_BUF->nexthdr == UIP_PROTO_ICMP6 && UIP_ICMP_BUF->type < 128) {
      uip_clear_buf();
      return;
    }
  }

#if UIP_CONF_IPV6_RPL
  rpl_remove_header();
#else
  uip_ext_len = 0;
#endif /* UIP_CONF_IPV6_RPL */

  /* remember data of original packet before shifting */
  uip_ipaddr_copy(&tmp_ipaddr, &UIP_IP_BUF->destipaddr);

  uip_len += UIP_IPICMPH_LEN + UIP_ICMP6_ERROR_LEN;

  if(uip_len > UIP_LINK_MTU) {
    uip_len = UIP_LINK_MTU;
  }

  memmove((uint8_t *)UIP_ICMP6_ERROR_BUF + uip_ext_len + UIP_ICMP6_ERROR_LEN,
          (void *)UIP_IP_BUF, uip_len - UIP_IPICMPH_LEN - uip_ext_len - UIP_ICMP6_ERROR_LEN);

  UIP_IP_BUF->v = 6;
  if (uip_ext_len) {
    UIP_FIRST_EXT_BUF->next = UIP_PROTO_ICMP6;
  } else {
    UIP_IP_BUF->nexthdr = UIP_PROTO_ICMP6;
  }
  UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;

  /* the source should not be unspecified nor multicast, the check for
     multicast is done in uip_process */
  if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr)){
    uip_clear_buf();
    return;
  }

  uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);

  if(uip_is_addr_mcast(&tmp_ipaddr)){
    if(type == ICMP6_PARAM_PROB && code == ICMP6_PARAMPROB_OPTION){
      uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &tmp_ipaddr);
    } else {
      uip_clear_buf();
      return;
    }
  } else {
#if UIP_CONF_ROUTER
    /* need to pick a source that corresponds to this node */
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &tmp_ipaddr);
#else
    uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &tmp_ipaddr);
#endif
  }

  UIP_ICMP_BUF->type = type;
  UIP_ICMP_BUF->icode = code;
  UIP_ICMP6_ERROR_BUF->param = htonl(param);
  UIP_IP_BUF->len = (uip_len - UIP_IPH_LEN);
  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();


  PRINTF("Sending ICMPv6 ERROR message type %d code %d to ", type, code);
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
  if((UIP_IP_BUF->ttl != UIP_ND6_HOP_LIMIT) ||
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
    UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;

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

void
uip_nd6_ns_output(uip_ipaddr_t * src, uip_ipaddr_t * dest, uip_ipaddr_t * tgt)
{
  uip_ext_len = 0;
  UIP_IP_BUF->v = 6;
  UIP_IP_BUF->nexthdr = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;

  if(dest == NULL) {
    uip_create_solicited_node(tgt, &UIP_IP_BUF->destipaddr);
  } else {
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, dest);
  }
  UIP_ICMP_BUF->type = ICMP6_NS;
  UIP_ICMP_BUF->icode = 0;
  UIP_ND6_NS_BUF->reserved = 0;
  uip_ipaddr_copy((uip_ipaddr_t *) &UIP_ND6_NS_BUF->tgtipaddr, tgt);
  UIP_IP_BUF->len = 0;       /* length will not be more than 255 */
  /*
   * check if we add a SLLAO option: for DAD, MUST NOT, for NUD, MAY
   * (here yes), for Address resolution , MUST
   */
  if(!(uip_ds6_is_my_addr(tgt))) {
    if(src != NULL) {
      uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, src);
    } else {
      uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
    }
    if (uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr)) {
      PRINTF("Dropping NS due to no suitable source address\n");
      uip_clear_buf();
      return;
    }
    UIP_IP_BUF->len =
      UIP_ICMPH_LEN + UIP_ND6_NS_LEN + UIP_ND6_OPT_LLAO_LEN;

    create_llao(&uip_buf[uip_l2_l3_icmp_hdr_len + UIP_ND6_NS_LEN],
                UIP_ND6_OPT_SLLAO);

    uip_len =
      UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_NS_LEN + UIP_ND6_OPT_LLAO_LEN;
  } else {
    uip_create_unspecified(&UIP_IP_BUF->srcipaddr);
    UIP_IP_BUF->len = UIP_ICMPH_LEN + UIP_ND6_NS_LEN;
    uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_NS_LEN;
  }

  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

  PRINTF("Sending NS to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" with target address ");
  PRINT6ADDR(tgt);
  PRINTF("\n");
  return;
}


#if !UIP_CONF_ROUTER
/*---------------------------------------------------------------------------*/
void
uip_nd6_rs_output(void)
{
  UIP_IP_BUF->v = 6;
  UIP_IP_BUF->nexthdr = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;
  uip_create_linklocal_allrouters_mcast(&UIP_IP_BUF->destipaddr);
  uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
  UIP_ICMP_BUF->type = ICMP6_RS;
  UIP_ICMP_BUF->icode = 0;
  UIP_IP_BUF->len= 0;       /* length will not be more than 255 */

  if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr)) {
    UIP_IP_BUF->len = UIP_ICMPH_LEN + UIP_ND6_RS_LEN;
    uip_len = uip_l3_icmp_hdr_len + UIP_ND6_RS_LEN;
  } else {
    uip_len = uip_l3_icmp_hdr_len + UIP_ND6_RS_LEN + UIP_ND6_OPT_LLAO_LEN;
    UIP_IP_BUF->len =
      UIP_ICMPH_LEN + UIP_ND6_RS_LEN + UIP_ND6_OPT_LLAO_LEN;

    create_llao(&uip_buf[uip_l2_l3_icmp_hdr_len + UIP_ND6_RS_LEN],
                UIP_ND6_OPT_SLLAO);
  }

  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();
  PRINTF("Sendin RS to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");
  return;
}
/*---------------------------------------------------------------------------*/
/**
 * Process a Router Advertisement
 *
 * - Possible actions when receiving a RA: add router to router list,
 *   recalculate reachable time, update link hop limit, update retrans timer.
 * - If MTU option: update MTU.
 * - If SLLAO option: update entry in neighbor cache
 * - If prefix option: start autoconf, add prefix to prefix list
 */
void
ra_input(void)
{
  uip_lladdr_t lladdr_aligned;

  PRINTF("Received RA from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");

#if UIP_CONF_IPV6_CHECKS
  if((UIP_IP_BUF->ttl != UIP_ND6_HOP_LIMIT) ||
     (!uip_is_addr_linklocal(&UIP_IP_BUF->srcipaddr)) ||
     (UIP_ICMP_BUF->icode != 0)) {
    PRINTF("RA received is bad");
    goto discard;
  }
#endif /*UIP_CONF_IPV6_CHECKS */

  if(UIP_ND6_RA_BUF->cur_ttl != 0) {
    uip_ds6_if.cur_hop_limit = UIP_ND6_RA_BUF->cur_ttl;
    PRINTF("uip_ds6_if.cur_hop_limit %u\n", uip_ds6_if.cur_hop_limit);
  }

  if(UIP_ND6_RA_BUF->reachable_time != 0) {
    if(uip_ds6_if.base_reachable_time !=
       ntohl(UIP_ND6_RA_BUF->reachable_time)) {
      uip_ds6_if.base_reachable_time = ntohl(UIP_ND6_RA_BUF->reachable_time);
      uip_ds6_if.reachable_time = uip_ds6_compute_reachable_time();
    }
  }
  if(UIP_ND6_RA_BUF->retrans_timer != 0) {
    uip_ds6_if.retrans_timer = ntohl(UIP_ND6_RA_BUF->retrans_timer);
  }

  /* Options processing */
  nd6_opt_offset = UIP_ND6_RA_LEN;
  while(uip_l3_icmp_hdr_len + nd6_opt_offset < uip_len) {
    if(UIP_ND6_OPT_HDR_BUF->len == 0) {
      PRINTF("RA received is bad");
      goto discard;
    }
    switch (UIP_ND6_OPT_HDR_BUF->type) {
    case UIP_ND6_OPT_SLLAO:
      PRINTF("Processing SLLAO option in RA\n");
      nd6_opt_llao = (uint8_t *) UIP_ND6_OPT_HDR_BUF;
      nbr = uip_ds6_nbr_lookup(&UIP_IP_BUF->srcipaddr);
      if(!extract_lladdr_from_llao_aligned(&lladdr_aligned)) {
        /* failed to extract llao - discard packet */
        goto discard;
      }
      if(nbr == NULL) {
        nbr = uip_ds6_nbr_add(&UIP_IP_BUF->srcipaddr, &lladdr_aligned,
                              1, NBR_STALE, NBR_TABLE_REASON_IPV6_ND, NULL);
      } else {
        const uip_lladdr_t *lladdr = uip_ds6_nbr_get_ll(nbr);
        if(lladdr == NULL) {
          goto discard;
        }
        if(nbr->state == NBR_INCOMPLETE) {
          nbr->state = NBR_STALE;
        }
        if(memcmp(&nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET],
                  lladdr, UIP_LLADDR_LEN) != 0) {
          /* change of link layer address */
          if(nbr_table_update_lladdr((const linkaddr_t *)lladdr, (const linkaddr_t *)&lladdr_aligned, 1) == 0) {
            /* failed to update the lladdr */
            goto discard;
          }
          nbr->state = NBR_STALE;
        }
        nbr->isrouter = 1;
      }
      break;
    case UIP_ND6_OPT_MTU:
      PRINTF("Processing MTU option in RA\n");
      uip_ds6_if.link_mtu =
        ntohl(((uip_nd6_opt_mtu *) UIP_ND6_OPT_HDR_BUF)->mtu);
      break;
    case UIP_ND6_OPT_PREFIX_INFO:
      PRINTF("Processing PREFIX option in RA\n");
      nd6_opt_prefix_info = (uip_nd6_opt_prefix_info *) UIP_ND6_OPT_HDR_BUF;
      if((ntohl(nd6_opt_prefix_info->validlt) >=
          ntohl(nd6_opt_prefix_info->preferredlt))
         && (!uip_is_addr_linklocal(&nd6_opt_prefix_info->prefix))) {
        /* on-link flag related processing */
        if(nd6_opt_prefix_info->flagsreserved1 & UIP_ND6_RA_FLAG_ONLINK) {
          prefix =
            uip_ds6_prefix_lookup(&nd6_opt_prefix_info->prefix,
                                  nd6_opt_prefix_info->preflen);
          if(prefix == NULL) {
            if(nd6_opt_prefix_info->validlt != 0) {
              if(nd6_opt_prefix_info->validlt != UIP_ND6_INFINITE_LIFETIME) {
                prefix = uip_ds6_prefix_add(&nd6_opt_prefix_info->prefix,
                                            nd6_opt_prefix_info->preflen,
                                            ntohl(nd6_opt_prefix_info->
                                                  validlt));
              } else {
                prefix = uip_ds6_prefix_add(&nd6_opt_prefix_info->prefix,
                                            nd6_opt_prefix_info->preflen, 0);
              }
            }
          } else {
            switch (nd6_opt_prefix_info->validlt) {
            case 0:
              uip_ds6_prefix_rm(prefix);
              break;
            case UIP_ND6_INFINITE_LIFETIME:
              prefix->isinfinite = 1;
              break;
            default:
              PRINTF("Updating timer of prefix ");
              PRINT6ADDR(&prefix->ipaddr);
              PRINTF(" new value %lu\n", ntohl(nd6_opt_prefix_info->validlt));
              //stimer_set(&prefix->vlifetime,
              //           uip_ntohl(nd6_opt_prefix_info->validlt));
              prefix->isinfinite = 0;
              break;
            }
          }
        }
        /* End of on-link flag related processing */
        /* autonomous flag related processing */
        if((nd6_opt_prefix_info->flagsreserved1 & UIP_ND6_RA_FLAG_AUTONOMOUS)
           && (nd6_opt_prefix_info->validlt != 0)
           && (nd6_opt_prefix_info->preflen == UIP_DEFAULT_PREFIX_LEN)) {

          uip_ipaddr_copy(&ipaddr, &nd6_opt_prefix_info->prefix);
          uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
          addr = uip_ds6_addr_lookup(&ipaddr);
          if((addr != NULL) && (addr->type == ADDR_AUTOCONF)) {
            if(nd6_opt_prefix_info->validlt != UIP_ND6_INFINITE_LIFETIME) {
              /* The processing below is defined in RFC4862 section 5.5.3 e */
              
              // goes in below if condition
             // || (ntohl(nd6_opt_prefix_info->validlt) >
              //    stimer_remaining(&addr->vlifetime))

              if((ntohl(nd6_opt_prefix_info->validlt) > 2 * 60 * 60) ) {
                PRINTF("Updating timer of address ");
                PRINT6ADDR(&addr->ipaddr);
                PRINTF(" new value %lu\n",
                       ntohl(nd6_opt_prefix_info->validlt));
                //stimer_set(&addr->vlifetime,
                //           uip_ntohl(nd6_opt_prefix_info->validlt));
              } else {
                //stimer_set(&addr->vlifetime, 2 * 60 * 60);
                PRINTF("Updating timer of address ");
                PRINT6ADDR(&addr->ipaddr);
                PRINTF(" new value %lu\n", (unsigned long)(2 * 60 * 60));
              }
              addr->isinfinite = 0;
            } else {
              addr->isinfinite = 1;
            }
          } else {
            if(ntohl(nd6_opt_prefix_info->validlt) ==
               UIP_ND6_INFINITE_LIFETIME) {
              uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
            } else {
              uip_ds6_addr_add(&ipaddr, ntohl(nd6_opt_prefix_info->validlt),
                               ADDR_AUTOCONF);
            }
          }
        }
        /* End of autonomous flag related processing */
      }
      break;
#if UIP_ND6_RA_RDNSS
    case UIP_ND6_OPT_RDNSS:
      if(UIP_ND6_RA_BUF->flags_reserved & (UIP_ND6_O_FLAG << 6)) {
        PRINTF("Processing RDNSS option\n");
        uint8_t naddr = (UIP_ND6_OPT_RDNSS_BUF->len - 1) / 2;
        uip_ipaddr_t *ip = (uip_ipaddr_t *)(&UIP_ND6_OPT_RDNSS_BUF->ip);
        PRINTF("got %d nameservers\n", naddr);
        while(naddr-- > 0) {
          PRINTF(" nameserver: ");
          PRINT6ADDR(ip);
          PRINTF(" lifetime: %lx\n", ntohl(UIP_ND6_OPT_RDNSS_BUF->lifetime));
          uip_nameserver_update(ip, ntohl(UIP_ND6_OPT_RDNSS_BUF->lifetime));
          ip++;
        }
      }
      break;
#endif /* UIP_ND6_RA_RDNSS */
    default:
      PRINTF("ND option not supported in RA");
      break;
    }
    nd6_opt_offset += (UIP_ND6_OPT_HDR_BUF->len << 3);
  }

  defrt = uip_ds6_defrt_lookup(&UIP_IP_BUF->srcipaddr);
  if(UIP_ND6_RA_BUF->router_lifetime != 0) {
    if(nbr != NULL) {
      nbr->isrouter = 1;
    }
    if(defrt == NULL) {
      uip_ds6_defrt_add(&UIP_IP_BUF->srcipaddr,
                        (unsigned
                         long)(ntohs(UIP_ND6_RA_BUF->router_lifetime)));
    } else {
      //stimer_set(&(defrt->lifetime),
       //          (unsigned long)(uip_ntohs(UIP_ND6_RA_BUF->router_lifetime)));
    }
  } else {
    if(defrt != NULL) {
      uip_ds6_defrt_rm(defrt);
    }
  }

#if UIP_CONF_IPV6_QUEUE_PKT
  /* If the nbr just became reachable (e.g. it was in NBR_INCOMPLETE state
   * and we got a SLLAO), check if we had buffered a pkt for it */
  /*  if((nbr != NULL) && (nbr->queue_buf_len != 0)) {
    uip_len = nbr->queue_buf_len;
    memcpy(UIP_IP_BUF, nbr->queue_buf, uip_len);
    nbr->queue_buf_len = 0;
    return;
    }*/
  if(nbr != NULL && uip_packetqueue_buflen(&nbr->packethandle) != 0) {
    uip_len = uip_packetqueue_buflen(&nbr->packethandle);
    memcpy(UIP_IP_BUF, uip_packetqueue_buf(&nbr->packethandle), uip_len);
    uip_packetqueue_free(&nbr->packethandle);
    return;
  }

#endif /*UIP_CONF_IPV6_QUEUE_PKT */

discard:
  uip_clear_buf();
  return;
}
#endif /* !UIP_CONF_ROUTER */

#if UIP_CONF_ROUTER
#if UIP_ND6_SEND_RA
/*---------------------------------------------------------------------------*/
static void
rs_input(void)
{

  PRINTF("Received RS from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");


#if UIP_CONF_IPV6_CHECKS
  /*
   * Check hop limit / icmp code
   * target address must not be multicast
   * if the NA is solicited, dest must not be multicast
   */
  if((UIP_IP_BUF->ttl != UIP_ND6_HOP_LIMIT) || (UIP_ICMP_BUF->icode != 0)) {
    PRINTF("RS received is bad\n");
    goto discard;
  }
#endif /*UIP_CONF_IPV6_CHECKS */

  /* Only valid option is Source Link-Layer Address option any thing
     else is discarded */
  nd6_opt_offset = UIP_ND6_RS_LEN;
  nd6_opt_llao = NULL;

  while(uip_l3_icmp_hdr_len + nd6_opt_offset < uip_len) {
#if UIP_CONF_IPV6_CHECKS
    if(UIP_ND6_OPT_HDR_BUF->len == 0) {
      PRINTF("RS received is bad\n");
      goto discard;
    }
#endif /*UIP_CONF_IPV6_CHECKS */
    switch (UIP_ND6_OPT_HDR_BUF->type) {
    case UIP_ND6_OPT_SLLAO:
      nd6_opt_llao = (uint8_t *)UIP_ND6_OPT_HDR_BUF;
      break;
    default:
      PRINTF("ND option not supported in RS\n");
      break;
    }
    nd6_opt_offset += (UIP_ND6_OPT_HDR_BUF->len << 3);
  }
  /* Options processing: only SLLAO */
  if(nd6_opt_llao != NULL) {
#if UIP_CONF_IPV6_CHECKS
    if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr)) {
      PRINTF("RS received is bad\n");
      goto discard;
    } else {
#endif /*UIP_CONF_IPV6_CHECKS */
      uip_lladdr_t lladdr_aligned;
      extract_lladdr_from_llao_aligned(&lladdr_aligned);
      if((nbr = uip_ds6_nbr_lookup(&UIP_IP_BUF->srcipaddr)) == NULL) {
        /* we need to add the neighbor */
        uip_ds6_nbr_add(&UIP_IP_BUF->srcipaddr, &lladdr_aligned,
                        0, NBR_STALE, NBR_TABLE_REASON_IPV6_ND, NULL);
      } else {
        /* If LL address changed, set neighbor state to stale */
        const uip_lladdr_t *lladdr = uip_ds6_nbr_get_ll(nbr);
        if(lladdr == NULL) {
          goto discard;
        }
        if(memcmp(&nd6_opt_llao[UIP_ND6_OPT_DATA_OFFSET],
            lladdr, UIP_LLADDR_LEN) != 0) {
          uip_ds6_nbr_t nbr_data;
          nbr_data = *nbr;
          uip_ds6_nbr_rm(nbr);
          nbr = uip_ds6_nbr_add(&UIP_IP_BUF->srcipaddr, &lladdr_aligned,
                                0, NBR_STALE, NBR_TABLE_REASON_IPV6_ND, NULL);
          nbr->reachable = nbr_data.reachable;
          nbr->sendns = nbr_data.sendns;
          nbr->nscount = nbr_data.nscount;
        }
        nbr->isrouter = 0;
      }
#if UIP_CONF_IPV6_CHECKS
    }
#endif /*UIP_CONF_IPV6_CHECKS */
  }

  /* Schedule a sollicited RA */
  uip_nd6_ra_output(NULL);

discard:
  uip_clear_buf();
  return;
}

/*---------------------------------------------------------------------------*/
void
uip_nd6_ra_output(uip_ipaddr_t * dest)
{

  UIP_IP_BUF->v = 6;
  UIP_IP_BUF->nexthdr = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = UIP_ND6_HOP_LIMIT;

  if(dest == NULL) {
    uip_create_linklocal_allnodes_mcast(&UIP_IP_BUF->destipaddr);
  } else {
    /* For sollicited RA */
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, dest);
  }
  uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);

  UIP_ICMP_BUF->type = ICMP6_RA;
  UIP_ICMP_BUF->icode = 0;

  UIP_ND6_RA_BUF->cur_ttl = uip_ds6_if.cur_hop_limit;

  UIP_ND6_RA_BUF->flags_reserved =
    (UIP_ND6_M_FLAG << 7) | (UIP_ND6_O_FLAG << 6);

  UIP_ND6_RA_BUF->router_lifetime = htons(UIP_ND6_ROUTER_LIFETIME);
  //UIP_ND6_RA_BUF->reachable_time = uip_htonl(uip_ds6_if.reachable_time);
  //UIP_ND6_RA_BUF->retrans_timer = uip_htonl(uip_ds6_if.retrans_timer);
  UIP_ND6_RA_BUF->reachable_time = 0;
  UIP_ND6_RA_BUF->retrans_timer = 0;

  uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + UIP_ND6_RA_LEN;
  nd6_opt_offset = UIP_ND6_RA_LEN;


  /* Prefix list */
  for(prefix = uip_ds6_prefix_list;
      prefix < uip_ds6_prefix_list + UIP_DS6_PREFIX_NB; prefix++) {
    if((prefix->isused) && (prefix->advertise)) {
      UIP_ND6_OPT_PREFIX_BUF->type = UIP_ND6_OPT_PREFIX_INFO;
      UIP_ND6_OPT_PREFIX_BUF->len = UIP_ND6_OPT_PREFIX_INFO_LEN / 8;
      UIP_ND6_OPT_PREFIX_BUF->preflen = prefix->length;
      UIP_ND6_OPT_PREFIX_BUF->flagsreserved1 = prefix->l_a_reserved;
      UIP_ND6_OPT_PREFIX_BUF->validlt = htonl(prefix->vlifetime);
      UIP_ND6_OPT_PREFIX_BUF->preferredlt = htonl(prefix->plifetime);
      UIP_ND6_OPT_PREFIX_BUF->reserved2 = 0;
      uip_ipaddr_copy(&(UIP_ND6_OPT_PREFIX_BUF->prefix), &(prefix->ipaddr));
      nd6_opt_offset += UIP_ND6_OPT_PREFIX_INFO_LEN;
      uip_len += UIP_ND6_OPT_PREFIX_INFO_LEN;
    }
  }

  /* Source link-layer option */
  create_llao((uint8_t *)UIP_ND6_OPT_HDR_BUF, UIP_ND6_OPT_SLLAO);

  uip_len += UIP_ND6_OPT_LLAO_LEN;
  nd6_opt_offset += UIP_ND6_OPT_LLAO_LEN;

  /* MTU */
  UIP_ND6_OPT_MTU_BUF->type = UIP_ND6_OPT_MTU;
  UIP_ND6_OPT_MTU_BUF->len = UIP_ND6_OPT_MTU_LEN >> 3;
  UIP_ND6_OPT_MTU_BUF->reserved = 0;
  //UIP_ND6_OPT_MTU_BUF->mtu = uip_htonl(uip_ds6_if.link_mtu);
  UIP_ND6_OPT_MTU_BUF->mtu = htonl(1500);

  uip_len += UIP_ND6_OPT_MTU_LEN;
  nd6_opt_offset += UIP_ND6_OPT_MTU_LEN;

#if UIP_ND6_RA_RDNSS
  if(uip_nameserver_count() > 0) {
    uint8_t i = 0;
    uip_ipaddr_t *ip = &UIP_ND6_OPT_RDNSS_BUF->ip;
    uip_ipaddr_t *dns = NULL;
    UIP_ND6_OPT_RDNSS_BUF->type = UIP_ND6_OPT_RDNSS;
    UIP_ND6_OPT_RDNSS_BUF->reserved = 0;
    UIP_ND6_OPT_RDNSS_BUF->lifetime = uip_nameserver_next_expiration();
    if(UIP_ND6_OPT_RDNSS_BUF->lifetime != UIP_NAMESERVER_INFINITE_LIFETIME) {
      UIP_ND6_OPT_RDNSS_BUF->lifetime -= clock_seconds();
    }
    while((dns = uip_nameserver_get(i)) != NULL) {
      uip_ipaddr_copy(ip++, dns);
      i++;
    }
    UIP_ND6_OPT_RDNSS_BUF->len = UIP_ND6_OPT_RDNSS_LEN + (i << 1);
    PRINTF("%d nameservers reported\n", i);
    uip_len += UIP_ND6_OPT_RDNSS_BUF->len << 3;
    nd6_opt_offset += UIP_ND6_OPT_RDNSS_BUF->len << 3;
  }
#endif /* UIP_ND6_RA_RDNSS */

  UIP_IP_BUF->len = (uip_len - UIP_IPH_LEN);
  /*ICMP checksum */
  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

  PRINTF("Sending RA to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");
  return;
}
#endif /* UIP_ND6_SEND_RA */
#endif /* UIP_CONF_ROUTER */



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
      #if UIP_CONF_ROUTER
        rs_input();
      #endif
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