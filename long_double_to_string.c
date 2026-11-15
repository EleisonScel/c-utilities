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

#include "long_double_to_string.h"

#include "assert_m.h"	/* static_assert_m	*/

#include <stdbool.h>	/* bool				*/
#include <stddef.h>		/* size_t			*/
#include <stdint.h>		/* uint_least32_t	*/

#include <limits.h>		/* CHAR_BIT	*/
#include <string.h>		/* memcpy	*/
#include <float.h>		/* FLT_RADIX*/
#include <math.h>		/* signbit	*/

struct LDTS_Decimal_Data {
	unsigned char	digits_array[LDTS_LIMIT_BASE_DIGITS];
	intmax_t		exponent;
};

/* isn't necessary to be defined on the platform */
#define LDTS_UNSIGNED_MAXIMUM_32 0xFFFFFFFFUL
#define LDTS_UNSIGNED_MAXIMUM_64 0xFFFFFFFFFFFFFFFFULL

#ifndef LDTS_IS_BITS_32

#	if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__) || defined(__PPC64__)	||	\
		defined(__ppc64__) || defined(__s390x__) || (defined(__riscv) && __riscv_xlen == 64)
#		define LDTS_IS_BITS_32 0

#	elif defined(_WIN32) || defined(__i386__) || defined(__arm__) ||							\
	(defined(__riscv) && __riscv_xlen == 32)
#		define LDTS_IS_BITS_32 1

#	elif defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8
#		define LDTS_IS_BITS_32 0
#	elif defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
#		define LDTS_IS_BITS_32 1

#	elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L								\
	&& defined(UINTPTR_MAX) && UINTPTR_MAX == LDTS_UNSIGNED_MAXIMUM_64
#		define LDTS_IS_BITS_32 0
#	elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L								\
	&& defined(UINTPTR_MAX) && UINTPTR_MAX == LDTS_UNSIGNED_MAXIMUM_32
#		define LDTS_IS_BITS_32 1

#	elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L								\
	&& defined(SIZE_MAX) && (SIZE_MAX == LDTS_UNSIGNED_MAXIMUM_64)
#		define LDTS_IS_BITS_32 0
#	elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L								\
	&& defined(SIZE_MAX) && (SIZE_MAX == LDTS_UNSIGNED_MAXIMUM_32)
#		define LDTS_IS_BITS_32 1

#	else
#		ifndef LDTS_NO_WARNING_SYSTEM_BIT_DEPTH
#			if defined(_MSC_VER)
#				pragma message("Failed to determine the system bit depth, "\
						"64 used which may be unoptimal")
#			else
#				warning \
					"Failed to determine the system bit depth, 64 used which may be unoptimal"
#			endif
#		endif /* LDTS_NO_WARNING_SYSTEM_BIT_DEPTH */
#		define LDTS_IS_BITS_32 0
#	endif /* __SIZEOF_POINTER__ */

#endif /* LDTS_IS_BITS_32 */

#if LDTS_IS_BITS_32 == 0

#	if defined(__SIZEOF_INT128__)
typedef uint_least64_t					ldts_big_number_t;
typedef unsigned __int128				ldts_big_number_wide_t;
#		define LDTS_HAS_BIG_NUMBER_128	1

#	elif defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 128
typedef uint_least64_t					ldts_big_number_t;
typedef unsigned _BitInt(128)			ldts_big_number_wide_t;
#		define LDTS_HAS_BIG_NUMBER_128	1

#	else
typedef uint_least32_t					ldts_big_number_t;
typedef uint_least64_t					ldts_big_number_wide_t;
#		define LDTS_HAS_BIG_NUMBER_128	0

#	endif /* __SIZEOF_INT128__ */

#else
typedef uint_least32_t					ldts_big_number_t;
typedef uint_least64_t					ldts_big_number_wide_t;
#		define LDTS_HAS_BIG_NUMBER_128	0

#endif /* LDTS_IS_BITS_32 == 0 */

#if LDTS_HAS_BIG_NUMBER_128 == 1
#	define LDTS_BIG_NUMBER_SCALE			64
#	define LDTS_BIG_NUMBER_DIGIT_MAXIMUM	((ldts_big_number_t) LDTS_UNSIGNED_MAXIMUM_64)
#else
#	define LDTS_BIG_NUMBER_SCALE			32
#	define LDTS_BIG_NUMBER_DIGIT_MAXIMUM	((ldts_big_number_t) LDTS_UNSIGNED_MAXIMUM_32)
#endif /* __SIZEOF_INT128__ */

static_assert_m(
	sizeof(ldts_big_number_t) * CHAR_BIT >= LDTS_BIG_NUMBER_SCALE,
	"the big number must be at least specified amount of bytes"
);
static_assert_m(
	sizeof(ldts_big_number_wide_t) * CHAR_BIT >= 2 * LDTS_BIG_NUMBER_SCALE,
	"the wide big number must be at least twice as wide as big number"
);

/* only 2, 4, 8, 10, 16 FLT_RADIX supported */
#if FLT_RADIX != 2 && FLT_RADIX != 4 && FLT_RADIX != 8 && FLT_RADIX != 10 && FLT_RADIX != 16
#	error "Unsupported FLT_RADIX; guard"
#endif /* FLT_RADIX */

#define LDTS_RADIX_BITS		\
	((FLT_RADIX == 2) ? 1	\
	:(FLT_RADIX == 4) ? 2	\
	:(FLT_RADIX == 8) ? 3 : 4)

#define LDTS_RADIX_IS_POWER_OF_TWO \
	((FLT_RADIX == 2) || (FLT_RADIX == 4) || (FLT_RADIX == 8) || (FLT_RADIX == 16))

/* round_down(log_FLT_RADIX(2^LDTS_BIG_NUMBER_SCALE - 1)) */
#if LDTS_BIG_NUMBER_SCALE == 32
#	if FLT_RADIX == 2
#		define LDTS_CHUNK_DIGITS 31
#	elif FLT_RADIX == 4
#		define LDTS_CHUNK_DIGITS 15
#	elif FLT_RADIX == 8
#		define LDTS_CHUNK_DIGITS 10
#	elif FLT_RADIX == 10
#		define LDTS_CHUNK_DIGITS 9
#	elif FLT_RADIX == 16
#		define LDTS_CHUNK_DIGITS 7
#	else
#		error "Unsupported FLT_RADIX; LDTS_BIG_NUMBER_SCALE 32"
#	endif /* FLT_RADIX */
#elif LDTS_BIG_NUMBER_SCALE == 64
#	if FLT_RADIX == 2
#		define LDTS_CHUNK_DIGITS 63
#	elif FLT_RADIX == 4
#		define LDTS_CHUNK_DIGITS 31
#	elif FLT_RADIX == 8
#		define LDTS_CHUNK_DIGITS 21
#	elif FLT_RADIX == 10
#		define LDTS_CHUNK_DIGITS 19
#	elif FLT_RADIX == 16
#		define LDTS_CHUNK_DIGITS 15
#	else
#		error "Unsupported FLT_RADIX; LDTS_BIG_NUMBER_SCALE 64"
#	endif /* FLT_RADIX */
#else
#	error "Unsupported LDTS_BIG_NUMBER_SCALE"
#endif /* LDTS_BIG_NUMBER_SCALE */

#if LDTS_RADIX_IS_POWER_OF_TWO == 1
static_assert_m(
	LDTS_CHUNK_DIGITS <= INTMAX_MAX / LDTS_RADIX_BITS,
	"chunk bits is big enough to cause an undefined-behavior"
);
#	define LDTS_CHUNK_BITS (INTMAX_C(1) * LDTS_CHUNK_DIGITS * LDTS_RADIX_BITS)
static_assert_m(
	LDTS_CHUNK_BITS < LDTS_BIG_NUMBER_SCALE,
	"chunk bits is big enough to cause an undefined-behavior"
);
#endif /* LDTS_RADIX_IS_POWER_OF_TWO == 1 */

#if LDTS_BIG_NUMBER_SCALE == 32
#	if FLT_RADIX == 2
#		define LDTS_BIG_NUMBER_EXPONENT		((ldts_big_number_t) 0x80000000UL) /* 2^31 */
#	elif FLT_RADIX == 4
#		define LDTS_BIG_NUMBER_EXPONENT		((ldts_big_number_t) 0x40000000UL) /* 4^15 */
#	elif FLT_RADIX == 8
#		define LDTS_BIG_NUMBER_EXPONENT		((ldts_big_number_t) 0x40000000UL) /* 8^10 */
#	elif FLT_RADIX == 10
#		define LDTS_BIG_NUMBER_EXPONENT		((ldts_big_number_t) 0x3B9ACA00UL) /* 10^9 */
#	elif FLT_RADIX == 16
#		define LDTS_BIG_NUMBER_EXPONENT		((ldts_big_number_t) 0x10000000UL) /* 16^7 */
#	else
#		error "Unsupported FLT_RADIX; LDTS_BIG_NUMBER_SCALE 32"
#	endif /* FLT_RADIX */
#elif LDTS_BIG_NUMBER_SCALE == 64
#	if FLT_RADIX == 2
#		define LDTS_BIG_NUMBER_EXPONENT		((ldts_big_number_t) 0x8000000000000000ULL)/* 2^63 */
#	elif FLT_RADIX == 4
#		define LDTS_BIG_NUMBER_EXPONENT		((ldts_big_number_t) 0x4000000000000000ULL)/* 4^31 */
#	elif FLT_RADIX == 8
#		define LDTS_BIG_NUMBER_EXPONENT		((ldts_big_number_t) 0x8000000000000000ULL)/* 8^21 */
#	elif FLT_RADIX == 10
#		define LDTS_BIG_NUMBER_EXPONENT		((ldts_big_number_t) 0x8AC7230489E80000ULL)/* 10^19*/
#	elif FLT_RADIX == 16
#		define LDTS_BIG_NUMBER_EXPONENT		((ldts_big_number_t) 0x1000000000000000ULL)/* 16^15*/
#	else
#		error "Unsupported FLT_RADIX; LDTS_BIG_NUMBER_SCALE 64"
#	endif /* FLT_RADIX */
#else
#	error "Unsupported LDTS_BIG_NUMBER_SCALE"
#endif /* LDTS_BIG_NUMBER_SCALE */

#if LDTS_BIG_NUMBER_SCALE == 32
#	define LDTS_CHUNK_POWER_OF_5_DIGITS		13
#	define LDTS_CHUNK_POWER_OF_5_MULTIPLIER	((ldts_big_number_t) UINTMAX_C(1220703125))
#	define LDTS_CHUNK_DECIMAL_DIGITS		9
#	define LDTS_CHUNK_DECIMAL_MULTIPLIER	((ldts_big_number_t) UINTMAX_C(1000000000))
#elif LDTS_BIG_NUMBER_SCALE == 64
#	define LDTS_CHUNK_POWER_OF_5_DIGITS		27
#	define LDTS_CHUNK_POWER_OF_5_MULTIPLIER	((ldts_big_number_t) UINTMAX_C(7450580596923828125))
#	define LDTS_CHUNK_DECIMAL_DIGITS		19
#	define LDTS_CHUNK_DECIMAL_MULTIPLIER	((ldts_big_number_t) UINTMAX_C(10000000000000000000))
#else
#	error "Unsupported LDTS_BIG_NUMBER_SCALE"
#endif

static_assert_m(
	LDTS_CHUNK_DECIMAL_MULTIPLIER <= LDTS_BIG_NUMBER_DIGIT_MAXIMUM &&
	LDTS_CHUNK_DECIMAL_MULTIPLIER > LDTS_BIG_NUMBER_DIGIT_MAXIMUM / 10,
	"LDTS_CHUNK_DECIMAL_MULTIPLIER isn't the maximal power of ten fitting the big number"
);
static_assert_m(
	LDTS_CHUNK_POWER_OF_5_MULTIPLIER <= LDTS_BIG_NUMBER_DIGIT_MAXIMUM &&
	LDTS_CHUNK_POWER_OF_5_MULTIPLIER > LDTS_BIG_NUMBER_DIGIT_MAXIMUM / 5,
	"LDTS_CHUNK_POWER_OF_5_MULTIPLIER isn't the maximal power of five fitting the big number"
);

#if LDTS_RADIX_IS_POWER_OF_TWO == 1
static_assert_m(
	LDTS_BIG_NUMBER_EXPONENT == ( (uint_least64_t) 1 << LDTS_CHUNK_BITS),
	"LDTS_BIG_NUMBER_EXPONENT is not the radix power"
);
#elif FLT_RADIX == 10
static_assert_m(
	LDTS_CHUNK_DIGITS == LDTS_CHUNK_DECIMAL_DIGITS &&
	LDTS_BIG_NUMBER_EXPONENT == LDTS_CHUNK_DECIMAL_MULTIPLIER,
	"LDTS_BIG_NUMBER_EXPONENT is not the radix power"
);
#else
#	error "Unsupported FLT_RADIX; static assert"
#endif

