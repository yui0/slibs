// https://qiita.com/Pctg-x8/items/52c7e018556ec5c867de
// drm-common.h
/*
 * Copyright (c) 2017 Rob Clark <rclark@redhat.com>
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

#ifndef _DRM_COMMON_H
#define _DRM_COMMON_H

#include <xf86drm.h>
#include <xf86drmMode.h>

struct gbm;
struct egl;

struct plane {
	drmModePlane *plane;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct crtc {
	drmModeCrtc *crtc;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct connector {
	drmModeConnector *connector;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct drm {
	int fd;

	/* only used for atomic: */
	struct plane *plane;
	struct crtc *crtc;
	drmModeCrtc *orig_crtc;
	struct connector *connector;
	int crtc_index;
	int kms_in_fence_fd;
	int kms_out_fence_fd;

	drmModeModeInfo *mode;
	uint32_t crtc_id;
	uint32_t connector_id;

	/* number of frames to run for: */
	unsigned int count;

	int (*run)(const struct gbm *gbm, const struct egl *egl);
	int (*draw)(const struct gbm *gbm, const struct egl *egl);
};

struct drm_fb {
	struct gbm_bo *bo;
	uint32_t fb_id;
};

struct drm_fb * drm_fb_get_from_bo(struct gbm_bo *bo);

int init_drm(struct drm *drm, const char *device, const char *mode_str, unsigned int vrefresh, unsigned int count);
const struct drm * init_drm_legacy(const char *device, const char *mode_str, unsigned int vrefresh, unsigned int count);
const struct drm * init_drm_atomic(const char *device, const char *mode_str, unsigned int vrefresh, unsigned int count);

#endif /* _DRM_COMMON_H */


// common.h
/*
 * Copyright (c) 2017 Rob Clark <rclark@redhat.com>
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

#ifndef _COMMON_H
#define _COMMON_H

#ifndef GL_ES_VERSION_2_0
#include <GLES2/gl2.h>
#endif
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <gbm.h>
#include <drm_fourcc.h>
#include <stdbool.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* from mesa's util/macros.h: */
#define MIN2( A, B )   ( (A)<(B) ? (A) : (B) )
#define MAX2( A, B )   ( (A)>(B) ? (A) : (B) )
#define MIN3( A, B, C ) ((A) < (B) ? MIN2(A, C) : MIN2(B, C))
#define MAX3( A, B, C ) ((A) > (B) ? MAX2(A, C) : MAX2(B, C))

static inline unsigned
u_minify(unsigned value, unsigned levels)
{
	return MAX2(1, value >> levels);
}

#ifndef DRM_FORMAT_MOD_LINEAR
#define DRM_FORMAT_MOD_LINEAR 0
#endif

#ifndef DRM_FORMAT_MOD_INVALID
#define DRM_FORMAT_MOD_INVALID ((((__u64)0) << 56) | ((1ULL << 56) - 1))
#endif

#ifndef EGL_KHR_platform_gbm
#define EGL_KHR_platform_gbm 1
#define EGL_PLATFORM_GBM_KHR              0x31D7
#endif /* EGL_KHR_platform_gbm */

#ifndef EGL_EXT_platform_base
#define EGL_EXT_platform_base 1
typedef EGLDisplay (EGLAPIENTRYP PFNEGLGETPLATFORMDISPLAYEXTPROC) (EGLenum platform, void *native_display, const EGLint *attrib_list);
typedef EGLSurface (EGLAPIENTRYP PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC) (EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list);
typedef EGLSurface (EGLAPIENTRYP PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC) (EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list);
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLDisplay EGLAPIENTRY eglGetPlatformDisplayEXT (EGLenum platform, void *native_display, const EGLint *attrib_list);
EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurfaceEXT (EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list);
EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurfaceEXT (EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list);
#endif
#endif /* EGL_EXT_platform_base */

#ifndef EGL_VERSION_1_5
#define EGLImage EGLImageKHR
#endif /* EGL_VERSION_1_5 */

#define WEAK __attribute__((weak))

/* Define tokens from EGL_EXT_image_dma_buf_import_modifiers */
#ifndef EGL_EXT_image_dma_buf_import_modifiers
#define EGL_EXT_image_dma_buf_import_modifiers 1
#define EGL_DMA_BUF_PLANE3_FD_EXT         0x3440
#define EGL_DMA_BUF_PLANE3_OFFSET_EXT     0x3441
#define EGL_DMA_BUF_PLANE3_PITCH_EXT      0x3442
#define EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT 0x3443
#define EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT 0x3444
#define EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT 0x3445
#define EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT 0x3446
#define EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT 0x3447
#define EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT 0x3448
#define EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT 0x3449
#define EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT 0x344A
#endif

#define NUM_BUFFERS 2

struct gbm {
	struct gbm_device *dev;
	struct gbm_surface *surface;
	struct gbm_bo *bos[NUM_BUFFERS];    /* for the surfaceless case */
	uint32_t format;
	int width, height;
};

const struct gbm * init_gbm(int drm_fd, int w, int h, uint32_t format, uint64_t modifier, bool surfaceless);

struct framebuffer {
	EGLImageKHR image;
	GLuint tex;
	GLuint fb;
};

struct egl {
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
	EGLSurface surface;
	struct framebuffer fbs[NUM_BUFFERS];    /* for the surfaceless case */

	PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
	PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
	PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
	PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR;
	PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR;
	PFNEGLWAITSYNCKHRPROC eglWaitSyncKHR;
	PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR;
	PFNEGLDUPNATIVEFENCEFDANDROIDPROC eglDupNativeFenceFDANDROID;

	/* AMD_performance_monitor */
	PFNGLGETPERFMONITORGROUPSAMDPROC         glGetPerfMonitorGroupsAMD;
	PFNGLGETPERFMONITORCOUNTERSAMDPROC       glGetPerfMonitorCountersAMD;
	PFNGLGETPERFMONITORGROUPSTRINGAMDPROC    glGetPerfMonitorGroupStringAMD;
	PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC  glGetPerfMonitorCounterStringAMD;
	PFNGLGETPERFMONITORCOUNTERINFOAMDPROC    glGetPerfMonitorCounterInfoAMD;
	PFNGLGENPERFMONITORSAMDPROC              glGenPerfMonitorsAMD;
	PFNGLDELETEPERFMONITORSAMDPROC           glDeletePerfMonitorsAMD;
	PFNGLSELECTPERFMONITORCOUNTERSAMDPROC    glSelectPerfMonitorCountersAMD;
	PFNGLBEGINPERFMONITORAMDPROC             glBeginPerfMonitorAMD;
	PFNGLENDPERFMONITORAMDPROC               glEndPerfMonitorAMD;
	PFNGLGETPERFMONITORCOUNTERDATAAMDPROC    glGetPerfMonitorCounterDataAMD;

	bool modifiers_supported;

	void (*draw)(uint64_t start_time, unsigned frame);
};

static inline int __egl_check(void *ptr, const char *name)
{
	if (!ptr) {
		printf("no %s\n", name);
		return -1;
	}
	return 0;
}

#define egl_check(egl, name) __egl_check((egl)->name, #name)

const struct egl * init_egl(const struct gbm *gbm);

int create_program(const char *vs_src, const char *fs_src);
int link_program(unsigned program);

int init_shadertoy(const struct gbm *gbm, struct egl *egl, const char *shadertoy);

void init_perfcntrs(const struct egl *egl, const char *perfcntrs);
void start_perfcntrs(void);
void end_perfcntrs(void);
void finish_perfcntrs(void);
void dump_perfcntrs(unsigned nframes, uint64_t elapsed_time_ns);

#define NSEC_PER_SEC (INT64_C(1000) * USEC_PER_SEC)
#define USEC_PER_SEC (INT64_C(1000) * MSEC_PER_SEC)
#define MSEC_PER_SEC INT64_C(1000)

uint64_t get_time_ns(void);

#endif /* _COMMON_H */


