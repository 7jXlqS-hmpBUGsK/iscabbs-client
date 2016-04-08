#include "defs.h"
#include "ext.h"

// The memory for UserEntry objects is also owned by the list.

UserEntry *
new_UserEntry (const char *name)
{
    UserEntry *u = (UserEntry *) calloc (1, sizeof (UserEntry));

    if (name)
        strncpy (u->name, name, sizeof (u->name) - 1);
    return u;
}

UserEntry *
ulist_insert (UList * L, const char *name)
{
    UserEntry *u = ulist_find (L, name);

    if (u)
        return u;
    u = new_UserEntry (name);
    ++L->sz;
    L->arr = (UserEntry **) realloc (L->arr, L->sz * sizeof (UserEntry *));
    L->arr[L->sz - 1] = u;
    return u;
}

UserEntry *
ulist_find (UList * L, const char *name)
{
    if (name)
        for (size_t i = 0; i != L->sz; ++i)
            if (strcmp (L->arr[i]->name, name) == 0)
                return L->arr[i];
    return NULL;
}

bool
ulist_erase (UList * L, const char *name)
{
    // Assume it's only in the list once.
    for (UserEntry ** i = L->arr, **E = L->arr + L->sz; i != E; ++i)
        if (strcmp ((*i)->name, name) == 0) {
            free (*i);
            // shift the remaining entries down 
            memmove (i, i + 1, (E - (i + 1)) * sizeof (UserEntry *));
            --L->sz;
            return true;
        }
    return false;
}

void
ulist_clear (UList * L)
{
    for (UserEntry ** i = L->arr, **E = L->arr + L->sz; i != E; ++i)
        free (*i);
    L->sz = 0;
    free (L->arr);
    L->arr = NULL;
}

static int
cmp_by_name_ (const UserEntry ** a, const UserEntry ** b)
{
    return strcmp ((*a)->name, (*b)->name);
}

static int
cmp_by_time_ (const UserEntry ** a, const UserEntry ** b)
{
    // careful to avoid overflow in time_t
    // note we sort in reverse order
    return (*a)->login_tm < (*b)->login_tm ? +1 : ((*a)->login_tm > (*b)->login_tm ? -1 : 0);
}

typedef int (*qsort_callback_fn_) (const void *, const void *);

void
ulist_sort_by_name (UList * L)
{
    qsort (L->arr, L->sz, sizeof (L->arr[0]), (qsort_callback_fn_) cmp_by_name_);
}

void
ulist_sort_by_time (UList * L)
{
    qsort (L->arr, L->sz, sizeof (L->arr[0]), (qsort_callback_fn_) cmp_by_time_);
}

/* vim:set expandtab cindent tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0: */
