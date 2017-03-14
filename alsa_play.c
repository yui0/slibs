// ©2017 Yuichiro Nakada
// clang -Os -o alsa_play alsa_play.c -lasound

#include <stdio.h>
#include "alsa.h"

int main(int argc, char *argv[])
{
	char *dev = "hw:0,0"; //"default";
	if (argc>1) dev = argv[1];

	char *name = "alsa_record.dat";
	if (argc>2) name = argv[2];

	AUDIO a;
	AUDIO_init(&a, dev, 48000, 1, 32, 1);	// device, 48000 samplerate, 1 channels, 32 frame

	int frame;
	FILE *fp = fopen(name, "rb");
	do {
		if (!fread(a.buffer, a.size, 1, fp)) break;
		//if (feof(fp)) break;
		frame = AUDIO_play(&a);
		//printf("%d ", frame);
	} while (1);
	fclose(fp);

	AUDIO_close(&a);
}

