/**
 * @file vector.c
 * @brief Implementation of a dynamic array (vector) in C.
 *
 * This file contains the implementation of a dynamic array, also known as a 
 * vector, which provides functionalities for dynamic resizing, element access, 
 * and manipulation.
 *
 * @author Abdulla Almosalami (memphis242)
 * @date April 12, 2025
 * @copyright MIT License
 */

/* File Inclusions */
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

#include "conficol_shared.h"
#include "vector_cfg.h"
#include "vector.h"

/* Local Macro Definitions */

// Macro constants
#define EXPANSION_FACTOR                    (2)   // TODO: Make the expansion factor user-configurable - note that a floating-point number causes a warning: conversion from 'size_t' {aka 'long long unsigned int'} to 'double' may change value [-Wconversion]
#define DEFAULT_INITIAL_CAPACITY            (10)  //! Not 1 because there would likely be a resize shortly after
#define DEFAULT_MAX_CAPACITY_FACTOR         (10)  //! How many multiples of initial capacity do we set max capacity by default
#define DEFAULT_LEN_TO_CAPACITY_FACTOR      (2)   //! How many multiples of length should capacity be set to by default

// If user did not define a maximum vector length, go off of system limits.
#ifndef MAX_VEC_LEN
#define TENTATIVE_MAX_VEC_LEN UINT32_MAX
#if ( SIZE_MAX < PTRDIFF_MAX )
   #define SYSTEM_LIMIT SIZE_MAX
#else
   #define SYSTEM_LIMIT PTRDIFF_MAX
#endif // SIZE_MAX < PTRDIFF_MAX
#if (SYSTEM_LIMIT < TENTATIVE_MAX_VEC_LEN)
   #define MAX_VEC_LEN  SYSTEM_LIMIT
#else
   #define MAX_VEC_LEN TENTATIVE_MAX_VEC_LEN
#endif // (SYSTEM_LIMIT < TENTATIVE_MAX_VEC_LEN)
#endif // MAX_VEC_LEN

// Function-like macros

#define IS_EMPTY(self) ( 0 == (self)->len )
#define PTR_TO_IDX(vec, idx) ( (uint8_t *)((vec)->arr) + ((vec)->element_size * (idx)) )

/* Local Datatypes */

struct Vector
{
   void * arr;
   size_t element_size;
   size_t len;
   size_t capacity;
   size_t max_capacity;
   struct Allocator mem_mgr;
};

enum ShiftDir
{
   ShiftDir_Left,
   ShiftDir_Right,
   ShiftDir_InvalidDir
};

/* Private Function Prototypes */

static struct Vector * vec_pool_dispatch(void);
static void            vec_pool_reclaim(const struct Vector *);
static bool            vec_isalloc(const struct Vector *);

static bool vec_expand(struct Vector *);
static bool vec_expandby(struct Vector *, size_t);
static void shiftn( struct Vector *, size_t, enum ShiftDir, size_t);

/* Public API Implementations */

/******************************************************************************/
struct Vector * VectorNew( size_t element_size,
                           size_t init_capacity,
                           size_t max_capacity,
                           const void * init_data,
                           size_t init_dlen,
                           const struct Allocator * mem_mgr )
{
   // Invalid inputs
   if ( (0 == element_size) ||
        (init_capacity > MAX_VEC_LEN) ||
        (0 == max_capacity) ||
        (init_capacity > max_capacity) )
   {
      return NULL;
   }

   struct Vector * new_vec = vec_pool_dispatch();
   if ( NULL == new_vec )
   {
      return NULL;
   }

   if ( (NULL == mem_mgr) ||
        (NULL == mem_mgr->alloc) || (NULL == mem_mgr->realloc) || (NULL == mem_mgr->reclaim) )
   {
      // TODO: Return error code informing the user an incorrect configuration of mem_mgr was sent in.
      new_vec->mem_mgr = DEFAULT_ALLOCATOR;
   }
   else
   {
      new_vec->mem_mgr = *mem_mgr;
   }

   if ( new_vec->mem_mgr.alloca_init != NULL )
   {
      // The arena pointer may be NULL, but I won't let that stop me from calling
      // the allocator's init fcn, because it may not need it.
      new_vec->mem_mgr.alloca_init( new_vec->mem_mgr.arena );
   }

   if ( 0 == init_capacity )
   {
      new_vec->arr = NULL;
   }
   else
   {
      new_vec->arr = new_vec->mem_mgr.alloc( element_size * init_capacity,
                                             new_vec->mem_mgr.arena );
   }

   // If we failed to allocate space for the array...
   if ( (init_capacity > 0) && (NULL == new_vec->arr) )
   {
      // TODO: Return error code informing user that allocation of underlying array failed.
      new_vec->capacity = 0;
      new_vec->len = 0;
   }
   else
   {
      new_vec->capacity = init_capacity;
      if ( (init_data != NULL) && (init_dlen <= init_capacity) )
      {
         memcpy( new_vec->arr, init_data, element_size * init_dlen );
         new_vec->len = init_dlen;
      }
      else if ( (init_data == NULL) && (init_dlen > 0) && (init_dlen <= init_capacity) )
      {
         memset( new_vec->arr, 0, init_dlen * element_size );
         new_vec->len = init_dlen;
      }
      else if ( init_dlen > init_capacity )
      {
         // TODO: Inform user they sent in an invalid init_dlen
         new_vec->len = 0;
      }
      else
      {
         new_vec->len = 0;
      }
   }

   new_vec->element_size = element_size;

   if ( max_capacity > MAX_VEC_LEN )
   {
      // TODO: Throw exception for max_capacity too large
      new_vec->max_capacity = MAX_VEC_LEN;
   }
   else
   {
      new_vec->max_capacity = max_capacity;
   }

   return new_vec;
}

