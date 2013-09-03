/**
   @file list.h
   @brief Basic support for lists

   Searching for intersection with a ray and a tree and (maybe) other
**/

#include <assert.h>
#include <gc.h>
#include <stdlib.h>
#include "list.h"

/**
   \brief Construct a pair of two objects.
**/
pair* cons (void *obj1, void *obj2)
{
    pair *res = GC_MALLOC (sizeof(pair));
    res->head = obj1;
    res->tail = obj2;
    return res;
}

/**
   \brief Get i-th element in list.
   \return The element or NULL if the list is NULL or too short
**/
void* nth (int i, pair *list)
{
    assert (i>=0);

    if (list == NULL) return NULL;
    else if (i==0) return list->head;
    else return nth (i-1, list->tail);
}

/**
   \brief Get length of a list.
   \return i + length
**/
int length (pair *list, int i)
{
    if (list == NULL) return i;
    else return length (list->tail, i+1);
}

/**
   \brief Add the second list to the end of the first.
   
   This is a destructive operation.
   \return Concatenated list
**/
void nconc (pair *list1, pair *list2)
{
    if (list1 == NULL) return;
    else if (list1->tail == NULL) list1->tail = list2;
    else nconc (list1->tail, list2);
}
