#ifndef _MY_LIST_H
#define _MY_LIST_H

#define LIST_CONCAT2(s1, s2) s1##s2
#define LIST_CONCAT(s1, s2) LIST_CONCAT2(s1, s2)
#define LIST(name) \
         static void *LIST_CONCAT(name,_list) = NULL; \
         static list_t name = (list_t)&LIST_CONCAT(name,_list)

typedef void ** list_t;

typedef struct listNode {
    struct listNode *prev;
    struct listNode *next;
    void *value;
} listNode;

typedef struct listIter {
    listNode *next;
    int direction;
} listIter;

typedef struct list {
    listNode *head;
    listNode *tail;
    void *(*dup)(void *ptr);
    void (*free)(void *ptr);
    int (*match)(void *ptr, void *key);
    unsigned long len;
} list;

/* Functions implemented as macros */
#define listLength(l) ((l)->len)
#define listFirst(l) ((l)->head)
#define listLast(l) ((l)->tail)
#define listPrevNode(n) ((n)->prev)
#define listNextNode(n) ((n)->next)
#define listNodeValue(n) ((n)->value)

#define listSetDupMethod(l,m) ((l)->dup = (m))
#define listSetFreeMethod(l,m) ((l)->free = (m))
#define listSetMatchMethod(l,m) ((l)->match = (m))

#define listGetDupMethod(l) ((l)->dup)
#define listGetFree(l) ((l)->free)
#define listGetMatchMethod(l) ((l)->match)

/* Prototype for list_t*/
void list_create(list_t list);
void list_release(list_t list);
void list_pop(list_t list);
void list_push(list_t list, void * item);
void list_add_node_head(list_t list, void *value);
void list_add(list_t list, void *value);

void list_del_node(list_t list, listNode * node);
void list_insert(list_t list, void * prevItem, void * newitem);
void list_duplicate(list_t dest, list_t src);
int  list_length(list_t list);
listIter * list_iterator(list_t list, int direction);
listNode * list_next(listIter * iter);
void list_release_iterator(listIter * iter);
listNode * list_search_key(list_t list, void *key);
listNode * list_index_at(list_t list, long index);
void list_rewind(list_t list, listIter *li);
void list_rewind_tail(list_t list, listIter * li);
void list_rotate(list_t list);

/* Prototypes */
list *listCreate(void);
void listRelease(list *list);
list *listAddNodeHead(list *list, void *value);
list *listAddNodeTail(list *list, void *value);
list *listInsertNode(list *list, listNode *old_node, void *value, int after);
void listDelNode(list *list, listNode *node);
listIter *listGetIterator(list *list, int direction);
listNode *listNext(listIter *iter);
void listReleaseIterator(listIter *iter);
list *listDup(list *orig);
listNode *listSearchKey(list *list, void *key);
listNode *listIndex(list *list, long index);
void listRewind(list *list, listIter *li);
void listRewindTail(list *list, listIter *li);
void listRotate(list *list);

/* Directions for iterators */
#define FORWARD 0
#define BACKWARD 1

#endif