#define LDTS_BIG_NUMBER_DOUBLE_SCALE		((long double) LDTS_BIG_NUMBER_EXPONENT)

/* log_2(10) * value approximation is wrong for value greater than 3774668 */
#define LDTS_POWER_OF_10_LIMIT			INTMAX_C(3774668)
#define LDTS_POWER_OF_10_BITS_AMOUNT(N)	(INTMAX_C(1) * (N) * 6107016 / 1838395 + 1)

#define LDTS_LIMIT_DIGITS_RADIX_NEGATIVE\
	(INTMAX_C(1) * LDBL_MANT_DIG - LDBL_MIN_EXP + LDTS_CHUNK_DIGITS - 1)

#if LDTS_RADIX_IS_POWER_OF_TWO == 1
static_assert_m(
	LDTS_LIMIT_DIGITS_RADIX_NEGATIVE <= SIZE_MAX / LDTS_RADIX_BITS,
	"The radix shift amount doesn't fit in size_t"
);
#endif

#if LDTS_RADIX_LG_EXACT == 1
#	if LDBL_MANT_DIG > LDTS_POWER_OF_10_LIMIT - LDTS_CHUNK_DIGITS - 1
#		error "Unsupported long double precision"
#	else
#		define LDTS_LIMIT_DIGITS_BITS_NEGATIVE \
				LDTS_POWER_OF_10_BITS_AMOUNT(INTMAX_C(1) * LDBL_MANT_DIG + LDTS_CHUNK_DIGITS + 1)
#		define LDTS_LIMIT_DIGITS_BITS_POSITIVE LDTS_LIMIT_DIGITS_BITS_NEGATIVE
#	endif
#else
#	define LDTS_LIMIT_SCALING_STEPS LDTS_DECIMAL_DIGITS(10 * FLT_RADIX - 1)
#	if LDBL_MAX_10_EXP > LDTS_POWER_OF_10_LIMIT - 2 ||						\
		LDTS_LIMIT_DIGITS_RADIX_NEGATIVE > (INTMAX_MAX						\
		- LDTS_POWER_OF_10_BITS_AMOUNT(LDTS_LIMIT_SCALING_STEPS + 1)) / LDTS_RADIX_BITS
#		error "Unsupported long double exponent range"
#	else
#		define LDTS_LIMIT_DIGITS_BITS_NEGATIVE								\
			(LDTS_LIMIT_DIGITS_RADIX_NEGATIVE * LDTS_RADIX_BITS +			\
			LDTS_POWER_OF_10_BITS_AMOUNT(LDTS_LIMIT_SCALING_STEPS + 1))
/* +2 is 1 for theoretical rounding and another 1 for extra digit of exponent */
#		define LDTS_LIMIT_DIGITS_BITS_POSITIVE								\
			LDTS_POWER_OF_10_BITS_AMOUNT(INTMAX_C(1) * LDBL_MAX_10_EXP + 2)
#	endif
#endif /* LDTS_RADIX_LG_EXACT */

#define LDTS_LIMIT_DIGITS_BITS_NUMBER										\
		((LDTS_LIMIT_DIGITS_BITS_POSITIVE > LDTS_LIMIT_DIGITS_BITS_NEGATIVE)\
		? LDTS_LIMIT_DIGITS_BITS_POSITIVE : LDTS_LIMIT_DIGITS_BITS_NEGATIVE)

static_assert_m(
	LDTS_LIMIT_DIGITS_BITS_NUMBER <= INTMAX_MAX - (LDTS_BIG_NUMBER_SCALE - 1),
	"LDTS_LIMIT_DIGITS would overflow"
);
/* digits needed to represent  */
#define LDTS_LIMIT_DIGITS \
	((LDTS_LIMIT_DIGITS_BITS_NUMBER + LDTS_BIG_NUMBER_SCALE - 1) / LDTS_BIG_NUMBER_SCALE)

/* the stack would overflow anyway */
static_assert_m(
	LDTS_LIMIT_DIGITS >= 1 && LDTS_LIMIT_DIGITS <= ((size_t) -1) / sizeof(ldts_big_number_t),
	"a big number does not fit into memory"
);

struct LDTS_Big_Number {
	ldts_big_number_t	digits_array[LDTS_LIMIT_DIGITS];
	size_t				length;
};

#define LDTS_LOG_2_U2(T)	( (T) >= 2u		? 1 : 0 )
#define LDTS_LOG_2_U4(T)	( (T) >= 4u		? 2 + LDTS_LOG_2_U2((T) >> 2) : LDTS_LOG_2_U2(T) )
#define LDTS_LOG_2_U8(T)	( (T) >= 16u	? 4 + LDTS_LOG_2_U4((T) >> 4) : LDTS_LOG_2_U4(T) )
#define LDTS_LOG_2_U16(T)	( (T) >= 256u	? 8 + LDTS_LOG_2_U8((T) >> 8) : LDTS_LOG_2_U8(T) )
#define LDTS_LOG_2_U32(T)	( (T) >= 65536u	? 16+ LDTS_LOG_2_U16((T)>> 16): LDTS_LOG_2_U16(T))
/* expect the unsgined value to get, -1 must signal the error */
#define LDTS_LOG_2_U64(T)										\
		(((T) <= INTMAX_C(0))									\
		? -1													\
		: ((T) * UINTMAX_C(1) >= UINTMAX_C(4294967296)			\
			? 32 + LDTS_LOG_2_U32(((T) * UINTMAX_C(1)) >> 32)	\
			: LDTS_LOG_2_U32((T) * UINTMAX_C(1))))

#define LDTS_ARRAY_STATIC_SIZE_POWER (LDTS_LOG_2_U64(LDBL_MAX_EXP - 1) + 1)

/* the stack would overflow anyway */
static_assert_m(
	LDTS_ARRAY_STATIC_SIZE_POWER >= 1 &&
	LDTS_ARRAY_STATIC_SIZE_POWER <= ((size_t) -1) / sizeof(long double),
	"powers array does not fit into memory"
);
static_assert_m(
	(LDTS_ARRAY_STATIC_SIZE_POWER - 1) < sizeof(intmax_t) * CHAR_BIT &&
	((uintmax_t) 1 << (LDTS_ARRAY_STATIC_SIZE_POWER - 1)) <= INTMAX_MAX,
	"Exponent conversion was able to cause undefined-behavior"
);

#if(defined LDTS_FORCE_C11 + defined LDTS_FORCE_WINDOWS + defined LDTS_FORCE_POSIX +\
	defined LDTS_FORCE_THREAD_LOCAL + defined LDTS_FORCE_RECOMPUTE +				\
	defined LDTS_FORCE_STATIC_ARRAY + defined LDTS_FORCE_ATOMIC ) > 1
#	error "Multiple LDTS_FORCE_* macros defined. Please select only one backend."
#endif

#define LDTS_BIGGEST_ARRAY_STATIC 21

/* since short circuit isn't guaranteed in preprocessor */
#ifdef __has_feature
#	define LDTS_HAS_FEATURE(T)		__has_feature(T)
#	define LDTS_HAVE_HAS_FEATURE	1
#else
#	define LDTS_HAS_FEATURE(T)		0
#	define LDTS_HAVE_HAS_FEATURE	0
#endif

#ifdef __has_extension
#	define LDTS_HAS_EXTENSION(T)	__has_extension(T)
#	define LDTS_HAVE_HAS_EXTENSION	1
#else
#	define LDTS_HAS_EXTENSION(T)	0
#	define LDTS_HAVE_HAS_EXTENSION	0
#endif

#ifdef __has_include
#	define LDTS_HAS_INCLUDE(T)		__has_include(T)
#	define LDTS_HAVE_HAS_INCLUDE	1
#else
#	define LDTS_HAS_INCLUDE(T)		0
#	define LDTS_HAVE_HAS_INCLUDE	0
#endif

#ifdef __has_builtin
#	define LDTS_HAS_BUILTIN(T)		__has_builtin(T)
#	define LDTS_HAVE_HAS_BUILTIN	1
#else
#	define LDTS_HAS_BUILTIN(T)		0
#	define LDTS_HAVE_HAS_BUILTIN	0
#endif

#if LDTS_HAS_INCLUDE(<threads.h>)
#	define LDTS_HAS_INCLUDE_THREADS_H	1
#endif
#if LDTS_HAS_INCLUDE(<pthread.h>)
#	define LDTS_HAS_INCLUDE_PTHREADS_H	1
#endif
#if LDTS_HAS_INCLUDE(<unistd.h>)
#	define LDTS_HAS_INCLUDE_UNISTD_H	1
#endif
#if LDTS_HAS_INCLUDE(<stdatomic.h>)
#	define LDTS_HAS_INCLUDE_STDATOMIC_H	1
#endif

#if LDTS_HAS_BUILTIN(__atomic_load_n) && LDTS_HAS_BUILTIN(__atomic_store_n) &&		\
	LDTS_HAS_BUILTIN(__atomic_compare_exchange_n) && defined(__ATOMIC_RELAXED) &&	\
	defined(__ATOMIC_RELEASE) && defined(__ATOMIC_ACQUIRE) && defined(__ATOMIC_ACQ_REL)
#		define LDTS_HAS_ATOMIC_BUILTINS 1
#endif

/* external definition force */
#if defined LDTS_FORCE_STATIC_ARRAY
#	if (FLT_RADIX != 2 && FLT_RADIX != 4 && FLT_RADIX != 8 && FLT_RADIX != 10 && FLT_RADIX != 16)
#		error "No suitable static table exists; remove LDTS_FORCE_STATIC_ARRAY"
#	elif LDTS_ARRAY_STATIC_SIZE_POWER > LDTS_BIGGEST_ARRAY_STATIC
#		error "Table capacity exceeded; remove LDTS_FORCE_STATIC_ARRAY"
#	endif
#	define LDTS_BACKEND_STATIC_ARRAY

#elif defined LDTS_FORCE_ATOMIC
#	if defined LDTS_HAS_ATOMIC_BUILTINS
#		define LDTS_BACKEND_ATOMIC
#	elif defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 201112L &&\
		!defined __STDC_NO_ATOMICS__ && LDTS_HAS_INCLUDE_STDATOMIC_H
#		define LDTS_BACKEND_ATOMIC
#	else
#		error "No verifiable atomics; remove LDTS_FORCE_ATOMIC"
#	endif

#elif defined LDTS_FORCE_WINDOWS
#	if !defined _WIN32
#		error "The Windows backend isn't available; remove LDTS_FORCE_WINDOWS"
#	elif !defined _WIN32_WINNT
#		error "_WIN32_WINNT isn't defined; define _WIN32_WINNT as 0x0600 or change FORCE macros"
#	elif (_WIN32_WINNT + 0) < 0x0600
#		error "Windows backend requires at least Windows Vista (_WIN32_WINNT 0x0600); "\
			"change the value or change FORCE macros"
#	endif
#	define LDTS_BACKEND_WINDOWS

#elif defined LDTS_FORCE_C11
#	if !defined __STDC_VERSION__ || (__STDC_VERSION__ + 0L) < 201112L || \
		defined __STDC_NO_THREADS__
#		error "Standard C solution isn't available; remove LDTS_FORCE_C11"
#	elif !defined LDTS_HAS_INCLUDE_THREADS_H
#		error "threads.h isn't available; remove LDTS_FORCE_C11"
#	endif /* __STDC_VERSION__ */
#	define LDTS_BACKEND_STANDARD

#elif defined LDTS_FORCE_POSIX
#	if !defined LDTS_HAS_INCLUDE_PTHREADS_H
#		error "pthread.h isn't available; remove LDTS_FORCE_POSIX"
#	elif !defined _WIN32
#		if !defined LDTS_HAS_INCLUDE_UNISTD_H
#			error "unistd.h isn't available; remove LDTS_FORCE_POSIX"
#		endif
#		include <unistd.h>
#		if !defined _POSIX_THREADS || (_POSIX_THREADS + 0) < 0
#			error "POSIX threads aren't available; remove LDTS_FORCE_POSIX"
#		endif
#	endif /* LDTS_HAS_INCLUDE_PTHREADS_H */
#	define LDTS_BACKEND_POSIX

#elif defined LDTS_FORCE_THREAD_LOCAL
#	if !defined __STDC_VERSION__ || (__STDC_VERSION__ + 0L) < 201112L
#		error "Thread local requires at least C11; remove LDTS_FORCE_THREAD_LOCAL"
#	endif
#	define LDTS_BACKEND_THREAD_LOCAL

#elif defined LDTS_FORCE_RECOMPUTE
#	define LDTS_BACKEND_RECOMPUTE

/* auto choose */
#elif LDTS_ARRAY_STATIC_SIZE_POWER <= LDTS_BIGGEST_ARRAY_STATIC &&\
		(FLT_RADIX == 2 || FLT_RADIX == 4 || FLT_RADIX == 8 || FLT_RADIX == 10 || FLT_RADIX == 16)
#	define LDTS_BACKEND_STATIC_ARRAY

