// ©2017 Yuichiro Nakada
// clang -Os -o alsa_record alsa_record.c -lasound

#include <stdio.h>
#include "alsa.h"

int main(int argc, char *argv[])
{
	char *dev = "plughw:PCH,0";
	if (argc>1) dev = argv[1];

	char *name = "alsa_record.dat";
	if (argc>2) name = argv[2];

	AUDIO a;
	AUDIO_init(&a, dev, 48000, 1, 32, 0);	// device, 48000 samplerate, 1 channels, 32 frame

	do {
		int f = AUDIO_frame(&a); 	// audio data in a.buffer

		FILE *fp = fopen(name, "ab");
		fwrite(a.buffer, a.size, 1, fp);
		fclose(fp);
	} while (1);

	AUDIO_close(&a);
}

