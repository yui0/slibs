/* public domain Simple, Minimalistic, Audio library for ALSA
 *	©2017,2020 Yuichiro Nakada
 *
 * Basic usage:
 *	AUDIO a;
 *	AUDIO_init(&a, "plughw:PCH,0", 48000, 2, 32, 1, 0); // device, 48000 samplerate, 2 channels, 32 frame
 *	...
 *	int f = AUDIO_frame(&a); // audio data in a.buffer
 *	...
 *	AUDIO_close(&a);
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

int AUDIO_init(AUDIO *thiz, char *dev, unsigned int freq, int ch, int frames, int flag, int format)
{
	// Open PCM device.
	int rc = snd_pcm_open(&thiz->handle, dev, flag ? SND_PCM_STREAM_PLAYBACK : SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		fprintf(stderr, "unable to open pcm device '%s' (%s)\n", dev, snd_strerror(rc));
		return 1;
	}

	// Allocate a hardware parameters object.
	snd_pcm_hw_params_t *params;
	snd_pcm_hw_params_alloca(&params);

	// Fill it in with default values.
	snd_pcm_hw_params_any(thiz->handle, params);

	// Set the desired hardware parameters.
	// Interleaved mode
	rc = snd_pcm_hw_params_set_access(thiz->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0) {
		fprintf(stderr, "cannot set access type (%s)\n", snd_strerror(rc));
		snd_pcm_drain(thiz->handle);
		snd_pcm_close(thiz->handle);
		return 1;
	}

	// Signed 16-bit little-endian format
	rc = snd_pcm_hw_params_set_format(thiz->handle, params, format ? format : SND_PCM_FORMAT_S16_LE);
	if (rc < 0) {
		fprintf(stderr, "cannot set sample format (%s)\n", snd_strerror(rc));
		snd_pcm_drain(thiz->handle);
		snd_pcm_close(thiz->handle);
		return 1;
	}

	// Channels
	rc = snd_pcm_hw_params_set_channels(thiz->handle, params, ch);
	if (rc < 0) {
		fprintf(stderr, "cannot set channel count (%s)\n", snd_strerror(rc));
		snd_pcm_drain(thiz->handle);
		snd_pcm_close(thiz->handle);
		return 1;
	}

	// 44100 bits/second sampling rate (CD quality)
	int dir;
	rc = snd_pcm_hw_params_set_rate_near(thiz->handle, params, &freq, &dir);
	if (rc < 0) {
		fprintf(stderr, "cannot set sample rate (%s)\n", snd_strerror(rc));
		snd_pcm_drain(thiz->handle);
		snd_pcm_close(thiz->handle);
		return 1;
	}

	// Set period size to 32 frames.
	thiz->frames = frames;
	snd_pcm_hw_params_set_period_size_near(thiz->handle, params, &thiz->frames, &dir);

	// Write the parameters to the driver
	rc = snd_pcm_hw_params(thiz->handle, params);
	if (rc < 0) {
		fprintf(stderr, "unable to set parameters (%s)\n", snd_strerror(rc));
		snd_pcm_drain(thiz->handle);
		snd_pcm_close(thiz->handle);
		return 1;
	}

	// Use a buffer large enough to hold one period
	snd_pcm_hw_params_get_period_size(params, &thiz->frames, &dir);
//	thiz->size = thiz->frames * 2 * ch; /* 2 bytes/sample, 2 channels */
	thiz->size = thiz->frames * 4 * ch; /* 4 bytes/sample, 2 channels */
	thiz->buffer = (char*)malloc(thiz->size);

	snd_pcm_hw_params_get_period_time(params, &freq, &dir);
	return 0;
}

int AUDIO_frame(AUDIO *thiz)
{
	int rc = snd_pcm_readi(thiz->handle, thiz->buffer, thiz->frames);
	if (rc == -EPIPE) {
		// EPIPE means overrun
		fprintf(stderr, "overrun occurred\n");
		snd_pcm_prepare(thiz->handle);
	} else if (rc < 0) {
		fprintf(stderr, "read failed (%s)\n", snd_strerror(rc));
	} else if (rc != (int)thiz->frames) {
		fprintf(stderr, "short read, read %d frames\n", rc);
	}
	return rc;
}

int AUDIO_play(AUDIO *thiz, char *data, int frames)
{
	int rc = snd_pcm_writei(thiz->handle, data, frames);
	if (rc == -EPIPE) {
		// EPIPE means overrun
		fprintf(stderr, "overrun occurred\n");
		snd_pcm_recover(thiz->handle, rc, 0);
		//snd_pcm_prepare(thiz->handle);
	} else if (rc < 0) {
		fprintf(stderr, "write failed (%s)\n", snd_strerror(rc));
	} else if (rc != frames) {
		fprintf(stderr, "short write, write %d/%d frames\n", rc, (int)thiz->frames);
	}
	return rc;
}