#elif defined _WIN32 && defined _WIN32_WINNT && (_WIN32_WINNT + 0) >= 0x0600
#	define LDTS_BACKEND_WINDOWS

#elif defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 201112L &&\
		!defined __STDC_NO_THREADS__ && defined LDTS_HAS_INCLUDE_THREADS_H
#	define LDTS_BACKEND_STANDARD

#else
#	if !defined _WIN32 && defined LDTS_HAS_INCLUDE_PTHREADS_H && defined LDTS_HAS_INCLUDE_UNISTD_H
#		include <unistd.h>
#		if defined _POSIX_THREADS && (_POSIX_THREADS + 0) >= 0
#			define LDTS_BACKEND_POSIX
#		endif
#	endif /* _WIN32 */
#	ifndef LDTS_BACKEND_POSIX
#		if defined LDTS_HAS_ATOMIC_BUILTINS ||									\
			(defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 201112L &&	\
			!defined __STDC_NO_ATOMICS__ && LDTS_HAS_INCLUDE_STDATOMIC_H)
#			define LDTS_BACKEND_ATOMIC
#		elif defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 201112L
#			define LDTS_BACKEND_THREAD_LOCAL
#		else
#			define LDTS_BACKEND_RECOMPUTE
#		endif /* __STDC_VERSION__ */
#	endif
#endif

/* only 2, 4, 8, 10, 16 FLT_RADIX supported */
#ifdef LDTS_BACKEND_STATIC_ARRAY
static_assert_m(
	LDTS_ARRAY_STATIC_SIZE_POWER >= 1,
	"Array length must be at least 1 to cover normalization logic"
);

#	if FLT_RADIX == 2
/* 37 * log_2(10) ≈ 122.9 */
#		define LDTS_MAX_EXP_MINIMAL			123
#		define LDTS_ARRAY_STATIC_VALUE_1	0x1p+1L
#		define LDTS_ARRAY_STATIC_VALUE_2	0x1p+2L
#		define LDTS_ARRAY_STATIC_VALUE_3	0x1p+4L
#		define LDTS_ARRAY_STATIC_VALUE_4	0x1p+8L
#		define LDTS_ARRAY_STATIC_VALUE_5	0x1p+16L
#		define LDTS_ARRAY_STATIC_VALUE_6	0x1p+32L
#		define LDTS_ARRAY_STATIC_VALUE_7	0x1p+64L

/* optional values */
#		define LDTS_ARRAY_STATIC_VALUE_8	0x1p+128L
#		define LDTS_ARRAY_STATIC_VALUE_9	0x1p+256L
#		define LDTS_ARRAY_STATIC_VALUE_10	0x1p+512L
#		define LDTS_ARRAY_STATIC_VALUE_11	0x1p+1024L
#		define LDTS_ARRAY_STATIC_VALUE_12	0x1p+2048L
#		define LDTS_ARRAY_STATIC_VALUE_13	0x1p+4096L
#		define LDTS_ARRAY_STATIC_VALUE_14	0x1p+8192L
#		define LDTS_ARRAY_STATIC_VALUE_15	0x1p+16384L
#		define LDTS_ARRAY_STATIC_VALUE_16	0x1p+32768L
#		define LDTS_ARRAY_STATIC_VALUE_17	0x1p+65536L
#		define LDTS_ARRAY_STATIC_VALUE_18	0x1p+131072L
#		define LDTS_ARRAY_STATIC_VALUE_19	0x1p+262144L
#		define LDTS_ARRAY_STATIC_VALUE_20	0x1p+524288L
#		define LDTS_ARRAY_STATIC_VALUE_21	0x1p+1048576L

#	elif FLT_RADIX == 4
/* 37 * log_4(10) ≈ 61.4 */
#		define LDTS_MAX_EXP_MINIMAL			62
#		define LDTS_ARRAY_STATIC_VALUE_1	0x1p+2L
#		define LDTS_ARRAY_STATIC_VALUE_2	0x1p+4L
#		define LDTS_ARRAY_STATIC_VALUE_3	0x1p+8L
#		define LDTS_ARRAY_STATIC_VALUE_4	0x1p+16L
#		define LDTS_ARRAY_STATIC_VALUE_5	0x1p+32L
#		define LDTS_ARRAY_STATIC_VALUE_6	0x1p+64L
/* optional values */
#		define LDTS_ARRAY_STATIC_VALUE_7	0x1p+128L
#		define LDTS_ARRAY_STATIC_VALUE_8	0x1p+256L
#		define LDTS_ARRAY_STATIC_VALUE_9	0x1p+512L
#		define LDTS_ARRAY_STATIC_VALUE_10	0x1p+1024L
#		define LDTS_ARRAY_STATIC_VALUE_11	0x1p+2048L
#		define LDTS_ARRAY_STATIC_VALUE_12	0x1p+4096L
#		define LDTS_ARRAY_STATIC_VALUE_13	0x1p+8192L
#		define LDTS_ARRAY_STATIC_VALUE_14	0x1p+16384L
#		define LDTS_ARRAY_STATIC_VALUE_15	0x1p+32768L
#		define LDTS_ARRAY_STATIC_VALUE_16	0x1p+65536L
#		define LDTS_ARRAY_STATIC_VALUE_17	0x1p+131072L
#		define LDTS_ARRAY_STATIC_VALUE_18	0x1p+262144L
#		define LDTS_ARRAY_STATIC_VALUE_19	0x1p+524288L
#		define LDTS_ARRAY_STATIC_VALUE_20	0x1p+1048576L
#		define LDTS_ARRAY_STATIC_VALUE_21	0x1p+2097152L

#	elif FLT_RADIX == 8
/* 37 * log_8(10) ≈ 40.9 */
#		define LDTS_MAX_EXP_MINIMAL			41
#		define LDTS_ARRAY_STATIC_VALUE_1	0x1p+3L
#		define LDTS_ARRAY_STATIC_VALUE_2	0x1p+6L
#		define LDTS_ARRAY_STATIC_VALUE_3	0x1p+12L
#		define LDTS_ARRAY_STATIC_VALUE_4	0x1p+24L
#		define LDTS_ARRAY_STATIC_VALUE_5	0x1p+48L
#		define LDTS_ARRAY_STATIC_VALUE_6	0x1p+96L
/* optional values */
#		define LDTS_ARRAY_STATIC_VALUE_7	0x1p+192L
#		define LDTS_ARRAY_STATIC_VALUE_8	0x1p+384L
#		define LDTS_ARRAY_STATIC_VALUE_9	0x1p+768L
#		define LDTS_ARRAY_STATIC_VALUE_10	0x1p+1536L
#		define LDTS_ARRAY_STATIC_VALUE_11	0x1p+3072L
#		define LDTS_ARRAY_STATIC_VALUE_12	0x1p+6144L
#		define LDTS_ARRAY_STATIC_VALUE_13	0x1p+12288L
#		define LDTS_ARRAY_STATIC_VALUE_14	0x1p+24576L
#		define LDTS_ARRAY_STATIC_VALUE_15	0x1p+49152L
#		define LDTS_ARRAY_STATIC_VALUE_16	0x1p+98304L
#		define LDTS_ARRAY_STATIC_VALUE_17	0x1p+196608L
#		define LDTS_ARRAY_STATIC_VALUE_18	0x1p+393216L
#		define LDTS_ARRAY_STATIC_VALUE_19	0x1p+786432L
#		define LDTS_ARRAY_STATIC_VALUE_20	0x1p+1572864L
#		define LDTS_ARRAY_STATIC_VALUE_21	0x1p+3145728L

#	elif FLT_RADIX == 10
/* 1E+37 exactly */
#		define LDTS_MAX_EXP_MINIMAL			38
#		define LDTS_ARRAY_STATIC_VALUE_1	1e1L
#		define LDTS_ARRAY_STATIC_VALUE_2	1e2L
#		define LDTS_ARRAY_STATIC_VALUE_3	1e4L
#		define LDTS_ARRAY_STATIC_VALUE_4	1e8L
#		define LDTS_ARRAY_STATIC_VALUE_5	1e16L
#		define LDTS_ARRAY_STATIC_VALUE_6	1e32L
/* optional values */
#		define LDTS_ARRAY_STATIC_VALUE_7	1e64L
#		define LDTS_ARRAY_STATIC_VALUE_8	1e128L
#		define LDTS_ARRAY_STATIC_VALUE_9	1e256L
#		define LDTS_ARRAY_STATIC_VALUE_10	1e512L
#		define LDTS_ARRAY_STATIC_VALUE_11	1e1024L
#		define LDTS_ARRAY_STATIC_VALUE_12	1e2048L
#		define LDTS_ARRAY_STATIC_VALUE_13	1e4096L
#		define LDTS_ARRAY_STATIC_VALUE_14	1e8192L
#		define LDTS_ARRAY_STATIC_VALUE_15	1e16384L
#		define LDTS_ARRAY_STATIC_VALUE_16	1e32768L
#		define LDTS_ARRAY_STATIC_VALUE_17	1e65536L
#		define LDTS_ARRAY_STATIC_VALUE_18	1e131072L
#		define LDTS_ARRAY_STATIC_VALUE_19	1e262144L
#		define LDTS_ARRAY_STATIC_VALUE_20	1e524288L
#		define LDTS_ARRAY_STATIC_VALUE_21	1e1048576L

#	elif FLT_RADIX == 16
/* 37 * log_16(10) ≈ 30.7 */
#		define LDTS_MAX_EXP_MINIMAL			31
#		define LDTS_ARRAY_STATIC_VALUE_1	0x1p+4L
#		define LDTS_ARRAY_STATIC_VALUE_2	0x1p+8L
#		define LDTS_ARRAY_STATIC_VALUE_3	0x1p+16L
#		define LDTS_ARRAY_STATIC_VALUE_4	0x1p+32L
#		define LDTS_ARRAY_STATIC_VALUE_5	0x1p+64L
/* optional values */
#		define LDTS_ARRAY_STATIC_VALUE_6	0x1p+128L
#		define LDTS_ARRAY_STATIC_VALUE_7	0x1p+256L
#		define LDTS_ARRAY_STATIC_VALUE_8	0x1p+512L
#		define LDTS_ARRAY_STATIC_VALUE_9	0x1p+1024L
#		define LDTS_ARRAY_STATIC_VALUE_10	0x1p+2048L
#		define LDTS_ARRAY_STATIC_VALUE_11	0x1p+4096L
#		define LDTS_ARRAY_STATIC_VALUE_12	0x1p+8192L
#		define LDTS_ARRAY_STATIC_VALUE_13	0x1p+16384L
#		define LDTS_ARRAY_STATIC_VALUE_14	0x1p+32768L
#		define LDTS_ARRAY_STATIC_VALUE_15	0x1p+65536L
#		define LDTS_ARRAY_STATIC_VALUE_16	0x1p+131072L
#		define LDTS_ARRAY_STATIC_VALUE_17	0x1p+262144L
#		define LDTS_ARRAY_STATIC_VALUE_18	0x1p+524288L
#		define LDTS_ARRAY_STATIC_VALUE_19	0x1p+1048576L
#		define LDTS_ARRAY_STATIC_VALUE_20	0x1p+2097152L
#		define LDTS_ARRAY_STATIC_VALUE_21	0x1p+4194304L

#	else
#		error "Unsupported FLT_RADIX; LDTS_BACKEND_STATIC_ARRAY"
#	endif /* FLT_RADIX */

static_assert_m(
	LDBL_MAX_EXP >= LDTS_MAX_EXP_MINIMAL, "long double maximal exponent violates the C standard"
);

#	define LDTS_ARRAY_STATIC_SIZE_MINIMUM (LDTS_LOG_2_U64(LDTS_MAX_EXP_MINIMAL - 1) + 1)

#	if LDTS_ARRAY_STATIC_SIZE_MINIMUM > LDTS_BIGGEST_ARRAY_STATIC
#		error "Required array length exceeds the predefined maximum"
#	endif

#	if LDTS_ARRAY_STATIC_SIZE_POWER < LDTS_ARRAY_STATIC_SIZE_MINIMUM
#		error "Array length is too small to cover the long double range"
#	endif

