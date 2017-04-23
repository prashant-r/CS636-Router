// /*/**
//  * @ingroup route
//  */
// syscall rtAdd(const struct netaddr *dst, const struct netaddr *gate,
//               const struct netaddr *mask, struct netif *nif)
// {
//     struct rtEntry *rtptr;
//     uchar octet;
//     ushort length;
//     int i;

//     /* Error check pointers */
//     if ((NULL == dst) || (NULL == mask) || (NULL == nif))
//     {
//         return SYSERR;
//     }
//     RT_TRACE("Dest = %d.%d.%d.%d", dst->addr[0], dst->addr[1],
//              dst->addr[2], dst->addr[3]);
//     RT_TRACE("Mask = %d.%d.%d.%d", mask->addr[0], mask->addr[1],
//              mask->addr[2], mask->addr[3]);
//     if (gate)
//         RT_TRACE("Gate = %d.%d.%d.%d", gate->addr[0], gate->addr[1],
//                  gate->addr[2], gate->addr[3]);
//     RT_TRACE("nif = %d", nif - netiftab);

//     /* Allocate an entry in the route table */
//     rtptr = rtAlloc();
//     if ((SYSERR == (int)rtptr) || (NULL == rtptr))
//     {
//         return SYSERR;
//     }

//     /* Populate the entry */
//     netaddrcpy(&rtptr->dst, dst);
//     netaddrmask(&rtptr->dst, mask);
//     if (NULL == gate)
//     {
//         rtptr->gateway.type = NULL;
//     }
//     else
//     {
//         netaddrcpy(&rtptr->gateway, gate);
//     }
//     netaddrcpy(&rtptr->mask, mask);
//     rtptr->nif = nif;

//     /* Calculate mask length */
//     length = 0;
//     for (i = 0; i < mask->len; i++)
//     {
//         octet = mask->addr[i];
//         // This totals non-zero bits
//         while (octet > 0)
//         {
//             if (octet & 0x01)
//             {
//                 length++;
//             }
//             octet = octet >> 1;
//         }
//     }
//     rtptr->masklen = length;

//     rtptr->state = RT_USED;
//     return OK;
// }

// /**
//  * @ingroup route
//  *
//  * Allocates an entry from the route table.
//  * @return entry in route table, NULL if all used, SYSERR if error occurs
//  */
// struct rtEntry *rtAlloc(void)
// {
//     int i;
//     irqmask im;

//     RT_TRACE("Allocating route entry");

//     im = disable();
//     for (i = 0; i < RT_NENTRY; i++)
//     {
//         /* If entry is free, return entry */
//         if (RT_FREE == rttab[i].state)
//         {
//             rttab[i].state = RT_PEND;
//             RT_TRACE("Free entry %d", i);
//             restore(im);
//             return &rttab[i];
//         }
//     }

//     restore(im);
//     RT_TRACE("No free entry");
//     return NULL;
// }

// syscall rtClear(struct netif *nif)
// {
//     int i;
//     irqmask im;

//     /* Error check pointers */
//     if (NULL == nif)
//     {
//         return SYSERR;
//     }

//     im = disable();
//     for (i = 0; i < RT_NENTRY; i++)
//     {
//         if ((RT_USED == rttab[i].state) && (nif == rttab[i].nif))
//         {
//             rttab[i].state = RT_FREE;
//             rttab[i].nif = NULL;
//         }
//     }
//     restore(im);
//     return OK;
// }

// /**
//  * @ingroup route
//  *
//  * Set the default route.
//  * @param gate gateway for default route
//  * @param nif network interface for default route
//  * @return OK if added/updated successfully, otherwise SYSERR
//  */
// syscall rtDefault(const struct netaddr *gate, struct netif *nif)
// {
//     struct rtEntry *rtptr;
//     int i;
//     struct netaddr mask;

//     /* Error check pointers */
//     if ((NULL == gate) || (NULL == nif) || (gate->len > NET_MAX_ALEN))
//     {
//         RT_TRACE("Invalid args");
//         return SYSERR;
//     }

//     /* Setup mask for default route */
//     mask.type = gate->type;
//     mask.len = gate->len;
//     bzero(mask.addr, mask.len);

//     /* Check if a default route already exists */
//     rtptr = NULL;
//     for (i = 0; i < RT_NENTRY; i++)
//     {
//         if ((RT_USED == rttab[i].state)
//             && (netaddrequal(&rttab[i].mask, &mask)))
//         {
//             RT_TRACE("Default route exists, entry %d", i);
//             return OK;
//         }
//     }

//     /* Allocate an entry in the route table */
//     rtptr = rtAlloc();
//     if ((SYSERR == (int)rtptr) || (NULL == rtptr))
//     {
//         RT_TRACE("Failed to allocate route entry");
//         return SYSERR;
//     }

//     /* Populate the entry */
//     netaddrcpy(&rtptr->dst, &mask);
//     netaddrcpy(&rtptr->gateway, gate);
//     netaddrcpy(&rtptr->mask, &mask);
//     rtptr->nif = nif;

//     /* Calculate mask length */
//     rtptr->masklen = 0;

//     rtptr->state = RT_USED;
//     RT_TRACE("Populated default route");
//     return OK;
// }

// syscall rtInit(void)
// {
//     int i = 0;

//     /* Initialize route table */
//     for (i = 0; i < RT_NENTRY; i++)
//     {
//         bzero(&rttab[i], sizeof(struct rtEntry));
//         rttab[i].state = RT_FREE;
//     }

//     /* Initialize route queue */
//     rtqueue = mailboxAlloc(RT_NQUEUE);
//     if (SYSERR == rtqueue)
//     {
//         return SYSERR;
//     }

