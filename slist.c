/*
 * slist.c - Functions which maintain a sorted (non-linked) list of
 * arbitrary data.
 */

#include "defs.h"
#include "ext.h"
#include <stdarg.h>


/*
 * slistCreate creates a list with the given number of items already
 * allocated.  If the number of items is >0, the pointers must be
 * passed as arguments.
 */
slist *slistCreate(int nitems, int (*sortfn) (), ...)
{
    int i;
    slist *list;
    va_list ap;

    assert(nitems >= 0);
    assert(sortfn);

    if (!(list = (slist *) calloc(1, sizeof(slist))))
	return NULL;
    list->nitems = nitems;
    list->sortfn = sortfn;
    if (nitems > 0) {
	if (!(list->items = (void *) calloc(1, nitems * sizeof(void *))))
	     return NULL;
	va_start(ap, sortfn);
	for (i = 0; i < nitems; i++)
	    list->items[i] = va_arg(ap, void *);
	va_end(ap);
    } else
	list->items = NULL;
    return list;
}


/*
 * slistDestroy destroys a list.  It does not destroy the data items
 * in the list.
 */
void slistDestroy(list)
slist *list;
{
    free(list->items);
    list->items = NULL;
    free(list);
    list = NULL;
}


/*
 * slistDestroyItems destroys the data items in a list.
 */
void slistDestroyItems(list)
slist *list;
{
    int i;

    for (i = 0; i < list->nitems; i++) {
	free(list->items[i]);
	list->items[i] = NULL;
    }
}


/*
 * slistAddItem adds an item to the list.
 */
int slistAddItem(list, item, deferSort)
slist *list;
void *item;
int deferSort;
{
    void **p;

    list->nitems++;
    if (!(p = (void *) realloc(list->items, list->nitems * sizeof(void *))))
	 return 0;
    list->items = p;
    list->items[list->nitems - 1] = item;
    if (!deferSort)
	slistSort(list);
    return 1;
}


/*
 * slistRemoveItem removes an item from the list.  It does not free the
 * object being pointed to.
 */
int slistRemoveItem(list, item)
slist *list;
int item;
{
    void **p;
    int i;

    assert(list);
    assert(item >= 0);
    assert(item < list->nitems);

printf("slistRemoveItem(list, %d): nitems=%d\r\n", item, list->nitems);
    list->items[item] = NULL;
    if (item < --list->nitems)
	for (i = item; i < list->nitems; i++)
	    list->items[i] = list->items[i + 1];
    p = (void *) realloc(list->items, list->nitems * sizeof(void *));
    if (!p && list->nitems)	/* request failed */
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
int slistFind(slist *list, void *toFind, int (*findfn) (const void *, const void *))
{
    int i, upper, lower, k;

    assert(list);
    assert(findfn);

    if (!toFind)		/* Fail if nothing to find */
	return -1;
    upper = list->nitems - 1;
    lower = 0;
    while (upper >= lower) {
	i = (upper + lower) / 2;
	k = findfn(toFind, list->items[i]);
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
void slistSort(list)
slist *list;
{
    assert(list);

    qsort(list->items, list->nitems, sizeof(void *), list->sortfn);
}


/*
 * slistIntersection creates the intersection of two slists, that is, the
 * list of items which appear on both lists.  We presume that we can create
 * the intersection if and only if the two lists use the same sortfn.  Data
 * members are copied using a shallow copy from list1, since this is typically
 * used for "throwaway" lists...  This function is designed to run in linear
 * linear time for list1 + list2.  Returns the list, an empty list if there
 * are no intersecting items, or NULL on error.  Do NOT destroy the list
 * items; they don't belong to you!
 */
slist *slistIntersection(list1, list2)
	const slist *list1;
	const slist *list2;
{
	int n1;		/* Count of items processed */
	int n2;		/* Count of items processed */
	slist *dest;	/* The list being created */

	assert(list1);
	assert(list2);
	assert(list1->sortfn == list2->sortfn);

	if (!list1 || !list2 || list1->sortfn != list2->sortfn) return NULL;

	n1 = 0;
	n2 = 0;

	dest = slistCreate(0, list1->sortfn);
	if (!dest) return NULL;

	for (; n1 < list1->nitems; n1++) {
		/*
		 * Now run through list2 until we find either a matching
		 * item, or an item that is greater than the one in list1
		 * that we are currently looking at.
		 */
		int r;

		/* First item in n2 not less than current item n1 */
		while ((r = dest->sortfn(list1->items[n1], list2->items[n2])) < 0) {
			n2++;
			/* If this happens, we're done */
			if (n2 > list2->nitems)
				break;
		}

		/* If this happens, we're done; nothing else will match */
		if (n2 > list2->nitems)
			break;

		/* If item is not less than and not greater than, it's equal */
		if (!(dest->sortfn(list2->items[n2], list1->items[n1]) < 0)) {
			if (!slistAddItem(dest, list1->items[n1], 1)) {
				slistDestroy(dest);
				return NULL;
			}
		}
	}
	slistSort(dest);
	return dest;
}
