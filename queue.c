/* queue.c
 * Fns. for manipulating queue objects
 */

#include "defs.h"
#include "ext.h"

struct queue {
    size_t  head;               /* Index of current head */
    size_t  cap;                /* Number of objects queue can hold */
    size_t  nobjs;              /* Number of objects queued */
    char   *arr[0];             /* Pointer to beginning of queue */
};

#define QSLOT(I) (q->arr[(q->head + (I)) % q->cap])

static void
queue_invariant_ (const queue * q)
{
    if (q) {
        assert (q->head < q->cap);
        assert (q->nobjs <= q->cap);
        // all used slots are non-NULL
        for (size_t i = 0; i != q->nobjs; ++i)
            assert (QSLOT (i));
        // all un-used slots are NULL
        for (size_t i = q->nobjs; i != q->cap; ++i)
            assert (QSLOT (i) == NULL);
    }
}

void
delete_queue (queue * q)
{
    if (q) {
        for (size_t i = 0; i != q->nobjs; ++i)
            free (QSLOT (i));
        free (q);
    }
}

queue  *
new_queue (size_t max_nobjs)
{
    queue  *q = (queue *) calloc (sizeof (queue) + max_nobjs * sizeof (char *), 1);

    q->cap = max_nobjs;
    queue_invariant_ (q);
    return q;
}

size_t
queue_size (const queue * q)
{
    return q->nobjs;
}

const char *
queue_at (const queue * q, size_t i /*logical index */ )
{
    return QSLOT (i);
}

/* Insert an object into the queue.  obj is a pointer to the object, and
 * q is a pointer to the queue.  Returns 1 on success, 0 if the queue is
 * full.
 */
bool
push_queue (const char *obj, queue * q)
{
    if (q->nobjs >= q->cap)     /* Is the queue full? */
        return false;

    /* Copy the object into its queue position */
    QSLOT (q->nobjs) = strdup (obj);
    q->nobjs++;

    queue_invariant_ (q);
    return true;
}


/* Remove an object from the queue.  q is a pointer to the queue.  The object
 * is copied into the location pointed to by obj.  Returns 0 if the queue is
 * empty, 1 on success.
 */
bool
pop_queue (char *obj, queue * q)
{
    if (q->nobjs == 0)
        return false;

    /* Copy the object. */
    strcpy (obj, q->arr[q->head]);
    free (q->arr[q->head]);
    q->arr[q->head] = NULL;
    q->nobjs--;

    if (++q->head >= q->cap)
        q->head = 0;

    queue_invariant_ (q);
    return true;
}


/* is_queued checks to see if a character string is currently queued.
 * Returns 1 if the string is queued, 0 if not.
 */
bool
is_queued (const char *obj, const queue * q)
{
    for (size_t i = 0; i != q->nobjs; ++i)
        if (!strcmp (obj, QSLOT (i)))
            return true;
    queue_invariant_ (q);
    return false;
}
