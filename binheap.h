/*
 * Copyright (C) 2017 Corneliu Zuzu <zuzelzzz@gmail.com>.
 *
 * binheap.h
 *
 * Lock-less binary-heap implementation. Linux kernel coding-style.
 * Applications: priority-queue, scheduler.
 * Can be used as minimum-heap or maximum-heap.
 * Uses GCC statement-expressions extension.
 * External use of '__'-preceded macros is not recommended.
 * Only grows on realloc, doesn't shrink back.
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

#ifndef BINHEAP_likely
#error "BINHEAP_likely must be defined."
#endif

#ifndef BINHEAP_unlikely
#error "BINHEAP_unlikely must be defined."
#endif

/* Instantiates a binary-heap typename, i.e. a 'class'. */
#define BINHEAP_TEMPLATE_INSTANTIATE(binheap_struct_name, T)                \
struct binheap_struct_name                                                  \
{                                                                           \
    /* 'size' = number of elements the binary-heap currently has. */        \
    unsigned int size;                                                      \
                                                                            \
    /*                                                                      \
     * 'capacity' = how many elements can we insert without having to       \
     * realloc 'nodes'. At any given moment, the allocated size             \
     * of 'nodes' is that of ('capacity' + 1) elements.                     \
     * As soon as 'size' becomes equal to 'capacity' (due to                \
     * inserting elements), 'nodes' is reallocated.                         \
     */                                                                     \
    unsigned int capacity;                                                  \
                                                                            \
    /*                                                                      \
     * Growth factor, ratio and increment = when reallocating, the new      \
     * 'capacity' value is obtained with the sequence:                      \
     *      old_cap = capacity;                                             \
     *      new_cap = (old_cap * (1 + factor)) / (1 + ratio);               \
     *      new_cap += increment;                                           \
     *      if ( unlikely(new_cap == old_cap) ) new_cap++;                  \
     *                                                                      \
     * E.g. for a capacity that always doubles, set factor to 1 and         \
     * both factor and increment to 0.                                      \
     */                                                                     \
    unsigned int growth_factor;                                             \
    unsigned int growth_ratio;                                              \
    unsigned int growth_increment;                                          \
                                                                            \
    T* nodes;                                                               \
}

/* Gets the type of an instantiated heap. */
#define BINHEAP_TYPE(h)             __typeof__((h))

/* Gets the type of the binary-heap elements. */
#define BINHEAP_ELEM_TYPE(h)        __typeof__((h).nodes[0])

/* Gets the size of a single binary-heap element. */
#define BINHEAP_ELEM_SIZE(h)        sizeof(BINHEAP_ELEM_TYPE(h))

/* Formulas for obtaining indexes for left-child, right-child & parent. */
#define __BH_LEFT(i)                ((i << 1ul) + 1)
#define __BH_RIGHT(i)               ((i << 1ul) + 2)
#define __BH_PARENT(i)              ((i - 1) >> 1ul)

/* Initializes a previously declared binary-heap. */
#define binheap_init(h, cap, factor, ratio, increment)                      \
{                                                                           \
    BINHEAP_ELEM_TYPE((h))* ___N;                                           \
    size_t ___N_allocsz = (1 + (cap)) *                                     \
                           BINHEAP_ELEM_SIZE((h));                          \
    ___N = BINHEAP_malloc(___N_allocsz);                                    \
    (h) = (BINHEAP_TYPE(h)) { .size = 0, .capacity = (cap),                 \
                              .growth_factor = (factor),                    \
                              .growth_ratio = (ratio),                      \
                              .growth_increment = (increment),              \
                              .nodes = ___N };                              \
}

/* Destroys a binary-heap. */
#define binheap_destroy(h)                                                  \
{                                                                           \
    BINHEAP_free((h).nodes);                                                \
    BINHEAP_memset(&(h), 0, sizeof((h)));                                   \
}

#define __binheap_grow_capacity(h)                                          \
{                                                                           \
    unsigned int ___new_cap = h.capacity;                                   \
    ___new_cap *= (1 + h.growth_factor);                                    \
    ___new_cap /= (1 + h.growth_ratio);                                     \
    ___new_cap += h.growth_increment;                                       \
    if ( BINHEAP_unlikely(___new_cap <= h.capacity) ) h.capacity++;         \
    else h.capacity = ___new_cap;                                           \
    h.nodes = BINHEAP_realloc(h.nodes,                                      \
                              (1 + h.capacity) *                            \
                              BINHEAP_ELEM_SIZE(h));                        \
}

#define __binheap_add_last(h, elem)                                         \
{                                                                           \
    if ( BINHEAP_unlikely(h.size == h.capacity) )                           \
        __binheap_grow_capacity(h);                                         \
    h.nodes[h.size] = elem;                                                 \
    h.size++;                                                               \
}

#define __binheap_up(h, eidx, elem, cmp_ok_fn)                              \
{                                                                           \
    unsigned int ___pidx;                                                   \
    do                                                                      \
    {                                                                       \
        /* Stop if the element became root. */                              \
        if ( BINHEAP_unlikely(eidx == 0) ) break;                           \
        ___pidx = __BH_PARENT(eidx);                                        \
                                                                            \
        /* Stop if the inserted element finally has the proper parent. */   \
        if ( BINHEAP_unlikely(cmp_ok_fn(h.nodes[___pidx], elem)) ) break;   \
                                                                            \
        /* Swap with parent. */                                             \
        h.nodes[eidx] = h.nodes[___pidx];                                   \
        h.nodes[___pidx] = elem;                                            \
        eidx = ___pidx;                                                     \
    } while(1);                                                             \
}

