/*!
 * @file    test_vector.c
 * @brief   Test file for the the vector component
 *
 * @author  Abdullah Almosalami @memphis242
 * @date    Tues Apr 19, 2025
 * @copyright MIT License
 */

/* File Inclusions */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <time.h>
#include <unity/unity.h>
#include <unity/unity_memory.h>

#include "vector.h"

/* Local Macro Definitions */
#define MAX_INIT_ATTEMPTS 100 //! Used in the VECTOR_NEW_KEEP_TRYIN macro to limit VectorNew(, &DEFAULT_ALLOCATOR) attempst
#define ARR_LEN(arr) ( sizeof(arr) / sizeof(arr[0]) )

#define VECTOR_NEW_KEEP_TRYIN(ptr, elsz, icap, mcap, init_data, init_dlen, mem_mgr) \
{                                                                              \
   size_t iter = 0;                                                            \
   do                                                                          \
   {                                                                           \
      VectorFree(ptr);                                                         \
      ptr = VectorNew( elsz, icap, mcap, init_data, init_dlen, mem_mgr);       \
      iter++;                                                                  \
   } while (                                                                   \
      (NULL == ptr) &&                                                         \
      (VectorCapacity(ptr) == 0) &&                                            \
      (iter < MAX_INIT_ATTEMPTS)                                               \
   );                                                                          \
}

/* Datatypes */

/* Local Variables */

void * test_nil_alloc(size_t req_sz, void * ctx);
void * test_nil_realloc(void * old_ptr, size_t new_sz, size_t old_sz, void * ctx);
void test_nil_reclaim(void * old_ptr, size_t old_sz, void * ctx);

static const struct Allocator TestNilMemMgr =
{
   .alloc = test_nil_alloc,
   .realloc = test_nil_realloc,
   .reclaim = test_nil_reclaim,
   .alloca_init = NULL,
   .arena = NULL
};

/* Forward Function Declarations */

void setUp(void);
void tearDown(void);

void test_VectorNew_Invalid_ZeroElementSz(void);
void test_VectorNew_Invalid_MaxCapLessThanInitCap(void);
void test_VectorNew_Invalid_ZeroMaxCap(void);
void test_VectorNew_Invalid_InitialLen(void);
void test_VectorNew_ValidInputCombo_3DPoints(void);
void test_VectorNew_CapacityLimit(void);
void test_VectorNew_ElementSzLimit(void);
void test_VectorNew_InitData(void);
void test_VectorNew_InitialLenLessThanInitialCap(void);
void test_VectorNew_InitialLenSameAsInitialCap(void);

void test_VectorOpsOnNullVectors(void);

void test_VectorFree(void);
void test_VectorLength(void);
void test_VectorCapacity(void);
void test_VectorMaxCapacity(void);
void test_VectorElementSize(void);
void test_VectorIsEmpty(void);
void test_VectorIsFull(void);

void test_VectorPush_SimplePush(void);
void test_VectorPush_UntilCapacity(void);
void test_VectorPush_PastInitialCapacity(void);
void test_VectorPush_PastMaxCapacity(void);
void test_VectorPush_IntoVecWithZeroMaxCap(void);
void test_VectorPush_InitialCapOfZero(void);
void test_VectorPush_AfterResetting(void);
void test_VectorPush_AfterHardResetting(void);

void test_VectorInsertion_AtZeroWithVectorLessThanCapacity(void);
void test_VectorInsertion_AtZeroWithVectorAtCapacity(void);
void test_VectorInsertion_AtZeroWithVectorAtMaxCapacity(void);
void test_VectorInsertion_AtEndEqualsVecPush(void);
void test_VectorInsertion_AtMiddle(void);
void test_VectorInsertion_AtMiddleOfEmptyVec(void);

void test_VectorGetElement_ValidIdx(void);
void test_VectorGetElement_IdxPastLen(void);
void test_VectorGetElement_IdxPastCap(void);
void test_VectorLastElement(void);

void test_VectorCpyElement_ValidIdx(void);
void test_VectorCpyElement_NullBufferPassedIn(void);
void test_VectorCpyElement_IdxPastLen(void);
void test_VectorCpyElement_IdxPastCap(void);
void test_VectorCpyLastElement(void);
void test_VectorRoundTrip_CpyElementToSetElement(void);

void test_VectorSetElement_AfterPushes(void);
void test_VectorSetElement_AfterInitLen(void);
void test_VectorSetElement_PastLen(void);
void test_VectorSetElement_PastCap(void);

void test_VectorRemoveElement_AtZeroWithVectorPartiallyFull(void);
void test_VectorRemoveElement_AtZeroWithSinglePresentElement(void);
void test_VectorRemoveElement_AtZeroWithEmptyVector(void);
void test_VectorRemoveElement_AtMiddle(void);
void test_VectorRemoveElement_AtLen(void);
void test_VectorRemoveElement_LastElement(void);
void test_VectorRemoveElement_PastLen(void);
void test_VectorRemoveElement_AtZeroWithVectorPartiallyFull_WithBuf(void);
void test_VectorRemoveElement_AtZeroWithSinglePresentElement_WithBuf(void);
void test_VectorRemoveElement_AtZeroWithEmptyVector_WithBuf(void);
void test_VectorRemoveElement_AtMiddle_WithBuf(void);
void test_VectorRemoveElement_AtLen_WithBuf(void);
void test_VectorRemoveElement_LastElement_WithBuf(void);
void test_VectorRemoveElement_PastLen_WithBuf(void);
void test_VectorRemoveLastElement_EmptyVec(void);

void test_VectorClearElementAt_Normal(void);
void test_VectorClearElementAt_InvalidIdx(void);

void test_VectorClear_Normal(void);
void test_VectorClear_EmptyVec(void);
void test_VectorClear_InvalidVec(void);

void test_VectorReset_EmptyVec(void);
void test_VectorReset(void);

void test_VectorHardReset(void);
void test_VectorHardReset_EmptyVector(void);

void test_VectorDuplicate_SmallVector(void);
void test_VectorDuplicate_ReallyLargeVector(void);
void test_VectorDuplicate_CatchBadCapDuplication(void);
void test_VectorDuplicate_NullVector(void);

void test_VectorMove_SmallVector(void);
void test_VectorMove_ReallyLargeVector(void);
void test_VectorMove_NullVector(void);
void test_VectorMove_MismatchedVec(void);

void test_VectorsAreEqual_SameVectors(void);
void test_VectorsAreEqual_DifferentElementSz(void);
void test_VectorsAreEqual_DifferentLength(void);
void test_VectorsAreEqual_DifferentCapacity(void);
void test_VectorsAreEqual_DifferentMaxCapacity(void);
void test_VectorsAreEqual_DifferentElementValues(void);

void test_VectorSplitAt_ValidIdx(void);
void test_VectorSplitAt_IdxZero(void);
void test_VectorSplitAt_IdxPastLen(void);
void test_VectorSplitAt_EmptyVector(void);
void test_VectorSplitAt_NullVector(void);
void test_VectorSplitAt_ValidIdx_StructData(void);

void test_VectorSlice_ValidIndices_IntData(void);
void test_VectorSlice_ValidIndices_StructData(void);
void test_VectorSlice_IdxStartEqualsIdxEnd(void);
void test_VectorSlice_IdxStartZero(void);
void test_VectorSlice_FullVector(void);
void test_VectorSlice_IdxEndAtLastElement(void);
void test_VectorSlice_EmptyVector(void);
void test_VectorSlice_NullVector(void);
void test_VectorSlice_IdxStartGreaterThanIdxEnd(void);
void test_VectorSlice_IdxEndOutOfRange(void);

void test_VectorConcatenate_BasicUse(void);
void test_VectorConcatenate_VecsNotMutated(void);
void test_VectorConcatenate_OneVecIsEmpty(void);
void test_VectorConcatenate_BothVecsEmpty(void);
void test_VectorConcatenate_FullVectors(void);
void test_VectorConcatenate_NullArguments(void);
void test_VectorConcatenate_DifferentElementSizes(void);
void test_VectorConcatenate_ConcatenateSplitRoundTrip(void);

void test_VectorRangePush_ValidInts(void);
void test_VectorRangePush_ValidStructs(void);
void test_VectorRangePush_ExpandCapacity(void);
void test_VectorRangePush_ZeroLen(void);
void test_VectorRangePush_NullVec(void);
void test_VectorRangePush_NullData(void);
void test_VectorRangePush_ExceedsMaxCapacity(void);
void test_VectorRangePush_ExactlyMaxCapacity(void);

void test_VectorRangeInsert_ValidInts(void);
void test_VectorRangeInsert_ValidStructs(void);
void test_VectorRangeInsert_ExpandCapacity(void);
void test_VectorRangeInsert_ZeroLen(void);
void test_VectorRangeInsert_NullVec(void);
void test_VectorRangeInsert_NullData(void);
void test_VectorRangeInsert_ExceedsMaxCapacity(void);
void test_VectorRangeInsert_ExactlyMaxCapacity(void);
void test_VectorRangeInsert_Push_Equivalence(void);
void test_VectorRangeInsert_InvalidIdx(void);
void test_VectorRangeInsert_OneElm(void);
void test_VectorRangeInsert_AfterHardResetting(void);

void test_VectorRangeCpy_ValidIdices_IntData(void);
void test_VectorRangeCpy_DoesNotMutate(void);
void test_VectorRangeCpy_ValidIndices_StructData(void);
void test_VectorRangeCpy_FullVector(void);
void test_VectorRangeCpy_FullVector_IncorrectEndIdx(void);
void test_VectorRangeCpy_EmptyVec(void);
void test_VectorRangeCpy_InvalidIdx(void);
void test_VectorRangeCpy_InvalidVec(void);
void test_VectorRangeCpy_SameIdices(void);

void test_VectorRangeCpyToEnd_ValidIdices_IntData(void);
void test_VectorRangeCpyToEnd_DoesNotMutate(void);
void test_VectorRangeCpyToEnd_ValidIndices_StructData(void);
void test_VectorRangeCpyToEnd_FullVector(void);
void test_VectorRangeCpyToEnd_EmptyVec(void);
void test_VectorRangeCpyToEnd_InvalidIdx(void);
void test_VectorRangeCpyToEnd_InvalidVec(void);
void test_VectorRangeCpyToEnd_EndIdx(void);

void test_VectorRangeSetWithArr_ValidIdices_IntData(void);
void test_VectorRangeSetWithArr_DoesNotMutate(void);
void test_VectorRangeSetWithArr_ValidIndices_StructData(void);
void test_VectorRangeSetWithArr_FullVector(void);
void test_VectorRangeSetWithArr_FullVector_IncorrectEndIdx(void);
void test_VectorRangeSetWithArr_EmptyVec(void);
void test_VectorRangeSetWithArr_InvalidIdx(void);
void test_VectorRangeSetWithArr_InvalidVec(void);
void test_VectorRangeSetWithArr_SameIdices(void);
void test_VectorRange_RoundTrip_CpyElementsToSetElementsInRng(void);

void test_VectorRangeSetToVal_ValidIdices_IntData(void);
void test_VectorRangeSetToVal_ValidIndices_StructData(void);
void test_VectorRangeSetToVal_FullVector(void);
void test_VectorRangeSetToVal_FullVector_IncorrectEndIdx(void);
void test_VectorRangeSetToVal_EmptyVec(void);
void test_VectorRangeSetToVal_InvalidIdx(void);
void test_VectorRangeSetToVal_InvalidVec(void);
void test_VectorRangeSetToVal_SameIdices(void);
void test_VectorRangeSetToVal_NullValue(void);
void test_VectorRangeSetToVal_SingleElement(void);
void test_VectorRangeSetToVal_AtBeginning(void);
void test_VectorRangeSetToVal_AtEnd(void);
void test_VectorRangeSetToVal_ZeroValue(void);
void test_VectorRangeSetToVal_EquivalenceWithRangeClear(void);

void test_VectorRange_RemoveElementsInRng_Normal(void);
void test_VectorRange_RemoveElementsInRng_NormalWithBuf(void);
void test_VectorRange_RemoveElementsInRng_AllElements(void);
void test_VectorRange_RemoveElementsInRng_EmptyVec(void);
void test_VectorRange_RemoveElementsInRng_AtBeginning(void);
void test_VectorRange_RemoveElementsInRng_AtEnd(void);
void test_VectorRange_RemoveElementsInRng_InvalidIndices(void);
void test_VectorRange_RemoveElementsInRng_InvalidVec(void);
void test_VectorRange_RoundTrip_InsertAndRemove(void);
void test_VectorRangeRemove_OneElm(void);

void test_VectorRange_ClearElementsInRng_Normal(void);
void test_VectorRange_ClearElementsInRng_AllElements(void);
void test_VectorRange_ClearElementsInRng_EmptyVec(void);
void test_VectorRange_ClearElementsInRng_AtBeginning(void);
void test_VectorRange_ClearElementsInRng_AtEnd(void);
void test_VectorRange_ClearElementsInRng_InvalidIndices(void);
void test_VectorRange_ClearElementsInRng_InvalidVec(void);

void test_VIterator_BasicRead_FullVec(void);
void test_VIterator_BasicUpdate_FullVec(void);
void test_VIterator_BasicRead_FullVec_Reverse(void);
void test_VIterator_BasicUpdate_FullVec_Reverse(void);
void test_VIterator_BasicRead_SubRng_Normal(void);
void test_VIterator_BasicUpdate_SubRng_Normal(void);
void test_VIterator_BasicRead_SubRng_Reverse(void);
void test_VIterator_BasicUpdate_SubRng_Reverse(void);
void test_VIterator_BasicRead_SubRng_NormalWithWrap(void);
void test_VIterator_BasicUpdate_SubRng_NormalWithWrap(void);
void test_VIterator_BasicRead_SubRng_ReverseWithWrap(void);
void test_VIterator_BasicUpdate_SubRng_ReverseWithWrap(void);

/* Meat of the Program */