/******************************************************************************/
void VectorFree( struct Vector * self )
{
   if ( (self != NULL) && vec_isalloc(self) )
   {
      if ( (self->mem_mgr.reclaim != NULL) && (self->arr != NULL) )
      {
         memset_scramble(self->arr, self->capacity * self->element_size);
         self->mem_mgr.reclaim(self->arr, self->capacity * self->element_size, self->mem_mgr.arena );
      }
      memset_scramble(self, sizeof(struct Vector));
      vec_pool_reclaim(self);
   }
}

/******************************************************************************/
struct Vector * VectorDuplicate( const struct Vector * self )
{
   if ( (NULL == self) || !vec_isalloc(self) ||
        // Check for internal paradoxes within passed in vector...
        (0 == self->element_size) ||
        (self->len > self->capacity) ||
        (self->capacity > self->max_capacity) ||
        ( (self->len > 0) && (NULL == self->arr) )
        || (NULL == self->mem_mgr.alloc)
      )
   {
      return NULL;
   }

   struct Vector * dup = vec_pool_dispatch();
   if ( NULL == dup )
   {
      // TODO: Throw exception that the vector handle failed to get duplicated.
      return NULL;
   }

   memcpy( dup, self, sizeof(struct Vector) );

   dup->arr = NULL;
   if ( dup->len > 0 )
   {
      dup->arr = self->mem_mgr.alloc( dup->capacity * dup->element_size, self->mem_mgr.arena );
      if ( dup->arr != NULL )
      {
         memcpy( dup->arr,
                 self->arr,
                 dup->len * dup->element_size );
      }
      else
      {
         // TODO: Throw exception that underlying data failed to get duplicated.
      }
   }

   // dupd vector _must not_ reference original vector's data!
   assert( dup->arr != self->arr );

   return dup;
}

/******************************************************************************/
bool VectorMove( struct Vector * dest, struct Vector * src )
{
   assert( dest == NULL || (dest != NULL && dest->mem_mgr.reclaim != NULL) );

   if ( (NULL == src) || !vec_isalloc(src) || (NULL == dest) || !vec_isalloc(dest) ||
        (dest->element_size != src->element_size) ||
        (dest->mem_mgr.alloc   != src->mem_mgr.alloc) ||
        (dest->mem_mgr.realloc != src->mem_mgr.realloc) ||
        (dest->mem_mgr.reclaim != src->mem_mgr.reclaim) ||
        (dest->mem_mgr.arena   != src->mem_mgr.arena) )
   {
      return false;
   }

   // Free resources of existing destination vector, if applicable
   if ( dest->arr != NULL )
   {
      memset_scramble(dest->arr, dest->capacity * dest->element_size);
      dest->mem_mgr.reclaim(dest->arr, dest->element_size * dest->capacity, dest->mem_mgr.arena);
   }

   // Move resources over
   dest->arr = src->arr;
   dest->len = src->len;
   dest->capacity = src->capacity;
   dest->max_capacity = src->max_capacity;

   // Leave original vector in valid but empty state
   // Note: This is not the same as hard resetting. We don't want the original
   //       underlying array to be free'd because "ownership" of that has been
   //       moved to dest.
   src->capacity = 0;
   src->arr = NULL;
   src->len = 0;

   return true;
}

/******************************************************************************/
bool VectorsAreEqual( const struct Vector * a, const struct Vector * b )
{
   // Check for NULL pointers
   if ( (NULL == a) || !vec_isalloc(a) || (NULL == b) || !vec_isalloc(b) )   return false;

   // First check lengths
   if (a->len != b->len) return false;
   
   // Then check the element sizes
   if (a->element_size != b->element_size) return false;

   // Then check capacities
   if (a->capacity != b->capacity) return false;

   // Then check max capacities
   if (a->max_capacity != b->max_capacity) return false;

   // Then check each element in the vector
   for ( size_t i = 0; i < a->len; i++ )
   {
      if ( memcmp( PTR_TO_IDX(a,i), PTR_TO_IDX(b,i), a->element_size ) )
      {
         return false;
      }
   }

   // We'll allow the vectors to be allocated from different allocators. I can't
   // think of a case where it also matters how the vectors were allocated, if
   // everything else about them is equal.

   return true;
}

