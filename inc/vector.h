/**
 * @file vector.c
 * @brief API for a dynamic array (vector) in C.
 *
 * @author Abdulla Almosalami (memphis242)
 * @date April 12, 2025
 * @copyright MIT License
 */

/* File Inclusions */
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "conficol_shared.h"
#include "vector_cfg.h"
// TODO: Exceptions

/* Public Macro Definitions */
#define VITER_RESET(iter) (iter.curr_idx = iter.init_idx)

/* Public Datatypes */

// Opaque type declaration to act as a handle for the user to pass into the API
struct Vector;
struct VIterator;

enum IterDirection
{
   IterDir_Right,
   IterDir_Left,
   IterDir_Normal=IterDir_Right,
   IterDir_Reverse=IterDir_Left,
   IterDir_RightWrap,
   IterDir_LeftWrap,
   IterDir_RightBounce,
   IterDir_LeftBounce,
   NumOfIterDirs
};

/* Public API */

/*************************** Constructor/Destructor ***************************/

/**
 * @brief Constructor
 * @param element_size     The size of each element in the vector (in bytes)
 * @param initial_capacity The initial number of elements the vector can hold
 * @param max_capacity     Maximum number of elements the vector can hold ever
 * @param init_data        Data to initialize the array if desired; NULL otherwise
 * @param init_dlen        Length of initialization data, if applicable; 0 otherwise
 * @param mem_mgr          Allocator that the user provides; if NULL, defaults to stdlib
 *                         This is to be used for the underlying data array only.
 *                         The vector object itself will come from a static fixed-size
 *                         object pool internally.
 * @return A pointer to the initialized vector, or NULL if allocation fails.
 * @example
 *    struct Vector * v1 = VectorNew(sizeof(int), 10, 100, NULL, 0, NULL);
 *    struct Vector * v2 = VectorNew(sizeof(int), 10, 100, (int[]){1, 2, 3}, 3, NULL);
 *    struct Allocator my_allocator = { .alloc = myalloc; ... };
 *    struct MyData { ... some struct definition ... };
 *    struct Vector * v3 = VectorNew(sizeof(struct MyData), 10, 100, (struct MyData[]){...}, 3, &my_allocator);
 */
struct Vector * VectorNew( size_t element_size,
                           size_t init_capacity,
                           size_t max_capacity,
                           void * init_data,
                           size_t init_dlen,
                           const struct Allocator * mem_mgr );

/**
 * @brief Destructor
 * @param self Vector handle (if NULL, nothing happens)
 */
void VectorFree( struct Vector * self );

/******************** Vector-Vector Operations (Copy/Move) ********************/

/**
 * @brief Copy constructor (deep).
 * @param self Vector handle (if NULL, nothing happens)
 */
struct Vector * VectorDuplicate( const struct Vector * self );

/**
 * @brief Move constructor.
 * @note No memory is allocated for the destination vector.
 * @note dest needs to have the same element size and allocator as src
 * @param self Vector handle (if NULL, nothing happens)
 */
bool VectorMove( struct Vector * dest, struct Vector * src );

/**
 * @brief Equivalent lengths, sizes, maximums, and underlying data.
 * @param a Vector handle for the first vector
 * @param b Vector handle for the second vector
 */
bool VectorsAreEqual( const struct Vector * a, const struct Vector * b );

/**
 * @brief Concatenates two vectors into a new vector.
 * @note The two vectors must at least have elements of the same size
 * @note Empty vectors are allowed
 * @note Sum of lens must be less than the max len of any single vector
 * @note The resultant vector shall have a length, capacity, and max capacity
 *       that is a sum of the argument vectors.
 * @note v1's allocator will be used, not v2's.
 * @param v1 The first vector (elements will appear first in the result).
 * @param v2 The second vector (elements will appear after v1's elements).
 * @return Pointer to the newly allocated vector, or NULL on failure.
 */
struct Vector * VectorConcatenate( const struct Vector * v1,
                                   const struct Vector * v2 );

/******************************** Basic Stats *********************************/

/**
 * @brief Retrieves the number of elements in the vector.
 * @param self Vector handle (if NULL, nothing happens)
 */
size_t VectorLength( const struct Vector * self );

/**
 * @brief Retrieves the current capacity of the vector.
 * @param self Vector handle (if NULL, nothing happens)
 */
size_t VectorCapacity( const struct Vector * self );

