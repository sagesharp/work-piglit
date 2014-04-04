/*
 * Copyright 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file Tests for EGL_CHROMIUM_get_sync_values.
 *
 * This file tests the unregistered ChromeOS-specific EGL extension
 * EGL_CHROMIUM_get_sync_values.  The extension is similar to the GLX
 * OML_sync_control extension, but only implements the glXGetSyncValuesOML
 * function.  This tests that function.
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "piglit-util-egl.h"
#include "piglit-util-gl-common.h"
#include "../../egl-util.h"

/* Piglit extension function pointer. */
EGLBoolean (*peglGetSyncValuesCHROMIUM)(EGLDisplay display, EGLSurface surface, EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc);

static const struct piglit_subtest subtests[];

static const EGLuint64KHR canary = 0xdecafc0ffee;
const char *extensions[] = { "EGL_CHROMIUM_get_sync_values", NULL };
static const EGLint config_attribs[] = {
	EGL_RED_SIZE,		EGL_DONT_CARE,
	EGL_GREEN_SIZE,		EGL_DONT_CARE,
	EGL_BLUE_SIZE,		EGL_DONT_CARE,
	EGL_ALPHA_SIZE,		EGL_DONT_CARE,
	EGL_DEPTH_SIZE,		EGL_DONT_CARE,
	EGL_STENCIL_SIZE,	EGL_DONT_CARE,
	EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
	EGL_NONE,
};


static char *argv = "egl_chromium_get_sync_values";

static enum piglit_result
test_eglGetSyncValuesCHROMIUM_null_pointers(struct egl_state *state)
{
	EGLuint64KHR counter;
	EGLBoolean ok;

	/* Note: it would be fun to get trinity fuzzing Mesa calls. */
	ok = peglGetSyncValuesCHROMIUM(NULL, state->surf, &counter, &counter, &counter);
	if (ok) {
		piglit_loge("eglGetSyncValuesCHROMIUM succeeded with NULL display");
		return PIGLIT_FAIL;
	}
	ok = peglGetSyncValuesCHROMIUM(state->egl_dpy, NULL, &counter, &counter, &counter);
	if (ok) {
		piglit_loge("eglGetSyncValuesCHROMIUM succeeded with NULL surface");
		return PIGLIT_FAIL;
	}
	ok = peglGetSyncValuesCHROMIUM(state->egl_dpy, state->surf, NULL, &counter, &counter);
	if (ok) {
		piglit_loge("eglGetSyncValuesCHROMIUM succeeded with NULL ust pointer");
		return PIGLIT_FAIL;
	}
	ok = peglGetSyncValuesCHROMIUM(state->egl_dpy, state->surf, &counter, NULL, &counter);
	if (ok) {
		piglit_loge("eglGetSyncValuesCHROMIUM succeeded with NULL msc pointer");
		return PIGLIT_FAIL;
	}
	ok = peglGetSyncValuesCHROMIUM(state->egl_dpy, state->surf, &counter, &counter, NULL);
	if (ok) {
		piglit_loge("eglGetSyncValuesCHROMIUM succeeded with NULL sbc pointer");
		return PIGLIT_FAIL;
	}
	piglit_logi("eglGetSyncValuesCHROMIUM handles NULL pointers!");
	return PIGLIT_PASS;
}

static enum piglit_result
get_ust_values(struct egl_state *state, EGLuint64KHR *ust,
		EGLuint64KHR *ust2, EGLuint64KHR *ust3)
{
	EGLBoolean ok;
	EGLuint64KHR msc = canary;
	EGLuint64KHR sbc = canary;

	ok = peglGetSyncValuesCHROMIUM(state->egl_dpy, state->surf, ust, &msc, &sbc);
	if (!ok) {
		piglit_loge("Unexpected failure on first UST fetch");
		return PIGLIT_FAIL;
	}
	sleep(1);
	ok = peglGetSyncValuesCHROMIUM(state->egl_dpy, state->surf, ust2, &msc, &sbc);
	if (!ok) {
		piglit_loge("Unexpected failure on second UST fetch");
		return PIGLIT_FAIL;
	}
	sleep(1);
	ok = peglGetSyncValuesCHROMIUM(state->egl_dpy, state->surf, ust3, &msc, &sbc);
	if (!ok) {
		piglit_loge("Unexpected failure on third UST fetch");
		return PIGLIT_FAIL;
	}
	return PIGLIT_PASS;
}