static const long double ldts_power_array_global[LDTS_ARRAY_STATIC_SIZE_POWER] =
{
	LDTS_ARRAY_STATIC_VALUE_1, LDTS_ARRAY_STATIC_VALUE_2, LDTS_ARRAY_STATIC_VALUE_3,
	LDTS_ARRAY_STATIC_VALUE_4, LDTS_ARRAY_STATIC_VALUE_5
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 6
	, LDTS_ARRAY_STATIC_VALUE_6
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 7
	, LDTS_ARRAY_STATIC_VALUE_7
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 8
	, LDTS_ARRAY_STATIC_VALUE_8
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 9
	, LDTS_ARRAY_STATIC_VALUE_9
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 10
	, LDTS_ARRAY_STATIC_VALUE_10
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 11
	, LDTS_ARRAY_STATIC_VALUE_11
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 12
	, LDTS_ARRAY_STATIC_VALUE_12
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 13
	, LDTS_ARRAY_STATIC_VALUE_13
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 14
	, LDTS_ARRAY_STATIC_VALUE_14
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 15
	, LDTS_ARRAY_STATIC_VALUE_15
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 16
	, LDTS_ARRAY_STATIC_VALUE_16
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 17
	, LDTS_ARRAY_STATIC_VALUE_17
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 18
	, LDTS_ARRAY_STATIC_VALUE_18
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 19
	, LDTS_ARRAY_STATIC_VALUE_19
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 20
	, LDTS_ARRAY_STATIC_VALUE_20
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER >= 21
	, LDTS_ARRAY_STATIC_VALUE_21
#	endif
#	if LDTS_ARRAY_STATIC_SIZE_POWER > LDTS_BIGGEST_ARRAY_STATIC ||\
		(LDTS_BIGGEST_ARRAY_STATIC != 21)
#		error "Array length exceeds the predefined maximum"
#	endif
};
#endif /* LDTS_BACKEND_STATIC_ARRAY */

#if defined LDTS_BACKEND_STANDARD
#	include <threads.h>

#elif defined LDTS_BACKEND_POSIX
#	include <pthread.h>

#elif defined LDTS_BACKEND_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <windows.h>

#elif defined LDTS_BACKEND_ATOMIC
#	ifndef LDTS_ATOMIC_SPIN_YIELD_FUNCTION
#		define LDTS_ATOMIC_SPIN_YIELD_FUNCTION() ((void) 0)
#	endif
#	ifndef LDTS_HAS_ATOMIC_BUILTINS
#		include <stdatomic.h>
#	endif

#	if defined LDTS_HAS_ATOMIC_BUILTINS
#		define LDTS_ATOMIC_STATE_TYPE			unsigned char
#		define LDTS_ATOMIC_LOAD_ACQUIRE(OBJECT)	__atomic_load_n(&(OBJECT), __ATOMIC_ACQUIRE)
#		define LDTS_ATOMIC_STORE_RELEASE(OBJECT, VALUE)								\
			__atomic_store_n(&(OBJECT), (VALUE), __ATOMIC_RELEASE)
#		define LDTS_ATOMIC_TRY_BEGIN(OBJECT, EXPECTED_POINTER)						\
			__atomic_compare_exchange_n(&(OBJECT), (EXPECTED_POINTER),				\
				(LDTS_ATOMIC_STATE_TYPE) LDTS_ATOMIC_STATE_INITIALIZATION,			\
				0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)
#	else
#		define LDTS_ATOMIC_STATE_TYPE			_Atomic unsigned char
#		define LDTS_ATOMIC_LOAD_ACQUIRE(OBJECT)										\
			atomic_load_explicit(&(OBJECT), memory_order_acquire)
#		define LDTS_ATOMIC_STORE_RELEASE(OBJECT, VALUE)								\
			atomic_store_explicit(&(OBJECT), (VALUE), memory_order_release)
#		define LDTS_ATOMIC_TRY_BEGIN(OBJECT, EXPECTED_POINTER)						\
			atomic_compare_exchange_strong_explicit(&(OBJECT), (EXPECTED_POINTER),	\
				(unsigned char) LDTS_ATOMIC_STATE_INITIALIZATION,					\
				memory_order_acq_rel, memory_order_acquire)
#	endif

#elif defined LDTS_BACKEND_THREAD_LOCAL
#	if defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 202311L
#		define LDTS_THREAD_LOCAL thread_local
#	else
#		define LDTS_THREAD_LOCAL _Thread_local
#	endif
#endif /* LDTS_BACKEND_STANDARD */

#if defined _MSC_VER && (LDTS_BIG_NUMBER_SCALE == 32 ||\
	((defined(_M_X64) || defined(_M_ARM64)) && LDTS_BIG_NUMBER_SCALE == 64))
#	include <intrin.h>
#endif

static void ldts_normalize(long double * restrict value_pointer, intmax_t * restrict exponent_pointer);
static void ldts_handle_value(const long double value, struct LDTS_Decimal_Data * restrict decimal_pointer);
static void ldts_scale_to_unit(struct LDTS_Big_Number * restrict dividend_pointer, struct LDTS_Big_Number * restrict divisor_pointer, intmax_t * restrict exponent_pointer);
static void ldts_digits_fill_many(struct LDTS_Big_Number * restrict dividend_pointer, struct LDTS_Big_Number * restrict divisor_pointer, struct LDTS_Decimal_Data * restrict decimal_pointer);
static void ldts_digit_last_round(struct LDTS_Big_Number * restrict dividend_pointer, const struct LDTS_Big_Number * restrict divisor_pointer, struct LDTS_Decimal_Data * restrict decimal_pointer);
static void ldts_big_numbers_shift(struct LDTS_Big_Number * restrict big_number_pointer, const size_t big_numbers_to_shift);
static void ldts_digits_fill_single(struct LDTS_Big_Number * restrict dividend_pointer, struct LDTS_Decimal_Data * restrict decimal_pointer, const ldts_big_number_t divisor_single);
static void ldts_big_numbers_subtract(struct LDTS_Big_Number * restrict minuend_pointer, const struct LDTS_Big_Number * restrict subtrahend_pointer);
static void ldts_big_number_shift_left(struct LDTS_Big_Number * restrict big_number_pointer, const size_t bits_shift_amount);
static void ldts_big_number_push_digit(struct LDTS_Big_Number * restrict big_number_pointer, const ldts_big_number_t big_number);
static void ldts_decimal_ratio_compute(const long double value, struct LDTS_Big_Number * restrict dividend_pointer, struct LDTS_Big_Number * restrict divisor_pointer, intmax_t * restrict exponent_pointer);
static void ldts_big_number_to_small_add(struct LDTS_Big_Number * restrict big_number_pointer, const ldts_big_number_t big_number);
static void ldts_big_number_to_small_multiply(struct LDTS_Big_Number * restrict big_number_pointer, const ldts_big_number_t big_number);
static void ldts_mantissa_digit_first_extract(long double * restrict value_pointer, struct LDTS_Big_Number * restrict dividend_pointer);
static void ldts_big_numbers_normalize_divisor(struct LDTS_Big_Number * restrict dividend_pointer, struct LDTS_Big_Number * restrict divisor_pointer);
static void ldts_big_number_multiply_by_power_5(struct LDTS_Big_Number * restrict big_number_pointer, intmax_t exponent);
static void ldts_big_numbers_subtract_operation(struct LDTS_Big_Number * restrict minuend_pointer, const struct LDTS_Big_Number * restrict subtrahend_pointer);
static void ldts_big_number_remainder_chunk_update(struct LDTS_Big_Number * restrict minuend_pointer, const struct LDTS_Big_Number * restrict subtrahend_pointer, const ldts_big_number_t multiplier_minuend, const ldts_big_number_t multiplier_subtrahend);

#if defined LDTS_BACKEND_STANDARD || defined LDTS_BACKEND_POSIX
static void ldts_power_array_compute_global(void);
#endif

static inline void ldts_store_digits(unsigned char digits_array[static LDTS_LIMIT_BASE_DIGITS], const size_t index, size_t chunk, ldts_big_number_t next_chunk_digits);
#ifndef LDTS_BACKEND_STATIC_ARRAY
static inline void ldts_power_array_compute(long double ldts_power_array[static LDTS_ARRAY_STATIC_SIZE_POWER]);
#endif /* LDTS_BACKEND_STATIC_ARRAY */

#if defined LDTS_BACKEND_WINDOWS
static BOOL CALLBACK ldts_power_array_callback(PINIT_ONCE once_pointer, PVOID parameter, PVOID * context_pointer);
#endif

static bool ldts_minus_check_module_write(long double * restrict value_pointer);

static int_least8_t ldts_big_number_compare(const struct LDTS_Big_Number * restrict dividend_pointer, const struct LDTS_Big_Number * restrict divisor_pointer);

static uint_least8_t ldts_leftest_bit_shift(ldts_big_number_t first_big_number);

static intmax_t ldts_unpack(long double value, struct LDTS_Big_Number * restrict dividend_pointer, intmax_t * restrict exponent_pointer);
#if LDTS_RADIX_IS_POWER_OF_TWO == 1
static intmax_t ldts_unpack_exponent(const long double value, struct LDTS_Big_Number * restrict dividend_pointer, intmax_t * restrict decimal_exponent_pointer);
static intmax_t ldts_closest_smallest_decimal_exponent(const intmax_t exponent_radix);
#endif /* LDTS_RADIX_IS_POWER_OF_TWO == 1 */

static size_t ldts_line_assemble(char buffer_array[static restrict LDTS_LIMIT_LONG_DOUBLE_BUFFER], const bool has_sign, const struct LDTS_Decimal_Data * restrict decimal_pointer);
static size_t ldts_handle_infinity(char buffer[static sizeof("-Inf")], const bool has_sign);
static size_t ldts_handle_not_a_number(char buffer[static sizeof("NaN")]);

static inline size_t ldts_chunk_size_get(const size_t digit_index);

static ldts_big_number_t ldts_big_number_divide_estimate(const struct LDTS_Big_Number * restrict dividend_pointer, const struct LDTS_Big_Number * restrict divisor_pointer, const ldts_big_number_t multiplier);
static ldts_big_number_t ldts_big_number_remainder_chunk_operation(struct LDTS_Big_Number * restrict minuend_pointer, const struct LDTS_Big_Number * restrict subtrahend_pointer, const size_t length, const ldts_big_number_t multiplier_minuend, const ldts_big_number_t multiplier_subtrahend);

static inline ldts_big_number_t ldts_chunk_multiplier_get(const size_t chunk);

#if defined LDTS_BACKEND_STANDARD || defined LDTS_BACKEND_POSIX || defined LDTS_BACKEND_WINDOWS
static long double ldts_power_array_global[LDTS_ARRAY_STATIC_SIZE_POWER] = { FLT_RADIX };
#endif
#ifndef LDTS_BACKEND_RECOMPUTE
static inline const long double * ldts_power_array_get(void);
#endif

size_t ldts_long_double_to_string_static(
		char buffer_array[static restrict LDTS_LIMIT_LONG_DOUBLE_BUFFER], long double value
	)
{
	if(	isnan(value) != 0 )
		return ldts_handle_not_a_number(buffer_array);

	const bool has_sign = ldts_minus_check_module_write(&value);

	if(	isinf(value) != 0 )
		return ldts_handle_infinity(buffer_array, has_sign);

	struct LDTS_Decimal_Data decimal;
	ldts_handle_value(value, &decimal);

	return ldts_line_assemble(buffer_array, has_sign, &decimal );
}

/* Function:
 * extract a sign bit from a long double and make the value non-negative
 *
 * Precondition:
 * value_pointer	- isn't NULL
 *
 * Parameters:
 * value_pointer	- value to check and modify
 *
 * Returns:
 * true				- value is negative; sign extracted
 * false			- value isn't negative
 */
static bool ldts_minus_check_module_write(long double * restrict value_pointer) {
	const bool has_sign = (signbit(*value_pointer) != 0);

	if(	has_sign == true )
		*value_pointer = -*value_pointer;

	return has_sign;
}

/* Function:
 * write a "NaN" string into the buffer
 *
 * Precondition:
 * buffer	- points to at least sizeof("NaN") bytes
 *
 * Parameters:
 * buffer	- destination buffer
 *
 * Returns:
 * length	- number of characters written (without '\0' terminator)
 */
static size_t ldts_handle_not_a_number(char buffer[static sizeof("NaN")]) {
	memcpy( buffer, "NaN", sizeof("NaN") );
	return sizeof("NaN") - 1;
}

/* Function:
 * write a "Inf" or "-Inf" string into the buffer
 *
 * Precondition:
 * buffer	- points to at least sizeof("-Inf") bytes
 *
 * Parameters:
 * buffer	- destination buffer
 * has_sign	- true if the value is negative
 *
 * Returns:
 * length	- number of characters written (without '\0' terminator)
 */
static size_t ldts_handle_infinity(char buffer[static sizeof("-Inf")], const bool has_sign) {
	size_t text_length;
	const char * text_pointer;
	if(	has_sign == true ) {
		text_pointer	= "-Inf";
		text_length		= sizeof("-Inf") - 1;
	} else {
		text_pointer	= "Inf";
		text_length		= sizeof("Inf") - 1;
	}
	memcpy( buffer, text_pointer, text_length + 1 );

	return text_length;
}

/* Function:
 * convert a finite non-negative long double into a decimal digit sequence and decimal exponent
 *
 * Precondition:
 * value			- finite, non-negative number
 * decimal_pointer	- isn't NULL
 *
 * Parameters:
 * value			- value to convert
 * decimal_pointer	- storage for the extracted digits and decimal exponent
 */
