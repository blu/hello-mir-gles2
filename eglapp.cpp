// Based on code by Daniel van Vugt <daniel.van.vugt@canonical.com>
// original license follows:

/*
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <EGL/egl.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <mir_toolkit/mir_client_library.h>

#include "eglapp.h"

static const char appname[] = "egldemo";

static struct {
	MirConnection* connection;
	MirBufferStream* bufferStream;
	MirSurface* surface;
	int target_width;
	int target_height;
} mir;

static struct {
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
} egl = {
	EGL_NO_DISPLAY,
	EGL_NO_SURFACE,
	EGL_NO_CONTEXT
};

static volatile sig_atomic_t running;
static volatile sig_atomic_t zooming;

int eglapp_target_width()
{
	return mir.target_width;
}

int eglapp_target_height()
{
	return mir.target_height;
}

void eglapp_shutdown(void)
{
	if (EGL_NO_SURFACE != egl.surface) {
		eglDestroySurface(egl.display, egl.surface);
		egl.surface = EGL_NO_SURFACE;
	}

	if (EGL_NO_CONTEXT != egl.context) {
		eglDestroyContext(egl.display, egl.context);
		egl.context = EGL_NO_CONTEXT;
	}

	if (EGL_NO_DISPLAY != egl.display) {
		eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglTerminate(egl.display);
		egl.display = EGL_NO_DISPLAY;
	}

	if (NULL != mir.surface) {
		mir_surface_release_sync(mir.surface);
		mir.surface = NULL;
	}
	if (NULL != mir.bufferStream) {
		mir_buffer_stream_release_sync(mir.bufferStream);
		mir.bufferStream = NULL;
	}
	if (NULL != mir.connection) {
		mir_connection_release(mir.connection);
		mir.connection = NULL;
	}
}

static void shutdown(int signum)
{
	if (running) {
		running = 0;
		printf("Signal %d received. Good night.\n", signum);
	}
}

bool eglapp_running(void)
{
	return running;
}

bool eglapp_zooming(void)
{
	return zooming;
}

void eglapp_swap_buffers(void)
{
	if (!running)
		return;

	eglSwapBuffers(egl.display, egl.surface);
}

static void eglapp_handle_input(MirSurface* /*surface*/, MirEvent const* ev, void* /*context*/)
{
	if (mir_event_get_type(ev) == mir_event_type_resize) {
		MirResizeEvent const* rev = mir_event_get_resize_event(ev);
		const int width = mir_resize_event_get_width(rev);
		const int height = mir_resize_event_get_height(rev);

		fprintf(stderr, "resize: %dx%d\n", width, height);

		if ((width == mir.target_width && height == mir.target_height) ||
			(width == mir.target_height && height == mir.target_width)) {

			mir.target_width = width;
			mir.target_height = height;
			zooming = 0;

			fprintf(stderr, "resize done\n");
		}
		return;
	}

	if (mir_event_get_type(ev) == mir_event_type_input) {
		MirInputEvent const* iev = mir_event_get_input_event(ev);

		if (mir_input_event_get_type(iev) == mir_input_event_type_key) {
			MirKeyboardEvent const* kev = mir_input_event_get_keyboard_event(iev);

			if (mir_keyboard_event_action(kev) == mir_keyboard_action_up) {
				MirInputEventModifiers mods = mir_keyboard_event_modifiers(kev);

				if (!(mods & mir_input_event_modifier_alt) && !(mods & mir_input_event_modifier_ctrl)) {

					if (mir_keyboard_event_key_code(kev) == XKB_KEY_Escape)
						running = 0;
				}
			}
		}
		return;
	}
}

