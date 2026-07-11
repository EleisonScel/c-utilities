/* Copyright 2026 EleisonScel
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

#include "aligned_memory.h"

#include "assert_m.h"	/*	assert_m	*/

#include <stddef.h>		/*	size_t		*/
#include <stdint.h>		/*	SIZE_MAX	*/
#include <string.h>		/*	memcpy		*/

/* Logic to choose the alignment backend
 *
 * Available backends and its usage (in order of preferences unless forces):
 *	- AM_BACKEND_WINDOWS	: _aligned_offset_malloc, _aligned_offset_realloc, _aligned_free
 *								(requires _WIN32)
 *	- AM_BACKEND_POSIX		: posix_memalign (requires _POSIX_C_SOURCE >= 200112L, C99)
 *	- AM_BACKEND_STANDARD	: aligned_alloc (requires C11, not available on Windows)
 *	- AM_BACKEND_FALLBACK	: malloc with manual alignment calculation (requires C99)
 */
#ifdef AM_FORCE_WINDOWS
#	if !defined _WIN32
#		error "Windows memory alignment solution isn't available, remove AM_FORCE_WINDOWS flag"
#	endif
#	define AM_BACKEND_WINDOWS
#elif defined AM_FORCE_POSIX
#	if !defined _POSIX_C_SOURCE || (_POSIX_C_SOURCE + 0L) < 200112L
#		error "POSIX memory alignment solution isn't available, remove AM_FORCE_POSIX flag"
#	endif
#	define AM_BACKEND_POSIX
#elif defined AM_FORCE_C11
#	if defined _MSC_VER || defined _WIN32 ||\
		!defined __STDC_VERSION__ || (__STDC_VERSION__ + 0L) < 201112L
#		error "Standard C memory alignment solution isn't available, remove AM_FORCE_C11 flag"
#	endif
#	define AM_BACKEND_STANDARD
#elif defined AM_FORCE_FALLBACK
#	define AM_BACKEND_FALLBACK
#elif defined _WIN32
#	define AM_BACKEND_WINDOWS
#elif defined _POSIX_C_SOURCE && (_POSIX_C_SOURCE + 0L) >= 200112L
#	define AM_BACKEND_POSIX
#elif !defined _MSC_VER && defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 201112L
#	define AM_BACKEND_STANDARD
#else
#	define AM_BACKEND_FALLBACK
#endif/* AM_FORCE_POSIX */

/* AM_BACKEND_FALLBACK requires uintptr to perform pointer arithmetic */
#ifdef AM_BACKEND_FALLBACK
#	ifndef UINTPTR_MAX
#		error "No aligned allocation method available"
#	endif /* UINTPTR_MAX */
	/* just an extra check since that's guaranteed */
	static_assert_m(
		sizeof(uintptr_t) >= sizeof(void *),
		"Unsigned integer pointer size must be wide enough to hold the size of a pointer"
	);
#endif /* AM_BACKEND_FALLBACK */

/* corresponding include */
#if defined AM_BACKEND_WINDOWS
#	include <malloc.h>		/*	_aligned_offset_malloc	*/
#else
#	include <stdlib.h>		/*	malloc					*/
#endif /* AM_BACKEND_WINDOWS */

/* Power-of-two pointer size check;
 * disable using AM_ALLOW_NON_POWER_OF_TWO_ALIGNMENT for AM_BACKEND_FALLBACK
 */
#ifdef AM_ALLOW_NON_POWER_OF_TWO_ALIGNMENT
#	if defined AM_BACKEND_STANDARD || defined AM_BACKEND_POSIX || defined AM_BACKEND_WINDOWS
#		error "POSIX and standard C require power of two alignment; try AM_FORCE_FALLBACK"
#	endif
#	define AM_ALIGN_BITWISE 0
#else
	static_assert_m(
		(sizeof(void *) & (sizeof(void *) - (size_t) 1)) == (size_t) 0,
		"The size of a pointer must be a power of two"
	);
#	define AM_ALIGN_BITWISE 1
#endif

