// #include <xinu.h>
// /* ethernet headers are always exactly 14 bytes */
// //uint8_t uip_ext_len;
// ///uint16_t uip_len;

// void ipv6_in(char * packet)
// {
//   // uip_buf = (unsigned char *) (packet + 0);
//   // uip_next_hdr = &UIP_IP_BUF->proto;


//   // if((UIP_IP_BUF->len[0] << 8) + UIP_IP_BUF->len[1] <= uip_len) {
//   //   uip_len = (UIP_IP_BUF->len[0] << 8) + UIP_IP_BUF->len[1] + UIP_IPH_LEN;
//   // }
//   // /* Print its length */
//   // switch(*uip_next_hdr)
//   // {
//   //   case UIP_PROTO_ICMP6:
//   //     /* ICMPv6 */
//   //     icmp6_input();
      
//   // }

// }


// uint16_t upper_layer_chksum(uint8_t proto)
// {
// /* gcc 4.4.0 - 4.6.1 (maybe 4.3...) with -Os on 8 bit CPUS incorrectly compiles:
//  * int bar (int);
//  * int foo (unsigned char a, unsigned char b) {
//  *   int len = (a << 8) + b; //len becomes 0xff00&<random>+b
//  *   return len + bar (len);
//  * }
//  * upper_layer_len triggers this bug unless it is declared volatile.
//  * See https://sourceforge.net/apps/mantisbt/contiki/view.php?id=3
//  */
// //   volatile uint16_t upper_layer_len;
// //   uint16_t sum;

// //   upper_layer_len = (((uint16_t)(UIP_IP_BUF->len[0]) << 8) + UIP_IP_BUF->len[1] - uip_ext_len);

// //   PRINTF("Upper layer checksum len: %d from: %d\n", upper_layer_len,
// //          UIP_IPH_LEN + UIP_LLH_LEN + uip_ext_len);

// //   /* First sum pseudoheader. */
// //   /* IP protocol and length fields. This addition cannot carry. */
// //   sum = upper_layer_len + proto;
// //   /* Sum IP source and destination addresses. */
// //   sum = chksum(sum, (uint8_t *)&UIP_IP_BUF->srcipaddr, 2 * sizeof(uip_ipaddr_t));

// //   /* Sum TCP header and data. */
// //   sum = chksum(sum, &uip_buf[UIP_IPH_LEN + UIP_LLH_LEN + uip_ext_len],
// //                upper_layer_len);

// //   return (sum == 0) ? 0xffff : uip_htons(sum);
// // }

// // uint16_t chksum(uint16_t sum, const uint8_t *data, uint16_t len)
// // {
// //   uint16_t t;
// //   const uint8_t *dataptr;
// //   const uint8_t *last_byte;

// //   dataptr = data;
// //   last_byte = data + len - 1;

// //   while(dataptr < last_byte) {   /* At least two more bytes */
// //     t = (dataptr[0] << 8) + dataptr[1];
// //     sum += t;
// //     if(sum < t) {
// //       sum++;      /* carry */
// //     }
// //     dataptr += 2;
// //   }

// //   if(dataptr == last_byte) {
// //     t = (dataptr[0] << 8) + 0;
// //     sum += t;
// //     if(sum < t) {
// //       sum++;      /* carry */
// //     }
// //   }

// //   /* Return sum in host byte order. */
// //   return sum;
// }


// uint16_t uip_htons(uint16_t val)
// {
//   return UIP_HTONS(val);
// }


// uint16_t uip_icmp6chksum(void)
// {
//   return upper_layer_chksum(UIP_PROTO_ICMP6);

// }

// void icmp6_input()
// {

//   /* This is IPv6 ICMPv6 processing code. */
//   // PRINTF("icmp6_input: length %d type: %d \n", uip_len, UIP_ICMP_BUF->type);

//   // #if UIP_CONF_IPV6_CHECKS
//   //   /* Compute and check the ICMP header checksum */
//   //   if(uip_icmp6chksum() != 0xffff) {
//   //     PRINTF("icmpv6: bad checksum.\n");
//   //     //goto drop;
//   //   }
//   // #endif /*UIP_CONF_IPV6_CHECKS*/
  
//   //  //* Search generic input handlers.
//   //  //* The handler is in charge of setting uip_len to 0
   
