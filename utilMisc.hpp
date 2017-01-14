#ifndef testbed_H__
#define testbed_H__

#include <stdio.h>
#include <GLES2/gl2.h>

GLuint make_buffer(
	GLenum target,
	const void *buffer_data,
	GLsizei buffer_size);

GLuint make_shader(GLenum type, const char *filename);
GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);

namespace util {

bool reportGLError(FILE* f = stderr);
bool reportEGLError(FILE* f = stderr);

} // namespace util

namespace hook {

bool init_resources(
	const unsigned argc,
	const char* const* argv);

bool deinit_resources();
bool requires_depth();
bool render_frame();

} // namespace hook

#endif // testbed_H__
