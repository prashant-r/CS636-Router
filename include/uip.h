#ifndef UIP_H_
#define UIP_H_
#define DEBUG 1
#if DEBUG
#define PRINTF(...) kprintf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",lladdr->addr[0], lladdr->addr[1], lladdr->addr[2], lladdr->addr[3],lladdr->addr[4], lladdr->addr[5])
#define PRINTLLADDRDOT(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",lladdr.addr[0], lladdr.addr[1], lladdr.addr[2], lladdr.addr[3],lladdr.addr[4], lladdr.addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#endif
#define NETSTACK_CONF_WITH_IPV6 1
#if NETSTACK_CONF_WITH_IPV6
  #define UIP_IPH_LEN    40
  #define UIP_FRAGH_LEN  8 
#else /* NETSTACK_CONF_WITH_IPV6 */
  #define UIP_IPH_LEN    20    /* Size of IP header */
  #endif /* NETSTACK_CONF_WITH_IPV6 */
#define UIP_LLH_LEN     14
#define UIP_UDPH_LEN    8    /* Size of UDP header */
#define UIP_TCPH_LEN   20    /* Size of TCP header */
#define UIP_ICMPH_LEN   4    /* Size of ICMP header */
#define UIP_TTL         64
#define UIP_LINK_MTU   1280
#define UIP_BUFSIZE (UIP_LINK_MTU + UIP_LLH_LEN)
#define UIP_IPUDPH_LEN (UIP_UDPH_LEN + UIP_IPH_LEN)   /* Size of IP + UDP header */
#define UIP_IPTCPH_LEN (UIP_TCPH_LEN + UIP_IPH_LEN)   /* Size of IP + TCP header */
#define UIP_TCPIP_HLEN UIP_IPTCPH_LEN
#define UIP_IPICMPH_LEN (UIP_IPH_LEN + UIP_ICMPH_LEN) /* Size of ICMP + IP header */
#define UIP_LLIPH_LEN (UIP_LLH_LEN + UIP_IPH_LEN)     /* Size of L2 + IP header */
#define uip_l2_l3_hdr_len (UIP_LLH_LEN + UIP_IPH_LEN + uip_ext_len)
#define uip_l2_l3_icmp_hdr_len (UIP_LLH_LEN + UIP_IPH_LEN + uip_ext_len + UIP_ICMPH_LEN)
#define uip_l3_hdr_len (UIP_IPH_LEN + uip_ext_len)
#define uip_l3_icmp_hdr_len (UIP_IPH_LEN + uip_ext_len + UIP_ICMPH_LEN)

#ifndef UIP_LITTLE_ENDIAN
#define UIP_LITTLE_ENDIAN  3412
#endif /* UIP_LITTLE_ENDIAN */
#ifndef UIP_BIG_ENDIAN
#define UIP_BIG_ENDIAN     1234
#endif /* UIP_BIG_ENDIAN */
#define UIP_BYTE_ORDER     (UIP_LITTLE_ENDIAN)

#define UIP_ETHTYPE_IPV6 0x86dd
#ifndef UIP_HTONS
#   if UIP_BYTE_ORDER == UIP_BIG_ENDIAN
#      define UIP_HTONS(n) (n)
#      define UIP_HTONL(n) (n)
#   else /* UIP_BYTE_ORDER == UIP_BIG_ENDIAN */
#      define UIP_HTONS(n) (uint16_t)((((uint16_t) (n)) << 8) | (((uint16_t) (n)) >> 8))
#      define UIP_HTONL(n) (((uint32_t)UIP_HTONS(n) << 16) | UIP_HTONS((uint32_t)(n) >> 16))
#   endif /* UIP_BYTE_ORDER == UIP_BIG_ENDIAN */
#else
#error "UIP_HTONS already defined!"
#endif /* UIP_HTONS 



extern unsigned char *uip_buf;
extern uint16_t uip_len;
*/

#define SIZE_ETHERNET 14
uint8_t *uip_next_hdr;
unsigned char * uip_buf;
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define UIP_LLADDR_LEN 6
typedef struct uip_80211_addr {
   uint8_t addr[6];
} uip_80211_addr;

typedef uip_80211_addr uip_lladdr_t;
uip_lladdr_t uip_lladdr;


typedef union uip_ip4addr_t {
  uint8_t  u8[4];                       /* Initializer, must come first. */
  uint16_t u16[2];
} uip_ip4addr_t;

typedef union uip_ip6addr_t {
  uint8_t  u8[16];                      /* Initializer, must come first. */
  uint16_t u16[8];
} uip_ip6addr_t;

