#if PLATFORM_GL
	#include <GL/gl.h>
#else
	#include <GLES2/gl2.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>

#include "util_file.hpp"
#include "util_misc.hpp"
#include "scoped.hpp"
#include "pure_macro.hpp"

using util::scoped_ptr;

template < typename T >
class generic_free
{
public:
	void operator()(T* arg)
	{
		free(arg);
	}
};

static bool setupShaderFromString(
	const GLuint shader_name,
	const char* const source,
	const size_t length)
{
	assert(0 != source);
	assert(0 != length);

	if (GL_FALSE == glIsShader(shader_name)) {
		std::cerr << __FUNCTION__ <<
			" argument is not a valid shader object" << std::endl;
		return false;
	}

#if PLATFORM_GLES
	GLboolean compiler_present = GL_FALSE;
	glGetBooleanv(GL_SHADER_COMPILER, &compiler_present);

	if (GL_TRUE != compiler_present) {
		std::cerr << "no shader compiler present (binary only)" << std::endl;
		return false;
	}

#endif
	const GLint src_len = (GLint) length;

	glShaderSource(shader_name, 1, &source, &src_len);
	glCompileShader(shader_name);

	if (util::reportGLError()) {
		std::cerr << "cannot compile shader from source (binary only?)" << std::endl;
		return false;
	}

	GLint success = GL_FALSE;
	glGetShaderiv(shader_name, GL_COMPILE_STATUS, &success);

	if (GL_TRUE != success) {
		GLint len;
		glGetShaderiv(shader_name, GL_INFO_LOG_LENGTH, &len);
		const scoped_ptr< GLchar, generic_free > log(
			reinterpret_cast< GLchar* >(malloc(sizeof(GLchar) * len)));
		glGetShaderInfoLog(shader_name, len, NULL, log());
		std::cerr << "shader compile log: ";
		std::cerr.write(log(), std::streamsize(len));
		std::cerr << std::endl;
		return false;
	}

	return true;
}

bool util::setupShader(
	const GLuint shader_name,
	const char* const filename)
{
	assert(0 != filename);

	size_t length;
	const scoped_ptr< char, generic_free > source(get_buffer_from_file(filename, length));

	if (0 == source()) {
		std::cerr << __FUNCTION__ <<
			" failed to read shader file '" << filename << "'" << std::endl;
		return false;
	}

	return setupShaderFromString(shader_name, source(), length);
}

bool util::setupShaderWithPatch(
	const GLuint shader_name,
	const char* const filename,
	const size_t patch_count,
	const std::string* const patch)
{
	assert(0 != filename);
	assert(0 != patch);

	size_t length;
	const scoped_ptr< char, generic_free > source(get_buffer_from_file(filename, length));

	if (0 == source()) {
		std::cerr << __FUNCTION__ <<
			" failed to read shader file '" << filename << "'" << std::endl;
		return false;
	}

	std::string src_final(source(), length);
	size_t npatched = 0;

	for (size_t i = 0; i < patch_count; ++i) {
		const std::string& patch_src = patch[i * 2 + 0];
		const std::string& patch_dst = patch[i * 2 + 1];

		if (patch_src.empty() || patch_src == patch_dst)
			continue;

		const size_t len_src = patch_src.length();
		const size_t len_dst = patch_dst.length();

		std::cout << "turn: " << patch_src << "\ninto: " << patch_dst << std::endl;

		for (size_t pos = src_final.find(patch_src);
			std::string::npos != pos;
			pos = src_final.find(patch_src, pos + len_dst)) {

			src_final.replace(pos, len_src, patch_dst);
			++npatched;
		}
	}

	std::cout << "substitutions: " << npatched << std::endl;

	return setupShaderFromString(shader_name, src_final.c_str(), src_final.length());
}

bool util::setupProgram(
	const GLuint prog,
	const GLuint shader_vert,
	const GLuint shader_frag)
{
	if (GL_FALSE == glIsProgram(prog)) {
		std::cerr << __FUNCTION__ <<
			" argument is not a valid program object" << std::endl;
		return false;
	}

	if (GL_FALSE == glIsShader(shader_vert) ||
		GL_FALSE == glIsShader(shader_frag)) {

		std::cerr << __FUNCTION__ <<
			" argument is not a valid shader object" << std::endl;
		return false;
	}

	glAttachShader(prog, shader_vert);
	glAttachShader(prog, shader_frag);
	glLinkProgram(prog);

	GLint success = GL_FALSE;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);

	if (GL_TRUE != success) {
		GLint len;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
		const scoped_ptr< GLchar, generic_free > log(
			reinterpret_cast< GLchar* >(malloc(sizeof(GLchar) * len)));
		glGetProgramInfoLog(prog,  len,  NULL, log());
		std::cerr << "shader link log: ";
		std::cerr.write(log(), std::streamsize(len));
		std::cerr << std::endl;
		return false;
	}

	return true;
}

