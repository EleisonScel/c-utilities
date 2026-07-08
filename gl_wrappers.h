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

#include "gl_utils.h"
#include "assert_m.h"

#include <stdbool.h>

#include <GL/glew.h>
#include <GL/gl.h>

static inline bool glCreateShader_wrapped( GLenum shader_type, GLuint * out_shader )
{
#ifndef NDEBUG
	assert_m( out_shader != NULL, "No shader found" );
	gl_errors_clean();
#endif/*NDEBUG*/

	*out_shader = glCreateShader( shader_type );

#ifndef NDEBUG
	return gl_errors_check( "glCreateShader" );
#else

	return true;

#endif/*NDEBUG*/
}

static inline bool glShaderSource_wrapped(
		GLuint shader, GLsizei sources_amount, const char * const * shader_sources,
		const GLint * shader_length
	)
{
#ifndef NDEBUG
	assert_m( shader			!= 0,	"No shader found"				);
	assert_m( shader_sources	!= NULL,"No shader source code found"	);
	assert_m( sources_amount	> 0,	"Wrong amount of shader sources");
	gl_errors_clean();
#endif/*NDEBUG*/

	glShaderSource( shader, sources_amount, shader_sources, shader_length );

#ifndef NDEBUG
	return gl_errors_check( "glShaderSource" );
#else

	return true;

#endif/*NDEBUG*/
}

static inline bool glCompileShader_wrapped( GLuint shader ) {
#ifndef NDEBUG
	assert_m( shader != 0, "No shader found" );
	gl_errors_clean();
#endif/*NDEBUG*/

	glCompileShader( shader );

#ifndef NDEBUG
	return gl_errors_check( "glCompileShader" );
#else

	return true;

#endif/*NDEBUG*/
}

static inline bool glCreateProgram_wrapped( GLuint * out_shader_program ) {
#ifndef NDEBUG
	assert_m( out_shader_program != NULL, "No shader program found" );
	gl_errors_clean();
#endif/*NDEBUG*/

	*out_shader_program = glCreateProgram();

#ifndef NDEBUG
	return gl_errors_check( "glCreateProgram" );
#else

	return true;

#endif/*NDEBUG*/
}

static inline bool glAttachShader_wrapped( GLuint shader_program, GLuint shader ) {
#ifndef NDEBUG
	assert_m( shader_program	!= 0, "No shader program found"	);
	assert_m( shader			!= 0, "No shader found"			);
	gl_errors_clean();
#endif/*NDEBUG*/

	glAttachShader( shader_program, shader );

#ifndef NDEBUG
	return gl_errors_check( "glAttachShader" );
#else

	return true;

#endif/*NDEBUG*/
}

static inline bool glLinkProgram_wrapped( GLuint shader_program ) {
#ifndef NDEBUG
	assert_m( shader_program != 0, "No shader program found" );
	gl_errors_clean();
#endif/*NDEBUG*/

	glLinkProgram( shader_program );

#ifndef NDEBUG
	return gl_errors_check( "glLinkProgram" );
#else

	return true;

#endif/*NDEBUG*/
}

static inline bool glDeleteShader_wrapped( GLuint shader ) {
#ifndef NDEBUG
	assert_m( shader != 0, "No shader to delete found" );
	gl_errors_clean();
#endif/*NDEBUG*/

	glDeleteShader( shader );

#ifndef NDEBUG
	return gl_errors_check( "glDeleteShader" );
#else

	return true;

#endif/*NDEBUG*/
}

static inline bool glDeleteProgram_wrapped( GLuint program ) {
#ifndef NDEBUG
	assert_m( program != 0, "No program to delete found" );
	gl_errors_clean();
#endif/*NDEBUG*/

	glDeleteProgram( program );

#ifndef NDEBUG
	return gl_errors_check( "glDeleteProgram" );
#else

	return true;

#endif/*NDEBUG*/
}
