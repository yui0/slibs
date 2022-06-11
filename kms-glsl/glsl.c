/*
 * Copyright (c) 2012 Arvin Schnell <arvin.schnell@gmail.com>
 * Copyright (c) 2012 Rob Clark <rob@ti.com>
 * Copyright (c) 2020 Antonin Stefanutti <antonin.stefanutti@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "kms-glsl.h"

static const struct egl *egl;
static const struct gbm *gbm;
static const struct drm *drm;

static const char *shortopts = "Ac:D:f:hm:p:v:x";

static const struct option longopts[] = {
		{"atomic",      no_argument,       0, 'A'},
		{"count",       required_argument, 0, 'c'},
		{"device",      required_argument, 0, 'D'},
		{"format",      required_argument, 0, 'f'},
		{"help",        no_argument,       0, 'h'},
		{"modifier",    required_argument, 0, 'm'},
		{"perfcntr",    required_argument, 0, 'p'},
		{"vmode",       required_argument, 0, 'v'},
		{"surfaceless", no_argument,       0, 'x'},
		{0,             0,                 0, 0}
};

static void usage(const char *name) {
	printf("Usage: %s [-AcDfmpvx] <shader_file>\n"
		   "\n"
		   "options:\n"
		   "    -A, --atomic             use atomic modesetting and fencing\n"
		   "    -c, --count              run for the specified number of frames\n"
		   "    -D, --device=DEVICE      use the given device\n"
		   "    -f, --format=FOURCC      framebuffer format\n"
		   "    -h, --help               print usage\n"
		   "    -m, --modifier=MODIFIER  hardcode the selected modifier\n"
		   "    -p, --perfcntr=LIST      sample specified performance counters using\n"
		   "                             the AMD_performance_monitor extension (comma\n"
		   "                             separated list)\n"
		   "    -v, --vmode=VMODE        specify the video mode in the format\n"
		   "                             <mode>[-<vrefresh>]\n"
		   "    -x, --surfaceless        use surfaceless mode, instead of GBM surface\n",
		   name);
}

int main(int argc, char *argv[]) {
	const char *device = NULL;
	const char *shadertoy = NULL;
	const char *perfcntr = NULL;
	char mode_str[DRM_DISPLAY_MODE_LEN] = "";
	char *p;
	uint32_t format = DRM_FORMAT_XRGB8888;
	uint64_t modifier = DRM_FORMAT_MOD_LINEAR;
	int atomic = 0;
	int opt;
	unsigned int len;
	unsigned int vrefresh = 0;
	unsigned int count = ~0;
	bool surfaceless = false;
	int ret;

	while ((opt = getopt_long_only(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (opt) {
			case 'A':
				atomic = 1;
				break;
			case 'c':
				count = strtoul(optarg, NULL, 0);
				break;
			case 'D':
				device = optarg;
				break;
			case 'f': {
				char fourcc[4] = "    ";
				int length = strlen(optarg);
				if (length > 0)
					fourcc[0] = optarg[0];
				if (length > 1)
					fourcc[1] = optarg[1];
				if (length > 2)
					fourcc[2] = optarg[2];
				if (length > 3)
					fourcc[3] = optarg[3];
				format = fourcc_code(fourcc[0], fourcc[1],
									 fourcc[2], fourcc[3]);
				break;
			}
			case 'h':
				usage(argv[0]);
				return 0;
			case 'm':
				modifier = strtoull(optarg, NULL, 0);
				break;
			case 'p':
				perfcntr = optarg;
				break;
			case 'v':
				p = strchr(optarg, '-');
				if (p == NULL) {
					len = strlen(optarg);
				} else {
					vrefresh = strtoul(p + 1, NULL, 0);
					len = p - optarg;
				}
				if (len > sizeof(mode_str) - 1)
					len = sizeof(mode_str) - 1;
				strncpy(mode_str, optarg, len);
				mode_str[len] = '\0';
				break;
			case 'x':
				surfaceless = true;
				break;
			default:
				usage(argv[0]);
				return -1;
		}
	}

	if (argc - optind != 1) {
		usage(argv[0]);
		return -1;
	}
	shadertoy = argv[optind];

	if (atomic) {
		drm = init_drm_atomic(device, mode_str, vrefresh, count);
	} else {
		drm = init_drm_legacy(device, mode_str, vrefresh, count);
	}
	if (!drm) {
		printf("failed to initialize %s DRM\n", atomic ? "atomic" : "legacy");
		return -1;
	}

	gbm = init_gbm(drm->fd, drm->mode->hdisplay, drm->mode->vdisplay,
				format, modifier, surfaceless);
	if (!gbm) {
		printf("failed to initialize GBM\n");
		return -1;
	}

	egl = init_egl(gbm);
	if (!egl) {
		printf("failed to initialize EGL\n");
		return -1;
	}

	ret = init_shadertoy(gbm, egl, shadertoy);
	if (ret < 0) {
		return -1;
	}

	if (perfcntr) {
		init_perfcntrs(egl, perfcntr);
	}

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	drm->run(gbm, egl);

	finish_egl();
//	finish_gbm();
	finish_drm();
//	return drm->run(gbm, egl);
}
