// berry-dm
// Copyright © 2015,2022 Yuichiro Nakada

// clang -Os -I .. -I ../3rd/ aviewer.c -o aviewer -lm
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/ioctl.h>
#include <time.h>

//#include "termbox.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//#define JEBP_IMPLEMENTATION
//#include "jebp.h"
#include "aimage.h"

#if 0
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define ICEIL(dividend, divisor) \
	(((dividend) + ((divisor) - 1)) / (divisor))

void aviewer(char *name, int sx, int sy)
{
	unsigned char *pixels;
	int width, height, bpp, frames;
//	pixels = stbi_load(name, &width, &height, &bpp, 4/*RGBA*/);
	pixels = stbi_xload(name, &width, &height, &frames);
	int rx = MAX(ICEIL(width, sx), 1);
	int ry = MAX(ICEIL(height, sy), 1);
	printf("%s %dx%d r[%d,%d] / Screen %d,%d\n", name, width, height, rx, ry, sx, sy);
//	putImage(pixels, width, height, rx, ry, sx, sy);
	printf("frames:%d\n", frames);
	char screen[frames][sx*sy*14+11*sy+1];
	for (int i=0; i<frames; i++) {
		simage(screen[i], &pixels[width*height*4*i+2*i], width, height, rx, ry, sx, sy);
	}
	stbi_image_free(pixels);

	for (int i=0; i<frames; i++) {
//		ECLEAR();
		ELOCATE(1, 1);
		printf("%s", screen[i]);

		struct timespec req;
		req.tv_sec  = 0;
		req.tv_nsec = 50000000;
		nanosleep(&req, NULL);
		//usleep(10000);
	}
}
#endif

void usage()
{
	printf("Usage: `aviewer <input> [width height]`\n");
}

int main(int argc, char *argv[])
{
#ifdef H_TERMBOX
	tb_init();
//	tb_select_output_mode(TB_OUTPUT_TRUECOLOR);
	tb_select_output_mode(TB_OUTPUT_256);
	tb_clear();

	int width, height;
	width = tb_width();
	height = tb_height();
#else
	// get terminal size
	int width, height;
	struct winsize ws;
	if (ioctl(0, TIOCGWINSZ, &ws) != -1) {
		if (0 < ws.ws_col && ws.ws_col == (size_t)ws.ws_col) {
			width = ws.ws_col;
			height = ws.ws_row;
		} 
	}
#endif

	if (argc < 2 || 4 < argc) {
		usage();
	} else if (argc == 2) {
		aviewer(argv[1], width, height);
	} else {
		aviewer(argv[1], atoi(argv[2]), atoi(argv[3]));
	}

#ifdef H_TERMBOX
	tb_shutdown();
#endif
	return 0;
}
