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

#ifndef CLAMP_VALUES_H
#define CLAMP_VALUES_H

#include "assert_m.h"	/* assert	*/

#include <stddef.h>		/* size_t	*/
#include <stdint.h>		/* int64_t	*/

#include <math.h>		/* isfinite	*/

/* clamp values in range of [minimum, maximum], minimum shall be less than or equal to the maximum
 * for floating-point types: if minimum, maximum or value isn't finite - return unchanged value
 */

static inline int64_t clamp_int64_t( int64_t value, int64_t minimum, int64_t maximum ) {
	assert_m( minimum <= maximum, "Minimum must be less than or equal to the maximum" );
	return (value < minimum) ? minimum : (value > maximum) ? maximum : value;
}

static inline uint64_t clamp_uint64_t( uint64_t value, uint64_t minimum, uint64_t maximum ) {
	assert_m( minimum <= maximum, "Minimum must be less than or equal to the maximum" );
	return (value < minimum) ? minimum : (value > maximum) ? maximum : value;
}

static inline size_t clamp_size_t( size_t value, size_t minimum, size_t maximum ) {
	assert_m( minimum <= maximum, "Minimum must be less than or equal to the maximum" );
	return (value < minimum) ? minimum : (value > maximum) ? maximum : value;
}

static inline float clamp_float( float value, float minimum, float maximum ) {
	assert_m(
		isfinite(minimum) && isfinite(maximum) && isfinite(value),
		"Numbers to clamp must be finite"
	);
	if ( !isfinite(minimum) || !isfinite(maximum) || !isfinite(value) ) return value;
	assert_m( minimum <= maximum, "Minimum must be less than or equal to the maximum" );
	return (value < minimum) ? minimum : (value > maximum) ? maximum : value;
}

static inline double clamp_double( double value, double minimum, double maximum ) {
	assert_m(
		isfinite(minimum) && isfinite(maximum) && isfinite(value),
		"Numbers to clamp must be finite"
	);
	if ( !isfinite(minimum) || !isfinite(maximum) || !isfinite(value) ) return value;
	assert_m( minimum <= maximum, "Minimum must be less than or equal to the maximum" );
	return (value < minimum) ? minimum : (value > maximum) ? maximum : value;
}

static inline long double clamp_long_double(
		long double value, long double minimum, long double maximum
	)
{
	assert_m(
		isfinite(minimum) && isfinite(maximum) && isfinite(value),
		"Numbers to clamp must be finite"
	);
	if ( !isfinite(minimum) || !isfinite(maximum) || !isfinite(value) ) return value;
	assert_m( minimum <= maximum, "Minimum must be less than or equal to the maximum" );
	return (value < minimum) ? minimum : (value > maximum) ? maximum : value;
}

#endif /* CLAMP_VALUES_H */