/******************************************************************************/
struct Vector * VectorConcatenate( const struct Vector * v1,
                                   const struct Vector * v2 )
{
   if ( (NULL == v1) || !vec_isalloc(v1) || (NULL == v2) || !vec_isalloc(v2) ||
        (v1->element_size != v2->element_size) ||
        (v2->len > (MAX_VEC_LEN - v1->len)) // Unsupported length
      )
   {
      return NULL;
   }

   assert(
      ( (v1->len > 0) && (v1->arr != NULL) ) ||
        (0 == v1->len)
   );
   assert(
      ( (v2->len > 0) && (v2->arr != NULL) ) ||
        (0 == v2->len)
   );
   assert(v1->element_size > 0);
   assert(v2->element_size > 0);
   assert(v1->element_size == v2->element_size);

   struct Vector * NewVec = NULL;
   // If one of the vectors is empty, simply create a duplicate of the non-empty
   // vector. If both vectors are empty, create an empty vector.
   if ( (0 == v1->len) && (0 == v2->len) )
   {
      NewVec = VectorNew( v1->element_size,
                           DEFAULT_INITIAL_CAPACITY,
                           DEFAULT_INITIAL_CAPACITY * DEFAULT_MAX_CAPACITY_FACTOR,
                           NULL, 0,
                           &v1->mem_mgr );
   }

   else if ( (v1->len > 0)  && (v2->len == 0) )
   {
      NewVec = VectorDuplicate(v1);
   }
   else if ( (v1->len == 0) && (v2->len > 0) )
   {
      NewVec = VectorDuplicate(v2);
   }

   else  // Both must be non-empty
   {
      size_t new_vec_len = v1->len + v2->len;
      size_t new_vec_cap = MAX_VEC_LEN;
      if ( v2->capacity < (MAX_VEC_LEN - v1->capacity) )
      {
         new_vec_cap = v1->capacity + v2->capacity;
      }
      size_t new_vec_max_cap = MAX_VEC_LEN;
      if ( v2->max_capacity < (MAX_VEC_LEN - v1->max_capacity) )
      {
         new_vec_max_cap = v1->max_capacity + v2->max_capacity;
      }

      NewVec = VectorNew( v1->element_size,
                           new_vec_cap,
                           new_vec_max_cap,
                           NULL, new_vec_len,
                           &v1->mem_mgr );
      if ( (NewVec != NULL) && (NewVec->arr != NULL) )
      {
         memcpy( NewVec->arr,                 v1->arr, (v1->len * v1->element_size) );
         memcpy( PTR_TO_IDX(NewVec, v1->len), v2->arr, (v2->len * v2->element_size) );
      }
      else
      {
         // Something failed in creating the vector...
         VectorFree(NewVec);
         NewVec = NULL; // We'll return NULL
      }
   }

   return NewVec;
}

/******************************************************************************/
size_t VectorLength( const struct Vector * self )
{
   if ( (NULL == self) || !vec_isalloc(self) )
   {
      return 0;
   }
   return self->len;
}

/******************************************************************************/
size_t VectorCapacity( const struct Vector * self )
{
   if ( (NULL == self) || !vec_isalloc(self) )
   {
      return 0;
   }
   return self->capacity;
}

/******************************************************************************/
size_t VectorMaxCapacity( const struct Vector * self )
{
   if ( (NULL == self) || !vec_isalloc(self) )
   {
      return 0;
   }
   return self->max_capacity;
}

/******************************************************************************/
size_t VectorElementSize( const struct Vector * self )
{
   if ( (NULL == self) || !vec_isalloc(self) )
   {
      return 0;
   }
   return self->element_size;
}

/******************************************************************************/
bool VectorIsEmpty( const struct Vector * self )
{
   if ( (NULL == self) || !vec_isalloc(self) )
   {
      return true;
   }
   return IS_EMPTY(self);
}

/******************************************************************************/
bool VectorIsFull( const struct Vector * self )
{
   if ( (NULL == self) || !vec_isalloc(self) )
   {
      return false;
   }
   return self->len == self->max_capacity;
}

/******************************************************************************/
bool VectorPush( struct Vector * self, const void * element )
{
   // Early return op
   // Invalid inputs
   if ( (NULL == self) || !vec_isalloc(self) || (NULL == element) )
   {
      // TODO: Throw exception
      return false;
   }

   assert( self->len <= self->capacity );
   assert( self->len <= self->max_capacity );
   assert( (self->element_size * self->len) <= PTRDIFF_MAX );
   assert( ( self->capacity == 0 && self->arr == NULL ) ||
           ( self->capacity >  0 && self->arr != NULL ) );

   bool ret_val = true;

   // Ensure there's space
   bool successfully_expanded = true;
   if ( self->len == self->capacity )
   {
      successfully_expanded = vec_expand(self);
   }

   if ( successfully_expanded )
   {
      void * insertion_spot = (void *)PTR_TO_IDX(self, self->len);
      memcpy( insertion_spot, element, self->element_size );
      self->len++;
   }
   else
   {
      ret_val = false;
   }

   return ret_val;
}