#define __binheap_down(h, eidx, elem, cmp_ok_fn)                            \
{                                                                           \
    unsigned int ___leftidx, ___rightidx;                                   \
    unsigned int ___swapidx;                                                \
    do                                                                      \
    {                                                                       \
        /* Stop if the element became leaf (<=> has no left child). */      \
        ___leftidx = __BH_LEFT(eidx);                                       \
        if ( BINHEAP_unlikely(___leftidx >= h.size) ) break;                \
                                                                            \
        /* Stop if the element is finally a proper parent. */               \
        ___rightidx = __BH_RIGHT(eidx);                                     \
        if ( BINHEAP_likely(___rightidx < h.size) &&                        \
             cmp_ok_fn(h.nodes[___rightidx], h.nodes[___leftidx]) )         \
            ___swapidx = ___rightidx;                                       \
        else                                                                \
            ___swapidx = ___leftidx;                                        \
                                                                            \
        if ( BINHEAP_unlikely(cmp_ok_fn(elem, h.nodes[___swapidx])) )       \
            break;                                                          \
                                                                            \
        /* Swap with the child that can be the parent of the other. */      \
        h.nodes[eidx] = h.nodes[___swapidx];                                \
        h.nodes[___swapidx] = elem;                                         \
        eidx = ___swapidx;                                                  \
    } while(1);                                                             \
}

/*
 * Inserts an element into the binary-heap.
 * Notes:
 *  - Only lvalue 'elem' supported, don't provide rvalue.
 *  - cmp_ok_fn(p, c) must return true if the parent p and his child c are in
 *    the proper order relative to each-other in the binary-heap.
 *    To expand on that, in a min-heap cmp_ok_fn must return true when p <= c
 *    and in a max-heap it must return true inversely, i.e. when p >= c.
 *
 * Returns a pointer to the inserted element.
 */
#define binheap_insert(h, elem, cmp_ok_fn)                                  \
({                                                                          \
    unsigned int ___eidx;                                                   \
    __binheap_add_last((h), (elem));                                        \
    ___eidx = (h).size - 1;                                                 \
    __binheap_up((h), ___eidx, (elem), cmp_ok_fn);                          \
    &(h).nodes[___eidx];                                                    \
})

#define __binheap_eptr_to_eidx(h, eptr, eidx)                               \
{                                                                           \
    size_t ___ptrdiff = (size_t) ((size_t) eptr - (size_t) &h.nodes[0]);    \
    BINHEAP_assert( (___ptrdiff %  BINHEAP_ELEM_SIZE(h)) == 0 );            \
    eidx = (unsigned int) (___ptrdiff / BINHEAP_ELEM_SIZE(h));              \
}

#define __binheap_delete_by_index(h, eidx, cmp_ok_fn)                       \
{                                                                           \
    BINHEAP_assert(eidx < h.size);                                          \
    h.size--;                                                               \
                                                                            \
    /* When the last element is removed, nothing more need be done. */      \
    if ( BINHEAP_likely(eidx != h.size) )                                   \
    {                                                                       \
        unsigned int ___pidx;                                               \
        BINHEAP_ELEM_TYPE(h) ___elem;                                       \
                                                                            \
        /* Overwrite with last. */                                          \
        ___elem = h.nodes[h.size];                                          \
        h.nodes[eidx] = ___elem;                                            \
                                                                            \
        ___pidx = __BH_PARENT(eidx);                                        \
                                                                            \
        /*                                                                  \
         * Case 1: the element and his parent are in the proper             \
         *         order or the element is root -> do heap-down.            \
         */                                                                 \
        if ( BINHEAP_likely(eidx == 0) ||                                   \
             cmp_ok_fn(h.nodes[___pidx], ___elem) )                         \
        {                                                                   \
            __binheap_down(h, eidx, ___elem, cmp_ok_fn);                    \
        }                                                                   \
        /*                                                                  \
         * Case 2: the element and the child that could be the parent of    \
         *         the other child are in the proper order -> do heap-up.   \
         */                                                                 \
        else                                                                \
        {                                                                   \
            __binheap_up(h, eidx, ___elem, cmp_ok_fn);                      \
        }                                                                   \
    }                                                                       \
}

/* Deletes the specified element from the heap. */
#define binheap_delete(h, eptr, cmp_ok_fn)                                  \
{                                                                           \
    unsigned int ___eidx;                                                   \
    __binheap_eptr_to_eidx((h), (eptr), ___eidx);                           \
    __binheap_delete_by_index((h), ___eidx, cmp_ok_fn);                     \
}

/* Deletes the heap's root. */
#define binheap_delete_root(h, cmp_ok_fn)                                   \
{                                                                           \
    unsigned int ___eidx = 0;                                               \
    __binheap_delete_by_index((h), ___eidx, cmp_ok_fn);                     \
}

/* Returns a pointer to the root element of the heap. */
#define binheap_get_root(h) (&(h).nodes[0])

#endif /* _DS_BINHEAP_H_ */
