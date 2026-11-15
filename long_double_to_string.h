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

/* File:
 * long_double_to_string.h
 *
 * Description:
 * Portable conversion of long double to a decimal string in scientific notation
 */

#ifndef LONG_DOUBLE_TO_STRING_H
#define LONG_DOUBLE_TO_STRING_H

#	include "assert_m.h"/* static_assert_m	*/

#	include <float.h>	/* FLT_RADIX		*/
#	include <limits.h>	/* CHAR_BIT			*/

#	include <stdint.h>	/* INTMAX_MAX		*/
#	include <stddef.h>	/* size_t			*/

/* WARNING:
 * 1. Buffer size
 *  ldts_long_double_to_string_static function relies on the fact buffer must be at least
 *    LDTS_LIMIT_LONG_DOUBLE_BUFFER bytes long, no bounds check is performed,
 *    no NULL check is performed.
 *
 * 2. Output format
 *  output is always scientific notation; there is no fixed-point variant.
 *
 * 3. Function name
 *  static in function name refers to a caller-provided static-sized buffer.
 *  the function never allocates memory and never checks the buffer for NULL.
 *
 * 4. Compile-time requirements:
 *  FLT_RADIX in {2, 4, 8, 10, 16}
 *  self-consistent <float.h> which wouldn't cause an overflow during the computation
 *    (automatically checked at compile time).
 *  At most 1 LDTS_FORCE_* definition
 *
 * 5. Style
 *  output isn't identical to printf conversions and doesn't rely on locale;
 *  - positive exponent has no leading plus and no leading zeroes ("1e5" instead of "1e+05")
 *  - decimal point is always '.' regardless of the current locale
 *  - trailing zeroes are trimmed ("1.5e5" instead of "1.500000e+05")
 *  - special values use "NaN", "Inf", "-Inf" spelling, not the implementation-defined one
 *  - output format isn't configurable
 */
/* Logic to choose the array caching backend
 *
 * Available backends in order of auto-selection preference unless forced:
 *  1. LDTS_BACKEND_STATIC_ARRAY	: compile-time constant power array
 *										requires LDTS_ARRAY_STATIC_SIZE_POWER 
 *													<= LDTS_BIGGEST_ARRAY_STATIC
 *  2. LDTS_BACKEND_WINDOWS			: InitOnceExecuteOnce
 *										requires _WIN32, _WIN32_WINNT defined and >= 0x0600
 *  3. LDTS_BACKEND_STANDARD		: call_once
 *										requires C11, non-Windows, <threads.h>
 *  4. LDTS_BACKEND_POSIX			: pthread_once
 *										requires POSIX threads, <unistd.h>, <pthread.h>
 *  5. LDTS_BACKEND_ATOMIC			: atomic compare-exchange lazy initialization
 *										requires <stdatomic.h> or
 *										__atomic_* builtins with some definitions
 *  6. LDTS_BACKEND_THREAD_LOCAL	: per-thread storage with lazy initialization
 *										requires C11
 *  7. LDTS_BACKEND_RECOMPUTE		: per-call power array computation
 *										no requirements, no caching
 */

/* Available user macro definition before including the header:
 * - LDTS_FORCE_STATIC_ARRAY	- force LDTS_BACKEND_STATIC_ARRAY
 * - LDTS_FORCE_WINDOWS			- force LDTS_BACKEND_WINDOWS
 * - LDTS_FORCE_C11				- force LDTS_BACKEND_STANDARD
 * - LDTS_FORCE_POSIX			- force LDTS_BACKEND_POSIX
 * - LDTS_FORCE_ATOMIC			- force LDTS_BACKEND_ATOMIC
 * - LDTS_FORCE_THREAD_LOCAL	- force LDTS_BACKEND_THREAD_LOCAL
 * - LDTS_FORCE_RECOMPUTE		- force LDTS_BACKEND_RECOMPUTE
 *
 * - LDTS_IS_BITS_32					- override the auto-detected system bit depth (0 or 1)
 * - LDTS_NO_WARNING_SYSTEM_BIT_DEPTH	- silence a failed bit-depth detection warning
 * - LDTS_ATOMIC_SPIN_YIELD_FUNCTION()	- spin-wait yield function for the atomic backend
 */


#	if	!defined(FLT_RADIX)		|| !defined(LDBL_MANT_DIG)	|| !defined(LDBL_MIN_EXP)	|| \
		!defined(LDBL_MAX_EXP)	|| !defined(LDBL_DIG)		|| !defined(LDBL_MIN_10_EXP)|| \
		!defined(LDBL_MAX_10_EXP)
#		error "Required floating-point limits are unavailable"
#	endif /* defined */

#	if FLT_RADIX == 2
#		define LDTS_LG_DIVISOR	6107016
#	elif FLT_RADIX == 4
#		define LDTS_LG_DIVISOR	3053508
#	elif FLT_RADIX == 8
#		define LDTS_LG_DIVISOR	2035672
#	elif FLT_RADIX == 10
#		define LDTS_LG_DIVISOR	1
#	elif FLT_RADIX == 16
#		define LDTS_LG_DIVISOR	1526754
#	else
#		error "FLT_RADIX unverified and isn't supported"
#	endif /* LDTS_LG_DIVISOR */

