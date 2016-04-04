/* queue.c
 * Fns. for manipulating queue objects
 */

#include "defs.h"
#include "ext.h"

/* Make a queue containing nobjs objects of size size.  Return a pointer to
 * the queue or NULL if it could not be created.
 */
queue  *
new_queue (size_t max_nobjs)
{
    queue  *q = (queue *) calloc (sizeof (queue) + max_nobjs * sizeof (char *), 1);

    q->size = max_nobjs;
    return q;
}

/* Insert an object into the queue.  obj is a pointer to the object, and
 * q is a pointer to the queue.  Returns 1 on success, 0 if the queue is
 * full.
 */
bool
push_queue (const char *obj, queue * q)
{
    if (q->nobjs >= q->size)    /* Is the queue full? */
        return false;

#ifdef DEBUG
    std_printf ("{Queuing %s, %d objects} ", obj, q->nobjs);
#endif
    /* Copy the object into its queue position */
    q->arr[(q->head + q->nobjs) % q->size] = strdup (obj);
    q->nobjs++;

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

#ifdef DEBUG
    std_printf ("{Dequeuing %s, %d objects}\r\n", p, q->nobjs);
#endif
    /* Copy the object. */
    strcpy (obj, q->arr[q->head]);
    free (q->arr[q->head]);
    q->arr[q->head] = NULL;
    q->nobjs--;

    if (++q->head >= q->size)
        q->head = 0;

    return true;
}


/* is_queued checks to see if a character string is currently queued.
 * Returns 1 if the string is queued, 0 if not.
 */
bool
is_queued (const char *obj, const queue * q)
{
    for (size_t i = 0; i != q->nobjs; ++i)
        if (!strcmp (obj, q->arr[(q->head + i) % q->size]))
            return true;
    return false;
}
