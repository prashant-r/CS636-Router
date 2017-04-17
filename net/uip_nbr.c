#include <xinu.h>

linkaddr_t linkaddr_node_addr;
#if LINKADDR_SIZE == 2
const linkaddr_t linkaddr_null = { { 0, 0 } };
#else /*LINKADDR_SIZE == 2*/
#if LINKADDR_SIZE == 8
const linkaddr_t linkaddr_null = { { 0, 0, 0, 0, 0, 0, 0, 0 } };
#endif /*LINKADDR_SIZE == 8*/
#if LINKADDR_SIZE == 6
const linkaddr_t linkaddr_null = { { 0, 0, 0, 0, 0, 0 } };
#endif /*LINKADDR_SIZE == 6*/
#endif /*LINKADDR_SIZE == 2*/


/*---------------------------------------------------------------------------*/
void
linkaddr_copy(linkaddr_t *dest, const linkaddr_t *src)
{
  memcpy(dest, src, LINKADDR_SIZE);
}
/*---------------------------------------------------------------------------*/
int
linkaddr_cmp(const linkaddr_t *addr1, const linkaddr_t *addr2)
{
  return (memcmp(addr1, addr2, LINKADDR_SIZE) == 0);
}
/*---------------------------------------------------------------------------*/
void
linkaddr_set_node_addr(linkaddr_t *t)
{
  linkaddr_copy(&linkaddr_node_addr, t);
}
#ifdef UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED
#define NEIGHBOR_STATE_CHANGED(n) UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED(n)
void NEIGHBOR_STATE_CHANGED(uip_ds6_nbr_t *n);
#else
#define NEIGHBOR_STATE_CHANGED(n)
#endif /* UIP_DS6_CONF_NEIGHBOR_STATE_CHANGED */

#ifdef UIP_CONF_DS6_LINK_NEIGHBOR_CALLBACK
#define LINK_NEIGHBOR_CALLBACK(addr, status, numtx) UIP_CONF_DS6_LINK_NEIGHBOR_CALLBACK(addr, status, numtx)
void LINK_NEIGHBOR_CALLBACK(const linkaddr_t *addr, int status, int numtx);
#else
#define LINK_NEIGHBOR_CALLBACK(addr, status, numtx)
#endif /* UIP_CONF_DS6_LINK_NEIGHBOR_CALLBACK */

NBR_TABLE_GLOBAL(uip_ds6_nbr_t, ds6_neighbors);

