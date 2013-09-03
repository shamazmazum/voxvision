#ifndef _LIST_H_
#define _LIST_H_

/**
   Cons cell
**/
typedef struct
{
    void *head; /**< Head object */
    void *tail; /**< Tail object */
} pair;

pair* cons (void*, void*);
void* nth (int, pair*);
int length (pair*, int);
void nconc (pair*, pair*);

#endif
