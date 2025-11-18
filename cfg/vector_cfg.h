/**
 * @file vector_cfg.h
 * @brief Configuration of aspects of the vector implementation.
 *
 * @author Abdulla Almosalami (memphis242)
 * @date June 10, 2025
 * @copyright MIT License
 */

/* File Inclusions */
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* Public Macro Definitions */

//! Uncomment to set a maximum vector length for any vector
#ifndef MAX_VEC_LEN // Define at compile-command time if desired
#define MAX_VEC_LEN 1000
#endif // MAX_VEC_LEN

//! Specify the sizes of the fixed-size vector object pools, which is internally used to deploy vector/vector iterator objects
#define VEC_STRUCT_POOL_SIZE       (25)
#define VITERATOR_STRUCT_POOL_SIZE (VEC_STRUCT_POOL_SIZE * 2)

