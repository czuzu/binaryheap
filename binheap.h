/*
 * Copyright (C) 2017 Corneliu Zuzu <zuzelzzz@gmail.com>.
 *
 * binheap.h
 *
 * Thread-unsafe binary-heap implementation.
 * Customizable (cross-compiler, cross-environment).
 * Linux kernel coding-style.
 * Applications: priority-queue, scheduler.
 * Can be used as minimum-heap or maximum-heap.
 * Only grows on realloc, doesn't shrink back.
 * Pointers not checked, user responsible to ensure non-NULL values.
 *
 *  Created on: Feb 21, 2017
 *      Author: corneliu.zuzu
 */

#ifndef _DS_BINHEAP_H_
#define _DS_BINHEAP_H_

#ifndef BINHEAP_malloc
#error "BINHEAP_malloc must be defined."
#endif

#ifndef BINHEAP_free
#error "BINHEAP_free must be defined."
#endif

#ifndef BINHEAP_realloc
#error "BINHEAP_realloc must be defined."
#endif

#ifndef BINHEAP_memset
#error "BINHEAP_memset must be defined."
#endif

#ifndef BINHEAP_assert
#error "BINHEAP_assert must be defined."
#endif

#ifndef BINHEAP_inline
#error "BINHEAP_inline must be defined."
#endif

#ifndef BINHEAP_likely
#error "BINHEAP_likely must be defined."
#endif

#ifndef BINHEAP_unlikely
#error "BINHEAP_unlikely must be defined."
#endif

#ifndef BINHEAP_ENOMEM
#error "BINHEAP_ENOMEM must be defined."
#endif

/* Formulas for obtaining indexes for left-child, right-child & parent. */
#define __BH_LEFT(i)                ((i << 1ul) + 1)
#define __BH_RIGHT(i)               ((i << 1ul) + 2)
#define __BH_PARENT(i)              ((i - 1) >> 1ul)

/* Offset of member MEMBER in a struct of type TYPE. */
#define __BH_OFFSET_OF(TYPE, MEMBER) __builtin_offsetof(TYPE, MEMBER)
#define __BH_CONTAINER_OF(ptr, type, member)                        \
    ({                                                              \
        const __typeof__(((type *) 0)->member) *__mptr = (ptr);     \
        (type *)((char *) __mptr - __BH_OFFSET_OF(type, member));   \
    })

struct binheap_entry
{
    unsigned int idx;
};

typedef unsigned int (*binheap_cmp_ok_fn)(struct binheap_entry* p,
										  struct binheap_entry* c);

struct binheap
{
    /* 'size' = number of elements the binary-heap currently has. */
    unsigned int size;

    /*
     * 'capacity' = how many elements can we insert without having to
     * realloc 'nodes'. At any given moment, the allocated size
     * of 'nodes' is that of ('capacity' + 1) elements.
     * As soon as 'size' becomes equal to 'capacity' (due to
     * inserting elements), 'nodes' is reallocated.
     */
    unsigned int capacity;

    /*
     * Growth factor, ratio and increment = when reallocating, the new
     * 'capacity' value is obtained with the sequence:
     *      old_cap = capacity;
     *      new_cap = (old_cap * (1 + factor)) / (1 + ratio);
     *      new_cap += increment;
     *      if ( unlikely(new_cap == old_cap) ) new_cap++;
     *
     * E.g. for a capacity that always doubles, set factor to 1 and
     * both factor and increment to 0.
     */
    unsigned int growth_factor;
    unsigned int growth_ratio;
    unsigned int growth_increment;

    struct binheap_entry** nodes;
};

/* Initializes a binary-heap. */
static BINHEAP_inline
int binheap_init(struct binheap* h,
                 unsigned int cap,
                 unsigned int growth_factor,
                 unsigned int growth_ratio,
                 unsigned int growth_increment)
{
    size_t alloc_sz;

    /* Allocate nodes. */
    alloc_sz = (cap + 1) * sizeof(struct binheap_entry*);
    h->nodes = (struct binheap_entry**) BINHEAP_malloc(alloc_sz);

    if ( BINHEAP_unlikely(h->nodes == NULL) )
        return -BINHEAP_ENOMEM;

    /* Initialize parameters. */
    h->size = 0;
    h->capacity = cap;
    h->growth_factor = growth_factor;
    h->growth_ratio = growth_ratio;
    h->growth_increment = growth_increment;

    return 0;
}

/* Destroys a binary-heap. */
static BINHEAP_inline
void binheap_destroy(struct binheap* h)
{
    BINHEAP_free(h->nodes);
    BINHEAP_memset(h, 0, sizeof(struct binheap));
}

static BINHEAP_inline
void __binheap_grow_capacity(struct binheap* h)
{
    size_t realloc_sz;
    struct binheap_entry** new_nodes;
    unsigned int new_cap;

    /* Compute new capacity. */
    new_cap = h->capacity * (1 + h->growth_factor);
    new_cap /= (1 + h->growth_ratio);
    new_cap += h->growth_increment;
    if ( BINHEAP_unlikely(new_cap <= h->capacity) )
        new_cap = h->capacity + 1;

    /* Realloc. */
    realloc_sz = (1 + new_cap) * sizeof(struct binheap_entry*);
    new_nodes = BINHEAP_realloc(h->nodes, realloc_sz);

    /* Realloc failed? */
    if ( BINHEAP_unlikely(new_nodes == NULL) )
        return;

    /* Update binheap. */
    h->capacity = new_cap;
    h->nodes = new_nodes;
}

