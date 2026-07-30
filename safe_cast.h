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

/* Portable overflow-checked integer conversions.
 *
 * Every sa_ovf_<source>_to_<destination> function checks whether
 * a value of the source type fits into the destination type without
 * truncation, sign loss or undefined behaviour.
 *
 * Precondition:
 * type_to_pointer	- isn't NULL
 *
 * Arguments:
 * type_from		- type to convert
 * type_to_pointer	- pointer to store the converted value
 *
 * Returns:
 * true				- overflow, destination pointer is NULL
 * false			- success
 */

#pragma once

#ifndef SAFE_CAST_H
#define SAFE_CAST_H

#	include "assert_m.h"/* assert_m */

#	include <stddef.h>	/* size_t	*/
#	include <stdint.h>	/* int8_t	*/
#	include <stdbool.h>	/* bool		*/

#	ifndef SIZE_MAX
#		error "SIZE_MAX is not defined. A C99 compliant compiler is required."
#	endif
#	ifndef PTRDIFF_MAX
#		error "PTRDIFF_MAX is not defined. A C99 compliant compiler is required."
#	endif
#	ifndef INTMAX_MAX
#		error "INTMAX_MAX is not defined. A C99 compliant compiler is required."
#	endif
#	ifndef UINTMAX_MAX
#		error "UINTMAX_MAX is not defined. A C99 compliant compiler is required."
#	endif

#	if defined(__STDC_VERSION__) && (__STDC_VERSION__ + 0L) == 202311L
#		if PTRDIFF_MAX < 32767L
#			error "ptrdiff_t must be at least 16 bits C23"
#		endif
#	else
#		if PTRDIFF_MAX < 65535L
#			error "ptrdiff_t must be at least 17 bits C99-C17"
#		endif
#	endif /* __STDC_VERSION__ == 202311L */
static_assert_m( SIZE_MAX	>= 65535u,					"size_t must be at least 16 bits"	);
static_assert_m( INTMAX_MAX	>= 0x7FFFFFFFFFFFFFFFLL,	"intmax_t must be at least 64 bits"	);
static_assert_m( UINTMAX_MAX>= 0xFFFFFFFFFFFFFFFFULL,	"uintmax_t must be at least 64 bits");

#	if defined(UINT8_MAX) && defined(INT8_MAX)
static inline bool sa_ovf_uint8_t_to_int8_t(uint8_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uint8_t) INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* UINT8_MAX */