/******************************************************************************/
bool VectorInsert( struct Vector * self,
                     size_t idx,
                     const void * element )
{
   // Early return op
   // Invalid inputs
   if ( (NULL == self) || !vec_isalloc(self) || (NULL == element) || (idx > self->len) )
   {
      // TODO: Throw exception
      return false;
   }

   assert( self->len <= self->capacity );
   assert( self->len <= self->max_capacity );
   assert( idx <= self->len );

   bool ret_val = true;

   // Ensure there's space
   bool successfully_expanded = true;
   if ( self->len == self->capacity )
   {
      successfully_expanded = vec_expand(self);
   }

   if ( successfully_expanded )
   {
      if ( idx < self->len )
      {
         shiftn(self, idx, ShiftDir_Right, 1);
      }
      void * insertion_spot = (void *)PTR_TO_IDX(self, idx);
      memcpy( insertion_spot, element, self->element_size );
      self->len++;
   }
   else
   {
      ret_val = false;
   }

   return ret_val;
}

/******************************************************************************/
void * VectorGet( const struct Vector * self, size_t idx )
{
   if ( (NULL == self) || !vec_isalloc(self) || (idx >= self->len) )
   {
      return NULL;
   }

   assert(self->arr != NULL);

   return (void *)PTR_TO_IDX(self, idx);
}

/******************************************************************************/
void * VectorLastElement( const struct Vector * self )
{
   if ( (NULL == self) || !vec_isalloc(self) || (0 == self->len) )
   {
      return NULL;
   }

   assert( self->len <= self->capacity );
   assert( self->len <= self->max_capacity );
   assert( (self->element_size * self->len) <= PTRDIFF_MAX );
   assert( self->arr != NULL );

   return (void *)PTR_TO_IDX(self, (self->len - 1));
}

/******************************************************************************/
bool VectorCpyElementAt( const struct Vector * self, size_t idx, void * data )
{
   if ( (NULL == self) || !vec_isalloc(self) || (idx >= self->len) || (NULL == data))
   {
      return false;
   }

   assert(self->arr != NULL);
   assert(self->element_size > 0);

   (void)memcpy( data, (void *)PTR_TO_IDX(self, idx), self->element_size );

   return true;
}

/******************************************************************************/
bool VectorCpyLastElement( const struct Vector * self, void * data )
{
   if ( (NULL == self) || !vec_isalloc(self) || (0 == self->len) || (NULL == data) )
   {
      return false;
   }

   assert( self->len <= self->capacity );
   assert( self->len <= self->max_capacity );
   assert( self->element_size > 0 );
   assert( (self->element_size * self->len) <= PTRDIFF_MAX );
   assert( self->arr != NULL );

   (void)memcpy( data,
                 VectorLastElement(self),
                 self->element_size );
   
   return true;
}

/******************************************************************************/
bool VectorSet( struct Vector * self,
                           size_t idx,
                           const void * element )
{
   if ( (NULL == self) || !vec_isalloc(self) || (idx >= self->len) || (NULL == element) )
   {
      return false;
   }

   assert(self->arr != NULL);
   assert(self->element_size > 0);

   memcpy( PTR_TO_IDX(self, idx), element, self->element_size );

   return true;
}

/******************************************************************************/
bool VectorRemove( struct Vector * self, size_t idx, void * data )
{
   if ( (NULL == self) || !vec_isalloc(self) || (idx >= self->len) || (self->len == 0) )
   {
      return false;
   }

   assert(self->arr != NULL);
   assert(self->element_size > 0);
   assert(self->len <= self->capacity);
   assert(self->capacity <= self->max_capacity);

   if ( data != NULL )
   {
      memcpy(data, PTR_TO_IDX(self, idx), self->element_size);
   }
   if ( idx < (self->len - 1) )
   {
      shiftn(self, idx + 1, ShiftDir_Left, 1);
   }
   self->len--;

   return true;
}

/******************************************************************************/
bool VectorRemoveLastElement( struct Vector * self, void * data )
{
   if ( (NULL == self) || !vec_isalloc(self) || VectorLength(self) == 0 ) return false;
   return VectorRemove( self, VectorLength(self) - 1, data );
}

/******************************************************************************/
bool VectorClearElementAt( struct Vector * self, size_t idx )
{
   if ( (NULL == self) || !vec_isalloc(self) || (NULL == self->arr) ||
        (0 == self->len) || (idx >= self->len) )
   {
      return false;
   }

   assert(self->element_size > 0);
   assert(self->arr != NULL);

   memset( PTR_TO_IDX(self, idx), 0, self->element_size );

   return true;
}

