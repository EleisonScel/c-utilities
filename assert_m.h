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

#pragma	once

#ifndef	ASSERT_M_H
#define	ASSERT_M_H

#ifdef NDEBUG

#	define assert_m(			condition, message		) ((void) 0)
#	define assert_fail_m(		message					) ((void) 0)
#	define assert_check_m(		condition, message		) ((condition) ? 1 : 0 )
#	define assert_fail_check_m(	message					) (1)

#	if defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 199901L

#		define assert_mf(			condition, format, ...	) ((void) 0)
#		define assert_fail_mf(		message, ...			) ((void) 0)
#		define assert_check_mf(		condition, format, ...	) ((condition) ? 1 : 0 )
#		define assert_fail_check_mf(format, ...				) (1)

#	else

#		define assert_mf(			condition, format	)\
				assert_mf_requires_variadic_preprocessor(			condition, format	)
#		define assert_fail_mf(		message				)\
				assert_fail_mf_requires_variadic_preprocessor(		message				)
#		define assert_check_mf(		condition, message	)\
				assert_check_mf_requires_variadic_preprocessor(		condition, message	)
#		define assert_fail_check_mf(message				)\
				assert_fail_check_mf_requires_variadic_preprocessor(message				)

#	endif /* __STDC_VERSION__ >= 199901L */

#else

#	include <stdio.h>	/* fprintf	*/
#	include <stdarg.h>	/* va_list	*/
#	include <stdlib.h>	/* abort	*/

/* noreturn */
#	if defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 202311L
#		define ASSERT_M_NORETURN [[noreturn]]
#	elif defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 201112L
#		define ASSERT_M_NORETURN _Noreturn
#	elif defined __GNUC__
#		define ASSERT_M_NORETURN __attribute__((noreturn))
#	elif defined _MSC_VER
#		define ASSERT_M_NORETURN __declspec(noreturn)
#	else
#		define ASSERT_M_NORETURN
#	endif /* __STDC_VERSION__ >= 202311L */

/* inline */
#	if defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 199901L
#		define ASSERT_M_INLINE inline
#	elif defined(__GNUC__)
#		define ASSERT_M_INLINE __inline__
#	elif defined(_MSC_VER)
#		define ASSERT_M_INLINE __inline
#	else
#		define ASSERT_M_INLINE
#	endif /* __STDC_VERSION__ >= 199901L */

/* restrict */
#	if defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 199901L
#		define ASSERT_M_RESTRICT restrict
#	else
#		define ASSERT_M_RESTRICT
#	endif /* __STDC_VERSION__ >= 199901L */

/* function name */
#	if defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 199901L
#		define ASSERT_M_FUNCTION_NAME __func__
#	elif defined(__GNUC__) || defined (_MSC_VER)
#		define ASSERT_M_FUNCTION_NAME __FUNCTION__
#	else
#		define ASSERT_M_FUNCTION_NAME "unknown"
#	endif /* __STDC_VERSION__ >= 199901L */

/* printf format checking: lets the compiler verify every formatted call */
#	if defined(__GNUC__) || defined(__clang__)
#		define ASSERT_M_FORMAT_INFORMATION( format_argument_index, argument_index )	\
			__attribute__(( format( printf, format_argument_index, argument_index )	))
#	else
#		define ASSERT_M_FORMAT_INFORMATION( format_argument_index, argument_index )
#	endif

	ASSERT_M_NORETURN static ASSERT_M_INLINE void assert_fail_mf_private(
			const int						line				,
			const char * ASSERT_M_RESTRICT	condition_pointer	,
			const char * ASSERT_M_RESTRICT	file_pointer		,
			const char * ASSERT_M_RESTRICT	function_pointer	,
			const char * ASSERT_M_RESTRICT	format_pointer		, ...
		)
		ASSERT_M_FORMAT_INFORMATION( 5, 6 );

	ASSERT_M_NORETURN static ASSERT_M_INLINE void assert_fail_mf_private(
			const int						line				,
			const char * ASSERT_M_RESTRICT	condition_pointer	,
			const char * ASSERT_M_RESTRICT	file_pointer		,
			const char * ASSERT_M_RESTRICT	function_pointer	,
			const char * ASSERT_M_RESTRICT	format_pointer		, ...
		)
	{
		va_list arguments;
		va_start( arguments, format_pointer );

		(void) fprintf( stderr, "Assertion failed" );

		if(	condition_pointer != NULL )
			(void) fprintf( stderr, ": %s", condition_pointer );

		(void) fprintf( stderr,"\nFile: %s, line %d, %s\n", file_pointer, line, function_pointer );
		(void) vfprintf(stderr, format_pointer, arguments );
		(void) fprintf( stderr, "\n" );
		(void) fflush( stderr );
		va_end( arguments );
		abort();
#	if defined(__GNUC__) || defined(__clang__)
		__builtin_unreachable();
#	endif /* __GNUC__, __clang__ */
	}

	ASSERT_M_NORETURN static ASSERT_M_INLINE void assert_fail_m_private(
			const int						line,
			const char * ASSERT_M_RESTRICT	condition_pointer,
			const char * ASSERT_M_RESTRICT	message_pointer,
			const char * ASSERT_M_RESTRICT	file_pointer,
			const char * ASSERT_M_RESTRICT	function_pointer
		)
	{
		assert_fail_mf_private(
			line, condition_pointer, file_pointer, function_pointer,
			"Message: (%s)", message_pointer
		);
	}

