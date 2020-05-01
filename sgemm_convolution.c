// ©2020 Yuichiro Nakada
// clang convolution.c -o convolution -lm -lOpenCL

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
//#define STB_IMAGE_RESIZE_IMPLEMENTATION
//#include "stb_image_resize.h"

//#include "sgemm_ocl.h"
#include "sgemm_ocl1.h"

#define real	float

//	gemm('N', 'N', 1, w*h, 4*4, magic_kernel, workspace, pix);
void gemm(char ta, char tb, int M, int N, int K, float *a, float *b, float *c)
{
/*	int lda = M*K;
	int ldb = K*N;
	int ldc = M*N;*/
	for (int m=0; m<M; m++) {
		for (int n=0; n<N; n++) {
			float sum = 0.0;
/*			for (int k=0; k<K; k++) {
				sum += a[m + k * lda] * b[k + n * ldb];
			}
//			Z[m + n * ldc] = alpha * sum + beta * Z[m + n * ldc];
			c[m + n * ldc] = sum;*/
/*			// CTN
			for (int k=0; k<K; k++) {
				sum += a[k + m * lda] * b[k + n * ldb];
			}
			//Z[m + n * ldc] = alpha * sum + beta * Z[m + n * ldc];
			c[m + n * ldc] = sum;*/
/*			// CTT
			for (int k=0; k<K; k++) {
				sum += a[k + m * lda] * b[n + k * ldb];
			}
			//Z[m + n * ldc] = alpha * sum + beta * Z[m + n * ldc];
			c[m + n * ldc] = sum;*/

			for (int k=0; k<K; k++) {
//				sum += a[k*M + m] * b[n*K + k];
				sum += a[k*M + m] * b[n + N*k]; // NT
			}
			//Z[m + n * ldc] = alpha * sum + beta * Z[m + n * ldc];
			c[n*M + m] = sum;
		}
	}
}

static inline void im2col(const real *im, const int channels,
	const int height, const int width, const int kernel_h, const int kernel_w,
	const int pad_h, const int pad_w, const int stride_h, const int stride_w, real *col)
{
	int height_col = (height + 2 * pad_h - kernel_h) / stride_h + 1;
	int width_col = (width + 2 * pad_w - kernel_w) / stride_w + 1;
	int channels_col = channels * kernel_h * kernel_w;

	for (int c=0; c<channels_col; c++) {
		int w_offset = c % kernel_w;
		int h_offset = (c / kernel_w) % kernel_h;
		int c_im = c / kernel_h / kernel_w;
		for (int h=0; h<height_col; h++) {
			for (int w=0; w<width_col; w++) {
				int h_pad = h * stride_h - pad_h + h_offset;
				int w_pad = w * stride_w - pad_w + w_offset;
				if (h_pad >= 0 && h_pad < height && w_pad >= 0 && w_pad < width)
					col[(c * height_col + h) * width_col + w] =
						im[(c_im * height + h_pad) * width + w_pad];
				else
					col[(c * height_col + h) * width_col + w] = 0;
			}
		}
	}
}

real magic_kernel[4*4] = {
	1/64.0, 3/64.0, 3/64.0, 1/64.0,
	3/64.0, 9/64.0, 9/64.0, 3/64.0,
	3/64.0, 9/64.0, 9/64.0, 3/64.0,
	1/64.0, 3/64.0, 3/64.0, 1/64.0,
};

/*const int tab64[64] = {
	63,  0, 58,  1, 59, 47, 53,  2,
	60, 39, 48, 27, 54, 33, 42,  3,
	61, 51, 37, 40, 49, 18, 28, 20,
	55, 30, 34, 11, 43, 14, 22,  4,
	62, 57, 46, 52, 38, 26, 32, 41,
	50, 36, 17, 19, 29, 10, 13, 21,
	56, 45, 25, 31, 35, 16,  9, 12,
	44, 24, 15,  8, 23,  7,  6,  5};

int log2_64(uint64_t value)
{
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value |= value >> 32;
	return tab64[((uint64_t)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
}*/
int main(int argc, char* argv[])
{
	char *name = argv[1];
	char *outfile = "convolution.jpg";

/*	if (argc <=1) {
		usage(stderr, argv);
		return 0;
	}*/

	int platform = 0;
	int device = 0;
	for (int i=1; i<argc; i++) {
		if (!strcmp(argv[i], "-p")) {
			platform = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-d")) {
			device = atoi(argv[++i]);
		} else {
			name = argv[i];
		}
	}

	uint8_t *pixels;
	int w, h, bpp;
	pixels = stbi_load(name, &w, &h, &bpp, 3);
	assert(pixels);

	real *pix = malloc(sizeof(real)*w*h*3*10);
	for (int sy=0; sy<h; sy++) {
		for (int sx=0; sx<w; sx++) {
			pix[(w*sy+sx)]       = pixels[(w*sy+sx)*3  ] /255.0;
			pix[(w*sy+sx)+w*h]   = pixels[(w*sy+sx)*3+1] /255.0;
			pix[(w*sy+sx)+w*h*2] = pixels[(w*sy+sx)*3+2] /255.0;
		}
	}

	sgemm_ocl_init(platform, device, (w*h+w*h*4*4+4*4)*sizeof(float));
//	sgemm_ocl_init(platform, device, 1<<log2_64((w*h+w*h*4*4+4*4)*sizeof(float)));

	real *workspace = malloc(sizeof(real)*w*h*3*4*4*3);
//	im2col(pix, 3, h, w, 4, 4, 2, 2, 1, 1, workspace);
//	w += 1;
//	h += 1;
	im2col(pix, 3, h, w, 4, 4, 1, 1, 1, 1, workspace);
	w -= 1;
	h -= 1;

	// z = x * W [A(m,k) B(k,n) C(m,n)], cnhw
//	gemm('C', 'N', 'N', l->ox*l->oy*1, l->ch, l->ksize*l->ksize*l->ich, 1, workspace, l->ox*l->oy, l->W, l->ksize*l->ksize*l->ich, 0, l->z +l->outputs*i, l->ox*l->oy);
	//sgemm_ocl('N', 'N', w*h, 1, 4*4, workspace, magic_kernel, pix);
//	gemm('N', 'N', w*h, 1, 4*4, workspace, magic_kernel, pix);

	// https://qiita.com/t-tkd3a/items/6b17f296d61d14e12953
	sgemm_ocl('N', 'T', 1, w*h, 4*4, magic_kernel, workspace, pix);
//	gemm('N', 'T', 1, w*h, 4*4, magic_kernel, workspace, pix);

	sgemm_ocl_finish();

	for (int sy=0; sy<h; sy++) {
		for (int sx=0; sx<w; sx++) {
			pixels[(w*sy+sx)*3  ] = pix[(w*sy+sx)  ] *255.0;
			pixels[(w*sy+sx)*3+1] = pix[(w*sy+sx)  ] *255.0;
			pixels[(w*sy+sx)*3+2] = pix[(w*sy+sx)  ] *255.0;
		}
	}
	stbi_write_jpg(outfile, w, h, 3, pixels, 0);

	free(workspace);
	free(pix);

	stbi_image_free(pixels);
}
