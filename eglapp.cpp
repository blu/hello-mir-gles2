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
	eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate(egl.display);

	if (mir.surface != NULL) {
		mir_surface_release_sync(mir.surface);
		mir.surface = NULL;
	}
	if (mir.bufferStream != NULL) {
		mir_buffer_stream_release_sync(mir.bufferStream);
		mir.bufferStream = NULL;
	}
	if (mir.connection != NULL) {
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
#if 0
    static time_t lasttime = 0;
    static int lastcount = 0;
    static int count = 0;
    time_t now = time(NULL);
    time_t dtime;
    int dcount;

#endif
    if (!running)
        return;

    eglSwapBuffers(egl.display, egl.surface);

#if 0
    count++;
    dcount = count - lastcount;
    dtime = now - lasttime;
    if (dtime)
    {
        printf("%d FPS\n", dcount);
        lasttime = now;
        lastcount = count;
    }

#endif
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

static const char* str_from_pixel_format(
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
	return "alien-pixel-format";
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
			str_from_pixel_format(output->current_format));

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
	CHECK(display != EGL_NO_DISPLAY, "Can't eglGetDisplay");

	EGLBoolean ok;

	ok = eglInitialize(display, NULL, NULL);
	CHECK(ok, "Can't eglInitialize");

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

	fprintf(stdout, "mir surface format: %s\n", str_from_pixel_format(surfParam.pixel_format));

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

	ok = eglMakeCurrent(display, surface, surface, context);
	CHECK(ok, "Can't eglMakeCurrent");

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
