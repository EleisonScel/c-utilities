/* Copyright 2026 EleisonNox
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
/* File:
 * aligned_memory.h
 *
 * Description:
 * Portable aligned memory allocation, reallocation and free
 */

#ifndef ALIGNED_MEMORY_H
#define ALIGNED_MEMORY_H

#include <stddef.h> /* size_t */

/* Logic to choose the alignment backend
 *
 * Available backends and its requirements (in order of preferences unless forces):
 *	- AM_BACKEND_WINDOWS	: alignment must be a power of two,
 *							. _WIN32,
 *							. C99
 *	- AM_BACKEND_POSIX		: alignment must be a power of two and a multiple of a pointer size,
 *							. _POSIX_C_SOURCE >= 200112L
 *							. C99
 *	- AM_BACKEND_STANDARD	: alignment must be a power of two,
 *							. C11,
 *							. non-Windows system
 *	- AM_BACKEND_FALLBACK	:
 *		- default (AM_ALIGN_BITWISE = 1)					:
 *								: pointer size and alignment must be a power of two
 *		- with AM_ALLOW_NON_POWER_OF_TWO_ALIGNMENT defined	:
 *								: any non-zero alignment allowed
 *									(uses a slower division-based method)
 *							. uintptr_t
 *							. C99
 *
 * Available user macro definition before including the header:
 *	- AM_FORCE_WINDOWS						: force Windows backend
 *	- AM_FORCE_POSIX						: force POSIX backend
 *	- AM_FORCE_C11							: force C11 backend
 *	- AM_FORCE_FALLBACK						: force fallback backend
 *	- AM_ALLOW_NON_POWER_OF_TWO_ALIGNMENT	: allow non-power-of-two alignment in
 *												(AM_BACKEND_FALLBACK only)
 */

/* Function:
 * release a raw memory block allocated with an am_aligned_malloc
 *
 * Parameters:
 * pointer	- pointer to memory block previously allocated with an am_aligned_malloc
 */
void am_aligned_free( void * restrict pointer );

/* Function:
 * allocate aligned memory block
 *
 * Parameters:
 * alignment				- desired address alignment
 * size						- size of memory block to allocate in bytes
 *
 * Returns:
 * pointer					- block was allocated (must be released with am_aligned_free)
 * NULL						- invalid arguments or allocation failure
 */
void * am_aligned_malloc( size_t alignment, size_t size );
/* Function:
 * reallocate aligned memory block
 *
 * Precondition:
 * pointer					- must be previously allocated with an am_aligned_malloc
 * size_new					- must be positive
 *
 * Parameters:
 * pointer					- existing aligned memory block
 * size_new					- size of desired memory block to reallocate
 *
 * Returns:
 * pointer					- block was reallocated
 * NULL						- invalid arguments or failed to allocate memory block
 */
void * am_aligned_realloc( void * restrict pointer, size_t size_new );
/* Function:
 * allocate zero-initialized aligned memory for an array
 *
 * Parameters:
 * alignment		- desired address alignment
 * elements_amount	- amount of elements
 * element_size		- size of each element
 *
 * Returns:
 * pointer			- zero-filled aligned memory was allocated
 *						(must be released with am_aligned_free)
 * NULL				- invalid arguments, potential overflow or allocation failure
 */
void * am_aligned_calloc( size_t alignment, size_t elements_amount, size_t element_size );

#endif/* ALIGNED_MEMORY_H */