#	if LDTS_LG_DIVISOR == 1
#		define LDTS_LG_DIVIDEND 1
#	else
#		define LDTS_LG_DIVIDEND	1838395
#	endif /* LDTS_LG_DIVIDEND */

#	define LDTS_RADIX_LG_EXACT ((LDTS_LG_DIVIDEND == 1) ? 1 : 0)

/* last number for which round down of "lg(FLT_RADIX) * number" is correct */
#	define LDTS_LIMIT_LG_VALUE ((LDTS_RADIX_LG_EXACT == 1)	\
		? INTMAX_MAX										\
		: LDTS_LG_DIVISOR - 1)

#	if LDTS_LIMIT_LG_VALUE > INTMAX_MAX / LDTS_LG_DIVIDEND
#		error "lg would overflow intmax_t"
#	endif /* LDTS_LIMIT_LG_VALUE */

#	if LDTS_RADIX_LG_EXACT == 0
#		define LDTS_ROUND_DOWN_LG_RADIX(X) (INTMAX_C(1) * (X) * LDTS_LG_DIVIDEND / LDTS_LG_DIVISOR)
#	endif

#	if LDBL_MANT_DIG < 1 || LDBL_MIN_EXP >= 0 || LDBL_MAX_EXP <= 0
#		error "Unsupported long double representation"
#	elif LDBL_MANT_DIG > LDTS_LIMIT_LG_VALUE + LDBL_MIN_EXP || LDBL_MAX_EXP > LDTS_LIMIT_LG_VALUE
#		error "Exponent range is beyond the point where the lg fraction diverges"
#	endif

static_assert_m( LDBL_DIG		>= 10, "LDBL_DIG is below minimum"				);
static_assert_m( LDBL_MAX_10_EXP>= 37, "LDBL_MAX_10_EXP is below the minimum"	);
static_assert_m( LDBL_MIN_10_EXP<=-37, "LDBL_MIN_10_EXP is above the maximum"	);

#	if LDTS_RADIX_LG_EXACT == 1
static_assert_m(
	LDBL_DIG == INTMAX_C(1) * LDBL_MANT_DIG - 1, "LDBL_DIG disagrees with LDBL_MANT_DIG"
);
static_assert_m(
	LDBL_MIN_10_EXP == INTMAX_C(1) * LDBL_MIN_EXP - 1,
	"LDBL_MIN_10_EXP disagrees with LDBL_MIN_EXP"
);
static_assert_m(
	LDBL_MAX_10_EXP == INTMAX_C(1) * LDBL_MAX_EXP - 1,
	"LDBL_MAX_10_EXP disagrees with LDBL_MAX_EXP"
);
#	else
static_assert_m(
	LDBL_DIG == LDTS_ROUND_DOWN_LG_RADIX(INTMAX_C(1) * LDBL_MANT_DIG - 1),
	"LDBL_DIG disagrees with LDBL_MANT_DIG"
);
static_assert_m(
	LDBL_MIN_10_EXP == -LDTS_ROUND_DOWN_LG_RADIX(INTMAX_C(1) - LDBL_MIN_EXP),
	"LDBL_MIN_10_EXP disagrees with LDBL_MIN_EXP"
);
static_assert_m(
	LDBL_MAX_10_EXP == LDTS_ROUND_DOWN_LG_RADIX(LDBL_MAX_EXP),
	"LDBL_MAX_10_EXP disagrees with LDBL_MAX_EXP"
);
#	endif /* LDTS_RADIX_LG_EXACT */

#	if LDTS_RADIX_LG_EXACT == 1
#		define LDTS_LIMIT_BASE_DIGITS_TEMPORARY (INTMAX_C(1) * LDBL_MANT_DIG)
#	else
#		define LDTS_LIMIT_BASE_DIGITS_TEMPORARY (LDTS_ROUND_DOWN_LG_RADIX(LDBL_MANT_DIG) + 2)
#	endif

#	if LDTS_LIMIT_BASE_DIGITS_TEMPORARY > INT_MAX
#		error "LDTS_LIMIT_BASE_DIGITS_TEMPORARY is unreasonably large"
#	else
#		define LDTS_LIMIT_BASE_DIGITS ((int) LDTS_LIMIT_BASE_DIGITS_TEMPORARY)
#	endif /* LDTS_LIMIT_BASE_DIGITS_TEMPORARY */

#	if defined(LDBL_DECIMAL_DIG)
/* LDBL_DECIMAL_DIG may have 2 more */
static_assert_m(
	LDBL_DECIMAL_DIG >= LDTS_LIMIT_BASE_DIGITS_TEMPORARY,
	"LDBL_DECIMAL_DIG disagrees with LDBL_MANT_DIG"
);
#	elif defined(DECIMAL_DIG)
static_assert_m(
	DECIMAL_DIG >= LDTS_LIMIT_BASE_DIGITS_TEMPORARY,
	"DECIMAL_DIG below the LDBL_MANT_DIG needs"
);
#	endif /* LDBL_DECIMAL_DIG */