int AUDIO_play0(AUDIO *thiz)
{
	return AUDIO_play(thiz, thiz->buffer, thiz->frames);
}

void AUDIO_wait(AUDIO *thiz, int msec)
{
	snd_pcm_wait(thiz->handle, msec);
}

void AUDIO_close(AUDIO *thiz)
{
	snd_pcm_drain(thiz->handle);
	snd_pcm_close(thiz->handle);
	free(thiz->buffer);
}

// Normalize a PCM device string ("hw:7,0", "plughw:Kazane,0", ...) into an
// ALSA mixer card name ("hw:7", "hw:Kazane"). Returns 0 on success.
static int AUDIO_mixer_card_from_dev(const char *dev, char *out, size_t outsz)
{
	const char *p = dev ? dev : "default";
	if (!strncmp(p, "plug", 4)) {
		p += 4; /* plughw: -> hw: */
	}
	if (!strncmp(p, "hw:", 3) || !strncmp(p, "default", 7)) {
		snprintf(out, outsz, "%s", p);
	} else {
		/* surround51:CARD=..., sysdefault:CARD=Foo, etc. -> try as-is first */
		snprintf(out, outsz, "%s", p);
	}
	char *comma = strchr(out, ',');
	if (comma) {
		*comma = 0;
	}
	return 0;
}

// Pick a playback-volume mixer element. Prefers common names, then the first
// selem that exposes a playback volume (USB DACs often name it after the
// product, e.g. "Kazane+", rather than "Master"/"PCM").
static snd_mixer_elem_t *AUDIO_find_playback_volume(snd_mixer_t *mixer)
{
	static const char *prefer[] = {
		"Master", "PCM", "Speaker", "Headphone", "Playback", "Digital", NULL
	};
	snd_mixer_selem_id_t *sid;
	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);

	for (int i = 0; prefer[i]; i++) {
		snd_mixer_selem_id_set_name(sid, prefer[i]);
		snd_mixer_elem_t *elem = snd_mixer_find_selem(mixer, sid);
		if (elem && snd_mixer_selem_has_playback_volume(elem)) {
			return elem;
		}
	}
	for (snd_mixer_elem_t *elem = snd_mixer_first_elem(mixer); elem;
	     elem = snd_mixer_elem_next(elem)) {
		if (snd_mixer_selem_has_playback_volume(elem)) {
			return elem;
		}
	}
	return NULL;
}

// Sets playback volume (0.0-1.0) through the ALSA mixer instead of scaling
// PCM samples, so the decoded audio that reaches AUDIO_play() is left
// completely untouched. `card` may be a mixer card ("hw:7") or a full PCM
// device string ("hw:7,0" / "plughw:Kazane,0") - the latter is normalized.
// Returns 0 on success, -1 if no usable mixer control was found.
int AUDIO_set_volume(const char *card, float vol)
{
	snd_mixer_t *mixer;
	snd_mixer_elem_t *elem;
	char mixcard[128];

	if (vol < 0.0f) {
		vol = 0.0f;
	}
	if (vol > 1.0f) {
		vol = 1.0f;
	}

	AUDIO_mixer_card_from_dev(card, mixcard, sizeof(mixcard));

	if (snd_mixer_open(&mixer, 0) < 0) {
		return -1;
	}
	if (snd_mixer_attach(mixer, mixcard) < 0) {
		/* Last resort: walk every card and use the first that works. */
		snd_mixer_close(mixer);
		if (snd_mixer_open(&mixer, 0) < 0) {
			return -1;
		}
		int c = -1, attached = 0;
		while (snd_card_next(&c) >= 0 && c >= 0) {
			char hw[32];
			snprintf(hw, sizeof(hw), "hw:%d", c);
			if (snd_mixer_attach(mixer, hw) >= 0) {
				attached = 1;
				break;
			}
		}
		if (!attached) {
			snd_mixer_close(mixer);
			return -1;
		}
	}
	if (snd_mixer_selem_register(mixer, NULL, NULL) < 0) {
		snd_mixer_close(mixer);
		return -1;
	}
	if (snd_mixer_load(mixer) < 0) {
		snd_mixer_close(mixer);
		return -1;
	}

	elem = AUDIO_find_playback_volume(mixer);
	if (!elem) {
		snd_mixer_close(mixer);
		return -1;
	}

	long min, max;
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	long v = min + (long)((max - min) * vol + 0.5f);
	snd_mixer_selem_set_playback_volume_all(elem, v);
	if (snd_mixer_selem_has_playback_switch(elem)) {
		snd_mixer_selem_set_playback_switch_all(elem, vol > 0.0f ? 1 : 0);
	}

	snd_mixer_close(mixer);
	return 0;
}