int main(void)
{
   UNITY_BEGIN();

   RUN_TEST(test_VectorNew_Invalid_ZeroElementSz);
   RUN_TEST(test_VectorNew_Invalid_MaxCapLessThanInitCap);
   RUN_TEST(test_VectorNew_Invalid_ZeroMaxCap);
   RUN_TEST(test_VectorNew_Invalid_InitialLen);
   RUN_TEST(test_VectorNew_ValidInputCombo_3DPoints);
   RUN_TEST(test_VectorNew_CapacityLimit);
   RUN_TEST(test_VectorNew_ElementSzLimit);
   RUN_TEST(test_VectorNew_InitData);
   RUN_TEST(test_VectorNew_InitialLenLessThanInitialCap);
   RUN_TEST(test_VectorNew_InitialLenSameAsInitialCap);

   RUN_TEST(test_VectorOpsOnNullVectors);

   RUN_TEST(test_VectorFree);
   RUN_TEST(test_VectorLength);
   RUN_TEST(test_VectorCapacity);
   RUN_TEST(test_VectorMaxCapacity);
   RUN_TEST(test_VectorElementSize);
   RUN_TEST(test_VectorIsEmpty);
   RUN_TEST(test_VectorIsFull);

   RUN_TEST(test_VectorPush_SimplePush);
   RUN_TEST(test_VectorPush_UntilCapacity);
   RUN_TEST(test_VectorPush_PastInitialCapacity);
   RUN_TEST(test_VectorPush_PastMaxCapacity);
   RUN_TEST(test_VectorPush_IntoVecWithZeroMaxCap);
   RUN_TEST(test_VectorPush_InitialCapOfZero);
   RUN_TEST(test_VectorPush_AfterResetting);
   RUN_TEST(test_VectorPush_AfterHardResetting);

   RUN_TEST(test_VectorInsertion_AtZeroWithVectorLessThanCapacity);
   RUN_TEST(test_VectorInsertion_AtZeroWithVectorAtCapacity);
   RUN_TEST(test_VectorInsertion_AtZeroWithVectorAtMaxCapacity);
   RUN_TEST(test_VectorInsertion_AtEndEqualsVecPush);
   RUN_TEST(test_VectorInsertion_AtMiddle);
   RUN_TEST(test_VectorInsertion_AtMiddleOfEmptyVec);

   RUN_TEST(test_VectorGetElement_ValidIdx);
   RUN_TEST(test_VectorGetElement_IdxPastLen);
   RUN_TEST(test_VectorGetElement_IdxPastCap);
   RUN_TEST(test_VectorLastElement);

   RUN_TEST(test_VectorCpyElement_ValidIdx);
   RUN_TEST(test_VectorCpyElement_NullBufferPassedIn);
   RUN_TEST(test_VectorCpyElement_IdxPastLen);
   RUN_TEST(test_VectorCpyElement_IdxPastCap);
   RUN_TEST(test_VectorCpyLastElement);
   RUN_TEST(test_VectorRoundTrip_CpyElementToSetElement);

   RUN_TEST(test_VectorSetElement_AfterPushes);
   RUN_TEST(test_VectorSetElement_AfterInitLen);
   RUN_TEST(test_VectorSetElement_PastLen);
   RUN_TEST(test_VectorSetElement_PastCap);

   RUN_TEST(test_VectorRemoveElement_AtZeroWithVectorPartiallyFull);
   RUN_TEST(test_VectorRemoveElement_AtZeroWithSinglePresentElement);
   RUN_TEST(test_VectorRemoveElement_AtZeroWithEmptyVector);
   RUN_TEST(test_VectorRemoveElement_AtMiddle);
   RUN_TEST(test_VectorRemoveElement_AtLen);
   RUN_TEST(test_VectorRemoveElement_LastElement);
   RUN_TEST(test_VectorRemoveElement_PastLen);
   RUN_TEST(test_VectorRemoveElement_AtZeroWithVectorPartiallyFull_WithBuf);
   RUN_TEST(test_VectorRemoveElement_AtZeroWithSinglePresentElement_WithBuf);
   RUN_TEST(test_VectorRemoveElement_AtZeroWithEmptyVector_WithBuf);
   RUN_TEST(test_VectorRemoveElement_AtMiddle_WithBuf);
   RUN_TEST(test_VectorRemoveElement_AtLen_WithBuf);
   RUN_TEST(test_VectorRemoveElement_LastElement_WithBuf);
   RUN_TEST(test_VectorRemoveElement_PastLen_WithBuf);
   RUN_TEST(test_VectorRemoveLastElement_EmptyVec);

   RUN_TEST(test_VectorClearElementAt_Normal);
   RUN_TEST(test_VectorClearElementAt_InvalidIdx);

   RUN_TEST(test_VectorClear_Normal);
   RUN_TEST(test_VectorClear_EmptyVec);
   RUN_TEST(test_VectorClear_InvalidVec);

   RUN_TEST(test_VectorReset_EmptyVec);
   RUN_TEST(test_VectorReset);

   RUN_TEST(test_VectorHardReset);
   RUN_TEST(test_VectorHardReset_EmptyVector);

   RUN_TEST(test_VectorDuplicate_SmallVector);
   RUN_TEST(test_VectorDuplicate_ReallyLargeVector);
   RUN_TEST(test_VectorDuplicate_CatchBadCapDuplication);
   RUN_TEST(test_VectorDuplicate_NullVector);

   RUN_TEST(test_VectorMove_SmallVector);
   RUN_TEST(test_VectorMove_ReallyLargeVector);
   RUN_TEST(test_VectorMove_NullVector);
   RUN_TEST(test_VectorMove_MismatchedVec);

   //RUN_TEST(test_VectorsAreEqual_SameVectors);
   //RUN_TEST(test_VectorsAreEqual_DifferentElementSz);
   //RUN_TEST(test_VectorsAreEqual_DifferentLength);
   //RUN_TEST(test_VectorsAreEqual_DifferentCapacity);
   //RUN_TEST(test_VectorsAreEqual_DifferentMaxCapacity);
   //RUN_TEST(test_VectorsAreEqual_DifferentElementValues);

   //RUN_TEST(test_VectorSplitAt_ValidIdx);
   //RUN_TEST(test_VectorSplitAt_IdxZero);
   //RUN_TEST(test_VectorSplitAt_IdxPastLen);
   //RUN_TEST(test_VectorSplitAt_EmptyVector);
   //RUN_TEST(test_VectorSplitAt_NullVector);
   //RUN_TEST(test_VectorSplitAt_ValidIdx_StructData);

   //RUN_TEST(test_VectorSlice_ValidIndices_IntData);
   //RUN_TEST(test_VectorSlice_ValidIndices_StructData);
   //RUN_TEST(test_VectorSlice_IdxStartEqualsIdxEnd);
   //RUN_TEST(test_VectorSlice_IdxStartZero);
   //RUN_TEST(test_VectorSlice_FullVector);
   //RUN_TEST(test_VectorSlice_IdxEndAtLastElement);
   //RUN_TEST(test_VectorSlice_EmptyVector);
   //RUN_TEST(test_VectorSlice_NullVector);
   //RUN_TEST(test_VectorSlice_IdxStartGreaterThanIdxEnd);
   //RUN_TEST(test_VectorSlice_IdxEndOutOfRange);

   //RUN_TEST(test_VectorConcatenate_BasicUse);
   //RUN_TEST(test_VectorConcatenate_VecsNotMutated);
   //RUN_TEST(test_VectorConcatenate_OneVecIsEmpty);
   //RUN_TEST(test_VectorConcatenate_BothVecsEmpty);
   //RUN_TEST(test_VectorConcatenate_FullVectors);
   //RUN_TEST(test_VectorConcatenate_NullArguments);
   //RUN_TEST(test_VectorConcatenate_DifferentElementSizes);
   //RUN_TEST(test_VectorConcatenate_ConcatenateSplitRoundTrip);

   //RUN_TEST(test_VectorRangePush_ValidInts);
   //RUN_TEST(test_VectorRangePush_ValidStructs);
   //RUN_TEST(test_VectorRangePush_ExpandCapacity);
   //RUN_TEST(test_VectorRangePush_ZeroLen);
   //RUN_TEST(test_VectorRangePush_NullVec);
   //RUN_TEST(test_VectorRangePush_NullData);
   //RUN_TEST(test_VectorRangePush_ExceedsMaxCapacity);
   //RUN_TEST(test_VectorRangePush_ExactlyMaxCapacity);

   //RUN_TEST(test_VectorRangeInsert_ValidInts);
   //RUN_TEST(test_VectorRangeInsert_ValidStructs);
   //RUN_TEST(test_VectorRangeInsert_ExpandCapacity);
   //RUN_TEST(test_VectorRangeInsert_ZeroLen);
   //RUN_TEST(test_VectorRangeInsert_NullVec);
   //RUN_TEST(test_VectorRangeInsert_NullData);
   //RUN_TEST(test_VectorRangeInsert_ExceedsMaxCapacity);
   //RUN_TEST(test_VectorRangeInsert_ExactlyMaxCapacity);
   //RUN_TEST(test_VectorRangeInsert_Push_Equivalence);
   //RUN_TEST(test_VectorRangeInsert_InvalidIdx);
   //RUN_TEST(test_VectorRangeInsert_OneElm);
   //RUN_TEST(test_VectorRangeInsert_AfterHardResetting);

   //RUN_TEST(test_VectorRangeCpy_ValidIdices_IntData);
   //RUN_TEST(test_VectorRangeCpy_DoesNotMutate);
   //RUN_TEST(test_VectorRangeCpy_ValidIndices_StructData);
   //RUN_TEST(test_VectorRangeCpy_FullVector);
   //RUN_TEST(test_VectorRangeCpy_FullVector_IncorrectEndIdx);
   //RUN_TEST(test_VectorRangeCpy_EmptyVec);
   //RUN_TEST(test_VectorRangeCpy_InvalidIdx);
   //RUN_TEST(test_VectorRangeCpy_InvalidVec);
   //RUN_TEST(test_VectorRangeCpy_SameIdices);

   //RUN_TEST(test_VectorRangeCpyToEnd_ValidIdices_IntData);
   //RUN_TEST(test_VectorRangeCpyToEnd_DoesNotMutate);
   //RUN_TEST(test_VectorRangeCpyToEnd_ValidIndices_StructData);
   //RUN_TEST(test_VectorRangeCpyToEnd_FullVector);
   //RUN_TEST(test_VectorRangeCpyToEnd_EmptyVec);
   //RUN_TEST(test_VectorRangeCpyToEnd_InvalidIdx);
   //RUN_TEST(test_VectorRangeCpyToEnd_InvalidVec);
   //RUN_TEST(test_VectorRangeCpyToEnd_EndIdx);

   //RUN_TEST(test_VectorRangeSetWithArr_ValidIdices_IntData);
   //RUN_TEST(test_VectorRangeSetWithArr_DoesNotMutate);
   //RUN_TEST(test_VectorRangeSetWithArr_ValidIndices_StructData);
   //RUN_TEST(test_VectorRangeSetWithArr_FullVector);
   //RUN_TEST(test_VectorRangeSetWithArr_FullVector_IncorrectEndIdx);
   //RUN_TEST(test_VectorRangeSetWithArr_EmptyVec);
   //RUN_TEST(test_VectorRangeSetWithArr_InvalidIdx);
   //RUN_TEST(test_VectorRangeSetWithArr_InvalidVec);
   //RUN_TEST(test_VectorRangeSetWithArr_SameIdices);
   //RUN_TEST(test_VectorRange_RoundTrip_CpyElementsToSetElementsInRng);

   //RUN_TEST(test_VectorRangeSetToVal_ValidIdices_IntData);
   //RUN_TEST(test_VectorRangeSetToVal_ValidIndices_StructData);
   //RUN_TEST(test_VectorRangeSetToVal_FullVector);
   //RUN_TEST(test_VectorRangeSetToVal_FullVector_IncorrectEndIdx);
   //RUN_TEST(test_VectorRangeSetToVal_EmptyVec);
   //RUN_TEST(test_VectorRangeSetToVal_InvalidIdx);
   //RUN_TEST(test_VectorRangeSetToVal_InvalidVec);
   //RUN_TEST(test_VectorRangeSetToVal_SameIdices);
   //RUN_TEST(test_VectorRangeSetToVal_NullValue);
   //RUN_TEST(test_VectorRangeSetToVal_SingleElement);
   //RUN_TEST(test_VectorRangeSetToVal_AtBeginning);
   //RUN_TEST(test_VectorRangeSetToVal_AtEnd);
   //RUN_TEST(test_VectorRangeSetToVal_ZeroValue);
   //RUN_TEST(test_VectorRangeSetToVal_EquivalenceWithRangeClear);

   //RUN_TEST(test_VectorRange_RemoveElementsInRng_Normal);
   //RUN_TEST(test_VectorRange_RemoveElementsInRng_NormalWithBuf);
   //RUN_TEST(test_VectorRange_RemoveElementsInRng_AllElements);
   //RUN_TEST(test_VectorRange_RemoveElementsInRng_AtBeginning);
   //RUN_TEST(test_VectorRange_RemoveElementsInRng_AtEnd);
   //RUN_TEST(test_VectorRange_RemoveElementsInRng_EmptyVec);
   //RUN_TEST(test_VectorRange_RemoveElementsInRng_InvalidIndices);
   //RUN_TEST(test_VectorRange_RemoveElementsInRng_InvalidVec);
   //RUN_TEST(test_VectorRange_RoundTrip_InsertAndRemove);
   //RUN_TEST(test_VectorRangeRemove_OneElm);

   //RUN_TEST(test_VectorRange_ClearElementsInRng_Normal);
   //RUN_TEST(test_VectorRange_ClearElementsInRng_AllElements);
   //RUN_TEST(test_VectorRange_ClearElementsInRng_EmptyVec);
   //RUN_TEST(test_VectorRange_ClearElementsInRng_AtBeginning);
   //RUN_TEST(test_VectorRange_ClearElementsInRng_AtEnd);
   //RUN_TEST(test_VectorRange_ClearElementsInRng_InvalidIndices);
   //RUN_TEST(test_VectorRange_ClearElementsInRng_InvalidVec);

   //RUN_TEST(test_VIterator_BasicRead_FullVec);
   //RUN_TEST(test_VIterator_BasicUpdate_FullVec);
   //RUN_TEST(test_VIterator_BasicRead_FullVec_Reverse);
   //RUN_TEST(test_VIterator_BasicUpdate_FullVec_Reverse);
   //RUN_TEST(test_VIterator_BasicRead_SubRng_Normal);
   //RUN_TEST(test_VIterator_BasicUpdate_SubRng_Normal);
   //RUN_TEST(test_VIterator_BasicRead_SubRng_Reverse);
   //RUN_TEST(test_VIterator_BasicUpdate_SubRng_Reverse);
   //RUN_TEST(test_VIterator_BasicRead_SubRng_NormalWithWrap);
   //RUN_TEST(test_VIterator_BasicUpdate_SubRng_NormalWithWrap);
   //RUN_TEST(test_VIterator_BasicRead_SubRng_ReverseWithWrap);
   //RUN_TEST(test_VIterator_BasicUpdate_SubRng_ReverseWithWrap);

   return UNITY_END();
}

/********************************* Test Setup *********************************/

void setUp(void)
{
   UnityMalloc_StartTest();
}

void tearDown(void)
{
   UnityMalloc_EndTest();
}