#if NETSTACK_CONF_WITH_IPV6
typedef uip_ip6addr_t uip_ipaddr_t;
#else /* NETSTACK_CONF_WITH_IPV6 */
typedef uip_ip4addr_t uip_ipaddr_t;
#endif /* NETSTACK_CONF_WITH_IPV6 */
static uip_ipaddr_t tmp_ipaddr;
/* The TCP and IP headers. */
struct uip_tcpip_hdr {
#if NETSTACK_CONF_WITH_IPV6
  /* IPv6 header. */
  uint8_t vtc,
    tcflow;
  uint16_t flow;
  uint8_t len[2];
  uint8_t proto, ttl;
  uip_ip6addr_t srcipaddr, destipaddr;
#else /* NETSTACK_CONF_WITH_IPV6 */
  /* IPv4 header. */
  uint8_t vhl,
    tos,
    len[2],
    ipid[2],
    ipoffset[2],
    ttl,
    proto;
  uint16_t ipchksum;
  uip_ipaddr_t srcipaddr, destipaddr;
#endif /* NETSTACK_CONF_WITH_IPV6 */

  /* TCP header. */
  uint16_t srcport,
    destport;
  uint8_t seqno[4],
    ackno[4],
    tcpoffset,
    flags,
    wnd[2];
  uint16_t tcpchksum;
  uint8_t urgp[2];
  uint8_t optdata[4];
};

/* The ICMP and IP headers. */
struct uip_icmpip_hdr {
#if NETSTACK_CONF_WITH_IPV6
  /* IPv6 header. */
  uint8_t vtc,
    tcf;
  uint16_t flow;
  uint8_t len[2];
  uint8_t proto, ttl;
  uip_ip6addr_t srcipaddr, destipaddr;
#else /* NETSTACK_CONF_WITH_IPV6 */
  /* IPv4 header. */
  uint8_t vhl,
    tos,
    len[2],
    ipid[2],
    ipoffset[2],
    ttl,
    proto;
  uint16_t ipchksum;
  uip_ipaddr_t srcipaddr, destipaddr;
#endif /* NETSTACK_CONF_WITH_IPV6 */

  /* ICMP header. */
  uint8_t type, icode;
  uint16_t icmpchksum;
#if !NETSTACK_CONF_WITH_IPV6
  uint16_t id, seqno;
  uint8_t payload[1];
#endif /* !NETSTACK_CONF_WITH_IPV6 */
};


/* The UDP and IP headers. */
struct uip_udpip_hdr {
#if NETSTACK_CONF_WITH_IPV6
  /* IPv6 header. */
  uint8_t vtc,
    tcf;
  uint16_t flow;
  uint8_t len[2];
  uint8_t proto, ttl;
  uip_ip6addr_t srcipaddr, destipaddr;
#else /* NETSTACK_CONF_WITH_IPV6 */
  /* IP header. */
  uint8_t vhl,
    tos,
    len[2],
    ipid[2],
    ipoffset[2],
    ttl,
    proto;
  uint16_t ipchksum;
  uip_ipaddr_t srcipaddr, destipaddr;
#endif /* NETSTACK_CONF_WITH_IPV6 */

  /* UDP header. */
  uint16_t srcport,
    destport;
  uint16_t udplen;
  uint16_t udpchksum;
};

/* The IP header */
struct uip_ip_hdr {
#if NETSTACK_CONF_WITH_IPV6
  /* IPV6 header */
  uint8_t vtc;
  uint8_t tcflow;
  uint16_t flow;
  uint8_t len[2];
  uint8_t proto, ttl;
  uip_ip6addr_t srcipaddr, destipaddr;
#else /* NETSTACK_CONF_WITH_IPV6 */
  /* IPV4 header */
  uint8_t vhl,
    tos,
    len[2],
    ipid[2],
    ipoffset[2],
    ttl,
    proto;
  uint16_t ipchksum;
  uip_ipaddr_t srcipaddr, destipaddr;
#endif /* NETSTACK_CONF_WITH_IPV6 */
};

typedef struct uip_ext_hdr {
  uint8_t next;
  uint8_t len;
} uip_ext_hdr;

/* Hop by Hop option header */
typedef struct uip_hbho_hdr {
  uint8_t next;
  uint8_t len;
} uip_hbho_hdr;

/* destination option header */
typedef struct uip_desto_hdr {
  uint8_t next;
  uint8_t len;
} uip_desto_hdr;

/* We do not define structures for PAD1 and PADN options */

/*
 * routing header
 * the routing header as 4 common bytes, then routing header type
 * specific data there are several types of routing header. Type 0 was
 * deprecated as per RFC5095 most notable other type is 2, used in
 * RFC3775 (MIPv6) here we do not implement MIPv6, so we just need to
 * parse the 4 first bytes
 */
typedef struct uip_routing_hdr {
  uint8_t next;
  uint8_t len;
  uint8_t routing_type;
  uint8_t seg_left;
} uip_routing_hdr;

/* RPL Source Routing Header */
typedef struct uip_rpl_srh_hdr {
  uint8_t cmpr; /* CmprI and CmprE */
  uint8_t pad;
  uint8_t reserved[2];
} uip_rpl_srh_hdr;

/* fragmentation header */
typedef struct uip_frag_hdr {
  uint8_t next;
  uint8_t res;
  uint16_t offsetresmore;
  uint32_t id;
} uip_frag_hdr;

/*
 * an option within the destination or hop by hop option headers
 * it contains type an length, which is true for all options but PAD1
 */