/* Metadata structure, holds information needed to free or reallocate,
 * stored right before user-visible pointer
 */
#if defined AM_NO_REALLOC && defined AM_BACKEND_WINDOWS
#	define			AM_HAS_HEADER						0
#	define AM_HEADER_SIZE 0
#else /* AM_NO_REALLOC, AM_BACKEND_WINDOWS */
struct AM_Alignment_Header_Info {
#	ifndef AM_BACKEND_WINDOWS
#		if !defined AM_BACKEND_STANDARD && !defined AM_BACKEND_POSIX || defined AM_NO_REALLOC
	void	* raw_memory_pointer;		/* pointer to release by corresponding free function*/
#			define	AM_HAS_RAW_MEMORY_POINTER			1
#		else
#			define	AM_HAS_RAW_MEMORY_POINTER			0
#		endif /* AM_BACKEND_STANDARD, AM_BACKEND_POSIX, AM_NO_REALLOC */
#		ifndef AM_NO_REALLOC
	size_t	user_memory_allocated_size;	/* size requested by the caller						*/
#			define	AM_HAS_USER_MEMORY_ALLOCATED_SIZE	1
#		else
#			define	AM_HAS_USER_MEMORY_ALLOCATED_SIZE	0
#		endif /* AM_NO_REALLOC */
#	else /* AM_BACKEND_WINDOWS */
#		define		AM_HAS_RAW_MEMORY_POINTER			0
#		define		AM_HAS_USER_MEMORY_ALLOCATED_SIZE	0
#	endif /* AM_BACKEND_WINDOWS */
#	ifndef AM_NO_REALLOC
	size_t	alignment;					/* requested alignment								*/
#		define		AM_HAS_ALIGNMENT					1
#	else
#		define		AM_HAS_ALIGNMENT					0
#	endif /* AM_NO_REALLOC */
};
#	define			AM_HAS_HEADER						1
#	define AM_HEADER_SIZE sizeof(struct AM_Alignment_Header_Info)
#endif /* AM_NO_REALLOC, AM_BACKEND_WINDOWS */

static void * am_aligned_allocation(
	size_t alignment, size_t size, void ** restrict out_raw_memory_pointer
);

static inline size_t am_check_array_bounds( size_t elements_amount, size_t element_size );

