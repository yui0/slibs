//---------------------------------------------------------
//	Cat's eye
//
//		©2013,2021 Yuichiro Nakada
//---------------------------------------------------------

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
//#define DR_MP3_IMPLEMENTATION
//#include "dr_mp3.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdint.h>
//#define USE_FIXED
#ifdef USE_FIXED
#include "ifft.h"
#else
#include "fft.h"
#endif

void create_spectrogram(char *name)
{
	uint32_t channels;
	uint32_t sampleRate;
	drwav_uint64 totalPCMFrameCount;
	float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(name, &channels, &sampleRate, &totalPCMFrameCount, NULL);
	if (pSampleData == NULL) return;

	int frameLength = 30;	// データ窓長(30ms)
	int frameShift = 20;	// データ窓シフト(20ms)
	int m = (int)log2(sampleRate*frameLength/1000.0);
	int window = 1<<m;
	int step = sampleRate*frameShift/1000.0;
//	int size = (totalPCMFrameCount/step+1)*step;
	int size = totalPCMFrameCount + window;

	printf("%s [%dHz] len:%llu\n", name, sampleRate, totalPCMFrameCount);
	printf("frame length: %d ms[%d], frame shift: %d ms[%d]\n\n", frameLength, window, frameShift, step);

#ifdef USE_FIXED
	short *rm = (short*)calloc(window, sizeof(short));
	short *im = (short*)calloc(window, sizeof(short));
/*	for (int i=0; i<totalPCMFrameCount; i++) {
		rm[i] = (short)(pSampleData[i*channels]*32767);
	}*/
#else
	double *rm = (double*)calloc(window, sizeof(double));
	double *im = (double*)calloc(window, sizeof(double));
/*	for (int i=0; i<totalPCMFrameCount; i++) {
		rm[i] = pSampleData[i*channels];
	}*/
#endif

	int count = (totalPCMFrameCount-step)/step;
	uint8_t *img = (uint8_t*)calloc(window/2*/*size/step*/count, sizeof(uint8_t)*3);

	printf(" Time(ms)\n");
	for (int i=0; i<count; i++) {
		printf("[%7d]", (i*step)*1000/sampleRate);

		for (int n=0; n<window; n++) {
			double hamming = ( 0.5 - 0.5 * cos( (2*n*M_PI) / (window - 1) ) ) * pSampleData[(i*step+n)*channels];
#ifdef USE_FIXED
			rm[n] = (short)(hamming*32767);
#else
			rm[n] = hamming;
#endif
		}

#ifdef USE_FIXED
		fix_fft(rm, im, m, 0);
//		int r = analyze(&rm[i], sampleRate*frameLength/1000.0, sampleRate, lpcOrder);
		for (int n=0; n<window/2; n++) {
			/*double rm2 = rm[n]/32768.0 * rm[n]/32768.0;
			double im2 = im[n]/32768.0 * im[n]/32768.0;
			double a = 10*log10(sqrt(rm2 + im2))*60;*/
			double a = 10*log10(sqrt(rm[n] * rm[n] + im[n] * im[n]))*20;
			if (a<0) a = 0;
			if (a>255) a = 255;
			img[window/2*i*3+n*3] = 256-a;
		}
		for (int n=0; n<window/2; n+=10) printf(" %d", rm[n]);
		printf("\n");
#else
		fft(window, rm, im);
		for (int n=0; n<window/2; n++) {
//			double a = 10*log10(sqrt(rm[n] * rm[n] + im[n] * im[n]))*60;
			double a = 10*log10(sqrt(rm[n] * rm[n] + im[n] * im[n]))*4+256;
			printf(" %f", a);
			if (a<0) a = 0;
			if (a>255) a = 255;
			img[window/2*i*3+n*3] = 256-a;
		}
		for (int n=0; n<window/2; n+=10) printf(" %f", rm[n]);
		printf("\n");
#endif
	}
//	stbi_write_jpg("im.jpg", window, size/step, 3, im, 0);

#if 0
	double max = 0;
	double min = 0;
	for (int i=0; i<size; i++) {
		if (im[i]>max) max = im[i];
		if (im[i]<min) min = im[i];
	}
	double range = max - min;
	printf("max: %f, min: %f\n", max, min);
	for (int i=0; i<size; i++) {
//		img[i*3] = (im[i]-min)/range*256;
#ifdef USE_FIXED
		double a = log(sqrt(rm[i] * rm[i] + im[i] * im[i]))*128;
#else
		double a = 10*log10(sqrt(rm[i] * rm[i] + im[i] * im[i]))*60;
#endif
		if (a<0) a = 0;
		if (a>255) a = 255;
//		img[i*3] = a;
		img[i*3] = 256-a;
		printf("%d ", img[i*3]);
	}
#endif
//	stbi_write_jpg("spectrogram.jpg", window, size/step, 3, img, 0);
	stbi_write_jpg("spectrogram.jpg", window/2, count, 3, img, 0);
	free(img);

	free(im);
	free(rm);
//	drwav_free(pSampleData);
}

int main(int argc, char *argv[])
{
	create_spectrogram(argv[1]);
}