/**
 * @brief Retrieves the maximum capacity of the vector.
 * @param self Vector handle (if NULL, nothing happens)
 */
size_t VectorMaxCapacity( const struct Vector * self );

/**
 * @brief Retrieves the size of each element in the vector.
 * @param self Vector handle (if NULL, nothing happens)
 */
size_t VectorElementSize( const struct Vector * self );

/**
 * @brief Checks if the vector is empty.
 * @param self Vector handle (if NULL, nothing happens)
 * @return true if empty, false otherwise.
 */
bool VectorIsEmpty( const struct Vector * self );

/**
 * @brief Checks if the vector is full - i.e., at max capacity.
 * @param self Vector handle (if NULL, nothing happens)
 * @return true if full, false otherwise.
 */
bool VectorIsFull( const struct Vector * self );

/******************************** Vector Ops **********************************/

/**
 * @brief Inserts an element at the _end_ of the vector.
 * @param self Vector handle (if NULL, nothing happens)
 * @param element A pointer to the element to be inserted
 * @return `true` if the insertion is successful, `false` otherwise.
 */
bool VectorPush( struct Vector * self, const void * element );

/**
 * @brief Inserts an element into the vector at the specified index.
 * @note You are not allowed to insert past the length of the vector.
 * @note Causes shift of elements when insertion is in the middle
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx The index at which the element should be inserted.
 * @param element Pointer to the element to be inserted.
 * @return true if the element was successfully inserted, false otherwise
 *         (e.g., memory allocation fails).
 */
bool VectorInsert( struct Vector * self, size_t idx, const void * element );

/**
 * @brief Retrieves a pointer to the element at the specified index in the vector.
 * @note Future vector operations may render this pointer stale
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx The index of the element to retrieve, 0-indexed.
 * @return A pointer to the element if the retrieval was successful; NULL otherwise
 */
void * VectorGet( const struct Vector * self, size_t idx );

/**
 * @brief Retrieves a pointer to the last element in the vector.
 * @param self Vector handle (if NULL, nothing happens)
 * @return Pointer to the data that will be returned.
 */
void * VectorLastElement( const struct Vector * self );

/**
* @brief Copies the element at the specified index in the vector to a provided buffer.
* @param self Vector handle (if NULL, nothing happens)
* @param idx The index of the element to retrieve, 0-indexed.
* @param data Pointer to the buffer where the element will be copied.
* @return true if the retrieval and copy were successful, false otherwise
*/
bool VectorCpyElementAt( const struct Vector * self, size_t idx, void * data );

/**
 * @brief Copies the last element in the vector to a provided buffer.
 * @param self Vector handle (if NULL, nothing happens)
 * @param data Pointer to the buffer where the last element will be copied.
 * @return true if the last element was successfully copied to the buffer, false otherwise
 */
bool VectorCpyLastElement( const struct Vector * self, void * data );

/**
 * @brief Sets the value of an element at the specified index in the vector.
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx The index of the element to update, 0-indexed.
 * @param element A pointer to the new value to set at the specified index.
 * @return true if the update was successful, false otherwise
 */
bool VectorSet( struct Vector * self, size_t idx, const void * element );

/**
 * @brief Removes an element from the vector at the specified index and optional
 *        retrieves its data.
 * @note No realloc is done to release memory.
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx The index of the element to be removed
 * @param data Pointer to buffer to copy data to (if NULL, deletion still occurs)
 * @return true if the element was removed, false otherwise
 */
bool VectorRemove( struct Vector * self, size_t idx, void * data );

/**
 * @brief Removes the last element from the vector and optionally retrieves its data.
 * @param self Vector handle (if NULL, nothing happens)
 * @param data Pointer to buffer to copy data to (if NULL, deletion still occurs)
 * @return true if the element was removed, false otherwise
 */
bool VectorRemoveLastElement( struct Vector * self, void * data );

/**
 * @brief Clears (zeros) the element at the specified index in the vector.
 * @param self Vector handle (if NULL, nothing happens).
 * @param idx Index of the element to clear.
 * @return true if the element was successfully cleared, false otherwise
 */
bool VectorClearElementAt( struct Vector * self, size_t idx );

/**
 * @brief Clears (zeros) all elements in the vector.
 * @param self Vector handle (if NULL, nothing happens)
 * @return true if the operation was successful, false otherwise
 */
bool VectorClear( struct Vector * self );