void * am_aligned_malloc( size_t alignment, size_t size ) {
	assert_m(size		!= 0, "Amount of bytes for allocation shouldn't be zero");
	assert_m(alignment	!= 0, "Alignment shouldn't be zero, it's a divider"		);

	if ( alignment == 0 || size == 0 )
		return NULL;

#if defined AM_BACKEND_STANDARD || defined AM_BACKEND_POSIX || defined AM_BACKEND_WINDOWS ||\
	(defined AM_BACKEND_FALLBACK && AM_ALIGN_BITWISE == 1)
	assert_mf(
		(alignment & (alignment - 1)) == 0,
		"Alignment must be a power of two for this backend (alignment: %zu)", alignment
	);
	if ( (alignment & (alignment - 1)) != 0 )
		return NULL;
#	ifdef AM_BACKEND_POSIX
	assert_mf(
		alignment % sizeof(void *) == 0,
		"Alignment must be a multiple of size of pointer (alignment: %zu)", alignment
	);
	if ( alignment % sizeof(void *) != 0 )
		return NULL;
#	endif /* AM_BACKEND_POSIX */
#endif /* AM_BACKEND_STANDARD AM_BACKEND_POSIX AM_BACKEND_WINDOWS ... */

#ifdef AM_BACKEND_WINDOWS
	assert_mf(
		size <= SIZE_MAX - AM_HEADER_SIZE,
		"Size of block for allocation shouldn't be that big (size: %zu)", size
	);
	if ( size > SIZE_MAX - AM_HEADER_SIZE )
		return NULL;

#elif defined AM_BACKEND_STANDARD || defined AM_BACKEND_POSIX

	const size_t header_information_size_aligned =
		(AM_HEADER_SIZE + alignment - 1) & ~(size_t)(alignment - 1);

	assert_mf(
		size <= SIZE_MAX - header_information_size_aligned,
		"Allocation size must leave space for the information (size: %zu, information: %zu)",
		size, header_information_size_aligned
	);
	if ( size > SIZE_MAX - header_information_size_aligned )
		return NULL;
	assert_mf(
		size + header_information_size_aligned <= SIZE_MAX - (alignment - 1),
		"Allocation alignment isn't valid for this size (size: %zu, alignment: %zu)",
		size + header_information_size_aligned, alignment
	);
	if ( size + header_information_size_aligned > SIZE_MAX - (alignment - 1) )
		return NULL;

#else /* FALLBACK */

	assert_mf(
		alignment <= SIZE_MAX - AM_HEADER_SIZE + 1,
		"Allocation alignment must leave space for the information "
		"(alignment: %zu , information: %zu)", alignment, AM_HEADER_SIZE
	);
	if ( alignment > SIZE_MAX - AM_HEADER_SIZE + 1 )
		return NULL;

	size_t header_information_size_required = alignment - 1 + AM_HEADER_SIZE;
	assert_mf(
		header_information_size_required <= SIZE_MAX - size,
		"Allocation size must leave space for the required overhead (size: %zu, overhead: %zu)",
		size, header_information_size_required
	);
	if ( header_information_size_required > SIZE_MAX - size )
		return NULL;

#endif /* AM_BACKEND_WINDOWS */

	void * raw_memory_pointer = NULL;
	void * result_pointer = am_aligned_allocation( alignment, size, &raw_memory_pointer );
	if ( result_pointer == NULL )
		return NULL;
	assert_m(
		raw_memory_pointer != NULL,
		"If there is a result pointer there must be and a raw pointer"
	);

#if AM_HAS_HEADER == 1

struct AM_Alignment_Header_Info header = {
#	if AM_HAS_RAW_MEMORY_POINTER			== 1
	.raw_memory_pointer			= raw_memory_pointer,
#	endif /* AM_HAS_RAW_MEMORY_POINTER */
#	if AM_HAS_USER_MEMORY_ALLOCATED_SIZE	== 1
	.user_memory_allocated_size	= size,
#	endif /* AM_HAS_USER_MEMORY_ALLOCATED_SIZE */
#	if AM_HAS_ALIGNMENT						== 1
	.alignment					= alignment
#	endif /* AM_HAS_ALIGNMENT */
};
	memcpy(
		(char *) result_pointer - AM_HEADER_SIZE,
		&header,
		AM_HEADER_SIZE
	);

#endif /* AM_HAS_HEADER == 1 */

#	ifdef UINTPTR_MAX
	assert_m(
		( (uintptr_t)result_pointer % alignment) == 0,
		"Computed aligned address is not aligned"
	);
#	endif /* UINTPTR_MAX */
	return result_pointer;
}

void * am_aligned_malloc_array( size_t alignment, size_t elements_amount, size_t element_size ) {
	size_t total_bytes = am_check_array_bounds( elements_amount, element_size );
	if ( total_bytes == 0 )
		return NULL;
	return am_aligned_malloc( alignment, total_bytes );
}

#ifndef AM_NO_CALLOC

void * am_aligned_calloc( size_t alignment, size_t elements_amount, size_t element_size ) {
	size_t total_bytes = am_check_array_bounds( elements_amount, element_size );
	if ( total_bytes == 0 )
		return NULL;

	void * result_pointer = am_aligned_malloc( alignment, total_bytes );
	if ( result_pointer == NULL )
		return NULL;

	memset( result_pointer, 0, total_bytes );
	return result_pointer;
}

#endif /* AM_NO_CALLOC */

#ifndef AM_NO_REALLOC

void * am_aligned_realloc_array(
		void * restrict pointer, size_t elements_amount, size_t element_size
	)
{
	size_t total_bytes = am_check_array_bounds( elements_amount, element_size );
	if ( total_bytes == 0 )
		return NULL;

	return am_aligned_realloc( pointer, total_bytes );
}

