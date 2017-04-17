#ifndef UIP_DS6_NEIGHBOR_H_
#define UIP_DS6_NEIGHBOR_H_

#if UIP_CONF_IPV6_QUEUE_PKT
#include "net/ip/uip-packetqueue.h"
#endif                   

#ifdef LINKADDR_CONF_SIZE
#define LINKADDR_SIZE LINKADDR_CONF_SIZE
#else /* LINKADDR_SIZE */
#define LINKADDR_SIZE 2
#endif /* LINKADDR_SIZE */

void ipv6_in(unsigned char * packet);


typedef union {
  unsigned char u8[LINKADDR_SIZE];
#if LINKADDR_SIZE == 2
  uint16_t u16;
#endif /* LINKADDR_SIZE == 2 */
} linkaddr_t;

typedef union {
  uint8_t u8[8];
  uint16_t u16[4];
} linkaddr_extended_t;

/**
 * \brief      Copy a Rime address
 * \param dest The destination
 * \param from The source
 *
 *             This function copies a Rime address from one location
 *             to another.
 *
 */
void linkaddr_copy(linkaddr_t *dest, const linkaddr_t *from);

/**
 * \brief      Compare two Rime addresses
 * \param addr1 The first address
 * \param addr2 The second address
 * \return     Non-zero if the addresses are the same, zero if they are different
 *
 *             This function compares two Rime addresses and returns
 *             the result of the comparison. The function acts like
 *             the '==' operator and returns non-zero if the addresses
 *             are the same, and zero if the addresses are different.
 *
 */
int linkaddr_cmp(const linkaddr_t *addr1, const linkaddr_t *addr2);


/**
 * \brief      Set the address of the current node
 * \param addr The address
 *
 *             This function sets the Rime address of the node.
 *
 */
void linkaddr_set_node_addr(linkaddr_t *addr);

/**
 * \brief      The Rime address of the node
 *
 *             This variable contains the Rime address of the
 *             node. This variable should not be changed directly;
 *             rather, the linkaddr_set_node_addr() function should be
 *             used.
 *
 */
extern linkaddr_t linkaddr_node_addr;

/**
 * \brief      The null Rime address
 *
 *             This variable contains the null Rime address. The null
 *             address is used in route tables to indicate that the
 *             table entry is unused. Nodes with no configured address
 *             has the null address. Nodes with their node address set
 *             to the null address will have problems communicating
 *             with other nodes.
 *
 */
extern const linkaddr_t linkaddr_null;


/*--------------------------------------------------*/
/** \brief Possible states for the nbr cache entries */
#define  NBR_INCOMPLETE 0
#define  NBR_REACHABLE 1
#define  NBR_STALE 2
#define  NBR_DELAY 3
#define  NBR_PROBE 4


/* Neighbor table size */
#ifdef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_MAX_NEIGHBORS NBR_TABLE_CONF_MAX_NEIGHBORS
#else /* NBR_TABLE_CONF_MAX_NEIGHBORS */
#define NBR_TABLE_MAX_NEIGHBORS 8
#endif /* NBR_TABLE_CONF_MAX_NEIGHBORS */

/* An item in a neighbor table */
typedef void nbr_table_item_t;

/* Callback function, called when removing an item from a table */
typedef void(nbr_table_callback)(nbr_table_item_t *item);

/* A neighbor table */
typedef struct nbr_table {
  int index;
  int item_size;
  nbr_table_callback *callback;
  nbr_table_item_t *data;
} nbr_table_t;

/** \brief A static neighbor table. To be initialized through nbr_table_register(name) */
#define NBR_TABLE(type, name) \
  static type _##name##_mem[NBR_TABLE_MAX_NEIGHBORS]; \
  static nbr_table_t name##_struct = { 0, sizeof(type), NULL, (nbr_table_item_t *)_##name##_mem }; \
  static nbr_table_t *name = &name##_struct \

/** \brief A non-static neighbor table. To be initialized through nbr_table_register(name) */
#define NBR_TABLE_GLOBAL(type, name) \
  static type _##name##_mem[NBR_TABLE_MAX_NEIGHBORS]; \
  static nbr_table_t name##_struct = { 0, sizeof(type), NULL, (nbr_table_item_t *)_##name##_mem }; \
  nbr_table_t *name = &name##_struct \

/** \brief Declaration of non-static neighbor tables */
#define NBR_TABLE_DECLARE(name) extern nbr_table_t *name

typedef enum {
  NBR_TABLE_REASON_UNDEFINED,
  NBR_TABLE_REASON_RPL_DIO,
  NBR_TABLE_REASON_RPL_DAO,
  NBR_TABLE_REASON_RPL_DIS,
  NBR_TABLE_REASON_ROUTE,
  NBR_TABLE_REASON_IPV6_ND,
  NBR_TABLE_REASON_MAC,
  NBR_TABLE_REASON_LLSEC,
  NBR_TABLE_REASON_LINK_STATS,
} nbr_table_reason_t;