// drm-atomic.c
/*
 * Copyright (c) 2017 Rob Clark <rclark@redhat.com>
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

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// #include "common.h"
// #include "drm-common.h"

#define VOID2U64(x) ((uint64_t)(unsigned long)(x))

static struct drm _drm = {
	.kms_out_fence_fd = -1,
};

static int add_connector_property(drmModeAtomicReq *req, uint32_t obj_id,
					const char *name, uint64_t value)
{
	struct connector *obj = _drm.connector;
	unsigned int i;
	int prop_id = 0;

	for (i = 0 ; i < obj->props->count_props ; i++) {
		if (strcmp(obj->props_info[i]->name, name) == 0) {
			prop_id = obj->props_info[i]->prop_id;
			break;
		}
	}

	if (prop_id < 0) {
		printf("no connector property: %s\n", name);
		return -EINVAL;
	}

	return drmModeAtomicAddProperty(req, obj_id, prop_id, value);
}

static int add_crtc_property(drmModeAtomicReq *req, uint32_t obj_id,
				const char *name, uint64_t value)
{
	struct crtc *obj = _drm.crtc;
	unsigned int i;
	int prop_id = -1;

	for (i = 0 ; i < obj->props->count_props ; i++) {
		if (strcmp(obj->props_info[i]->name, name) == 0) {
			prop_id = obj->props_info[i]->prop_id;
			break;
		}
	}

	if (prop_id < 0) {
		printf("no crtc property: %s\n", name);
		return -EINVAL;
	}

	return drmModeAtomicAddProperty(req, obj_id, prop_id, value);
}

static int add_plane_property(drmModeAtomicReq *req, uint32_t obj_id,
				const char *name, uint64_t value)
{
	struct plane *obj = _drm.plane;
	unsigned int i;
	int prop_id = -1;

	for (i = 0 ; i < obj->props->count_props ; i++) {
		if (strcmp(obj->props_info[i]->name, name) == 0) {
			prop_id = obj->props_info[i]->prop_id;
			break;
		}
	}


	if (prop_id < 0) {
		printf("no plane property: %s\n", name);
		return -EINVAL;
	}

	return drmModeAtomicAddProperty(req, obj_id, prop_id, value);
}

static int drm_atomic_commit(uint32_t fb_id, uint32_t flags)
{
	drmModeAtomicReq *req;
	uint32_t plane_id = _drm.plane->plane->plane_id;
	uint32_t blob_id;
	int ret;

	req = drmModeAtomicAlloc();

	if (flags & DRM_MODE_ATOMIC_ALLOW_MODESET) {
		if (add_connector_property(req, _drm.connector_id, "CRTC_ID",
						_drm.crtc_id) < 0)
				return -1;

		if (drmModeCreatePropertyBlob(_drm.fd, _drm.mode, sizeof(*_drm.mode),
						&blob_id) != 0)
			return -1;

		if (add_crtc_property(req, _drm.crtc_id, "MODE_ID", blob_id) < 0)
			return -1;

		if (add_crtc_property(req, _drm.crtc_id, "ACTIVE", 1) < 0)
			return -1;
	}

	add_plane_property(req, plane_id, "FB_ID", fb_id);
	add_plane_property(req, plane_id, "CRTC_ID", _drm.crtc_id);
	add_plane_property(req, plane_id, "SRC_X", 0);
	add_plane_property(req, plane_id, "SRC_Y", 0);
	add_plane_property(req, plane_id, "SRC_W", _drm.mode->hdisplay << 16);
	add_plane_property(req, plane_id, "SRC_H", _drm.mode->vdisplay << 16);
	add_plane_property(req, plane_id, "CRTC_X", 0);
	add_plane_property(req, plane_id, "CRTC_Y", 0);
	add_plane_property(req, plane_id, "CRTC_W", _drm.mode->hdisplay);
	add_plane_property(req, plane_id, "CRTC_H", _drm.mode->vdisplay);

	if (_drm.kms_in_fence_fd != -1) {
		add_crtc_property(req, _drm.crtc_id, "OUT_FENCE_PTR",
				VOID2U64(&_drm.kms_out_fence_fd));
		add_plane_property(req, plane_id, "IN_FENCE_FD", _drm.kms_in_fence_fd);
	}

	ret = drmModeAtomicCommit(_drm.fd, req, flags, NULL);
	if (ret)
		goto out;

	if (_drm.kms_in_fence_fd != -1) {
		close(_drm.kms_in_fence_fd);
		_drm.kms_in_fence_fd = -1;
	}

out:
	drmModeAtomicFree(req);

	return ret;
}

static EGLSyncKHR create_fence(const struct egl *egl, int fd)
{
	EGLint attrib_list[] = {
		EGL_SYNC_NATIVE_FENCE_FD_ANDROID, fd,
		EGL_NONE,
	};
	EGLSyncKHR fence = egl->eglCreateSyncKHR(egl->display,
			EGL_SYNC_NATIVE_FENCE_ANDROID, attrib_list);
	assert(fence);
	return fence;
}

static int atomic_run(const struct gbm *gbm, const struct egl *egl)
{
	struct gbm_bo *bo = NULL;
	struct drm_fb *fb;
	uint32_t i = 0;
	uint32_t flags = DRM_MODE_ATOMIC_NONBLOCK;
	uint64_t start_time, report_time, cur_time;
	int ret;

	if (egl_check(egl, eglDupNativeFenceFDANDROID) ||
		egl_check(egl, eglCreateSyncKHR) ||
		egl_check(egl, eglDestroySyncKHR) ||
		egl_check(egl, eglWaitSyncKHR) ||
		egl_check(egl, eglClientWaitSyncKHR))
		return -1;

	/* Allow a modeset change for the first commit only. */
	flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;

	start_time = report_time = get_time_ns();

	while (i < _drm.count) {
		unsigned frame = i;
		struct gbm_bo *next_bo;
		EGLSyncKHR gpu_fence = NULL;   /* out-fence from gpu, in-fence to kms */
		EGLSyncKHR kms_fence = NULL;   /* in-fence to gpu, out-fence from kms */

		if (_drm.kms_out_fence_fd != -1) {
			kms_fence = create_fence(egl, _drm.kms_out_fence_fd);
			assert(kms_fence);

			/* driver now has ownership of the fence fd: */
			_drm.kms_out_fence_fd = -1;

			/* wait "on the gpu" (ie. this won't necessarily block, but
			 * will block the rendering until fence is signaled), until
			 * the previous pageflip completes so we don't render into
			 * the buffer that is still on screen.
			 */
			egl->eglWaitSyncKHR(egl->display, kms_fence, 0);
		}

		/* Start fps measuring on second frame, to remove the time spent
		 * compiling shader, etc, from the fps:
		 */
		if (i == 1) {
			start_time = report_time = get_time_ns();
		}

		if (!gbm->surface) {
			glBindFramebuffer(GL_FRAMEBUFFER, egl->fbs[frame % NUM_BUFFERS].fb);
		}

		egl->draw(start_time, i++);

		/* insert fence to be singled in cmdstream.. this fence will be
		 * signaled when gpu rendering done
		 */
		gpu_fence = create_fence(egl, EGL_NO_NATIVE_FENCE_FD_ANDROID);
		assert(gpu_fence);

		if (gbm->surface) {
			eglSwapBuffers(egl->display, egl->surface);
		}

		/* after swapbuffers, gpu_fence should be flushed, so safe
		 * to get fd:
		 */
		_drm.kms_in_fence_fd = egl->eglDupNativeFenceFDANDROID(egl->display, gpu_fence);
		egl->eglDestroySyncKHR(egl->display, gpu_fence);
		assert(_drm.kms_in_fence_fd != -1);

		if (gbm->surface) {
			next_bo = gbm_surface_lock_front_buffer(gbm->surface);
		} else {
			next_bo = gbm->bos[frame % NUM_BUFFERS];
		}
		if (!next_bo) {
			printf("Failed to lock frontbuffer\n");
			return -1;
		}
		fb = drm_fb_get_from_bo(next_bo);
		if (!fb) {
			printf("Failed to get a new framebuffer BO\n");
			return -1;
		}

		if (kms_fence) {
			EGLint status;

			/* Wait on the CPU side for the _previous_ commit to
			 * complete before we post the flip through KMS, as
			 * atomic will reject the commit if we post a new one
			 * whilst the previous one is still pending.
			 */
			do {
				status = egl->eglClientWaitSyncKHR(egl->display,
								   kms_fence,
								   0,
								   EGL_FOREVER_KHR);
			} while (status != EGL_CONDITION_SATISFIED_KHR);

			egl->eglDestroySyncKHR(egl->display, kms_fence);
		}

		cur_time = get_time_ns();
		if (cur_time > (report_time + 2 * NSEC_PER_SEC)) {
			double elapsed_time = cur_time - start_time;
			double secs = elapsed_time / (double)NSEC_PER_SEC;
			unsigned frames = i - 1;  /* first frame ignored */
			printf("Rendered %u frames in %f sec (%f fps)\n",
				frames, secs, (double)frames/secs);
			report_time = cur_time;
		}

		/* Check for user input: */
		struct pollfd fdset[] = { {
			.fd = STDIN_FILENO,
			.events = POLLIN,
		} };
		ret = poll(fdset, ARRAY_SIZE(fdset), 0);
		if (ret > 0) {
			printf("user interrupted!\n");
			return 0;
		}

		/*
		 * Here you could also update drm plane layers if you want
		 * hw composition
		 */
		ret = drm_atomic_commit(fb->fb_id, flags);
		if (ret) {
			printf("failed to commit: %s\n", strerror(errno));
			return -1;
		}

		/* release last buffer to render on again: */
		if (bo && gbm->surface)
			gbm_surface_release_buffer(gbm->surface, bo);
		bo = next_bo;

		/* Allow a modeset change for the first commit only. */
		flags &= ~(DRM_MODE_ATOMIC_ALLOW_MODESET);
	}

	finish_perfcntrs();

	cur_time = get_time_ns();
	double elapsed_time = cur_time - start_time;
	double secs = elapsed_time / (double)NSEC_PER_SEC;
	unsigned frames = i - 1;  /* first frame ignored */
	printf("Rendered %u frames in %f sec (%f fps)\n",
		frames, secs, (double)frames/secs);

	dump_perfcntrs(frames, elapsed_time);

	return ret;
}