static const char* string_from_pixel_format(
	const MirPixelFormat pf)
{
	switch (pf) {
	case mir_pixel_format_invalid:
		return "mir_pixel_format_invalid";
	case mir_pixel_format_abgr_8888:
		return "mir_pixel_format_abgr_8888";
	case mir_pixel_format_xbgr_8888:
		return "mir_pixel_format_xbgr_8888";
	case mir_pixel_format_argb_8888:
		return "mir_pixel_format_argb_8888";
	case mir_pixel_format_xrgb_8888:
		return "mir_pixel_format_xrgb_8888";
	case mir_pixel_format_bgr_888:
		return "mir_pixel_format_bgr_888";
	case mir_pixel_format_rgb_888:
		return "mir_pixel_format_rgb_888";
	case mir_pixel_format_rgb_565:
		return "mir_pixel_format_rgb_565";
	case mir_pixel_format_rgba_5551:
		return "mir_pixel_format_rgba_5551";
	case mir_pixel_format_rgba_4444:
		return "mir_pixel_format_rgba_4444";
	}
	return "unknown-pixel-format";
}

static unsigned int bpp_from_pixel_format(
	const MirPixelFormat pf)
{
	switch (pf) {
	case mir_pixel_format_invalid:
		return 0;
	case mir_pixel_format_abgr_8888:
	case mir_pixel_format_xbgr_8888:
	case mir_pixel_format_argb_8888:
	case mir_pixel_format_xrgb_8888:
		return 32;
	case mir_pixel_format_bgr_888:
	case mir_pixel_format_rgb_888:
		return 24;
	case mir_pixel_format_rgb_565:
	case mir_pixel_format_rgba_5551:
	case mir_pixel_format_rgba_4444:
		return 16;
	}
	return 0;
}

static const char* string_from_EGL_attrib(
	const EGLint attr)
{
	switch (attr) {
	case EGL_BUFFER_SIZE:
		return "EGL_BUFFER_SIZE";
	case EGL_ALPHA_SIZE:
		return "EGL_ALPHA_SIZE";
	case EGL_BLUE_SIZE:
		return "EGL_BLUE_SIZE";
	case EGL_GREEN_SIZE:
		return "EGL_GREEN_SIZE";
	case EGL_RED_SIZE:
		return "EGL_RED_SIZE";
	case EGL_DEPTH_SIZE:
		return "EGL_DEPTH_SIZE";
	case EGL_STENCIL_SIZE:
		return "EGL_STENCIL_SIZE";
	case EGL_CONFIG_CAVEAT:
		return "EGL_CONFIG_CAVEAT";
	case EGL_CONFIG_ID:
		return "EGL_CONFIG_ID";
	case EGL_LEVEL:
		return "EGL_LEVEL";
	case EGL_MAX_PBUFFER_HEIGHT:
		return "EGL_MAX_PBUFFER_HEIGHT";
	case EGL_MAX_PBUFFER_PIXELS:
		return "EGL_MAX_PBUFFER_PIXELS";
	case EGL_MAX_PBUFFER_WIDTH:
		return "EGL_MAX_PBUFFER_WIDTH";
	case EGL_NATIVE_RENDERABLE:
		return "EGL_NATIVE_RENDERABLE";
	case EGL_NATIVE_VISUAL_ID:
		return "EGL_NATIVE_VISUAL_ID";
	case EGL_NATIVE_VISUAL_TYPE:
		return "EGL_NATIVE_VISUAL_TYPE";
	case EGL_SAMPLES:
		return "EGL_SAMPLES";
	case EGL_SAMPLE_BUFFERS:
		return "EGL_SAMPLE_BUFFERS";
	case EGL_SURFACE_TYPE:
		return "EGL_SURFACE_TYPE";
	case EGL_TRANSPARENT_TYPE:
		return "EGL_TRANSPARENT_TYPE";
	case EGL_TRANSPARENT_BLUE_VALUE:
		return "EGL_TRANSPARENT_BLUE_VALUE";
	case EGL_TRANSPARENT_GREEN_VALUE:
		return "EGL_TRANSPARENT_GREEN_VALUE";
	case EGL_TRANSPARENT_RED_VALUE:
		return "EGL_TRANSPARENT_RED_VALUE";
	case EGL_BIND_TO_TEXTURE_RGB:
		return "EGL_BIND_TO_TEXTURE_RGB";
	case EGL_BIND_TO_TEXTURE_RGBA:
		return "EGL_BIND_TO_TEXTURE_RGBA";
	case EGL_MIN_SWAP_INTERVAL:
		return "EGL_MIN_SWAP_INTERVAL";
	case EGL_MAX_SWAP_INTERVAL:
		return "EGL_MAX_SWAP_INTERVAL";
	case EGL_LUMINANCE_SIZE:
		return "EGL_LUMINANCE_SIZE";
	case EGL_ALPHA_MASK_SIZE:
		return "EGL_ALPHA_MASK_SIZE";
	case EGL_COLOR_BUFFER_TYPE:
		return "EGL_COLOR_BUFFER_TYPE";
	case EGL_RENDERABLE_TYPE:
		return "EGL_RENDERABLE_TYPE";
	case EGL_CONFORMANT:
		return "EGL_CONFORMANT";
	}

	return "unknown-egl-attrib";
}

