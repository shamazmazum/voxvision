#include <assert.h>
#include <gc.h>
#include <stdlib.h>
#include "list.h"

pair* cons (void *obj1, void *obj2)
{
    pair *res = GC_MALLOC (sizeof(pair));
    res->head = obj1;
    res->tail = obj2;
    return res;
}

void* nth (int i, pair *list)
{
    assert (i>=0);

    if (list == NULL) return NULL;
    else if (i==0) return list->head;
    else return nth (i-1, list->tail);
}

int length (pair *list, int i)
{
    if (list == NULL) return i;
    else return length (list->tail, i+1);
}

void nconc (pair *list1, pair *list2)
{
    if (list1 == NULL) return;
    else if (list1->tail == NULL) list1->tail = list2;
    else nconc (list1->tail, list2);
}
