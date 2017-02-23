/*
 * binheap_test.c
 *
 *  Created on: Feb 23, 2017
 *      Author: corneliu.zuzu
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define BINHEAP_malloc          malloc
#define BINHEAP_free            free
#define BINHEAP_realloc         realloc
#define BINHEAP_memset          memset
#define BINHEAP_assert          assert
#define BINHEAP_likely(x)       __builtin_expect(!!(x), 1)
#define BINHEAP_unlikely(x)     __builtin_expect(!!(x), 0)

#include <binheap.h>

BINHEAP_TEMPLATE_INSTANTIATE(minheap_int, int);
typedef struct minheap_int minheap_int_t;

typedef bool bool_t;

static inline bool_t
minheap_int_cmp_ok_fn(int parent, int child)
{
    return (parent <= child);
}

#define minheap_init(h, cap, factor, ratio, increment) \
    binheap_init((h), (cap), (factor), (ratio), (increment));

#define minheap_destroy(h) \
    binheap_destroy((h))

#define minheap_insert(h, v) \
    binheap_insert((h), (v), minheap_int_cmp_ok_fn);

#define minheap_delete(h, eptr) \
    binheap_delete((h), (eptr), minheap_int_cmp_ok_fn);

#define minheap_delete_root(h) \
    binheap_delete_root((h), minheap_int_cmp_ok_fn);

#define minheap_get_root(h) \
    binheap_get_root((h))

/*
 *
 *
 * Actual test code
 *
 *
 */

#include <stdio.h>

minheap_int_t minheap_of_ints;

int test_values[] =
    { 26, 35, 12, 20, 5, 34, 23, 14, 24, 9 };
int test_values_sorted[] =
    { 5, 9, 12, 14, 20, 23, 24, 26, 34, 35 };

static inline void test_binheap(void)
{
    int i, tmp, *tmpptr;
    const int count = sizeof(test_values) / sizeof(int);
    const int initial_capacity = count;

    minheap_init(minheap_of_ints, initial_capacity, 1, 0, 0);

    for(i = 0; i < count; i++)
    {
        int v = test_values[i];
        int* vptr = minheap_insert(minheap_of_ints, v);
        assert(*vptr == v);
        assert(minheap_of_ints.size == i + 1);
    }

    tmp = *minheap_get_root(minheap_of_ints);
    printf("Min-heap root: %d\n", tmp);
    assert(tmp == test_values_sorted[0]);

    printf("Min-heap size after inserts: %d\n", minheap_of_ints.size);
    printf("Min-heap capacity after inserts: %d\n", minheap_of_ints.capacity);
    assert(minheap_of_ints.capacity == initial_capacity);

    tmp = 7;
    tmpptr = minheap_insert(minheap_of_ints, tmp);
    printf("Min-heap size after one more addition: %d\n", minheap_of_ints.size);
    assert(minheap_of_ints.size == count + 1);
    printf("Min-heap capacity after realloc: %d\n", minheap_of_ints.capacity);
    assert(minheap_of_ints.capacity == 2 * initial_capacity);

    assert(*tmpptr == tmp);
    minheap_delete(minheap_of_ints, tmpptr);
    printf("Min-heap size after one deletion: %d\n", minheap_of_ints.size);
    assert(minheap_of_ints.size == count);

    printf("Min-heap elements, extracted in order: ");
    for(i = 0; i < count; i++)
    {
        int v = *minheap_get_root(minheap_of_ints);
        minheap_delete_root(minheap_of_ints);
        printf("%d ", v);
        assert(v == test_values_sorted[i]);
        assert(minheap_of_ints.size == count - (i + 1));
    }

    minheap_destroy(minheap_of_ints);
}

int main()
{
    test_binheap();
    return 0;
}
