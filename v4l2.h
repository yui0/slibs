/* public domain Simple, Minimalistic, Video4Linux library
 *	©2017 Yuichiro Nakada
 *
 * Latest revisions:
 * 	1.00 (21-02-2017) initial release
 *
 * Basic usage:
 *	VIDEO_init(&video, "/dev/video0", 640, 480);
 *	VIDEO_captureStart(&video);
 *	...
 *	VIDEO_frameRead(&video); // video data in video.rgb, video.width, video.height
 *	...
 *	VIDEO_captureStop(&video);
 *	VIDEO_close(&video);
 *
 * Notes:
 *
 * */

// compile with all three access methods
#if !defined(IO_READ) && !defined(IO_MMAP) && !defined(IO_USERPTR)
#define IO_READ
#define IO_MMAP
#define IO_USERPTR
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>

#define CLEAR(x)	memset(&(x), 0, sizeof(x))

typedef enum {
#ifdef IO_READ
	IO_METHOD_READ,
#endif
#ifdef IO_MMAP
	IO_METHOD_MMAP,
#endif
#ifdef IO_USERPTR
	IO_METHOD_USERPTR,
#endif
} io_method;

struct buffer {
	void *start;
	size_t length;
};

typedef struct {
	int fd;
	struct buffer *buffers;
	unsigned int n_buffers;
	io_method io;

	char *deviceName;
	unsigned int width;
	unsigned int height;
	unsigned char *rgb;
} VIDEO;

// Convert from YUV422 format to RGB888. Formulae are described on http://en.wikipedia.org/wiki/YUV
static void YUV422toRGB888(int width, int height, unsigned char *src, unsigned char *dst)
{
	int line, column;
	unsigned char *py, *pu, *pv;
	unsigned char *tmp = dst;

	/* In thiz format each four bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr.
	   Each Y goes to one of the pixels, and the Cb and Cr belong to both pixels. */
	py = src;
	pu = src + 1;
	pv = src + 3;

#define CLIP(x) ( (x)>=0xFF ? 0xFF : ( (x) <= 0x00 ? 0x00 : (x) ) )

	for (line=0; line < height; ++line) {
		for (column=0; column < width; ++column) {
			*tmp++ = CLIP((double)*py + 1.402*((double)*pv-128.0));
			*tmp++ = CLIP((double)*py - 0.344*((double)*pu-128.0) - 0.714*((double)*pv-128.0));
			*tmp++ = CLIP((double)*py + 1.772*((double)*pu-128.0));

			// increase py every time
			py += 2;
			// increase pu,pv every second time
			if ((column & 1)==1) {
				pu += 4;
				pv += 4;
			}
		}
	}
}

static void errno_exit(const char* s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

static int xioctl(int fd, int request, void* argp)
{
	int r;
	do {
		r = ioctl(fd, request, argp);
	} while (-1 == r && EINTR == errno);
	return r;
}

static void imageProcess(VIDEO *thiz, const void* p)
{
	// convert from YUV422 to RGB888
	YUV422toRGB888(thiz->width, thiz->height, (unsigned char*)p, thiz->rgb);
}

// read single frame
static int VIDEO_frameRead(VIDEO *thiz)
{
	struct v4l2_buffer buf;
#ifdef IO_USERPTR
	unsigned int i;
#endif

	switch (thiz->io) {
#ifdef IO_READ
	case IO_METHOD_READ:
		if (-1 == read(thiz->fd, thiz->buffers[0].start, thiz->buffers[0].length)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
			// Could ignore EIO, see spec.

			// fall through
			default:
				errno_exit("read");
			}
		}
		imageProcess(thiz, thiz->buffers[0].start);
		break;
#endif
#ifdef IO_MMAP
	case IO_METHOD_MMAP:
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl(thiz->fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
			// Could ignore EIO, see spec

			// fall through
			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}
		assert(buf.index < thiz->n_buffers);

		imageProcess(thiz, thiz->buffers[buf.index].start);
		if (-1 == xioctl(thiz->fd, VIDIOC_QBUF, &buf)) {
			errno_exit("VIDIOC_QBUF");
		}
		break;
#endif
#ifdef IO_USERPTR
	case IO_METHOD_USERPTR:
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl(thiz->fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
			// Could ignore EIO, see spec.

			// fall through
			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}

		for (i=0; i < thiz->n_buffers; ++i) {
			if (buf.m.userptr == (unsigned long) thiz->buffers[i].start && buf.length == thiz->buffers[i].length) {
				break;
			}
		}
		assert(i < thiz->n_buffers);

		imageProcess(thiz, (void*)buf.m.userptr);
		if (-1 == xioctl(thiz->fd, VIDIOC_QBUF, &buf)) {
			errno_exit("VIDIOC_QBUF");
		}
		break;
#endif
	}

	return 1;
}

