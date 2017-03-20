/* net.h */
//fe80:0:0:0:3333:ffff:fe00:09b9
//33:33:ff:00:09:b9
//98:4f:ee:00:09 :b9
//ping6 -I eth0.136 fe80:0:0:0:3333:ffff:fe00:081d

/* net.h */

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define NETSTK		8192 		/* Stack size for network setup */
#define NETPRIO		500    		/* Network startup priority 	*/
#define NETBOOTFILE	128		/* Size of the netboot filename	*/
/* Constants used in the networking code */

#define	ETH_ARP     0x0806		/* Ethernet type for ARP	*/
#define	ETH_IP      0x0800		/* Ethernet type for IP		*/
#define	ETH_IPv6    0x86DD		/* Ethernet type for IPv6	*/

#define	OTH_ADDR_LEN	ETH_ADDR_LEN	/* Othernet address size	*/

/* Format of an Ethernet or Othernet packet */

#pragma pack(2)
struct	netpacket	{
	byte	net_ethdst[ETH_ADDR_LEN];/* Ethernet dest. MAC address	*/
	byte	net_ethsrc[ETH_ADDR_LEN];/* Ethernet source MAC address	*/
	uint16	net_ethtype;		/* Ethernet type field		*/
	byte	net_ipvh;		/* IP version and hdr length	*/
	byte	net_iptos;		/* IP type of service		*/
	uint16	net_iplen;		/* IP total packet length	*/
	uint16	net_ipid;		/* IP datagram ID		*/
	uint16	net_ipfrag;		/* IP flags & fragment offset	*/
	byte	net_ipttl;		/* IP time-to-live		*/
	byte	net_ipproto;		/* IP protocol (actually type)	*/
	uint16	net_ipcksum;		/* IP checksum			*/
	uint32	net_ipsrc;		/* IP source address		*/
	uint32	net_ipdst;		/* IP destination address	*/
	union {
	 struct {
	  uint16 	net_udpsport;	/* UDP source protocol port	*/
	  uint16	net_udpdport;	/* UDP destination protocol port*/
	  uint16	net_udplen;	/* UDP total length		*/
	  uint16	net_udpcksum;	/* UDP checksum			*/
	  byte		net_udpdata[1500-28];/* UDP payload (1500-above)*/
	 };
	 struct {
	  byte		net_ictype;	/* ICMP message type		*/
	  byte		net_iccode;	/* ICMP code field (0 for ping)	*/
	  uint16	net_iccksum;	/* ICMP message checksum	*/
	  uint16	net_icident; 	/* ICMP identifier		*/
	  uint16	net_icseq;	/* ICMP sequence number		*/
	  byte		net_icdata[1500-28];/* ICMP payload (1500-above)*/
	 };
	 int16	net_iface;		/* Interface over which the	*/
	};
};
#pragma pack()

#pragma pack(2)
struct	netpacket_fmt2	{
	byte	net_dst[ETH_ADDR_LEN];	/* Destination MAC address	*/
	byte	net_src[ETH_ADDR_LEN];	/* Source MAC address		*/
	uint16	net_type;		/* Layer 2 type field		*/
	byte	net_payload[1500];	/* Packet payload		*/
	uint32	net_ethcrc;		/* Ethernet CRC			*/
	int16	net_iface;		/* Interface over which the	*/
					/*   packet arrived (placed	*/
					/*   after the actual packet so	*/
					/*   we can pass the packet	*/
					/*   address directly to write	*/
};

struct uip_eth_addr {
	    uint8_t addr[6];
};

struct uip_eth_hdr {
  struct uip_eth_addr dest;
  struct uip_eth_addr src;
  uint16_t type;
};

#define	PACKLEN	sizeof(struct netpacket)

extern	bpid32	netbufpool;		/* ID of net packet buffer pool	*/



/* Definintions for network interfaces (used by IP) */

#define	NIFACES	3		/* Number of interfaces -- one for the	*/
				/*   Ethernet plus two "othernets"	*/

/* Interface state definitions */

#define	IF_UP		1	/* Interface is currently on line	*/
#define	IF_DOWN		0	/* Interface is currently offline	*/

#define	IF_QUEUESIZE	20	/* Size of the incoming packet queue	*/
				/*   for each interface			*/

#define	IF_NLEN		32	/* Max characters in an interface name	*/

/* Network interface structure */

struct	ifentry	{
	char	if_name[IF_NLEN];/* Name for the interface		*/
	bool8	if_state;	/* Interface is either up or down	*/
	did32	if_dev;		/* Device ID of the layer2 device used	*/
				/*    with this interface		*/
	byte	if_macucast[ETH_ADDR_LEN]; /* MAC unicast address	*/
	byte	if_macbcast[ETH_ADDR_LEN]; /* MAC broadcast address	*/
	sid32	if_sem;		/* semaphore counts incoming packets	*/
				/*    in the queue			*/
	struct	netpacket *if_queue[IF_QUEUESIZE]; /* queue to hold	*/
				/*  incoming packets for this interface	*/
	int32	if_head;	/* next entry in packet queue to remove	*/
	int32	if_tail;	/* next slot in packet queue to insert	*/

	/* *** NOTE ADDITIONAL IPv4/IPv6/ARP/ND fields go here *** */
};

extern	struct	ifentry	if_tab[];

extern	int32	ifprime;	/* Primary interface.  For a host, the	*/
				/*   only interface that's up, for a	*/
				/*   nat box, interface 0.  -1 means	*/
				/*   no network is active.		*/
extern	bool8	host;		/* TRUE if this node is running as a	*/
				/*   host; FALSE if acting as a NAT box	*/
extern	int32	bingid;		/* User's bing ID			*/


#pragma pack(2)
/* Separate structures for each data packet type */

struct eth_packet        {
	byte	net_ethdst[ETH_ADDR_LEN];/* Ethernet dest. MAC address	*/
	byte	net_ethsrc[ETH_ADDR_LEN];/* Ethernet source MAC address	*/
	uint16	net_ethtype;		/* Ethernet type field		*/
    byte    net_ethdata[1500-ETH_HDR_LEN];   /* Ethernet payload (1500-above)*/
};
#pragma pack()

extern	struct	eth_packet *currpkt;	/* ptr to current input packet	*/
struct	network	{			/* Network information		*/
	uint32	ipucast;		/* Computer's IP unicast address*/
	uint32	ipbcast;		/* IP broadcast address		*/
	uint32	ipmask;			/* IP address mask		*/
	uint32	ipprefix;		/* IP (network) prefix		*/
	uint32	iprouter;		/* Default router address	*/
	uint32	bootserver;		/* Boot server address		*/
	uint32	dnsserver;		/* DNS server address		*/
	uint32	ntpserver;		/* NTP (time) server address	*/
	bool8	ipvalid;		/* nonzero => above are valid	*/
	byte	ethucast[ETH_ADDR_LEN];	/* Ethernet multicast address	*/
	byte	ethbcast[ETH_ADDR_LEN];	/* Ethernet broadcast address	*/
	char	bootfile[NETBOOTFILE];	/* Name of boot file		*/
};

extern	struct	network NetData;	/* Local Network Interface info	*/