void * am_aligned_realloc( void * restrict pointer, size_t size_new ) {
	assert_m( pointer	!= NULL,	"No memory block found for reallocation"		);
	assert_m( size_new	!= 0,		"Reallocation to a zero size is not available"	);

	if ( pointer == NULL || size_new == 0 ) return NULL;

	struct AM_Alignment_Header_Info header_old;
	memcpy(
		&header_old,
		(char *)pointer - AM_HEADER_SIZE,
		AM_HEADER_SIZE
	);

	const size_t alignment = header_old.alignment;
#	if defined AM_BACKEND_WINDOWS

	void * raw_memory_pointer_old = (char *)pointer - AM_HEADER_SIZE;
	assert_mf(
		size_new <= SIZE_MAX - AM_HEADER_SIZE,
		"Size of block for reallocation shouldn't be that big (size: %zu)", size_new
	);
	if ( size_new > SIZE_MAX - AM_HEADER_SIZE )
		return NULL;

	void * raw_memory_pointer_new = _aligned_offset_realloc(
		raw_memory_pointer_old,
		size_new + AM_HEADER_SIZE,
		alignment,
		AM_HEADER_SIZE
	);
	if ( raw_memory_pointer_new == NULL ) return NULL;
	struct AM_Alignment_Header_Info header_new = { alignment };
	memcpy(
		raw_memory_pointer_new,
		&header_new,
		AM_HEADER_SIZE
	);

	return (void *) ((char *) raw_memory_pointer_new + AM_HEADER_SIZE);

#	else /* AM_BACKEND_WINDOWS */

	const size_t size_old = header_old.user_memory_allocated_size;

#		if defined AM_BACKEND_STANDARD || defined AM_BACKEND_POSIX

	const size_t header_information_size_aligned =
		(AM_HEADER_SIZE + alignment - 1) & ~(size_t)(alignment - 1);

	assert_mf(
		size_new <= SIZE_MAX - header_information_size_aligned,
		"Allocation size must leave space for the information (size: %zu, information: %zu)",
		size_new, header_information_size_aligned
	);
	if ( size_new > SIZE_MAX - header_information_size_aligned )
		return NULL;
	assert_mf(
		size_new + header_information_size_aligned <= SIZE_MAX - (alignment - 1),
		"Allocation alignment is not valid for this size "
		"(size to allocate: %zu, alignment: %zu)", size_new + header_information_size_aligned,
		alignment
	);
	if ( size_new + header_information_size_aligned > SIZE_MAX - (alignment - 1) )
		return NULL;

	void * raw_memory_pointer = NULL;
	void * new_pointer = am_aligned_allocation( alignment, size_new, &raw_memory_pointer );
	if ( new_pointer == NULL ) return NULL;
	assert_m(
		raw_memory_pointer != NULL,
		"If there is a result pointer there must be and a raw pointer"
	);

	struct AM_Alignment_Header_Info header_new = {
		.alignment					= alignment,
		.user_memory_allocated_size	= size_new
	};
	memcpy(
		(char *) new_pointer - AM_HEADER_SIZE,
		&header_new,
		AM_HEADER_SIZE
	);

	memcpy(
		new_pointer,
		pointer,
		(size_old > size_new ? size_new : size_old)
	);

	am_aligned_free( pointer );

	return new_pointer;

#		elif defined AM_BACKEND_FALLBACK

	assert_m(
		alignment <= SIZE_MAX - AM_HEADER_SIZE + 1,
		"Alignment does not leave enough space for the size and its information"
	);
	if ( alignment > SIZE_MAX - AM_HEADER_SIZE + 1 )
		return NULL;
	size_t size_to_allocate = alignment - 1 + AM_HEADER_SIZE;

	assert_mf(
		size_new <= SIZE_MAX - size_to_allocate,
		"Lack of the space for allocation "
		"(size: %zu, real size: %zu)", size_new, size_to_allocate
	);
	if ( size_new > SIZE_MAX - size_to_allocate )
		return NULL;
	size_to_allocate += size_new;

	void * raw_memory_pointer_old = header_old.raw_memory_pointer;
	ptrdiff_t offset_old = (char *) pointer - (char *)raw_memory_pointer_old;

	void * raw_memory_pointer_new = realloc( raw_memory_pointer_old, size_to_allocate );
	if ( raw_memory_pointer_new == NULL ) return NULL;

	uintptr_t address = (uintptr_t) raw_memory_pointer_new + (uintptr_t) AM_HEADER_SIZE;
	uintptr_t aligned = 0;

#			if AM_ALIGN_BITWISE == 1
	aligned = (address + alignment - 1) & ~(uintptr_t)(alignment - 1);
#			else /* AM_ALIGN_BITWISE == 1 */
	size_t remainder = address % alignment;
	aligned = (remainder != 0)
		? address + alignment - remainder
		: address;
#			endif /* AM_ALIGN_BITWISE */

	void * data_source_pointer =
		(void *)( (uintptr_t) raw_memory_pointer_new + (uintptr_t) offset_old );

	if ( aligned != (uintptr_t) data_source_pointer )
		memmove(
			(void *) aligned,
			data_source_pointer,
			size_old < size_new ? size_old : size_new
		);

	struct AM_Alignment_Header_Info header_new = {
#			if AM_HAS_RAW_MEMORY_POINTER			== 1
		.raw_memory_pointer			= raw_memory_pointer_new,
#			endif
#			if AM_HAS_ALIGNMENT						== 1
		.alignment					= alignment,
#			endif
#			if AM_HAS_USER_MEMORY_ALLOCATED_SIZE	== 1
		.user_memory_allocated_size	= size_new
#			endif
	};
	memcpy(
		(char *)aligned - AM_HEADER_SIZE,
		&header_new,
		AM_HEADER_SIZE
	);

	return (void *) aligned;
#		endif /* AM_BACKEND_POSIX */
#	endif /* AM_BACKEND_WINDOWS */
}

