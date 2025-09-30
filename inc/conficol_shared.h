/**
 * @file conficol_shared.h
 * @brief Shared macros and datatypes across the conficol library.
 *
 * @author Abdulla Almosalami (memphis242)
 * @date Jul 16, 2025
 * @copyright MIT License
 */

#ifndef CONFICOL_SHARED_H
#define CONFICOL_SHARED_H

/* File Inclusions */
#include <stdint.h>

/* Public Macro Definitions */
#define DEFAULT_ALLOCATOR              \
(                                      \
 (struct Allocator){                   \
   .alloc = default_alloc,             \
   .realloc = default_realloc,         \
   .reclaim = default_reclaim,         \
   .alloca_init = NULL,                \
   .arena = NULL                       \
 }                                     \
)

/* Public Datatypes */

/**
 * @brief Generic interface to a user-provided allocator.
 * @note The intended use of such an allocator is to provide allocations of
 *       arbitrary sizes of contiguous bytes (e.g., for the array that underlies
 *       a Vector object).
 * @note In contrast, allocations of fixed size objects (e.g., the struct that
 *       represents a Vector) will be handled internally, and will likely be
 *       static pools of those fixed-size objects. The size of such pools will
 *       be user configurable through a macro (e.g., cfg/vector_cfg.h).
 * @note All function members take a void * arena parameter that may be used
 *       by an implementation to carry out the allocations/reclamations. This is
 *       likely going to be a user-defined struct (e.g., struct Arena { uint8_t
 *       start; uint8_t end; uint8_t * bump_ptr; } ) that is cast from void *
 *       to the user-defined struct type in the allocator fcn implementations.
 *
 * Since the user supplies the functions to make up their allocator of choice,
 * they are flexible to pick an allocator that suits their needs. If the allocator
 * function prototypes don't exactly match this struct, it is likely trivial
 * to make wrapper functions around the desired allocator functions.
 *
 * For instance, on the one end, one could just use the DEFAULT_ALLOCATOR, which
 * ties wrappers around the stdlib malloc, realloc, and free functions, with
 * nothing provided for the alloca_init member. On another end, one can use a
 * simple "bump-allocator" scheme (aka, stack alloactor), that simply increments
 * a pointer that indicates the location of the next block to be allocated (note
 * that this scheme, while efficient, isn't great at freeing blocks in arbitrary
 * order), and such an implementation can at run-time provide the pool from which
 * the allocator acts upon. And there's plenty in between!
 *
 * @param alloc   Fcn that allocates memory and returns a pointer to the start
 *                of that memory.
 * @param realloc Fcn that takes a ptr to a previously allocated block and
 *                resizes it, returning a pointer to that new block.
 * @param reclaim Fcn that "reclaims" (i.e., frees) a previously allocated block
 *                pointed to by the passed-in ptr.
 * @param alloca_init A init fcn for the allocator - e.g., may be used to
 *                    initialize subdivisions of the arena argument and compute
 *                    free lists, from which allocations, splitting, coalescence,
 *                    etc., may be done.
 * @param arena   The pool of memory from which allocations are made from.
 */
struct Allocator
{
   void * (*alloc)(size_t req_sz, void * arena);
   void * (*realloc)(void * old_ptr, size_t new_sz, size_t old_sz, void * arena);
   void   (*reclaim)(void * old_ptr, size_t old_sz, void * arena);
   void   (*alloca_init)(void * arena);
   void * arena;
};

/* Public Functions */

// These will simply be wrappers around the common stdlib fcns, ignoring the
// unused parameters as needed.
void * default_alloc(size_t req_sz, void *);
void * default_realloc(void * old_ptr, size_t new_sz, size_t, void *);
void   default_reclaim(void * old_ptr, size_t, void *);

/**
 * @brief Randomizes the bytes for the region passed in.
 * @note This is only meant to be used internally, and `assert` statements on
 *       ptr being non-NULL and nbytes being > 0 are made.
 */
void memset_scramble(void * const ptr, size_t nbytes);

/**
 * @brief Keeps track of tokens and issues new ones to registered contexts.
 */
void tocken_tracker(void);

#endif // CONFICOL_SHARED_H