static int atomic_draw(const struct gbm *gbm, const struct egl *egl)
{
	static struct gbm_bo *bo = NULL;
	struct drm_fb *fb;
	static uint32_t i = 0;
	static uint32_t flags = DRM_MODE_ATOMIC_NONBLOCK;
	static uint64_t start_time = 0;
	static uint64_t report_time, cur_time;
	int ret;

	if (!start_time) {
		if (egl_check(egl, eglDupNativeFenceFDANDROID) ||
			egl_check(egl, eglCreateSyncKHR) ||
			egl_check(egl, eglDestroySyncKHR) ||
			egl_check(egl, eglWaitSyncKHR) ||
			egl_check(egl, eglClientWaitSyncKHR))
			return -1;

		/* Allow a modeset change for the first commit only. */
		flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;

		start_time = report_time = get_time_ns();
	}

	unsigned frame = i;
	struct gbm_bo *next_bo;
	EGLSyncKHR gpu_fence = NULL;   /* out-fence from gpu, in-fence to kms */
	EGLSyncKHR kms_fence = NULL;   /* in-fence to gpu, out-fence from kms */

	if (_drm.kms_out_fence_fd != -1) {
		kms_fence = create_fence(egl, _drm.kms_out_fence_fd);
		assert(kms_fence);

		/* driver now has ownership of the fence fd: */
		_drm.kms_out_fence_fd = -1;

		/* wait "on the gpu" (ie. this won't necessarily block, but
		 * will block the rendering until fence is signaled), until
		 * the previous pageflip completes so we don't render into
		 * the buffer that is still on screen.
		 */
		egl->eglWaitSyncKHR(egl->display, kms_fence, 0);
	}

	/* Start fps measuring on second frame, to remove the time spent
	 * compiling shader, etc, from the fps:
	 */
	if (i == 1) {
		start_time = report_time = get_time_ns();
	}

	if (!gbm->surface) {
		glBindFramebuffer(GL_FRAMEBUFFER, egl->fbs[frame % NUM_BUFFERS].fb);
	}

	egl->draw(start_time, i++);

	/* insert fence to be singled in cmdstream.. this fence will be
	 * signaled when gpu rendering done
	 */
	gpu_fence = create_fence(egl, EGL_NO_NATIVE_FENCE_FD_ANDROID);
	assert(gpu_fence);

	if (gbm->surface) {
		eglSwapBuffers(egl->display, egl->surface);
	}

	/* after swapbuffers, gpu_fence should be flushed, so safe
	 * to get fd:
	 */
	_drm.kms_in_fence_fd = egl->eglDupNativeFenceFDANDROID(egl->display, gpu_fence);
	egl->eglDestroySyncKHR(egl->display, gpu_fence);
	assert(_drm.kms_in_fence_fd != -1);

	if (gbm->surface) {
		next_bo = gbm_surface_lock_front_buffer(gbm->surface);
	} else {
		next_bo = gbm->bos[frame % NUM_BUFFERS];
	}
	if (!next_bo) {
		printf("Failed to lock frontbuffer\n");
		return -1;
	}
	fb = drm_fb_get_from_bo(next_bo);
	if (!fb) {
		printf("Failed to get a new framebuffer BO\n");
		return -1;
	}

	if (kms_fence) {
		EGLint status;

		/* Wait on the CPU side for the _previous_ commit to
		 * complete before we post the flip through KMS, as
		 * atomic will reject the commit if we post a new one
		 * whilst the previous one is still pending.
		 */
		do {
			status = egl->eglClientWaitSyncKHR(egl->display,
							   kms_fence,
							   0,
							   EGL_FOREVER_KHR);
		} while (status != EGL_CONDITION_SATISFIED_KHR);

		egl->eglDestroySyncKHR(egl->display, kms_fence);
	}

#if 0
	cur_time = get_time_ns();
	if (cur_time > (report_time + 2 * NSEC_PER_SEC)) {
		double elapsed_time = cur_time - start_time;
		double secs = elapsed_time / (double)NSEC_PER_SEC;
		unsigned frames = i - 1;  /* first frame ignored */
		printf("Rendered %u frames in %f sec (%f fps)\n",
			frames, secs, (double)frames/secs);
		report_time = cur_time;
	}
#endif

	/* Check for user input: */
/*	struct pollfd fdset[] = { {
		.fd = STDIN_FILENO,
		.events = POLLIN,
	} };
	ret = poll(fdset, ARRAY_SIZE(fdset), 0);
	if (ret > 0) {
		printf("user interrupted!\n");
		return 0;
	}*/

	/*
	 * Here you could also update drm plane layers if you want
	 * hw composition
	 */
	ret = drm_atomic_commit(fb->fb_id, flags);
	if (ret) {
		printf("failed to commit: %s\n", strerror(errno));
		return -1;
	}

	/* release last buffer to render on again: */
	if (bo && gbm->surface)
		gbm_surface_release_buffer(gbm->surface, bo);
	bo = next_bo;

	/* Allow a modeset change for the first commit only. */
	flags &= ~(DRM_MODE_ATOMIC_ALLOW_MODESET);

	return ret;
}

/* Pick a plane.. something that at a minimum can be connected to
 * the chosen crtc, but prefer primary plane.
 *
 * Seems like there is some room for a drmModeObjectGetNamedProperty()
 * type helper in lib_drm..
 */
static int get_plane_id(void)
{
	drmModePlaneResPtr plane_resources;
	uint32_t i, j;
	int ret = -EINVAL;
	int found_primary = 0;

	plane_resources = drmModeGetPlaneResources(_drm.fd);
	if (!plane_resources) {
		printf("drmModeGetPlaneResources failed: %s\n", strerror(errno));
		return -1;
	}

	for (i = 0; (i < plane_resources->count_planes) && !found_primary; i++) {
		uint32_t id = plane_resources->planes[i];
		drmModePlanePtr plane = drmModeGetPlane(_drm.fd, id);
		if (!plane) {
			printf("drmModeGetPlane(%u) failed: %s\n", id, strerror(errno));
			continue;
		}

		if (plane->possible_crtcs & (1 << _drm.crtc_index)) {
			drmModeObjectPropertiesPtr props =
				drmModeObjectGetProperties(_drm.fd, id, DRM_MODE_OBJECT_PLANE);

			/* primary or not, this plane is good enough to use: */
			ret = id;

			for (j = 0; j < props->count_props; j++) {
				drmModePropertyPtr p =
					drmModeGetProperty(_drm.fd, props->props[j]);

				if ((strcmp(p->name, "type") == 0) &&
						(props->prop_values[j] == DRM_PLANE_TYPE_PRIMARY)) {
					/* found our primary plane, lets use that: */
					found_primary = 1;
				}

				drmModeFreeProperty(p);
			}

			drmModeFreeObjectProperties(props);
		}

		drmModeFreePlane(plane);
	}

	drmModeFreePlaneResources(plane_resources);

	return ret;
}

