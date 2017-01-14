// Based on code by Don Bright (https://github.com/donbright/hello-mir-gles2)
// Based on code by Joe Groff (https://github.com/jckarter/hello-gl)
// Based on code by Daniel van Vugt <daniel.van.vugt@canonical.com>
// original license follows:

/*
 * Trivial GL demo; flashes the screen. Showing how simple life is with eglapp.
 *
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Daniel van Vugt <daniel.van.vugt@canonical.com>
 */

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "eglapp.h"
#include "util_file.hpp"
#if GUEST_APP
#include "util_misc.hpp"
#endif

namespace util {

static const char* string_from_GL_error(
	const GLenum error)
{
	switch (error) {
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	}

	return "unknown-gl-error";
}

bool reportGLError(FILE* f)
{
	const GLenum error = glGetError();

	if (GL_NO_ERROR == error)
		return false;

	fprintf(f, "GL error: %s\n", string_from_GL_error(error));
	return true;
}

static bool reportGLCaps(FILE* f)
{
	const GLubyte* str_version	= glGetString(GL_VERSION);
	const GLubyte* str_vendor	= glGetString(GL_VENDOR);
	const GLubyte* str_renderer	= glGetString(GL_RENDERER);
	const GLubyte* str_glsl_ver	= glGetString(GL_SHADING_LANGUAGE_VERSION);
	const GLubyte* str_exten	= glGetString(GL_EXTENSIONS);

	fprintf(f, "gl version, vendor, renderer, glsl version, extensions:"
		"\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n",
		(const char*) str_version,
		(const char*) str_vendor,
		(const char*) str_renderer,
		(const char*) str_glsl_ver,
		(const char*) str_exten);

	GLint params[2]; // we won't need more than 2

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, params);
	fprintf(f, "GL_MAX_TEXTURE_SIZE: %d\n", params[0]);

	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, params);
	fprintf(f, "GL_MAX_CUBE_MAP_TEXTURE_SIZE: %d\n", params[0]);

	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, params);
	fprintf(f, "GL_MAX_VIEWPORT_DIMS: %d, %d\n", params[0], params[1]);

	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, params);
	fprintf(f, "GL_MAX_RENDERBUFFER_SIZE: %d\n", params[0]);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, params);
	fprintf(f, "GL_MAX_VERTEX_ATTRIBS: %d\n", params[0]);

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, params);
	fprintf(f, "GL_MAX_VERTEX_UNIFORM_VECTORS: %d\n", params[0]);

	glGetIntegerv(GL_MAX_VARYING_VECTORS, params);
	fprintf(f, "GL_MAX_VARYING_VECTORS: %d\n", params[0]);

	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, params);
	fprintf(f, "GL_MAX_FRAGMENT_UNIFORM_VECTORS: %d\n", params[0]);

	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, params);
	fprintf(f, "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: %d\n", params[0]);

	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, params);
	fprintf(f, "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS: %d\n", params[0]);

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, params);
	fprintf(f, "GL_MAX_TEXTURE_IMAGE_UNITS: %d\n", params[0]);

	fputc('\n', f);
	return true;
}

} // namespace util

/*
 * Global data used by our render callback:
 */
static struct {
	GLuint vertex_buffer, element_buffer;
	GLuint vertex_shader, fragment_shader, program;
	
	struct {
		GLint blend_factor;
	} uniforms;

	struct {
		GLint position;
	} attributes;

	GLfloat blend_factor;
} g_resources;

/*
 * Functions for creating OpenGL objects:
 */
GLuint make_buffer(
	GLenum target,
	const void *buffer_data,
	GLsizei buffer_size)
{
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(target, buffer);
	glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
	return buffer;
}