#endif /* AM_NO_REALLOC */

void am_aligned_free( void * restrict pointer ) {
	assert_m( pointer != NULL, "no memory block to free found" );
	if ( pointer == NULL ) return;

#ifdef AM_BACKEND_WINDOWS
	_aligned_free( (char *)pointer - AM_HEADER_SIZE );
#else
	struct AM_Alignment_Header_Info header;
	memcpy(
		&header,
		(char *) pointer - AM_HEADER_SIZE,
		AM_HEADER_SIZE
	);
#	if AM_HAS_ALIGNMENT == 1 && (defined AM_BACKEND_STANDARD || defined AM_BACKEND_POSIX)
	/* overflow isn't possible because its backed on same logic within an allocation function */
	const size_t original_pointer_offset =
		(AM_HEADER_SIZE + header.alignment - 1) & ~(size_t)(header.alignment - 1);
	free( (char *)pointer - original_pointer_offset );
#	else /* AM_BACKEND_FALLBACK, AM_NO_REALLOC */
	free( header.raw_memory_pointer );
#	endif /* AM_BACKEND_STANDARD, AM_BACKEND_POSIX */
#endif /* AM_BACKEND_WINDOWS */
}

/* Function:
 * allocate aligned memory block
 *
 * Precondition:
 * correct arguments
 *
 * Parameters:
 * alignment				- desired address alignment
 * size						- size of memory block to allocate
 * out_raw_memory_pointer	- original memory block pointer
 *
 * Returns:
 * pointer					- block was allocated
 * NULL						- failed to allocate block
 */
