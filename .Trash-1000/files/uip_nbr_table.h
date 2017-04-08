#ifndef NBR_TABLE_H_
#define NBR_TABLE_H_

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

#endif /* NBR_TABLE_H_ */