const struct drm * init_drm_atomic(const char *device, const char *mode_str,
		unsigned int vrefresh, unsigned int count)
{
	uint32_t plane_id;
	int ret;

	ret = init_drm(&_drm, device, mode_str, vrefresh, count);
	if (ret)
		return NULL;

	ret = drmSetClientCap(_drm.fd, DRM_CLIENT_CAP_ATOMIC, 1);
	if (ret) {
		printf("no atomic modesetting support: %s\n", strerror(errno));
		return NULL;
	}

	ret = get_plane_id();
	if (!ret) {
		printf("could not find a suitable plane\n");
		return NULL;
	} else {
		plane_id = ret;
	}

	/* We only do single plane to single crtc to single connector, no
	 * fancy multi-monitor or multi-plane stuff.  So just grab the
	 * plane/crtc/connector property info for one of each:
	 */
	_drm.plane = calloc(1, sizeof(*_drm.plane));
	_drm.crtc = calloc(1, sizeof(*_drm.crtc));
	_drm.connector = calloc(1, sizeof(*_drm.connector));

#define get_resource(type, Type, id) do { 					\
		_drm.type->type = drmModeGet##Type(_drm.fd, id);			\
		if (!_drm.type->type) {						\
			printf("could not get %s %i: %s\n",			\
					#type, id, strerror(errno));		\
			return NULL;						\
		}								\
	} while (0)

	get_resource(plane, Plane, plane_id);
	get_resource(crtc, Crtc, _drm.crtc_id);
	get_resource(connector, Connector, _drm.connector_id);

#define get_properties(type, TYPE, id) do {					\
		uint32_t i;							\
		_drm.type->props = drmModeObjectGetProperties(_drm.fd,		\
				id, DRM_MODE_OBJECT_##TYPE);			\
		if (!_drm.type->props) {						\
			printf("could not get %s %u properties: %s\n", 		\
					#type, id, strerror(errno));		\
			return NULL;						\
		}								\
		_drm.type->props_info = calloc(_drm.type->props->count_props,	\
				sizeof(*_drm.type->props_info));			\
		for (i = 0; i < _drm.type->props->count_props; i++) {		\
			_drm.type->props_info[i] = drmModeGetProperty(_drm.fd,	\
					_drm.type->props->props[i]);		\
		}								\
	} while (0)

	get_properties(plane, PLANE, plane_id);
	get_properties(crtc, CRTC, _drm.crtc_id);
	get_properties(connector, CONNECTOR, _drm.connector_id);

	_drm.run = atomic_run;
	_drm.draw = atomic_draw;

	return &_drm;
}


// common.c
/*
 * Copyright (c) 2017 Rob Clark <rclark@redhat.com>
 * Copyright © 2013 Intel Corporation
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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// #include "common.h"

static struct gbm kg_gbm;
static struct egl kg_egl;

WEAK struct gbm_surface *
gbm_surface_create_with_modifiers(struct gbm_device *gbm,
				uint32_t width, uint32_t height,
				uint32_t format,
				const uint64_t *modifiers,
				const unsigned int count);
WEAK struct gbm_bo *
gbm_bo_create_with_modifiers(struct gbm_device *gbm,
				uint32_t width, uint32_t height,
				uint32_t format,
				const uint64_t *modifiers,
				const unsigned int count);

static struct gbm_bo * init_bo(uint64_t modifier)
{
	struct gbm_bo *bo = NULL;

	if (gbm_bo_create_with_modifiers) {
		bo = gbm_bo_create_with_modifiers(kg_gbm.dev,
						kg_gbm.width, kg_gbm.height,
						kg_gbm.format,
						&modifier, 1);
	}

	if (!bo) {
		if (modifier != DRM_FORMAT_MOD_LINEAR) {
			fprintf(stderr, "Modifiers requested but support isn't available\n");
			return NULL;
		}

		bo = gbm_bo_create(kg_gbm.dev,
				kg_gbm.width, kg_gbm.height,
				kg_gbm.format,
				GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	}

	if (!bo) {
		printf("failed to create gbm bo\n");
		return NULL;
	}

	return bo;
}

static struct gbm * init_surfaceless(uint64_t modifier)
{
	for (unsigned i = 0; i < ARRAY_SIZE(kg_gbm.bos); i++) {
		kg_gbm.bos[i] = init_bo(modifier);
		if (!kg_gbm.bos[i])
			return NULL;
	}
	return &kg_gbm;
}

static struct gbm * init_surface(uint64_t modifier)
{
	if (gbm_surface_create_with_modifiers) {
		kg_gbm.surface = gbm_surface_create_with_modifiers(kg_gbm.dev,
								kg_gbm.width, kg_gbm.height,
								kg_gbm.format,
								&modifier, 1);

	}

	if (!kg_gbm.surface) {
		if (modifier != DRM_FORMAT_MOD_LINEAR) {
			fprintf(stderr, "Modifiers requested but support isn't available\n");
			return NULL;
		}
		kg_gbm.surface = gbm_surface_create(kg_gbm.dev,
						kg_gbm.width, kg_gbm.height,
						kg_gbm.format,
						GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);

	}

	if (!kg_gbm.surface) {
		printf("failed to create GBM surface\n");
		return NULL;
	}

	return &kg_gbm;
}

const struct gbm * init_gbm(int drm_fd, int w, int h, uint32_t format,
		uint64_t modifier, bool surfaceless)
{
	kg_gbm.dev = gbm_create_device(drm_fd);
	kg_gbm.format = format;
	kg_gbm.surface = NULL;

	kg_gbm.width = w;
	kg_gbm.height = h;

	if (surfaceless)
		return init_surfaceless(modifier);

	return init_surface(modifier);
}

void finish_gbm()
{
	if (kg_gbm.surface) gbm_surface_destroy(kg_gbm.surface);
	gbm_device_destroy(kg_gbm.dev);
}

static bool has_ext(const char *extension_list, const char *ext)
{
	const char *ptr = extension_list;
	int len = strlen(ext);

	if (ptr == NULL || *ptr == '\0')
		return false;

	while (true) {
		ptr = strstr(ptr, ext);
		if (!ptr)
			return false;

		if (ptr[len] == ' ' || ptr[len] == '\0')
			return true;

		ptr += len;
	}
}

static int
match_config_to_visual(EGLDisplay egl_display,
			EGLint visual_id,
			EGLConfig *configs,
			int count)
{
	int i;

	for (i = 0; i < count; ++i) {
		EGLint id;

		if (!eglGetConfigAttrib(egl_display,
				configs[i], EGL_NATIVE_VISUAL_ID,
				&id))
			continue;

		if (id == visual_id)
			return i;
	}

	return -1;
}

static bool
egl_choose_config(EGLDisplay egl_display, const EGLint *attribs,
				EGLint visual_id, EGLConfig *config_out)
{
	EGLint count = 0;
	EGLint matched = 0;
	EGLConfig *configs;
	int config_index = -1;

	if (!eglGetConfigs(egl_display, NULL, 0, &count) || count < 1) {
		printf("No EGL configs to choose from.\n");
		return false;
	}
	configs = malloc(count * sizeof *configs);
	if (!configs)
		return false;

	if (!eglChooseConfig(egl_display, attribs, configs,
					count, &matched) || !matched) {
		printf("No EGL configs with appropriate attributes.\n");
		goto out;
	}

	if (!visual_id)
		config_index = 0;

	if (config_index == -1)
		config_index = match_config_to_visual(egl_display,
							visual_id,
							configs,
							matched);

	if (config_index != -1)
		*config_out = configs[config_index];

out:
	free(configs);
	if (config_index == -1)
		return false;

	return true;
}

static bool
create_framebuffer(const struct egl *egl, struct gbm_bo *bo,
		struct framebuffer *fb) {
	assert(egl->eglCreateImageKHR);
	assert(bo);
	assert(fb);

	// 1. Create EGLImage.
	int fd = gbm_bo_get_fd(bo);
	if (fd < 0) {
		printf("failed to get fd for bo: %d\n", fd);
		return false;
	}

	EGLint khr_image_attrs[17] = {
		EGL_WIDTH, gbm_bo_get_width(bo),
		EGL_HEIGHT, gbm_bo_get_height(bo),
		EGL_LINUX_DRM_FOURCC_EXT, (int)gbm_bo_get_format(bo),
		EGL_DMA_BUF_PLANE0_FD_EXT, fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, gbm_bo_get_stride(bo),
		EGL_NONE, EGL_NONE,	/* modifier lo */
		EGL_NONE, EGL_NONE,	/* modifier hi */
		EGL_NONE,
	};

	if (egl->modifiers_supported) {
		const uint64_t modifier = gbm_bo_get_modifier(bo);
		if (modifier != DRM_FORMAT_MOD_LINEAR) {
			size_t attrs_index = 12;
			khr_image_attrs[attrs_index++] =
				EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
			khr_image_attrs[attrs_index++] = modifier & 0xfffffffful;
			khr_image_attrs[attrs_index++] =
				EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
			khr_image_attrs[attrs_index++] = modifier >> 32;
		}
	}

	fb->image = egl->eglCreateImageKHR(egl->display, EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, NULL /* no client buffer */,
			khr_image_attrs);

	if (fb->image == EGL_NO_IMAGE_KHR) {
		printf("failed to make image from buffer object\n");
		return false;
	}

	// EGLImage takes the fd ownership
	close(fd);

	// 2. Create GL texture and framebuffer
	glGenTextures(1, &fb->tex);
	glBindTexture(GL_TEXTURE_2D, fb->tex);
	egl->glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, fb->image);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &fb->fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb->fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			fb->tex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("failed framebuffer check for created target buffer\n");
		glDeleteFramebuffers(1, &fb->fb);
		glDeleteTextures(1, &fb->tex);
		return false;
	}

	return true;
}

const struct egl *init_egl(const struct gbm *gbm)
{
	EGLint major, minor;

	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 0,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	const char *egl_exts_client, *egl_exts_dpy, *gl_exts;

#define get_proc_client(ext, name) do { \
		if (has_ext(egl_exts_client, #ext)) \
			kg_egl.name = (void *)eglGetProcAddress(#name); \
	} while (0)
#define get_proc_dpy(ext, name) do { \
		if (has_ext(egl_exts_dpy, #ext)) \
			kg_egl.name = (void *)eglGetProcAddress(#name); \
	} while (0)

#define get_proc_gl(ext, name) do { \
		if (has_ext(gl_exts, #ext)) \
			kg_egl.name = (void *)eglGetProcAddress(#name); \
	} while (0)

	egl_exts_client = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	get_proc_client(EGL_EXT_platform_base, eglGetPlatformDisplayEXT);

	if (kg_egl.eglGetPlatformDisplayEXT) {
		kg_egl.display = kg_egl.eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR,
				gbm->dev, NULL);
	} else {
		kg_egl.display = eglGetDisplay((void *)gbm->dev);
	}

	if (!eglInitialize(kg_egl.display, &major, &minor)) {
		printf("failed to initialize\n");
		return NULL;
	}

	egl_exts_dpy = eglQueryString(kg_egl.display, EGL_EXTENSIONS);
	get_proc_dpy(EGL_KHR_image_base, eglCreateImageKHR);
	get_proc_dpy(EGL_KHR_image_base, eglDestroyImageKHR);
	get_proc_dpy(EGL_KHR_fence_sync, eglCreateSyncKHR);
	get_proc_dpy(EGL_KHR_fence_sync, eglDestroySyncKHR);
	get_proc_dpy(EGL_KHR_fence_sync, eglWaitSyncKHR);
	get_proc_dpy(EGL_KHR_fence_sync, eglClientWaitSyncKHR);
	get_proc_dpy(EGL_ANDROID_native_fence_sync, eglDupNativeFenceFDANDROID);

	kg_egl.modifiers_supported = has_ext(egl_exts_dpy,
					"EGL_EXT_image_dma_buf_import_modifiers");

	printf("Using display %p with EGL version %d.%d\n",
			kg_egl.display, major, minor);

	printf("===================================\n");
	printf("EGL information:\n");
	printf("  version: \"%s\"\n", eglQueryString(kg_egl.display, EGL_VERSION));
	printf("  vendor: \"%s\"\n", eglQueryString(kg_egl.display, EGL_VENDOR));
	printf("  client extensions: \"%s\"\n", egl_exts_client);
	printf("  display extensions: \"%s\"\n", egl_exts_dpy);
	printf("===================================\n");

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		printf("failed to bind api EGL_OPENGL_ES_API\n");
		return NULL;
	}

	if (!egl_choose_config(kg_egl.display, config_attribs, gbm->format,
			&kg_egl.config)) {
		printf("failed to choose config\n");
		return NULL;
	}

	kg_egl.context = eglCreateContext(kg_egl.display, kg_egl.config,
			EGL_NO_CONTEXT, context_attribs);
	if (kg_egl.context == NULL) {
		printf("failed to create context\n");
		return NULL;
	}

	if (!gbm->surface) {
		kg_egl.surface = EGL_NO_SURFACE;
	} else {
		kg_egl.surface = eglCreateWindowSurface(kg_egl.display, kg_egl.config,
				(EGLNativeWindowType)gbm->surface, NULL);
		if (kg_egl.surface == EGL_NO_SURFACE) {
			printf("failed to create EGL surface\n");
			return NULL;
		}
	}

	/* connect the context to the surface */
	eglMakeCurrent(kg_egl.display, kg_egl.surface, kg_egl.surface, kg_egl.context);

	gl_exts = (char *) glGetString(GL_EXTENSIONS);
	printf("OpenGL ES 2.x information:\n");
	printf("  version: \"%s\"\n", glGetString(GL_VERSION));
	printf("  shading language version: \"%s\"\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("  vendor: \"%s\"\n", glGetString(GL_VENDOR));
	printf("  renderer: \"%s\"\n", glGetString(GL_RENDERER));
	printf("  extensions: \"%s\"\n", gl_exts);
	printf("===================================\n");

	get_proc_gl(GL_OES_EGL_image, glEGLImageTargetTexture2DOES);

	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorGroupsAMD);
	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorCountersAMD);
	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorGroupStringAMD);
	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorCounterStringAMD);
	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorCounterInfoAMD);
	get_proc_gl(GL_AMD_performance_monitor, glGenPerfMonitorsAMD);
	get_proc_gl(GL_AMD_performance_monitor, glDeletePerfMonitorsAMD);
	get_proc_gl(GL_AMD_performance_monitor, glSelectPerfMonitorCountersAMD);
	get_proc_gl(GL_AMD_performance_monitor, glBeginPerfMonitorAMD);
	get_proc_gl(GL_AMD_performance_monitor, glEndPerfMonitorAMD);
	get_proc_gl(GL_AMD_performance_monitor, glGetPerfMonitorCounterDataAMD);

	if (!gbm->surface) {
		for (unsigned i = 0; i < ARRAY_SIZE(gbm->bos); i++) {
			if (!create_framebuffer(&kg_egl, gbm->bos[i], &kg_egl.fbs[i])) {
				printf("failed to create framebuffer\n");
				return NULL;
			}
		}
	}

	return &kg_egl;
}

