/* queue.c
 * Fns. for manipulating queue objects
 */

#include "defs.h"
#include "ext.h"

/* Make a queue containing nobjs objects of size size.  Return a pointer to
 * the queue or NULL if it could not be created.
 */
queue  *
new_queue (int size, int nobjs)
{
    queue  *q;

    if (!(q = (queue *) calloc (1, sizeof (queue) + size * nobjs)))
        return (queue *) NULL;

    q->start = (char *) (q + 1);
    q->size = nobjs;
    q->objsize = size;
    q->head = q->tail = q->nobjs = 0;

    return q;
}

/* Insert an object into the queue.  obj is a pointer to the object, and
 * q is a pointer to the queue.  Returns 1 on success, 0 if the queue is
 * full.
 */
int
push_queue (char *obj, queue * q)
{
    int     i;
    char   *p;                  /* Pointer into the queue */

    if (q->nobjs >= q->size)    /* Is the queue full? */
        return 0;

    q->nobjs++;

    /* Find the target address within the queue to insert object. */
    p = q->start + q->objsize * q->tail;

#ifdef DEBUG
    std_printf ("{Queuing %s, %d objects} ", obj, q->nobjs);
#endif
    /* Copy the object into its queue position */
    for (i = q->objsize; --i >= 0; *p++ = *obj++) ;

    /* Wrap around if we've gone past the end of the queue. */
    if (++q->tail >= q->size)
        q->tail = 0;

    return 1;
}


/* Remove an object from the queue.  q is a pointer to the queue.  The object
 * is copied into the location pointed to by obj.  Returns 0 if the queue is
 * empty, 1 on success.
 */
int
pop_queue (char *obj, queue * q)
{
    int     i;
    char   *p;                  /* Pointer into the queue */

    if (q->nobjs <= 0)
        return q->nobjs = 0;    /* Queue is empty */

    q->nobjs--;                 /* Removing an object... */

    /* Find the object within the queue. */
    p = q->start + (q->objsize * q->head);

#ifdef DEBUG
    std_printf ("{Dequeuing %s, %d objects}\r\n", p, q->nobjs);
#endif
    /* Copy the object. */
    for (i = q->objsize; --i >= 0; *obj++ = *p++) ;

    if (++q->head >= q->size)
        q->head = 0;

    return 1;
}


/* is_queued checks to see if a character string is currently queued.
 * Returns 1 if the string is queued, 0 if not.
 */
int
is_queued (char *obj, queue * q)
{
    char   *p;                  /* Pointer inside queue */
    int     i;                  /* Object counter */

    /* Move to head of queue. */
    for (p = q->start + (q->objsize * q->head), i = 0; i < q->nobjs; i++) {
        /* Do the comparison. */
        if (!strcmp (p, obj))
            return 1;
        p += q->objsize;
        if (p >= (char *) (q->start + (q->objsize * q->size)))
            p = q->start;
    }
    return 0;
}