/************************ Vector Initialization Tests *************************/
void test_VectorNew_Invalid_ZeroElementSz(void)
{
   struct Vector * vec;
   vec = VectorNew(0, 20, 50, NULL, 0, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_NULL( vec );
   VectorFree(vec);
}

void test_VectorNew_Invalid_MaxCapLessThanInitCap(void)
{
   struct Vector * vec;
   vec = VectorNew(0, 50, 20, NULL, 0, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_NULL( vec );
   VectorFree(vec);
}

void test_VectorNew_Invalid_ZeroMaxCap(void)
{
   // This is invalid because max capacity will not be mutable,
   // and a max capacity of 0 is useless. We won't allow it.
   struct Vector * vec;
   vec = VectorNew(10, 0, 0, NULL, 0, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_NULL( vec );
   VectorFree(vec);
}

void test_VectorNew_Invalid_InitialLen(void)
{
   struct Vector * vec;
   int init_data[11] = {0}; // 11 elements initialized to 0
   vec = VectorNew( sizeof(int), 10, 100, init_data, 1100, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_EQUAL_size_t(0, VectorLength(vec));
   VectorFree(vec);
}

void test_VectorNew_ValidInputCombo_3DPoints(void)
{
   const size_t InitialCaps[] = { 0, 2, 1000 };
   const size_t MaxCap = 10000;
   struct MyData_S { int x; int y; int z; };
   struct Vector * vec;
   // FIXME: Have to assume that malloc will succeed at least once for this test to mean anything...
   for ( size_t i = 0; i < ARR_LEN(InitialCaps); i++ )
   {
      vec = VectorNew(sizeof(struct MyData_S), InitialCaps[i], MaxCap, NULL, 0, &DEFAULT_ALLOCATOR);

      if ( vec != NULL )
      {
         // Confirm correct initialization
         TEST_ASSERT_EQUAL_size_t( sizeof(struct MyData_S), VectorElementSize(vec) );
         TEST_ASSERT_EQUAL_size_t( InitialCaps[i], VectorCapacity(vec) );
         TEST_ASSERT_EQUAL_size_t( MaxCap, VectorMaxCapacity(vec) );
      }

      VectorFree(vec);
   }
}

void test_VectorNew_CapacityLimit(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN(vec, 1, UINT32_MAX, UINT32_MAX, NULL, 0, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_EQUAL_size_t( 1, VectorElementSize(vec) );
   TEST_ASSERT_EQUAL_size_t( UINT32_MAX, VectorCapacity(vec) );
   TEST_ASSERT_EQUAL_size_t( UINT32_MAX, VectorMaxCapacity(vec) );
   VectorFree(vec);
}

void test_VectorNew_ElementSzLimit(void)
{
   // Now push things to the limit and try to create a vector at the system's limit.
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN(vec, UINT32_MAX, 1, 1, NULL, 0, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_EQUAL_size_t( UINT32_MAX, VectorElementSize(vec) );
   TEST_ASSERT_EQUAL_size_t( 1, VectorCapacity(vec) );
   TEST_ASSERT_EQUAL_size_t( 1, VectorMaxCapacity(vec) );
   VectorFree(vec);
}

void test_VectorNew_InitData(void)
{
   struct Vector * v = VectorNew(sizeof(int), 5, 10, (int[]){1, 2, 3}, 3, NULL);
   TEST_ASSERT_EQUAL_INT(1, *(int *)VectorGet(v,0));
   TEST_ASSERT_EQUAL_INT(2, *(int *)VectorGet(v,1));
   TEST_ASSERT_EQUAL_INT(3, *(int *)VectorGet(v,2));
   VectorFree(v);
}

void test_VectorNew_InitialLenLessThanInitialCap(void)
{
   struct Vector * vec = NULL;
   const size_t INIT_LEN = 5;
   unsigned char init_data[50 * 5] = {0}; // Zero-initialized array for INIT_LEN elements of size 50
   VECTOR_NEW_KEEP_TRYIN(vec, 50, INIT_LEN * 2, INIT_LEN * 10, init_data, INIT_LEN, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_EQUAL_size_t( INIT_LEN,     VectorLength(vec) );
   TEST_ASSERT_EQUAL_size_t( INIT_LEN * 2, VectorCapacity(vec) );
   // Verify initial elements have been zero'd out
   for ( size_t i = 0; i < INIT_LEN; i++ )
   {
      TEST_ASSERT_EQUAL_MEMORY(&init_data[i], VectorGet(vec,i), 50);
   }
   VectorFree(vec);
}

void test_VectorNew_InitialLenSameAsInitialCap(void)
{
   struct Vector * vec = NULL;
   const size_t INIT_LEN = 10;
   unsigned char init_data[50 * 10] = {0}; // Zero-initialized array for INIT_LEN elements of size 50
   VECTOR_NEW_KEEP_TRYIN(vec, 50, INIT_LEN, INIT_LEN * 10, init_data, INIT_LEN, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_EQUAL_size_t( INIT_LEN, VectorLength(vec) );
   TEST_ASSERT_EQUAL_size_t( INIT_LEN, VectorCapacity(vec) );
   // Verify initial elements have been zero'd out
   for ( size_t i = 0; i < INIT_LEN; i++ )
   {
      TEST_ASSERT_EQUAL_MEMORY(&init_data[i], VectorGet(vec,i), 50);
   }
   VectorFree(vec);
}

/**************************** Vector Ops On Nulls *****************************/
void test_VectorOpsOnNullVectors(void)
{
   // Call any API that takes in a pointer, and ensure appropriate behavior, or
   // that the application does not crash.
   VectorFree(NULL);
   TEST_ASSERT_EQUAL_size_t(0, VectorLength(NULL));
   TEST_ASSERT_EQUAL_size_t(0, VectorCapacity(NULL));
   TEST_ASSERT_EQUAL_size_t(0, VectorMaxCapacity(NULL));
   TEST_ASSERT_EQUAL_size_t(0, VectorElementSize(NULL));
   TEST_ASSERT_TRUE(VectorIsEmpty(NULL));
   TEST_ASSERT_FALSE(VectorIsFull(NULL));
   TEST_ASSERT_FALSE(VectorPush(NULL, NULL));
   TEST_ASSERT_FALSE(VectorInsert(NULL, 0, NULL));
   TEST_ASSERT_FALSE(VectorInsert(NULL, UINT32_MAX, NULL));
   TEST_ASSERT_NULL(VectorGet(NULL, 0));
   TEST_ASSERT_NULL(VectorGet(NULL, UINT32_MAX));
   TEST_ASSERT_FALSE(VectorCpyElementAt(NULL, 0, NULL));
   TEST_ASSERT_FALSE(VectorCpyElementAt(NULL, UINT32_MAX, NULL));
   TEST_ASSERT_FALSE(VectorSet(NULL, 0, NULL));
   TEST_ASSERT_FALSE(VectorSet(NULL, UINT32_MAX, NULL));
   TEST_ASSERT_FALSE(VectorRemove(NULL, 0, NULL));
   TEST_ASSERT_FALSE(VectorRemove(NULL, UINT32_MAX, NULL));
   TEST_ASSERT_NULL(VectorLastElement(NULL));
   TEST_ASSERT_FALSE(VectorCpyLastElement(NULL, NULL));
   TEST_ASSERT_FALSE(VectorClearElementAt(NULL, 0));
   TEST_ASSERT_FALSE(VectorReset(NULL));
   TEST_ASSERT_FALSE(VectorHardReset(NULL));
   TEST_ASSERT_NULL(VectorDuplicate(NULL));
   TEST_ASSERT_FALSE(VectorsAreEqual(NULL, NULL));
   TEST_ASSERT_FALSE(VectorsAreEqual(NULL, (struct Vector *)1));
   TEST_ASSERT_FALSE(VectorsAreEqual((struct Vector *)1, NULL));
   TEST_ASSERT_FALSE(VectorRemoveLastElement(NULL, NULL));
   // FIXME: Finish this up with the rest of the API
}

/***************************** Simple Vector Ops ******************************/
void test_VectorFree(void)
{
   struct Vector * vec = NULL;

   VECTOR_NEW_KEEP_TRYIN(vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   // FIXME: Need to assert post-conditions of a vector free operation...
   VectorFree(vec);
}

void test_VectorLength(void) {
    struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
    TEST_ASSERT_EQUAL_size_t(0, VectorLength(vec));
    VectorPush(vec, &(int){42});
    TEST_ASSERT_EQUAL_size_t(1, VectorLength(vec));
    VectorFree(vec);
}

void test_VectorCapacity(void)
{
    struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
    TEST_ASSERT_EQUAL_size_t(10, VectorCapacity(vec));
    VectorFree(vec);
}

void test_VectorMaxCapacity(void) {
    struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
    TEST_ASSERT_EQUAL_size_t(100, VectorMaxCapacity(vec));
    VectorFree(vec);
}

void test_VectorElementSize(void) {
    struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
    TEST_ASSERT_EQUAL_size_t(sizeof(int), VectorElementSize(vec));
    VectorFree(vec);
}

void test_VectorIsEmpty(void)
{
    struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
    TEST_ASSERT_TRUE(VectorIsEmpty(vec));
    VectorPush(vec, &(int){42});
    TEST_ASSERT_FALSE(VectorIsEmpty(vec));
    VectorFree(vec);
}

void test_VectorIsFull(void)
{
   // Initialize a vector with a small capacity
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN(vec, sizeof(int), 3, 3, NULL, 0, &DEFAULT_ALLOCATOR);

   // Verify the vector is not full initially
   TEST_ASSERT_FALSE(VectorIsFull(vec));

   // Add elements to the vector
   int value = 42;
   VectorPush(vec, &value);
   TEST_ASSERT_FALSE(VectorIsFull(vec)); // Still not full after one element

   VectorPush(vec, &value);
   TEST_ASSERT_FALSE(VectorIsFull(vec)); // Still not full after two elements

   VectorPush(vec, &value);
   TEST_ASSERT_TRUE(VectorIsFull(vec)); // Should be full after three elements

   // Attempt to push another element (should fail if capacity is enforced)
   VectorPush(vec, &value);
   TEST_ASSERT_TRUE(VectorIsFull(vec)); // Still full after failed push

   VectorFree(vec);
}

/******************************** Vector Push *********************************/
void test_VectorPush_SimplePush(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN(vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int value1 = 42;
   int value2 = 84;

   // Now try pushing...
   // There should be no issues pushing because the initial capacity
   // is greater than 2.
   // Push the first value and verify
   TEST_ASSERT_TRUE(VectorPush(vec, &value1));
   TEST_ASSERT_EQUAL_size_t(1, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(value1, *(int *)VectorGet(vec, 0));

   // Push the second value and verify
   TEST_ASSERT_TRUE(VectorPush(vec, &value2));
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(value2, *(int *)VectorGet(vec, 1));

   // Ensure the vector is not empty
   TEST_ASSERT_FALSE(VectorIsEmpty(vec));

   VectorFree(vec);
}

void test_VectorPush_UntilCapacity(void)
{
   struct Vector * vec = NULL;
   struct MyData_S
   {
      float x;
      float y;
      float z;
   };
   const size_t MAX_CAP = (size_t)(1.0e6);
   const size_t INIT_CAP = MAX_CAP / 1000;

   VECTOR_NEW_KEEP_TRYIN( vec, sizeof(struct MyData_S), INIT_CAP, MAX_CAP, NULL, 0, &DEFAULT_ALLOCATOR);

   // Now push until you've reached the initial capacity, and confirm along the
   // way that the element was truly pushed in...
   struct MyData_S test_element = { .x = 0.0f, .y = FLT_MAX, .z = -FLT_MIN };
   struct MyData_S * last_element;
   size_t vec_len = 0;
   srand( (unsigned int) time(NULL) );
   while ( VectorLength(vec) < INIT_CAP )
   {
      // Random values each time so that we don't falsely believe we've added
      // an element because every lastElement() is the same.
      test_element.x = ((float)rand() / (float)RAND_MAX) * FLT_MAX;
      test_element.y = ((float)rand() / (float)RAND_MAX) * FLT_MAX;
      test_element.z = ((float)rand() / (float)RAND_MAX) * FLT_MAX;
      bool push_successfull = VectorPush( vec, &test_element );
      if ( push_successfull )
      {
         vec_len++;
         last_element = VectorLastElement(vec);
         TEST_ASSERT_EQUAL_FLOAT( test_element.x, last_element->x );
         TEST_ASSERT_EQUAL_FLOAT( test_element.y, last_element->y );
         TEST_ASSERT_EQUAL_FLOAT( test_element.z, last_element->z );
         TEST_ASSERT_EQUAL_size_t( vec_len, VectorLength(vec) );
      }
   }
   // We should have hit the initial capacity but not any more than that
   TEST_ASSERT_EQUAL_size_t( INIT_CAP, VectorLength(vec) );
   TEST_ASSERT_EQUAL_size_t( INIT_CAP, VectorCapacity(vec) );
   // If every single push failed, something's off...
   TEST_ASSERT_FALSE( VectorIsEmpty(vec) );

   // Now try pushing past the capacity
   while ( VectorLength(vec) <= INIT_CAP )
   {
      // Random values each time so that we don't falsely believe we've added
      // an element because every lastElement() is the same.
      test_element.x = ((float)rand() / (float)RAND_MAX) * FLT_MAX;
      test_element.y = ((float)rand() / (float)RAND_MAX) * FLT_MAX;
      test_element.z = ((float)rand() / (float)RAND_MAX) * FLT_MAX;
      bool push_successfull = VectorPush( vec, &test_element );
      if ( push_successfull )
      {
         vec_len++;
         last_element = VectorLastElement(vec);
         TEST_ASSERT_EQUAL_FLOAT( test_element.x, last_element->x );
         TEST_ASSERT_EQUAL_FLOAT( test_element.y, last_element->y );
         TEST_ASSERT_EQUAL_FLOAT( test_element.z, last_element->z );
         TEST_ASSERT_EQUAL_size_t( vec_len, VectorLength(vec) );
      }
   }

   // We should have gotten past the initial capacity and grown
   TEST_ASSERT_GREATER_THAN_UINT32( INIT_CAP, VectorLength(vec) );
   TEST_ASSERT_GREATER_THAN_UINT32( INIT_CAP, VectorCapacity(vec) );

   VectorFree(vec);
}

void test_VectorPush_PastInitialCapacity(void)
{
   struct Vector * vec = NULL;
   struct MyData_S
   {
      float x;
      float y;
      float z;
   };
   const size_t MAX_CAP = (size_t)(1.0e6);
   const size_t INIT_CAP = MAX_CAP / 1000;

   VECTOR_NEW_KEEP_TRYIN( vec, sizeof(struct MyData_S), 100, MAX_CAP, NULL, 0, &DEFAULT_ALLOCATOR );

   // Fill to initial capacity
   struct MyData_S test_element = { .x = 0.0f, .y = FLT_MAX, .z = -FLT_MIN };
   struct MyData_S * last_element;
   size_t vec_len = 0;
   while ( VectorLength(vec) < INIT_CAP )
   {
      bool push_successfull = VectorPush( vec, &test_element );
      vec_len = push_successfull ? (vec_len + 1) : vec_len;
   }

   // Now try pushing past the capacity
   // Change the test element to distinguish it from the previously added elmnts
   test_element.x = 10.5f;
   test_element.y = -10.5;
   test_element.z = 105.0f;
   while ( VectorLength(vec) <= INIT_CAP )
   {
      bool push_successfull = VectorPush( vec, &test_element );
      vec_len = push_successfull ? (vec_len + 1) : vec_len;
   }
   last_element = VectorLastElement(vec);
   TEST_ASSERT_EQUAL_FLOAT( test_element.x, last_element->x );
   TEST_ASSERT_EQUAL_FLOAT( test_element.y, last_element->y );
   TEST_ASSERT_EQUAL_FLOAT( test_element.z, last_element->z );

   VectorFree(vec);
}

void test_VectorPush_PastMaxCapacity(void)
{
   struct Vector * vec = NULL;
   struct MyData_S
   {
      float x;
      float y;
      float z;
   };
   const size_t MAX_CAP = (size_t)(10.0e3);
   const size_t INIT_CAP = MAX_CAP / 1000;

   VECTOR_NEW_KEEP_TRYIN( vec,  sizeof(struct MyData_S), INIT_CAP, MAX_CAP, NULL, 0, &DEFAULT_ALLOCATOR );

   // Fill to initial capacity
   struct MyData_S test_element = { .x = 0.0f, .y = FLT_MAX, .z = -FLT_MIN };
   struct MyData_S * last_element;
   size_t vec_len = 0;
   while ( VectorLength(vec) < MAX_CAP )
   {
      bool push_successfull = VectorPush( vec, &test_element );
      vec_len = push_successfull ? (vec_len + 1) : vec_len;
   }

   // Now try pushing past the max capacity many times
   // Change the test element to distinguish it from the previously added elmnts
   struct MyData_S new_test_element = { .x = 10.5f, .y = -10.5f, .z = 105.0f };
   for ( uint16_t i = 0; i < 1000; i++ )
   {
      bool push_successfull = VectorPush( vec, &new_test_element );
      last_element = VectorLastElement(vec);
      TEST_ASSERT_FALSE(push_successfull);
      // Last element is still the original data
      TEST_ASSERT_EQUAL_FLOAT( test_element.x, last_element->x );
      TEST_ASSERT_EQUAL_FLOAT( test_element.y, last_element->y );
      TEST_ASSERT_EQUAL_FLOAT( test_element.z, last_element->z );
   }

   VectorFree(vec);
}

void test_VectorPush_IntoVecWithZeroMaxCap(void)
{
   struct Vector * vec;
   struct MyData_S
   {
      char str[20];
      unsigned long long int id;
   } test_element =
   {
      .str = "Test Element",
      .id  = 0
   };
   vec = VectorNew( sizeof(struct MyData_S), 0, 0, NULL, 0, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_FALSE( VectorPush( vec, &test_element ) );
   TEST_ASSERT_TRUE( VectorIsEmpty(vec) );
}

void test_VectorPush_InitialCapOfZero(void)
{
   // FIXME: Assumes no mallocs fail
   struct Vector * vec = VectorNew( sizeof(int), 0, 10, NULL, 0, NULL );
   TEST_ASSERT_TRUE( VectorPush(vec, &(int){5}) );
   TEST_ASSERT_EQUAL_INT( *((int *)VectorLastElement(vec)), 5 );
   VectorFree(vec);
}

void test_VectorPush_AfterResetting(void)
{
   // FIXME: Assumes no mallocs fail
   struct Vector * vec = VectorNew( sizeof(int), 5, 10, NULL, 0, NULL );
   VectorPush(vec, &(int){5});
   VectorReset(vec);
   TEST_ASSERT_TRUE( VectorPush(vec, &(int){10}) );
   TEST_ASSERT_EQUAL_INT( *((int *)VectorLastElement(vec)), 10 );
   VectorFree(vec);
}

void test_VectorPush_AfterHardResetting(void)
{
   // FIXME: Assumes no mallocs fail
   struct Vector * vec = VectorNew( sizeof(int), 5, 10000, NULL, 0, NULL );
   VectorPush(vec, &(int){5});
   VectorHardReset(vec);
   TEST_ASSERT_TRUE( VectorPush(vec, &(int){5}) );
   TEST_ASSERT_EQUAL_INT( *((int *)VectorLastElement(vec)), 5 );
   // Check that expansion was done correctly after 0 capacity by pushing
   // many elements. If memory was not allocated properly, should crash.
   for ( size_t i = 0; i < 10000; i++ )
   {
      VectorPush(vec, &(int){5});
   }
   VectorFree(vec);
}

/****************************** Vector Insertion ******************************/
void test_VectorInsertion_AtZeroWithVectorLessThanCapacity(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN( vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR );

   int val = 20;
   while ( VectorLength(vec) < 5 )
   {
      (void)VectorPush(vec, &val);
      val++;
   }

   // Do an insertion at the beginning. I expect everything downstream
   // to have been shifted over as well.
   int test_val = 100;
   int * result;
   while( !VectorInsert(vec, 0, &test_val) ); // Ensure insertion success
   result = (int *)VectorGet(vec, 0);
   TEST_ASSERT_EQUAL_INT(test_val, *result);
   TEST_ASSERT_EQUAL_size_t(6, VectorLength(vec));
   for ( size_t i = 1; i < 6; i++ )
   {
      result = VectorGet(vec, i);
      TEST_ASSERT_NOT_NULL(result);
      TEST_ASSERT_EQUAL_INT( 20 + (i-1), *result );
   }

   // Cleanup
   VectorFree(vec);
}

void test_VectorInsertion_AtZeroWithVectorAtCapacity(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN( vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR );

   int val = 20;
   while ( VectorLength(vec) < 10 )
   {
      (void)VectorPush(vec, &val);
      val++;
   }

   // Do an insertion at the beginning. I expect everything downstream
   // to have been shifted over as well.
   int test_val = 100;
   int * result;
   while( !VectorInsert(vec, 0, &test_val) ); // Ensure insertion success
   result = (int *)VectorGet(vec, 0);
   TEST_ASSERT_EQUAL_INT(test_val, *result);
   TEST_ASSERT_EQUAL_size_t(11, VectorLength(vec));
   for ( size_t i = 0; i < 10; i++ )
   {
      TEST_ASSERT_EQUAL_INT( 20 + i, *( (int *)VectorGet(vec, i + 1) ) );
   }

   // Cleanup
   VectorFree(vec);
}

void test_VectorInsertion_AtZeroWithVectorAtMaxCapacity(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN( vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR );

   int val = 20;
   while ( VectorLength(vec) < 100 )
   {
      (void)VectorPush(vec, &val);
      val++;
   }

   // Try to do an insertion many times. This should fail.
   // Also, elements in vector should basically remain intact.
   int test_val = 100;
   for ( size_t i = 0; i < 100; i++ )
   {
      TEST_ASSERT_FALSE( VectorInsert(vec, 0, &test_val) );
   }
   for ( size_t i = 0; i < 100; i++ )
   {
      TEST_ASSERT_EQUAL_INT( 20 + i, *( (int *)VectorGet(vec, i) ) );
   }

   VectorFree(vec);
}

void test_VectorInsertion_AtEndEqualsVecPush(void)
{
   struct Vector * vec1 = NULL;
   struct Vector * vec2 = NULL;
   VECTOR_NEW_KEEP_TRYIN( vec1, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR );
   VECTOR_NEW_KEEP_TRYIN( vec2, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR );

   int val = 20;
   size_t iter = 0;
   while ( (VectorLength(vec1) < 100) && (iter < 1000) )
   {
      bool result = VectorPush(vec1, &val);
      if ( result) val++;
      iter++;
   }
   val = 20;
   size_t idx = 0;
   iter = 0;
   while ( (VectorLength(vec2) < 100) && (iter < 1000) )
   {
      bool result = VectorInsert(vec2, idx, &val);
      if ( result )
      {
         val++;
         idx++;
      }
      iter++;
   }
   // It is possible that the 1000 iteration attempts were not enough to insert
   // the 100 elements depending on how much reallocations failed. Feels unlikely
   // but Murphy, ya know.

   TEST_ASSERT_TRUE( VectorsAreEqual(vec1, vec2) );

   VectorFree(vec1);
   VectorFree(vec2);
}

void test_VectorInsertion_AtMiddle(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN( vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR );

   int val = 20;
   const size_t IDX_OF_INSERTION = 4;
   while ( VectorLength(vec) < (IDX_OF_INSERTION *2) )
   {
      (void)VectorPush(vec, &val);
   }

   // Do an insertion at the beginning. I expect everything downstream
   // to have been shifted over as well.
   int test_val = 100;
   int * result;
   while( !VectorInsert(vec, IDX_OF_INSERTION, &test_val) ); // Ensure insertion success
   result = (int *)VectorGet(vec, IDX_OF_INSERTION);
   TEST_ASSERT_EQUAL_INT(test_val, *result);
   TEST_ASSERT_EQUAL_size_t(2*IDX_OF_INSERTION + 1, VectorLength(vec));
   for ( size_t i = 1; i < VectorLength(vec); i++ )
   {
      if ( i == IDX_OF_INSERTION )  continue;

      result = VectorGet(vec, i);
      TEST_ASSERT_NOT_NULL(result);
      TEST_ASSERT_EQUAL_INT( val, *result );
   }

   // Cleanup
   VectorFree(vec);
}

void test_VectorInsertion_AtMiddleOfEmptyVec(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN( vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR );

   int val = 20;
   const size_t IDX_OF_INSERTION = 4;
   while ( VectorLength(vec) < (IDX_OF_INSERTION *2) )
   {
      (void)VectorPush(vec, &val);
   }

   // Do an insertion at the beginning. I expect everything downstream
   // to have been shifted over as well.
   int test_val = 100;
   int * result;
   while( !VectorInsert(vec, IDX_OF_INSERTION, &test_val) ); // Ensure insertion success
   result = (int *)VectorGet(vec, IDX_OF_INSERTION);
   TEST_ASSERT_EQUAL_INT(test_val, *result);
   TEST_ASSERT_EQUAL_size_t(2*IDX_OF_INSERTION + 1, VectorLength(vec));
   for ( size_t i = 1; i < VectorLength(vec); i++ )
   {
      if ( i == IDX_OF_INSERTION )  continue;

      result = VectorGet(vec, i);
      TEST_ASSERT_NOT_NULL(result);
      TEST_ASSERT_EQUAL_INT( val, *result );
   }

   // Cleanup
   VectorFree(vec);
}

/******************************** Vector Gets *********************************/
void test_VectorGetElement_ValidIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   VectorPush(vec, &(int){42});
   int * retrieved = (int *)VectorGet(vec, 0);
   TEST_ASSERT_NOT_NULL(retrieved);
   TEST_ASSERT_EQUAL_INT(42, *retrieved);
   VectorFree(vec);
}

void test_VectorGetElement_IdxPastLen(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42;
   VectorPush(vec, &value);
   int * retrieved = (int *)VectorGet(vec, 1);
   TEST_ASSERT_NULL(retrieved);
   VectorFree(vec);
}

void test_VectorGetElement_IdxPastCap(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42;
   VectorPush(vec, &value);
   int * retrieved = (int *)VectorGet(vec, 10000000);
   TEST_ASSERT_NULL(retrieved);
   VectorFree(vec);
}

void test_VectorLastElement(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42;
   VectorPush(vec, &value);
   int * last = (int *)VectorLastElement(vec);
   TEST_ASSERT_NOT_NULL(last);
   TEST_ASSERT_EQUAL_INT(42, *last);
   VectorFree(vec);
}

/**************************** Vector Copy Element *****************************/
void test_VectorCpyElement_ValidIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42;
   int buffer;
   VectorPush(vec, &value);
   TEST_ASSERT_TRUE(VectorCpyElementAt(vec, 0, &buffer));
   TEST_ASSERT_EQUAL_INT(value, buffer);
   VectorFree(vec);
}

void test_VectorCpyElement_NullBufferPassedIn(void)
{
   // Easy mistake to make for the user.
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42;
   int * buffer = NULL;
   VectorPush(vec, &value);
   TEST_ASSERT_FALSE(VectorCpyElementAt(vec, 0, buffer));
   VectorFree(vec);
}

void test_VectorCpyElement_IdxPastLen(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42;
   int buffer = 0;
   VectorPush(vec, &value);
   TEST_ASSERT_FALSE(VectorCpyElementAt(vec, 1, &buffer));
   TEST_ASSERT_EQUAL_INT(0, buffer);   // Confirm buffer unchanged
   VectorFree(vec);
}

void test_VectorCpyElement_IdxPastCap(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42;
   int buffer = 0;
   VectorPush(vec, &value);
   TEST_ASSERT_FALSE(VectorCpyElementAt(vec, 1000000000, &buffer));
   TEST_ASSERT_EQUAL_INT(0, buffer);   // Confirm buffer unchanged
   VectorFree(vec);
}

void test_VectorCpyLastElement(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42, buffer;
   VectorPush(vec, &value);
   TEST_ASSERT_TRUE(VectorCpyLastElement(vec, &buffer));
   TEST_ASSERT_NOT_EQUAL_size_t( &buffer, VectorGet(vec, 0) );
   TEST_ASSERT_EQUAL_INT(42, buffer);
   VectorFree(vec);
}

void test_VectorRoundTrip_CpyElementToSetElement(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42;
   int buffer;
   VectorPush(vec, &value);
   VectorCpyElementAt(vec, 0, &buffer);
   buffer++;
   TEST_ASSERT_TRUE(VectorSet(vec, 0, &buffer));
   buffer = *( (int *)VectorGet(vec, 0) );
   TEST_ASSERT_EQUAL_INT(value + 1, buffer);
   VectorFree(vec);
}

/******************************** Vector Sets *********************************/
void test_VectorSetElement_AfterPushes(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN(vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value1 = 42, value2 = 84;
   VectorPush(vec, &value1);
   TEST_ASSERT_TRUE(VectorSet(vec, 0, &value2));
   TEST_ASSERT_EQUAL_INT(value2, *(int *)VectorGet(vec, 0));
   VectorFree(vec);
}

void test_VectorSetElement_AfterInitLen(void)
{
   struct Vector * vec = NULL;
   int init_data[5] = {0}; // Zero-initialized array with 5 elements
   VECTOR_NEW_KEEP_TRYIN(vec, sizeof(int), 10, 100, init_data, 5, &DEFAULT_ALLOCATOR);
   int val = 5;
   TEST_ASSERT_TRUE(VectorSet(vec, 0, &val));
   TEST_ASSERT_TRUE(VectorSet(vec, 3, &val));
   TEST_ASSERT_EQUAL_INT(val, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(val, *(int *)VectorGet(vec, 3));
   VectorFree(vec);
}
void test_VectorSetElement_PastLen(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN(vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value1 = 42, value2 = 84;
   while ( VectorLength(vec) < 10 )
   {
      VectorPush(vec, &value1);
   }
   TEST_ASSERT_FALSE(VectorSet(vec, 10, &value2));
   // Vector unaffected
   for ( size_t i = 0; i < 10; i++ )
   {
      TEST_ASSERT_EQUAL_INT(value1, *(int *)VectorGet(vec, i));
   }
   VectorFree(vec);
}

void test_VectorSetElement_PastCap(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN(vec, sizeof(int), 10, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   int value1 = 42, value2 = 84;
   while ( VectorLength(vec) < 10 )
   {
      VectorPush(vec, &value1);
   }
   TEST_ASSERT_FALSE(VectorSet(vec, 10, &value2));
   // Vector unaffected
   for ( size_t i = 0; i < 10; i++ )
   {
      TEST_ASSERT_EQUAL_INT(value1, *(int *)VectorGet(vec, i));
   }
   VectorFree(vec);
}

/****************************** Vector Removals *******************************/
void test_VectorRemoveElement_AtZeroWithVectorPartiallyFull(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   TEST_ASSERT_TRUE(VectorRemove(vec, 0, NULL));
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(values[1], *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(values[2], *(int *)VectorGet(vec, 1));

   VectorFree(vec);
}

void test_VectorRemoveElement_AtZeroWithSinglePresentElement(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42;
   VectorPush(vec, &value);

   TEST_ASSERT_TRUE(VectorRemove(vec, 0, NULL));
   TEST_ASSERT_TRUE(VectorIsEmpty(vec));

   VectorFree(vec);
}

void test_VectorRemoveElement_AtZeroWithEmptyVector(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   TEST_ASSERT_FALSE(VectorRemove(vec, 0, NULL));
   TEST_ASSERT_TRUE(VectorIsEmpty(vec));

   VectorFree(vec);
}

void test_VectorRemoveElement_AtMiddle(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   TEST_ASSERT_TRUE(VectorRemove(vec, 1, NULL));
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(values[0], *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(values[2], *(int *)VectorGet(vec, 1));

   VectorFree(vec);
}

void test_VectorRemoveElement_AtLen(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   TEST_ASSERT_FALSE(VectorRemove(vec, VectorLength(vec), NULL));
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(vec));

   VectorFree(vec);
}

void test_VectorRemoveElement_LastElement(void)
{
   struct Vector * vec1 = NULL;
   struct Vector * vec2 = NULL;
   VECTOR_NEW_KEEP_TRYIN(vec1, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   VECTOR_NEW_KEEP_TRYIN(vec2, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++)
   {
      VectorPush(vec1, &values[i]);
      VectorPush(vec2, &values[i]);
   }

   TEST_ASSERT_TRUE(VectorRemove(vec1, 2, NULL));
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec1));
   TEST_ASSERT_EQUAL_INT(values[0], *(int *)VectorGet(vec1, 0));
   TEST_ASSERT_EQUAL_INT(values[1], *(int *)VectorGet(vec1, 1));

   // Should be the same result as VectorRemoveLastElement
   TEST_ASSERT_TRUE(VectorRemoveLastElement(vec2, NULL));
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec2));
   TEST_ASSERT_EQUAL_INT(values[0], *(int *)VectorGet(vec2, 0));
   TEST_ASSERT_EQUAL_INT(values[1], *(int *)VectorGet(vec2, 1));

   VectorFree(vec1);
   VectorFree(vec2);
}

void test_VectorRemoveElement_PastLen(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   TEST_ASSERT_FALSE(VectorRemove(vec, 5, NULL));
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(vec));

   VectorFree(vec);
}

void test_VectorRemoveElement_AtZeroWithVectorPartiallyFull_WithBuf(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer;
   TEST_ASSERT_TRUE(VectorRemove(vec, 0, &buffer));
   TEST_ASSERT_EQUAL_INT(42, buffer);
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(values[1], *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(values[2], *(int *)VectorGet(vec, 1));

   VectorFree(vec);
}

void test_VectorRemoveElement_AtZeroWithSinglePresentElement_WithBuf(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int value = 42;
   VectorPush(vec, &value);

   int buffer;
   TEST_ASSERT_TRUE(VectorRemove(vec, 0, &buffer));
   TEST_ASSERT_EQUAL_INT(42, buffer);
   TEST_ASSERT_TRUE(VectorIsEmpty(vec));

   VectorFree(vec);
}

void test_VectorRemoveElement_AtZeroWithEmptyVector_WithBuf(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int buffer = 52;
   TEST_ASSERT_FALSE(VectorRemove(vec, 0, &buffer));
   TEST_ASSERT_EQUAL_INT(52, buffer); // Confirm buffer is unchanged
   TEST_ASSERT_TRUE(VectorIsEmpty(vec));

   VectorFree(vec);
}

void test_VectorRemoveElement_AtMiddle_WithBuf(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer;
   TEST_ASSERT_TRUE(VectorRemove(vec, 1, &buffer));
   TEST_ASSERT_EQUAL_INT(84, buffer);
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(values[0], *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(values[2], *(int *)VectorGet(vec, 1));

   VectorFree(vec);
}

void test_VectorRemoveElement_AtLen_WithBuf(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer = 52;
   TEST_ASSERT_FALSE(VectorRemove(vec, 3, &buffer));
   TEST_ASSERT_EQUAL_INT(52, buffer); // Confirm buffer is unchanged
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(vec));

   VectorFree(vec);
}

void test_VectorRemoveElement_LastElement_WithBuf(void)
{
   struct Vector * vec1 = NULL;
   struct Vector * vec2 = NULL;
   VECTOR_NEW_KEEP_TRYIN(vec1, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   VECTOR_NEW_KEEP_TRYIN(vec2, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++)
   {
      VectorPush(vec1, &values[i]);
      VectorPush(vec2, &values[i]);
   }

   int buffer;
   TEST_ASSERT_TRUE(VectorRemove(vec1, 2, &buffer));
   TEST_ASSERT_EQUAL_INT(values[2], buffer);
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec1));
   TEST_ASSERT_EQUAL_INT(values[0], *(int *)VectorGet(vec1, 0));
   TEST_ASSERT_EQUAL_INT(values[1], *(int *)VectorGet(vec1, 1));

   // Should be the same result as VectorRemoveLastElement
   TEST_ASSERT_TRUE(VectorRemoveLastElement(vec2, &buffer));
   TEST_ASSERT_EQUAL_INT(values[2], buffer);
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec2));
   TEST_ASSERT_EQUAL_INT(values[0], *(int *)VectorGet(vec2, 0));
   TEST_ASSERT_EQUAL_INT(values[1], *(int *)VectorGet(vec2, 1));

   VectorFree(vec1);
   VectorFree(vec2);
}

void test_VectorRemoveElement_PastLen_WithBuf(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer;
   TEST_ASSERT_FALSE(VectorRemove(vec, 5, &buffer));
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(vec));

   VectorFree(vec);
}

void test_VectorRemoveLastElement_EmptyVec(void)
{
   struct Vector * vec = VectorNew(sizeof(char), 0, 10, NULL, 0, NULL);
   TEST_ASSERT_FALSE(VectorRemoveLastElement(vec, NULL));
   VectorFree(vec);
}

/******************************** Vector Clears *******************************/
void test_VectorClearElementAt_Normal(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   // Clear the second element
   TEST_ASSERT_TRUE(VectorClearElementAt(vec, 1));

   // Verify the cleared element is zeroed out
   int * cleared = (int *)VectorGet(vec, 1);
   TEST_ASSERT_NOT_NULL(cleared);
   TEST_ASSERT_EQUAL_INT(0, *cleared);

   // Verify other elements remain unchanged
   int * first = (int *)VectorGet(vec, 0);
   int * third = (int *)VectorGet(vec, 2);
   TEST_ASSERT_NOT_NULL(first);
   TEST_ASSERT_NOT_NULL(third);
   TEST_ASSERT_EQUAL_INT(42, *first);
   TEST_ASSERT_EQUAL_INT(126, *third);

   VectorFree(vec);
}

void test_VectorClearElementAt_InvalidIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   // Attempt to clear an invalid index (greater than length)
   TEST_ASSERT_FALSE(VectorClearElementAt(vec, 5));
   TEST_ASSERT_FALSE(VectorClearElementAt(vec, UINT32_MAX));

   // Verify that the vector remains unchanged
   for (size_t i = 0; i < 3; i++) {
      int * element = (int *)VectorGet(vec, i);
      TEST_ASSERT_NOT_NULL(element);
      TEST_ASSERT_EQUAL_INT(values[i], *element);
   }

   VectorFree(vec);
}

/********** Vector Range: Clear Elements *****************/

void test_VectorClear_Normal(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   VectorRangePush(vec, values, 5);

   TEST_ASSERT_TRUE(VectorClear(vec));
   TEST_ASSERT_EQUAL_size_t(5, VectorLength(vec));
   for (size_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorClear_EmptyVec(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   TEST_ASSERT_TRUE(VectorClear(vec));

   VectorFree(vec);
}

void test_VectorClear_InvalidVec(void)
{
   TEST_ASSERT_FALSE(VectorClear(NULL));
}

/******************************** Vector Resets *******************************/

void test_VectorReset_EmptyVec(void)
{
   // FIXME: Assumes allocations don't fail
   struct Vector * vec = VectorNew( sizeof(char), 0, 10, NULL, 0, NULL );
   TEST_ASSERT_TRUE( VectorReset(vec) );
   TEST_ASSERT_TRUE( VectorIsEmpty(vec) );
   VectorFree(vec);
}

void test_VectorReset(void)
{
   struct Vector * vec = NULL;

   VECTOR_NEW_KEEP_TRYIN(vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR)
   TEST_ASSERT_NOT_NULL(vec);

   int value = 42;
   while ( VectorLength(vec) < 10 )
   {
      VectorPush(vec, &value);
   }
   TEST_ASSERT_FALSE(VectorIsEmpty(vec));

   TEST_ASSERT_TRUE(VectorReset(vec));
   TEST_ASSERT_TRUE(VectorIsEmpty(vec));

   VectorFree(vec);
}

/******************************** Vector Reset *******************************/
void test_VectorHardReset(void)
{
   struct Vector * vec = NULL;

   VECTOR_NEW_KEEP_TRYIN(vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR)
   TEST_ASSERT_NOT_NULL(vec);

   // Add some elements to the vector
   int value1 = 42, value2 = 84, value3 = 126;
   VectorPush(vec, &value1);
   VectorPush(vec, &value2);
   VectorPush(vec, &value3);

   // Perform a hard reset
   TEST_ASSERT_TRUE(VectorHardReset(vec));

   // Verify the vector is empty
   TEST_ASSERT_EQUAL_size_t(0, VectorLength(vec));
   TEST_ASSERT_EQUAL_size_t(0, VectorCapacity(vec));
   TEST_ASSERT_TRUE(VectorIsEmpty(vec));

   VectorFree(vec);
}

void test_VectorHardReset_EmptyVector(void)
{
   struct Vector * vec = VectorNew( sizeof(int), 0, 10, NULL, 0, NULL );
   TEST_ASSERT_TRUE( VectorHardReset(vec) );
   VectorFree(vec);
}

/******************************* Vector Duplicate *****************************/
void test_VectorDuplicate_SmallVector(void)
{
   const size_t max_cap = 10;
   struct Vector * original = NULL;
   VECTOR_NEW_KEEP_TRYIN(original, sizeof(int), 5, max_cap, NULL, 0, &DEFAULT_ALLOCATOR);

   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(original, &values[i]);
   }

   struct Vector * duplicate = VectorDuplicate(original);
   TEST_ASSERT_NOT_NULL(duplicate);

   // Verify the duplicate matches the original
   TEST_ASSERT_EQUAL_size_t(VectorLength(original), VectorLength(duplicate));
   TEST_ASSERT_EQUAL_size_t(VectorCapacity(original), VectorCapacity(duplicate));
   TEST_ASSERT_EQUAL_size_t(VectorMaxCapacity(original), VectorMaxCapacity(duplicate));
   TEST_ASSERT_EQUAL_size_t(VectorElementSize(original), VectorElementSize(duplicate));

   for (size_t i = 0; i < VectorLength(original); i++) {
      int * original_element = (int *)VectorGet(original, i);
      int * duplicate_element = (int *)VectorGet(duplicate, i);
      TEST_ASSERT_NOT_NULL(original_element);
      TEST_ASSERT_NOT_NULL(duplicate_element);
      TEST_ASSERT_EQUAL_INT(* original_element, *duplicate_element);
   }

   // Catch incorrect capacity duplication by pushing many elements to the duplicate.
   // If the capacity allocation was incorrect, and the internal implementation
   // simply did the equivalent of dup->capacity = og->cap, we hopefully get an
   // out-of-bounds sourced crash. This is more likely with the larger vector,
   // but a strict system will catch the slightest out-of-bounds heap access.
   for ( size_t i = 0; i < (max_cap - VectorLength(duplicate)); i++ )
      VectorPush(duplicate, &(int){1});

   VectorFree(original);
   VectorFree(duplicate);
}

void test_VectorDuplicate_ReallyLargeVector(void)
{
   struct Vector * original = NULL;
   const size_t og_len = 10000000;
   const size_t og_cap = og_len * 10;
   VECTOR_NEW_KEEP_TRYIN(original, sizeof(uint8_t), og_len, og_cap, NULL, 0, &DEFAULT_ALLOCATOR);

   for (size_t i = 0; i < og_len; i++) {
      VectorPush(original, &(uint8_t){4});
   }

   struct Vector * duplicate = VectorDuplicate(original);
   TEST_ASSERT_NOT_NULL(duplicate);

   // Verify the duplicate matches the original
   TEST_ASSERT_EQUAL_size_t(VectorLength(original), VectorLength(duplicate));
   TEST_ASSERT_EQUAL_size_t(VectorCapacity(original), VectorCapacity(duplicate));
   TEST_ASSERT_EQUAL_size_t(VectorMaxCapacity(original), VectorMaxCapacity(duplicate));
   TEST_ASSERT_EQUAL_size_t(VectorElementSize(original), VectorElementSize(duplicate));

   for (size_t i = 0; i < VectorLength(original); i++) {
      uint8_t * original_element  = (uint8_t *)VectorGet(original, i);
      uint8_t * duplicate_element = (uint8_t *)VectorGet(duplicate, i);
      TEST_ASSERT_NOT_NULL(original_element);
      TEST_ASSERT_NOT_NULL(duplicate_element);
      TEST_ASSERT_EQUAL_UINT8(* original_element, *duplicate_element);
   }

   // Catch incorrect capacity duplication by pushing many elements to the duplicate.
   // If the capacity allocation was incorrect, and the internal implementation
   // simply did the equivalent of dup->capacity = og->cap, we hopefully get an
   // out-of-bounds sourced crash. This is more likely with the larger vector,
   // but a strict system will catch the slightest out-of-bounds heap access.
   for ( size_t i = 0; i < (og_cap - VectorLength(duplicate)); i++ )
      VectorPush(duplicate, &(int){1});

   VectorFree(original);
   VectorFree(duplicate);
}

void test_VectorDuplicate_CatchBadCapDuplication(void)
{
   const size_t maxcap = 1000000;
   int init_data[10] = {0}; // Zero-initialized array with 10 elements
   struct Vector * og = VectorNew(sizeof(int), maxcap, maxcap, init_data, 10, NULL);
   struct Vector * dup = VectorDuplicate(og);

   // Catch incorrect capacity duplication by pushing many elements to the duplicate.
   // If the capacity allocation was incorrect, and the internal implementation
   // simply did the equivalent of dup->capacity = og->cap, we hopefully get an
   // out-of-bounds sourced crash. This is more likely with the larger vector,
   // but a strict system will catch the slightest out-of-bounds heap access.
   for ( size_t i = 0; i < (maxcap - VectorLength(dup)); i++ )
      VectorPush(dup, &(int){1});

   VectorFree(dup);
   VectorFree(og);
}

void test_VectorDuplicate_NullVector(void)
{
   struct Vector * original = NULL;

   struct Vector * duplicate = VectorDuplicate(original);
   TEST_ASSERT_NULL(duplicate);
}

/********************************* Vector Move ********************************/
void test_VectorMove_SmallVector(void)
{
   struct Vector * original = NULL;
   VECTOR_NEW_KEEP_TRYIN(original, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int values[] = {42, 84, 126};
   for (size_t i = 0; i < 3; i++)
   {
      VectorPush(original, &values[i]);
   }

   struct Vector * new_vec = VectorNew(sizeof(int), 10, 50, NULL, 0, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_TRUE(VectorMove(new_vec, original));
   TEST_ASSERT_NOT_NULL(new_vec);

   // Verify the contents of new_vec
   TEST_ASSERT_EQUAL_size_t(VectorLength(new_vec), 3);
   TEST_ASSERT_EQUAL_size_t(VectorCapacity(new_vec), 10);
   TEST_ASSERT_EQUAL_size_t(VectorMaxCapacity(new_vec), 100);
   TEST_ASSERT_EQUAL_size_t(VectorElementSize(new_vec), sizeof(int));
   for (size_t i = 0; i < 3; i++)
   {
      TEST_ASSERT_EQUAL_INT( *( (int *)VectorGet(new_vec, i) ), values[i] );
   }

   // Verify that the old vector is in an empty but valid state
   TEST_ASSERT_EQUAL_size_t(VectorLength(original), 0);
   TEST_ASSERT_EQUAL_size_t(VectorCapacity(original), 0);
   TEST_ASSERT_EQUAL_size_t(VectorMaxCapacity(original), 100);
   TEST_ASSERT_EQUAL_size_t(VectorElementSize(original), sizeof(int));

   VectorFree(original);
   VectorFree(new_vec);
}

void test_VectorMove_ReallyLargeVector(void)
{
   struct Vector * original = NULL;
   const size_t OriginalVecLen = 10000000;
   VECTOR_NEW_KEEP_TRYIN(original, sizeof(uint8_t), OriginalVecLen, OriginalVecLen, NULL, 0, &DEFAULT_ALLOCATOR);

   const uint8_t test_val = 5;
   for (size_t i = 0; i < OriginalVecLen; i++)
   {
      VectorPush(original, &test_val);
   }

   struct Vector * new_vec = VectorNew(sizeof(uint8_t), 10, 50, NULL, 0, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_TRUE(VectorMove(new_vec, original));
   TEST_ASSERT_NOT_NULL(new_vec);

   // Verify the contents of new_vec
   TEST_ASSERT_EQUAL_size_t(VectorLength(new_vec), OriginalVecLen);
   TEST_ASSERT_EQUAL_size_t(VectorCapacity(new_vec), OriginalVecLen);
   TEST_ASSERT_EQUAL_size_t(VectorMaxCapacity(new_vec), OriginalVecLen);
   TEST_ASSERT_EQUAL_size_t(VectorElementSize(new_vec), sizeof(uint8_t));
   for (size_t i = 0; i < OriginalVecLen; i++)
   {
      TEST_ASSERT_EQUAL_UINT8( *((uint8_t *)VectorGet(new_vec, i)), test_val );
   }

   // Verify that the old vector is in an empty but valid state
   TEST_ASSERT_EQUAL_size_t(VectorLength(original), 0);
   TEST_ASSERT_EQUAL_size_t(VectorCapacity(original), 0);
   TEST_ASSERT_EQUAL_size_t(VectorMaxCapacity(original), OriginalVecLen);
   TEST_ASSERT_EQUAL_size_t(VectorElementSize(original), sizeof(uint8_t));

   VectorFree(original);
   VectorFree(new_vec);
}

void test_VectorMove_NullVector(void)
{
   struct Vector * original = NULL;
   struct Vector * new_vec = NULL;
   TEST_ASSERT_FALSE(VectorMove(new_vec, original));
   TEST_ASSERT_NULL(new_vec);
   new_vec = VectorNew(sizeof(int), 10, 100, NULL, 0, NULL);
   struct Vector * cpy = VectorDuplicate(new_vec);
   TEST_ASSERT_FALSE(VectorMove(new_vec, original));
   TEST_ASSERT_TRUE(VectorsAreEqual(new_vec, cpy));
}

void test_VectorMove_MismatchedVec(void)
{
   // Different element size
   struct Vector * original = VectorNew(sizeof(int),  10, 100, NULL, 0, NULL);
   struct Vector * new_vec  = VectorNew(sizeof(char), 10, 100, NULL, 0, NULL);
   TEST_ASSERT_FALSE(VectorMove(new_vec, original));

   // Different memory managers
   VectorFree(new_vec);
   new_vec = VectorNew(sizeof(int), 5, 100, NULL, 0, &TestNilMemMgr);
   TEST_ASSERT_FALSE(VectorMove(new_vec, original));
}

/****************************** Vectors Are Equal *****************************/
void test_VectorsAreEqual_SameVectors(void)
{
   struct Vector * vec = NULL;
   VECTOR_NEW_KEEP_TRYIN(vec, sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   size_t iter = 0;
   int val = 10;
   while ( (VectorLength(vec) < 5) && (iter < 100) )
   {
      bool result = VectorPush(vec, &val);
      if ( result ) val += 5;
   }

   // Vector should obviously be equal to itself
   TEST_ASSERT_TRUE( VectorsAreEqual(vec, vec) );

   VectorFree(vec);
}

void test_VectorsAreEqual_DifferentElementSz(void)
{
   struct Vector * vec1 = VectorNew(sizeof(uint8_t), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * vec2 = VectorNew(sizeof(float), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   TEST_ASSERT_FALSE(VectorsAreEqual(vec1, vec2));

   VectorFree(vec1);
   VectorFree(vec2);
}

void test_VectorsAreEqual_DifferentLength(void)
{
   struct Vector * vec1 = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * vec2 = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int value = 42;
   VectorPush(vec1, &value);

   TEST_ASSERT_FALSE(VectorsAreEqual(vec1, vec2));

   VectorFree(vec1);
   VectorFree(vec2);
}

void test_VectorsAreEqual_DifferentCapacity(void)
{
   struct Vector * vec1 = VectorNew(sizeof(int), 5, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * vec2 = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   TEST_ASSERT_FALSE(VectorsAreEqual(vec1, vec2));

   VectorFree(vec1);
   VectorFree(vec2);
}

void test_VectorsAreEqual_DifferentMaxCapacity(void)
{
   struct Vector * vec1 = VectorNew(sizeof(int), 10, 50, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * vec2 = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   TEST_ASSERT_FALSE(VectorsAreEqual(vec1, vec2));

   VectorFree(vec1);
   VectorFree(vec2);
}

void test_VectorsAreEqual_DifferentElementValues(void)
{
   struct Vector * vec1 = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * vec2 = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int value1 = 42, value2 = 84;
   VectorPush(vec1, &value1);
   VectorPush(vec2, &value2);

   TEST_ASSERT_FALSE(VectorsAreEqual(vec1, vec2));

   VectorFree(vec1);
   VectorFree(vec2);
}

/************************ Vector Range: Banana Split ***********************/

void test_VectorSplitAt_ValidIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   struct Vector * split_vec = VectorSplitAt(vec, 2);
   TEST_ASSERT_NOT_NULL(split_vec);

   // Verify original vector
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(vec, 1));

   // Verify split vector
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(split_vec));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(split_vec, 0));
   TEST_ASSERT_EQUAL_INT(40, *(int *)VectorGet(split_vec, 1));
   TEST_ASSERT_EQUAL_INT(50, *(int *)VectorGet(split_vec, 2));

   VectorFree(vec);
   VectorFree(split_vec);
}

void test_VectorSplitAt_IdxZero(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   struct Vector * split_vec = VectorSplitAt(vec, 0);
   TEST_ASSERT_NULL(split_vec);

   // Verify original vector remains unchanged
   TEST_ASSERT_EQUAL_size_t(5, VectorLength(vec));
   for (size_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_INT(values[i], *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorSplitAt_IdxPastLen(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   struct Vector * split_vec = VectorSplitAt(vec, 10);
   TEST_ASSERT_NULL(split_vec);

   // Verify original vector remains unchanged
   TEST_ASSERT_EQUAL_size_t(5, VectorLength(vec));
   for (size_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_INT(values[i], *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorSplitAt_EmptyVector(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   struct Vector * split_vec = VectorSplitAt(vec, 0);
   TEST_ASSERT_NULL(split_vec);

   // Verify original vector remains unchanged
   TEST_ASSERT_TRUE(VectorIsEmpty(vec));

   VectorFree(vec);
}

void test_VectorSplitAt_NullVector(void)
{
   struct Vector * split_vec = VectorSplitAt(NULL, 0);
   TEST_ASSERT_NULL(split_vec);
}

void test_VectorSplitAt_ValidIdx_StructData(void)
{
   struct MyData_S
   {
      float x;
      float y;
      float z;
   };
   struct Vector * vec = VectorNew(sizeof(struct MyData_S), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   struct MyData_S values[5] =
   {
      { .x = 1.0f, .y = 2.0f, .z = 3.0f },
      { .x = 4.0f, .y = 5.0f, .z = 6.0f },
      { .x = 7.0f, .y = 8.0f, .z = 9.0f },
      { .x = 10.0f, .y = 11.0f, .z = 12.0f },
      { .x = 13.0f, .y = 14.0f, .z = 15.0f }
   };
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   struct Vector * split_vec = VectorSplitAt(vec, 2);
   TEST_ASSERT_NOT_NULL(split_vec);

   // Verify original vector
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec));
   TEST_ASSERT_EQUAL_FLOAT(1.0f, ((struct MyData_S *)VectorGet(vec, 0))->x);
   TEST_ASSERT_EQUAL_FLOAT(4.0f, ((struct MyData_S *)VectorGet(vec, 1))->x);

   // Verify split vector
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(split_vec));
   TEST_ASSERT_EQUAL_FLOAT(7.0f, ((struct MyData_S *)VectorGet(split_vec, 0))->x);
   TEST_ASSERT_EQUAL_FLOAT(10.0f, ((struct MyData_S *)VectorGet(split_vec, 1))->x);
   TEST_ASSERT_EQUAL_FLOAT(13.0f, ((struct MyData_S *)VectorGet(split_vec, 2))->x);

   VectorFree(vec);
   VectorFree(split_vec);
}

/*************************** Vector Range: Slice ***************************/

void test_VectorSlice_ValidIndices_IntData(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   // Slice from index 1 to 4 (exclusive, so elements 1,2,3)
   struct Vector * slice = VectorSlice(vec, 1, 4);
   TEST_ASSERT_NOT_NULL(slice);
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(slice));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(slice, 0));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(slice, 1));
   TEST_ASSERT_EQUAL_INT(40, *(int *)VectorGet(slice, 2));

   VectorFree(vec);
   VectorFree(slice);
}

void test_VectorSlice_ValidIndices_StructData(void)
{
   struct MyData_S
   {
      float x;
      float y;
      float z;
   };
   struct Vector * vec = VectorNew(sizeof(struct MyData_S), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   struct MyData_S values[5] =
   {
      { .x = 1.0f, .y = 2.0f, .z = 3.0f },
      { .x = 4.0f, .y = 5.0f, .z = 6.0f },
      { .x = 7.0f, .y = 8.0f, .z = 9.0f },
      { .x = 10.0f, .y = 11.0f, .z = 12.0f },
      { .x = 13.0f, .y = 14.0f, .z = 15.0f }
   };
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   // Slice from index 2 to 5 (exclusive, so elements 2,3,4)
   struct Vector * slice = VectorSlice(vec, 2, 5);
   TEST_ASSERT_NOT_NULL(slice);
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(slice));
   struct MyData_S * elm = (struct MyData_S *)VectorGet(slice, 0);
   TEST_ASSERT_EQUAL_FLOAT(7.0f, elm->x);
   TEST_ASSERT_EQUAL_FLOAT(8.0f, elm->y);
   TEST_ASSERT_EQUAL_FLOAT(9.0f, elm->z);
   elm = (struct MyData_S *)VectorGet(slice, 1);
   TEST_ASSERT_EQUAL_FLOAT(10.0f, elm->x);
   TEST_ASSERT_EQUAL_FLOAT(11.0f, elm->y);
   TEST_ASSERT_EQUAL_FLOAT(12.0f, elm->z);
   elm = (struct MyData_S *)VectorGet(slice, 2);
   TEST_ASSERT_EQUAL_FLOAT(13.0f, elm->x);
   TEST_ASSERT_EQUAL_FLOAT(14.0f, elm->y);
   TEST_ASSERT_EQUAL_FLOAT(15.0f, elm->z);

   VectorFree(vec);
   VectorFree(slice);
}

void test_VectorSlice_IdxStartEqualsIdxEnd(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   // Slice a single element (from 3 to 4, exclusive)
   struct Vector * slice = VectorSlice(vec, 3, 4);
   TEST_ASSERT_NOT_NULL(slice);
   TEST_ASSERT_EQUAL_size_t(1, VectorLength(slice));
   TEST_ASSERT_EQUAL_INT(40, *(int *)VectorGet(slice, 0));

   VectorFree(vec);
   VectorFree(slice);
}

void test_VectorSlice_IdxStartZero(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   // Slice from start to index 3 (exclusive, so elements 0,1,2)
   struct Vector * slice = VectorSlice(vec, 0, 3);
   TEST_ASSERT_NOT_NULL(slice);
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(slice));
   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(slice, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(slice, 1));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(slice, 2));

   VectorFree(vec);
   VectorFree(slice);
}

void test_VectorSlice_FullVector(void)
{
   // Basically just like duplication
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   // Slice that is the full vector (0 to 5 exclusive)
   struct Vector * slice = VectorSlice(vec, 0, 5);
   // Duplicate, which should be equivalent
   struct Vector * dup = VectorDuplicate(vec);

   TEST_ASSERT_NOT_NULL(slice);
   TEST_ASSERT_EQUAL_size_t(5, VectorLength(slice));
   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(slice, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(slice, 1));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(slice, 2));
   TEST_ASSERT_EQUAL_INT(40, *(int *)VectorGet(slice, 3));
   TEST_ASSERT_EQUAL_INT(50, *(int *)VectorGet(slice, 4));

   TEST_ASSERT_NOT_NULL(dup);
   TEST_ASSERT_EQUAL_size_t(5, VectorLength(dup));
   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(dup, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(dup, 1));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(dup, 2));
   TEST_ASSERT_EQUAL_INT(40, *(int *)VectorGet(dup, 3));
   TEST_ASSERT_EQUAL_INT(50, *(int *)VectorGet(dup, 4));

   VectorFree(vec);
   VectorFree(slice);
   VectorFree(dup);
}

void test_VectorSlice_IdxEndAtLastElement(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   // Slice from index 2 to 5 (exclusive, so elements 2,3,4)
   struct Vector * slice = VectorSlice(vec, 2, 5);
   TEST_ASSERT_NOT_NULL(slice);
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(slice));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(slice, 0));
   TEST_ASSERT_EQUAL_INT(40, *(int *)VectorGet(slice, 1));
   TEST_ASSERT_EQUAL_INT(50, *(int *)VectorGet(slice, 2));

   VectorFree(vec);
   VectorFree(slice);
}

void test_VectorSlice_EmptyVector(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   struct Vector * slice = VectorSlice(vec, 0, 0);
   TEST_ASSERT_NULL(slice);

   VectorFree(vec);
}

void test_VectorSlice_NullVector(void)
{
   struct Vector * slice = VectorSlice(NULL, 0, 0);
   TEST_ASSERT_NULL(slice);
}

void test_VectorSlice_IdxStartGreaterThanIdxEnd(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   struct Vector * slice = VectorSlice(vec, 3, 2);
   TEST_ASSERT_NULL(slice);

   VectorFree(vec);
}

void test_VectorSlice_IdxEndOutOfRange(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   struct Vector * slice = VectorSlice(vec, 1, 5);
   TEST_ASSERT_NULL(slice);

   VectorFree(vec);
}

/*********************** Vector Range: Concatenation ***********************/

void test_VectorConcatenate_BasicUse(void)
{
   struct Vector * v1 = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * v2 = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   int vals1[] = {1, 2, 3};
   int vals2[] = {4, 5};

   for (size_t i = 0; i < 3; i++) VectorPush(v1, &vals1[i]);
   for (size_t i = 0; i < 2; i++) VectorPush(v2, &vals2[i]);

   struct Vector * cat = VectorConcatenate(v1, v2);

   TEST_ASSERT_NOT_NULL(cat);
   TEST_ASSERT_EQUAL_size_t(5, VectorLength(cat));
   TEST_ASSERT_EQUAL_size_t(10, VectorCapacity(cat));
   TEST_ASSERT_EQUAL_size_t(20, VectorMaxCapacity(cat));
   for (size_t i = 0; i < 3; i++)
   {
      TEST_ASSERT_EQUAL_INT(vals1[i], *(int*)VectorGet(cat, i));
   }
   for (size_t i = 0; i < 2; i++)
   {
      TEST_ASSERT_EQUAL_INT(vals2[i], *(int*)VectorGet(cat, i+3));
   }
   
   VectorFree(v1);
   VectorFree(v2);
   VectorFree(cat);
}

void test_VectorConcatenate_VecsNotMutated(void)
{
   struct Vector * v1 = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * v2 = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   int vals1[] = {1, 2, 3};
   int vals2[] = {4, 5};

   for (size_t i = 0; i < 3; i++) VectorPush(v1, &vals1[i]);
   for (size_t i = 0; i < 2; i++) VectorPush(v2, &vals2[i]);
   struct Vector * v1_dup = VectorDuplicate(v1);
   struct Vector * v2_dup = VectorDuplicate(v2);

   struct Vector * cat = VectorConcatenate(v1, v2);

   TEST_ASSERT_EQUAL_INT( 1, vals1[0] );
   TEST_ASSERT_EQUAL_INT( 2, vals1[1] );
   TEST_ASSERT_EQUAL_INT( 3, vals1[2] );
   TEST_ASSERT_EQUAL_INT( 3, VectorLength(v1) );
   TEST_ASSERT_EQUAL_INT( 5, VectorCapacity(v1) );
   TEST_ASSERT_EQUAL_INT( 10, VectorMaxCapacity(v1) );
   TEST_ASSERT_TRUE( VectorsAreEqual(v1, v1_dup) );

   TEST_ASSERT_EQUAL_INT( 4, vals2[0] );
   TEST_ASSERT_EQUAL_INT( 5, vals2[1] );
   TEST_ASSERT_EQUAL_INT( 2, VectorLength(v2) );
   TEST_ASSERT_EQUAL_INT( 5, VectorCapacity(v2) );
   TEST_ASSERT_EQUAL_INT( 10, VectorMaxCapacity(v2) );
   TEST_ASSERT_TRUE( VectorsAreEqual(v2, v2_dup) );
   
   VectorFree(v1); VectorFree(v1_dup);
   VectorFree(v2); VectorFree(v2_dup);
   VectorFree(cat);
}

void test_VectorConcatenate_OneVecIsEmpty(void)
{
   struct Vector * v1 = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * v2 = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   int vals1[] = {1, 2, 3};
   for (size_t i = 0; i < 3; i++) VectorPush(v1, &vals1[i]);
   TEST_ASSERT_TRUE(VectorIsEmpty(v2));

   struct Vector * cat = VectorConcatenate(v1, v2);

   TEST_ASSERT_NOT_NULL(cat);
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(cat));
   for (size_t i = 0; i < 3; i++)
   {
      TEST_ASSERT_EQUAL_INT(vals1[i], *(int*)VectorGet(cat, i));
   }

   // Test reverse operation: cat2 = v2 + v1
   struct Vector * cat2 = VectorConcatenate(v2, v1);

   TEST_ASSERT_NOT_NULL(cat);
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(cat));
   for (size_t i = 0; i < 3; i++)
   {
      TEST_ASSERT_EQUAL_INT(vals1[i], *(int*)VectorGet(cat, i));
   }

   VectorFree(v1);
   VectorFree(v2);
   VectorFree(cat);
   VectorFree(cat2);
}

void test_VectorConcatenate_BothVecsEmpty(void)
{
   struct Vector * v1 = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * v2 = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_TRUE(VectorIsEmpty(v1));
   TEST_ASSERT_TRUE(VectorIsEmpty(v2));

   struct Vector * cat = VectorConcatenate(v1, v2);

   TEST_ASSERT_NOT_NULL(cat);
   TEST_ASSERT_TRUE(VectorIsEmpty(cat));

   VectorFree(v1);
   VectorFree(v2);
   VectorFree(cat);
}

void test_VectorConcatenate_FullVectors(void)
{
   struct Vector * v1 = VectorNew(sizeof(int), 3, 3, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * v2 = VectorNew(sizeof(int), 2, 2, NULL, 0, &DEFAULT_ALLOCATOR);
   int vals1[] = {1, 2, 3};
   int vals2[] = {4, 5};
   for (size_t i = 0; i < 3; i++) VectorPush(v1, &vals1[i]);
   for (size_t i = 0; i < 2; i++) VectorPush(v2, &vals2[i]);
   TEST_ASSERT_TRUE(VectorIsFull(v1));
   TEST_ASSERT_TRUE(VectorIsFull(v2));

   struct Vector * cat = VectorConcatenate(v1, v2);

   TEST_ASSERT_NOT_NULL(cat);
   TEST_ASSERT_EQUAL_INT(5, VectorLength(cat));
   TEST_ASSERT_TRUE(VectorIsFull(cat));

   VectorFree(v1);
   VectorFree(v2);
   VectorFree(cat);
}

void test_VectorConcatenate_NullArguments(void)
{
   struct Vector * v1 = VectorNew(sizeof(int), 2, 2, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * cat = VectorConcatenate(NULL, v1);
   TEST_ASSERT_NULL(cat);
   cat = VectorConcatenate(v1, NULL);
   TEST_ASSERT_NULL(cat);
   cat = VectorConcatenate(NULL, NULL);
   TEST_ASSERT_NULL(cat);
   VectorFree(v1);
}

void test_VectorConcatenate_DifferentElementSizes(void)
{
   struct Vector * v1 = VectorNew(sizeof(int), 2, 2, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * v2 = VectorNew(sizeof(double), 2, 2, NULL, 0, &DEFAULT_ALLOCATOR);

   struct Vector * cat = VectorConcatenate(v1, v2);
   TEST_ASSERT_NULL(cat);

   VectorFree(v1);
   VectorFree(v2);
}

void test_VectorConcatenate_ConcatenateSplitRoundTrip(void)
{
   // Concatenate and split at may be considered somewhat inverse operations.
   // Check that they behave that way. Note that capacity and max capacity are
   // not inverted through these operations.
   struct Vector * v1 = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * v2 = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   int vals1[] = {1, 2, 3};
   int vals2[] = {4, 5};

   for (size_t i = 0; i < 3; i++) VectorPush(v1, &vals1[i]);
   for (size_t i = 0; i < 2; i++) VectorPush(v2, &vals2[i]);

   struct Vector * cat = VectorConcatenate(v1, v2);
   struct Vector * v2_from_split = VectorSplitAt(cat, 3);

   TEST_ASSERT_EQUAL_INT( VectorLength(v1), VectorLength(cat) );
   for (size_t i = 0; i < 3; i++)
   {
      TEST_ASSERT_EQUAL_INT(
         *(int*)VectorGet(v1,  i),
         *(int*)VectorGet(cat, i) );
   }
   TEST_ASSERT_EQUAL_INT( VectorLength(v2), VectorLength(v2_from_split) );
   for (size_t i = 0; i < 2; i++)
   {
      TEST_ASSERT_EQUAL_INT(
         *(int*)VectorGet(v2,            i),
         *(int*)VectorGet(v2_from_split, i));
   }
   
   VectorFree(v1);
   VectorFree(v2);
   VectorFree(v2_from_split);
   VectorFree(cat);
}

/*********************** Vector Range: Push Elements ***********************/

void test_VectorRangePush_ValidInts(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   int data[] = {1, 2, 3};

   TEST_ASSERT_TRUE(VectorRangePush(vec, data, 3));
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(vec));
   for (size_t i = 0; i < 3; i++)
   {
      TEST_ASSERT_EQUAL_INT(data[i], *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorRangePush_ValidStructs(void)
{
   struct MyData_S {
      float x, y, z;
   };
   struct Vector * vec = VectorNew(sizeof(struct MyData_S), 2, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   struct MyData_S data[2] =
   {
      { .x = 1.0f, .y = 2.0f, .z = 3.0f},
      { .x = 4.0f, .y = 5.0f, .z = 6.0f}
   };

   TEST_ASSERT_TRUE(VectorRangePush(vec, data, 2));
   TEST_ASSERT_EQUAL_size_t(2, VectorLength(vec));
   struct MyData_S * elm = (struct MyData_S *)VectorGet(vec, 0);
   TEST_ASSERT_EQUAL_FLOAT(1.0f, elm->x);
   TEST_ASSERT_EQUAL_FLOAT(2.0f, elm->y);
   TEST_ASSERT_EQUAL_FLOAT(3.0f, elm->z);
   elm = (struct MyData_S *)VectorGet(vec, 1);
   TEST_ASSERT_EQUAL_FLOAT(4.0f, elm->x);
   TEST_ASSERT_EQUAL_FLOAT(5.0f, elm->y);
   TEST_ASSERT_EQUAL_FLOAT(6.0f, elm->z);

   VectorFree(vec);
}

void test_VectorRangePush_ExpandCapacity(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   int data[] = {1, 2, 3, 4};

   TEST_ASSERT_TRUE(VectorRangePush(vec, data, 4));
   TEST_ASSERT_EQUAL_size_t(4, VectorLength(vec));
   for (size_t i = 0; i < 4; i++)
   {
      TEST_ASSERT_EQUAL_INT(data[i], *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorRangePush_ZeroLen(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   int data[] = {1, 2, 3};

   TEST_ASSERT_FALSE(VectorRangePush(vec, data, 0));
   TEST_ASSERT_EQUAL_size_t(0, VectorLength(vec));

   VectorFree(vec);
}

void test_VectorRangePush_NullVec(void)
{
   int data[] = {1, 2, 3};
   TEST_ASSERT_FALSE(VectorRangePush(NULL, data, 3));
}

void test_VectorRangePush_NullData(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   TEST_ASSERT_FALSE(VectorRangePush(vec, NULL, 3));
   VectorFree(vec);
}

void test_VectorRangePush_ExceedsMaxCapacity(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 4, NULL, 0, &DEFAULT_ALLOCATOR);
   int data[] = {1, 2, 3, 4, 5};

   TEST_ASSERT_FALSE(VectorRangePush(vec, data, 5));
   TEST_ASSERT_EQUAL_size_t(0, VectorLength(vec));

   VectorFree(vec);
}

void test_VectorRangePush_ExactlyMaxCapacity(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 4, NULL, 0, &DEFAULT_ALLOCATOR);
   int data[] = {1, 2, 3, 4};

   TEST_ASSERT_TRUE(VectorRangePush(vec, data, 4));
   TEST_ASSERT_EQUAL_size_t(4, VectorLength(vec));
   TEST_ASSERT_EQUAL_size_t(4, VectorCapacity(vec));
   for (size_t i = 0; i < 4; i++)
   {
      TEST_ASSERT_EQUAL_INT(data[i], *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

/********************** Vector Range: Insert Elements **********************/

void test_VectorRangeInsert_ValidInts(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);

   int initial[] = {1, 2, 5};
   for (size_t i = 0; i < 3; i++) 
   {
      VectorPush(vec, &initial[i]);
   }

   int insert[] = {3, 4};
   // Insert at index 2 (before 5)
   TEST_ASSERT_TRUE(VectorRangeInsert(vec, 2, insert, 2));
   TEST_ASSERT_EQUAL_size_t(5, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(1, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(2, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(3, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(4, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(5, *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRangeInsert_ValidStructs(void)
{
   struct MyData_S { int x, y, z; };
   struct Vector * vec = VectorNew(sizeof(struct MyData_S), 4, 10, NULL, 0, &DEFAULT_ALLOCATOR);

   struct MyData_S initial[2] = { {1,2,3}, {4,5,6} };
   for (size_t i = 0; i < 2; i++)  VectorPush(vec, &initial[i]);

   struct MyData_S insert[2] = { {7,8,9}, {10,11,12} };
   // Insert at index 1
   TEST_ASSERT_TRUE(VectorRangeInsert(vec, 1, insert, 2));

   TEST_ASSERT_EQUAL_size_t(4, VectorLength(vec));

   struct MyData_S * elm = (struct MyData_S *)VectorGet(vec, 1);
   TEST_ASSERT_EQUAL_FLOAT(7, elm->x);
   TEST_ASSERT_EQUAL_FLOAT(8, elm->y);
   TEST_ASSERT_EQUAL_FLOAT(9, elm->z);
   elm = (struct MyData_S *)VectorGet(vec, 2);
   TEST_ASSERT_EQUAL_FLOAT(10, elm->x);
   TEST_ASSERT_EQUAL_FLOAT(11, elm->y);
   TEST_ASSERT_EQUAL_FLOAT(12, elm->z);
   elm = (struct MyData_S *)VectorGet(vec, 3);
   TEST_ASSERT_EQUAL_FLOAT(4, elm->x);
   TEST_ASSERT_EQUAL_FLOAT(5, elm->y);
   TEST_ASSERT_EQUAL_FLOAT(6, elm->z);

   VectorFree(vec);
}

void test_VectorRangeInsert_ExpandCapacity(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 10, NULL, 0, &DEFAULT_ALLOCATOR);

   int initial[] = {1, 2};
   for (size_t i = 0; i < 2; i++) VectorPush(vec, &initial[i]);

   int insert[] = {3, 4, 5};
   // Insert at index 1, should expand capacity
   TEST_ASSERT_TRUE(VectorRangeInsert(vec, 1, insert, 3));

   TEST_ASSERT_EQUAL_size_t(5, VectorLength(vec));

   TEST_ASSERT_EQUAL_INT(1, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(3, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(4, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(5, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(2, *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRangeInsert_ZeroLen(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 10, NULL, 0, &DEFAULT_ALLOCATOR);

   int data[] = {1, 2, 3};
   VectorPush(vec, &data[0]);

   TEST_ASSERT_FALSE(VectorRangeInsert(vec, 0, data, 0));
   TEST_ASSERT_EQUAL_size_t(1, VectorLength(vec));

   VectorFree(vec);
}

void test_VectorRangeInsert_NullVec(void)
{
   int data[] = {1, 2, 3};
   TEST_ASSERT_FALSE(VectorRangeInsert(NULL, 0, data, 2));
}

void test_VectorRangeInsert_NullData(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   VectorPush(vec, &(int){1});
   TEST_ASSERT_FALSE(VectorRangeInsert(vec, 0, NULL, 1));

   VectorFree(vec);
}

void test_VectorRangeInsert_ExceedsMaxCapacity(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 4, NULL, 0, &DEFAULT_ALLOCATOR);

   int data[] = {1, 2, 3, 4, 5};
   VectorPush(vec, &data[0]);

   // Would exceed max capacity (1+5 > 4)
   TEST_ASSERT_FALSE(VectorRangeInsert(vec, 0, data, 5));
   TEST_ASSERT_EQUAL_size_t(1, VectorLength(vec));

   VectorFree(vec);
}

void test_VectorRangeInsert_ExactlyMaxCapacity(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 5, NULL, 0, &DEFAULT_ALLOCATOR);

   int data[] = {1, 2, 3};
   int val = 0;
   VectorPush(vec, &val);
   val = 4;
   VectorPush(vec, &val);

   TEST_ASSERT_TRUE(VectorRangeInsert(vec, 1, data, 3));
   
   TEST_ASSERT_EQUAL_size_t(5, VectorLength(vec));
   TEST_ASSERT_EQUAL_size_t(5, VectorCapacity(vec));
   TEST_ASSERT_TRUE(VectorIsFull(vec));

   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(1, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(2, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(3, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(4, *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRangeInsert_Push_Equivalence(void)
{
   struct Vector * vecpush = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * vecinsert = VectorNew(sizeof(int), 5, 10, NULL, 0, &DEFAULT_ALLOCATOR);

   int data[] = {1, 2, 3};
   int val = 0;
   VectorPush(vecpush, &val);
   VectorPush(vecinsert, &val);
   VectorRangePush(vecpush, data, 3);
   VectorRangeInsert(vecinsert, 1, data, 3);

   for ( size_t i=0; i < 4; i++ )
   {
      TEST_ASSERT_EQUAL_INT( *(int *)VectorGet(vecpush, i),
                             *(int *)VectorGet(vecinsert, i) );
   }

   VectorFree(vecpush);
   VectorFree(vecinsert);
}

void test_VectorRangeInsert_InvalidIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 2, 4, NULL, 0, &DEFAULT_ALLOCATOR);

   int data[] = {1, 2, 3};
   VectorPush(vec, &data[0]);

   TEST_ASSERT_FALSE(VectorRangeInsert(vec, 2, &data[1], 2));
   TEST_ASSERT_FALSE(VectorRangeInsert(vec, 4, &data[1], 2));
   TEST_ASSERT_FALSE(VectorRangeInsert(vec, 1000, &data[1], 2));
   
   TEST_ASSERT_EQUAL_size_t(1, VectorLength(vec));

   VectorFree(vec);
}

void test_VectorRangeInsert_OneElm(void)
{
   struct Vector * v = VectorNew(sizeof(int), 5, 10, NULL, 0, NULL);
   (void)VectorInsert(v, 0, &(int){1});
   int val_ins; (void)VectorCpyElementAt(v, 0, &val_ins);
   (void)VectorSet(v, 0, &(int){0});
   (void)VectorReset(v);
   (void)VectorRangeInsert(v, 0, (int[]){1}, 1);
   int val_insrng; (void)VectorCpyElementAt(v, 0, &val_insrng);
   TEST_ASSERT_EQUAL_INT(val_ins, val_insrng);
   VectorFree(v);
}

void test_VectorRangeInsert_AfterHardResetting(void)
{
   const size_t LargeVecMaxCap = 10000;
   struct Vector * v = VectorNew(sizeof(int), 5, LargeVecMaxCap, NULL, 0, NULL);
   VectorPush(v, &(int){5});
   VectorHardReset(v);
   TEST_ASSERT_TRUE(VectorIsEmpty(v));
   TEST_ASSERT_EQUAL_size_t(VectorCapacity(v), 0);
   int arr[LargeVecMaxCap];
   TEST_ASSERT_TRUE(VectorRangeInsert(v, 0, arr, LargeVecMaxCap));
   VectorFree(v);
}

/****************** Vector Range: Copy Elements In Range *******************/

void test_VectorRangeCpy_ValidIdices_IntData(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[3];
   TEST_ASSERT_TRUE(VectorRangeCpy(vec, 1, 4, buffer));
   TEST_ASSERT_EQUAL_INT(20, buffer[0]);
   TEST_ASSERT_EQUAL_INT(30, buffer[1]);
   TEST_ASSERT_EQUAL_INT(40, buffer[2]);

   VectorFree(vec);
}

void test_VectorRangeCpy_DoesNotMutate(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[3];
   TEST_ASSERT_TRUE(VectorRangeCpy(vec, 1, 4, buffer));
   buffer[0] = 1;
   buffer[1] = 1;
   buffer[2] = 1;
   int * data;
   data = (int *)VectorGet(vec, 1);
   TEST_ASSERT_EQUAL_INT(20, *data);
   data = (int *)VectorGet(vec, 2);
   TEST_ASSERT_EQUAL_INT(30, *data);
   data = (int *)VectorGet(vec, 3);
   TEST_ASSERT_EQUAL_INT(40, *data);
}

void test_VectorRangeCpy_ValidIndices_StructData(void)
{
   struct MyData_S
   {
      float x;
      float y;
      float z;
   };
   struct Vector * vec = VectorNew(sizeof(struct MyData_S), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   struct MyData_S values[5] =
   {
      { .x = 1.0f, .y = 2.0f, .z = 3.0f },
      { .x = 4.0f, .y = 5.0f, .z = 6.0f },
      { .x = 7.0f, .y = 8.0f, .z = 9.0f },
      { .x = 10.0f, .y = 11.0f, .z = 12.0f },
      { .x = 13.0f, .y = 14.0f, .z = 15.0f }
   };
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   struct MyData_S buffer[2];
   TEST_ASSERT_TRUE(VectorRangeCpy(vec, 2, 4, buffer));
   TEST_ASSERT_EQUAL_FLOAT(7.0f, buffer[0].x);
   TEST_ASSERT_EQUAL_FLOAT(8.0f, buffer[0].y);
   TEST_ASSERT_EQUAL_FLOAT(9.0f, buffer[0].z);
   TEST_ASSERT_EQUAL_FLOAT(10.0f, buffer[1].x);
   TEST_ASSERT_EQUAL_FLOAT(11.0f, buffer[1].y);
   TEST_ASSERT_EQUAL_FLOAT(12.0f, buffer[1].z);

   VectorFree(vec);
}

void test_VectorRangeCpy_FullVector(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[5];
   TEST_ASSERT_TRUE(VectorRangeCpy(vec, 0, VectorLength(vec), buffer));
   TEST_ASSERT_EQUAL_INT(10, buffer[0]);
   TEST_ASSERT_EQUAL_INT(20, buffer[1]);
   TEST_ASSERT_EQUAL_INT(30, buffer[2]);
   TEST_ASSERT_EQUAL_INT(40, buffer[3]);
   TEST_ASSERT_EQUAL_INT(50, buffer[4]);

   VectorFree(vec);
}

void test_VectorRangeCpy_FullVector_IncorrectEndIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[5];
   TEST_ASSERT_FALSE(VectorRangeCpy(vec, 0, VectorLength(vec) + 1, buffer));

   VectorFree(vec);
}

void test_VectorRangeCpy_EmptyVec(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int buffer[3];
   TEST_ASSERT_FALSE(VectorRangeCpy(vec, 0, 3, buffer));

   VectorFree(vec);
}

void test_VectorRangeCpy_InvalidIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[3];
   TEST_ASSERT_FALSE(VectorRangeCpy(vec, 2, 5, buffer)); // End index out of range
   TEST_ASSERT_FALSE(VectorRangeCpy(vec, 5, 6, buffer)); // Start index out of range
   TEST_ASSERT_FALSE(VectorRangeCpy(vec, 2, 1, buffer)); // Start index > End index

   VectorFree(vec);
}

void test_VectorRangeCpy_InvalidVec(void)
{
   int buffer[3];
   TEST_ASSERT_FALSE(VectorRangeCpy(NULL, 0, 3, buffer));
}

void test_VectorRangeCpy_SameIdices(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[1];
   TEST_ASSERT_FALSE(VectorRangeCpy(vec, 2, 2, buffer));

   VectorFree(vec);
}

/************** Vector Range: Copy Elements From Idx To End ****************/

void test_VectorRangeCpyToEnd_ValidIdices_IntData(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[3];
   TEST_ASSERT_TRUE(VectorRangeCpyToEnd(vec, 2, buffer));
   TEST_ASSERT_EQUAL_INT(30, buffer[0]);
   TEST_ASSERT_EQUAL_INT(40, buffer[1]);
   TEST_ASSERT_EQUAL_INT(50, buffer[2]);

   VectorFree(vec);
}

void test_VectorRangeCpyToEnd_DoesNotMutate(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[3];
   VectorRangeCpyToEnd(vec, 2, buffer);
   buffer[0] = 1;
   buffer[1] = 1;
   buffer[2] = 1;
   int * data;
   data = (int *)VectorGet(vec, 2);
   TEST_ASSERT_EQUAL_INT(30, *data);
   data = (int *)VectorGet(vec, 3);
   TEST_ASSERT_EQUAL_INT(40, *data);
   data = (int *)VectorGet(vec, 4);
   TEST_ASSERT_EQUAL_INT(50, *data);

   VectorFree(vec);
}

void test_VectorRangeCpyToEnd_ValidIndices_StructData(void)
{
   struct MyData_S
   {
      float x;
      float y;
      float z;
   };
   struct Vector * vec = VectorNew(sizeof(struct MyData_S), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   struct MyData_S values[5] =
   {
      { .x = 1.0f, .y = 2.0f, .z = 3.0f },
      { .x = 4.0f, .y = 5.0f, .z = 6.0f },
      { .x = 7.0f, .y = 8.0f, .z = 9.0f },
      { .x = 10.0f, .y = 11.0f, .z = 12.0f },
      { .x = 13.0f, .y = 14.0f, .z = 15.0f }
   };
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   struct MyData_S buffer[3];
   TEST_ASSERT_TRUE(VectorRangeCpyToEnd(vec, 2, buffer));
   TEST_ASSERT_EQUAL_FLOAT(7.0f, buffer[0].x);
   TEST_ASSERT_EQUAL_FLOAT(8.0f, buffer[0].y);
   TEST_ASSERT_EQUAL_FLOAT(9.0f, buffer[0].z);
   TEST_ASSERT_EQUAL_FLOAT(10.0f, buffer[1].x);
   TEST_ASSERT_EQUAL_FLOAT(11.0f, buffer[1].y);
   TEST_ASSERT_EQUAL_FLOAT(12.0f, buffer[1].z);
   TEST_ASSERT_EQUAL_FLOAT(13.0f, buffer[2].x);
   TEST_ASSERT_EQUAL_FLOAT(14.0f, buffer[2].y);
   TEST_ASSERT_EQUAL_FLOAT(15.0f, buffer[2].z);

   VectorFree(vec);
}

void test_VectorRangeCpyToEnd_FullVector(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[5];
   TEST_ASSERT_TRUE(VectorRangeCpyToEnd(vec, 0, buffer));
   TEST_ASSERT_EQUAL_INT(10, buffer[0]);
   TEST_ASSERT_EQUAL_INT(20, buffer[1]);
   TEST_ASSERT_EQUAL_INT(30, buffer[2]);
   TEST_ASSERT_EQUAL_INT(40, buffer[3]);
   TEST_ASSERT_EQUAL_INT(50, buffer[4]);

   VectorFree(vec);
}

void test_VectorRangeCpyToEnd_EmptyVec(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int buffer[3];
   TEST_ASSERT_FALSE(VectorRangeCpyToEnd(vec, 0, buffer));

   VectorFree(vec);
}

void test_VectorRangeCpyToEnd_InvalidIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[3];
   TEST_ASSERT_FALSE(VectorRangeCpyToEnd(vec, 5, buffer));

   VectorFree(vec);
}

void test_VectorRangeCpyToEnd_InvalidVec(void)
{
   int buffer[3];
   TEST_ASSERT_FALSE(VectorRangeCpyToEnd(NULL, 0, buffer));
}

void test_VectorRangeCpyToEnd_EndIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[1];
   TEST_ASSERT_TRUE(VectorRangeCpyToEnd(vec, 4, buffer));
   TEST_ASSERT_EQUAL_INT(50, buffer[0]);

   VectorFree(vec);
}

/****************** Vector Range: Set Elements In Range ********************/

void test_VectorRangeSetWithArr_ValidIdices_IntData(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_values[] = {100, 200, 300};
   // Set indices 1, 2, 3 (exclusive end at 4)
   TEST_ASSERT_TRUE(VectorRangeSetWithArr(vec, 1, 4, new_values));

   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(100, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(200, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(300, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(50, *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRangeSetWithArr_DoesNotMutate(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_values[] = {100, 200, 300};
   // Set indices 1, 2, 3 (exclusive end at 4)
   TEST_ASSERT_TRUE(VectorRangeSetWithArr(vec, 1, 4, new_values));

   new_values[0] = 999;
   new_values[1] = 999;
   new_values[2] = 999;

   TEST_ASSERT_EQUAL_INT(100, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(200, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(300, *(int *)VectorGet(vec, 3));

   VectorFree(vec);
}

void test_VectorRangeSetWithArr_ValidIndices_StructData(void)
{
   struct MyData_S
   {
      float x;
      float y;
      float z;
   };
   struct Vector * vec = VectorNew(sizeof(struct MyData_S), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   struct MyData_S values[5] =
   {
      { .x = 1.0f, .y = 2.0f, .z = 3.0f },
      { .x = 4.0f, .y = 5.0f, .z = 6.0f },
      { .x = 7.0f, .y = 8.0f, .z = 9.0f },
      { .x = 10.0f, .y = 11.0f, .z = 12.0f },
      { .x = 13.0f, .y = 14.0f, .z = 15.0f }
   };
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   struct MyData_S new_values[2] =
   {
      { .x = 100.0f, .y = 200.0f, .z = 300.0f },
      { .x = 400.0f, .y = 500.0f, .z = 600.0f }
   };
   // Set indices 2, 3 (exclusive end at 4)
   TEST_ASSERT_TRUE(VectorRangeSetWithArr(vec, 2, 4, new_values));

   struct MyData_S * element = (struct MyData_S *)VectorGet(vec, 2);
   TEST_ASSERT_EQUAL_FLOAT(100.0f, element->x);
   TEST_ASSERT_EQUAL_FLOAT(200.0f, element->y);
   TEST_ASSERT_EQUAL_FLOAT(300.0f, element->z);

   element = (struct MyData_S *)VectorGet(vec, 3);
   TEST_ASSERT_EQUAL_FLOAT(400.0f, element->x);
   TEST_ASSERT_EQUAL_FLOAT(500.0f, element->y);
   TEST_ASSERT_EQUAL_FLOAT(600.0f, element->z);

   VectorFree(vec);
}

void test_VectorRangeSetWithArr_FullVector(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_values[] = {100, 200, 300, 400, 500};
   // Set indices 0,1,2,3,4 (exclusive end at 5)
   TEST_ASSERT_TRUE(VectorRangeSetWithArr(vec, 0, 5, new_values));

   for (size_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_INT(new_values[i], *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorRangeSetWithArr_FullVector_IncorrectEndIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_values[] = {100, 200, 300, 400, 500};
   // End index out of range (exclusive end at 6, but length is 5)
   TEST_ASSERT_FALSE(VectorRangeSetWithArr(vec, 0, 6, new_values));

   VectorFree(vec);
}

void test_VectorRangeSetWithArr_EmptyVec(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int new_values[] = {100, 200, 300};
   // No elements to set
   TEST_ASSERT_FALSE(VectorRangeSetWithArr(vec, 0, 3, new_values));

   VectorFree(vec);
}

void test_VectorRangeSetWithArr_InvalidIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_values[] = {100, 200};
   // End index out of range (exclusive end at 5, but length is 3)
   TEST_ASSERT_FALSE(VectorRangeSetWithArr(vec, 2, 5, new_values));
   // Start index out of range (start at 5, length is 3)
   TEST_ASSERT_FALSE(VectorRangeSetWithArr(vec, 5, 6, new_values));
   // Start index > end index
   TEST_ASSERT_FALSE(VectorRangeSetWithArr(vec, 2, 1, new_values));

   VectorFree(vec);
}

void test_VectorRangeSetWithArr_InvalidVec(void)
{
   TEST_ASSERT_FALSE(VectorRangeSetWithArr(NULL, 0, 3, (int[]){ 100, 200, 300 } ));
}

void test_VectorRangeSetWithArr_SameIdices(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_value = 100;
   TEST_ASSERT_FALSE(VectorRangeSetWithArr(vec, 2, 2, &new_value));

   VectorFree(vec);
}

void test_VectorRange_RoundTrip_CpyElementsToSetElementsInRng(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int buffer[3] = {0};
   // Copy indices 1,2,3 (exclusive end at 4)
   VectorRangeCpy(vec, 1, 4, buffer);
   buffer[0] = 100;
   buffer[1] = 200;
   buffer[2] = 300;
   // Set indices 1,2,3 (exclusive end at 4)
   VectorRangeSetWithArr(vec, 1, 4, buffer);

   TEST_ASSERT_EQUAL_INT(10,  *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(100, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(200, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(300, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(50,  *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

/* Set to Val */

void test_VectorRangeSetToVal_ValidIdices_IntData(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_val = 999;
   // Set indices 1, 2, 3 (exclusive end at 4)
   TEST_ASSERT_TRUE(VectorRangeSetToVal(vec, 1, 4, &new_val));

   TEST_ASSERT_EQUAL_INT(10,      *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(new_val, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(new_val, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(new_val, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(50,      *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRangeSetToVal_ValidIndices_StructData(void)
{
   struct MyData_S
   {
      float x;
      float y;
      float z;
   };
   struct Vector * vec = VectorNew(sizeof(struct MyData_S), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   struct MyData_S values[5] =
   {
      { .x = 1.0f, .y = 2.0f, .z = 3.0f },
      { .x = 4.0f, .y = 5.0f, .z = 6.0f },
      { .x = 7.0f, .y = 8.0f, .z = 9.0f },
      { .x = 10.0f, .y = 11.0f, .z = 12.0f },
      { .x = 13.0f, .y = 14.0f, .z = 15.0f }
   };
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   struct MyData_S new_value = { .x = 100.0f, .y = 200.0f, .z = 300.0f };
   // Set indices 2, 3 (exclusive end at 4)
   TEST_ASSERT_TRUE(VectorRangeSetToVal(vec, 2, 4, &new_value));

   struct MyData_S * element = (struct MyData_S *)VectorGet(vec, 2);
   TEST_ASSERT_EQUAL_FLOAT(100.0f, element->x);
   TEST_ASSERT_EQUAL_FLOAT(200.0f, element->y);
   TEST_ASSERT_EQUAL_FLOAT(300.0f, element->z);

   element = (struct MyData_S *)VectorGet(vec, 3);
   TEST_ASSERT_EQUAL_FLOAT(100.0f, element->x);
   TEST_ASSERT_EQUAL_FLOAT(200.0f, element->y);
   TEST_ASSERT_EQUAL_FLOAT(300.0f, element->z);

   // Verify other elements unchanged
   element = (struct MyData_S *)VectorGet(vec, 1);
   TEST_ASSERT_EQUAL_FLOAT(4.0f, element->x);
   TEST_ASSERT_EQUAL_FLOAT(5.0f, element->y);
   TEST_ASSERT_EQUAL_FLOAT(6.0f, element->z);

   VectorFree(vec);
}

void test_VectorRangeSetToVal_FullVector(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_value = 777;
   // Set all elements (exclusive end at 5)
   TEST_ASSERT_TRUE(VectorRangeSetToVal(vec, 0, 5, &new_value));

   for (size_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_INT(777, *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorRangeSetToVal_FullVector_IncorrectEndIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_value = 777;
   // End index out of range (exclusive end at 6, but length is 5)
   TEST_ASSERT_FALSE(VectorRangeSetToVal(vec, 0, 6, &new_value));

   // Verify vector unchanged
   for (size_t i = 0; i < 5; i++) {
      TEST_ASSERT_EQUAL_INT(values[i], *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorRangeSetToVal_EmptyVec(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   int new_value = 777;
   // No elements to set
   TEST_ASSERT_FALSE(VectorRangeSetToVal(vec, 0, 3, &new_value));

   VectorFree(vec);
}

void test_VectorRangeSetToVal_InvalidIdx(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_value = 777;
   // End index out of range (exclusive end at 5, but length is 3)
   TEST_ASSERT_FALSE(VectorRangeSetToVal(vec, 2, 5, &new_value));
   // Start index out of range (start at 5, length is 3)
   TEST_ASSERT_FALSE(VectorRangeSetToVal(vec, 5, 6, &new_value));
   // Start index > end index
   TEST_ASSERT_FALSE(VectorRangeSetToVal(vec, 2, 1, &new_value));

   // Verify vector unchanged
   for (size_t i = 0; i < 3; i++) {
      TEST_ASSERT_EQUAL_INT(values[i], *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorRangeSetToVal_InvalidVec(void)
{
   int new_value = 777;
   TEST_ASSERT_FALSE(VectorRangeSetToVal(NULL, 0, 3, &new_value));
}

void test_VectorRangeSetToVal_SameIdices(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_value = 777;
   // Same start and end indices (no range)
   TEST_ASSERT_FALSE(VectorRangeSetToVal(vec, 2, 2, &new_value));

   // Verify vector unchanged
   for (size_t i = 0; i < 3; i++) {
      TEST_ASSERT_EQUAL_INT(values[i], *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorRangeSetToVal_NullValue(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   for (size_t i = 0; i < 3; i++) {
      VectorPush(vec, &values[i]);
   }

   // NULL value pointer should fail
   TEST_ASSERT_FALSE(VectorRangeSetToVal(vec, 0, 2, NULL));

   // Verify vector unchanged
   for (size_t i = 0; i < 3; i++) {
      TEST_ASSERT_EQUAL_INT(values[i], *(int *)VectorGet(vec, i));
   }

   VectorFree(vec);
}

void test_VectorRangeSetToVal_SingleElement(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_value = 888;
   // Set single element (exclusive end at 3, so only index 2)
   TEST_ASSERT_TRUE(VectorRangeSetToVal(vec, 2, 3, &new_value));

   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(888, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(40, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(50, *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRangeSetToVal_AtBeginning(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_value = 111;
   // Set first two elements (indices 0, 1; exclusive end at 2)
   TEST_ASSERT_TRUE(VectorRangeSetToVal(vec, 0, 2, &new_value));

   TEST_ASSERT_EQUAL_INT(111, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(111, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(40, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(50, *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRangeSetToVal_AtEnd(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int new_value = 222;
   // Set last two elements (indices 3, 4; exclusive end at 5)
   TEST_ASSERT_TRUE(VectorRangeSetToVal(vec, 3, 5, &new_value));

   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(222, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(222, *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRangeSetToVal_ZeroValue(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec, &values[i]);
   }

   int zero_value = 0;
   // Set middle elements to zero (indices 1, 2, 3; exclusive end at 4)
   TEST_ASSERT_TRUE(VectorRangeSetToVal(vec, 1, 4, &zero_value));

   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(50, *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRangeSetToVal_EquivalenceWithRangeClear(void)
{
   struct Vector * vec1 = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   struct Vector * vec2 = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   
   for (size_t i = 0; i < 5; i++) {
      VectorPush(vec1, &values[i]);
      VectorPush(vec2, &values[i]);
   }

   int zero_value = 0;
   // Set range to zero using VectorRangeSetToVal
   VectorRangeSetToVal(vec1, 1, 4, &zero_value);
   // Clear same range using VectorRangeClear
   VectorRangeClear(vec2, 1, 4);

   // Both vectors should be identical
   TEST_ASSERT_TRUE(VectorsAreEqual(vec1, vec2));

   VectorFree(vec1);
   VectorFree(vec2);
}

/*************** Vector Range: Remove Elements in Range ********************/

void test_VectorRange_RemoveElementsInRng_Normal(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50, 60};
   VectorRangePush(vec, values, 6);

   // Remove indices 2, 3, 4 (exclusive end at 5)
   TEST_ASSERT_TRUE(VectorRangeRemove(vec, 2, 5, NULL));
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(60, *(int *)VectorGet(vec, 2));

   VectorFree(vec);
}

void test_VectorRange_RemoveElementsInRng_NormalWithBuf(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50, 60};
   VectorRangePush(vec, values, 6);

   int buf[3] = {0};
   // Remove indices 2, 3, 4 (exclusive end at 5)
   TEST_ASSERT_TRUE(VectorRangeRemove(vec, 2, 5, buf));
   TEST_ASSERT_EQUAL_INT(30, buf[0]);
   TEST_ASSERT_EQUAL_INT(40, buf[1]);
   TEST_ASSERT_EQUAL_INT(50, buf[2]);
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(60, *(int *)VectorGet(vec, 2));

   VectorFree(vec);
}

void test_VectorRange_RemoveElementsInRng_AllElements(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   VectorRangePush(vec, values, 3);
   struct Vector * vec_dup = VectorDuplicate(vec);

   int buf[3] = {0};
   // Remove all elements (exclusive end at 3)
   TEST_ASSERT_TRUE(VectorRangeRemove(vec, 0, 3, buf));
   TEST_ASSERT_EQUAL_INT(10, buf[0]);
   TEST_ASSERT_EQUAL_INT(20, buf[1]);
   TEST_ASSERT_EQUAL_INT(30, buf[2]);
   TEST_ASSERT_EQUAL_size_t(0, VectorLength(vec));
   TEST_ASSERT_TRUE(VectorIsEmpty(vec));
   VectorReset(vec_dup);
   TEST_ASSERT_TRUE(VectorsAreEqual(vec, vec_dup));

   VectorFree(vec);
}

void test_VectorRange_RemoveElementsInRng_EmptyVec(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   // Try to remove from empty vector
   TEST_ASSERT_FALSE(VectorRangeRemove(vec, 0, 1, NULL));
   TEST_ASSERT_TRUE(VectorIsEmpty(vec));

   VectorFree(vec);
}

void test_VectorRange_RemoveElementsInRng_AtBeginning(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   VectorRangePush(vec, values, 5);

   // Remove first two elements (indices 0, 1; exclusive end at 2)
   TEST_ASSERT_TRUE(VectorRangeRemove(vec, 0, 2, NULL));
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(40, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(50, *(int *)VectorGet(vec, 2));

   VectorFree(vec);
}

void test_VectorRange_RemoveElementsInRng_AtEnd(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   VectorRangePush(vec, values, 5);

   // Remove last two elements (indices 3, 4; exclusive end at 5)
   TEST_ASSERT_TRUE(VectorRangeRemove(vec, 3, 5, NULL));
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(vec, 2));

   VectorFree(vec);
}

void test_VectorRange_RemoveElementsInRng_InvalidIndices(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   VectorRangePush(vec, values, 3);

   // idx_start > idx_end
   TEST_ASSERT_FALSE(VectorRangeRemove(vec, 2, 1, NULL));
   // idx_end > len
   TEST_ASSERT_FALSE(VectorRangeRemove(vec, 1, 4, NULL));
   // idx_start >= len
   TEST_ASSERT_FALSE(VectorRangeRemove(vec, 3, 3, NULL));
   // Large indices
   TEST_ASSERT_FALSE(VectorRangeRemove(vec, (size_t)-1, 1, NULL));
   TEST_ASSERT_FALSE(VectorRangeRemove(vec, 1, (size_t)-1, NULL));
   TEST_ASSERT_FALSE(VectorRangeRemove(vec, (size_t)-1, (size_t)-1, NULL));

   VectorFree(vec);
}

void test_VectorRange_RemoveElementsInRng_InvalidVec(void)
{
   TEST_ASSERT_FALSE(VectorRangeRemove(NULL, 0, 1, NULL));
}

void test_VectorRange_RoundTrip_InsertAndRemove(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int data[] = {1, 2, 3, 4, 5};
   VectorRangePush(vec, data, 5);
   struct Vector * vec_dup = VectorDuplicate(vec);

   // Insert some elements
   int insert[] = {10, 11, 12};
   VectorRangeInsert(vec, 2, insert, 3);
   TEST_ASSERT_FALSE( VectorsAreEqual(vec_dup, vec) );

   // Now remove the inserted elements (exclusive end at 5)
   int buf[3] = {0};
   TEST_ASSERT_TRUE(VectorRangeRemove(vec, 2, 5, buf));
   TEST_ASSERT_EQUAL_INT(10, buf[0]);
   TEST_ASSERT_EQUAL_INT(11, buf[1]);
   TEST_ASSERT_EQUAL_INT(12, buf[2]);
   // Vector should be back to original
   TEST_ASSERT_EQUAL_size_t(5, VectorLength(vec));
   TEST_ASSERT_TRUE( VectorsAreEqual(vec_dup, vec) );

   VectorFree(vec);
   VectorFree(vec_dup);
}

void test_VectorRangeRemove_OneElm(void)
{
   struct Vector * v = VectorNew(sizeof(int), 5, 10, NULL, 0, NULL);
   (void)VectorPush(v, &(int){1});
   TEST_ASSERT_FALSE(VectorIsEmpty(v));
   int val_ins; (void)VectorRemove(v, 0, &val_ins);
   TEST_ASSERT_TRUE(VectorIsEmpty(v));
   (void)VectorPush(v, &(int){1});
   TEST_ASSERT_FALSE(VectorIsEmpty(v));
   int val_insrng; (void)VectorRangeRemove(v, 0, 1, &val_insrng);
   TEST_ASSERT_EQUAL_INT(val_ins, val_insrng);
   TEST_ASSERT_TRUE(VectorIsEmpty(v));
   VectorFree(v);
}

/*************** Vector Range: Clear Elements in Range *********************/

void test_VectorRange_ClearElementsInRng_Normal(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50, 60};
   VectorRangePush(vec, values, 6);

   // Clear elements at indices 2, 3, 4 (exclusive end at 5)
   TEST_ASSERT_TRUE(VectorRangeClear(vec, 2, 5));
   TEST_ASSERT_EQUAL_size_t(6, VectorLength(vec));
   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 4));
   TEST_ASSERT_EQUAL_INT(60, *(int *)VectorGet(vec, 5));

   VectorFree(vec);
}

void test_VectorRange_ClearElementsInRng_AllElements(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   VectorRangePush(vec, values, 3);

   // Clear all elements (exclusive end at 3)
   TEST_ASSERT_TRUE(VectorRangeClear(vec, 0, 3));
   for (size_t i = 0; i < 3; i++) {
      TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, i));
   }
   TEST_ASSERT_EQUAL_size_t(3, VectorLength(vec));

   VectorFree(vec);
}

void test_VectorRange_ClearElementsInRng_EmptyVec(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);

   // Try to clear on empty vector
   TEST_ASSERT_FALSE(VectorRangeClear(vec, 0, 1));
   TEST_ASSERT_TRUE(VectorIsEmpty(vec));

   VectorFree(vec);
}

void test_VectorRange_ClearElementsInRng_AtBeginning(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   VectorRangePush(vec, values, 5);

   // Clear first two elements (indices 0, 1; exclusive end at 2)
   TEST_ASSERT_TRUE(VectorRangeClear(vec, 0, 2));
   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(40, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(50, *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRange_ClearElementsInRng_AtEnd(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30, 40, 50};
   VectorRangePush(vec, values, 5);

   // Clear last two elements (indices 3, 4; exclusive end at 5)
   TEST_ASSERT_TRUE(VectorRangeClear(vec, 3, 5));
   TEST_ASSERT_EQUAL_INT(10, *(int *)VectorGet(vec, 0));
   TEST_ASSERT_EQUAL_INT(20, *(int *)VectorGet(vec, 1));
   TEST_ASSERT_EQUAL_INT(30, *(int *)VectorGet(vec, 2));
   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 3));
   TEST_ASSERT_EQUAL_INT(0, *(int *)VectorGet(vec, 4));

   VectorFree(vec);
}

void test_VectorRange_ClearElementsInRng_InvalidIndices(void)
{
   struct Vector * vec = VectorNew(sizeof(int), 10, 100, NULL, 0, &DEFAULT_ALLOCATOR);
   int values[] = {10, 20, 30};
   VectorRangePush(vec, values, 3);

   // idx_start > idx_end
   TEST_ASSERT_FALSE(VectorRangeClear(vec, 2, 1));
   // idx_end > len
   TEST_ASSERT_FALSE(VectorRangeClear(vec, 1, 4));
   // idx_start >= len
   TEST_ASSERT_FALSE(VectorRangeClear(vec, 3, 3));
   // Large indices
   TEST_ASSERT_FALSE(VectorRangeClear(vec, (size_t)-1, 1));
   TEST_ASSERT_FALSE(VectorRangeClear(vec, 1, (size_t)-1));
   TEST_ASSERT_FALSE(VectorRangeClear(vec, (size_t)-1, (size_t)-1));

   VectorFree(vec);
}

void test_VectorRange_ClearElementsInRng_InvalidVec(void)
{
   TEST_ASSERT_FALSE(VectorRangeClear(NULL, 0, 1));
}

/********************************** VIterator *********************************/

//void test_VIterator_BasicRead_FullVec(void)
//{
//   struct Vector * v = VectorNew(sizeof(int), 5, 10, (int[]){1, 2, 3, 4, 5}, 5, NULL);
//   int i = 1;
//   FOREACH_VEC_READ(int, val, v,
//      TEST_ASSERT_EQUAL_INT(i++, val);
//   );
//   TEST_ASSERT_EQUAL_INT(6, i);
//   VectorFree(v);
//}
//
//void test_VIterator_BasicUpdate_FullVec(void)
//{
//   struct Vector * v = VectorNew(sizeof(int), 5, 10, (int[]){1, 2, 3, 4, 5}, 5, NULL);
//   FOREACH_VEC_REF( int*, valptr, v,
//      (*valptr)++;
//   );
//   int i = 2;
//   FOREACH_VEC_READ( int, val, v,
//      TEST_ASSERT_EQUAL_INT(i++, val);
//   );
//   TEST_ASSERT_EQUAL_INT(7, i); // Confirm that we really did iterate through all elements
//   VectorFree(v);
//}
//
//void test_VIterator_BasicRead_FullVec_Reverse(void)
//{
//   struct Vector * v = VectorNew(sizeof(int), 5, 10, (int[]){1, 2, 3, 4, 5}, 5, NULL);
//   int i = 5;
//   FOREACH_VEC_READ_REVERSE(int, val, v,
//      TEST_ASSERT_EQUAL_INT(i--, val);
//   );
//   TEST_ASSERT_EQUAL_INT(0, i);
//   VectorFree(v);
//}
//
//void test_VIterator_BasicUpdate_FullVec_Reverse(void)
//{
//   struct Vector * v = VectorNew(sizeof(int), 5, 10, (int[]){1, 2, 3, 4, 5}, 5, NULL);
//   int i = 0;
//   FOREACH_VEC_REF_REVERSE( int*, valptr, v,
//      *valptr += i++;
//   );
//   FOREACH_VEC_READ_REVERSE( int, val, v,
//      TEST_ASSERT_EQUAL_INT(5, val);
//   );
//   VectorFree(v);
//}
//
//void test_VIterator_BasicRead_SubRng_Normal(void)
//{
//   struct Vector * v = VectorNew(sizeof(int), 10, 20, (int[]){1,2,3,4,5,6,7,8,9,10}, 10, NULL);
//   int i = 3;
//   FOREACH_VEC_READ_RNG( int, val, v, 2, 5, IterDir_Normal,
//      TEST_ASSERT_EQUAL_INT(i++, val);
//   );
//   TEST_ASSERT_EQUAL_INT(6, i);
//   VectorFree(v);
//}
//
//void test_VIterator_BasicUpdate_SubRng_Normal(void)
//{
//   struct Vector * v = VectorNew(sizeof(int), 10, 20, (int[]){1,2,3,4,5,6,7,8,9,10}, 10, NULL);
//   FOREACH_VEC_REF_RNG( int*, ptr, v, 2, 5, IterDir_Normal,
//      (*ptr)++;
//   );
//   int i = 1;
//   FOREACH_VEC_READ( int, val, v,
//      static bool first_time = true;
//      if ( i == 3 )
//         i++;
//      else if ( i == 7 && first_time )
//      {
//         i--;
//         first_time = false;
//      }
//      TEST_ASSERT_EQUAL_INT(i++, val);
//   );
//   TEST_ASSERT_EQUAL_INT(11, i);
//   VectorFree(v);
//
//}
//
//void test_VIterator_BasicRead_SubRng_Reverse(void)
//{
//   struct Vector * v = VectorNew(sizeof(int), 5, 10, (int[]){1,2,3,4,5,6,7}, 5, NULL);
//   int i = 4;
//   FOREACH_VEC_READ_RNG(int, val, v, 3, 0, IterDir_Reverse,
//      TEST_ASSERT_EQUAL_INT(i--, val);
//   );
//   TEST_ASSERT_EQUAL_INT(1, i);
//   VectorFree(v);
//}
//
//void test_VIterator_BasicUpdate_SubRng_Reverse(void)
//{
//   struct Vector * v = VectorNew(sizeof(int), 5, 10, (int[]){1,2,3,4,5,6,7}, 5, NULL);
//   FOREACH_VEC_REF_RNG(int*, ptr, v, 3, 0, IterDir_Reverse,
//      (*ptr)++;
//   );
//   int i = 5;
//   FOREACH_VEC_READ_RNG(int, val, v, 3, 0, IterDir_Reverse,
//      TEST_ASSERT_EQUAL_INT(i--, val);
//   );
//   TEST_ASSERT_EQUAL_INT(2, i);
//   VectorFree(v);
//}
//
//void test_VIterator_BasicRead_SubRng_NormalWithWrap(void)
//{
////   struct Vector * v = VectorNew(sizeof(int), 5, 10, (int[]){1,2,3,4,5}, 5, NULL);
////   FOREACH_VEC_READ_RNG(int, val, v, 3, 3, IterDir_RightWrap,
////      
//}
//
//void test_VIterator_BasicUpdate_SubRng_NormalWithWrap(void)
//{
//   TEST_ASSERT_TRUE(false);
//}
//
//void test_VIterator_BasicRead_SubRng_ReverseWithWrap(void)
//{
//   TEST_ASSERT_TRUE(false);
//}
//
//void test_VIterator_BasicUpdate_SubRng_ReverseWithWrap(void)
//{
//   TEST_ASSERT_TRUE(false);
//}

/************************* Local Function Definitions *************************/

void * test_nil_alloc(size_t req_sz, void * ctx)
{
   (void)req_sz;
   (void)ctx;
   return NULL;
}

void * test_nil_realloc(void * old_ptr, size_t new_sz, size_t old_sz, void * ctx)
{
   (void)old_ptr;
   (void)new_sz;
   (void)old_sz;
   (void)ctx;
   return NULL;
}

void test_nil_reclaim(void * old_ptr, size_t old_sz, void * ctx)
{
   (void)old_ptr;
   (void)old_sz;
   (void)ctx;
}