/** \name Neighbor tables: register and loop through table elements */
/** @{ */
int nbr_table_register(nbr_table_t *table, nbr_table_callback *callback);
nbr_table_item_t *nbr_table_head(nbr_table_t *table);
nbr_table_item_t *nbr_table_next(nbr_table_t *table, nbr_table_item_t *item);
/** @} */

/** \name Neighbor tables: add and get data */
/** @{ */
nbr_table_item_t *nbr_table_add_lladdr(nbr_table_t *table, const linkaddr_t *lladdr, nbr_table_reason_t reason, void *data);
nbr_table_item_t *nbr_table_get_from_lladdr(nbr_table_t *table, const linkaddr_t *lladdr);
/** @} */

/** \name Neighbor tables: set flags (unused, locked, unlocked) */
/** @{ */
int nbr_table_remove(nbr_table_t *table, nbr_table_item_t *item);
int nbr_table_lock(nbr_table_t *table, nbr_table_item_t *item);
int nbr_table_unlock(nbr_table_t *table, nbr_table_item_t *item);
/** @} */

/** \name Neighbor tables: address manipulation */
/** @{ */
linkaddr_t *nbr_table_get_lladdr(nbr_table_t *table, const nbr_table_item_t *item);
int nbr_table_update_lladdr(const linkaddr_t *old_addr, const linkaddr_t *new_addr, int remove_if_duplicate);
/** @} */


NBR_TABLE_DECLARE(ds6_neighbors);

/** \brief An entry in the nbr cache */
typedef struct uip_ds6_nbr {
  uip_ipaddr_t ipaddr;
  uint8_t isrouter;
  uint8_t state;
#if UIP_ND6_SEND_NS || UIP_ND6_SEND_RA
  int reachable;
  int sendns;
  uint8_t nscount;
#endif /* UIP_ND6_SEND_NS || UIP_ND6_SEND_RA */
#if UIP_CONF_IPV6_QUEUE_PKT
  struct uip_packetqueue_handle packethandle;
#define UIP_DS6_NBR_PACKET_LIFETIME CLOCK_SECOND * 4
#endif                          /*UIP_CONF_QUEUE_PKT */
} uip_ds6_nbr_t;

void uip_ds6_neighbors_init(void);

/** \brief Neighbor Cache basic routines */
uip_ds6_nbr_t *uip_ds6_nbr_add(const uip_ipaddr_t *ipaddr,
                               const uip_lladdr_t *lladdr,
                               uint8_t isrouter, uint8_t state,
                               nbr_table_reason_t reason, void *data);
int uip_ds6_nbr_rm(uip_ds6_nbr_t *nbr);
const uip_lladdr_t *uip_ds6_nbr_get_ll(const uip_ds6_nbr_t *nbr);
const uip_ipaddr_t *uip_ds6_nbr_get_ipaddr(const uip_ds6_nbr_t *nbr);
uip_ds6_nbr_t *uip_ds6_nbr_lookup(const uip_ipaddr_t *ipaddr);
uip_ds6_nbr_t *uip_ds6_nbr_ll_lookup(const uip_lladdr_t *lladdr);
uip_ipaddr_t *uip_ds6_nbr_ipaddr_from_lladdr(const uip_lladdr_t *lladdr);
const uip_lladdr_t *uip_ds6_nbr_lladdr_from_ipaddr(const uip_ipaddr_t *ipaddr);
void uip_ds6_link_neighbor_callback(int status, int numtx);
void uip_ds6_neighbor_periodic(void);
int uip_ds6_nbr_num(void);

#if UIP_ND6_SEND_NS
/**
 * \brief Refresh the reachable state of a neighbor. This function
 * may be called when a node receives an IPv6 message that confirms the
 * reachability of a neighbor.
 * \param ipaddr pointer to the IPv6 address whose neighbor reachability state
 * should be refreshed.
 */
void uip_ds6_nbr_refresh_reachable_state(const uip_ipaddr_t *ipaddr);
#endif /* UIP_ND6_SEND_NS */

/**
 * \brief
 *    This searches inside the neighbor table for the neighbor that is about to
 *    expire the next.
 *
 * \return
 *    A reference to the neighbor about to expire the next or NULL if
 *    table is empty.
 */
uip_ds6_nbr_t *uip_ds6_get_least_lifetime_neighbor(void);


static uint8_t nd6_opt_offset;                     /** Offset from the end of the icmpv6 header to the option in uip_buf*/
static uint8_t *nd6_opt_llao;   /**  Pointer to llao option in uip_buf */
static uip_ds6_nbr_t *nbr; /**  Pointer to a nbr cache entry*/
static uip_ds6_addr_t *addr; /**  Pointer to an interface address */


#endif /* UIP_DS6_NEIGHBOR_H_ */
/** @} */