// ©2017 Yuichiro Nakada
// clang -Os -o adpcm_codec adpcm_codec.c
#include <stdio.h>
#include "adpcm.h"
#include "wav.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

void encode(char *name)
{
	unsigned int channels;
	unsigned int sampleRate;
	unsigned long totalSampleCount;
	s16 *pSampleData = drwav_open_and_read_file_s16(name, &channels, &sampleRate, &totalSampleCount);
	if (pSampleData) {
		printf("ch:%d, %dHz, %lu samples\n", channels, sampleRate, totalSampleCount);

		int prev = 0;
		int index = 0;
		u8 *enc = malloc(sizeof(u8)*totalSampleCount);
		ADPCM_encode(&prev, &index, pSampleData, totalSampleCount, enc);

		FILE *fp = fopen("a.adpcm", "wb");
		if (!fp) return;
		fputc(channels, fp);
		fputc(sampleRate&0xff, fp);
		fputc((sampleRate>>8)&0xff, fp);
		//fwrite(&sampleRate, sizeof(unsigned int), 1, fp);
		//fwrite(&totalSampleCount, sizeof(unsigned long), 1, fp);
		fwrite(enc, totalSampleCount/2, 1, fp);
		fclose(fp);

		free(enc);
		drwav_free(pSampleData);
	}
}

void decode(char *name)
{
	FILE *fp = fopen(name, "rb");
	if (!fp) return;

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp) -3;
	fseek(fp, 0, SEEK_SET);

	int channels = fgetc(fp);
	int sampleRate = fgetc(fp);
	sampleRate |= (fgetc(fp)<<8);

	u8 *enc = malloc(sizeof(u8)*size);
	s16 *data = malloc(sizeof(s16)*size);
	fread(enc, size, 1, fp);
	fclose(fp);

	int prev = 0;
	int index = 0;
	ADPCM_decode(&prev, &index, enc, size*2, data);
	WAVE_write("a.wav", size*2, data, sampleRate);

	free(data);
	free(enc);
}

int main(int argc, char *argv[])
{
	if (argc<1) {
		printf("%s [wav/adpcm file name]\n", argv[0]);
		fprintf(fp,
			"Usage: %s [options] [wav/adpcm file name]\n\n"
			"Options:\n"
			"-h | --help          Print this message\n"
			"",
			argv[0]);
		return 0;
	}

	if (strstr(argv[1], ".wav")) {
		encode(argv[1]);
	} else if (strstr(argv[1], ".adpcm")) {
		decode(argv[1]);
	} else {
		unsigned int channels;
		unsigned int sampleRate;
		unsigned long totalSampleCount;
		//float *pSampleData = drwav_open_and_read_file_f32(argv[1], &channels, &sampleRate, &totalSampleCount);
		s16 *pSampleData = drwav_open_and_read_file_s16(argv[2], &channels, &sampleRate, &totalSampleCount);
		if (pSampleData) {
			printf("ch:%d, %dHz, %lu samples\n", channels, sampleRate, totalSampleCount);

			s16 *data = malloc(sizeof(s16)*totalSampleCount);
			u8 *enc = malloc(sizeof(u8)*totalSampleCount);
			/*for (int i=0; i<totalSampleCount; i++) {
				data[i] = pSampleData[i]*32767;
			}*/

			int prev = 0;
			int index = 0;
			ADPCM_encode(&prev, &index, /*data*/pSampleData, totalSampleCount, enc);

			prev = 0;
			index = 0;
			ADPCM_decode(&prev, &index, enc, totalSampleCount, data);

			WAVE_write("test.wav", totalSampleCount, data, sampleRate);

			free(enc);
			free(data);
			drwav_free(pSampleData);
		}
	}

	return 0;
}

