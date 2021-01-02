// Symmetric Nearest Neighbor
//  ©2021 Yuichiro Nakada
// clang -Os -o imgp_snn imgp_snn.c -lm
// ./imgp_snn -r 2 01.jpg -o 01_illust.jpg

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
	char *outfile = "imgp_snn.jpg";
	int round = 2;

	/*if (argc <=1) {
		usage(stderr, argv);
		return 0;
	}*/
	for (int i=1; i<argc; i++) {
		if (!strcmp(argv[i], "-o")) {
			outfile = argv[++i];
		} else if (!strcmp(argv[i], "-r")) {
			round = atoi(argv[++i]);
		} else {
			name = argv[i];
			//printf("%s\n", name);
		}
	}

	int w, h, bpp;
	uint8_t *pixels = stbi_load(name, &w, &h, &bpp, 3);
	assert(pixels);

	uint8_t *snn = malloc(w*h*4);
	imgp_filter_snn(pixels, w, h, snn, round);
	stbi_write_jpg(outfile, w, h, 3, snn, 0);
	free(snn);
	stbi_image_free(pixels);
}