/******************************************************************************/

bool VectorClear( struct Vector * self )
{
   if ( self != NULL && vec_isalloc(self) && self->len == 0 ) return true; // trivial clear
   else if ( self == NULL || !vec_isalloc(self) ) return false;
   return VectorRangeClear(self, 0, self->len);
}

/******************************************************************************/
bool VectorReset( struct Vector * self )
{
   if ( (NULL == self) || !vec_isalloc(self) )
   {
      return false;
   }

   self->len = 0;
   return true;
}

/******************************************************************************/
bool VectorHardReset( struct Vector * self )
{
   if ( (NULL == self) || !vec_isalloc(self) )
   {
      return false;
   }

   assert( (self->arr == NULL && self->capacity == 0) ||
           (self->arr != NULL && self->capacity >  0) );
   if ( NULL == self->arr)
   {
      // Already in a reset state
      return true;
   }

   assert(self->arr != NULL);
   assert(self->element_size > 0);
   assert(self->mem_mgr.reclaim != NULL);

   memset( self->arr, 0, self->capacity * self->element_size );
   self->mem_mgr.reclaim( self->arr, self->capacity * self->element_size, self->mem_mgr.arena );
   self->arr = NULL; // After freeing memory, clear out stale pointers!
   self->len = 0;
   self->capacity = 0;

   return true;
}

/* Range Based Vector Operations */

/******************************************************************************/
/******************************************************************************/

struct Vector * VectorSplitAt( struct Vector * self, size_t idx )
{
   if ( (NULL == self) || !vec_isalloc(self) || (self->len == 0) || (self->capacity == 0) ||
        (idx >= self->len) || (idx == 0) )
   {
      // TODO: Throw exception
      return NULL;
   }

   assert(self->len > 0);
   assert(self->arr != NULL);
   assert(self->element_size > 0);

   size_t new_vec_len = self->len - idx;
   struct Vector * new_vec = VectorNew( self->element_size,
                                        new_vec_len * 2,
                                        new_vec_len * 4,
                                        self->arr, new_vec_len,
                                        &self->mem_mgr );
   if ( (NULL == new_vec) || (NULL == new_vec->arr) )
   {
      return NULL;
   }

   // NOTE: Don't mutate the original vector until after we've successfully
   //       initialized the new vector!
   self->len = idx;    // Does not include original element at idx
   memcpy( new_vec->arr,
           PTR_TO_IDX(self, idx),
           new_vec_len * self->element_size );
   
   #ifdef NO_DATA_LEFT_BEHIND
   memset( PTR_TO_IDX(self, idx), 0, new_vec_len * self->element_size );
   #endif

   return new_vec;
}

/******************************************************************************/

struct Vector * VectorSlice( const struct Vector * self,
                               size_t idx_start,
                               size_t idx_end )
{
   if ( (NULL == self) || !vec_isalloc(self) || (self->len == 0) || (self->capacity == 0) ||
        (idx_start >= self->len) || (idx_end > self->len) ||
        (idx_start > idx_end)    || (idx_end == 0) )
   {
      // TODO: Throw exception
      return NULL;
   }

   // Slicing the whole vector is the same as duplication
   if ( (0 == idx_start) && (self->len == idx_end) )
   {
      return VectorDuplicate(self);
   }

   assert(self->len > 0);
   assert(self->arr != NULL);
   assert(self->element_size > 0);

   size_t new_vec_len = idx_end - idx_start;
   struct Vector * new_vec = VectorNew( self->element_size,
                                        new_vec_len * 2,
                                        new_vec_len * 4,
                                        PTR_TO_IDX(self, idx_start), new_vec_len,
                                        &self->mem_mgr );
   if ( (NULL == new_vec) || (NULL == new_vec->arr) )
   {
      return NULL;
   }

   memcpy( new_vec->arr,
           PTR_TO_IDX(self, idx_start),
           new_vec_len * self->element_size );
   
   return new_vec;
}

/******************************************************************************/
/******************************************************************************/

bool VectorRangePush( struct Vector * self, const void * data, size_t dlen )
{
   if ( (NULL == self) || !vec_isalloc(self) || (NULL == data) ||
        ( (self->len + dlen) > self->max_capacity ) || (dlen == 0) )
   {
      // TODO: Throw exception
      return false;
   }

   assert( self->len <= self->capacity );
   assert( self->capacity <= self->max_capacity );
   assert( (self->len == 0) || ( (self->len > 0) && (self->arr != NULL) ) );

   // Ensure there's space
   bool successfully_expanded = true;
   if ( (self->len + dlen) > self->capacity )
   {
      successfully_expanded = vec_expandby(
                                       self,
                                       /* Expand relative to capacity, not len*/
                                       dlen - (self->capacity - self->len) );
   }

   if ( successfully_expanded )
   {
      void * insertion_spot = (void *)PTR_TO_IDX(self, self->len);
      memcpy( insertion_spot, data, (self->element_size * dlen) );
      self->len += dlen;
   }
   else
   {
      return false;
   }

   return true;
}

