#include <xinu.h>


static struct netpacket createPacket;

void create_ethernet__packet(void);

void uip_ds6_send_rs(void)
{
	printf("send rs\n");
	uip_buf = (unsigned char *) (&createPacket);
    uip_nd6_rs_output();
    ip_hton();
    create_ethernet__packet();
    eth_hton(&createPacket);
    PRINTF("UIP len is %d \n", uip_len);
	pdump((struct netpacket *)uip_buf);
	write(ETHER0, (char *) uip_buf, uip_len);
}

void create_ethernet__packet()
{
    memcpy(&BUF->dest, &macbcast, UIP_LLADDR_LEN);
    control(ETHER0, ETH_CTRL_GET_MAC, (int32) &uip_lladdr, 0);
	memcpy(&BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
	BUF->type = 0x086DD;
	uip_len += sizeof(struct uip_eth_hdr);
}

void uip_ds6_periodic(void)
{

	printf("ds6 periodic\n");
}