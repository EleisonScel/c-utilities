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

#pragma once

#ifndef SAFE_ALLOC_H
#define SAFE_ALLOC_H

#	include "assert_m.h"			/* assert_check_m	*/
#	include "safe_multiplication.h"	/* sa_ovf_mul_size_t*/

#	include <string.h> /* memset	*/

#	include <stddef.h> /* size_t	*/
#	include <stdlib.h> /* malloc	*/

static inline size_t sa_check_array_bounds( size_t elements_amount, size_t element_size );

/* Function:
 * safe version of malloc
 *
 * elements_amount	- desired amount of elements
 * element_size		- size of each element
 *
 * Returns:
 * pointer			- block was allocated
 * NULL				- invalid arguments, overflow, allocation failed
 */
static inline void * sa_malloc_array( size_t elements_amount, size_t element_size ) {
	size_t total_bytes = sa_check_array_bounds( elements_amount, element_size );
	return (total_bytes == 0) ? NULL : malloc( total_bytes );
}

/* Function:
 * safe version of calloc
 *
 * elements_amount	- desired amount of elements
 * element_size		- size of each element
 *
 * Returns:
 * pointer			- block was allocated
 * NULL				- invalid arguments, overflow, allocation failed
 */
static inline void * sa_calloc_array( size_t elements_amount, size_t element_size ) {
	if ( sa_check_array_bounds( elements_amount, element_size ) == 0 )
		return NULL;
	return calloc( elements_amount, element_size );
}

/* Function:
 * safe version of realloc
 *
 * pointer_original	- original memory pointer
 * elements_amount	- desired amount of elements
 * element_size		- size of each element
 *
 * Returns:
 * pointer			- block was reallocated
 * NULL				- invalid arguments, overflow, reallocation failed
 */
static inline void * sa_realloc_array(
		void * pointer_original, size_t elements_amount, size_t element_size
	)
{
	size_t total_bytes = sa_check_array_bounds( elements_amount, element_size );
	return (total_bytes == 0) ? NULL : realloc( pointer_original, total_bytes );
}

/* Function:
 * safe version of realloc with nullification of a new data
 *
 * pointer_original		- original memory pointer
 * elements_amount_old	- old amount of elements
 * elements_amount_new	- desired amount of elements
 * element_size			- size of each element
 *
 * Returns:
 * pointer				- block was reallocated, new space was nullifide
 * NULL					- invalid arguments, overflow, reallocation failed
 */
static inline void * sa_recalloc_array(
		void * pointer_original, size_t elements_amount_old, size_t elements_amount_new,
		size_t element_size
	)
{
	size_t total_bytes = sa_check_array_bounds( elements_amount_new, element_size );
	if ( total_bytes == 0 )
		return NULL;

	void * pointer_new = realloc( pointer_original, total_bytes );
	if ( pointer_new == NULL )
		return NULL;

	if ( elements_amount_new > elements_amount_old ) {
		char * pointer_to_clean_from = (char *) pointer_new + (elements_amount_old * element_size);
		memset(
			pointer_to_clean_from,
			0,
			(elements_amount_new - elements_amount_old) * element_size
		);
	}

	return pointer_new;
}

/* Function:
 * validate the allocation size
 *
 * elements_amount	- amount of elements
 * element_size		- size of each element
 *
 * Returns:
 * non-zero			- allocation size
 * zero				- incorrect arguments
 */
static inline size_t sa_check_array_bounds( size_t elements_amount, size_t element_size ) {
	size_t total_bytes;
	if( assert_check_m(elements_amount != 0, "Amount of elements for allocation shouldn't be zero")
		== false ||
		assert_check_m(element_size	 != 0, "Element size shouldn't be zero, it's a divider")
		== false || sa_ovf_mul_size_t(elements_amount, element_size, &total_bytes ) == true )
		return 0;

	return total_bytes;
}

#endif /* SAFE_ALLOC_H */
