/*
 * binheap_test.c
 *
 *  Created on: Feb 23, 2017
 *      Author: corneliu.zuzu
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define BINHEAP_malloc          malloc
#define BINHEAP_free            free
#define BINHEAP_realloc         realloc
#define BINHEAP_memset          memset
#define BINHEAP_assert          assert
#define BINHEAP_likely(x)       __builtin_expect(!!(x), 1)
#define BINHEAP_unlikely(x)     __builtin_expect(!!(x), 0)
#define BINHEAP_inline          inline __attribute__((always_inline))
#define BINHEAP_ENOMEM          ENOMEM

#include <binheap.h>

struct int_elem
{
    int value;
    struct binheap_entry as_minheap_entry;
};

static inline unsigned int
minheap_int_cmp_ok(struct binheap_entry* parent, struct binheap_entry* child)
{
    struct int_elem *p, *c;
    p = binheap_get_capsule(parent, struct int_elem, as_minheap_entry);
    c = binheap_get_capsule(child, struct int_elem, as_minheap_entry);
    return (p->value <= c->value) ? 1 : 0;
}

#define minheap_init(h, cap, factor, ratio, increment) \
    binheap_init((h), (cap), (factor), (ratio), (increment))

#define minheap_destroy(h) \
    binheap_destroy((h))

#define minheap_insert(h, ie) \
    binheap_insert((h), &(ie)->as_minheap_entry, minheap_int_cmp_ok)

#define minheap_delete(h, ie) \
    binheap_delete((h), &(ie)->as_minheap_entry, minheap_int_cmp_ok)

#define minheap_delete_root(h) \
    binheap_delete_root((h), minheap_int_cmp_ok)

#define minheap_get_root(h) \
    binheap_get_capsule(binheap_get_root((h)), struct int_elem, as_minheap_entry)

#define minheap_is_empty(h)        ((h)->size == 0)

/*
 *
 *
 * Actual test code
 *
 *
 */

#include <stdio.h>

struct binheap minheap_of_ints;

struct int_elem test_values[] =
    { {26}, {35}, {12}, {20}, {5}, {34}, {23}, {14}, {24}, {9} };
struct int_elem test_values_sorted[] =
    { {5}, {9}, {12}, {14}, {20}, {23}, {24}, {26}, {34}, {35} };

static inline void test_binheap(void)
{
    struct int_elem tmp;
    int i;
    const int count = sizeof(test_values) / sizeof(test_values[0]);
    const int initial_capacity = count;

    minheap_init(&minheap_of_ints, initial_capacity, 1, 0, 0);

    for(i = 0; i < count; i++)
    {
        struct int_elem *v = &test_values[i];
        minheap_insert(&minheap_of_ints, v);
        assert(minheap_of_ints.size == i + 1);
    }

    tmp = *minheap_get_root(&minheap_of_ints);
    printf("Min-heap root: %d\n", tmp.value);
    assert(tmp.value == test_values_sorted[0].value);

    printf("Min-heap size after inserts: %d\n", minheap_of_ints.size);
    printf("Min-heap capacity after inserts: %d\n", minheap_of_ints.capacity);
    assert(minheap_of_ints.capacity == initial_capacity);

    tmp.value = 7;
    minheap_insert(&minheap_of_ints, &tmp);
    printf("Min-heap size after one more addition: %d\n", minheap_of_ints.size);
    assert(minheap_of_ints.size == count + 1);
    printf("Min-heap capacity after realloc: %d\n", minheap_of_ints.capacity);
    assert(minheap_of_ints.capacity == 2 * initial_capacity);

    minheap_delete(&minheap_of_ints, &tmp);
    printf("Min-heap size after one deletion: %d\n", minheap_of_ints.size);
    assert(minheap_of_ints.size == count);

    printf("Min-heap elements, extracted in order: ");
    for(i = 0; i < count; i++)
    {
        tmp = *minheap_get_root(&minheap_of_ints);
        minheap_delete_root(&minheap_of_ints);
        printf("%d ", tmp.value);
        assert(tmp.value == test_values_sorted[i].value);
        assert(minheap_of_ints.size == count - (i + 1));
    }

    minheap_destroy(&minheap_of_ints);
}

int main()
{
    test_binheap();
    return 0;
}