/**
 * @brief Resets the vector length to zero
 * @note Leaves the previously allocated memory for the vector's data untouched.
 * @param self Vector handle (if NULL, nothing happens)
 */
bool VectorReset( struct Vector * self );

/**
 * @brief Zeros elements first, then frees, then resets the vector length and capacity to zero.
 * @param self Vector handle (if NULL, nothing happens)
 */
bool VectorHardReset( struct Vector * self );

/*************************** Vector Range Ops *****************************/

/**
 * @brief Splits a vector at the specified index into two, returning the second
 *        half in a new vector and truncating the original vector to the first half.
 * @note Passing in an idx of 0 results no action being taken and NULL return
 * @note This will mutate the original vector.
 * @param self Pointer to the original vector to be split.
 * @param idx The index at which to split the vector. Elements from this index
 *            onward will be moved to the new vector.
 * @return Vector that received second half of split; NULL otherwise
 */
struct Vector * VectorSplitAt( struct Vector * self, size_t idx );

/**
 * @brief Creates a slice (subvector) from the given vector.
 * @param self Pointer to the original Vector structure.
 * @param idx_start The starting index of the slice (inclusive).
 * @param idx_end The ending index of the slice (exclusive).
 * @return Pointer to a new Vector containing the specified slice; NULL otherwise
 */
struct Vector * VectorSlice( const struct Vector * self, size_t idx_start, size_t idx_end );

/**
 * @brief Pushes several elements into the vector.
 * @param self Vector handle (if NULL, nothing happens)
 * @param data Pointer to the source data to be copied into the vector.
 * @param dlen Number of elements to push into the vector.
 * @return true if the elements were successfully pushed; false otherwise
 */
bool VectorRangePush( struct Vector * self, const void * data, size_t dlen );

/**
 * @brief Inserts a range of elements into the vector at the specified index.
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx  Index at which to insert the new elements.
 * @param data Pointer to the data to be inserted (not mutated).
 * @param dlen Number of elements to insert.
 * @return true if the insertion was successful, false otherwise
 */
bool VectorRangeInsert( struct Vector * self, size_t idx, const void * data, size_t dlen );

/**
 * @brief Copies elements from a specified range in the vector to a provided buffer.
 * @note Passing in the same start and end results in nothing being copied.
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx_start The starting index of the range (inclusive).
 * @param idx_end The ending index of the range (exclusive).
 * @param buffer Pointer to the destination buffer where the elements will be copied.
 * @return true if the elements were successfully copied; false otherwise
 */
bool VectorRangeCpy( const struct Vector * self, size_t idx_start, size_t idx_end, void * buffer );

/**
 * @brief Copies elements from the specified index to the end of the vector into a provided buffer.
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx The starting index from which elements will be copied (inclusive).
 * @param buffer Pointer to the destination buffer where the elements will be copied.
 * @return true if the operation is successful, false otherwise
 */
bool VectorRangeCpyToEnd( const struct Vector * self, size_t idx, void * buffer );

/**
 * @brief Sets the data in a sub-range of the vector to the data array provided.
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx_start The starting index of the range (inclusive).
 * @param idx_end The ending index of the range (exclusive).
 * @param arr Pointer to the data array that contains the new values for the subrange.
 * @note Of course, arr should be sized at least (idx_end - idx_start).
 * @note This is not to set the vector sub-range to a single value. Use VectorRangeSetToVal for that.
 * @example
 *          Assume vec is a vector of int's with 20 elements like so:
 *          vec: { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ... (20 0's in total) }
 * 
 *          int arr[5] = { 1, 2, 3, 4, 5 };
 *          VectorRangeSetWithArr(vec, 4, 9, arr );
 * 
 *          Results in:
 *          vec: { 0, 0, 0, 0, 1, 2, 3, 4, 5, 0, ... }
 * 
 *          VectorRangeSetWithArr(vec, 4, 9, &(int){5}); <-- INVALID!
 *
 * @return true if the operation is successful, false otherwise
 */
bool VectorRangeSetWithArr( struct Vector * self, size_t idx_start, size_t idx_end, const void * arr );