static void * am_aligned_allocation(
		size_t alignment, size_t size, void ** restrict out_raw_memory_pointer
	)
{
	assert_m( out_raw_memory_pointer != NULL, "No memory pointer to save specified" );
	assert_mf(
		size > 0 && size <= SIZE_MAX - AM_HEADER_SIZE,
		"Allocation size must leave space for the information (size: %zu)", size
	);
	assert_m( alignment > 0, "Alignment is a divider and can not be zero" );

	void * raw_memory_pointer = NULL;

#ifdef AM_BACKEND_FALLBACK
	assert_m(
		alignment <= SIZE_MAX - AM_HEADER_SIZE - size + 1,
		"Alignment does not leave enough space for the size and its information"
	);

	size_t size_to_allocate = size + alignment - 1 + AM_HEADER_SIZE;
	assert_m( size_to_allocate >= size, "Allocation size overflow" );

	raw_memory_pointer = malloc( size_to_allocate );
	if ( raw_memory_pointer == NULL ) return NULL;

	/*  no overflow possible after malloc */
	uintptr_t address = (uintptr_t)raw_memory_pointer + (uintptr_t)AM_HEADER_SIZE;
	uintptr_t aligned = 0;

#	if AM_ALIGN_BITWISE == 1
	aligned = (address + alignment - 1) & ~(uintptr_t)(alignment - 1);
#	else /* AM_ALIGN_BITWISE == 0 */
	size_t remainder = address % alignment;
	aligned = (remainder != 0)
		? address + alignment - remainder
		: address;
#	endif /* AM_ALIGN_BITWISE == 1 */

	*out_raw_memory_pointer = raw_memory_pointer;
	return (void *) aligned;

#elif defined AM_BACKEND_WINDOWS

	raw_memory_pointer = _aligned_offset_malloc(
		size + AM_HEADER_SIZE,
		alignment,
		AM_HEADER_SIZE
	);
	if ( raw_memory_pointer == NULL ) return NULL;

	*out_raw_memory_pointer = raw_memory_pointer;

	return (void*)((char *)raw_memory_pointer + AM_HEADER_SIZE);

#else /* AM_BACKEND_STANDARD, AM_BACKEND_POSIX */

	const size_t header_information_size_aligned =
		(AM_HEADER_SIZE + alignment - 1) & ~(size_t)(alignment - 1);

	assert_m(
		size <= SIZE_MAX - header_information_size_aligned,
		"Allocation size must leave space for the information"
	);

#	ifdef AM_BACKEND_STANDARD
	size_t to_allocate = size + header_information_size_aligned;
#		if (__STDC_VERSION__ + 0L) < 202311L
	assert_m(
		to_allocate <= SIZE_MAX - (alignment - 1),
		"Allocation size must leave space for the information"
	);
	if ( to_allocate > SIZE_MAX - alignment + 1 )
		return NULL;
	to_allocate = (to_allocate + alignment - 1) & ~(size_t)(alignment - 1);
#		endif /* __STDC_VERSION__ < 202311L */

	raw_memory_pointer = aligned_alloc( alignment, to_allocate );
	if ( raw_memory_pointer == NULL )
		return NULL;

#	else /* AM_BACKEND_POSIX */
	if ( posix_memalign(
			&raw_memory_pointer, alignment, size + header_information_size_aligned ) != 0
		) return NULL;
#	endif /* AM_BACKEND_STANDARD */

	*out_raw_memory_pointer = raw_memory_pointer;
	return (void *)((char *)raw_memory_pointer + header_information_size_aligned);

#endif /* AM_BACKEND_FALLBACK */
}

/* Function:
 * validate the allocation size
 *
 * alignment		- desired address alignment
 * elements_amount	- amount of elements
 * element_size		- size of each element
 *
 * Returns:
 * non-zero			- allocation size
 * zero				- incorrect arguments
 */
static inline size_t am_check_array_bounds( size_t elements_amount, size_t element_size ) {
	/* alignment check is located in the am_aligned_malloc function */
	assert_m( elements_amount	!= 0, "Amount of elements for allocation shouldn't be zero"	);
	assert_m( element_size		!= 0, "Element size shouldn't be zero, it's a divider"		);

	if ( element_size == 0 || elements_amount == 0 )
		return 0;

	assert_mf(
		elements_amount <= SIZE_MAX / element_size,
		"Size of block for allocation shouldn't be that big "
		"(elements amount: %zu, element size: %zu)", elements_amount, element_size
	);
	if ( elements_amount > SIZE_MAX / element_size )
		return 0;

	return elements_amount * element_size;
}
