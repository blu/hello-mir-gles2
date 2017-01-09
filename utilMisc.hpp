#ifndef testbed_H__
#define testbed_H__

#include <stdio.h>
#if defined(PLATFORM_GL)
#include <GL/gl.h>
#else
#include <GLES2/gl2.h>
#endif

namespace util {

bool
setupShader(
	const GLuint shader_name,
	const char* const filename);

bool
setupShaderWithPatch(
	const GLuint shader_name,
	const char* const filename,
	const unsigned patch_count,
	const char* const* patch);

bool
setupProgram(
	const GLuint prog_name,
	const GLuint shader_v_name,
	const GLuint shader_f_name);

bool reportGLError(FILE*);
bool reportEGLError(FILE*);

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