void finish_egl()
{
	// Release context resources
	eglMakeCurrent(kg_egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(kg_egl.display, kg_egl.surface);
	eglDestroyContext(kg_egl.display, kg_egl.context);
	eglTerminate(kg_egl.display);
}

int create_program(const char *vs_src, const char *fs_src)
{
	GLuint vertex_shader, fragment_shader, program;
	GLint ret;

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertex_shader, 1, &vs_src, NULL);
	glCompileShader(vertex_shader);

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("vertex shader compilation failed!:\n");
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &ret);
		if (ret > 1) {
			log = malloc(ret);
			glGetShaderInfoLog(vertex_shader, ret, NULL, log);
			printf("%s", log);
			free(log);
		}

		return -1;
	}

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragment_shader, 1, &fs_src, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("fragment shader compilation failed!:\n");
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &ret);

		if (ret > 1) {
			log = malloc(ret);
			glGetShaderInfoLog(fragment_shader, ret, NULL, log);
			printf("%s", log);
			free(log);
		}

		return -1;
	}

	program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	return program;
}

int link_program(unsigned program)
{
	GLint ret;

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("program linking failed!:\n");
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &ret);

		if (ret > 1) {
			log = malloc(ret);
			glGetProgramInfoLog(program, ret, NULL, log);
			printf("%s", log);
			free(log);
		}

		return -1;
	}

	return 0;
}

uint64_t get_time_ns(void)
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return tv.tv_nsec + tv.tv_sec * NSEC_PER_SEC;
}


// drm-legacy.c
/*
 * Copyright (c) 2017 Rob Clark <rclark@redhat.com>
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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

// #include "common.h"
// #include "drm-common.h"

static struct drm kg_drm;

static void page_flip_handler(int fd, unsigned int frame,
			unsigned int sec, unsigned int usec, void *data)
{
	/* suppress 'unused parameter' warnings */
	(void)fd, (void)frame, (void)sec, (void)usec;

	int *waiting_for_flip = data;
	*waiting_for_flip = 0;
}

static int legacy_run(const struct gbm *gbm, const struct egl *egl)
{
	fd_set fds;
	drmEventContext evctx = {
		.version = 2,
		.page_flip_handler = page_flip_handler,
	};
	struct gbm_bo *bo;
	struct drm_fb *fb;
	uint32_t i = 0;
	uint64_t start_time, report_time, cur_time;
	int ret;

	if (gbm->surface) {
		eglSwapBuffers(egl->display, egl->surface);
		bo = gbm_surface_lock_front_buffer(gbm->surface);
	} else {
		bo = gbm->bos[0];
	}
	fb = drm_fb_get_from_bo(bo);
	if (!fb) {
		fprintf(stderr, "Failed to get a new framebuffer BO\n");
		return -1;
	}

	/* set mode: */
	ret = drmModeSetCrtc(kg_drm.fd, kg_drm.crtc_id, fb->fb_id, 0, 0,
			&kg_drm.connector_id, 1, kg_drm.mode);
	if (ret) {
		printf("failed to set mode: %s\n", strerror(errno));
		return ret;
	}

	start_time = report_time = get_time_ns();

	while (i < kg_drm.count) {
		unsigned frame = i;
		struct gbm_bo *next_bo;
		int waiting_for_flip = 1;

		/* Start fps measuring on second frame, to remove the time spent
		 * compiling shader, etc, from the fps:
		 */
		if (i == 1) {
			start_time = report_time = get_time_ns();
		}

		if (!gbm->surface) {
			glBindFramebuffer(GL_FRAMEBUFFER, egl->fbs[frame % NUM_BUFFERS].fb);
		}

		egl->draw(start_time, i++);

		if (gbm->surface) {
			eglSwapBuffers(egl->display, egl->surface);
			next_bo = gbm_surface_lock_front_buffer(gbm->surface);
		} else {
			glFinish();
			next_bo = gbm->bos[frame % NUM_BUFFERS];
		}
		fb = drm_fb_get_from_bo(next_bo);
		if (!fb) {
			fprintf(stderr, "Failed to get a new framebuffer BO\n");
			return -1;
		}

		/*
		 * Here you could also update drm plane layers if you want
		 * hw composition
		 */

		ret = drmModePageFlip(kg_drm.fd, kg_drm.crtc_id, fb->fb_id,
				DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip);
		if (ret) {
			printf("failed to queue page flip: %s\n", strerror(errno));
			return -1;
		}

		while (waiting_for_flip) {
			FD_ZERO(&fds);
			FD_SET(0, &fds);
			FD_SET(kg_drm.fd, &fds);

			ret = select(kg_drm.fd + 1, &fds, NULL, NULL, NULL);
			if (ret < 0) {
				printf("select err: %s\n", strerror(errno));
				return ret;
			} else if (ret == 0) {
				printf("select timeout!\n");
				return -1;
			} else if (FD_ISSET(0, &fds)) {
				printf("user interrupted!\n");
				return 0;
			}
			drmHandleEvent(kg_drm.fd, &evctx);
		}

		cur_time = get_time_ns();
		if (cur_time > (report_time + 2 * NSEC_PER_SEC)) {
			double elapsed_time = cur_time - start_time;
			double secs = elapsed_time / (double)NSEC_PER_SEC;
			unsigned frames = i - 1;  /* first frame ignored */
			printf("Rendered %u frames in %f sec (%f fps)\n",
				frames, secs, (double)frames/secs);
			report_time = cur_time;
		}

		/* release last buffer to render on again: */
		if (gbm->surface) {
			gbm_surface_release_buffer(gbm->surface, bo);
		}
		bo = next_bo;
	}

	finish_perfcntrs();

	cur_time = get_time_ns();
	double elapsed_time = cur_time - start_time;
	double secs = elapsed_time / (double)NSEC_PER_SEC;
	unsigned frames = i - 1;  /* first frame ignored */
	printf("Rendered %u frames in %f sec (%f fps)\n",
		frames, secs, (double)frames/secs);

	dump_perfcntrs(frames, elapsed_time);

	return 0;
}

static int legacy_draw(const struct gbm *gbm, const struct egl *egl)
{
	fd_set fds;
	drmEventContext evctx = {
		.version = 2,
		.page_flip_handler = page_flip_handler,
	};
	static struct gbm_bo *bo;
	struct drm_fb *fb;
	static uint32_t i = 0;
	static uint64_t start_time = 0;
	static uint64_t report_time, cur_time;
	int ret;

	if (!start_time) {
		if (gbm->surface) {
			eglSwapBuffers(egl->display, egl->surface);
			bo = gbm_surface_lock_front_buffer(gbm->surface);
		} else {
			bo = gbm->bos[0];
		}
		fb = drm_fb_get_from_bo(bo);
		if (!fb) {
			fprintf(stderr, "Failed to get a new framebuffer BO\n");
			return -1;
		}

		/* set mode: */
		ret = drmModeSetCrtc(kg_drm.fd, kg_drm.crtc_id, fb->fb_id, 0, 0,
				&kg_drm.connector_id, 1, kg_drm.mode);
		if (ret) {
			printf("failed to set mode: %s\n", strerror(errno));
			return ret;
		}

		start_time = report_time = get_time_ns();
	}

	unsigned frame = i;
	struct gbm_bo *next_bo;
	int waiting_for_flip = 1;

	/* Start fps measuring on second frame, to remove the time spent
	 * compiling shader, etc, from the fps:
	 */
	if (i == 1) {
		start_time = report_time = get_time_ns();
	}

	if (!gbm->surface) {
		glBindFramebuffer(GL_FRAMEBUFFER, egl->fbs[frame % NUM_BUFFERS].fb);
	}

	egl->draw(start_time, i++);

	if (gbm->surface) {
		eglSwapBuffers(egl->display, egl->surface);
		next_bo = gbm_surface_lock_front_buffer(gbm->surface);
	} else {
		glFinish();
		next_bo = gbm->bos[frame % NUM_BUFFERS];
	}
	fb = drm_fb_get_from_bo(next_bo);
	if (!fb) {
		fprintf(stderr, "Failed to get a new framebuffer BO\n");
		return -1;
	}

	/*
	 * Here you could also update drm plane layers if you want
	 * hw composition
	 */

	ret = drmModePageFlip(kg_drm.fd, kg_drm.crtc_id, fb->fb_id,
			DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip);
	if (ret) {
		printf("failed to queue page flip: %s\n", strerror(errno));
		return -1;
	}

	while (waiting_for_flip) {
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(kg_drm.fd, &fds);

		ret = select(kg_drm.fd + 1, &fds, NULL, NULL, NULL);
		if (ret < 0) {
			printf("select err: %s\n", strerror(errno));
			return ret;
		} else if (ret == 0) {
			printf("select timeout!\n");
			return -1;
		/*} else if (FD_ISSET(0, &fds)) {
			printf("user interrupted!\n");
			return 0;*/
		}
		drmHandleEvent(kg_drm.fd, &evctx);
	}

#if 0
	cur_time = get_time_ns();
	if (cur_time > (report_time + 2 * NSEC_PER_SEC)) {
		double elapsed_time = cur_time - start_time;
		double secs = elapsed_time / (double)NSEC_PER_SEC;
		unsigned frames = i - 1;  /* first frame ignored */
		printf("Rendered %u frames in %f sec (%f fps)\n",
			frames, secs, (double)frames/secs);
		report_time = cur_time;
	}
#endif

	/* release last buffer to render on again: */
	if (gbm->surface) {
		gbm_surface_release_buffer(gbm->surface, bo);
	}
	bo = next_bo;

	return 0;
}

