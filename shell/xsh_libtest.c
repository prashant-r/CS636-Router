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
	if(err == 0)
	{
		kprintf("\nExample list test Failed.\n");
		return -1;
	}
	kprintf("\nExample list test passed.\n");
	return -1*~err;
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

  // Test iterator
  iter = list_iterator(example_list, FORWARD);
  while ((node = list_next(iter)) != NULL) {
  	struct example_list_struct * v = node->value;
    if(v->number != count++)
    {
    	return 0;
    }
  }

  if(list_length(example_list) != count)
  {
  	printf("\n list length failed. \n");
  	return 0;
  }

  // Test rewind
  list_rewind_tail(example_list, iter);

  // Test delete node
  list_del_node(example_list, list_next(iter));

  list_rewind(example_list, iter);

  // Test delete
  while ((node = list_next(iter)) != NULL) {
  	struct example_list_struct * v = node->value;
    if(v->number == count)
    {
    	printf(" \n delete failed. \n");
    	return 0;
    }
  }

  if(list_length(example_list) != count-1)
  {
  	printf("\n list length should have been 1. \n");
  	return 0;
  }


  // Test push functionality
  list_push(example_list, &element2);

  list_rewind(example_list, iter);
  count = 1;
  while ((node = list_next(iter)) != NULL) {
  	struct example_list_struct * v = node->value;
    if(v->number != count++)
    {
    	printf("\n list push test failed.\n");
    	return 0;
    }
  }
  // Test release iterator
  list_release_iterator(iter);

  // Test release
  list_release(example_list);
  return 1;
}