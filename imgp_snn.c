// Symmetric Nearest Neighbor
// clang -Os -o imgp_snn imgp_snn.c -lm
// ./imgp_snn 01.jpg

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#include "imgp.h"

int main(int argc, char* argv[])
{
	int w, h, bpp;
	uint8_t *pixels = stbi_load(argv[1], &w, &h, &bpp, 3);
	assert(pixels);

	uint8_t *snn = malloc(w*h*4);
	imgp_filter_snn(pixels, w, h, snn, 2);
	stbi_write_jpg("imgp_snn.jpg", w, h, 3, snn, 0);
	free(snn);
	stbi_image_free(pixels);
}