/******************************************************************************/

bool VectorRangeInsert( struct Vector * self,
                        size_t idx, 
                        const void * data,
                        size_t dlen )
{
   if ( (NULL == self) || !vec_isalloc(self) || (NULL == data) ||
        ( (self->len + dlen) > self->max_capacity ) || (dlen == 0) ||
        (idx > self->len) )
   {
      // TODO: Throw exception
      return false;
   }

   assert( self->len <= self->capacity );
   assert( self->capacity <= self->max_capacity );
   assert( (self->len == 0) || ( (self->len > 0) && (self->arr != NULL) ) );

   if ( dlen == 1 )
   {
      return VectorInsert(self, idx, data);
   }

   // Ensure there's space
   bool successfully_expanded = true;
   if ( (self->len + dlen) > self->capacity )
   {
      successfully_expanded = vec_expandby(
                                       self,
                                       /* Expand relative to capacity, not len*/
                                       dlen - (self->capacity - self->len) );
   }

   if ( successfully_expanded )
   {
      if ( idx < self->len )
      {
         shiftn(self, idx, ShiftDir_Right, dlen);
      }
      void * insertion_spot = (void *)PTR_TO_IDX(self, idx);
      memcpy( insertion_spot, data, (self->element_size * dlen) );
      self->len += dlen;
   }
   else
   {
      return false;
   }

   return true;
}

/******************************************************************************/

bool VectorRangeCpy( const struct Vector * self,
                     size_t idx_start,
                     size_t idx_end,
                     void * buffer )
{
   if ( (NULL == self) || !vec_isalloc(self) || (NULL == buffer) ||
        (idx_start >= self->len) || (idx_end > self->len) ||
        (idx_start >= idx_end)
      )
   {
      return false;
   }

   assert(self->len > 0);
   assert(self->arr != NULL);
   assert(self->element_size > 0);

   uint8_t * ptr_to_start = PTR_TO_IDX(self, idx_start);
   size_t idx_diff = idx_end - idx_start;
   memcpy( buffer, ptr_to_start, (idx_diff * self->element_size) );

   return true;
}

/******************************************************************************/

bool VectorRangeCpyToEnd( const struct Vector * self,
                          size_t idx,
                          void * buffer )
{
   if ( (NULL == self) || !vec_isalloc(self) )  return false;
   return VectorRangeCpy(self, idx, self->len, buffer);
}

/******************************************************************************/
/******************************************************************************/

bool VectorRangeSetWithArr( struct Vector * self,
                     size_t idx_start,
                     size_t idx_end,
                     const void * arr )
{
   if ( (NULL == self) || !vec_isalloc(self) || (NULL == arr) ||
        (idx_start >= self->len) || (idx_end > self->len) ||
        (idx_start >= idx_end) ) 
   {
      return false;
   }

   assert(self->len > 0);
   assert(self->arr != NULL);
   assert(self->element_size > 0);

   uint8_t * ptr_to_start = PTR_TO_IDX(self, idx_start);
   size_t idx_diff = idx_end - idx_start;
   memcpy( ptr_to_start, arr, (idx_diff * self->element_size) );

   return true;
}

/******************************************************************************/

bool VectorRangeSetToVal( struct Vector * self, size_t idx_start, size_t idx_end, const void * val )
{
   if ( (NULL == self) || !vec_isalloc(self) || (NULL == val) ||
        (idx_start >= self->len) || (idx_end > self->len) ||
        (idx_start >= idx_end) ) 
   {
      return false;
   }

   assert(self->len > 0);
   assert(self->arr != NULL);
   assert(self->element_size > 0);

   uint8_t * ptr = PTR_TO_IDX(self, idx_start);
   for ( size_t i = 0; i < (idx_end - idx_start); i++, (ptr += self->element_size) )
      memcpy( ptr, val, self->element_size );

   return true;
}

/******************************************************************************/

bool VectorRangeRemove( struct Vector * self,
                        size_t idx_start,
                        size_t idx_end,
                        void * buf )
{
   if ( (NULL == self) || !vec_isalloc(self) || (NULL == self->arr) ||
        (idx_start >= self->len) || (idx_end > self->len) ||
        (idx_start >= idx_end) || (self->len == 0) ) 
   {
      return false;
   }

   assert(self->len > 0);
   assert(self->arr != NULL);
   assert(self->element_size > 0);

   if ( idx_start == (idx_end - 1) )
   {
      return VectorRemove(self, idx_start, buf);
   }

   size_t num_of_removed = idx_end - idx_start;
   if ( NULL != buf )
   {
      VectorRangeCpy(self, idx_start, idx_end, buf);
   }
   // Only need to shift over if the removal does not include the end
   if ( idx_end < self->len )
   {
      shiftn(self, idx_end, ShiftDir_Left, num_of_removed);
   }
#ifdef SECURE_REMOVAL
   else
   {
      // Zero out that leftover data
      VectorRangeClear(self, idx_start, idx_end);
   }
#endif
   self->len -= num_of_removed;

   return true;
}