static void ldts_handle_value(
		const long double value, struct LDTS_Decimal_Data * restrict decimal_pointer
	)
{
	assert_m( decimal_pointer != NULL, "No decimal data storage found" );
	assert_m( value >= 0.0L, "value must be non-negative" );

	if(	value == 0.0L ) {
		*decimal_pointer = (struct LDTS_Decimal_Data) {0};
		return;
	}

	/* value = dividend * radix^exponent				*/
	/* value / 10^x == dividend / divisor			*/
	/* 1 <= (dividend / divisor) < 10 * FLT_RADIX 	*/
	struct LDTS_Big_Number dividend, divisor;
	ldts_decimal_ratio_compute( value, &dividend, &divisor, &decimal_pointer->exponent );

	ldts_scale_to_unit( &dividend, &divisor, &decimal_pointer->exponent );

	/* extract digits by decimal chunks */
	if( dividend.length == 1 && divisor.length == 1 )
		ldts_digits_fill_single(&dividend, decimal_pointer, divisor.digits_array[0]);
	else
		ldts_digits_fill_many( &dividend, &divisor, decimal_pointer);

	ldts_digit_last_round(&dividend, &divisor, decimal_pointer);
}

/* Function:
 * extract decimal digits by dividing the dividend by a scalar divisor
 *
 * Precondition:
 * dividend_pointer	- isn't NULL
 * decimal_pointer	- isn't NULL
 * divisor_single	- isn't 0
 *
 * Parameters:
 * dividend_pointer	- dividend remainder storage
 * decimal_pointer	- storage for the extracted digits
 * divisor_single	- scalar divisor
 */
static void ldts_digits_fill_single(
		struct LDTS_Big_Number * restrict dividend_pointer,
		struct LDTS_Decimal_Data * restrict decimal_pointer,
		const ldts_big_number_t divisor_single
	)
{
	for(size_t digit_decimal = 0; digit_decimal < (size_t) LDTS_LIMIT_BASE_DIGITS;)
	{
		const size_t chunk = ldts_chunk_size_get(digit_decimal);
		if(	dividend_pointer->length == 0 ) {
			memset(
				decimal_pointer->digits_array + digit_decimal,
				0,
				chunk * sizeof(*decimal_pointer->digits_array)
			);
			digit_decimal += chunk;
			continue;
		}

		const ldts_big_number_t multiplier = ldts_chunk_multiplier_get(chunk);
		ldts_big_number_t next_chunk_digits;

		if(	dividend_pointer->digits_array[0] <= LDTS_BIG_NUMBER_DIGIT_MAXIMUM / multiplier ) {
			const ldts_big_number_t scaled_dividend =
				dividend_pointer->digits_array[0] * multiplier;
			next_chunk_digits = scaled_dividend / divisor_single;
			dividend_pointer->digits_array[0] =
				(ldts_big_number_t) (scaled_dividend % divisor_single);
		} else {
			const ldts_big_number_wide_t scaled_dividend = (ldts_big_number_wide_t)
				dividend_pointer->digits_array[0] * multiplier;
			next_chunk_digits = (ldts_big_number_t) (scaled_dividend / divisor_single);
			dividend_pointer->digits_array[0] =
				(ldts_big_number_t) (scaled_dividend % divisor_single);
		}

		if(	dividend_pointer->digits_array[0] == 0 )
			dividend_pointer->length = 0;

		ldts_store_digits(decimal_pointer->digits_array, digit_decimal, chunk, next_chunk_digits);
		digit_decimal += chunk;
	}
}

/* Function:
 * extract decimal digits by dividing a multi-element dividend by a multi-element divisor
 *
 * Precondition:
 * dividend_pointer			- isn't NULL
 * divisor_pointer			- isn't NULL
 * decimal_pointer			- isn't NULL
 * divisor_pointer->length	- isn't 0
 *
 * Parameters:
 * dividend_pointer			- dividend remainder storage
 * divisor_pointer			- divisor storage
 * decimal_pointer			- storage for the extracted digits
 */
static void ldts_digits_fill_many(
		struct LDTS_Big_Number * restrict dividend_pointer,
		struct LDTS_Big_Number * restrict divisor_pointer,
		struct LDTS_Decimal_Data * restrict decimal_pointer
	)
{
	ldts_big_numbers_normalize_divisor(dividend_pointer, divisor_pointer);

	for(size_t digit_decimal = 0; digit_decimal < (size_t) LDTS_LIMIT_BASE_DIGITS;)
	{
		const size_t chunk = ldts_chunk_size_get(digit_decimal);

		ldts_big_number_t next_chunk_digits = 0;
		if(	dividend_pointer->length != 0 )
		{
			const ldts_big_number_t multiplier = ldts_chunk_multiplier_get(chunk);
			next_chunk_digits =
				ldts_big_number_divide_estimate(dividend_pointer, divisor_pointer, multiplier);

			ldts_big_number_remainder_chunk_update(
				dividend_pointer, divisor_pointer, multiplier, next_chunk_digits
			);

			for(;ldts_big_number_compare(dividend_pointer, divisor_pointer) >= 0;
					++next_chunk_digits )
				ldts_big_numbers_subtract(dividend_pointer, divisor_pointer);

			assert_m( next_chunk_digits < multiplier, "Decimal chunk is above the limit");
		}

		ldts_store_digits(decimal_pointer->digits_array, digit_decimal, chunk, next_chunk_digits);

		digit_decimal += chunk;
	}
}

/* Function:
 * store a decimal chunk into the digit array in decimal order
 *
 * Precondition:
 * digits_array		- points to at least LDTS_LIMIT_BASE_DIGITS bytes
 * index, chunk		- sum doesn't exceed LDTS_LIMIT_BASE_DIGITS
 *
 * Parameters:
 * digits_array		- digits array to store the chunk into
 * index			- position of the first digit of the chunk
 * chunk			- amount of digits in the chunk
 * next_chunk_digits- chunk value to convert into digits
 */
static inline void ldts_store_digits(
		unsigned char digits_array[static LDTS_LIMIT_BASE_DIGITS], const size_t index,
		size_t chunk, ldts_big_number_t next_chunk_digits
	)
{
	for(; chunk-- > 0; next_chunk_digits /= 10 )
		digits_array[index + chunk] = (unsigned char) (next_chunk_digits % 10);
}

/* Function:
 * get the power of ten corresponding to the chunk size
 *
 * Precondition:
 * chunk		- in [0; LDTS_CHUNK_DECIMAL_DIGITS]
 *
 * Parameters:
 * chunk		- amount of decimal digits in the chunk
 *
 * Returns:
 * multiplier	- ten raised to the power of chunk
 */
static inline ldts_big_number_t ldts_chunk_multiplier_get(const size_t chunk) {
	/* 0 is actually impossible to have, pure memory waste */
	static const ldts_big_number_t table_array[LDTS_CHUNK_DECIMAL_DIGITS + 1] = {
#if LDTS_CHUNK_DECIMAL_DIGITS >= 9
		1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
#	if LDTS_CHUNK_DECIMAL_DIGITS == 19
		, 10000000000ULL, 100000000000ULL, 1000000000000ULL, 10000000000000ULL,
		100000000000000ULL, 1000000000000000ULL, 10000000000000000ULL, 100000000000000000ULL,
		1000000000000000000ULL, 10000000000000000000ULL
#	elif LDTS_CHUNK_DECIMAL_DIGITS != 9
#		error "Unsupported chunk decimal digits"
#	endif
#else
#	error "Unsupported chunk decimal digits"
#endif
	};

	return table_array[chunk];
}

/* Function:
 * get the maximum chunk size that fits the remaining digit capacity
 *
 * Precondition:
 * digit_index	- in [0; LDTS_LIMIT_BASE_DIGITS)
 *
 * Parameters:
 * digit_index	- position of the first digit of the chunk
 *
 * Returns:
 * size			- amount of decimal digits for the next chunk
 */
static inline size_t ldts_chunk_size_get(const size_t digit_index) {
	const size_t remaining = (size_t) LDTS_LIMIT_BASE_DIGITS - digit_index;
	return (remaining < (size_t) LDTS_CHUNK_DECIMAL_DIGITS)
		? remaining
		: (size_t) LDTS_CHUNK_DECIMAL_DIGITS;
}

/* Function:
 * shift both numbers until the most significant bit of the divisor's first element is set
 *
 * Precondition:
 * dividend_pointer												- isn't NULL
 * divisor_pointer												- isn't NULL
 * divisor_pointer->length										- isn't 0
 * divisor_pointer->digits_array[divisor_pointer->length - 1]	- isn't 0
 *
 * Parameters:
 * dividend_pointer	- dividend to shift along with the divisor
 * divisor_pointer	- divisor to normalize
 */
static void ldts_big_numbers_normalize_divisor(
		struct LDTS_Big_Number * restrict dividend_pointer,
		struct LDTS_Big_Number * restrict divisor_pointer
	)
{
	/* safe a sign bit */
	const uint_least8_t bits_shift_amount = ldts_leftest_bit_shift(
		divisor_pointer->digits_array[divisor_pointer->length - 1]
	);
	if(	bits_shift_amount != 0 ) {
		ldts_big_number_shift_left( divisor_pointer, bits_shift_amount );
		ldts_big_number_shift_left(dividend_pointer, bits_shift_amount );
	}
}

/* Function:
 * round the last extracted digit using the remaining dividend (round-half-to-even)
 *
 * Precondition:
 * dividend_pointer	- isn't NULL
 * divisor_pointer	- isn't NULL
 * decimal_pointer	- isn't NULL
 *
 * Parameters:
 * dividend_pointer	- remaining dividend storage
 * divisor_pointer	- divisor storage
 * decimal_pointer	- digits storage to round
 */
static void ldts_digit_last_round(
		struct LDTS_Big_Number * restrict dividend_pointer,
		const struct LDTS_Big_Number * restrict divisor_pointer,
		struct LDTS_Decimal_Data * restrict decimal_pointer
	)
{
	if(	dividend_pointer->length > 0 )
		ldts_big_number_to_small_multiply( dividend_pointer, 2 );

	const int_least8_t is_half_to_round =
		ldts_big_number_compare(dividend_pointer, divisor_pointer);

	const bool is_last_digit_odd_to_round =
		((decimal_pointer->digits_array[LDTS_LIMIT_BASE_DIGITS - 1] & (unsigned char) 1) != 0);

	if(	is_half_to_round > 0 || (is_half_to_round == 0 && is_last_digit_odd_to_round == true) )
	{
		size_t index = (size_t) LDTS_LIMIT_BASE_DIGITS;
		for(; index > 0 && decimal_pointer->digits_array[index - 1] == 9; --index)
			decimal_pointer->digits_array[index - 1] = 0;

		if(	index > 0 )
			++decimal_pointer->digits_array[index - 1];
		else {
			decimal_pointer->digits_array[0] = 1;
			++decimal_pointer->exponent;
		}
	}
}

/* Function:
 * scale the dividend / divisor ratio into the (0; 1) range by multiplying the divisor by 10 and
 * increasing the decimal exponent on each step
 *
 * Precondition:
 * dividend_pointer	- isn't NULL
 * divisor_pointer	- isn't NULL
 * exponent_pointer	- isn't NULL
 *
 * Parameters:
 * dividend_pointer	- dividend storage
 * divisor_pointer	- divisor storage
 * exponent_pointer	- decimal exponent storage
 */
static void ldts_scale_to_unit(
		struct LDTS_Big_Number * restrict dividend_pointer,
		struct LDTS_Big_Number * restrict divisor_pointer, intmax_t * restrict exponent_pointer
	)
{
	do {
		ldts_big_number_to_small_multiply( divisor_pointer, 10 );
	} while(
		ldts_big_number_compare(dividend_pointer, divisor_pointer) >= 0 &&
		++(*exponent_pointer)
	);
}

/* Function:
 * unpack the mantissa of a long double into a big number dividend
 *
 * Precondition:
 * value			- positive number
 * dividend_pointer	- isn't NULL
 * exponent_pointer	- isn't NULL
 *
 * Parameters:
 * value			- value to unpack
 * dividend_pointer	- storage for the mantissa dividend
 * exponent_pointer	- storage for the radix exponent
 *
 * Returns:
 * mantissa digits	- amount of extracted mantissa digits after the leading one
 */
static intmax_t ldts_unpack(
		long double value, struct LDTS_Big_Number * restrict dividend_pointer,
		intmax_t * restrict exponent_pointer
	)
{
	ldts_normalize(&value, exponent_pointer);

	ldts_mantissa_digit_first_extract( &value, dividend_pointer );

	intmax_t mantissa_digits_amount = 0;
	for(; value > 0.L && mantissa_digits_amount < LDBL_MANT_DIG - 1;
			mantissa_digits_amount += LDTS_CHUNK_DIGITS )
	{
		value *= LDTS_BIG_NUMBER_DOUBLE_SCALE;
		const ldts_big_number_t mantissa_digits_chunk = value;
		value -= (long double) mantissa_digits_chunk;
		ldts_big_number_to_small_multiply( dividend_pointer, LDTS_BIG_NUMBER_EXPONENT );
		ldts_big_number_to_small_add( dividend_pointer, mantissa_digits_chunk );
	}
	assert_m(
		value == 0.L,
		"The fraction has more digits than the precision, the arithmetic isn't exact"
	);

	return mantissa_digits_amount;
}

