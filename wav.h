/* Simple, Minimalistic, WAVE library
 *	©2017 Yuichiro Nakada
 *
 * Latest revisions:
 * 	1.00 (12-03-2017) initial release
 *
 * Basic usage:
 *	WAVE_write("file.wav", totalSampleCount, data, sampleRate);
 * */

void WAVE_write_little_endian(unsigned int word, int num_bytes, FILE *fp)
{
	unsigned buf;
	while (num_bytes>0) {
		buf = word & 0xff;
		fwrite(&buf, 1, 1, fp);
		num_bytes--;
		word >>= 8;
	}
}

void WAVE_write(char *filename, unsigned long num_samples, short int *data, int sample_rate)
{
	unsigned int num_channels;
	unsigned int bytes_per_sample;
	unsigned int byte_rate;
	unsigned long i;	// counter for samples

	num_channels = 1;	// monaural
	bytes_per_sample = 2;
	byte_rate = sample_rate * num_channels * bytes_per_sample;

	FILE *fp = fopen(filename, "w");
	if (!fp) return;
	//assert(fp);	// make sure it opened

	// write RIFF header
	fwrite("RIFF", 1, 4, fp);
	WAVE_write_little_endian(36 + bytes_per_sample * num_samples * num_channels, 4, fp);
	fwrite("WAVE", 1, 4, fp);

	// write fmt subchunk
	fwrite("fmt ", 1, 4, fp);
	WAVE_write_little_endian(16, 4, fp);	// SubChunk1Size is 16
	WAVE_write_little_endian(1, 2, fp);	// PCM is format 1
	WAVE_write_little_endian(num_channels, 2, fp);
	WAVE_write_little_endian(sample_rate, 4, fp);
	WAVE_write_little_endian(byte_rate, 4, fp);
	WAVE_write_little_endian(num_channels*bytes_per_sample, 2, fp);	// block align
	WAVE_write_little_endian(8*bytes_per_sample, 2, fp);	// bits/sample

	// write data subchunk
	fwrite("data", 1, 4, fp);
	WAVE_write_little_endian(bytes_per_sample * num_samples*num_channels, 4, fp);
	for (i=0; i<num_samples; i++) {
		WAVE_write_little_endian((unsigned int)data[i], bytes_per_sample, fp);
	}

	fclose(fp);
}