static void VIDEO_captureStop(VIDEO *thiz)
{
	enum v4l2_buf_type type;

	switch (thiz->io) {
#ifdef IO_READ
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;
#endif
#ifdef IO_MMAP
	case IO_METHOD_MMAP:
#endif
#ifdef IO_USERPTR
	case IO_METHOD_USERPTR:
#endif
#if defined(IO_MMAP) || defined(IO_USERPTR)
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(thiz->fd, VIDIOC_STREAMOFF, &type)) {
			errno_exit("VIDIOC_STREAMOFF");
		}
		break;
#endif
	}
}

static void VIDEO_captureStart(VIDEO *thiz)
{
	unsigned int i;
	enum v4l2_buf_type type;

	switch (thiz->io) {
#ifdef IO_READ
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;
#endif
#ifdef IO_MMAP
	case IO_METHOD_MMAP:
		for (i=0; i < thiz->n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory      = V4L2_MEMORY_MMAP;
			buf.index       = i;

			if (-1 == xioctl(thiz->fd, VIDIOC_QBUF, &buf)) {
				errno_exit("VIDIOC_QBUF");
			}
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(thiz->fd, VIDIOC_STREAMON, &type)) {
			errno_exit("VIDIOC_STREAMON");
		}
		break;
#endif
#ifdef IO_USERPTR
	case IO_METHOD_USERPTR:
		for (i=0; i < thiz->n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory      = V4L2_MEMORY_USERPTR;
			buf.index       = i;
			buf.m.userptr   = (unsigned long) thiz->buffers[i].start;
			buf.length      = thiz->buffers[i].length;

			if (-1 == xioctl(thiz->fd, VIDIOC_QBUF, &buf)) {
				errno_exit("VIDIOC_QBUF");
			}
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(thiz->fd, VIDIOC_STREAMON, &type)) {
			errno_exit("VIDIOC_STREAMON");
		}
		break;
#endif
	}
}

static void deviceUninit(VIDEO *thiz)
{
	unsigned int i;

	switch (thiz->io) {
#ifdef IO_READ
	case IO_METHOD_READ:
		free(thiz->buffers[0].start);
		break;
#endif
#ifdef IO_MMAP
	case IO_METHOD_MMAP:
		for (i=0; i < thiz->n_buffers; ++i)
			if (-1 == munmap(thiz->buffers[i].start, thiz->buffers[i].length)) {
				errno_exit("munmap");
			}
		break;
#endif
#ifdef IO_USERPTR
	case IO_METHOD_USERPTR:
		for (i=0; i < thiz->n_buffers; ++i) {
			free(thiz->buffers[i].start);
		}
		break;
#endif
	}

	free(thiz->buffers);
}

#ifdef IO_READ
static void readInit(VIDEO *thiz, unsigned int buffer_size)
{
	thiz->buffers = (struct buffer*)calloc(1, sizeof(struct buffer));
	if (!thiz->buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	thiz->buffers[0].length = buffer_size;
	thiz->buffers[0].start = malloc(buffer_size);
	if (!thiz->buffers[0].start) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
}
#endif

#ifdef IO_MMAP
static void mmapInit(VIDEO *thiz)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);
	req.count	= 4;
	req.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory	= V4L2_MEMORY_MMAP;

	if (-1 == xioctl(thiz->fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support memory mapping\n", thiz->deviceName);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\n", thiz->deviceName);
		exit(EXIT_FAILURE);
	}

	thiz->buffers = (struct buffer*)calloc(req.count, sizeof(struct buffer));
	if (!thiz->buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (thiz->n_buffers = 0; thiz->n_buffers < req.count; ++thiz->n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = thiz->n_buffers;

		if (-1 == xioctl(thiz->fd, VIDIOC_QUERYBUF, &buf)) {
			errno_exit("VIDIOC_QUERYBUF");
		}

		thiz->buffers[thiz->n_buffers].length = buf.length;
		thiz->buffers[thiz->n_buffers].start =
		        mmap(NULL /* start anywhere */, buf.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, thiz->fd, buf.m.offset);

		if (MAP_FAILED == thiz->buffers[thiz->n_buffers].start) {
			errno_exit("mmap");
		}
	}
}
#endif