/* Function:
 * extract the leading mantissa digit of a normalized value
 *
 * Precondition:
 * value_pointer	- isn't NULL
 *						points to a value in [1; FLT_RADIX)
 * dividend_pointer	- isn't NULL
 *
 * Parameters:
 * value_pointer	- value to extract the number integer part
 * dividend_pointer	- storage for the extracted digit
 */
static void ldts_mantissa_digit_first_extract(
		long double * restrict value_pointer, struct LDTS_Big_Number * restrict dividend_pointer
	)
{
	assert_m( value_pointer		!= NULL,"No value storage found"	);
	assert_m( dividend_pointer	!= NULL,"No dividend storage found");
	assert_m( *value_pointer	>= 1.L, "Value isn't normalized"	);

	const ldts_big_number_t mantissa_digit_first = (ldts_big_number_t) *value_pointer;
	*value_pointer -= (long double) mantissa_digit_first;

	dividend_pointer->digits_array[0] = mantissa_digit_first;
	dividend_pointer->length = 1;
}

/* Function:
 * normalize a positive value into the [1; FLT_RADIX) range and get its radix exponent
 *
 * Precondition:
 * value_pointer	- isn't NULL
 *						points to a positive value
 * exponent_pointer	- isn't NULL
 *
 * Parameters:
 * value_pointer	- storage of value to normalize
 * exponent_pointer	- storage for the radix exponent
 */
static void ldts_normalize(
		long double * restrict value_pointer, intmax_t * restrict exponent_pointer
	)
{
	assert_m( value_pointer		!=	NULL, "No value found"			);
	assert_m( exponent_pointer	!=	NULL, "No exponent found"		);
	assert_m( *value_pointer	>	0.0L, "Value must be positive"	);

#ifdef LDTS_BACKEND_RECOMPUTE
	long double power_array_mutable[LDTS_ARRAY_STATIC_SIZE_POWER] = { FLT_RADIX };
	ldts_power_array_compute(power_array_mutable);
	const long double * const power_array = power_array_mutable;
#else
	/* LDTS_ARRAY_STATIC_SIZE_POWER size */
	const long double * const power_array = ldts_power_array_get();
#endif /* LDTS_BACKEND_RECOMPUTE */

	*exponent_pointer = 0;
	if(	*value_pointer >= FLT_RADIX ) {
		for(size_t power_index = LDTS_ARRAY_STATIC_SIZE_POWER; power_index-- > 0; ) {
			if(	*value_pointer >= power_array[power_index] ) {
				*value_pointer /= power_array[power_index];
				*exponent_pointer += (intmax_t) ((uintmax_t) 1 << power_index);
			}
		}
	} else if( *value_pointer < 1.0L ) {
		long double scaled_candidate =
			*value_pointer * power_array[LDTS_ARRAY_STATIC_SIZE_POWER - 1];
		for(const intmax_t largest_power_exponent = (intmax_t)
			((uintmax_t) 1 << (LDTS_ARRAY_STATIC_SIZE_POWER - 1));
				scaled_candidate < FLT_RADIX;
					scaled_candidate *= power_array[LDTS_ARRAY_STATIC_SIZE_POWER - 1] )
		{
			*value_pointer = scaled_candidate;
			*exponent_pointer -= largest_power_exponent;
		}

		for( size_t index = LDTS_ARRAY_STATIC_SIZE_POWER; index-- > 0; ) {
			scaled_candidate = *value_pointer * power_array[index];
			if(	scaled_candidate < FLT_RADIX ) {
				*value_pointer = scaled_candidate;
				*exponent_pointer -= (intmax_t) ((uintmax_t) 1 << index);
			}
		}
	}

	assert_m(
		*value_pointer >= 1.0L && *value_pointer < FLT_RADIX,
		"The value is not scaled into the expected range"
	);
}

/* Function:
 * compare two big numbers
 *
 * Precondition:
 * first_pointer	- isn't NULL
 * second_pointer	- isn't NULL
 *
 * Parameters:
 * first_pointer	- first big number
 * second_pointer	- second big number
 *
 * Returns:
 *  1				- first is greater
 *  0				- equal
 * -1				- first is less
 */
static int_least8_t ldts_big_number_compare(
		const struct LDTS_Big_Number * restrict first_pointer,
		const struct LDTS_Big_Number * restrict second_pointer
	)
{
	if(	first_pointer->length != second_pointer->length )
		return (first_pointer->length > second_pointer->length) ? 1 : -1;

	for(size_t index = first_pointer->length; index > 0; --index ) {
		if(	first_pointer->digits_array[index - 1] != second_pointer->digits_array[index - 1])
			return( first_pointer->digits_array[index - 1] >
					second_pointer->digits_array[index - 1] )
					? 1 : -1;
	}
	return 0;
}

/* Function:
 * decompose a value into a dividend and divisor such that:
 * value = (dividend / divisor) * 10^exponent
 *
 * Precondition:
 * value			- positive number
 * dividend_pointer	- isn't NULL
 * divisor_pointer	- isn't NULL
 * exponent_pointer	- isn't NULL
 *
 * Parameters:
 * value			- value to decompose
 * dividend_pointer	- dividend storage
 * divisor_pointer	- divisor storage
 * exponent_pointer	- decimal exponent storage
 */
static void ldts_decimal_ratio_compute(
		const long double value, struct LDTS_Big_Number * restrict dividend_pointer,
		struct LDTS_Big_Number * restrict divisor_pointer,
		intmax_t * restrict exponent_pointer
	)
{
	assert_m( dividend_pointer		!= NULL,"No dividend storage found"		);
	assert_m( divisor_pointer	!= NULL,"No divisor storage found"		);
	assert_m( exponent_pointer		!= NULL,"No decimal exponent storage found"	);
	assert_m( value					> 0.0L,	"value must be positive"			);

	divisor_pointer->digits_array[0]= 1;
	divisor_pointer->length			= 1;

#if FLT_RADIX == 10
	/* excluding leading 1 */
	const intmax_t mantissa_digits_amount = ldts_unpack(
		value, dividend_pointer, exponent_pointer
	);

	if(	mantissa_digits_amount > 0 ) {
		ldts_big_number_multiply_by_power_5( divisor_pointer, mantissa_digits_amount );
		ldts_big_number_shift_left( divisor_pointer, (size_t) mantissa_digits_amount );
	}

	/* must lie in [1; 10) */
	assert_m(
		ldts_big_number_compare( dividend_pointer, divisor_pointer ) >= 0,
		"The ratio is below one"
	);

#elif LDTS_RADIX_IS_POWER_OF_TWO == 1

	const intmax_t exponent = ldts_unpack_exponent(value, dividend_pointer, exponent_pointer);

	if(	*exponent_pointer < 0 )
		ldts_big_number_multiply_by_power_5( dividend_pointer, -*exponent_pointer );
	else ldts_big_number_multiply_by_power_5(divisor_pointer,*exponent_pointer );

	const intmax_t power_of_two = exponent * LDTS_RADIX_BITS - *exponent_pointer;
	if(	power_of_two > 0 )
		ldts_big_number_shift_left( dividend_pointer, (size_t) power_of_two );
	else if ( power_of_two < 0 )
		ldts_big_number_shift_left(divisor_pointer, (size_t)-power_of_two);

	assert_m(
		ldts_big_number_compare( dividend_pointer, divisor_pointer ) >= 0,
		"The decimal exponent is too high"
	);
#else
#	error "Unsupported FLT_RADIX; ldts_decimal_ratio_compute"
#endif /* FLT_RADIX */
}

/* Function:
 * shifts a big number left by a given amount of bits
 *
 * Precondition:
 * big_number_pointer													- isn't NULL
 * bits_shift_amount													- isn't 0
 * big_number_pointer->length, bits_shift_amount / LDTS_BIG_NUMBER_SCALE- sum doesn't overflow
 *
 * Parameters:
 * big_number_pointer	- big number to shift
 * bits_shift_amount	- amount of bits to shift by
 */
static void ldts_big_number_shift_left(
		struct LDTS_Big_Number * restrict big_number_pointer, const size_t bits_shift_amount
	)
{
	assert_m( big_number_pointer!= NULL,"No big number storage found"	);
	assert_m( bits_shift_amount != 0,	"No shift found"				);

	ldts_big_numbers_shift(big_number_pointer, bits_shift_amount / (size_t)LDTS_BIG_NUMBER_SCALE);

	const size_t bits_shifts_remainder = bits_shift_amount % (size_t) LDTS_BIG_NUMBER_SCALE;
	if(	bits_shifts_remainder != 0 )
	{
		ldts_big_number_t carry = 0;

		for( size_t big_number_index = 0;
				big_number_index < big_number_pointer->length;
					++big_number_index )
		{
			const ldts_big_number_t big_number_current =
				big_number_pointer->digits_array[big_number_index];

			big_number_pointer->digits_array[big_number_index] =
				(big_number_current << bits_shifts_remainder) | carry;

			carry = big_number_current >> (LDTS_BIG_NUMBER_SCALE - bits_shifts_remainder);
		}

		if(	carry != 0 )
			ldts_big_number_push_digit( big_number_pointer, carry );
	}
}

/* Function:
 * shifts a big number left by a given amount of elements, zeroing the new low elements
 *
 * Precondition:
 * big_number_pointer								- isn't NULL
 * big_number_pointer->length, big_numbers_to_shift	- sum doesn't exceed LDTS_LIMIT_DIGITS
 *
 * Parameters:
 * big_number_pointer	- big number to shift
 * big_numbers_to_shift	- amount of elements to shift by
 */
static void ldts_big_numbers_shift(
		struct LDTS_Big_Number * restrict big_number_pointer, const size_t big_numbers_to_shift
	)
{
	assert_m( big_number_pointer != NULL,"No big number storage found" );
	assert_m(
		big_number_pointer->length + big_numbers_to_shift <= LDTS_LIMIT_DIGITS,
		"A big number out of its storage"
	);

	if(	big_numbers_to_shift == 0 )
		return;

	memmove(
		big_number_pointer->digits_array + big_numbers_to_shift,
		big_number_pointer->digits_array,
		big_number_pointer->length * sizeof(*big_number_pointer->digits_array)
	);
	memset(
		big_number_pointer->digits_array, 0,
		big_numbers_to_shift * sizeof(*big_number_pointer->digits_array)
	);
	big_number_pointer->length += big_numbers_to_shift;
}

#if LDTS_RADIX_IS_POWER_OF_TWO == 1
/* Function:
 * unpack a value into a mantissa dividend with its radix and decimal exponents 
 * value = dividend * FLT_RADIX^exponent
 * value / 10^x is in [1; 10 * FLT_RADIX)
 *
 * Precondition:
 * value					- finite, positive
 * dividend_pointer			- isn't NULL
 * decimal_exponent_pointer	- isn't NULL
 *
 * Parameters:
 * value					- value to unpack
 * dividend_pointer			- mantissa dividend storage
 * decimal_exponent_pointer	- decimal exponent storage
 *
 * Returns:
 * radix exponent			- radix exponent packing the mantissa tightest into chunks
 */
static intmax_t ldts_unpack_exponent(
		const long double value,
		struct LDTS_Big_Number * restrict dividend_pointer,
		intmax_t * restrict decimal_exponent_pointer
	)
{
	intmax_t closest_smallest_radix_exponent;

	const intmax_t mantissa_digits_amount = ldts_unpack(
		value, dividend_pointer, &closest_smallest_radix_exponent
	);

	*decimal_exponent_pointer =
		ldts_closest_smallest_decimal_exponent(closest_smallest_radix_exponent);

	return closest_smallest_radix_exponent - mantissa_digits_amount;
}

/* Function:
 * compute the smallest decimal exponent whose power doesn't exceed the given radix power
 *
 * Parameters:
 * exponent_radix	- radix exponent
 *
 * Returns:
 * exponent			- decimal exponent where 10^exponent <= FLT_RADIX^exponent_radix
 */
static intmax_t ldts_closest_smallest_decimal_exponent(const intmax_t exponent_radix) {
	return (exponent_radix >= 0)
		? LDTS_ROUND_DOWN_LG_RADIX(exponent_radix)
		:-LDTS_ROUND_DOWN_LG_RADIX(-exponent_radix) - 1;
}
#endif /* LDTS_RADIX_IS_POWER_OF_TWO */

/* Function:
 * multiply every element of a big number by a scalar
 *
 * Precondition:
 * big_number_pointer	- isn't NULL
 *
 * Parameters:
 * big_number_pointer	- big number to multiply
 * big_number			- scalar multiplier
 */