/******************************************************************************/
/******************************************************************************/

bool VectorRangeClear( struct Vector * self,
                       size_t idx_start,
                       size_t idx_end )
{
   assert( (self == NULL) ||
           (self != NULL &&
            ( (self->capacity == 0 && self->arr == NULL) ||
              (self->capacity >  0 && self->arr != NULL) ) ) );

   if ( self == NULL || !vec_isalloc(self) ||
        idx_start >= self->len || idx_end > self->len ||
        idx_start >= idx_end )
   {
      return false;
   }
   else if ( NULL == self->arr || 0 == self->len )
   {
      return true; // Trivial clear
   }

   memset(
      PTR_TO_IDX(self, idx_start),
      0,
      self->element_size * (idx_end - idx_start)
   );

   return true;
}

/******************************************************************************/

/* Private Function Implementations */

/**
 * @brief Expands the capacity of the vector to accommodate additional elements.
 *
 * @param self Vector handle.
 * @return true if the expansion was successful, false otherwise.
 */
static bool vec_expand( struct Vector * self )
{
   // Since this is a purely internal function, I will destructively assert at any invalid inputs
   assert(self != NULL);
   assert(self->element_size != 0);
   assert( (self->capacity == 0 && self->arr == NULL) ||
           (self->capacity >  0 && self->arr != NULL) );
   assert(self->len <= self->capacity);
   assert(self->len <= self->max_capacity);
   assert(self->mem_mgr.realloc != NULL);

   // If we're already at max capacity, can't expand further.
   if ( self->capacity == self->max_capacity )
   {
      return false;
   }

   // First determine new capacity, and then realloc.
   if ( 0 == self->capacity )
   {
      size_t new_capacity = (self->max_capacity < DEFAULT_INITIAL_CAPACITY) ?
                             self->max_capacity : DEFAULT_INITIAL_CAPACITY;
      self->arr = self->mem_mgr.alloc( new_capacity * self->element_size, self->mem_mgr.arena );
      if ( self->arr != NULL )
      {
         self->capacity = new_capacity;
         return true;
      }
   }
   else
   {
      size_t new_capacity;
      if ( (self->capacity * EXPANSION_FACTOR) < self->max_capacity )
      {
         new_capacity = self->capacity * EXPANSION_FACTOR;
      }
      else
      {
         new_capacity = self->max_capacity;
      }
      void * new_ptr = self->mem_mgr.realloc( self->arr,
                                              self->element_size * new_capacity,
                                              self->element_size * self->capacity,
                                              self->mem_mgr.arena );
      if ( new_ptr != NULL )
      {
         self->arr = new_ptr;
         self->capacity = new_capacity;
         return true;
      }
   }

   return false;
}

/**
 * @brief Expands the capacity of the vector by a specified number of elements.
 *
 * @param self Vector handle.
 * @param add_len The number of additional elements to expand the capacity by.
 * @return true if the expansion was successful; false otherwise.
 */
static bool vec_expandby( struct Vector * self, size_t add_cap )
{
   // Since this is a purely internal function, I will destructively assert at any invalid inputs
   assert(self != NULL);
   assert(self->element_size != 0);
   assert( (self->capacity == 0 && self->arr == NULL) ||
           (self->capacity >  0 && self->arr != NULL) );
   assert(self->len <= self->capacity);
   assert(self->len <= self->max_capacity);
   assert(self->mem_mgr.realloc != NULL);

   // If there's no space in the vector, we can't expand
   if ( add_cap > (self->max_capacity - self->capacity) )
   {
      return false;
   }

   if ( 0 == self->capacity )
   {
      size_t new_capacity = add_cap;
      self->arr = self->mem_mgr.alloc( new_capacity * self->element_size, self->mem_mgr.arena );
      if ( self->arr != NULL )
      {
         self->capacity = new_capacity;
         return true;
      }
   }
   else
   {
      size_t new_capacity = self->capacity + add_cap;
      void * new_ptr = self->mem_mgr.realloc( self->arr,
                                              self->element_size * new_capacity,
                                              self->element_size * self->capacity,
                                              self->mem_mgr.arena );
      if ( new_ptr != NULL )
      {
         self->arr = new_ptr;
         self->capacity = new_capacity;
         return true;
      }
   }

   return false;
}

/**
 * @brief Shifts elements in the vector either to the left or right from a given idx.
 *
 * @note This function won't automatically expand capacity. Make sure there's
 *       room before calling this function to shift right, or it will fail.
 * @note This function will also fail if you try to shift left by more than the
 *       length of the vector.
 * @param struct Vector * : Pointer to the Vector structure
 * @param size_t : The index at which the shift operation begins
 * @param enum ShiftDir : Direction of shift
 * @param n : Number of indices to shift by
 */