/**
 * Verify the UST counter functionality.  From the OML_sync_control spec:
 *
 * The Unadjusted System Time (or UST) is a 64-bit monotonically increasing
 * counter that is available throughout the system. UST is not a resource that
 * is controlled by OpenGL, so it is not defined further as part of this
 * extension.
 */
static enum piglit_result
test_eglGetSyncValuesCHROMIUM_ust_test(struct egl_state *state)
{
	EGLuint64KHR ust = canary;
	EGLuint64KHR ust2 = canary;
	EGLuint64KHR ust3 = canary;

	if (get_ust_values(state, &ust, &ust2, &ust3) != PIGLIT_PASS)
		return PIGLIT_FAIL;
	piglit_logi("UST values 1 second apart: %lu %lu %lu", ust, ust2, ust3);

	if (ust > ust2 || ust > ust3 || ust2 > ust3) {
		piglit_logi("Hit UST counter wrap?  Rerunning test.");
		if (get_ust_values(state, &ust, &ust2, &ust3) != PIGLIT_PASS)
			return PIGLIT_FAIL;
		if (ust > ust2 || ust > ust3 || ust2 > ust3) {
			piglit_loge("eglGetSyncValuesCHROMIUM UST is not monotonically increasing");
			piglit_loge("UST values 1 second apart: %lu %lu %lu",
					ust, ust2, ust3);
			return PIGLIT_FAIL;
		}
	}

	return PIGLIT_PASS;
}

static enum piglit_result
run_test(enum piglit_result (*test_func)(struct egl_state *state))
{
	struct egl_test test;
	int ret;

	egl_init_test(&test);
	test.extensions = extensions;
	test.config_attribs = config_attribs;
	test.draw = test_func;
	ret = egl_util_run(&test, 1, &argv, false);
	if (ret != EXIT_SUCCESS)
		return PIGLIT_FAIL;
	return test.result;
}

static enum piglit_result
init_eglGetSyncValuesCHROMIUM_null_pointers(void *test_data)
{
	return run_test(test_eglGetSyncValuesCHROMIUM_null_pointers);
}

static enum piglit_result
init_eglGetSyncValuesCHROMIUM_ust_test(void *test_data)
{
	return run_test(test_eglGetSyncValuesCHROMIUM_ust_test);
}

static const struct piglit_subtest subtests[] = {
	{
		"eglGetSyncValuesCHROMIUM_null_pointers",
		"eglGetSyncValuesCHROMIUM_null_pointers",
		init_eglGetSyncValuesCHROMIUM_null_pointers,
	},
	{
		"eglGetSyncValuesCHROMIUM_ust_test",
		"eglGetSyncValuesCHROMIUM_ust_test",
		init_eglGetSyncValuesCHROMIUM_ust_test,
	},
	{0},
};

int
main(int argc, char **argv)
{
	const char **selected_subtests = NULL;
	enum piglit_result result = PIGLIT_SKIP;
	size_t num_selected_subtests = 0;

	piglit_automatic = GL_TRUE;
	piglit_parse_subtest_args(&argc, argv, subtests, &selected_subtests,
			          &num_selected_subtests);

	peglGetSyncValuesCHROMIUM = (void*) eglGetProcAddress("eglGetSyncValuesCHROMIUM");
	result = piglit_run_selected_subtests(subtests, selected_subtests,
					      num_selected_subtests, result);
	piglit_report_result(result);
	assert(!"unreachable");
	return EXIT_FAILURE;
}