/*---------------------------------------------------------------------------*/
void
uip_ds6_neighbors_init(void)
{
  nbr_table_register(ds6_neighbors, (nbr_table_callback *)uip_ds6_nbr_rm);
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_add(const uip_ipaddr_t *ipaddr, const uip_lladdr_t *lladdr,
                uint8_t isrouter, uint8_t state, nbr_table_reason_t reason,
                void *data)
{
  uip_ds6_nbr_t *nbr = nbr_table_add_lladdr(ds6_neighbors, (linkaddr_t*)lladdr
                                            , reason, data);
  if(nbr) {
    uip_ipaddr_copy(&nbr->ipaddr, ipaddr);
#if UIP_ND6_SEND_RA || !UIP_CONF_ROUTER
    nbr->isrouter = isrouter;
#endif /* UIP_ND6_SEND_RA || !UIP_CONF_ROUTER */
    nbr->state = state;
#if UIP_CONF_IPV6_QUEUE_PKT
    uip_packetqueue_new(&nbr->packethandle);
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
#if UIP_ND6_SEND_NS
    if(nbr->state == NBR_REACHABLE) {
     // stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
    } else {
      /* We set the timer in expired state */
     // stimer_set(&nbr->reachable, 0);
    }
    //stimer_set(&nbr->sendns, 0);
    nbr->nscount = 0;
#endif /* UIP_ND6_SEND_NS */
    PRINTF("Adding neighbor with ip addr ");
    PRINT6ADDR(ipaddr);
    PRINTF(" link addr ");
    PRINTLLADDR(lladdr);
    PRINTF(" state %u\n", state);
    NEIGHBOR_STATE_CHANGED(nbr);
    return nbr;
  } else {
    PRINTF("uip_ds6_nbr_add drop ip addr ");
    PRINT6ADDR(ipaddr);
    PRINTF(" link addr (%p) ", lladdr);
    PRINTLLADDR(lladdr);
    PRINTF(" state %u\n", state);
    return NULL;
  }
}

/*---------------------------------------------------------------------------*/
int
uip_ds6_nbr_rm(uip_ds6_nbr_t *nbr)
{
  if(nbr != NULL) {
#if UIP_CONF_IPV6_QUEUE_PKT
    uip_packetqueue_free(&nbr->packethandle);
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
    NEIGHBOR_STATE_CHANGED(nbr);
    return nbr_table_remove(ds6_neighbors, nbr);
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
const uip_ipaddr_t *
uip_ds6_nbr_get_ipaddr(const uip_ds6_nbr_t *nbr)
{
  return (nbr != NULL) ? &nbr->ipaddr : NULL;
}

/*---------------------------------------------------------------------------*/
const uip_lladdr_t *
uip_ds6_nbr_get_ll(const uip_ds6_nbr_t *nbr)
{
  return (const uip_lladdr_t *)nbr_table_get_lladdr(ds6_neighbors, nbr);
}
/*---------------------------------------------------------------------------*/
int
uip_ds6_nbr_num(void)
{
  uip_ds6_nbr_t *nbr;
  int num;

  num = 0;
  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = nbr_table_next(ds6_neighbors, nbr)) {
    num++;
  }
  return num;
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_lookup(const uip_ipaddr_t *ipaddr)
{
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
  if(ipaddr != NULL) {
    while(nbr != NULL) {
      if(uip_ipaddr_cmp(&nbr->ipaddr, ipaddr)) {
        return nbr;
      }
      nbr = nbr_table_next(ds6_neighbors, nbr);
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_ll_lookup(const uip_lladdr_t *lladdr)
{
  return nbr_table_get_from_lladdr(ds6_neighbors, (linkaddr_t*)lladdr);
}

/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
uip_ds6_nbr_ipaddr_from_lladdr(const uip_lladdr_t *lladdr)
{
  uip_ds6_nbr_t *nbr = uip_ds6_nbr_ll_lookup(lladdr);
  return nbr ? &nbr->ipaddr : NULL;
}

/*---------------------------------------------------------------------------*/
const uip_lladdr_t *
uip_ds6_nbr_lladdr_from_ipaddr(const uip_ipaddr_t *ipaddr)
{
  uip_ds6_nbr_t *nbr = uip_ds6_nbr_lookup(ipaddr);
  return nbr ? uip_ds6_nbr_get_ll(nbr) : NULL;
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_link_neighbor_callback(int status, int numtx)
{
  printf("ds6 link neighbor callback");
//   const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
//   if(linkaddr_cmp(dest, &linkaddr_null)) {
//     return;
//   }

//   /* Update neighbor link statistics */
//   //link_stats_packet_sent(dest, status, numtx);
//   /* Call upper-layer callback (e.g. RPL) */
//   LINK_NEIGHBOR_CALLBACK(dest, status, numtx);

// #if UIP_DS6_LL_NUD
//    From RFC4861, page 72, last paragraph of section 7.3.3:
//    *
//    *         "In some cases, link-specific information may indicate that a path to
//    *         a neighbor has failed (e.g., the resetting of a virtual circuit). In
//    *         such cases, link-specific information may be used to purge Neighbor
//    *         Cache entries before the Neighbor Unreachability Detection would do
//    *         so. However, link-specific information MUST NOT be used to confirm
//    *         the reachability of a neighbor; such information does not provide
//    *         end-to-end confirmation between neighboring IP layers."
//    *
//    * However, we assume that receiving a link layer ack ensures the delivery
//    * of the transmitted packed to the IP stack of the neighbour. This is a
//    * fair assumption and allows battery powered nodes save some battery by
//    * not re-testing the state of a neighbour periodically if it
//    * acknowledges link packets. 
//   if(status == MAC_TX_OK) {
//     uip_ds6_nbr_t *nbr;
//     nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *)dest);
//     if(nbr != NULL && nbr->state != NBR_INCOMPLETE) {
//       nbr->state = NBR_REACHABLE;
//       stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
//       PRINTF("uip-ds6-neighbor : received a link layer ACK : ");
//       PRINTLLADDR((uip_lladdr_t *)dest);
//       PRINTF(" is reachable.\n");
//     }
//   }
// #endif /* UIP_DS6_LL_NUD */

}
#if UIP_ND6_SEND_NS
/*---------------------------------------------------------------------------*/
/** Periodic processing on neighbors */
void
uip_ds6_neighbor_periodic(void)
{
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
  PRINTF("Periodic processing on neighbors \n");
  while(nbr != NULL) {
    switch(nbr->state) {
    case NBR_REACHABLE:
//       if(stimer_expired(&nbr->reachable)) {
// #if UIP_CONF_IPV6_RPL
//         /* when a neighbor leave its REACHABLE state and is a default router,
//            instead of going to STALE state it enters DELAY state in order to
//            force a NUD on it. Otherwise, if there is no upward traffic, the
//            node never knows if the default router is still reachable. This
//            mimics the 6LoWPAN-ND behavior.
//          */
//         if(uip_ds6_defrt_lookup(&nbr->ipaddr) != NULL) {
//           PRINTF("REACHABLE: defrt moving to DELAY (");
//           PRINT6ADDR(&nbr->ipaddr);
//           PRINTF(")\n");
//           nbr->state = NBR_DELAY;
//           stimer_set(&nbr->reachable, UIP_ND6_DELAY_FIRST_PROBE_TIME);
//           nbr->nscount = 0;
//         } else {
//           PRINTF("REACHABLE: moving to STALE (");
//           PRINT6ADDR(&nbr->ipaddr);
//           PRINTF(")\n");
//           nbr->state = NBR_STALE;
//         }
// #else /* UIP_CONF_IPV6_RPL */
//         PRINTF("REACHABLE: moving to STALE (");
//         PRINT6ADDR(&nbr->ipaddr);
//         PRINTF(")\n");
//         nbr->state = NBR_STALE;
// #endif /* UIP_CONF_IPV6_RPL */
//       }
      break;
    case NBR_INCOMPLETE:
      if(nbr->nscount >= UIP_ND6_MAX_MULTICAST_SOLICIT) {
        uip_ds6_nbr_rm(nbr);
      }
      // } else if(stimer_expired(&nbr->sendns) && (uip_len == 0)) {
      //   nbr->nscount++;
      //   PRINTF("NBR_INCOMPLETE: NS %u\n", nbr->nscount);
      //   uip_nd6_ns_output(NULL, NULL, &nbr->ipaddr);
      //   stimer_set(&nbr->sendns, uip_ds6_if.retrans_timer / 1000);
      // }
      break;
    case NBR_DELAY:
      // if(stimer_expired(&nbr->reachable)) {
      //   nbr->state = NBR_PROBE;
      //   nbr->nscount = 0;
      //   PRINTF("DELAY: moving to PROBE\n");
      //   stimer_set(&nbr->sendns, 0);
      // }
      break;
    case NBR_PROBE:
      if(nbr->nscount >= UIP_ND6_MAX_UNICAST_SOLICIT) {
        
        printf("NBR_PROBE\n");
        // uip_ds6_defrt_t *locdefrt;
        // PRINTF("PROBE END\n");
        // if((locdefrt = uip_ds6_defrt_lookup(&nbr->ipaddr)) != NULL) {
        //   if (!locdefrt->isinfinite) {
        //     uip_ds6_defrt_rm(locdefrt);
        //   }
        // }
        // uip_ds6_nbr_rm(nbr);
      }
      // } else if(stimer_expired(&nbr->sendns) && (uip_len == 0)) {
      //   nbr->nscount++;
      //   PRINTF("PROBE: NS %u\n", nbr->nscount);
      //   uip_nd6_ns_output(NULL, &nbr->ipaddr, &nbr->ipaddr);
      //   //stimer_set(&nbr->sendns, uip_ds6_if.retrans_timer / 1000);
      // }
      break;
    default:
      break;
    }
    nbr = nbr_table_next(ds6_neighbors, nbr);
  }
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_nbr_refresh_reachable_state(const uip_ipaddr_t *ipaddr)
{
  uip_ds6_nbr_t *nbr;
  nbr = uip_ds6_nbr_lookup(ipaddr);
  if(nbr != NULL) {
    nbr->state = NBR_REACHABLE;
    nbr->nscount = 0;
    //stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
  }
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_get_least_lifetime_neighbor(void)
{
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
  uip_ds6_nbr_t *nbr_expiring = NULL;
  while(nbr != NULL) {
    nbr_expiring = nbr;
    nbr = nbr_table_next(ds6_neighbors, nbr);
  }
  return nbr_expiring;
}
#endif /* UIP_ND6_SEND_NS */