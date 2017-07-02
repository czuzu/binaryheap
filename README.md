# Binary Heap
Lightweight implementation of the binary-heap data-structure in C.

Function-inlining implementation (fast).

# Highlights

* Single header file (binheap.h) with no external dependencies
* Clean linux-kernel coding-style
* Can behave as both min-heap and max-heap
* Generic (just encapsulate a 'struct binheap_entry' in element type)
* Customizable realloc behavior (capacity growth formula can be adjusted)
* Operations: init, destroy, insert, delete, delete_root, get_root

# How to use

Take a look at **binheap_test.c** for a concrete example.

In a nutshell you have to:

#### 1. Define needed macros and include the header file

  ```
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
  ```

#### 2. Create a type encapsulating a 'struct binheap_entry' and the data you want the heap-elements to hold

  ```
  struct int_elem
  {
      int value;
      struct binheap_entry as_minheap_entry;
  };
  ```

#### 3. Define the comparison-ok function that establishes when the order between a parent and a child is ok in the heap. In a min-heap, for example:

  ```
  static inline unsigned int
  minheap_int_cmp_ok(struct binheap_entry* parent, struct binheap_entry* child)
  {
      struct int_elem *p, *c;
      p = binheap_get_capsule(parent, struct int_elem, as_minheap_entry);
      c = binheap_get_capsule(child, struct int_elem, as_minheap_entry);
      return (p->value <= c->value) ? 1 : 0;
  }
  ```

#### 4. Optionally define wrappers over provided operations (recommended):

  ```
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
    #define minheap_is_empty(h)		((h)->size == 0)
  ```

#### 5. And use it:

  ```
  struct binheap minheap_of_ints;

  // To initialize:
  minheap_init(&minheap_of_ints, initial_capacity, 1, 0, 0); /* 1, 0, 0 -> capacity doubles (2x) on realloc */

  // To insert an element:
  struct int_elem e = { .value = 7 };
  minheap_insert(&minheap_of_ints, &e);

  // To get the root element (minimum in a min-heap, maximum in a max-heap):
  struct int_elem* root = minheap_get_root(&minheap_of_ints);

  // To delete an element:
  minheap_delete(&minheap_of_ints, &e);

  // To delete the root element:
  minheap_delete_root(&minheap_of_ints);

  // And finally, to destroy the binary-heap after you're done using it:
  minheap_destroy(&minheap_of_ints);
  ```

# Caveats:

* Binary-heap capacity never shrinks, only grows according to the growth formula.

# Note about the .cproject, .project files;

An Eclipse CDT C Makefile project has been added (files .project, .cproject).
You need Eclipse CDT and make+gcc+gdb in PATH to import and run/debug it.
