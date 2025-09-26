/**
 * @file conficol_shared.c
 * @brief Definition of common objects.
 *
 * @author Abdulla Almosalami (memphis242)
 * @date Jul 19, 2025
 * @copyright MIT License
 */

/* File Inclusions */
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "conficol_shared.h"

#include "biski64/c/biski64.inl"

/* Public Function Definitions */

void * default_alloc(size_t req_sz, void * arena)
{
   (void)arena;
   return malloc(req_sz);
}

void * default_realloc(void * old_ptr, size_t new_sz, size_t old_sz, void * arena)
{
   (void)old_sz;
   (void)arena;
   return realloc(old_ptr, new_sz);
}

void default_reclaim(void * old_ptr, size_t old_sz, void * arena)
{
   (void)old_sz;
   (void)arena;
   free(old_ptr);
}

void memset_scramble(void * const ptr, size_t nbytes)
{
   assert(ptr != NULL);
   assert(nbytes > 0);

   // Seed off of the value of the first byte...
   // TODO: Figure out a better seed value... NOTE: gotta be multi-platform,
   // and account for embedded environments - time() and/or clock() isn't guaranteed!
   uint64_t first_byte = 0 | *(uint8_t *)ptr;
   uint64_t seed = first_byte
                   | (~first_byte << 8 )
                   | ( first_byte << 16)
                   | (~first_byte << 24)
                   | ( first_byte << 32)
                   | (~first_byte << 40)
                   | ( first_byte << 48)
                   | (~first_byte << 56);
   biski64_state rng_state;
   biski64_seed(&rng_state, seed);

   uint64_t * u64_ptr = (uint64_t *)ptr;
   for ( size_t i = 0; i < (nbytes / 4); ++i, ++u64_ptr )
      *u64_ptr = biski64_next(&rng_state);

   uint8_t leftover_bytes = nbytes % 4;
   if ( leftover_bytes > 0 )
   {
      uint64_t final_bytes = biski64_next(&rng_state);
      uint8_t * u8_ptr = (uint8_t *)ptr;
      for ( uint8_t i = 0; i < leftover_bytes; ++i )
         *u8_ptr = (uint8_t)((final_bytes >> i) & 0xFFu);
   }

   // TODO: Assert the distribution of the bytes...
}
