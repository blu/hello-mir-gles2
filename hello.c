// Baased on code from Joe Groff's tutorial posted at Durian Software
// see http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2:-Hello-World:-The-Slideshow.html

// Also based on:

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

// Modified by don bright <http://github.com/donbright> 2014

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
//#include <GL/glew.h>
#include "eglapp.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define GL_RGB8 GL_RGB8_OES
#define GL_BGR 0x80e0
#include "util.h"

/*
 * Global data used by our render callback:
 */

static struct {
    GLuint vertex_buffer, element_buffer;
    GLuint textures[2];
    GLuint vertex_shader, fragment_shader, program;
    
    struct {
        GLint fade_factor;
        GLint textures[2];
    } uniforms;

    struct {
        GLint position;
    } attributes;

    GLfloat fade_factor;
} g_resources;

/*
 * Functions for creating OpenGL objects:
 */
static GLuint make_buffer(
    GLenum target,
    const void *buffer_data,
    GLsizei buffer_size
) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}

static GLuint make_texture(const char *filename)
{
    int width, height;
    void *pixels = read_tga(filename, &width, &height);
    GLuint texture;

    if (!pixels)
        return 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexImage2D(
        GL_TEXTURE_2D, 0,           /* target, level */
        GL_RGB8,                    /* internal format */
        width, height, 0,           /* width, height, border */
        GL_BGR, GL_UNSIGNED_BYTE,   /* external format, type */
        pixels                      /* pixels */
    );
    free(pixels);
    return texture;
}

/*
static void show_info_log(
    GLuint object,
    PFNGLGETSHADERIVPROC glGet__iv,
    PFNGLGETSHADERINFOLOGPROC glGet__InfoLog
)
{
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
    free(log);
}
*/

static GLuint make_shader(GLenum type, const char *filename)
{
    GLint length;
    GLchar *source = file_contents(filename, &length);
    GLuint shader;
    GLint shader_ok;

    if (!source)
        return 0;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        fprintf(stderr, "Failed to compile %s:\n", filename);
            GLchar log[1024];
            glGetShaderInfoLog(shader, sizeof log - 1, NULL, log);
            log[sizeof log - 1] = '\0';
            printf("load_shader compile failed: %s\n", log);
            glDeleteShader(shader);
            shader = 0;
        //show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
//        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint program_ok;

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        //show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
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
    -1.0f,  1.0f,
     1.0f,  1.0f
};
static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };

/*
 * Load and create all of our resources:
 */
static int make_resources(void)
{
    fprintf(stderr,"make res stage 1\n");
    fflush(stderr);
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

    g_resources.textures[1] = make_texture("hello1.tga");
    g_resources.textures[0] = make_texture("hello2.tga");

    fprintf(stderr,"make res stage 2\n");
    fflush(stderr);
    if (g_resources.textures[0] == 0 || g_resources.textures[1] == 0)
        return 0;

    fprintf(stderr,"make res stage 3\n");
    fflush(stderr);
    g_resources.vertex_shader = make_shader(
        GL_VERTEX_SHADER,
        "hello-gl.v.glsl"
    );
    if (g_resources.vertex_shader == 0)
        return 0;

    fprintf(stderr,"make res stage 4\n");
    fflush(stderr);
    g_resources.fragment_shader = make_shader(
        GL_FRAGMENT_SHADER,
        "hello-gl.f.glsl"
    );
    if (g_resources.fragment_shader == 0)
        return 0;

    fprintf(stderr,"make res stage 5\n");
    fflush(stderr);
    g_resources.program = make_program(g_resources.vertex_shader, g_resources.fragment_shader);
    if (g_resources.program == 0)
        return 0;

    fprintf(stderr,"make res stage 6\n");
    fflush(stderr);
    g_resources.uniforms.fade_factor
        = glGetUniformLocation(g_resources.program, "fade_factor");
    g_resources.uniforms.textures[0]
        = glGetUniformLocation(g_resources.program, "textures[0]");
    g_resources.uniforms.textures[1]
        = glGetUniformLocation(g_resources.program, "textures[1]");

    fprintf(stderr,"make res stage 7\n");
    fflush(stderr);
    g_resources.attributes.position
        = glGetAttribLocation(g_resources.program, "position");

    return 1;
}

/////// 2 main loop functions

static void update_fade_factor(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    long int milliseconds = t.tv_nsec / 1.0e6;
    //int milliseconds = 1;//glutGet(GLUT_ELAPSED_TIME);
    //milliseconds += 400;
    g_resources.fade_factor = sinf((float)milliseconds * 0.001f) * 0.5f + 0.5f;
    //glutPostRedisplay();
}

static void render(void)
{
    glUseProgram(g_resources.program);

    glUniform1f(g_resources.uniforms.fade_factor, g_resources.fade_factor);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[0]);
    glUniform1i(g_resources.uniforms.textures[0], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[1]);
    glUniform1i(g_resources.uniforms.textures[1], 1);

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
//    glutSwapBuffers();
}

int main(int argc, char *argv[])
{
    fprintf(stderr, "begin..\n");
    fflush(stderr);
    unsigned int width = 400, height = 300; // match .tga files

//    glewInit();
/*
    if (!GLEW_VERSION_2_0) {
        fprintf(stderr, "OpenGL 2.0 not available\n");
        return 1;
    }
*/

    fprintf(stderr, "init..\n");
    if (!mir_eglapp_init(argc, argv, &width, &height))
        return 1;

    fprintf(stderr, "make resources..\n");
    if (!make_resources()) {
        fprintf(stderr, "Failed to load resources\n");
        return 1;
    }

    fprintf(stderr, "loop..\n");
    while (mir_eglapp_running())
    {
	fprintf(stderr,"loop\n");
	update_fade_factor();
        usleep( 10000 );
	render();

//        glClearColor(1.0f, 0.0f, 0.0f, mir_eglapp_background_opacity);
//        glClear(GL_COLOR_BUFFER_BIT);
        mir_eglapp_swap_buffers();
        //sleep(1);
/*
        glClearColor(0.0f, 1.0f, 0.0f, mir_eglapp_background_opacity);
        glClear(GL_COLOR_BUFFER_BIT);
        mir_eglapp_swap_buffers();
        sleep(1);

        glClearColor(0.0f, 0.0f, 1.0f, mir_eglapp_background_opacity);
        glClear(GL_COLOR_BUFFER_BIT);
        mir_eglapp_swap_buffers();
        sleep(1);
*/
    }

    mir_eglapp_shutdown();

    return 0;
}