static void shiftn( struct Vector * self, size_t start_idx,
                    enum ShiftDir direction, size_t n )
{
   assert(self != NULL);
   assert(self->arr != NULL);
   // Length must be less than capacity (i.e., shiftn should only be called after expansions)
   assert(self->len < self->capacity);
   // Extra checks to trap paradox states
   assert(self->element_size > 0);
   // Don't try to shift data past the data range of the array
   assert(start_idx < self->len);
   assert(start_idx < MAX_VEC_LEN);
   assert(direction < ShiftDir_InvalidDir);
   assert( (direction == ShiftDir_Right && ((n + self->len) <= self->capacity)) ||
           (direction == ShiftDir_Left  && (n <= self->len)) );

   uint8_t * old_spot = NULL;
   uint8_t * new_spot = NULL;
   if ( direction == ShiftDir_Right )
   {
      old_spot = PTR_TO_IDX(self, start_idx);
      new_spot = PTR_TO_IDX(self, start_idx + n);
   }
   else // shift left
   {
      old_spot = PTR_TO_IDX(self, start_idx);
      new_spot = PTR_TO_IDX(self, start_idx - n);
   }
   // Use memmove instead of memcpy because shifting means the new state of the
   // sequence being moved and the old state of that sequence will overlap if
   // the sequence's length is greater than the amount of shifting. memmove is
   // specifically made for this kind of operation, whereas memcpy isn't
   // guaranteed to behave correctly here.
   memmove( new_spot, old_spot, (self->len - start_idx) * self->element_size );
}


/********************** Fixed-Size Object Arena Material **********************/

struct VectorPoolItem
{
   struct Vector vec;
   bool is_allocated;
};

struct VectorPool
{
   struct VectorPoolItem pool[VEC_STRUCT_POOL_SIZE];
   size_t next_idx;
};

static struct VectorPool VecPool;

/**
 * @brief Allocates a new Vector structure from a static arena.
 * @return Pointer to the allocated Vector struct if successful, NULL otherwise.
 */
static struct Vector * vec_pool_dispatch(void)
{
   assert( VecPool.next_idx < VEC_STRUCT_POOL_SIZE );
   assert( VecPool.pool != NULL );
#ifndef NDEBUG
   // If next idx is allocated, by design, that must mean we are out of vectors.
   if ( VecPool.pool[VecPool.next_idx].is_allocated == true )
   {
      for ( size_t i = 0; i < VEC_STRUCT_POOL_SIZE; i++ )
      {
         assert( VecPool.pool[i].is_allocated == true );
      }
   }
#endif

   if ( VecPool.pool[VecPool.next_idx].is_allocated )
   {
      return NULL;
   }

   struct Vector * new_vec = &VecPool.pool[VecPool.next_idx].vec;
   VecPool.pool[VecPool.next_idx].is_allocated = true;

   // 🗒️: Potential to place this in a separate asynchronous thread?
   // Find the next available spot
   size_t j = VecPool.next_idx;
   for ( size_t i = 1; i < VEC_STRUCT_POOL_SIZE; i++, j++ )
   {
      if ( j >= VEC_STRUCT_POOL_SIZE ) j = 0; // Wrap-around

      if ( !VecPool.pool[j].is_allocated )
      {
         VecPool.next_idx = j;
         break;
      }
   }

   return new_vec;
}

static void vec_pool_reclaim(const struct Vector * ptr)
{
   if ( NULL == ptr )
   {
      return;
   }

   // Find the vector address that matches this pointer
   bool found = false;
   for ( size_t i = 0; i < VEC_STRUCT_POOL_SIZE; i++ )
   {
      if ( ptr == &VecPool.pool[i].vec )
      {
         found = true;
         if ( !VecPool.pool[i].is_allocated )
         {
            // TODO: Raise exception for attempting to free an unallocated vec
         }
         VecPool.pool[i].is_allocated = false;
         break;
      }
   }

   if ( !found )
   {
      // TODO: Raise an exception for attempting to free a random address
   }
}

static bool vec_isalloc(const struct Vector * ptr)
{
   if ( ptr == NULL ||
        ptr < &VecPool.pool[0].vec ||
        ptr > &VecPool.pool[VEC_STRUCT_POOL_SIZE - 1].vec )
   {
      return false;
   }

   // Assumes the objects in VecPool are arranged in increasing order of memory
   int left = 0, right = VEC_STRUCT_POOL_SIZE - 1;
   int mid;
   while ( left <= right )
   {
      mid = left + (right - left) / 2;

      if ( ptr == &VecPool.pool[mid].vec )
         return VecPool.pool[mid].is_allocated;
      else if ( ptr < &VecPool.pool[mid].vec )
         right = mid - 1;
      else
         left = mid + 1;
   }

   return false;
}