static const MirDisplayOutput* findActiveOutput(
	const MirDisplayConfiguration* conf)
{
	for (unsigned int i = 0; i < conf->num_outputs; i++) {
		const MirDisplayOutput* output = conf->outputs + i;
		if (output->used && output->connected && output->current_mode < output->num_modes) {
			return output;
		}
	}
	return NULL;
}

static bool parse_cli(
	const int argc, char **const argv,
	MirSurfaceParameters &surfParam,
	EGLint &swapinterval)
{
	if (argc == 1)
		return true;

	bool help = false;

	for (int i = 1; i < argc && !help; ++i) {
		const char *arg = argv[i];
	
		if (arg[0] != '-' || arg[1] == '\0') {
			help = true;
			continue;
		}

		if (0 == strcmp(arg + 1, "app"))
			break;

		if (0 == strcmp(arg + 1, "n")) {
			swapinterval = 0;
			continue;
		}

		if (0 == strcmp(arg + 1, "f")) {
			surfParam.width = 0;
			surfParam.height = 0;
			continue;
		}

		if (0 == strcmp(arg + 1, "s")) {
			unsigned int w, h;
			if (++i < argc && sscanf(argv[i], "%ux%u", &w, &h) == 2) {
				surfParam.width = w;
				surfParam.height = h;
				continue;
			}
			else {
				fprintf(stderr, "Invalid surface size\n");
			}
		}
		help = true;
	}

	if (help) {
		printf(
			"Usage: %s [<options>]\n"
			"  -h               Show this help text\n"
			"  -f               Force full screen\n"
			"  -n               Don't sync to vblank\n"
			"  -s WIDTHxHEIGHT  Force surface size\n",
			argv[0]);
	}

	return !help;
}

#define CHECK(_cond, _err) \
	if (!(_cond)) { \
		printf("%s\n", (_err)); \
		return false; \
	}

