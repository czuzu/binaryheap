# Binary Heap
Lightweight implementation of the binary-heap data-structure in C.

Purely-macros implementation (very fast).

# Highlights

* Single header file (binheap.h) with no external dependencies
* Clean linux-kernel coding-style
* Can behave as both min-heap and max-heap
* Generic (C++ template-like - see BINHEAP_TEMPLATE_INSTANTIATE)
* Customizable realloc behavior (capacity growth formula can be adjusted)
* Operations: init, destroy, insert, delete, delete_root, get_root

# How to use

Take a look at **binheap_test.c** for a concrete example.

In a nutshell you have to:

####1. Define needed macros and include the header file

  ```
  #include <stdlib.h>
  #include <string.h>
  #include <assert.h>

  #define BINHEAP_malloc          malloc
  #define BINHEAP_free            free
  #define BINHEAP_realloc         realloc
  #define BINHEAP_memset          memset
  #define BINHEAP_assert          assert
  #define BINHEAP_likely(x)       __builtin_expect(!!(x), 1)
  #define BINHEAP_unlikely(x)     __builtin_expect(!!(x), 0)

  #include <binheap.h>
  ```

####2. Instantiate the templates (C++-like) you want to instantiate by:

  ```
  BINHEAP_TEMPLATE_INSTANTIATE(minheap_int, int);
  typedef struct minheap_int minheap_int_t;
  ```

####3. Define the comparison-ok function that establishes when the order between a parent and a child is ok in the heap. In a min-heap, for example:

  ```
  static inline bool_t
  minheap_int_cmp_ok_fn(int parent, int child)
  {
      return (parent <= child);
  }
  ```

####4. Optionally define wrappers over provided operations:

  ```
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
  ```

####5. And use it:

  ```
  minheap_int_t minheap_of_ints;

  // To initialize:
  minheap_init(minheap_of_ints, 10, 1, 0, 0); /* 1, 0, 0 -> capacity doubles (2x) on realloc */

  // To insert an element:
  int e = 7;
  int* eptr = minheap_insert(minheap_of_ints, e); /* second param (e) must be lvalue */

  // To get the root element (minimum in a min-heap, maximum in a max-heap):
  int* root = minheap_get_root(minheap_of_ints);

  // To delete an element:
  minheap_delete(minheap_of_ints, eptr);

  // To delete the root element:
  minheap_delete_root(minheap_of_ints);

  // And finally, to destroy the binary-heap after you're done using it:
  minheap_destroy(minheap_of_ints);
  ```

# Caveats:

* Binary-heap capacity never shrinks, only grows according to the growth formula.
