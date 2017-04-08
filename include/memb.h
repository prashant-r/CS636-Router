
#ifndef MEMB_H_
#define MEMB_H_

/**
 * Declare a memory block.
 *
 * This macro is used to statically declare a block of memory that can
 * be used by the block allocation functions. The macro statically
 * declares a C array with a size that matches the specified number of
 * blocks and their individual sizes.
 *
 * Example:
 \code
MEMB(connections, struct connection, 16);
 \endcode
 *
 * \param name The name of the memory block (later used with
 * memb_init(), memb_alloc() and memb_free()).
 *
 * \param structure The name of the struct that the memory block holds
 *
 * \param num The total number of memory chunks in the block.
 *
 */
#define MEMB(name, structure, num) \
        static char CC_CONCAT(name,_memb_count)[num]; \
        static structure CC_CONCAT(name,_memb_mem)[num]; \
        static struct memb name = {sizeof(structure), num, \
                                          CC_CONCAT(name,_memb_count), \
                                          (void *)CC_CONCAT(name,_memb_mem)}

struct memb {
  unsigned short size;
  unsigned short num;
  char *count;
  void *mem;
};

/**
 * Initialize a memory block that was declared with MEMB().
 *
 * \param m A memory block previously declared with MEMB().
 */
void  memb_init(struct memb *m);

/**
 * Allocate a memory block from a block of memory declared with MEMB().
 *
 * \param m A memory block previously declared with MEMB().
 */
void *memb_alloc(struct memb *m);

/**
 * Deallocate a memory block from a memory block previously declared
 * with MEMB().
 *
 * \param m m A memory block previously declared with MEMB().
 *
 * \param ptr A pointer to the memory block that is to be deallocated.
 *
 * \return The new reference count for the memory block (should be 0
 * if successfully deallocated) or -1 if the pointer "ptr" did not
 * point to a legal memory block.
 */
char  memb_free(struct memb *m, void *ptr);

int memb_inmemb(struct memb *m, void *ptr);

int  memb_numfree(struct memb *m);

/** @} */
/** @} */

#endif /* MEMB_H_ */