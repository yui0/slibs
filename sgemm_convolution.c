// ©2020 Yuichiro Nakada
// clang convolution.c -o convolution -lm -lOpenCL

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
//#define STB_IMAGE_RESIZE_IMPLEMENTATION
//#include "stb_image_resize.h"

#define _DEBUG
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
//				sum += a[k + m*K] * b[n + N*k]; // TT
//				sum += a[k + m*K] * b[n*K + k]; // TN
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

	sgemm_ocl_init(platform, device, (w*h*1+w*h*1*4*4+4*4*1*1+w*h*1)*sizeof(float));

/*	real *workspace = malloc(sizeof(real)*w*h*3*4*4*3);
//	im2col(pix, 3, h, w, 4, 4, 2, 2, 1, 1, workspace);
//	w += 1;
//	h += 1;
	im2col(pix, 3, h, w, 4, 4, 1, 1, 1, 1, workspace);
	w -= 1;
	h -= 1;

	// https://qiita.com/t-tkd3a/items/6b17f296d61d14e12953
	sgemm_ocl('N', 'T', 1, w*h, 4*4, magic_kernel, workspace, pix);
//	gemm('N', 'T', 1, w*h, 4*4, magic_kernel, workspace, pix);
//	gemm('N', 'N', w*h, 1, 4*4, workspace, magic_kernel, pix);
	free(workspace);*/

/*	real *workspace = malloc(sizeof(real)*w*h*3*4*4*3);
	ocl_im2col(pix, 1, w, h, 4, 1, 1, workspace);
	w -= 1;
	h -= 1;
	gemm('N', 'T', 1, w*h, 4*4, magic_kernel, workspace, pix);
//	gemm('N', 'T', w*h, 1, 4*4, workspace, magic_kernel, pix);
//	gemm('N', 'N', w*h, 1, 4*4, workspace, magic_kernel, pix);
	free(workspace);*/

//	real *workspace = malloc(sizeof(real)*w*h*3*4*4*3);
//	im2col(pix, 1, h, w, 4, 4, 1, 1, 1, 1, workspace);
//	ocl_convolution(workspace, 1, w, h, magic_kernel, 4, 1, 1, pix, 1);
	ocl_convolution(pix, 1, w, h, magic_kernel, 4, 1, 1, pix, 1);
	w -= 1;
	h -= 1;

	sgemm_ocl_finish();

	for (int sy=0; sy<h; sy++) {
		for (int sx=0; sx<w; sx++) {
			pixels[(w*sy+sx)*3  ] = pix[(w*sy+sx)  ] *255.0;
			pixels[(w*sy+sx)*3+1] = pix[(w*sy+sx)  ] *255.0;
			pixels[(w*sy+sx)*3+2] = pix[(w*sy+sx)  ] *255.0;
		}
	}
	stbi_write_jpg(outfile, w, h, 3, pixels, 0);

	free(pix);

	stbi_image_free(pixels);
}