static void ldts_big_number_to_small_multiply(
		struct LDTS_Big_Number * restrict big_number_pointer, const ldts_big_number_t big_number
	)
{
	assert_m( big_number_pointer != NULL, "No big number storage found" );

	ldts_big_number_wide_t carry = 0;
	for(size_t index = 0; index < big_number_pointer->length; ++index )
	{
		const ldts_big_number_wide_t product = (ldts_big_number_wide_t)
			big_number_pointer->digits_array[index] * big_number + carry;
		big_number_pointer->digits_array[index] =
			(ldts_big_number_t) (product & LDTS_BIG_NUMBER_DIGIT_MAXIMUM);
		carry = product >> LDTS_BIG_NUMBER_SCALE;
	}

	if(	carry != 0 )
		ldts_big_number_push_digit( big_number_pointer, (ldts_big_number_t) carry );
}

/* Function:
 * add a scalar addend to a big number
 *
 * Precondition:
 * big_number_pointer	- isn't NULL
 *
 * Parameters:
 * big_number_pointer	- big number to add to
 * big_number			- scalar addend
 */
static void ldts_big_number_to_small_add(
		struct LDTS_Big_Number * restrict big_number_pointer, const ldts_big_number_t big_number
	)
{
#if LDTS_IS_BITS_32 == 1
	ldts_big_number_t carry = big_number;
	for(size_t index = 0; carry != 0 && index < big_number_pointer->length; ++index )
	{
		const ldts_big_number_t big_number_current	= big_number_pointer->digits_array[index];
		const ldts_big_number_t sum					= big_number_current + carry;
		big_number_pointer->digits_array[index]		= sum;
		/* must be fine for unsigned */
		carry = (sum < big_number_current) ? 1 : 0;
	}

	if(	carry != 0 )
		ldts_big_number_push_digit( big_number_pointer, carry );

#else

	ldts_big_number_wide_t carry = big_number;
	for(size_t index = 0; carry != 0 && index < big_number_pointer->length; ++index )
	{
		const ldts_big_number_wide_t sum =
			(ldts_big_number_wide_t) big_number_pointer->digits_array[index] + carry;
		/* mask is needed here */
		big_number_pointer->digits_array[index] =
			(ldts_big_number_t) (sum & LDTS_BIG_NUMBER_DIGIT_MAXIMUM);
		carry = sum >> LDTS_BIG_NUMBER_SCALE;
	}

	if(	carry != 0 )
		ldts_big_number_push_digit( big_number_pointer, (ldts_big_number_t) carry );
#endif
}

/* Function:
 * multiply a big number by five raised to the specified exponent
 *
 * Precondition:
 * big_number_pointer	- big number to multiply
 * exponent				- non-negative
 *
 * Parameters:
 * big_number_pointer	- big number to multiply
 * exponent				- power of five exponent
 */
static void ldts_big_number_multiply_by_power_5(
		struct LDTS_Big_Number * restrict big_number_pointer, intmax_t exponent
	)
{
	assert_m( big_number_pointer!= NULL,"No big number storage found"	);
	assert_m( exponent			>= 0,	"The exponent must be positive"	);


	for(; exponent >= LDTS_CHUNK_POWER_OF_5_DIGITS; exponent -= LDTS_CHUNK_POWER_OF_5_DIGITS )
		ldts_big_number_to_small_multiply( big_number_pointer, LDTS_CHUNK_POWER_OF_5_MULTIPLIER );

	if(	exponent > 0 ) {
		static const ldts_big_number_t table_array[LDTS_CHUNK_POWER_OF_5_DIGITS] = {
#if LDTS_CHUNK_POWER_OF_5_DIGITS >= 13
			1, 5, 25, 125, 625, 3125, 15625, 78125, 390625, 1953125, 9765625, 48828125, 244140625
#	if LDTS_CHUNK_POWER_OF_5_DIGITS == 27
			, 1220703125ULL, 6103515625ULL, 30517578125ULL, 152587890625ULL, 762939453125ULL,
			3814697265625ULL, 19073486328125ULL, 95367431640625ULL, 476837158203125ULL,
			2384185791015625ULL, 11920928955078125ULL, 59604644775390625ULL,
			298023223876953125ULL, 1490116119384765625ULL
#	elif LDTS_CHUNK_POWER_OF_5_DIGITS != 13
#		error "Unsupported chunk power of 5 digits"
#	endif
#else
#	error "Unsupported chunk power of 5 digits"
#endif
		};
		ldts_big_number_to_small_multiply( big_number_pointer, table_array[exponent] );
	}
}

/* Function:
 * append a scalar element to the end of a big number
 *
 * Precondition:
 * big_number_pointer			- isn't NULL
 * big_number_pointer->length	- less than LDTS_LIMIT_DIGITS
 *
 * Parameters:
 * big_number_pointer	- big number to append to
 * big_number			- scalar element to append
 */
static void ldts_big_number_push_digit(
		struct LDTS_Big_Number * restrict big_number_pointer, const ldts_big_number_t big_number
	)
{
	assert_m( big_number_pointer != NULL, "No big number storage found" );
	assert_m(
		big_number_pointer->length < LDTS_LIMIT_DIGITS, "A big number out of its storage"
	);
	big_number_pointer->digits_array[big_number_pointer->length] = big_number;
	++big_number_pointer->length;
}

/* Function:
 * subtract a big number from another and trim leading zero elements
 *
 * Precondition:
 * minuend_pointer		- isn't NULL;
 *							greater or equal subtrahend
 * subtrahend_pointer	- isn't NULL
 *
 * Parameters:
 * minuend_pointer		- storage of minuend to subtract from
 * subtrahend_pointer	- storage of subtrahend to subtract
 */
static void ldts_big_numbers_subtract(
		struct LDTS_Big_Number * restrict minuend_pointer,
		const struct LDTS_Big_Number * restrict subtrahend_pointer
	)
{
	assert_m( minuend_pointer	!= NULL, "No minuend number found"		);
	assert_m( subtrahend_pointer!= NULL, "No subtrahend number found"	);
	assert_m(
		ldts_big_number_compare(minuend_pointer, subtrahend_pointer) >= 0,
		"A big number is subtracted from a smaller one"
	);

	ldts_big_numbers_subtract_operation(minuend_pointer, subtrahend_pointer);

	for(; minuend_pointer->length > 0 &&
			minuend_pointer->digits_array[minuend_pointer->length - 1] == 0;
				--minuend_pointer->length );
}

/* Function:
 * subtract a big number from another without result normalization
 *
 * Precondition:
 * minuend_pointer		- isn't NULL;
 *							greater or equal subtrahend
 * subtrahend_pointer	- isn't NULL
 *
 * Parameters:
 * minuend_pointer		- storage of minuend to subtract from
 * subtrahend_pointer	- storage of subtrahend to subtract
 */
static void ldts_big_numbers_subtract_operation(
		struct LDTS_Big_Number * restrict minuend_pointer,
		const struct LDTS_Big_Number * restrict subtrahend_pointer
	)
{
	assert_m( minuend_pointer	!= NULL, "No minuend number found"		);
	assert_m( subtrahend_pointer!= NULL, "No subtrahend number found"	);
	assert_m(
		ldts_big_number_compare(minuend_pointer, subtrahend_pointer) >= 0,
		"A big number is subtracted from a smaller one"
	);

	ldts_big_number_t	loan	= 0;
	size_t				index	= 0;
	for(; index < subtrahend_pointer->length; ++index ) {
		const ldts_big_number_t current	= minuend_pointer->digits_array[index];
		const ldts_big_number_t to_take	= subtrahend_pointer->digits_array[index];

		minuend_pointer->digits_array[index] = current - to_take - loan;
		loan = (current < to_take) || (loan != 0 && current == to_take);
	}

	for(; loan != 0 && index < minuend_pointer->length; ++index ) {
		const ldts_big_number_t big_number_current = minuend_pointer->digits_array[index];
		minuend_pointer->digits_array[index] = big_number_current - 1;
		loan = (big_number_current < 1) ? 1 : 0;
	}
}

/* Function:
 * update the remainder as:
 * minuend = minuend * multiplier_minuend - subtrahend * multiplier_subtrahend
 *
 * Precondition:
 * minuend_pointer		- isn't NULL
 * subtrahend_pointer	- isn't NULL
 * multiplier_minuend	- isn't 0
 * multiplier_subtrahend- doesn't exceed the true quotient (not to cause overflow)
 *
 * Parameters:
 * minuend_pointer		- storage of minuend to update
 * subtrahend_pointer	- subtrahend storage
 * multiplier_minuend	- multiplier for the minuend
 * multiplier_subtrahend- multiplier for the subtrahend
 */
static void ldts_big_number_remainder_chunk_update(
		struct LDTS_Big_Number * restrict minuend_pointer,
		const struct LDTS_Big_Number * restrict subtrahend_pointer,
		const ldts_big_number_t multiplier_minuend, const ldts_big_number_t multiplier_subtrahend
	)
{
	assert_m( minuend_pointer	!= NULL,"No minuend number found"	);
	assert_m( subtrahend_pointer!= NULL,"No subtrahend number found");
	assert_m( multiplier_minuend	!= 0,	"No minuend multiplier found");

	if(	multiplier_subtrahend == 0 ) {
		ldts_big_number_to_small_multiply( minuend_pointer, multiplier_minuend );
		return;
	}

	/* The minuend may be smaller before scaling, but not after */
	const size_t length = (minuend_pointer->length > subtrahend_pointer->length)
		? minuend_pointer->length
		: subtrahend_pointer->length;

	ldts_big_number_t carry = ldts_big_number_remainder_chunk_operation(
		minuend_pointer, subtrahend_pointer, length, multiplier_minuend, multiplier_subtrahend
	);

	minuend_pointer->length = length;
	if(	carry != 0 )
		ldts_big_number_push_digit(minuend_pointer, carry);

	for(;minuend_pointer->length > 0 &&
		minuend_pointer->digits_array[minuend_pointer->length - 1] == 0;
			--minuend_pointer->length);
}

/* Function:
 * update the remainder as:
 * minuend = minuend * multiplier_minuend - subtrahend * multiplier_subtrahend
 *
 * Precondition:
 * minuend_pointer		- isn't NULL
 * subtrahend_pointer	- isn't NULL
 * multiplier_minuend	- isn't 0
 * length				- longest of minuend and subtrahend
 * multiplier_subtrahend- doesn't exceed the true quotient (not to cause overflow)
 *
 * Parameters:
 * minuend_pointer		- storage of minuend to update
 * subtrahend_pointer	- subtrahend storage
 * length				- amount of elements to process
 * multiplier_minuend	- multiplier for the minuend
 * multiplier_subtrahend- multiplier for the subtrahend
 *
 * Returns:
 * carry				- elements carried out of the most significant position
 */
static ldts_big_number_t ldts_big_number_remainder_chunk_operation(
		struct LDTS_Big_Number * restrict minuend_pointer,
		const struct LDTS_Big_Number * restrict subtrahend_pointer,
		const size_t length,
		const ldts_big_number_t multiplier_minuend, const ldts_big_number_t multiplier_subtrahend
	)
{
	ldts_big_number_t loan = 0;
	ldts_big_number_t carry = 0;

	for(size_t index = 0; index < length; ++index )
	{
		ldts_big_number_wide_t part_minuend = carry;
		if(	index < minuend_pointer->length )
			part_minuend += (ldts_big_number_wide_t)
				minuend_pointer->digits_array[index] * multiplier_minuend;

		ldts_big_number_wide_t part_subtrahend = loan;
		if(	index < subtrahend_pointer->length )
			part_subtrahend += (ldts_big_number_wide_t)
				subtrahend_pointer->digits_array[index] * multiplier_subtrahend;

		if(	part_minuend >= part_subtrahend ) {
			const ldts_big_number_wide_t difference = part_minuend - part_subtrahend;
			minuend_pointer->digits_array[index] = (ldts_big_number_t)
				(difference & LDTS_BIG_NUMBER_DIGIT_MAXIMUM);

			carry= (ldts_big_number_t) (difference >> LDTS_BIG_NUMBER_SCALE);
			loan = 0;
		} else {
			const ldts_big_number_wide_t difference = part_subtrahend - part_minuend;
			minuend_pointer->digits_array[index] = -(ldts_big_number_t) difference;

			loan = (ldts_big_number_t)
				((difference + LDTS_BIG_NUMBER_DIGIT_MAXIMUM) >> LDTS_BIG_NUMBER_SCALE);
			carry= 0;
		}
	}
	assert_m( loan == 0, "The subtrahend multiplier is above the true quotient" );

	return carry;
}

/* Function:
 * count the leading zero bits before the most significant set bit
 *
 * Precondition:
 * first_big_number	- isn't 0
 *
 * Parameters:
 * first_big_number	- value to inspect
 *
 * Returns:
 * amount			- number of leading zero bits
 */
