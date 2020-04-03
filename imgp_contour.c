// clang -Os -o imgp imgp.c -lm

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "imgp.h"

int main(int argc, char* argv[])
{
	char *name = argv[1];

	uint8_t *pixels;
	int w, h, bpp;
	pixels = stbi_load(name, &w, &h, &bpp, 3);
	assert(pixels);

	uint8_t *gray = malloc(w*h*4);
	uint8_t *dilated = gray+w*h;
	uint8_t *diff = gray+w*h*2;
	uint8_t *contour = gray+w*h*3;
	imgp_gray(pixels, w, h, w, gray, w);
	imgp_dilate(gray, w, h, /*5,*/ dilated);
	stbi_write_jpg("dilated.jpg", w, h, 1, dilated, 0);
	imgp_absdiff(gray, dilated, w, h, diff);
	stbi_write_jpg("diff.jpg", w, h, 1, diff, 0);
	imgp_reverse(diff, w, h, contour);
	stbi_write_jpg("contour.jpg", w, h, 1, contour, 0);
	free(gray);
	stbi_image_free(pixels);
}