const struct drm * init_drm_legacy(const char *device, const char *mode_str,
		unsigned int vrefresh, unsigned int count)
{
	int ret;

	ret = init_drm(&kg_drm, device, mode_str, vrefresh, count);
	if (ret)
		return NULL;

	kg_drm.run = legacy_run;
	kg_drm.draw = legacy_draw;

	return &kg_drm;
}


// shadertoy.c
/*
 * Copyright © 2020 Antonin Stefanutti <antonin.stefanutti@gmail.com>
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

#define _GNU_SOURCE

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <GLES3/gl3.h>

// #include "common.h"

GLint iTime, iFrame;

/*static const char *shadertoy_vs =
		"attribute vec3 position;                \n"
		"void main()                             \n"
		"{                                       \n"
		"    gl_Position = vec4(position, 1.0);  \n"
		"}                                       \n";

static const char *shadertoy_fs_tmpl =
		"precision mediump float;                                                             \n"
		"uniform vec3      iResolution;           // viewport resolution (in pixels)          \n"
		"uniform float     iTime;                 // shader playback time (in seconds)        \n"
		"uniform int       iFrame;                // current frame number                     \n"
		"uniform vec4      iMouse;                // mouse pixel coords                       \n"
		"uniform vec4      iDate;                 // (year, month, day, time in seconds)      \n"
		"uniform float     iSampleRate;           // sound sample rate (i.e., 44100)          \n"
		"uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)           \n"
		"uniform float     iChannelTime[4];       // channel playback time (in sec)           \n"
		"                                                                                     \n"
		"%s                                                                                   \n"
		"                                                                                     \n"
		"void main()                                                                          \n"
		"{                                                                                    \n"
		"    mainImage(gl_FragColor, gl_FragCoord.xy);                                        \n"
		"}                                                                                    \n";*/

static const char shadertoy_vs[] =
    "#version 300 es\n"
    "precision highp float;\n"
    "layout(location = 0) in vec4 iPosition;"
    "void main() {"
    "  gl_Position=iPosition;"
    "}\n";

static const char shadertoy_fs_tmpl[] =
    "#version 300 es\n"
    "precision highp float;\n"
    "uniform vec3 iResolution;"
    "uniform float iGlobalTime;" // legacy
    "uniform float iTime;"
    "uniform float iTimeDelta;"
    "uniform int   iFrame;"
    "uniform float iFrameRate;"
    "uniform float iChannelTime[4];"
    "uniform vec4 iMouse;"
    "uniform vec4 iDate;"
    "uniform float iSampleRate;"
    "uniform vec3 iChannelResolution[4];"
    "uniform sampler2D iChannel0;"
    "uniform sampler2D iChannel1;"
    "uniform sampler2D iChannel2;"
    "uniform sampler2D iChannel3;"
    "out vec4 fragColor;\n"
    "%s"
    "\nvoid main(){mainImage(fragColor, gl_FragCoord.xy);}";

static const GLfloat vertices[] = {
		// First triangle:
		1.0f, 1.0f,
		-1.0f, 1.0f,
		-1.0f, -1.0f,
		// Second triangle:
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
};

static char *load_shader(const char *file) {
	struct stat statbuf;
	char *frag;
	int fd, ret;

	fd = open(file, 0);
	if (fd < 0) {
		err(fd, "could not open '%s'", file);
	}

	ret = fstat(fd, &statbuf);
	if (ret < 0) {
		err(ret, "could not stat '%s'", file);
	}

	const char *text = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	asprintf(&frag, shadertoy_fs_tmpl, text);

	return frag;
}

static void draw_shadertoy(uint64_t start_time, unsigned frame) {
	glUniform1f(iTime, (get_time_ns() - start_time) / (double) NSEC_PER_SEC);
	// Replace the above to input ellapsed time relative to 60 FPS
	// glUniform1f(iTime, (float) frame / 60.0f);
	glUniform1ui(iFrame, frame);

	start_perfcntrs();

	glDrawArrays(GL_TRIANGLES, 0, 6);

	end_perfcntrs();
}

static GLint sampler_channel[4];
static GLuint sampler_channel_ID[4];
static void update_texture(int n, unsigned char* data, int w, int h)
{
	glActiveTexture(GL_TEXTURE0 + n);
	glBindTexture(GL_TEXTURE_2D, sampler_channel_ID[n]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	glUniform1i(sampler_channel[n], n);
}
static void texture_bind(unsigned char* data, int w, int h)
{
	for (int i=/*0*/3; i<4; ++i) {
		if (sampler_channel[i] < 0) {
//			info("Skipping data for unused iChannel%d\n", i);
		} else {
//			info("Binding data for iChannel%d\n", i);
			glGenTextures(1, &sampler_channel_ID[i]);
			update_texture(i, data, 256, 3);
			break;
		}
	}
	glActiveTexture(GL_TEXTURE0);
}

int init_shadertoy(const struct gbm *gbm, struct egl *egl, const char *file) {
	int ret;
	char *shadertoy_fs;
	GLuint program, vbo;
	GLint iResolution;

	shadertoy_fs = load_shader(file);

	ret = create_program(shadertoy_vs, shadertoy_fs);
	if (ret < 0) {
		printf("failed to create program\n");
		return -1;
	}

	program = ret;

	ret = link_program(program);
	if (ret) {
		printf("failed to link program\n");
		return -1;
	}

	glViewport(0, 0, gbm->width, gbm->height);
	glUseProgram(program);
	iTime = glGetUniformLocation(program, "iTime");
	iFrame = glGetUniformLocation(program, "iFrame");
	iResolution = glGetUniformLocation(program, "iResolution");
//	attrib_position = glGetAttribLocation(shader_program, "iPosition");
	sampler_channel[0] = glGetUniformLocation(program, "iChannel0");
	sampler_channel[1] = glGetUniformLocation(program, "iChannel1");
	sampler_channel[2] = glGetUniformLocation(program, "iChannel2");
	sampler_channel[3] = glGetUniformLocation(program, "iChannel3");
/*	uniform_cres = glGetUniformLocation(shader_program, "iChannelResolution");
	uniform_ctime = glGetUniformLocation(shader_program, "iChannelTime");
	uniform_date = glGetUniformLocation(shader_program, "iDate");
	uniform_gtime = glGetUniformLocation(shader_program, "iGlobalTime");
	uniform_time = glGetUniformLocation(shader_program, "iTime");
	uniform_mouse = glGetUniformLocation(shader_program, "iMouse");
	uniform_res = glGetUniformLocation(shader_program, "iResolution");
	uniform_srate = glGetUniformLocation(shader_program, "iSampleRate");*/
	glUniform3f(iResolution, gbm->width, gbm->height, 0);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) (intptr_t) 0);
	glEnableVertexAttribArray(0);

	egl->draw = draw_shadertoy;

	return 0;
}


// perfcntrs.c
/*
 * Copyright © 2020 Google, Inc.
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

#define _GNU_SOURCE

#include <assert.h>
#include <err.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLES3/gl3.h>

// #include "common.h"

/* Module to collect a specified set of performance counts, and accumulate
 * results, using the GL_AMD_performance_monitor extension.
 *
 * Call start_perfcntrs() before the draw(s) to measure, and end_perfcntrs()
 * after the last draw to measure.  This can be done multiple times, with
 * the results accumulated.
 */

/**
 * Accumulated counter result:
 */
union counter_result {
	uint32_t u32;   /* GL_UNSIGNED_INT */
	float    f;     /* GL_FLOAT, GL_PERCENTAGE_AMD */
	uint64_t u64;   /* GL_UNSIGNED_INT64_AMD */
};

/**
 * Tracking for a requested counter
 */
struct counter {
	union counter_result result;
	/* index into perfcntrs.groups[gidx].counters[cidx]
	 * Note that the group_idx/counter_idx is not necessarily the
	 * same as the group_id/counter_id.
	 */
	unsigned gidx;
	unsigned cidx;
};

/**
 * Description of gl counter groups and counters:
 */

struct gl_counter {
	char *name;
	GLuint counter_id;
	GLuint counter_type;
	struct counter *counter;  /* NULL if this is not a counter we track */
};

struct gl_counter_group {
	char *name;
	GLuint group_id;
	GLint max_active_counters;
	GLint num_counters;
	struct gl_counter *counters;

	/* number of counters in this group which are enabled: */
	int num_enabled_counters;
};

struct gl_monitor {
	GLuint id;
	bool valid;
	bool active;
};

/**
 * module state
 */
static struct {
	const struct egl *egl;

	/* The extension doesn't let us pause/resume a single counter, so
	 * instead use a sequence of monitors, one per start_perfcntrs()/
	 * end_perfcntrs() pair, so that we don't need to immediately read
	 * back a result, which could cause a stall.
	 */
	struct gl_monitor monitors[4];
	unsigned current_monitor;

	/* The requested counters to monitor:
	 */
	unsigned num_counters;
	struct counter *counters;

	/* The description of all counter groups and the counters they
	 * contain, not just including the ones we monitor.
	 */
	GLint num_groups;
	struct gl_counter_group *groups;

} perfcntr;

