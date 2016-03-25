/*
 * slist.c - Functions which maintain a sorted (non-linked) list of
 * arbitrary data.
 */

#include "defs.h"
#include "ext.h"
#include <stdarg.h>

/*
 * slistCreate creates an empty list.
 */
slist  *
slistCreate (int (*sortfn) (const void *, const void *))
{
    slist  *list;

    assert (sortfn);

    if (!(list = (slist *) calloc (1, sizeof (slist))))
        return NULL;
    list->sortfn = sortfn;
    return list;
}


/*
 * slistDestroy destroys a list.  It does not destroy the data items
 * in the list.
 */
void
slistDestroy (slist * list)
{
    free (list->items);
    list->items = NULL;
    free (list);
    list = NULL;
}


/*
 * slistDestroyItems destroys the data items in a list.
 */
void
slistDestroyItems (slist * list)
{
    for (size_t i = 0; i < list->nitems; i++) {
        free (list->items[i]);
        list->items[i] = NULL;
    }
}


/*
 * slistAddItem adds an item to the list.
 */
int
slistAddItem (slist * list, void *item, int deferSort)
{
    void  **p;

    list->nitems++;
    if (!(p = (void *) realloc (list->items, list->nitems * sizeof (void *))))
        return 0;
    list->items = p;
    list->items[list->nitems - 1] = item;
    if (!deferSort)
        slistSort (list);
    return 1;
}


/*
 * slistRemoveItem removes an item from the list.  It does not free the
 * object being pointed to.
 */
int
slistRemoveItem (slist * list, int item)
{
    void  **p;

    assert (list);
    assert (item >= 0);
    assert (item < (int) list->nitems);

    printf ("slistRemoveItem(list, %d): nitems=%zd\r\n", item, list->nitems);
    list->items[item] = NULL;
    if ((size_t) item < --list->nitems)
        for (size_t i = item; i < list->nitems; i++)
            list->items[i] = list->items[i + 1];
    p = (void *) realloc (list->items, list->nitems * sizeof (void *));
    if (!p && list->nitems)     /* request failed */
        return 0;

    list->items = p;
    return 1;
}


/* 
 * slistFind locates an item based on the results of the search function.
 * Returns entry on success, -1 on failure.  findfn() should compare a to b
 * and return <0 if a < b, >0 if a > b, or 0 if a == b.
 * Algorithm is a binary search.
 */
int
slistFind (slist * list, void *toFind, int (*findfn) (const void *, const void *))
{
    int     i, upper, lower, k;

    assert (list);
    assert (findfn);

    if (!toFind)                /* Fail if nothing to find */
        return -1;
    upper = list->nitems - 1;
    lower = 0;
    while (upper >= lower) {
        i = (upper + lower) / 2;
        k = findfn (toFind, list->items[i]);
        if (k == 0)
            return i;
        if (k < 0)
            upper = i - 1;
        else
            lower = i + 1;
    }
    return -1;
}


/*
 * slistSort sorts the list using a bubble sort.  TODO:  This is slow as hell
 * and really should be reworked to be faster.  Sometime....  sortfn should
 * compare a to b and return <0 if a < b, >0 if a > b, or 0 if a == b.
 */
void
slistSort (slist * list)
{
    assert (list);

    qsort (list->items, list->nitems, sizeof (void *), list->sortfn);
}