#	define assert_m( condition, message )	\
		( (condition)						\
			? (void) 0						\
			: assert_fail_m_private(		\
				__LINE__, #condition, (message), __FILE__, ASSERT_M_FUNCTION_NAME ))

#	define assert_fail_m( message )	\
		assert_fail_m_private( __LINE__, NULL, (message), __FILE__, ASSERT_M_FUNCTION_NAME )

#	define assert_check_m( condition, message )					\
		( (condition)											\
			? 1													\
			: ( assert_fail_m_private( __LINE__, #condition,	\
				(message), __FILE__, ASSERT_M_FUNCTION_NAME		\
			), 0 ) )

#	define assert_fail_check_m( message )						\
		(assert_fail_m_private( __LINE__, NULL, (message), __FILE__, ASSERT_M_FUNCTION_NAME ), 1)

#	if defined __STDC_VERSION__ && (__STDC_VERSION__ + 0L) >= 199901L

#		define assert_mf( condition, format, ... )							\
			( (condition)													\
				? (void) 0													\
				: assert_fail_mf_private( __LINE__, #condition, __FILE__,	\
					ASSERT_M_FUNCTION_NAME, (format), __VA_ARGS__			\
				))

#		define assert_fail_mf( format, ... )						\
				assert_fail_mf_private( __LINE__, NULL, __FILE__,	\
					ASSERT_M_FUNCTION_NAME, (format), __VA_ARGS__	)

#		define assert_check_mf(	condition, format, ...	)				\
			((condition)												\
			? 1															\
			: ( assert_fail_mf_private( __LINE__, #condition, __FILE__,	\
				ASSERT_M_FUNCTION_NAME, (format), __VA_ARGS__			\
			), 0 ) )

#		define assert_fail_check_mf( format, ...	)				\
				(assert_fail_mf_private( __LINE__, NULL, __FILE__,	\
					ASSERT_M_FUNCTION_NAME, (format), __VA_ARGS__ ), 1 )

#	else /* __STDC_VERSION__ >= 199901L */

#		define assert_mf(			condition, format	)\
				assert_mf_requires_variadic_preprocessor(			condition, format	)
#		define assert_fail_mf(		message				)\
				assert_fail_mf_requires_variadic_preprocessor(		message				)
#		define assert_check_mf(		condition, message	)\
				assert_check_mf_requires_variadic_preprocessor(		condition, message	)
#		define assert_fail_check_mf(message				)\
				assert_fail_check_mf_requires_variadic_preprocessor(message				)

#	endif

#endif /* NDEBUG */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

#	define static_assert_m(		condition, message	) _Static_assert( condition, message )
#	define static_assert_fail_m(message				) static_assert_m( 0, message )

#else /* __STDC_VERSION__ >= 201112L */

/* pre-C11 fallback:
 * declare an array with a negative size when the condition is false,
 * which results in a fatal compile error.
 *
 * The message is kept for the source-level documentation
 * (it cannot reach the diagnostic output on these older compilers)
 */

#	define ASSERT_M_CAT_( function_name, identifier ) function_name ## identifier
#	define ASSERT_M_CAT( function_name, identifier ) ASSERT_M_CAT_(function_name, identifier)

#	ifdef __COUNTER__
#		define ASSERT_M_UNIQUE_IDENTIFIER __COUNTER__
#	else
#		define ASSERT_M_UNIQUE_IDENTIFIER __LINE__
#	endif /* __COUNTER__ */

#	ifdef __GNUC__
		/* __attribute__((unused)) silences -Wunsued-local-typedefs	*
		 * if the assertion is inside of function body				*/
#		define static_assert_m( condition, message )								\
			typedef char ASSERT_M_CAT( assert_m_static, ASSERT_M_UNIQUE_IDENTIFIER )\
				[ (condition) ? 1 : -1 ] __attribute__((unused))
#	else
#		define static_assert_m( condition, message )								\
			typedef char ASSERT_M_CAT( assert_m_static, ASSERT_M_UNIQUE_IDENTIFIER )\
				[ (condition) ? 1 : -1 ]
#	endif /* __GNUC__ */

#	define static_assert_fail_m( message ) static_assert_m( 0, message )

#endif /* static_assert_m */

#endif /* ASSERT_M_H */