typedef struct uip_ext_hdr_opt {
  uint8_t type;
  uint8_t len;
} uip_ext_hdr_opt;

/* PADN option */
typedef struct uip_ext_hdr_opt_padn {
  uint8_t opt_type;
  uint8_t opt_len;
} uip_ext_hdr_opt_padn;

/* RPL option */
typedef struct uip_ext_hdr_opt_rpl {
  uint8_t opt_type;
  uint8_t opt_len;
  uint8_t flags;
  uint8_t instance;
  uint16_t senderrank;
} uip_ext_hdr_opt_rpl;

/* TCP header */
struct uip_tcp_hdr {
  uint16_t srcport;
  uint16_t destport;
  uint8_t seqno[4];
  uint8_t ackno[4];
  uint8_t tcpoffset;
  uint8_t flags;
  uint8_t  wnd[2];
  uint16_t tcpchksum;
  uint8_t urgp[2];
  uint8_t optdata[4];
};

/* The ICMP headers. */
struct uip_icmp_hdr {
  uint8_t type, icode;
  uint16_t icmpchksum;
#if !NETSTACK_CONF_WITH_IPV6
  uint16_t id, seqno;
#endif /* !NETSTACK_CONF_WITH_IPV6 */
};


/* The UDP headers. */
struct uip_udp_hdr {
  uint16_t srcport;
  uint16_t destport;
  uint16_t udplen;
  uint16_t udpchksum;
};

#define UIP_PROTO_ICMP  1
#define UIP_PROTO_TCP   6
#define UIP_PROTO_UDP   17
#define UIP_PROTO_ICMP6 58

#define UIP_IP_BUF                          ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF                      ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_UDP_BUF                        ((struct uip_udp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])
#define UIP_TCP_BUF                        ((struct uip_tcp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])
#define UIP_EXT_BUF                        ((struct uip_ext_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ROUTING_BUF                ((struct uip_routing_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_FRAG_BUF                      ((struct uip_frag_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_HBHO_BUF                      ((struct uip_hbho_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_DESTO_BUF                    ((struct uip_desto_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_EXT_HDR_OPT_BUF            ((struct uip_ext_hdr_opt *)&uip_buf[uip_l2_l3_hdr_len + uip_ext_opt_offset])
#define UIP_EXT_HDR_OPT_PADN_BUF  ((struct uip_ext_hdr_opt_padn *)&uip_buf[uip_l2_l3_hdr_len + uip_ext_opt_offset])

extern uint16_t chksum(uint16_t sum, const uint8_t *data, uint16_t len);
extern uint16_t upper_layer_chksum(uint8_t proto);
extern uint16_t uip_htons(uint16_t val);
extern uint16_t uip_icmp6chksum(void);
extern void icmp6_input();

#define UIP_ICMP6_INPUT_SUCCESS     0
#define UIP_ICMP6_INPUT_ERROR       1

#define UIP_ND6_NS_BUF            ((uip_nd6_ns *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ND6_NA_BUF            ((uip_nd6_na *)&uip_buf[uip_l2_l3_icmp_hdr_len])

void ns_input(void);
void echo_reply();

/**
 * \brief is address a multicast address, see RFC 4291
 * a is of type uip_ipaddr_t*
 * */
#define uip_is_addr_mcast(a)                    \
  (((a)->u8[0]) == 0xFF)

/**
 * \brief  is addr (a) a solicited node multicast address, see RFC 4291
 *  a is of type uip_ipaddr_t*
 */
#define uip_is_addr_solicited_node(a)          \
  ((((a)->u8[0])  == 0xFF) &&                  \
   (((a)->u8[1])  == 0x02) &&                  \
   (((a)->u16[1]) == 0x00) &&                  \
   (((a)->u16[2]) == 0x00) &&                  \
   (((a)->u16[3]) == 0x00) &&                  \
   (((a)->u16[4]) == 0x00) &&                  \
   (((a)->u8[10]) == 0x00) &&                  \
   (((a)->u8[11]) == 0x01) &&                  \
   (((a)->u8[12]) == 0xFF))

#define uip_ipaddr_copy(dest, src) (*(dest) = *(src))

/**
 * \brief is address a global multicast address (FFxE::/16),
 * a is of type uip_ip6addr_t*
 * */
#define uip_is_addr_mcast_global(a) \
  ((((a)->u8[0]) == 0xFF) && \
   (((a)->u8[1] & 0x0F) == 0x0E))

#define uip_clear_buf() { \
  uip_len = 0; \
  uip_ext_len = 0; \
}
#define UIP_ND6_NA_FLAG_ROUTER          0x80
#define UIP_ND6_NA_FLAG_SOLICITED       0x40
#define UIP_ND6_NA_FLAG_OVERRIDE        0x20
#define UIP_ND6_RA_FLAG_ONLINK          0x80
#define UIP_ND6_RA_FLAG_AUTONOMOUS      0x40

void
create_llao(uint8_t *llao, uint8_t type);

void echo_request_input(void);
void *memmove(void *v_dst, const void *v_src, int32_t c);

#endif /* UIP_H_ */