//      Spawn rtDameon thread 
//     ready(create
//           ((void *)rtDaemon, RT_THR_STK, RT_THR_PRIO, "rtDaemon", 0),
//           RESCHED_NO);

//     return OK;
// }


// /**
//  * @ingroup route
//  *
//  * Looks up an entry in the routing table
//  * @param addr the IP address that needs routing
//  * @return a route table entry, NULL if none matches, SYSERR on error
//  */
// struct rtEntry *rtLookup(const struct netaddr *addr)
// {
//     int i;
//     struct rtEntry *rtptr;
//     struct netaddr masked;
//     irqmask im;

//     rtptr = NULL;

//     RT_TRACE("Addr = %d.%d.%d.%d", addr->addr[0], addr->addr[1],
//              addr->addr[2], addr->addr[3]);

//     im = disable();
//     for (i = 0; i < RT_NENTRY; i++)
//     {
//         if (RT_USED == rttab[i].state)
//         {
//             /* Mask off address */
//             netaddrcpy(&masked, addr);
//             netaddrmask(&masked, &rttab[i].mask);

//             /* Check if match  */
//             if (netaddrequal(&masked, &rttab[i].dst))
//             {
//                 RT_TRACE("Matched entry %d", i);
//                 /* Remember match if no match so far or match is better */
//                 if ((NULL == rtptr)
//                     || (rtptr->masklen < rttab[i].masklen))
//                 {
//                     rtptr = &rttab[i];
//                 }
//             }
//         }
//     }
//     restore(im);

//     return rtptr;
// }


// /**
//  * @ingroup route
//  */
// syscall rtRecv(struct packet *pkt)
// {
//     irqmask im;

//     /* Error check pointers */
//     if (NULL == pkt)
//     {
//         return SYSERR;
//     }

//     /* If route queue is full, then drop packet */
//     im = disable();
//     if (mailboxCount(rtqueue) >= RT_NQUEUE)
//     {
//         restore(im);
//         RT_TRACE("Route queue full");
//         netFreebuf(pkt);
//         return OK;
//     }

//     /* Place packet in queue */
//     if (SYSERR == mailboxSend(rtqueue, (int)pkt))
//     {
//         restore(im);
//         RT_TRACE("Failed to enqueue packet");
//         netFreebuf(pkt);
//         return SYSERR;
//     }

//     restore(im);
//     RT_TRACE("Enqueued packet for routing");
//     return OK;
// }

// /**
//  * @ingroup route
//  *
//  * Removes any entry based on its destination.
//  * @param dst destination IP address
//  * @return OK if the entry is removed successfully, otherwise SYSERR
//  */
// syscall rtRemove(const struct netaddr *dst)
// {
//     int i;
//     irqmask im;

//     /* Error check pointers */
//     if (NULL == dst)
//     {
//         return SYSERR;
//     }

//     im = disable();
//     for (i = 0; i < RT_NENTRY; i++)
//     {
//         if ((RT_USED == rttab[i].state)
//             && netaddrequal(dst, &rttab[i].dst))
//         {
//             rttab[i].state = RT_FREE;
//             rttab[i].nif = NULL;
//         }
//     }
//     restore(im);
//     return OK;
// }


// /**
//  * @ingroup route
//  *
//  * Attempt to route a packet.
//  * @param pkt incoming packet to route
//  * @return OK if packet is routed successfully, otherwise SYSERR
//  */
// syscall rtSend(struct packet *pkt)
// {
//     struct ipv4Pkt *ip;
//     struct netaddr dst;
//     struct rtEntry *route;
//     struct netaddr *nxthop;

//     /* Error check pointers */
//     if (NULL == pkt)
//     {
//         return SYSERR;
//     }

//     ip = (struct ipv4Pkt *)pkt->nethdr;
//     dst.type = NETADDR_IPv4;
//     dst.len = IPv4_ADDR_LEN;
//     memcpy(dst.addr, ip->dst, dst.len);

//     route = rtLookup(&dst);

//     if ((SYSERR == (ulong)route) || (NULL == (ulong)route))
//     {
//         RT_TRACE("Routed packet: Network unreachable.");
//         icmpDestUnreach(pkt, ICMP_NET_UNR);
//         return SYSERR;
//     }

//     if (route->nif == pkt->nif)
//     {
//         // If outgoing interface is same as incoming, send ICMP redirect
//         if (NULL == route->gateway.type)
//         {
//             icmpRedirect(pkt, ICMP_RHST, route);
//         }
//         else
//         {
//             icmpRedirect(pkt, ICMP_RNET, route);
//         }
//     }

//     /* Update IP header */
//     ip->ttl--;
//     if (0 == ip->ttl)
//     {
//         // 11 - Time Exceeded
//         icmpTimeExceeded(pkt, ICMP_TTL_EXC);
//         return SYSERR;
//     }
//     ip->chksum = 0;
//     ip->chksum = netChksum((uchar *)ip, IPv4_HDR_LEN);

//     /* Change packet to new network interface */
//     pkt->nif = route->nif;

//     /* Determine if packet should be send to destination or gateway */
//     if (NULL == route->gateway.type)
//     {
//         nxthop = &dst;
//     }
//     else
//     {
//         nxthop = &route->gateway;
//     }

//     if (SYSERR == ipv4SendFrag(pkt, nxthop))
//     {
//         RT_TRACE("Routed packet: Host unreachable.");
//         icmpDestUnreach(pkt, ICMP_HST_UNR);
//         return SYSERR;
//     }

//     RT_TRACE("Routed packet was sent.");

//     return OK;
// }*/