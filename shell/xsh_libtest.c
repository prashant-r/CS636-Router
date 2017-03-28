/* xsh_rdstest.c - xsh_rdstest */
#include <xinu.h>
#include <stdio.h>

struct example_list_struct {
  struct example_list_struct *next;
  int number;
};

static struct example_list_struct element1, element2;
LIST(example_list);
bool8 example_list_test(void);

/*------------------------------------------------------------------------
 * xsh_libtest - shell to run tests related to lib folder
 *------------------------------------------------------------------------
 */
shellcmd xsh_libtest(int nargs, char *args[])
{
	bool8	err;
	int32	retval;
	int32	i, j;
	err = 1;
	err &= example_list_test();
	return err;
}


bool8 example_list_test(void)
{
  struct example_list_struct *s;
  list_create(example_list);
  element1.number = 1;
  list_add(example_list, &element1);
  element2.number = 2;
  list_add(example_list, &element2);
  int count = 1;
  listNode *node;
  listIter *iter;
  iter = list_iterator(example_list, FORWARD);
  while ((node = list_next(iter)) != NULL) {
  	struct example_list_struct * v = node->value;
    printf("List element number %d\n", v->number);
    if(v->number != count++)
    {
    	return 0;
    }
  }
  return 1;
}