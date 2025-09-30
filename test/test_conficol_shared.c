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
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include <time.h>
#include <unity/unity.h>
#include <unity/unity_memory.h>

#include "conficol_shared.h"

/* Local Macro Definitions */

/* Datatypes */

/* Local Variables */

/* Forward Function Declarations */

void setUp(void);
void tearDown(void);

void test_memset_scramble_SmallBuf(void);
void test_memset_scramble_LargeBuf(void);
// Some buffers that don't necessary fit perfectly within 4- or 8-byte boundaries
void test_memset_scramble_BufLen1(void);
void test_memset_scramble_BufLen5(void);
void test_memset_scramble_BufLen71(void);
void test_memset_scramble_BufLen800(void);
void test_memset_scramble_SingleByte(void);
void test_memset_scramble_DifferentDataTypes(void);
void test_memset_scramble_EntropyDistribution(void);
void test_memset_scramble_MinimalConsecutiveVals(void);
void test_memset_scramble_FullByteRange(void);

/* Helper Functions */
static void check_data_scrambled(uint8_t * buf, size_t nbytes, uint8_t og_val);

/* Meat of the Program */

int main(void)
{
   UNITY_BEGIN();

   RUN_TEST(test_memset_scramble_SmallBuf);
   RUN_TEST(test_memset_scramble_LargeBuf);
   RUN_TEST(test_memset_scramble_BufLen1);
   RUN_TEST(test_memset_scramble_BufLen5);
   RUN_TEST(test_memset_scramble_BufLen71);
   RUN_TEST(test_memset_scramble_BufLen800);
   RUN_TEST(test_memset_scramble_SingleByte);
   RUN_TEST(test_memset_scramble_DifferentDataTypes);
   RUN_TEST(test_memset_scramble_EntropyDistribution);
   RUN_TEST(test_memset_scramble_MinimalConsecutiveVals);
   RUN_TEST(test_memset_scramble_FullByteRange);

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

/******************************* memset_scramble ******************************/

void test_memset_scramble_SmallBuf(void)
{
   uint8_t buf[16];
   memset(buf, 0xAA, sizeof(buf));  // Initialize with pattern

   memset_scramble(buf, sizeof(buf));

   check_data_scrambled(buf, 16, 0xAA);
}

void test_memset_scramble_LargeBuf(void)
{
   const size_t buf_size = 1024;
   uint8_t * buf = malloc(buf_size);
   TEST_ASSERT_NOT_NULL(buf);

   memset(buf, 0x00, buf_size);  // Initialize with zeros
   memset_scramble(buf, buf_size);

   // Frequency counter of byte values in buf
   unsigned int byte_counts[256] = {0};
   for (size_t i = 0; i < buf_size; ++i)
      byte_counts[buf[i]]++;

   // Should have variety in byte values (not all same)
   size_t unique_bytes = 0;
   for (int i = 0; i < 256; ++i)
   {
      if (byte_counts[i] > 0)
         unique_bytes++;
   }

   // For 1024 bytes, expect at least 50 different byte values. Why 50? Well,
   // ideally, you'd have close to 256 unique byte values, but I wouldn't make
   // that a hard requirement; I'm personally happy with 50 unique values, as
   // the goal is scrambling and obscuring the original data.
   // TODO: Come up with a more mathetatically rigorous expectation for uniqueness.
   TEST_ASSERT_GREATER_THAN(50, unique_bytes);

   free(buf);
}

void test_memset_scramble_BufLen1(void)
{
   uint8_t buf[1] = {0};
   memset_scramble(buf, 1);
   TEST_ASSERT_NOT_EQUAL_UINT8(0, buf[0]);
}

void test_memset_scramble_BufLen5(void)
{
   uint8_t buf[5];
   memset(buf, 0xAA, sizeof(buf));  // Initialize with pattern

   memset_scramble(buf, sizeof(buf));

   check_data_scrambled(buf, 5, 0xAA);
}

void test_memset_scramble_BufLen71(void)
{
   uint8_t buf[71] = {0};
   memset(buf, 0xAA, sizeof(buf));  // Initialize with pattern

   memset_scramble(buf, 71);

   check_data_scrambled(buf, 71, 0xAA);
}

void test_memset_scramble_BufLen800(void)
{
   uint8_t buf[800] = {0};
   memset(buf, 0xAA, sizeof(buf));  // Initialize with pattern

   memset_scramble(buf, 800);

   check_data_scrambled(buf, 800, 0xAA);
}

void test_memset_scramble_SingleByte(void)
{
   uint8_t byte = 0x42;  // Set to known value
   memset_scramble(&byte, 1);
   
   // Single byte should be scrambled to something different
   TEST_ASSERT_NOT_EQUAL_UINT8(0x42, byte);
   TEST_ASSERT_NOT_EQUAL_UINT8(0x00, byte);
}

void test_memset_scramble_DifferentDataTypes(void)
{
   // Test with different data type structures
   struct test_struct {
      uint16_t a;
      uint32_t b;
      uint64_t c;
      uint8_t  d;
   } data = {0x1234, 0x56789ABC, 0xDEADBEEFCAFEBABE, 0x42};

   struct test_struct original = data;
   memset_scramble(&data, sizeof(data));

   // Check that the data changed
   TEST_ASSERT_FALSE(memcmp(&data, &original, sizeof(data)) == 0);
}

void test_memset_scramble_EntropyDistribution(void)
{
   // Test that scrambling produces reasonable entropy
   const size_t buf_size = 256;
   uint8_t buf[buf_size];

   memset(buf, 0x00, sizeof(buf));
   memset_scramble(buf, sizeof(buf));

   // Count 0s and 1s in all bits
   struct { size_t n_zeros; size_t n_ones; } bit_counts = {0};

   for ( size_t i = 0; i < buf_size; ++i )
   {
      for ( uint8_t bit = 0; bit < 8; ++bit )
      {
         if (buf[i] & (1 << bit))
            bit_counts.n_ones++;
         else
            bit_counts.n_zeros++;
      }
   }

   // Expect roughly balanced distribution (within 20% of perfect balance)
   const size_t total_bits = buf_size * 8;
   float ratio = (double)bit_counts.n_ones / total_bits;
   TEST_ASSERT_GREATER_THAN_FLOAT(0.3f, ratio);  // At least 30% ones
   TEST_ASSERT_LESS_THAN_FLOAT   (0.7f, ratio);  // At most  70% ones
}

void test_memset_scramble_MinimalConsecutiveVals(void)
{
   // Test that consecutive bytes don't show obvious patterns
   uint8_t buf[64];
   memset(buf, 0xCC, sizeof(buf));
   memset_scramble(buf, sizeof(buf));

   // Check for repetitive patterns (simple run-length check)
   int max_consecutive = 1;
   int current_run = 1;

   for ( size_t i = 1; i < sizeof(buf); ++i )
   {
      if ( buf[i] == buf[i-1] )
         current_run++;
      else {
         if ( current_run > max_consecutive )
            max_consecutive = current_run;
         current_run = 1;
      }
   }

   // Within a buffer size of 64 bytes, I wouldn't expect more than 9 consecutive
   // bytes. 9 is 64 bits + 1 byte, and if the scrambling attempts to take advantage
   // of 64-bit operations and values, we might get at least some 8-byte sequences
   // in there of identical consecutive bytes (e.g., 0UL).
   TEST_ASSERT_LESS_THAN(9, max_consecutive);
}

void test_memset_scramble_FullByteRange(void)
{
   // Test that scrambling can produce the full range of byte values
   const size_t iterations = 100;
   const size_t buf_size = 256;
   bool seen_values[256] = {false};

   for ( size_t iter = 0; iter < iterations; ++iter )
   {
      uint8_t buf[buf_size];

      // Try different initial values in buf
      memset(buf, (uint8_t)iter, sizeof(buf));
      memset_scramble(buf, sizeof(buf));

      // Mark which byte values we've seen
      for ( size_t i = 0; i < buf_size; ++i )
         seen_values[buf[i]] = true;
   }

   // Count how many different byte values we've observed
   size_t unique_values = 0;
   for (int i = 0; i < 256; ++i)
   {
      if (seen_values[i])
         unique_values++;
   }

   // I'd expect to see all 256 possible byte values at this point
   TEST_ASSERT_EQUAL_size_t(256, unique_values);
}

/*************************** Local Helper Functions ***************************/

static void check_data_scrambled(uint8_t * buf, size_t nbytes, uint8_t og_val)
{
   // Check that not all bytes are the same
   bool all_same = true;
   uint8_t first_byte = buf[0];
   for (size_t i = 1; i < nbytes; ++i)
   {
      if (buf[i] != first_byte)
      {
         all_same = false;
         break;
      }
   }

   TEST_ASSERT_FALSE(all_same);

   // Check that at least some bytes changed from original pattern
   bool any_changed = false;
   for (size_t i = 0; i < nbytes; ++i) {
      if (buf[i] != og_val) {
         any_changed = true;
         break;
      }
   }

   TEST_ASSERT_TRUE(any_changed);
}