/**
 * @brief Sets a sub-range of the vector to the single value provided.
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx_start The starting index of the range (inclusive).
 * @param idx_end The ending index of the range (exclusive).
 * @param val Pointer to the value to set each element in the subrange to.
 * @example Assume vec is a vector of int's with 20 elements like so:
 *          vec: { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ... (20 0's in total) }
 * 
 *          VectorRangeSetToVal(vec, 4, 9, &(int){5} );
 * 
 *          Results in:
 *          vec: { 0, 0, 0, 0, 5, 5, 5, 5, 5, 0, ... }
 *
 * @return true if the operation is successful, false otherwise
 */
bool VectorRangeSetToVal( struct Vector * self, size_t idx_start, size_t idx_end, const void * val );

/**
 * @brief Removes elements from the vector within the specified range.
 * @note Passing in the same start and end results in nothing being done.
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx_start The starting index of the range (inclusive).
 * @param idx_end The ending index of the range (exclusive).
 * @param buf Buffer to hold removed data (optional).
 * @return true if the elements were successfully removed, false otherwise.
 */
bool VectorRangeRemove( struct Vector * self, size_t idx_start, size_t idx_end, void * buf );

/**
 * @brief Clears (zeros) the elements in the specified range.
 * @param self Vector handle (if NULL, nothing happens)
 * @param idx_start The starting index of the range to clear (inclusive).
 * @param idx_end The ending index of the range to clear (exclusive).
 * @return true if the operation was successful, false otherwise
 */
bool VectorRangeClear( struct Vector * self, size_t idx_start, size_t idx_end );

/***************************** Vector Iterator API ****************************/

/**
 * @brief Creates a new VIterator object that can be used to iterate over a vector
 *        in a variety of ways (forward, reverse, wrap, bounce, ...).
 * @param vec The vector to associate /w this iterator
 * @param idx_start The starting index of the iterator
 * @param idx_end The index _after_ the one you wish to finally go through
 *                (e.g., to go up to _and including_ index i, you'd use i ± 1
 *                 depending on the direction you're going, including 1 out of
 *                 bounds of the vector)
 * @param direction The direction the iterator will go through the vector. See
 *                  the enum IterDirection list for your options.
 * @note For a given direction, only the following start/end idx values are valid:
 *    - IterDir_Right: start < end, start ≥ 0, end ≤ len
 *    - IterDir_Left:  start > end, start > 0, end ≥ -1
 *    - IterDir_RightWrap: start ≥ 0, ≤ len - 1; end ≥  0, ≤ len
 *    - IterDir_LeftWrap:  start ≥ 0, ≤ len - 1; end ≥ -1, ≤ len - 1
 *    - IterDir_RightBounce: start ≥ 0, ≤ len - 1; end ≥  0, ≤ len
 *    - IterDir_LeftBounce:  start ≥ 0, ≤ len - 1; end ≥ -1, ≤ len - 1
 * @return The new VIterator object if cfg is valid & allocation succeeded; NULL otherwise
 */
struct VIterator * VIteratorNew( const struct Vector * vec,
                                 ptrdiff_t idx_start,
                                 ptrdiff_t idx_end,
                                 enum IterDirection direction );

/**
 * @brief Free the memory allocated for the iterator.
 * @param it Iterator handle (if NULL, nothing happens)
 * @return none
 */
void VIteratorFree( struct VIterator * it );

/**
 * @brief Return a pointer to the data being pointed to by the iterator.
 * @param it The iterator handle
 * @return Pointer to the data at the current idx of the iterator
 */
void * VIteratorData( const struct VIterator * it );

/**
 * @brief Retrieve the initial idx of the iterator from its conception.
 * @param it The iterator handle
 */
ptrdiff_t VIteratorInitIdx( const struct VIterator * it );

/**
 * @brief Retrieve the current idx of the iterator.
 * @param it The iterator handle
 */
ptrdiff_t VIteratorCurrIdx( const struct VIterator * it );

/**
 * @brief Retrieve the end idx of the iterator from its conception.
 * @param it The iterator handle
 */
ptrdiff_t VIteratorEndIdx( const struct VIterator * it );

/**
 * @brief Returns whether the iterator has been nudged to its limit (after the final index)
 * @param it The iterator handle
 * @example Say the iterator is over indices 0 _through_ 5 (idx_start = 0, idx_end = 6).
 *          The nudge from 4 → 5 does **not** constitute a limit being hit.
 *          However, nudging again after curr_idx == 5 does cause the "limit hit"
 *          flag to be set. This is useful in for-each looping as the end condition.
 */
bool VIteratorLimitHit( const struct VIterator * it );