#ifdef IO_USERPTR
static void userptrInit(VIDEO *thiz, unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;
	unsigned int page_size;

	page_size = getpagesize();
	buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

	CLEAR(req);
	req.count	= 4;
	req.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory	= V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(thiz->fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support user pointer i/o\n", thiz->deviceName);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	thiz->buffers = (struct buffer*)calloc(4, sizeof(struct buffer));
	if (!thiz->buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (thiz->n_buffers = 0; thiz->n_buffers < 4; ++thiz->n_buffers) {
		thiz->buffers[thiz->n_buffers].length = buffer_size;
		thiz->buffers[thiz->n_buffers].start = memalign(/* boundary */ page_size, buffer_size);

		if (!thiz->buffers[thiz->n_buffers].start) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
}
#endif

static void deviceInit(VIDEO *thiz)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl(thiz->fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n", thiz->deviceName);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n", thiz->deviceName);
		exit(EXIT_FAILURE);
	}

	switch (thiz->io) {
#ifdef IO_READ
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf(stderr, "%s does not support read i/o\n", thiz->deviceName);
			exit(EXIT_FAILURE);
		}
		break;
#endif
#ifdef IO_MMAP
	case IO_METHOD_MMAP:
#endif
#ifdef IO_USERPTR
	case IO_METHOD_USERPTR:
#endif
#if defined(IO_MMAP) || defined(IO_USERPTR)
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf(stderr, "%s does not support streaming i/o\n", thiz->deviceName);
			exit(EXIT_FAILURE);
		}
		break;
#endif
	}

	/* Select video input, video standard and tune here. */
	CLEAR(cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(thiz->fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(thiz->fd, VIDIOC_S_CROP, &crop)) {
			/*switch (errno) {
			case EINVAL:
				// Cropping not supported.
				break;
			default:
				// Errors ignored.
			}*/
		}
	/*} else {
		// Errors ignored.*/
	}

	// v4l2_format
	CLEAR(fmt);
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = thiz->width;
	fmt.fmt.pix.height      = thiz->height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	if (-1 == xioctl(thiz->fd, VIDIOC_S_FMT, &fmt)) {
		errno_exit("VIDIOC_S_FMT");
	}

	/* Note VIDIOC_S_FMT may change width and height. */
	if (thiz->width != fmt.fmt.pix.width) {
		thiz->width = fmt.fmt.pix.width;
		fprintf(stderr, "Image width set to %i by device %s.\n", thiz->width, thiz->deviceName);
	}
	if (thiz->height != fmt.fmt.pix.height) {
		thiz->height = fmt.fmt.pix.height;
		fprintf(stderr, "Image height set to %i by device %s.\n", thiz->height, thiz->deviceName);
	}

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min) {
		fmt.fmt.pix.bytesperline = min;
	}
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min) {
		fmt.fmt.pix.sizeimage = min;
	}

	switch (thiz->io) {
#ifdef IO_READ
	case IO_METHOD_READ:
		readInit(thiz, fmt.fmt.pix.sizeimage);
		break;
#endif
#ifdef IO_MMAP
	case IO_METHOD_MMAP:
		mmapInit(thiz);
		break;
#endif
#ifdef IO_USERPTR
	case IO_METHOD_USERPTR:
		userptrInit(thiz, fmt.fmt.pix.sizeimage);
#endif
	}
}

static void VIDEO_deviceOpen(VIDEO *thiz)
{
	struct stat st;

	// stat file
	if (-1 == stat(thiz->deviceName, &st)) {
		fprintf(stderr, "Can't identify '%s': %d, %s\n", thiz->deviceName, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// check if its device
	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is no device\n", thiz->deviceName);
		exit(EXIT_FAILURE);
	}

	// open device
	thiz->fd = open(thiz->deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);
	if (-1 == thiz->fd) {
		fprintf(stderr, "Can't open '%s': %d, %s\n", thiz->deviceName, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void VIDEO_deviceClose(VIDEO *thiz)
{
	if (-1 == close(thiz->fd)) {
		errno_exit("close");
	}
	thiz->fd = -1;
}

void VIDEO_init(VIDEO *thiz, char *dev, int w, int h)
{
	thiz->fd = -1;
	thiz->buffers = 0;
	thiz->n_buffers = 0;
	thiz->io = IO_METHOD_MMAP;
	thiz->deviceName = dev;
	thiz->width = w;
	thiz->height = h;
	thiz->rgb = 0;

	VIDEO_deviceOpen(thiz);
	deviceInit(thiz);
	thiz->rgb = (unsigned char*)malloc(thiz->width * thiz->height *3);
}

void VIDEO_close(VIDEO *thiz)
{
	free(thiz->rgb);
	deviceUninit(thiz);
	VIDEO_deviceClose(thiz);
}
