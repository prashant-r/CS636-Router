/* pdump.c - packet dump function */

#include <xinu.h>

/*------------------------------------------------------------------------
 * pdump - dump a packet assuming all fields are in network byte order
 *------------------------------------------------------------------------
 */
void	pdump(struct  netpacket *pptr)
{
#ifdef IP_STUFF
	struct	arppacket *aptr;
#endif

	PRINTF("Packet ");
	if ( (pptr->net_iface >= 0) && (pptr->net_iface < NIFACES) ) {
		PRINTF("on %s ", if_tab[pptr->net_iface].if_name);
	}
	PRINTF("%02x:%02x:%02x:%02x:%02x:%02x >",
			pptr->net_src[0],
			pptr->net_src[1],
			pptr->net_src[2],
			pptr->net_src[3],
			pptr->net_src[4],
			pptr->net_src[5]
	       );
	PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x, ",
			pptr->net_dst[0],
			pptr->net_dst[1],
			pptr->net_dst[2],
			pptr->net_dst[3],
			pptr->net_dst[4],
			pptr->net_dst[5]
	       );

	PRINTF("ethertype ");

#ifdef IP_STUFF
	switch (ntohs(pptr->net_type)) {

		case 0x0806:
			PRINTF("ARP (0x%04x), length %d: ",
					ntohs(pptr->net_type),
					sizeof(struct arppacket));
			aptr = (struct arppacket *)pptr;
			
			if ( (ntohs(aptr->arp_htype) != ARP_HTYPE) ||
			     (ntohs(aptr->arp_ptype) != ARP_PTYPE) ) {
				break;
			}

			PRINTF("hard addr len=%d), proto addr len=%d), ",
					aptr->arp_hlen,
					aptr->arp_plen);

			if (ntohs(aptr->arp_op) == ARP_OP_REQ) {
				PRINTF("Request who-has %d.%d.%d.%d tell %d.%d.%d.%d, ",
						(ntohl(aptr->arp_tarpa)>>24)&0xff,
						(ntohl(aptr->arp_tarpa)>>16)&0xff,
						(ntohl(aptr->arp_tarpa)>>8)&0xff,
						ntohl(aptr->arp_tarpa)&0xff,
						(ntohl(aptr->arp_sndpa)>>24)&0xff,
						(ntohl(aptr->arp_sndpa)>>16)&0xff,
						(ntohl(aptr->arp_sndpa)>>8)&0xff,
						ntohl(aptr->arp_sndpa)&0xff);
			} else if (ntohs(aptr->arp_op) == ARP_OP_RPLY) {
				PRINTF("Reply %d.%d.%d.%d is-at %02x:%02x:%02x:%02x:%02x:%02x, ",
						(ntohl(aptr->arp_sndpa)>>24)&0xff,
						(ntohl(aptr->arp_sndpa)>>16)&0xff,
						(ntohl(aptr->arp_sndpa)>>8)&0xff,
						ntohl(aptr->arp_sndpa)&0xff,
						aptr->arp_sndha[0],
						aptr->arp_sndha[1],
						aptr->arp_sndha[2],
						aptr->arp_sndha[3],
						aptr->arp_sndha[4],
						aptr->arp_sndha[5]);
			} else {
				break;
			}

			PRINTF("length %d\n", sizeof(struct arppacket) - ETH_HDR_LEN);
			break;

		case 0x0800:
			PRINTF("IPv4 (0x%04x), length %d: ",
					ntohs(pptr->net_type),
					ntohs(pptr->net_iplen) + ETH_HDR_LEN);
			
			PRINTF("(");
			PRINTF("tos 0x%01x, ttl %d, id %d, offset %d ",
					pptr->net_iptos,
					pptr->net_ipttl,
					ntohs(pptr->net_ipid),
					(ntohs(pptr->net_ipfrag) & 0x1fff));

			PRINTF("flags ");
			if (ntohs(pptr->net_ipfrag) & 0x4000)
				PRINTF("[DF], ");
			else if (ntohs(pptr->net_ipfrag) & 0x2000)
				PRINTF("[MF], " );
			else
				PRINTF("[none], ");

			PRINTF("[ip checksum %s], ",
					ipcksum(pptr) == 0 ? "ok" : "fail");

			if (pptr->net_ipproto == IP_UDP) {
				PRINTF("proto UDP (%d), length %d",
						pptr->net_ipproto, ntohs(pptr->net_iplen));
				PRINTF(")\n");
				PRINTF("\t%d.%d.%d.%d > ",
						(ntohl(pptr->net_ipsrc)>>24)&0xff,
						(ntohl(pptr->net_ipsrc)>>16)&0xff,
						(ntohl(pptr->net_ipsrc)>>8)&0xff,
						(ntohl(pptr->net_ipsrc)&0xff));
				PRINTF("%d.%d.%d.%d: ",
						(ntohl(pptr->net_ipdst)>>24)&0xff,
						(ntohl(pptr->net_ipdst)>>16)&0xff,
						(ntohl(pptr->net_ipdst)>>8)&0xff,
						(ntohl(pptr->net_ipdst)&0xff));
				PRINTF("[udp checksum none] ");
				PRINTF("UDP, src port %d, dst port %d, length %d\n",
						ntohs(pptr->net_udpsport),
						ntohs(pptr->net_udpdport),
						ntohs(pptr->net_udplen) - UDP_HDR_LEN);
				break;
			} else if (pptr->net_ipproto == IP_ICMP) {
				PRINTF("proto ICMP (%d), length %d",
						pptr->net_ipproto, ntohs(pptr->net_iplen));
				PRINTF(")\n");
				PRINTF("\t%d.%d.%d.%d > ",
						(ntohl(pptr->net_ipsrc)>>24)&0xff,
						(ntohl(pptr->net_ipsrc)>>16)&0xff,
						(ntohl(pptr->net_ipsrc)>>8)&0xff,
						(ntohl(pptr->net_ipsrc)&0xff));
				PRINTF("%d.%d.%d.%d: ",
						(ntohl(pptr->net_ipdst)>>24)&0xff,
						(ntohl(pptr->net_ipdst)>>16)&0xff,
						(ntohl(pptr->net_ipdst)>>8)&0xff,
						(ntohl(pptr->net_ipdst)&0xff));

				if (pptr->net_ictype == ICMP_ECHOREQST)
					PRINTF("ICMP echo request, ");
				else if (pptr->net_ictype == ICMP_ECHOREPLY)
					PRINTF("ICMP echo reply, ");
				else
					break;

				PRINTF("id %d, seq %d\n",
						ntohs(pptr->net_icident),
						ntohs(pptr->net_icseq));
				
				break;
			}
			else
				break;
		case 0x086dd:
			printf("Came here \n");
			ip_debug_print();
			break;
		default:
			PRINTF("unknown\n");
			break;
	}