/**
 * @brief Reset the current idx of the iterator to its initial idx.
 * @param it The iterator handle
 */
void VIteratorReset( struct VIterator * it );

/**
 * @brief Moves the current index of the iterator to the applicable next index.
 * @param it Iterator handle (if NULL, returns false)
 * @return true if the nudge was successful, false otherwise
 */
bool VIteratorNudge( struct VIterator * it );

/**
 * @brief See the next index of the iterator, but don't mutate the iterator.
 * @param it Iterator handle (if NULL, returns PTRDIFF_MAX)
 * @return next index if valid iterator, PTRDIFF_MAX otherwise
 */
ptrdiff_t VIteratorPeek( struct VIterator * it );


/** Iterator Convenience Macros **/
// TODO: Handle failure to allocate iterator in macros to alert user. Sep DBG v REL versions?

#define FOREACH_VEC_READ(type, var, vector, body) \
   { \
      struct VIterator * _it_29LbM3 = \
         VIteratorNew( \
            vector, \
            0, \
            (ptrdiff_t)VectorLength(vector), \
            IterDir_Normal \
         );\
      assert( _it_29LbM3 != NULL ); \
      if ( _it_29LbM3 != NULL ) \
      { \
         for ( type var = *(type *)VIteratorData(_it_29LbM3); \
               !VIteratorLimitHit(_it_29LbM3); \
               (void)VIteratorNudge(_it_29LbM3), var = *(type *)VIteratorData(_it_29LbM3) ) \
         { \
            body \
         } \
      } \
      VIteratorFree(_it_29LbM3); \
   }

#define FOREACH_VEC_REF(type, var_ptr, vector, body) \
   { \
      struct VIterator * _it_29LbM3 = \
         VIteratorNew( \
            vector, \
            0, \
            (ptrdiff_t)VectorLength(vector), \
            IterDir_Normal \
         );\
      assert( _it_29LbM3 != NULL ); \
      if ( _it_29LbM3 != NULL ) \
      { \
         for ( type var_ptr = VIteratorData(_it_29LbM3); \
               !VIteratorLimitHit(_it_29LbM3); \
               (void)VIteratorNudge(_it_29LbM3), var_ptr = VIteratorData(_it_29LbM3) ) \
         { \
            body \
         } \
      } \
      VIteratorFree(_it_29LbM3); \
   }

#define FOREACH_VEC_READ_RNG(type, var, vector, idx_start, idx_end, direction, body) \
   { \
      struct VIterator * _it_29LbM3 = \
         VIteratorNew( \
            vector, \
            (ptrdiff_t)idx_start, \
            (ptrdiff_t)idx_end, \
            direction \
         );\
      assert( _it_29LbM3 != NULL ); \
      if ( _it_29LbM3 != NULL ) \
      { \
         for ( type var = *(type *)VIteratorData(_it_29LbM3); \
               !VIteratorLimitHit(_it_29LbM3); \
               (void)VIteratorNudge(_it_29LbM3), var = *(type *)VIteratorData(_it_29LbM3) ) \
         { \
            body \
         } \
      } \
      VIteratorFree(_it_29LbM3); \
   }

#define FOREACH_VEC_REF_RNG(type, var_ptr, vector, idx_start, idx_end, direction, body) \
   { \
      struct VIterator * _it_29LbM3 = \
         VIteratorNew( \
            vector, \
            (ptrdiff_t)idx_start, \
            (ptrdiff_t)idx_end, \
            direction \
         );\
      assert( _it_29LbM3 != NULL ); \
      if ( _it_29LbM3 != NULL ) \
      { \
         for ( type var_ptr = VIteratorData(_it_29LbM3); \
               !VIteratorLimitHit(_it_29LbM3); \
               (void)VIteratorNudge(_it_29LbM3), var_ptr = VIteratorData(_it_29LbM3) ) \
         { \
            body \
         } \
      } \
      VIteratorFree(_it_29LbM3); \
   }

#define FOREACH_VEC_READ_REVERSE(type, var, vector, body) \
   FOREACH_VEC_READ_RNG(type, var, vector, VectorLength(vector)-1, -1, IterDir_Reverse, body)

#define FOREACH_VEC_REF_REVERSE(type, var_ptr, vector, body) \
   FOREACH_VEC_REF_RNG(type, var_ptr, vector, VectorLength(vector)-1, -1, IterDir_Reverse, body)