static bool reportEGLCaps(
	const EGLDisplay display,
	FILE* f)
{
	assert(EGL_NO_DISPLAY != display);

	const char* str_version	= eglQueryString(display, EGL_VERSION);
	const char* str_vendor	= eglQueryString(display, EGL_VENDOR);
	const char* str_exten	= eglQueryString(display, EGL_EXTENSIONS);

	fprintf(f, "egl version, vendor, extensions:"
		"\n\t%s\n\t%s\n\t%s\n",
		str_version, str_vendor, str_exten);

	EGLConfig config[128];
	EGLint numConfig;

	EGLBoolean ok = eglGetConfigs(display, config, EGLint(sizeof(config) / sizeof(config[0])), &numConfig);
	CHECK(ok, "eglGetConfigs failed");

	for (EGLint i = 0; i < numConfig; ++i) {
		fprintf(f, "\nconfig %d\n", i);
		EGLint value;

		if (EGL_TRUE != eglGetConfigAttrib(display, config[i], EGL_CONFIG_CAVEAT, &value)) {
			fprintf(stderr, "eglGetConfigAttrib() failed\n");
			continue;
		}

		if (EGL_NON_CONFORMANT_CONFIG == value) {
			fprintf(f, "non-conformant caveat -- skip\n");
			continue;
		}

		static const EGLint attr[] = {
			EGL_BUFFER_SIZE,
			EGL_ALPHA_SIZE,
			EGL_BLUE_SIZE,
			EGL_GREEN_SIZE,
			EGL_RED_SIZE,
			EGL_DEPTH_SIZE,
			EGL_STENCIL_SIZE,
			EGL_CONFIG_CAVEAT,
			EGL_CONFIG_ID,
			EGL_LEVEL,
			EGL_MAX_PBUFFER_HEIGHT,
			EGL_MAX_PBUFFER_PIXELS,
			EGL_MAX_PBUFFER_WIDTH,
			EGL_NATIVE_RENDERABLE,
			EGL_NATIVE_VISUAL_ID,
			EGL_NATIVE_VISUAL_TYPE,
			EGL_SAMPLES,
			EGL_SAMPLE_BUFFERS,
			EGL_SURFACE_TYPE,
			EGL_TRANSPARENT_TYPE,
			EGL_TRANSPARENT_BLUE_VALUE,
			EGL_TRANSPARENT_GREEN_VALUE,
			EGL_TRANSPARENT_RED_VALUE,
			EGL_BIND_TO_TEXTURE_RGB,
			EGL_BIND_TO_TEXTURE_RGBA,
			EGL_MIN_SWAP_INTERVAL,
			EGL_MAX_SWAP_INTERVAL,
			EGL_LUMINANCE_SIZE,
			EGL_ALPHA_MASK_SIZE,
			EGL_COLOR_BUFFER_TYPE,
			EGL_RENDERABLE_TYPE,
			EGL_CONFORMANT
		};

		for (unsigned j = 0; j < sizeof(attr) / sizeof(attr[0]); ++j) {
			EGLint value;

			if (EGL_TRUE != eglGetConfigAttrib(display, config[i], attr[j], &value)) {
				fprintf(stderr, "eglGetConfigAttrib() failed (2)\n");
				continue;
			}

			fprintf(f, "\t%s: 0x%08x\n", string_from_EGL_attrib(attr[j]), value);
		}
	}

	if (numConfig)
		fputc('\n', f);

	return true;
}

