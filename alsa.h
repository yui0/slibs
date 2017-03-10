/* public domain Simple, Minimalistic, Audio library for ALSA
 *	©2017 Yuichiro Nakada
 *
 * Latest revisions:
 * 	1.00 (20-02-2017) initial release
 *
 * Basic usage:
 *	AUDIO a;
 *	AUDIO_init(&a, "plughw:PCH,0", 48000, 2, 32); // device, 48000 samplerate, 2 channels, 32 frame
 *	...
 *	int f = AUDIO_frame(&a); // audio data in a.buffer
 *	...
 *	AUDIO_close(&a);
 *
 * Notes:
 *
 * */

// Use the newer ALSA API
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

typedef struct {
	snd_pcm_t *handle;
	snd_pcm_uframes_t frames;
	char *buffer;
	int size;
} AUDIO;

void AUDIO_init(AUDIO *this, char *dev, unsigned int val, int ch, int frames)
{
	// Open PCM device for recording (capture).
	int rc = snd_pcm_open(&this->handle, dev, SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
		exit(1);
	}

	// Allocate a hardware parameters object.
	snd_pcm_hw_params_t *params;
	snd_pcm_hw_params_alloca(&params);

	// Fill it in with default values.
	snd_pcm_hw_params_any(this->handle, params);

	// Set the desired hardware parameters.
	// Interleaved mode
	snd_pcm_hw_params_set_access(this->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

	// Signed 16-bit little-endian format
	snd_pcm_hw_params_set_format(this->handle, params, SND_PCM_FORMAT_S16_LE);

	// Channels
	snd_pcm_hw_params_set_channels(this->handle, params, ch);

	// 44100 bits/second sampling rate (CD quality)
	int dir;
	snd_pcm_hw_params_set_rate_near(this->handle, params, &val, &dir);

	// Set period size to 32 frames.
	this->frames = frames;
	snd_pcm_hw_params_set_period_size_near(this->handle, params, &this->frames, &dir);

	// Write the parameters to the driver
	rc = snd_pcm_hw_params(this->handle, params);
	if (rc < 0) {
		fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
		exit(1);
	}

	// Use a buffer large enough to hold one period
	snd_pcm_hw_params_get_period_size(params, &this->frames, &dir);
	this->size = this->frames * 2 * ch; /* 2 bytes/sample, 2 channels */
	this->buffer = (char*)malloc(this->size);

	// We want to loop for 5 seconds
	snd_pcm_hw_params_get_period_time(params, &val, &dir);
}

int AUDIO_frame(AUDIO *this)
{
	int rc = snd_pcm_readi(this->handle, this->buffer, this->frames);
	if (rc == -EPIPE) {
		// EPIPE means overrun
		fprintf(stderr, "overrun occurred\n");
		snd_pcm_prepare(this->handle);
	} else if (rc < 0) {
		fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
	} else if (rc != (int)this->frames) {
		fprintf(stderr, "short read, read %d frames\n", rc);
	}
	return rc;
}

void AUDIO_close(AUDIO *this)
{
	snd_pcm_drain(this->handle);
	snd_pcm_close(this->handle);
	free(this->buffer);
}