#else
	PRINTF("0x%04x\n", ntohs(pptr->net_type));
#endif



	return;
}

/*------------------------------------------------------------------------
 * pdumph - dump a packet assuming all fields are in host byte order
 *------------------------------------------------------------------------
 */
void	pdumph(struct  netpacket *pptr)
{
#ifndef IP_STUFF
	struct	arppacket *aptr;
#endif

	PRINTF("%02x:%02x:%02x:%02x:%02x:%02x >",
			pptr->net_src[0],
			pptr->net_src[1],
			pptr->net_src[2],
			pptr->net_src[3],
			pptr->net_src[4],
			pptr->net_src[5]
	       );
	PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x, ",
			pptr->net_dst[0],
			pptr->net_dst[1],
			pptr->net_dst[2],
			pptr->net_dst[3],
			pptr->net_dst[4],
			pptr->net_dst[5]
	       );

	PRINTF("ethertype ");

#ifdef IP_STUFF
	switch (pptr->net_type) {

		case 0x0806:
			PRINTF("ARP (0x%04x), length %d: ",
					pptr->net_type,
					sizeof(struct arppacket));
			aptr = (struct arppacket *)pptr;
			
			if ( (aptr->arp_htype != ARP_HTYPE) ||
			     (aptr->arp_ptype != ARP_PTYPE) ) {
				break;
			}

			PRINTF("hard addr len=%d), proto addr len=%d), ",
					aptr->arp_hlen,
					aptr->arp_plen);

			if (aptr->arp_op == ARP_OP_REQ) {
				PRINTF("Request who-has %d.%d.%d.%d tell %d.%d.%d.%d, ",
						(aptr->arp_tarpa>>24)&0xff,
						(aptr->arp_tarpa>>16)&0xff,
						(aptr->arp_tarpa>>8)&0xff,
						aptr->arp_tarpa&0xff,
						(aptr->arp_sndpa>>24)&0xff,
						(aptr->arp_sndpa>>16)&0xff,
						(aptr->arp_sndpa>>8)&0xff,
						aptr->arp_sndpa&0xff);
			} else if (aptr->arp_op == ARP_OP_RPLY) {
				PRINTF("Reply %d.%d.%d.%d is-at %02x:%02x:%02x:%02x:%02x:%02x, ",
						(aptr->arp_sndpa>>24)&0xff,
						(aptr->arp_sndpa>>16)&0xff,
						(aptr->arp_sndpa>>8)&0xff,
						aptr->arp_sndpa&0xff,
						aptr->arp_sndha[0],
						aptr->arp_sndha[1],
						aptr->arp_sndha[2],
						aptr->arp_sndha[3],
						aptr->arp_sndha[4],
						aptr->arp_sndha[5]);
			} else {
				break;
			}

			PRINTF("length %d\n", sizeof(struct arppacket) - ETH_HDR_LEN);
			break;

		case 0x0800:
			PRINTF("IPv4 (0x%04x), length %d: ",
					pptr->net_type,
					pptr->net_iplen + ETH_HDR_LEN);
			
			PRINTF("(");
			PRINTF("tos 0x%01x, ttl %d, id %d, offset %d ",
					pptr->net_iptos,
					pptr->net_ipttl,
					pptr->net_ipid,
					pptr->net_ipfrag & 0x1fff);

			PRINTF("flags ");
			if (pptr->net_ipfrag & 0x4000)
				PRINTF("[DF], ");
			else if (pptr->net_ipfrag & 0x2000)
				PRINTF("[MF], " );
			else
				PRINTF("[none], ");

			PRINTF("[ip checksum %s], ",
					ipcksum(pptr) == 0 ? "ok" : "fail");

			if (pptr->net_ipproto == IP_UDP) {
				PRINTF("proto UDP (%d), length %d",
						pptr->net_ipproto, pptr->net_iplen);
				PRINTF(")\n");
				PRINTF("\t%d.%d.%d.%d > ",
						(pptr->net_ipsrc>>24)&0xff,
						(pptr->net_ipsrc>>16)&0xff,
						(pptr->net_ipsrc>>8)&0xff,
						(pptr->net_ipsrc&0xff));
				PRINTF("%d.%d.%d.%d: ",
						(pptr->net_ipdst>>24)&0xff,
						(pptr->net_ipdst>>16)&0xff,
						(pptr->net_ipdst>>8)&0xff,
						(pptr->net_ipdst&0xff));
				PRINTF("[udp checksum none] ");
				PRINTF("UDP, src port %d, dst port %d, length %d\n",
						pptr->net_udpsport,
						pptr->net_udpdport,
						pptr->net_udplen - UDP_HDR_LEN);
				break;
			} else if (pptr->net_ipproto == IP_ICMP) {
				PRINTF("proto ICMP (%d), length %d",
						pptr->net_ipproto, pptr->net_iplen);
				PRINTF(")\n");
				PRINTF("\t%d.%d.%d.%d > ",
						(pptr->net_ipsrc>>24)&0xff,
						(pptr->net_ipsrc>>16)&0xff,
						(pptr->net_ipsrc>>8)&0xff,
						(pptr->net_ipsrc&0xff));
				PRINTF("%d.%d.%d.%d: ",
						(pptr->net_ipsrc>>24)&0xff,
						(pptr->net_ipsrc>>16)&0xff,
						(pptr->net_ipsrc>>8)&0xff,
						(pptr->net_ipsrc&0xff));

				if (pptr->net_ictype == ICMP_ECHOREQST)
					PRINTF("ICMP echo request, ");
				else if (pptr->net_ictype == ICMP_ECHOREPLY)
					PRINTF("ICMP echo reply, ");
				else
					break;

				PRINTF("id %d, seq %d\n",
						pptr->net_icident,
						pptr->net_icseq);
				PRINTF("icmp ckeckcum %s\n",
					icmp_cksum((char *)&pptr->net_ictype,
					ntohs(pptr->net_iplen)-IP_HDR_LEN)==0?
					"OK":"failed");
				break;
			}
			else
				break;
		case 0x086DD:
			ip_debug_print();
			break;
		default:
			PRINTF("unknown\n");
			break;
	}
#else
	PRINTF("-> 0x%04x\n", pptr->net_type);
#endif

	return;
}

void
ip_debug_print()
{
  PRINTF("IP header:\n");
  PRINTF("+-------------------------------+\n");
  PRINTF("|%2"S16_F" |  %"X16_F"%"X16_F"  |      %"X16_F"%"X16_F"           | (v, traffic class, flow label)\n",
        UIP_IP_BUF->v,
        UIP_IP_BUF->tclass1, UIP_IP_BUF->tclass2,
        UIP_IP_BUF->flow1, UIP_IP_BUF->flow2);
  PRINTF("+-------------------------------+\n");
  PRINTF("|    %5"U16_F"      | %2"U16_F"  |  %2"U16_F"   | (len, nexthdr, hoplim)\n",
        (UIP_IP_BUF->len),
        UIP_IP_BUF->nexthdr,
        UIP_IP_BUF->ttl);
  PRINTF("+-------------------------------+\n");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("   | ( src ) ");
  PRINTF("\n+---------------------------------+\n");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("   | ( dest ) ");
  PRINTF("\n+-------------------------------+\n");

}