#	if defined(INT8_MAX) && defined(UINT8_MAX)
static inline bool sa_ovf_int8_t_to_uint8_t(int8_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(INT8_MAX) && defined(UINT16_MAX)
static inline bool sa_ovf_int8_t_to_uint16_t(int8_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(INT8_MAX) && defined(UINT32_MAX)
static inline bool sa_ovf_int8_t_to_uint32_t(int8_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(INT8_MAX) && defined(UINT64_MAX)
static inline bool sa_ovf_int8_t_to_uint64_t(int8_t type_from, uint64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uint64_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(INT8_MAX)
static inline bool sa_ovf_int8_t_to_size_t(int8_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(INT8_MAX)
static inline bool sa_ovf_int8_t_to_uintmax_t(int8_t type_from, uintmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uintmax_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(INT8_MAX) && defined(UINTPTR_MAX)
static inline bool sa_ovf_int8_t_to_uintptr_t(int8_t type_from, uintptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(UINT16_MAX) && defined(UINT8_MAX)
static inline bool sa_ovf_uint16_t_to_uint8_t(uint16_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* UINT16_MAX */

#	if defined(UINT16_MAX) && defined(INT8_MAX)
static inline bool sa_ovf_uint16_t_to_int8_t(uint16_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uint16_t) INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* UINT16_MAX */

#	if defined(UINT16_MAX) && defined(INT16_MAX)
static inline bool sa_ovf_uint16_t_to_int16_t(uint16_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uint16_t) INT16_MAX )
		return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* UINT16_MAX */

#	if defined(UINT16_MAX)
static inline bool sa_ovf_uint16_t_to_ptrdiff_t(uint16_t type_from, ptrdiff_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if PTRDIFF_MAX < UINT16_MAX
		|| type_from > (uint16_t) PTRDIFF_MAX
#		endif
		) return true;

	*type_to_pointer = (ptrdiff_t) type_from;
	return false;
}
#	endif /* UINT16_MAX */

#	if defined(UINT16_MAX) && defined(INTPTR_MAX)
static inline bool sa_ovf_uint16_t_to_intptr_t(uint16_t type_from, intptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTPTR_MAX < UINT16_MAX
		|| type_from > (uint16_t) INTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (intptr_t) type_from;
	return false;
}
#	endif /* UINT16_MAX */

#	if defined(INT16_MAX) && defined(UINT8_MAX)
static inline bool sa_ovf_int16_t_to_uint8_t(int16_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (int16_t) UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(INT16_MAX) && defined(INT8_MAX)
static inline bool sa_ovf_int16_t_to_int8_t(int16_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT8_MIN || type_from > INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(INT16_MAX) && defined(UINT16_MAX)
static inline bool sa_ovf_int16_t_to_uint16_t(int16_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(INT16_MAX) && defined(UINT32_MAX)
static inline bool sa_ovf_int16_t_to_uint32_t(int16_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(INT16_MAX) && defined(UINT64_MAX)
static inline bool sa_ovf_int16_t_to_uint64_t(int16_t type_from, uint64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uint64_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(INT16_MAX)
static inline bool sa_ovf_int16_t_to_size_t(int16_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(INT16_MAX)
static inline bool sa_ovf_int16_t_to_uintmax_t(int16_t type_from, uintmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uintmax_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(INT16_MAX) && defined(UINTPTR_MAX)
static inline bool sa_ovf_int16_t_to_uintptr_t(int16_t type_from, uintptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(UINT32_MAX) && defined(UINT8_MAX)
static inline bool sa_ovf_uint32_t_to_uint8_t(uint32_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(UINT32_MAX) && defined(INT8_MAX)
static inline bool sa_ovf_uint32_t_to_int8_t(uint32_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uint32_t) INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(UINT32_MAX) && defined(UINT16_MAX)
static inline bool sa_ovf_uint32_t_to_uint16_t(uint32_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT16_MAX )
		return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(UINT32_MAX) && defined(INT16_MAX)
static inline bool sa_ovf_uint32_t_to_int16_t(uint32_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uint32_t) INT16_MAX )
		return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(UINT32_MAX) && defined(INT32_MAX)
static inline bool sa_ovf_uint32_t_to_int32_t(uint32_t type_from, int32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uint32_t) INT32_MAX )
		return true;

	*type_to_pointer = (int32_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(UINT32_MAX)
static inline bool sa_ovf_uint32_t_to_size_t(uint32_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if SIZE_MAX < UINT32_MAX
		|| type_from > SIZE_MAX
#		endif
		) return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(UINT32_MAX)
static inline bool sa_ovf_uint32_t_to_ptrdiff_t(uint32_t type_from, ptrdiff_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if PTRDIFF_MAX < UINT32_MAX
		|| type_from > (uint32_t) PTRDIFF_MAX
#		endif
		) return true;

	*type_to_pointer = (ptrdiff_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(UINT32_MAX) && defined(UINTPTR_MAX)
static inline bool sa_ovf_uint32_t_to_uintptr_t(uint32_t type_from, uintptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINTPTR_MAX < UINT32_MAX
		|| type_from > UINTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(UINT32_MAX) && defined(INTPTR_MAX)
static inline bool sa_ovf_uint32_t_to_intptr_t(uint32_t type_from, intptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTPTR_MAX < UINT32_MAX
		|| type_from > (uint32_t) INTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (intptr_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(INT32_MAX) && defined(UINT8_MAX)
static inline bool sa_ovf_int32_t_to_uint8_t(int32_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (int32_t) UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(INT32_MAX) && defined(INT8_MAX)
static inline bool sa_ovf_int32_t_to_int8_t(int32_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT8_MIN || type_from > INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(INT32_MAX) && defined(UINT16_MAX)
static inline bool sa_ovf_int32_t_to_uint16_t(int32_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (int32_t) UINT16_MAX )
		return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(INT32_MAX) && defined(INT16_MAX)
static inline bool sa_ovf_int32_t_to_int16_t(int32_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT16_MIN || type_from > INT16_MAX )
		return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(INT32_MAX) && defined(UINT32_MAX)
static inline bool sa_ovf_int32_t_to_uint32_t(int32_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(INT32_MAX) && defined(UINT64_MAX)
static inline bool sa_ovf_int32_t_to_uint64_t(int32_t type_from, uint64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uint64_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(INT32_MAX)
static inline bool sa_ovf_int32_t_to_size_t(int32_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if SIZE_MAX < INT32_MAX
		|| type_from > (int32_t) SIZE_MAX
#		endif
		) return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(INT32_MAX)
static inline bool sa_ovf_int32_t_to_ptrdiff_t(int32_t type_from, ptrdiff_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if PTRDIFF_MAX < INT32_MAX
		|| type_from < PTRDIFF_MIN || type_from > PTRDIFF_MAX
#		endif
		) return true;

	*type_to_pointer = (ptrdiff_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(INT32_MAX)
static inline bool sa_ovf_int32_t_to_uintmax_t(int32_t type_from, uintmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uintmax_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(INT32_MAX) && defined(UINTPTR_MAX)
static inline bool sa_ovf_int32_t_to_uintptr_t(int32_t type_from, uintptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINTPTR_MAX < INT32_MAX
		|| type_from > (int32_t) UINTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(INT32_MAX) && defined(INTPTR_MAX)
static inline bool sa_ovf_int32_t_to_intptr_t(int32_t type_from, intptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTPTR_MAX < INT32_MAX
		|| type_from < INTPTR_MIN || type_from > INTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (intptr_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(UINT64_MAX) && defined(UINT8_MAX)
static inline bool sa_ovf_uint64_t_to_uint8_t(uint64_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX) && defined(INT8_MAX)
static inline bool sa_ovf_uint64_t_to_int8_t(uint64_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uint64_t) INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX) && defined(UINT16_MAX)
static inline bool sa_ovf_uint64_t_to_uint16_t(uint64_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT16_MAX )
		return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX) && defined(INT16_MAX)
static inline bool sa_ovf_uint64_t_to_int16_t(uint64_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uint64_t) INT16_MAX )
		return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX) && defined(UINT32_MAX)
static inline bool sa_ovf_uint64_t_to_uint32_t(uint64_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT32_MAX )
		return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX) && defined(INT32_MAX)
static inline bool sa_ovf_uint64_t_to_int32_t(uint64_t type_from, int32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uint64_t) INT32_MAX )
		return true;

	*type_to_pointer = (int32_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX) && defined(INT64_MAX)
static inline bool sa_ovf_uint64_t_to_int64_t(uint64_t type_from, int64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uint64_t) INT64_MAX )
		return true;

	*type_to_pointer = (int64_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX)
static inline bool sa_ovf_uint64_t_to_size_t(uint64_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if SIZE_MAX < UINT64_MAX
		|| type_from > SIZE_MAX
#		endif
		) return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX)
static inline bool sa_ovf_uint64_t_to_ptrdiff_t(uint64_t type_from, ptrdiff_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if PTRDIFF_MAX < UINT64_MAX
		|| type_from > (uint64_t) PTRDIFF_MAX
#		endif
		) return true;

	*type_to_pointer = (ptrdiff_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX)
static inline bool sa_ovf_uint64_t_to_intmax_t(uint64_t type_from, intmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTMAX_MAX < UINT64_MAX
		|| type_from > (uint64_t) INTMAX_MAX
#		endif
		) return true;

	*type_to_pointer = (intmax_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX) && defined(UINTPTR_MAX)
static inline bool sa_ovf_uint64_t_to_uintptr_t(uint64_t type_from, uintptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINTPTR_MAX < UINT64_MAX
		|| type_from > UINTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(UINT64_MAX) && defined(INTPTR_MAX)
static inline bool sa_ovf_uint64_t_to_intptr_t(uint64_t type_from, intptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTPTR_MAX < UINT64_MAX
		|| type_from > (uint64_t) INTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (intptr_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(INT64_MAX) && defined(UINT8_MAX)
static inline bool sa_ovf_int64_t_to_uint8_t(int64_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (int64_t) UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX) && defined(INT8_MAX)
static inline bool sa_ovf_int64_t_to_int8_t(int64_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT8_MIN || type_from > INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX) && defined(UINT16_MAX)
static inline bool sa_ovf_int64_t_to_uint16_t(int64_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (int64_t) UINT16_MAX )
		return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX) && defined(INT16_MAX)
static inline bool sa_ovf_int64_t_to_int16_t(int64_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT16_MIN || type_from > INT16_MAX )
		return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX) && defined(UINT32_MAX)
static inline bool sa_ovf_int64_t_to_uint32_t(int64_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (int64_t) UINT32_MAX )
		return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX) && defined(INT32_MAX)
static inline bool sa_ovf_int64_t_to_int32_t(int64_t type_from, int32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT32_MIN || type_from > INT32_MAX )
		return true;

	*type_to_pointer = (int32_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX) && defined(UINT64_MAX)
static inline bool sa_ovf_int64_t_to_uint64_t(int64_t type_from, uint64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uint64_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX)
static inline bool sa_ovf_int64_t_to_size_t(int64_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if SIZE_MAX < INT64_MAX
		|| type_from > (int64_t) SIZE_MAX
#		endif
		) return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX)
static inline bool sa_ovf_int64_t_to_ptrdiff_t(int64_t type_from, ptrdiff_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if PTRDIFF_MAX < INT64_MAX
		|| type_from < PTRDIFF_MIN || type_from > PTRDIFF_MAX
#		endif
		) return true;

	*type_to_pointer = (ptrdiff_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX)
static inline bool sa_ovf_int64_t_to_uintmax_t(int64_t type_from, uintmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uintmax_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX) && defined(UINTPTR_MAX)
static inline bool sa_ovf_int64_t_to_uintptr_t(int64_t type_from, uintptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINTPTR_MAX < INT64_MAX
		|| type_from > (int64_t) UINTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(INT64_MAX) && defined(INTPTR_MAX)
static inline bool sa_ovf_int64_t_to_intptr_t(int64_t type_from, intptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTPTR_MAX < INT64_MAX
		|| type_from < INTPTR_MIN || type_from > INTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (intptr_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

#	if defined(UINT8_MAX)
static inline bool sa_ovf_size_t_to_uint8_t(size_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* UINT8_MAX */

#	if defined(INT8_MAX)
static inline bool sa_ovf_size_t_to_int8_t(size_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (size_t) INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(UINT16_MAX)
static inline bool sa_ovf_size_t_to_uint16_t(size_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINT16_MAX < SIZE_MAX
		|| type_from > UINT16_MAX
#		endif
		) return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* UINT16_MAX */

#	if defined(INT16_MAX)
static inline bool sa_ovf_size_t_to_int16_t(size_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (size_t) INT16_MAX )
		return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(UINT32_MAX)
static inline bool sa_ovf_size_t_to_uint32_t(size_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINT32_MAX < SIZE_MAX
		|| type_from > UINT32_MAX
#		endif
		) return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(INT32_MAX)
static inline bool sa_ovf_size_t_to_int32_t(size_t type_from, int32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT32_MAX < SIZE_MAX
		|| type_from > (size_t) INT32_MAX
#		endif
		) return true;

	*type_to_pointer = (int32_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(UINT64_MAX)
static inline bool sa_ovf_size_t_to_uint64_t(size_t type_from, uint64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINT64_MAX < SIZE_MAX
		|| type_from > UINT64_MAX
#		endif
		) return true;

	*type_to_pointer = (uint64_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(INT64_MAX)
static inline bool sa_ovf_size_t_to_int64_t(size_t type_from, int64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT64_MAX < SIZE_MAX
		|| type_from > (size_t) INT64_MAX
#		endif
		) return true;

	*type_to_pointer = (int64_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

static inline bool sa_ovf_size_t_to_ptrdiff_t(size_t type_from, ptrdiff_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#	if PTRDIFF_MAX < SIZE_MAX
		|| type_from > (size_t) PTRDIFF_MAX
#	endif
		) return true;

	*type_to_pointer = (ptrdiff_t) type_from;
	return false;
}

static inline bool sa_ovf_size_t_to_uintmax_t(size_t type_from, uintmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#	if UINTMAX_MAX < SIZE_MAX
		|| type_from > UINTMAX_MAX
#	endif
		) return true;

	*type_to_pointer = (uintmax_t) type_from;
	return false;
}

static inline bool sa_ovf_size_t_to_intmax_t(size_t type_from, intmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#	if INTMAX_MAX < SIZE_MAX
		|| type_from > (size_t) INTMAX_MAX
#	endif
		) return true;

	*type_to_pointer = (intmax_t) type_from;
	return false;
}

#	if defined(UINTPTR_MAX)
static inline bool sa_ovf_size_t_to_uintptr_t(size_t type_from, uintptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINTPTR_MAX < SIZE_MAX
		|| type_from > UINTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(INTPTR_MAX)
static inline bool sa_ovf_size_t_to_intptr_t(size_t type_from, intptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTPTR_MAX < SIZE_MAX
		|| type_from > (size_t) INTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (intptr_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(UINT8_MAX)
static inline bool sa_ovf_ptrdiff_t_to_uint8_t(ptrdiff_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (ptrdiff_t) UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* UINT8_MAX */

#	if defined(INT8_MAX)
static inline bool sa_ovf_ptrdiff_t_to_int8_t(ptrdiff_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT8_MIN || type_from > INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(UINT16_MAX)
static inline bool sa_ovf_ptrdiff_t_to_uint16_t(ptrdiff_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINT16_MAX < PTRDIFF_MAX
		|| type_from > (ptrdiff_t) UINT16_MAX
#		endif
		) return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* UINT16_MAX */

#	if defined(INT16_MAX)
static inline bool sa_ovf_ptrdiff_t_to_int16_t(ptrdiff_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT16_MAX < PTRDIFF_MAX
		|| type_from < INT16_MIN || type_from > INT16_MAX
#		endif
		) return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(UINT32_MAX)
static inline bool sa_ovf_ptrdiff_t_to_uint32_t(ptrdiff_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINT32_MAX < PTRDIFF_MAX
		|| type_from > (ptrdiff_t) UINT32_MAX
#		endif
		) return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(INT32_MAX)
static inline bool sa_ovf_ptrdiff_t_to_int32_t(ptrdiff_t type_from, int32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT32_MAX < PTRDIFF_MAX
		|| type_from < INT32_MIN || type_from > INT32_MAX
#		endif
		) return true;

	*type_to_pointer = (int32_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(UINT64_MAX)
static inline bool sa_ovf_ptrdiff_t_to_uint64_t(ptrdiff_t type_from, uint64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINT64_MAX < PTRDIFF_MAX
		|| type_from > (ptrdiff_t) UINT64_MAX
#		endif
		) return true;

	*type_to_pointer = (uint64_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(INT64_MAX)
static inline bool sa_ovf_ptrdiff_t_to_int64_t(ptrdiff_t type_from, int64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT64_MAX < PTRDIFF_MAX
		|| type_from < INT64_MIN || type_from > INT64_MAX
#		endif
		) return true;

	*type_to_pointer = (int64_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

static inline bool sa_ovf_ptrdiff_t_to_size_t(ptrdiff_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#	if SIZE_MAX < PTRDIFF_MAX
		|| type_from > (ptrdiff_t) SIZE_MAX
#	endif
		) return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}

static inline bool sa_ovf_ptrdiff_t_to_uintmax_t(
		ptrdiff_t type_from, uintmax_t * type_to_pointer
	)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#	if UINTMAX_MAX < PTRDIFF_MAX
		|| type_from > (ptrdiff_t) UINTMAX_MAX
#	endif
		) return true;

	*type_to_pointer = (uintmax_t) type_from;
	return false;
}

static inline bool sa_ovf_ptrdiff_t_to_intmax_t(ptrdiff_t type_from, intmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#	if INTMAX_MAX < PTRDIFF_MAX
		|| type_from < INTMAX_MIN || type_from > INTMAX_MAX
#	endif
		) return true;

	*type_to_pointer = (intmax_t) type_from;
	return false;
}

#	if defined(UINTPTR_MAX)
static inline bool sa_ovf_ptrdiff_t_to_uintptr_t(
		ptrdiff_t type_from, uintptr_t * type_to_pointer
	)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINTPTR_MAX < PTRDIFF_MAX
		|| type_from > (ptrdiff_t) UINTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(INTPTR_MAX)
static inline bool sa_ovf_ptrdiff_t_to_intptr_t(ptrdiff_t type_from, intptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTPTR_MAX < PTRDIFF_MAX
		|| type_from < INTPTR_MIN || type_from > INTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (intptr_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(UINT8_MAX)
static inline bool sa_ovf_uintmax_t_to_uint8_t(uintmax_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* UINT8_MAX */

#	if defined(INT8_MAX)
static inline bool sa_ovf_uintmax_t_to_int8_t(uintmax_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uintmax_t) INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(UINT16_MAX)
static inline bool sa_ovf_uintmax_t_to_uint16_t(uintmax_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT16_MAX )
		return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* UINT16_MAX */

#	if defined(INT16_MAX)
static inline bool sa_ovf_uintmax_t_to_int16_t(uintmax_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uintmax_t) INT16_MAX )
		return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(UINT32_MAX)
static inline bool sa_ovf_uintmax_t_to_uint32_t(uintmax_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT32_MAX )
		return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(INT32_MAX)
static inline bool sa_ovf_uintmax_t_to_int32_t(uintmax_t type_from, int32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uintmax_t) INT32_MAX )
		return true;

	*type_to_pointer = (int32_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(UINT64_MAX)
static inline bool sa_ovf_uintmax_t_to_uint64_t(uintmax_t type_from, uint64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINT64_MAX < UINTMAX_MAX
		|| type_from > UINT64_MAX
#		endif
		) return true;

	*type_to_pointer = (uint64_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(INT64_MAX)
static inline bool sa_ovf_uintmax_t_to_int64_t(uintmax_t type_from, int64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uintmax_t) INT64_MAX )
		return true;

	*type_to_pointer = (int64_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

static inline bool sa_ovf_uintmax_t_to_size_t(uintmax_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#	if SIZE_MAX < UINTMAX_MAX
		|| type_from > SIZE_MAX
#	endif
		) return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}

static inline bool sa_ovf_uintmax_t_to_ptrdiff_t(
		uintmax_t type_from, ptrdiff_t * type_to_pointer
	)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#	if PTRDIFF_MAX < UINTMAX_MAX
		|| type_from > (uintmax_t) PTRDIFF_MAX
#	endif
		) return true;

	*type_to_pointer = (ptrdiff_t) type_from;
	return false;
}

static inline bool sa_ovf_uintmax_t_to_intmax_t(uintmax_t type_from, intmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#	if INTMAX_MAX < UINTMAX_MAX
		|| type_from > (uintmax_t) INTMAX_MAX
#	endif
		) return true;

	*type_to_pointer = (intmax_t) type_from;
	return false;
}

#	if defined(UINTPTR_MAX)
static inline bool sa_ovf_uintmax_t_to_uintptr_t(
		uintmax_t type_from, uintptr_t * type_to_pointer
	)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINTPTR_MAX < UINTMAX_MAX
		|| type_from > UINTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(INTPTR_MAX)
static inline bool sa_ovf_uintmax_t_to_intptr_t(uintmax_t type_from, intptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTPTR_MAX < UINTMAX_MAX
		|| type_from > (uintmax_t) INTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (intptr_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(UINT8_MAX)
static inline bool sa_ovf_intmax_t_to_uint8_t(intmax_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (intmax_t) UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* UINT8_MAX */

#	if defined(INT8_MAX)
static inline bool sa_ovf_intmax_t_to_int8_t(intmax_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT8_MIN || type_from > INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* INT8_MAX */

#	if defined(UINT16_MAX)
static inline bool sa_ovf_intmax_t_to_uint16_t(intmax_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (intmax_t) UINT16_MAX )
		return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* UINT16_MAX */

#	if defined(INT16_MAX)
static inline bool sa_ovf_intmax_t_to_int16_t(intmax_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT16_MIN || type_from > INT16_MAX )
		return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* INT16_MAX */

#	if defined(UINT32_MAX)
static inline bool sa_ovf_intmax_t_to_uint32_t(intmax_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (intmax_t) UINT32_MAX )
		return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* UINT32_MAX */

#	if defined(INT32_MAX)
static inline bool sa_ovf_intmax_t_to_int32_t(intmax_t type_from, int32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT32_MIN || type_from > INT32_MAX )
		return true;

	*type_to_pointer = (int32_t) type_from;
	return false;
}
#	endif /* INT32_MAX */

#	if defined(UINT64_MAX)
static inline bool sa_ovf_intmax_t_to_uint64_t(intmax_t type_from, uint64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINT64_MAX < INTMAX_MAX
		|| type_from > (intmax_t) UINT64_MAX
#		endif
		) return true;

	*type_to_pointer = (uint64_t) type_from;
	return false;
}
#	endif /* UINT64_MAX */

#	if defined(INT64_MAX)
static inline bool sa_ovf_intmax_t_to_int64_t(intmax_t type_from, int64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT64_MAX < INTMAX_MAX
		|| type_from < INT64_MIN || type_from > INT64_MAX
#		endif
		) return true;

	*type_to_pointer = (int64_t) type_from;
	return false;
}
#	endif /* INT64_MAX */

static inline bool sa_ovf_intmax_t_to_size_t(intmax_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#	if SIZE_MAX < INTMAX_MAX
		|| type_from > (intmax_t) SIZE_MAX
#	endif
		) return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}

static inline bool sa_ovf_intmax_t_to_ptrdiff_t(intmax_t type_from, ptrdiff_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#	if PTRDIFF_MAX < INTMAX_MAX
		|| type_from < PTRDIFF_MIN || type_from > PTRDIFF_MAX
#	endif
		) return true;

	*type_to_pointer = (ptrdiff_t) type_from;
	return false;
}

static inline bool sa_ovf_intmax_t_to_uintmax_t(intmax_t type_from, uintmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 )
		return true;

	*type_to_pointer = (uintmax_t) type_from;
	return false;
}

#	if defined(UINTPTR_MAX)
static inline bool sa_ovf_intmax_t_to_uintptr_t(intmax_t type_from, uintptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINTPTR_MAX < INTMAX_MAX
		|| type_from > (intmax_t) UINTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(INTPTR_MAX)
static inline bool sa_ovf_intmax_t_to_intptr_t(intmax_t type_from, intptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTPTR_MAX < INTMAX_MAX
		|| type_from < INTPTR_MIN || type_from > INTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (intptr_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(UINTPTR_MAX) && defined(UINT8_MAX)
static inline bool sa_ovf_uintptr_t_to_uint8_t(uintptr_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX) && defined(INT8_MAX)
static inline bool sa_ovf_uintptr_t_to_int8_t(uintptr_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uintptr_t) INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX) && defined(UINT16_MAX)
static inline bool sa_ovf_uintptr_t_to_uint16_t(uintptr_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINT16_MAX < UINTPTR_MAX
		|| type_from > UINT16_MAX
#		endif
		) return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX) && defined(INT16_MAX)
static inline bool sa_ovf_uintptr_t_to_int16_t(uintptr_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from > (uintptr_t) INT16_MAX )
		return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX) && defined(UINT32_MAX)
static inline bool sa_ovf_uintptr_t_to_uint32_t(uintptr_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINT32_MAX < UINTPTR_MAX
		|| type_from > UINT32_MAX
#		endif
		) return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX) && defined(INT32_MAX)
static inline bool sa_ovf_uintptr_t_to_int32_t(uintptr_t type_from, int32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT32_MAX < UINTPTR_MAX
		|| type_from > (uintptr_t) INT32_MAX
#		endif
		) return true;

	*type_to_pointer = (int32_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX) && defined(UINT64_MAX)
static inline bool sa_ovf_uintptr_t_to_uint64_t(uintptr_t type_from, uint64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINT64_MAX < UINTPTR_MAX
		|| type_from > UINT64_MAX
#		endif
		) return true;

	*type_to_pointer = (uint64_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX) && defined(INT64_MAX)
static inline bool sa_ovf_uintptr_t_to_int64_t(uintptr_t type_from, int64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT64_MAX < UINTPTR_MAX
		|| type_from > (uintptr_t) INT64_MAX
#		endif
		) return true;

	*type_to_pointer = (int64_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX)
static inline bool sa_ovf_uintptr_t_to_size_t(uintptr_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if SIZE_MAX < UINTPTR_MAX
		|| type_from > SIZE_MAX
#		endif
		) return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX)
static inline bool sa_ovf_uintptr_t_to_ptrdiff_t(
		uintptr_t type_from, ptrdiff_t * type_to_pointer
	)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if PTRDIFF_MAX < UINTPTR_MAX
		|| type_from > (uintptr_t) PTRDIFF_MAX
#		endif
		) return true;

	*type_to_pointer = (ptrdiff_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX)
static inline bool sa_ovf_uintptr_t_to_uintmax_t(
		uintptr_t type_from, uintmax_t * type_to_pointer
	)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if UINTMAX_MAX < UINTPTR_MAX
		|| type_from > UINTMAX_MAX
#		endif
		) return true;

	*type_to_pointer = (uintmax_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX)
static inline bool sa_ovf_uintptr_t_to_intmax_t(uintptr_t type_from, intmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTMAX_MAX < UINTPTR_MAX
		|| type_from > (uintptr_t) INTMAX_MAX
#		endif
		) return true;

	*type_to_pointer = (intmax_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(UINTPTR_MAX) && defined(INTPTR_MAX)
static inline bool sa_ovf_uintptr_t_to_intptr_t(uintptr_t type_from, intptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTPTR_MAX < UINTPTR_MAX
		|| type_from > (uintptr_t) INTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (intptr_t) type_from;
	return false;
}
#	endif /* UINTPTR_MAX */

#	if defined(INTPTR_MAX) && defined(UINT8_MAX)
static inline bool sa_ovf_intptr_t_to_uint8_t(intptr_t type_from, uint8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0 || type_from > (intptr_t) UINT8_MAX )
		return true;

	*type_to_pointer = (uint8_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX) && defined(INT8_MAX)
static inline bool sa_ovf_intptr_t_to_int8_t(intptr_t type_from, int8_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < INT8_MIN || type_from > INT8_MAX )
		return true;

	*type_to_pointer = (int8_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX) && defined(UINT16_MAX)
static inline bool sa_ovf_intptr_t_to_uint16_t(intptr_t type_from, uint16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINT16_MAX < INTPTR_MAX
		|| type_from > (intptr_t) UINT16_MAX
#		endif
		) return true;

	*type_to_pointer = (uint16_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX) && defined(INT16_MAX)
static inline bool sa_ovf_intptr_t_to_int16_t(intptr_t type_from, int16_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT16_MAX < INTPTR_MAX
		|| type_from < INT16_MIN || type_from > INT16_MAX
#		endif
		) return true;

	*type_to_pointer = (int16_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX) && defined(UINT32_MAX)
static inline bool sa_ovf_intptr_t_to_uint32_t(intptr_t type_from, uint32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINT32_MAX < INTPTR_MAX
		|| type_from > (intptr_t) UINT32_MAX
#		endif
		) return true;

	*type_to_pointer = (uint32_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX) && defined(INT32_MAX)
static inline bool sa_ovf_intptr_t_to_int32_t(intptr_t type_from, int32_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT32_MAX < INTPTR_MAX
		|| type_from < INT32_MIN || type_from > INT32_MAX
#		endif
		) return true;

	*type_to_pointer = (int32_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX) && defined(UINT64_MAX)
static inline bool sa_ovf_intptr_t_to_uint64_t(intptr_t type_from, uint64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINT64_MAX < INTPTR_MAX
		|| type_from > (intptr_t) UINT64_MAX
#		endif
		) return true;

	*type_to_pointer = (uint64_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX) && defined(INT64_MAX)
static inline bool sa_ovf_intptr_t_to_int64_t(intptr_t type_from, int64_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INT64_MAX < INTPTR_MAX
		|| type_from < INT64_MIN || type_from > INT64_MAX
#		endif
		) return true;

	*type_to_pointer = (int64_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX)
static inline bool sa_ovf_intptr_t_to_size_t(intptr_t type_from, size_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if SIZE_MAX < INTPTR_MAX
		|| type_from > (intptr_t) SIZE_MAX
#		endif
		) return true;

	*type_to_pointer = (size_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX)
static inline bool sa_ovf_intptr_t_to_ptrdiff_t(intptr_t type_from, ptrdiff_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if PTRDIFF_MAX < INTPTR_MAX
		|| type_from < PTRDIFF_MIN || type_from > PTRDIFF_MAX
#		endif
		) return true;

	*type_to_pointer = (ptrdiff_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX)
static inline bool sa_ovf_intptr_t_to_uintmax_t(intptr_t type_from, uintmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINTMAX_MAX < INTPTR_MAX
		|| type_from > (intptr_t) UINTMAX_MAX
#		endif
		) return true;

	*type_to_pointer = (uintmax_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX)
static inline bool sa_ovf_intptr_t_to_intmax_t(intptr_t type_from, intmax_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL
#		if INTMAX_MAX < INTPTR_MAX
		|| type_from < INTMAX_MIN || type_from > INTMAX_MAX
#		endif
		) return true;

	*type_to_pointer = (intmax_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#	if defined(INTPTR_MAX) && defined(UINTPTR_MAX)
static inline bool sa_ovf_intptr_t_to_uintptr_t(intptr_t type_from, uintptr_t * type_to_pointer)
{
	assert_m( type_to_pointer != NULL, "No place to write a result found" );

	if ( type_to_pointer == NULL || type_from < 0
#		if UINTPTR_MAX < INTPTR_MAX
		|| type_from > (intptr_t) UINTPTR_MAX
#		endif
		) return true;

	*type_to_pointer = (uintptr_t) type_from;
	return false;
}
#	endif /* INTPTR_MAX */

#endif /* SAFE_CAST_H */
