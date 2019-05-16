/*
 * This file Copyright (C) 2008-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: ptrarray.h 14241 2014-01-21 03:10:30Z jordan $
 */

#ifndef __TRANSMISSION__
 #error only libtransmission should #include this header.
#endif

#ifndef _TR_PTR_ARRAY_H_
#define _TR_PTR_ARRAY_H_

#include <assert.h>

#include "transmission.h"

/**
 * @addtogroup utils Utilities
 * @{
 */

/**
 * @brief simple pointer array that resizes itself dynamically.
 */
typedef struct tr_ptrArray
{
    void ** items;
    int     n_items;
    int     n_alloc;
}
tr_ptrArray;

typedef int (*PtrArrayCompareFunc)(const void * a, const void * b);

typedef void (*PtrArrayForeachFunc)(void *);

#define TR_PTR_ARRAY_INIT_STATIC { NULL, 0, 0 }

extern const tr_ptrArray TR_PTR_ARRAY_INIT;

/** @brief Destructor to free a tr_ptrArray's internal memory */
void tr_ptrArrayDestruct (tr_ptrArray*, PtrArrayForeachFunc func);

/** @brief Iterate through each item in a tr_ptrArray */
void tr_ptrArrayForeach (tr_ptrArray         * array,
                         PtrArrayForeachFunc   func);

/** @brief Return the nth item in a tr_ptrArray
    @return the nth item in a tr_ptrArray */
static inline void*
tr_ptrArrayNth (tr_ptrArray * array, int i)
{
    assert (array);
    assert (i >= 0);
    assert (i < array->n_items);

    return array->items[i];
}

/** @brief Remove the last item from the array and return it
    @return the pointer that's been removed from the array
    @see tr_ptrArrayBack () */
void* tr_ptrArrayPop (tr_ptrArray * array);

/** @brief Return the last item in a tr_ptrArray
    @return the last item in a tr_ptrArray, or NULL if the array is empty
    @see tr_ptrArrayPop () */
static inline void* tr_ptrArrayBack (tr_ptrArray * array)
{
    return array->n_items > 0 ? tr_ptrArrayNth (array, array->n_items - 1)
                              : NULL;
}

void tr_ptrArrayErase (tr_ptrArray * t, int begin, int end);

static inline void tr_ptrArrayRemove (tr_ptrArray * t, int pos)
{
    tr_ptrArrayErase (t, pos, pos+1);
}

/** @brief Peek at the array pointer and its size, for easy iteration */
void** tr_ptrArrayPeek (tr_ptrArray * array, int * size);

static inline void  tr_ptrArrayClear (tr_ptrArray * a) { a->n_items = 0; }

/** @brief Insert a pointer into the array at the specified position
    @return the index of the stored pointer */
int tr_ptrArrayInsert (tr_ptrArray * array, void * insertMe, int pos);

/** @brief Append a pointer into the array */
static inline int tr_ptrArrayAppend (tr_ptrArray * array, void * appendMe)
{
    return tr_ptrArrayInsert (array, appendMe, -1);
}

static inline void** tr_ptrArrayBase (const tr_ptrArray * a)
{
    return a->items;
}

/** @brief Return the number of items in the array
    @return the number of items in the array */
static inline int tr_ptrArraySize (const tr_ptrArray *  a)
{
    return a->n_items;
}

/** @brief Return True if the array has no pointers
    @return True if the array has no pointers */
static inline bool tr_ptrArrayEmpty (const tr_ptrArray * a)
{
    return tr_ptrArraySize (a) == 0;
}

int tr_ptrArrayLowerBound (const tr_ptrArray * array,
                           const void * key,
                           int compare (const void * arrayItem, const void * key),
                           bool * exact_match);

/** @brief Insert a pointer into the array at the position determined by the sort function
    @return the index of the stored pointer */
int tr_ptrArrayInsertSorted (tr_ptrArray * array,
                             void        * value,
                             int compare (const void*, const void*));

/** @brief Remove this specific pointer from a sorted ptrarray */
void tr_ptrArrayRemoveSortedPointer (tr_ptrArray * t,
                                     const void  * ptr,
                                     int           compare (const void*, const void*));


/** @brief Find a pointer from an array sorted by the specified sort function
    @return the matching pointer, or NULL if no match was found */
void* tr_ptrArrayFindSorted (tr_ptrArray * array,
                             const void  * key,
                             int compare (const void*, const void*));

/* @} */
#endif