GLuint make_shader(GLenum type, const char *filename)
{
	size_t length;
	GLchar *source = util::get_buffer_from_file(filename, length);

	if (!source)
		return 0;

	const GLint len = GLint(length);
	const GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, &len);
	glCompileShader(shader);
	free(source);

	GLint shader_ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);

	if (!shader_ok) {
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		GLchar *log = (GLchar*) alloca(sizeof(GLchar) * logLen);
		glGetShaderInfoLog(shader, logLen, NULL, log);
		fprintf(stderr, "Failed to compile %s:\n%*s\n", filename, logLen, log);
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
	GLint program_ok;
	GLuint program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
	if (!program_ok) {
		fprintf(stderr, "Failed to link shader program\n");
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

/*
 * Data used to seed our vertex array and element array buffers:
 */
static const GLfloat g_vertex_buffer_data[] = { 
	-1.0f, -1.0f,
	 1.0f, -1.0f,
	-1.0f,	1.0f,
	 1.0f,	1.0f
};
static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };

/*
 * Load and create all of our resources:
 */
static int make_resources(void)
{
	g_resources.vertex_buffer = make_buffer(
		GL_ARRAY_BUFFER,
		g_vertex_buffer_data,
		sizeof(g_vertex_buffer_data)
	);
	g_resources.element_buffer = make_buffer(
		GL_ELEMENT_ARRAY_BUFFER,
		g_element_buffer_data,
		sizeof(g_element_buffer_data)
	);

	g_resources.vertex_shader = make_shader(
		GL_VERTEX_SHADER, "hello-gl.v.glsl");

	if (g_resources.vertex_shader == 0)
		return 0;

	g_resources.fragment_shader = make_shader(
		GL_FRAGMENT_SHADER, "hello-gl.f.glsl");

	if (g_resources.fragment_shader == 0)
		return 0;

	g_resources.program = make_program(g_resources.vertex_shader, g_resources.fragment_shader);

	if (g_resources.program == 0)
		return 0;

	g_resources.uniforms.blend_factor
		= glGetUniformLocation(g_resources.program, "blend_factor");

	g_resources.attributes.position
		= glGetAttribLocation(g_resources.program, "position");

	return 1;
}

static uint64_t time_ns()
{
#if defined(CLOCK_MONOTONIC_RAW)
	const clockid_t clockid = CLOCK_MONOTONIC_RAW;

#else
	const clockid_t clockid = CLOCK_MONOTONIC;

#endif
	timespec t;
	clock_gettime(clockid, &t);
	return t.tv_sec * 1000000000ULL + t.tv_nsec;
}

static void update_blend_factor(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	const float nsec = t.tv_nsec * 1e-9;
	g_resources.blend_factor = sinf(nsec * float(M_PI * 2.0)) * .5f + .5f;
}

static void render(void)
{
	glUseProgram(g_resources.program);

	glUniform1f(g_resources.uniforms.blend_factor, g_resources.blend_factor);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
	glVertexAttribPointer(
		g_resources.attributes.position,  /* attribute */
		2,                                /* size */
		GL_FLOAT,                         /* type */
		GL_FALSE,                         /* normalized? */
		sizeof(GLfloat)*2,                /* stride */
		(void*)0                          /* array buffer offset */
	);
	glEnableVertexAttribArray(g_resources.attributes.position);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);
	glDrawElements(
		GL_TRIANGLE_STRIP,  /* mode */
		4,                  /* count */
		GL_UNSIGNED_SHORT,  /* type */
		(void*)0            /* element array buffer offset */
	);

	glDisableVertexAttribArray(g_resources.attributes.position);
}

int main(int argc, char **argv)
{
	if (!eglapp_init(argc, argv))
		return 1;

	util::reportGLCaps(stdout);

	fprintf(stderr, "make resources..\n");

#if GUEST_APP
	if (!hook::init_resources(argc, argv)) {
		fprintf(stderr, "Failed to load resources\n");
		return 1;
	}

#else
	if (!make_resources()) {
		fprintf(stderr, "Failed to load resources\n");
		return 1;
	}

#endif
	fprintf(stderr, "zooming..\n");

	while (eglapp_zooming()) {
		usleep(16000);
		eglapp_swap_buffers();
	}

	glViewport(GLint(0), GLint(0), GLsizei(eglapp_target_width()), GLsizei(eglapp_target_height()));

	size_t frameCount = 0;
	const uint64_t t0 = time_ns();

	while (eglapp_running()) {

#if GUEST_APP
		hook::render_frame();

#else
		update_blend_factor();
		render();

#endif
		eglapp_swap_buffers();
		frameCount++;
	}

	const double dt = (time_ns() - t0) * 1e-9;

	fprintf(stdout, "elapsed %f s, frames %llu, fps %f\n",
		dt, uint64_t(frameCount),  frameCount / dt);

#if GUEST_APP
	hook::deinit_resources();

#endif
	eglapp_shutdown();
	return 0;
}