static void get_groups_and_counters(const struct egl *egl)
{
	int n;

	egl->glGetPerfMonitorGroupsAMD(&perfcntr.num_groups, 0, NULL);
	perfcntr.groups = calloc(perfcntr.num_groups, sizeof(struct gl_counter_group));

	GLuint group_ids[perfcntr.num_groups];
	egl->glGetPerfMonitorGroupsAMD(NULL, perfcntr.num_groups, group_ids);

	for (int i = 0; i < perfcntr.num_groups; i++) {
		struct gl_counter_group *g = &perfcntr.groups[i];

		g->group_id = group_ids[i];

		egl->glGetPerfMonitorGroupStringAMD(g->group_id, 0, &n, NULL);
		g->name = malloc(n+1);
		egl->glGetPerfMonitorGroupStringAMD(g->group_id, n+1, NULL, g->name);

		egl->glGetPerfMonitorCountersAMD(g->group_id, &g->num_counters,
			&g->max_active_counters, 0, NULL);

		g->counters = calloc(g->num_counters, sizeof(struct gl_counter));

		GLuint counter_ids[g->num_counters];
		egl->glGetPerfMonitorCountersAMD(g->group_id, NULL, NULL,
			g->num_counters, counter_ids);

		printf("GROUP[%u]: name=%s, max_active_counters=%u, num_counters=%u\n",
			g->group_id, g->name, g->max_active_counters, g->num_counters);

		for (int j = 0; j < g->num_counters; j++) {
			struct gl_counter *c = &g->counters[j];

			c->counter_id = counter_ids[j];

			egl->glGetPerfMonitorCounterStringAMD(g->group_id,
				c->counter_id, 0, &n, NULL);
			c->name = malloc(n+1);
			egl->glGetPerfMonitorCounterStringAMD(g->group_id,
				c->counter_id, n+1, NULL, c->name);

			egl->glGetPerfMonitorCounterInfoAMD(g->group_id,
				c->counter_id, GL_COUNTER_TYPE_AMD,
				&c->counter_type);

			printf("\tCOUNTER[%u]: name=%s, counter_type=%04x\n",
				c->counter_id, c->name, c->counter_type);
		}
	}
}

static void find_counter(const char *name, unsigned *group_idx, unsigned *counter_idx)
{
	for (int i = 0; i < perfcntr.num_groups; i++) {
		struct gl_counter_group *g = &perfcntr.groups[i];

		for (int j = 0; j < g->num_counters; j++) {
			struct gl_counter *c = &g->counters[j];

			if (strcmp(name, c->name) == 0) {
				*group_idx = i;
				*counter_idx = j;
				return;
			}
		}
	}

	errx(-1, "Could not find counter: %s", name);
}

static void add_counter(const char *name)
{
	int idx = perfcntr.num_counters++;

	perfcntr.counters = realloc(perfcntr.counters,
		perfcntr.num_counters * sizeof(struct counter));

	struct counter *c = &perfcntr.counters[idx];
	memset(c, 0, sizeof(*c));

	find_counter(name, &c->gidx, &c->cidx);

	struct gl_counter_group *g = &perfcntr.groups[c->gidx];
	if (g->num_enabled_counters >= g->max_active_counters) {
		errx(-1, "Too many counters in group '%s'", g->name);
	}

	g->num_enabled_counters++;
}

/* parse list of performance counter names, and find their group+counter */
static void find_counters(const char *perfcntrs)
{
	char *cnames, *s;

	cnames = strdup(perfcntrs);
	while ((s = strstr(cnames, ","))) {
		char *name = cnames;
		s[0] = '\0';
		cnames = &s[1];

		add_counter(name);
	}

	add_counter(cnames);
}

void init_perfcntrs(const struct egl *egl, const char *perfcntrs)
{
	if (egl_check(egl, glGetPerfMonitorGroupsAMD) ||
	    egl_check(egl, glGetPerfMonitorCountersAMD) ||
	    egl_check(egl, glGetPerfMonitorGroupStringAMD) ||
	    egl_check(egl, glGetPerfMonitorCounterStringAMD) ||
	    egl_check(egl, glGetPerfMonitorCounterInfoAMD) ||
	    egl_check(egl, glGenPerfMonitorsAMD) ||
	    egl_check(egl, glDeletePerfMonitorsAMD) ||
	    egl_check(egl, glSelectPerfMonitorCountersAMD) ||
	    egl_check(egl, glBeginPerfMonitorAMD) ||
	    egl_check(egl, glEndPerfMonitorAMD) ||
	    egl_check(egl, glGetPerfMonitorCounterDataAMD)) {
		errx(-1, "AMD_performance_monitor is not supported");
	}

	get_groups_and_counters(egl);
	find_counters(perfcntrs);

	/* setup enabled counters.. do this after realloc() stuff,
	 * otherwise the counter pointer may not be valid:
	 */
	for (unsigned i = 0; i < perfcntr.num_counters; i++) {
		struct counter *c = &perfcntr.counters[i];
		perfcntr.groups[c->gidx].counters[c->cidx].counter = c;
	}

	perfcntr.egl = egl;
}

/* Create perf-monitor, and configure the counters it will monitor */
static void init_monitor(struct gl_monitor *m)
{
	const struct egl *egl = perfcntr.egl;

	assert(!m->valid);
	assert(!m->active);

	egl->glGenPerfMonitorsAMD(1, &m->id);

	for (int i = 0; i < perfcntr.num_groups; i++) {
		struct gl_counter_group *g = &perfcntr.groups[i];

		if (!g->num_enabled_counters)
			continue;

		int idx = 0;
		GLuint counters[g->num_enabled_counters];

		for (int j = 0; j < g->num_counters; j++) {
			struct gl_counter *c = &g->counters[j];

			if (!c->counter)
				continue;

			assert(idx < g->num_enabled_counters);
			counters[idx++] = c->counter_id;
		}

		assert(idx == g->num_enabled_counters);
		egl->glSelectPerfMonitorCountersAMD(m->id, GL_TRUE,
			g->group_id, g->num_enabled_counters, counters);
	}

	m->valid = true;
}

static struct gl_counter *lookup_counter(GLuint group_id, GLuint counter_id)
{
	for (int i = 0; i < perfcntr.num_groups; i++) {
		struct gl_counter_group *g = &perfcntr.groups[i];

		if (g->group_id != group_id)
			continue;

		for (int j = 0; j < g->num_counters; j++) {
			struct gl_counter *c = &g->counters[j];

			if (c->counter_id != counter_id)
				continue;

			return c;
		}
	}

	errx(-1, "invalid counter: group_id=%u, counter_id=%u",
		group_id, counter_id);
}

/* Collect monitor results and delete monitor */
static void finish_monitor(struct gl_monitor *m)
{
	const struct egl *egl = perfcntr.egl;

	assert(m->valid);
	assert(!m->active);

	GLuint result_size;
	egl->glGetPerfMonitorCounterDataAMD(m->id, GL_PERFMON_RESULT_SIZE_AMD,
		sizeof(GLint), &result_size, NULL);

	GLuint *data = malloc(result_size);

	GLsizei bytes_written;
	egl->glGetPerfMonitorCounterDataAMD(m->id, GL_PERFMON_RESULT_AMD,
			result_size, data, &bytes_written);

	GLsizei idx = 0;
	while ((4 * idx) < bytes_written) {
		GLuint group_id = data[idx++];
		GLuint counter_id = data[idx++];

		struct gl_counter *c = lookup_counter(group_id, counter_id);

		assert(c->counter);

		switch(c->counter_type) {
		case GL_UNSIGNED_INT:
			c->counter->result.u32 += *(uint32_t *)(&data[idx]);
			idx += 1;
			break;
		case GL_FLOAT:
			c->counter->result.f += *(float *)(&data[idx]);
			idx += 1;
			break;
		case GL_UNSIGNED_INT64_AMD:
			c->counter->result.u64 += *(uint64_t *)(&data[idx]);
			idx += 2;
			break;
		case GL_PERCENTAGE_AMD:
		default:
			errx(-1, "TODO unhandled counter type: 0x%04x",
				c->counter_type);
			break;
		}
	}

	egl->glDeletePerfMonitorsAMD(1, &m->id);
	m->valid = false;
}

void start_perfcntrs(void)
{
	const struct egl *egl = perfcntr.egl;

	if (!egl) {
		return;
	}

	struct gl_monitor *m = &perfcntr.monitors[perfcntr.current_monitor];

	/* once we wrap-around and start re-using existing slots, collect
	 * previous results and delete the monitor before re-using the slot:
	 */
	if (m->valid) {
		finish_monitor(m);
	}

	init_monitor(m);

	egl->glBeginPerfMonitorAMD(m->id);
	m->active = true;
}

void end_perfcntrs(void)
{
	const struct egl *egl = perfcntr.egl;

	if (!egl) {
		return;
	}

	struct gl_monitor *m = &perfcntr.monitors[perfcntr.current_monitor];

	assert(m->valid);
	assert(m->active);

	/* end collection, but defer collecting results to avoid stall: */
	egl->glEndPerfMonitorAMD(m->id);
	m->active = false;

	/* move to next slot: */
	perfcntr.current_monitor =
		(perfcntr.current_monitor + 1) % ARRAY_SIZE(perfcntr.monitors);
}

/* collect any remaining perfcntr results.. this should be called
 * before computing the elapsed time (passed to dump_perfcntrs())
 * to ensured queued up draws which are monitored complete, ie.
 * so that elapsed time covers the entirety of the monitored
 * draws.
 */
void finish_perfcntrs(void)
{
	if (!perfcntr.egl)
		return;

	/* collect any remaining results, it really doesn't matter the order */
	for (unsigned i = 0; i < ARRAY_SIZE(perfcntr.monitors); i++) {
		struct gl_monitor *m = &perfcntr.monitors[i];
		if (m->valid) {
			finish_monitor(m);
		}
	}
}

void dump_perfcntrs(unsigned nframes, uint64_t elapsed_time_ns)
{
	if (!perfcntr.egl) {
		return;
	}

	/* print column headers: */
	printf("FPS");
	for (unsigned i = 0; i < perfcntr.num_counters; i++) {
		struct counter *c = &perfcntr.counters[i];

		printf(",%s", perfcntr.groups[c->gidx].counters[c->cidx].name);
	}
	printf("\n");

	/* print results: */
	double secs = elapsed_time_ns / (double)NSEC_PER_SEC;
	printf("%f", (double)nframes/secs);
	for (unsigned i = 0; i < perfcntr.num_counters; i++) {
		struct counter *c = &perfcntr.counters[i];

		GLuint counter_type =
			perfcntr.groups[c->gidx].counters[c->cidx].counter_type;
		switch (counter_type) {
		case GL_UNSIGNED_INT:
			printf(",%u", c->result.u32);
			break;
		case GL_FLOAT:
			printf(",%f", c->result.f);
			break;
		case GL_UNSIGNED_INT64_AMD:
			printf(",%"PRIu64, c->result.u64);
			break;
		case GL_PERCENTAGE_AMD:
		default:
			errx(-1, "TODO unhandled counter type: 0x%04x",
				counter_type);
			break;
		}
	}
	printf("\n");
}