bool eglapp_init(int argc, char **argv)
{
	MirSurfaceParameters surfParam = {
		"eglappsurface",
		512,
		512,
		mir_pixel_format_xbgr_8888,
		mir_buffer_usage_hardware,
		mir_display_output_id_invalid
	};
	EGLint swapinterval = 1;

	if (!parse_cli(argc, argv, surfParam, swapinterval)) {
		return false;
	}

	mir.connection = mir_connect_sync(NULL, appname);
	CHECK(mir_connection_is_valid(mir.connection), "Can't get connection");

	{
		MirDisplayConfiguration* displayConfig = mir_connection_create_display_config(mir.connection);

		const MirDisplayOutput *output = findActiveOutput(displayConfig);
		CHECK(output, "No active outputs found.");

		const MirDisplayMode *mode = &output->modes[output->current_mode];

		printf("Connected to display: resolution %dx%d, position %dx%d, format %s\n",
			mode->horizontal_resolution,
			mode->vertical_resolution,
			output->position_x,
			output->position_y,
			string_from_pixel_format(output->current_format));

		if (surfParam.width == 0 || surfParam.height == 0) {
			surfParam.width = mode->horizontal_resolution;
			surfParam.height = mode->vertical_resolution;
			surfParam.output_id = output->output_id;
		}

		surfParam.pixel_format = output->current_format;

		mir_display_config_destroy(displayConfig);
	}

	eglBindAPI(EGL_OPENGL_ES_API);
	const EGLNativeDisplayType nativeDisplay = mir_connection_get_egl_native_display(mir.connection);
	const EGLDisplay display = eglGetDisplay(nativeDisplay);
	CHECK(display != EGL_NO_DISPLAY, "eglGetDisplay failed");

	EGLBoolean ok;

	EGLint major;
	EGLint minor;

	ok = eglInitialize(display, &major, &minor);
	CHECK(ok, "eglInitialize failed");

	reportEGLCaps(display, stdout);

	const EGLint configAttribs[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 0,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 8,
		EGL_SAMPLES, 0,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLint configCount;
	ok = eglChooseConfig(display, configAttribs, NULL, 0, &configCount);
	CHECK(ok, "Could not eglChooseConfig");

	EGLConfig* config = (EGLConfig*) alloca(configCount * sizeof(EGLConfig));
	ok = eglChooseConfig(display, configAttribs, config, configCount, &configCount);
	CHECK(ok, "Could not eglChooseConfig (2)");

	const EGLint contextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLContext context = eglCreateContext(display, config[0], EGL_NO_CONTEXT, contextAttribs);
	CHECK(context != EGL_NO_CONTEXT, "eglCreateContext failed");

	surfParam.pixel_format = mir_connection_get_egl_pixel_format(
		mir.connection, display, config[0]);

	fprintf(stdout, "mir surface format: %s\n", string_from_pixel_format(surfParam.pixel_format));

	MirSurfaceSpec* spec = mir_connection_create_spec_for_normal_surface(
		mir.connection,
		surfParam.width,
		surfParam.height,
		surfParam.pixel_format);

#if 0
	mir.bufferStream = mir_connection_create_buffer_stream_sync(
		mir.connection,
		surfParam.width,
		surfParam.height,
		surfParam.pixel_format,
		surfParam.buffer_usage);
	CHECK(mir_buffer_stream_is_valid(mir.bufferStream), "Invalid buffer stream");

	MirBufferStreamInfo streams;
	streams.stream = mir.bufferStream;
	streams.displacement_x = 0;
	streams.displacement_y = 0;

	mir_surface_spec_set_streams(spec, &streams, 1);
	mir_surface_spec_set_name(spec, surfParam.name);

	if (surfParam.output_id != mir_display_output_id_invalid)
		mir_surface_spec_set_fullscreen_on_output(spec, surfParam.output_id);

	mir.surface = mir_surface_create_sync(spec);

#else
	mir_surface_spec_set_buffer_usage(spec, mir_buffer_usage_hardware);
	mir_surface_spec_set_name(spec, surfParam.name);

	if (surfParam.output_id != mir_display_output_id_invalid)
		mir_surface_spec_set_fullscreen_on_output(spec, surfParam.output_id);

	mir.surface = mir_surface_create_sync(spec);
	mir.bufferStream = mir_surface_get_buffer_stream(mir.surface);

#endif
	mir_surface_spec_release(spec);

	if (!mir_surface_is_valid(mir.surface)) {
		printf("Can't create mir surface: %s\n", mir_surface_get_error_message(mir.surface));
		return false;
	}

	mir_surface_set_event_handler(mir.surface, eglapp_handle_input, NULL);

	EGLSurface surface = eglCreateWindowSurface(display, config[0],
		(EGLNativeWindowType) mir_buffer_stream_get_egl_native_window(mir.bufferStream), NULL);

	CHECK(surface != EGL_NO_SURFACE, "eglCreateWindowSurface failed");

	ok = eglSurfaceAttrib(display, surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_DESTROYED);
	CHECK(ok, "eglSurfaceAttrib failed");

	ok = eglMakeCurrent(display, surface, surface, context);
	CHECK(ok, "eglMakeCurrent failed");

	eglSwapInterval(display, swapinterval);

	signal(SIGINT, shutdown);
	signal(SIGTERM, shutdown);

	egl.display = display;
	egl.surface = surface;
	egl.context = context;

	mir.target_width = surfParam.width;
	mir.target_height = surfParam.height;

	running = 1;
	zooming = 1;

	return true;
}
