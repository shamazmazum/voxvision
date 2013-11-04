/**
   @file list.h
   @brief Basic support for lists

   Searching for intersection with a ray and a tree and (maybe) other
**/

#ifndef _LIST_H_
#define _LIST_H_

/**
   \brief Cons cell
**/
typedef struct
{
    void *head; /**< Head object */
    void *tail; /**< Tail object */
} pair;

/**
   \brief Construct a pair of two objects.
**/
pair* cons (void*, void*);

/**
   \brief Get i-th element in list.
   \return The element or NULL if the list is NULL or too short
**/
void* nth (int, pair*);

/**
   \brief Get length of a list.
   \return i + length
**/
int length (pair*, int);

/**
   \brief Add the second list to the end of the first.
   
   This is a destructive operation.
   \return Concatenated list
**/
void nconc (pair*, pair*);

#endif