static uint_least8_t ldts_leftest_bit_shift(ldts_big_number_t first_big_number) {
	assert_m( first_big_number != 0, "First big number is zero" );

#if defined _MSC_VER && LDTS_BIG_NUMBER_SCALE == 32

	unsigned long index;

	const unsigned char extra_check = _BitScanReverse( &index, (unsigned long) first_big_number );
	assert_m( extra_check != 0, "No set bit in a nonzero found" );
	(void) extra_check;

	return (uint_least8_t) (31 - index);

#elif defined _MSC_VER && LDTS_BIG_NUMBER_SCALE == 64 && (defined(_M_X64) || defined(_M_ARM64))

	unsigned long index;

	const unsigned char extra_check =
		_BitScanReverse64( &index, (unsigned __int64) first_big_number );
	assert_m( extra_check != 0, "No set bit in a nonzero found" );
	(void) extra_check;

	return (uint_least8_t) (63 - index);

#elif LDTS_BIG_NUMBER_SCALE == 32 && \
		LDTS_HAVE_HAS_BUILTIN == 1 && LDTS_HAS_BUILTIN(__builtin_clzl)

	return (uint_least8_t) (__builtin_clzl((unsigned long) first_big_number)
			- (sizeof(unsigned long) * CHAR_BIT - 32));

#elif LDTS_BIG_NUMBER_SCALE == 64 && \
		LDTS_HAVE_HAS_BUILTIN == 1 && LDTS_HAS_BUILTIN(__builtin_clzll)

	return (uint_least8_t) (__builtin_clzll((unsigned long long) first_big_number)
			- (sizeof(unsigned long long) * CHAR_BIT - 64));

#elif defined(UINT32_MAX) && LDTS_BIG_NUMBER_SCALE == 32

	const uint32_t magic_number = 0x07C4ACDD;
	static const uint_least8_t bits_table[32] = {
		 0,  9,  1, 10, 13, 21,  2, 29,
		11, 14, 16, 18, 22, 25,  3, 30,
		 8, 12, 20, 28, 15, 17, 24,  7,
		19, 27, 23,  6, 26,  5,  4, 31
	};

	first_big_number |= first_big_number >> 1;
	first_big_number |= first_big_number >> 2;
	first_big_number |= first_big_number >> 4;
	first_big_number |= first_big_number >> 8;
	first_big_number |= first_big_number >> 16;

	return (uint_least8_t) (31 - bits_table[((uint32_t)first_big_number * magic_number) >> 27]);

#elif defined(UINT64_MAX) && LDTS_BIG_NUMBER_SCALE == 64

	const uint64_t magic_number = 0x03F79D71B4CB0A89;
	static const uint_least8_t bits_table[64] = {
		 0, 47,  1, 56, 48, 27,  2, 60, 57, 49, 41, 37, 28, 16,  3, 61,
		54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11,  4, 62,
		46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45,
		25, 39, 14, 33, 19, 30,  9, 24, 13, 18,  8, 12,  7,  6,  5, 63
	};

	first_big_number |= first_big_number >> 1;
	first_big_number |= first_big_number >> 2;
	first_big_number |= first_big_number >> 4;
	first_big_number |= first_big_number >> 8;
	first_big_number |= first_big_number >> 16;
	first_big_number |= first_big_number >> 32;

	return (uint_least8_t) (63 - bits_table[((uint64_t)first_big_number * magic_number) >> 58]);

#else

	uint_least8_t leftest_big_index = 0;
	for( uint_least8_t step = LDTS_BIG_NUMBER_SCALE >> 1; step != 0; step >>= 1 ) {
		if(	first_big_number >> (leftest_big_index + step) != 0 )
			leftest_big_index += step;
	}

	return (uint_least8_t) ((LDTS_BIG_NUMBER_SCALE - 1) - leftest_big_index);

#endif
}

/* Function:
 * estimate the next quotient chunk from the most significant elements of dividend and divisor
 *
 * Precondition:
 * dividend_pointer			- isn't NULL
 *								doesn't exceed divisor
 * divisor_pointer			- isn't NULL
 * divisor_pointer->length	- isn't 0
 *
 * Parameters:
 * dividend_pointer	- dividend storage
 * divisor_pointer	- divisor storage
 * multiplier		- scale applied to the dividend before division
 *
 * Returns:
 * chunk			- initial value of the next quotient chunk;
 *						an approximation from below of the true value
 */
static ldts_big_number_t ldts_big_number_divide_estimate(
		const struct LDTS_Big_Number * restrict dividend_pointer,
		const struct LDTS_Big_Number * restrict divisor_pointer,
		const ldts_big_number_t multiplier
	)
{
	assert_m( dividend_pointer			!= NULL,"No dividend storage found"	);
	assert_m( divisor_pointer			!= NULL,"No divisor storage found"	);
	assert_m( divisor_pointer->length	!= 0,	"No divisor found"			);
	assert_m(
		dividend_pointer->length <= divisor_pointer->length,
		"Dividend must be less than or equal to the divisor"
	);

	if(	dividend_pointer->length < divisor_pointer->length )
		return 0;

	const ldts_big_number_t dividend_first =
		dividend_pointer->digits_array[dividend_pointer->length - 1];

	const ldts_big_number_t dividend_second = (dividend_pointer->length >= 2)
		? dividend_pointer->digits_array[dividend_pointer->length - 2]
		: 0;

	const ldts_big_number_t divisor_first =
		divisor_pointer->digits_array[divisor_pointer->length - 1];

	const ldts_big_number_wide_t product = (ldts_big_number_wide_t) dividend_first * multiplier
		+ (((ldts_big_number_wide_t) dividend_second * multiplier) >> LDTS_BIG_NUMBER_SCALE);

	return (ldts_big_number_t) (product / ((ldts_big_number_wide_t) divisor_first + 1));
}

/* Function:
 * assemble the final string representation from the extracted digits and exponent
 *
 * Precondition:
 * buffer_array		- at least LDTS_LIMIT_LONG_DOUBLE_BUFFER size
 * decimal_pointer	- isn't NULL
 *
 * Parameters:
 * buffer_array		- destination buffer
 * has_sign			- value sign
 * decimal_pointer	- digits and exponent storage
 *
 * Returns:
 * length			- number of characters written (without '\0' terminator)
 */
static size_t ldts_line_assemble(
		char buffer_array[static restrict LDTS_LIMIT_LONG_DOUBLE_BUFFER], const bool has_sign,
		const struct LDTS_Decimal_Data * restrict decimal_pointer
	)
{
	assert_m( decimal_pointer != NULL, "No decimal data storage found" );

	/* verified in header */
	size_t digits_amount = (size_t) LDTS_LIMIT_BASE_DIGITS;
	while (digits_amount > 1 && decimal_pointer->digits_array[digits_amount - 1] == 0)
		--digits_amount;

	char exponent_array[LDTS_LIMIT_SYMBOLS_EXPONENT];
	size_t exponent_length = 0;

	const bool has_sign_exponent = (decimal_pointer->exponent < 0);
	uintmax_t absolute_value = (has_sign_exponent == true)
		?-(uintmax_t) decimal_pointer->exponent
		: (uintmax_t) decimal_pointer->exponent;

	do {
		assert_m(exponent_length < LDTS_LIMIT_SYMBOLS_EXPONENT, "Exponent is beyond the limit");
		exponent_array[exponent_length++] = (char) ('0' + (char) (absolute_value % 10));
		absolute_value /= 10;
	} while ( absolute_value != 0 );

	assert_m(
		(has_sign == true ? 1 : 0) + digits_amount + ((digits_amount > 1) ? 1 : 0 ) + 1 +
		(has_sign_exponent ? 1 : 0) + exponent_length
		< LDTS_LIMIT_LONG_DOUBLE_BUFFER,
		"Buffer isn't long enough to store the text"
	);

	size_t length_current = 0;
	if(	has_sign == true )
		buffer_array[length_current++] = '-';

	buffer_array[length_current++] = (char) ('0' + decimal_pointer->digits_array[0]);
	if(	digits_amount > 1 ) {
		buffer_array[length_current++] = '.';
		for(size_t index = 1; index < digits_amount; ++index)
			buffer_array[length_current++] = (char) ('0' + decimal_pointer->digits_array[index]);
	}

	buffer_array[length_current++] = 'e';
	if( has_sign_exponent == true )
		buffer_array[length_current++] = '-';

	while( exponent_length-- > 0 )
		buffer_array[length_current++] = exponent_array[exponent_length];
	buffer_array[length_current] = '\0';

	return length_current;
}

#ifndef LDTS_BACKEND_STATIC_ARRAY
/* Function:
 * fill a power array with successive squares of FLT_RADIX
 *
 * Precondition:
 * ldts_power_array		- isn't NULL;
 * ldts_power_array[0]	- equals FLT_RADIX
 *
 * Parameters:
 * ldts_power_array		- array to fill
 */
static inline void ldts_power_array_compute(
		long double ldts_power_array[static LDTS_ARRAY_STATIC_SIZE_POWER]
	)
{
	assert_m( ldts_power_array[0] == FLT_RADIX, "Power array wasn't properly initialized" );

	for(size_t index = 1; index < LDTS_ARRAY_STATIC_SIZE_POWER; ++index)
		ldts_power_array[index] = ldts_power_array[index - 1] * ldts_power_array[index - 1];
}
#endif /* LDTS_BACKEND_STATIC_ARRAY */

#if defined LDTS_BACKEND_WINDOWS
/* Function:
 * Windows once-initialization callback to compute the global power array
 *
 * Parameters:
 * (unused)
 *
 * Returns:
 * TRUE		- array initialized
 */
static BOOL CALLBACK ldts_power_array_callback(
		PINIT_ONCE once_pointer, PVOID parameter, PVOID * context_pointer
	)
{
	(void) once_pointer; (void) parameter; (void) context_pointer;

	ldts_power_array_compute(ldts_power_array_global);

	return TRUE;
}
#endif

#if defined LDTS_BACKEND_STANDARD || defined LDTS_BACKEND_POSIX
/* Function:
 * callback function for a global fill a power array function
 */
static void ldts_power_array_compute_global(void) {
	ldts_power_array_compute(ldts_power_array_global);
}
#endif

/* Function:
 * retrieve the initialized power array for the chosen backend
 *
 * Returns:
 * pointer - initialized power array address
 */
#ifndef LDTS_BACKEND_RECOMPUTE
static inline const long double * ldts_power_array_get(void) {
#	if defined LDTS_BACKEND_STANDARD

	static once_flag initialization_flag = ONCE_FLAG_INIT;
	call_once( &initialization_flag, ldts_power_array_compute_global );

	return ldts_power_array_global;

#	elif defined LDTS_BACKEND_POSIX

	static pthread_once_t initialization_flag = PTHREAD_ONCE_INIT;
	pthread_once( &initialization_flag, ldts_power_array_compute_global );

	return ldts_power_array_global;

#	elif defined LDTS_BACKEND_WINDOWS

	static INIT_ONCE initialization_flag = INIT_ONCE_STATIC_INIT;
	InitOnceExecuteOnce( &initialization_flag, ldts_power_array_callback, NULL, NULL );

	return ldts_power_array_global;

#	elif defined LDTS_BACKEND_ATOMIC

	enum {
		LDTS_ATOMIC_STATE_UNINITIALIZED,
		LDTS_ATOMIC_STATE_INITIALIZATION,
		LDTS_ATOMIC_STATE_INITIALIZED
	};

	static LDTS_ATOMIC_STATE_TYPE initialization_state = LDTS_ATOMIC_STATE_UNINITIALIZED;
	static long double ldts_power_array[LDTS_ARRAY_STATIC_SIZE_POWER] = { FLT_RADIX };

	if(	LDTS_ATOMIC_LOAD_ACQUIRE(initialization_state) != LDTS_ATOMIC_STATE_INITIALIZED )
	{
		unsigned char expected = (unsigned char) LDTS_ATOMIC_STATE_UNINITIALIZED;
		if(	LDTS_ATOMIC_TRY_BEGIN( initialization_state, &expected) != 0 ) {
			ldts_power_array_compute(ldts_power_array);
			LDTS_ATOMIC_STORE_RELEASE(initialization_state, LDTS_ATOMIC_STATE_INITIALIZED);
		} else if( expected != LDTS_ATOMIC_STATE_INITIALIZED ) {
			while(LDTS_ATOMIC_LOAD_ACQUIRE(initialization_state) != LDTS_ATOMIC_STATE_INITIALIZED)
			{
				LDTS_ATOMIC_SPIN_YIELD_FUNCTION();
			}
		}
	}

	return ldts_power_array;

#	elif defined LDTS_BACKEND_THREAD_LOCAL

	static LDTS_THREAD_LOCAL bool ldts_power_array_initialized = false;
	static LDTS_THREAD_LOCAL long double ldts_power_array[LDTS_ARRAY_STATIC_SIZE_POWER] =
		{ FLT_RADIX };

	if(	ldts_power_array_initialized != true ) {
		ldts_power_array_compute( ldts_power_array );
		ldts_power_array_initialized = true;
	}

	return ldts_power_array;

#	elif defined LDTS_BACKEND_STATIC_ARRAY

	return ldts_power_array_global;

#	endif /* LDTS_BACKEND_STANDARD */
}
#endif /* LDTS_BACKEND_RECOMPUTE */