//   // if(uip_icmp6_input(UIP_ICMP_BUF->type,
//   //                    UIP_ICMP_BUF->icode) == UIP_ICMP6_INPUT_ERROR) {
//   //   PRINTF("Unknown ICMPv6 message type/code %d\n", UIP_ICMP_BUF->type);
//   //   // UIP_STAT(++uip_stat.icmp.drop);
//   //   // UIP_STAT(++uip_stat.icmp.typeerr);
//   //   // UIP_LOG("icmp6: unknown ICMPv6 message.");
//   //   // uip_clear_buf();
//   // }
// }

// uint8_t uip_icmp6_input(uint8_t type, uint8_t icode)
// {
//   switch(type)
//   {
//     case ICMP6_ECHO_REQUEST:
//       PRINTF("Its a request\n");
//       echo_request_input();
//       break;
//     case ICMP6_ECHO_REPLY:
//       PRINTF("Its a reply \n");
//       break;
//     case ICMP6_NS:
//       PRINTF("It's a NS \n");
//       echo_request_input();
//       break;
//     case ICMP6_NA:
//       PRINTF("Its a NA \n");
//       break;
//     case ICMP6_RS:
//       PRINTF("Its a RS \n");
//       break;
//     case ICMP6_RA:
//       PRINTF("Its a RA \n");
//       break;
//     default:
//       return UIP_ICMP6_INPUT_ERROR;
//       break;
//   }
//   return UIP_ICMP6_INPUT_SUCCESS;
// }

// void echo_request_input(void)
// {
//   // /*
//   //  * we send an echo reply. It is trivial if there was no extension
//   //  * headers in the request otherwise we need to remove the extension
//   //  * headers and change a few fields
//   //  */
//   // PRINTF("Received Echo Request from ");
//   // PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
//   // PRINTF(" to ");
//   // PRINT6ADDR(&UIP_IP_BUF->destipaddr);
//   // PRINTF("\n");

//   // /* IP header */
//   // UIP_IP_BUF->ttl = 3;

//   // if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)){
//   //   uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);
//   //   memset(&UIP_IP_BUF->srcipaddr, 0, sizeof(uip_ip6addr_t));
//   // } else {
//   //   uip_ipaddr_copy(&tmp_ipaddr, &UIP_IP_BUF->srcipaddr);
//   //   uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
//   //   uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &tmp_ipaddr);
//   // }

//   // if(uip_ext_len > 0) {
//   //   /* Remove extension headers if any */
//   //   UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
//   //   uip_len -= uip_ext_len;
//   //   UIP_IP_BUF->len[0] = ((uip_len - UIP_IPH_LEN) >> 8);
//   //   UIP_IP_BUF->len[1] = ((uip_len - UIP_IPH_LEN) & 0xff);
//   //   /* move the echo request payload (starting after the icmp header)
//   //    * to the new location in the reply.
//   //    * The shift is equal to the length of the extension headers present
//   //    * Note: UIP_ICMP_BUF still points to the echo request at this stage
//   //    */
//   //   memmove((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN - uip_ext_len,
//   //       (uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN,
//   //       (uip_len - UIP_IPH_LEN - UIP_ICMPH_LEN));
//   //   uip_ext_len = 0;
//   // }

//   // /* Below is important for the correctness of UIP_ICMP_BUF and the
//   //  * checksum
//   //  */

//   // /* Note: now UIP_ICMP_BUF points to the beginning of the echo reply */
//   // UIP_ICMP_BUF->type = ICMP6_ECHO_REPLY;
//   // UIP_ICMP_BUF->icode = 0;
//   // UIP_ICMP_BUF->icmpchksum = 0;
//   // UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

//   // PRINTF("Sending Echo Reply to ");
//   // PRINT6ADDR(&UIP_IP_BUF->destipaddr);
//   // PRINTF(" from ");
//   // PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
//   // PRINTF("\n");
//   // return;
// }

// void *memmove(void *v_dst, const void *v_src, int32_t c)
// {
//   // const char *src = v_src;
//   // char *dst = v_dst;

//   // if (!c)
//   //   return v_dst;

//   // /* Use memcpy when source is higher than dest */
//   // if (v_dst <= v_src)
//   //   return memcpy(v_dst, v_src, c);

//   // /* copy backwards, from end to beginning */
//   // src += c;
//   // dst += c;

//   // /* Simple, byte oriented memmove. */
//   // while (c--)
//   //   *--dst = *--src;

//   // return v_dst;
// }