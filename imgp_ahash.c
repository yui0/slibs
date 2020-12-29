// Average Hash
// clang -Os -o imgp_ahash imgp_ahash.c -lm
// ./imgp_ahash 01.jpg 02.jpg

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#include "imgp.h"

int main(int argc, char* argv[])
{
	uint8_t *ahash = imgp_get_ahash(argv[1]);
	for (int i=0; i<AHASH_SIZE*AHASH_SIZE/8; i++) {
		printf("%02x", ahash[i]);
	}
	printf("\n");

	if (argc>2) {
		uint8_t *ahash2 = imgp_get_ahash(argv[2]);
		for (int i=0; i<AHASH_SIZE*AHASH_SIZE/8; i++) {
			printf("%02x", ahash2[i]);
		}
		printf("\n");

		int d = imgp_get_distance(ahash, ahash2);
		printf("distance: %d\n", d);
		free(ahash2);
	}

	free(ahash);
}
