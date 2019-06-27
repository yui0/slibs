// Average Hash
// clang -Os -o ahash ahash.c -lm

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#include "imgp.h"

int main(int argc, char* argv[])
{
	char *name = argv[1];

	uint8_t *pixels;
	int w, h, bpp;
	pixels = stbi_load(name, &w, &h, &bpp, 3);
	assert(pixels);

	uint8_t *gray = malloc(w*h+AHASH_SIZE*AHASH_SIZE);
	imgp_gray(pixels, w, h, w, gray, w);
	//stbi_write_jpg("gray.jpg", w, h, 1, gray, 0);

	uint8_t ahash[AHASH_SIZE*AHASH_SIZE/8];
	imgp_ahash(gray, w, h, ahash);

	for (int i=0; i<AHASH_SIZE*AHASH_SIZE/8; i++) {
		printf("%02x", ahash[i]);
	}
	printf("\n");

	free(gray);
	stbi_image_free(pixels);
}