static BINHEAP_inline
void __binheap_add_last(struct binheap* h, struct binheap_entry* e)
{
    if ( BINHEAP_unlikely(h->size == h->capacity) )
    {
        unsigned int old_cap = h->capacity;
        __binheap_grow_capacity(h);

        /* Realloc failed? */
        if ( BINHEAP_unlikely(h->capacity == old_cap) )
            return;
    }

    h->nodes[e->idx = h->size] = e;
    h->size++;
}

static BINHEAP_inline
void __binheap_up(struct binheap* h,
                  struct binheap_entry* e,
                  binheap_cmp_ok_fn cmp_ok)
{
    struct binheap_entry* p;
    unsigned int eidx, pidx;

    eidx = e->idx;

    do
    {
        /* Stop if the element became root. */
        if ( BINHEAP_unlikely(eidx == 0) )
            break;

        /* Stop if the inserted element finally has the proper parent. */
        pidx = __BH_PARENT(eidx);
        p = h->nodes[pidx];
        if ( BINHEAP_unlikely(cmp_ok(p, e)) )
            break;

        /* Swap with parent. */
        p->idx = eidx;
        h->nodes[eidx] = p;
        h->nodes[eidx = pidx] = e;
    } while ( 1 );

    e->idx = eidx;
}

static BINHEAP_inline
void __binheap_down(struct binheap* h,
                    struct binheap_entry* e,
                    binheap_cmp_ok_fn cmp_ok)
{
    unsigned int eidx, lidx, ridx, swapidx;

    eidx = e->idx;

    do
    {
        /* Stop if the element became leaf (<=> has no left child). */
        lidx = __BH_LEFT(eidx);
        if ( BINHEAP_unlikely(lidx >= h->size) )
            break;

        /* Stop if the element is finally a proper parent. */
        ridx = __BH_RIGHT(eidx);
        if ( BINHEAP_likely(ridx < h->size) &&
             cmp_ok(h->nodes[ridx], h->nodes[lidx]) )
            swapidx = ridx;
        else
            swapidx = lidx;

        if ( BINHEAP_unlikely(cmp_ok(e, h->nodes[swapidx])) )
            break;

        /* Swap with the child that can be the parent of the other. */
        h->nodes[swapidx]->idx = eidx;
        h->nodes[eidx] = h->nodes[swapidx];
        h->nodes[eidx = swapidx] = e;
    } while ( 1 );

    e->idx = eidx;
}

/*
 * Inserts an entry into the binary-heap.
 * Note: cmp_ok(p, c) must return 1 if the parent p and his child c are in
 *       the proper order relative to each-other in the binary-heap.
 *       To expand on that, in a min-heap cmp_ok must return 1 when p <= c
 *       and in a max-heap it must return 1 inversely, i.e. when p >= c.
 */
static BINHEAP_inline
int binheap_insert(struct binheap* h,
                   struct binheap_entry* e,
                   binheap_cmp_ok_fn cmp_ok)
{
    unsigned int old_size;

    old_size = h->size;
    __binheap_add_last(h, e);
    /* Realloc failed? */
    if ( BINHEAP_unlikely(h->size == old_size) )
        return -BINHEAP_ENOMEM;

    __binheap_up(h, e, cmp_ok);

    return 0;
}

/* Deletes the specified entry from the heap. */
static BINHEAP_inline
void binheap_delete(struct binheap* h,
                    struct binheap_entry* e,
                    binheap_cmp_ok_fn cmp_ok)
{
    unsigned int eidx, pidx;

    eidx = e->idx;

    BINHEAP_assert(eidx < h->size);
    h->size--;

    /* When the last element is removed, nothing more need be done. */
    if ( BINHEAP_unlikely(eidx == h->size) )
        return;

    /* Overwrite with last. */
    e = h->nodes[h->size];
    h->nodes[e->idx = eidx] = e;

    pidx = __BH_PARENT(eidx);

    /*
     * Case 1: the element and his parent are in the proper
     *         order or the element is root -> do heap-down.
     */
    if ( BINHEAP_likely(eidx == 0) || cmp_ok(h->nodes[pidx], e) )
        __binheap_down(h, e, cmp_ok);

    /*
     * Case 2: the element and the child that could be the parent of
     *         the other child are in the proper order -> do heap-up.
     */
    else
        __binheap_up(h, e, cmp_ok);
}

/* Returns a pointer to the root element of the heap entry. */
static BINHEAP_inline
struct binheap_entry* binheap_get_root(struct binheap* h)
{
    return (h->nodes[0]);
}

/* Deletes the heap's root entry. */
static BINHEAP_inline void binheap_delete_root(struct binheap* h,
                                               binheap_cmp_ok_fn cmp_ok)
{
    binheap_delete(h, binheap_get_root(h), cmp_ok);
}

/**
 * binheap_entry - get the struct for this entry
 * @ptr:    the &struct binheap_entry pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the binheap_entry member within the struct.
 */
#define binheap_get_capsule(ptr, type, member) \
            __BH_CONTAINER_OF(ptr, type, member)

#endif /* _DS_BINHEAP_H_ */