static_assert_m(
	(uintmax_t) LDBL_MANT_DIG + 1 <= (uintmax_t) sizeof(long double) * CHAR_BIT,
	"long double storage is smaller than its declared precision"
);

#	define LDTS_DECIMAL_DIGITS(X) (					\
		(X) < 10							? 1 :	\
		(X) < 100							? 2 :	\
		(X) < 1000							? 3 :	\
		(X) < 10000							? 4 :	\
		(X) < 100000						? 5 :	\
		(X) < 1000000						? 6 :	\
		(X) < 10000000						? 7 :	\
		(X) < 100000000						? 8 :	\
		(X) < 1000000000					? 9 :	\
		(X) < INTMAX_C(10000000000)			? 10:	\
		(X) < INTMAX_C(100000000000)		? 11:	\
		(X) < INTMAX_C(1000000000000)		? 12:	\
		(X) < INTMAX_C(10000000000000)		? 13:	\
		(X) < INTMAX_C(100000000000000)		? 14:	\
		(X) < INTMAX_C(1000000000000000)	? 15:	\
		(X) < INTMAX_C(10000000000000000)	? 16:	\
		(X) < INTMAX_C(100000000000000000)	? 17:	\
		(X) < INTMAX_C(1000000000000000000)	? 18: 19)

/* positive exponent without a plus;										*
 * +1: theoretically mantissa may round up growing the exponent by 1,		*
 *		which may increase the amount of digits for number representation	*/
#	define LDTS_LIMIT_SYMBOLS_EXPONENT_MAXIMUM (LDTS_DECIMAL_DIGITS(LDBL_MAX_10_EXP) + 1)

#	if LDTS_RADIX_LG_EXACT == 1
#		define LDTS_LIMIT_DIGITS_EXPONENT_DENORMAL (INTMAX_C(1) * LDBL_MANT_DIG - LDBL_MIN_EXP)
#	else
#		define LDTS_LIMIT_DIGITS_EXPONENT_DENORMAL										\
			(LDTS_ROUND_DOWN_LG_RADIX(INTMAX_C(1) * LDBL_MANT_DIG - LDBL_MIN_EXP) + 1 )
#	endif /* LDTS_RADIX_LG_EXACT */

#	define LDTS_LIMIT_SYMBOLS_EXPONENT_MINIMUM											\
		(LDTS_DECIMAL_DIGITS(LDTS_LIMIT_DIGITS_EXPONENT_DENORMAL) + 1)

#	define LDTS_LIMIT_SYMBOLS_EXPONENT													\
		((LDTS_LIMIT_SYMBOLS_EXPONENT_MINIMUM > LDTS_LIMIT_SYMBOLS_EXPONENT_MAXIMUM)	\
			? LDTS_LIMIT_SYMBOLS_EXPONENT_MINIMUM										\
			: LDTS_LIMIT_SYMBOLS_EXPONENT_MAXIMUM )

static_assert_m(
	INTMAX_C(4) + LDTS_LIMIT_BASE_DIGITS + LDTS_LIMIT_SYMBOLS_EXPONENT <= INT_MAX,
	"LDTS_LIMIT_LONG_DOUBLE_BUFFER does not fit in int"
);
static_assert_m(
	INTMAX_C(4) + LDTS_LIMIT_BASE_DIGITS + LDTS_LIMIT_SYMBOLS_EXPONENT <= SIZE_MAX,
	"LDTS_LIMIT_LONG_DOUBLE_BUFFER does not fit in size_t"
);

/* Maximal amount of bytes (with a '\0' terminator) required to store any long double string
 * representation produced by ldts_long_double_to_string_static
 */
#	define LDTS_LIMIT_LONG_DOUBLE_BUFFER (	\
		1	/* sign				*/			\
		+ LDTS_LIMIT_BASE_DIGITS			\
		+ 1	/* decimal point	*/			\
		+ 1	/* 'e'				*/			\
		+ LDTS_LIMIT_SYMBOLS_EXPONENT		\
		+ 1 /* \0				*/			)

/* Function:
 * convert a long double value to its decimal string representation in scientific notation
 * without a precision loss;
 * 1. Output format: [-]d[.ddd...]e[-]ddd
 * - mantissa is normalized into the [1; 10) range
 * - trailing zeroes are trimmed
 * - a positive exponent is written without a leading plus and leading zeroes
 * 2. Special Values:
 * - not-a-number	: NaN
 * - infinity		: Inf, -Inf
 * - zero			: 0e0, -0e0
 *
 * Precondition:
 * buffer_array		- points to at least LDTS_LIMIT_LONG_DOUBLE_BUFFER bytes
 *
 * Parameters:
 * buffer_array		- destination buffer
 * value			- long double value to convert
 *
 * Returns:
 *	length			- number of characters written into a buffer (without '\0' terminator)
 */
size_t ldts_long_double_to_string_static(char buffer_array[static restrict LDTS_LIMIT_LONG_DOUBLE_BUFFER], long double value);

#endif /* LONG_DOUBLE_TO_STRING_H */