// drm-common.c
/*
 * Copyright (c) 2017 Rob Clark <rclark@redhat.com>
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

#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// #include "common.h"
// #include "drm-common.h"

WEAK union gbm_bo_handle
gbm_bo_get_handle_for_plane(struct gbm_bo *bo, int plane);

WEAK uint64_t
gbm_bo_get_modifier(struct gbm_bo *bo);

WEAK int
gbm_bo_get_plane_count(struct gbm_bo *bo);

WEAK uint32_t
gbm_bo_get_stride_for_plane(struct gbm_bo *bo, int plane);

WEAK uint32_t
gbm_bo_get_offset(struct gbm_bo *bo, int plane);

static void
drm_fb_destroy_callback(struct gbm_bo *bo, void *data)
{
	int drm_fd = gbm_device_get_fd(gbm_bo_get_device(bo));
	struct drm_fb *fb = data;

	if (fb->fb_id)
		drmModeRmFB(drm_fd, fb->fb_id);

	free(fb);
}

struct drm_fb * drm_fb_get_from_bo(struct gbm_bo *bo)
{
	int drm_fd = gbm_device_get_fd(gbm_bo_get_device(bo));
	struct drm_fb *fb = gbm_bo_get_user_data(bo);
	uint32_t width, height, format,
		 strides[4] = {0}, handles[4] = {0},
		 offsets[4] = {0}, flags = 0;
	int ret = -1;

	if (fb)
		return fb;

	fb = calloc(1, sizeof *fb);
	fb->bo = bo;

	width = gbm_bo_get_width(bo);
	height = gbm_bo_get_height(bo);
	format = gbm_bo_get_format(bo);

	if (gbm_bo_get_handle_for_plane && gbm_bo_get_modifier &&
	    gbm_bo_get_plane_count && gbm_bo_get_stride_for_plane &&
	    gbm_bo_get_offset) {

		uint64_t modifiers[4] = {0};
		modifiers[0] = gbm_bo_get_modifier(bo);
		const int num_planes = gbm_bo_get_plane_count(bo);
		for (int i = 0; i < num_planes; i++) {
			handles[i] = gbm_bo_get_handle_for_plane(bo, i).u32;
			strides[i] = gbm_bo_get_stride_for_plane(bo, i);
			offsets[i] = gbm_bo_get_offset(bo, i);
			modifiers[i] = modifiers[0];
		}

		if (modifiers[0]) {
			flags = DRM_MODE_FB_MODIFIERS;
			printf("Using modifier %" PRIx64 "\n", modifiers[0]);
		}

		ret = drmModeAddFB2WithModifiers(drm_fd, width, height,
				format, handles, strides, offsets,
				modifiers, &fb->fb_id, flags);
	}

	if (ret) {
		if (flags)
			fprintf(stderr, "Modifiers failed!\n");

		memcpy(handles, (uint32_t [4]){gbm_bo_get_handle(bo).u32,0,0,0}, 16);
		memcpy(strides, (uint32_t [4]){gbm_bo_get_stride(bo),0,0,0}, 16);
		memset(offsets, 0, 16);
		ret = drmModeAddFB2(drm_fd, width, height, format,
				handles, strides, offsets, &fb->fb_id, 0);
	}

	if (ret) {
		printf("failed to create fb: %s\n", strerror(errno));
		free(fb);
		return NULL;
	}

	gbm_bo_set_user_data(bo, fb, drm_fb_destroy_callback);

	return fb;
}

static uint32_t find_crtc_for_encoder(const drmModeRes *resources,
		const drmModeEncoder *encoder) {
	int i;

	for (i = 0; i < resources->count_crtcs; i++) {
		/* possible_crtcs is a bitmask as described here:
		 * https://dvdhrm.wordpress.com/2012/09/13/linux-drm-mode-setting-api
		 */
		const uint32_t crtc_mask = 1 << i;
		const uint32_t crtc_id = resources->crtcs[i];
		if (encoder->possible_crtcs & crtc_mask) {
			return crtc_id;
		}
	}

	/* no match found */
	return -1;
}

static uint32_t find_crtc_for_connector(const struct drm *drm, const drmModeRes *resources,
		const drmModeConnector *connector) {
	int i;

	for (i = 0; i < connector->count_encoders; i++) {
		const uint32_t encoder_id = connector->encoders[i];
		drmModeEncoder *encoder = drmModeGetEncoder(drm->fd, encoder_id);

		if (encoder) {
			const uint32_t crtc_id = find_crtc_for_encoder(resources, encoder);

			drmModeFreeEncoder(encoder);
			if (crtc_id != 0) {
				return crtc_id;
			}
		}
	}

	/* no match found */
	return -1;
}

static int get_resources(int fd, drmModeRes **resources)
{
	*resources = drmModeGetResources(fd);
	if (*resources == NULL)
		return -1;
	return 0;
}

#define MAX_DRM_DEVICES 64

static int find_drm_device(drmModeRes **resources)
{
	drmDevicePtr devices[MAX_DRM_DEVICES] = { NULL };
	int num_devices, fd = -1;

	num_devices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);
	if (num_devices < 0) {
		printf("drmGetDevices2 failed: %s\n", strerror(-num_devices));
		return -1;
	}

	for (int i = 0; i < num_devices; i++) {
		drmDevicePtr device = devices[i];
		int ret;

		if (!(device->available_nodes & (1 << DRM_NODE_PRIMARY)))
			continue;
		/* OK, it's a primary device. If we can get the
		 * drmModeResources, it means it's also a
		 * KMS-capable device.
		 */
		fd = open(device->nodes[DRM_NODE_PRIMARY], O_RDWR);
		if (fd < 0)
			continue;
		ret = get_resources(fd, resources);
		if (!ret)
			break;
		close(fd);
		fd = -1;
	}
	drmFreeDevices(devices, num_devices);

	if (fd < 0)
		printf("no drm device found!\n");
	return fd;
}

int init_drm(struct drm *drm, const char *device, const char *mode_str,
		unsigned int vrefresh, unsigned int count)
{
	drmModeRes *resources;
	drmModeConnector *connector = NULL;
	drmModeEncoder *encoder = NULL;
	int i, ret, area;

	if (device) {
		drm->fd = open(device, O_RDWR);
		ret = get_resources(drm->fd, &resources);
		if (ret < 0 && errno == EOPNOTSUPP)
			printf("%s does not look like a modeset device\n", device);
	} else {
		drm->fd = find_drm_device(&resources);
	}

	if (drm->fd < 0) {
		printf("could not open drm device\n");
		return -1;
	}

	if (!resources) {
		printf("drmModeGetResources failed: %s\n", strerror(errno));
		return -1;
	}

	/* find a connected connector: */
	for (i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(drm->fd, resources->connectors[i]);
		if (connector->connection == DRM_MODE_CONNECTED) {
			/* it's connected, let's use this! */
			break;
		}
		drmModeFreeConnector(connector);
		connector = NULL;
	}

	if (!connector) {
		/* we could be fancy and listen for hotplug events and wait for
		 * a connector..
		 */
		printf("no connected connector!\n");
		return -1;
	}

	/* find user requested mode: */
	if (mode_str && *mode_str) {
		for (i = 0; i < connector->count_modes; i++) {
			drmModeModeInfo *current_mode = &connector->modes[i];

			if (strcmp(current_mode->name, mode_str) == 0) {
				if (vrefresh == 0 || current_mode->vrefresh == vrefresh) {
					drm->mode = current_mode;
					break;
				}
			}
		}
		if (!drm->mode)
			printf("requested mode not found, using default mode!\n");
	}

	/* find preferred mode or the highest resolution mode: */
	if (!drm->mode) {
		for (i = 0, area = 0; i < connector->count_modes; i++) {
			drmModeModeInfo *current_mode = &connector->modes[i];

			if (current_mode->type & DRM_MODE_TYPE_PREFERRED) {
				drm->mode = current_mode;
				break;
			}

			int current_area = current_mode->hdisplay * current_mode->vdisplay;
			if (current_area > area) {
				drm->mode = current_mode;
				area = current_area;
			}
		}
	}

	if (!drm->mode) {
		printf("could not find mode!\n");
		return -1;
	}

	/* find encoder: */
	for (i = 0; i < resources->count_encoders; i++) {
		encoder = drmModeGetEncoder(drm->fd, resources->encoders[i]);
		if (encoder->encoder_id == connector->encoder_id)
			break;
		drmModeFreeEncoder(encoder);
		encoder = NULL;
	}

	if (encoder) {
		drm->crtc_id = encoder->crtc_id;
	} else {
		uint32_t crtc_id = find_crtc_for_connector(drm, resources, connector);
		if (crtc_id == 0) {
			printf("no crtc found!\n");
			return -1;
		}

		drm->crtc_id = crtc_id;
	}

	for (i = 0; i < resources->count_crtcs; i++) {
		if (resources->crtcs[i] == drm->crtc_id) {
			drm->crtc_index = i;
			break;
		}
	}

	drmModeFreeResources(resources);

	drm->connector_id = connector->connector_id;
	drm->count = count;

	// backup original crtc so we can restore the original video mode on exit.
	drm->orig_crtc = drmModeGetCrtc(drm->fd, encoder->crtc_id);

	return 0;
}

void finish_drm()
{
	// Restore the original videomode/connector/scanoutbuffer combination (the original CRTC, that is). 
	drmModeSetCrtc(kg_drm.fd, kg_drm.orig_crtc->crtc_id, kg_drm.orig_crtc->buffer_id,
		kg_drm.orig_crtc->x, kg_drm.orig_crtc->y, &kg_drm.connector_id, 1, &kg_drm.orig_crtc->mode);

	/*if (fb->fb_id) {
		drmModeRmFB(drm.fd, fb->fb_id);
	}*/

	close(kg_drm.fd);
}
