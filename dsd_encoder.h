/* public domain Simple, Minimalistic, DSD library
 *	©2026 Yuichiro Nakada
 *
 * Basic usage:

#include <stdio.h>
#define DSD_ENCODER_IMPLEMENTATION
#include "dsd_encoder.h"

int main() {
    // 任意の整数倍オーバーサンプル (例: 44.1kHz → DSD64)
    DSDEncoder* enc = dsd_encoder_init(2, 44100, 2822400);
    if (!enc) return -1;

    float pcm_chunk[256 * 2];
    uint8_t dsd_out[256 * 8 * 2];
    size_t bytes_per_ch = 0;
    dsd_encoder_process(enc, pcm_chunk, 256, SND_PCM_FORMAT_FLOAT_LE,
                        dsd_out, sizeof(dsd_out), &bytes_per_ch);

    dsd_encoder_free(enc);
    return 0;
}
*/

#ifndef DSD_ENCODER_H
#define DSD_ENCODER_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ALSA format fallbacks (shared with dsd.h when both are included). */
#ifndef SND_PCM_FORMAT_S16_LE
#define SND_PCM_FORMAT_S16_LE 2
#endif
#ifndef SND_PCM_FORMAT_S24_LE
#define SND_PCM_FORMAT_S24_LE 6
#endif
#ifndef SND_PCM_FORMAT_S32_LE
#define SND_PCM_FORMAT_S32_LE 10
#endif
#ifndef SND_PCM_FORMAT_FLOAT_LE
#define SND_PCM_FORMAT_FLOAT_LE 14
#endif
#ifndef SND_PCM_FORMAT_S24_3LE
#define SND_PCM_FORMAT_S24_3LE 32
#endif

#ifndef DSDENC_MAX_CHANNELS
#define DSDENC_MAX_CHANNELS 8
#endif
#define DSDENC_MAX_STAGES 9           /* 2^9 = 512x → DSD512 */
#define DSDENC_HALFBAND_TAPS 31       /* odd length; odd taps forced to 0 (halfband) */
#define DSDENC_DELAY_SIZE 32          /* power-of-2 ring >= taps */
#define DSDENC_DELAY_MASK (DSDENC_DELAY_SIZE - 1)
#define DSDENC_PHASE_TAPS ((DSDENC_HALFBAND_TAPS + 1) / 2) /* even-phase non-zero taps */

/* Output bit layout (matches dsd.h containers). */
#define DSDENC_LAYOUT_DSF 0  /* per-channel contiguous, LSB-first */
#define DSDENC_LAYOUT_DFF 1  /* byte-interleaved, MSB-first */

/* Upsampling mode */
#define DSDENC_MODE_HALFBAND 0 /* power-of-2 OSR: cascaded 2x halfband */
#define DSDENC_MODE_LINEAR   1 /* any integer OSR: linear interpolation */

typedef struct {
    float delay[DSDENC_DELAY_SIZE];
    unsigned idx; /* write index into ring */
} DsdEncInterpStage;

typedef struct {
    float x1, x2;
    float y_prev;
} DsdEncModState;

typedef struct {
    int channels;
    int pcm_rate;
    int dsd_rate;
    int stages;              /* log2(osr) when MODE_HALFBAND; 0 otherwise */
    int osr;                 /* dsd_rate / pcm_rate */
    int mode;                /* DSDENC_MODE_* */
    int layout;              /* DSDENC_LAYOUT_* */
    float halfband_even[DSDENC_PHASE_TAPS]; /* polyphase even branch */
    float halfband_center;                  /* odd branch = delayed * center */
    int center_delay;                       /* samples of delay for odd phase */
    float input_scale;

    DsdEncInterpStage interp[DSDENC_MAX_CHANNELS][DSDENC_MAX_STAGES];
    DsdEncModState    mod[DSDENC_MAX_CHANNELS];
    float             prev_pcm[DSDENC_MAX_CHANNELS]; /* for linear mode */

    uint8_t bit_acc[DSDENC_MAX_CHANNELS];
    int     bit_acc_count[DSDENC_MAX_CHANNELS];

    int dop_toggle;
} DSDEncoder;

#ifdef DSD_ENCODER_IMPLEMENTATION

static float dsdenc_bessel_i0(float x) {
    float sum = 1.0f, term = 1.0f, x2 = (x * x) * 0.25f;
    for (int k = 1; k < 32; ++k) {
        term *= x2 / ((float)k * (float)k);
        sum += term;
        if (term < 1e-15f * sum) break;
    }
    return sum;
}

/* Design a true halfband FIR (odd taps = 0 except center), Kaiser windowed.
 * Returns polyphase even-branch coefficients and the center tap for the odd branch. */
static void dsdenc_design_halfband(float* even_out, int phase_taps, float* center_out,
                                   int* center_delay_out, float beta) {
    const int taps = DSDENC_HALFBAND_TAPS;
    float h[DSDENC_HALFBAND_TAPS];
    int M = taps - 1;
    float center = M * 0.5f;
    float i0_beta = dsdenc_bessel_i0(beta);
    float sum = 0.0f;

    for (int n = 0; n < taps; ++n) {
        float m = (float)n - center;
        float sinc;
        if (fabsf(m) < 1e-6f) {
            sinc = 0.5f;
        } else {
            sinc = sinf((float)M_PI * m * 0.5f) / ((float)M_PI * m);
        }
        float ratio = m / center;
        float w = dsdenc_bessel_i0(beta * sqrtf(1.0f - ratio * ratio)) / i0_beta;
        h[n] = sinc * w;
        sum += h[n];
    }
    /* Normalize DC to 1, then *2 to compensate zero-insertion (polyphase form). */
    float scale = 2.0f / sum;
    for (int n = 0; n < taps; ++n) h[n] *= scale;

    /* Force exact halfband: odd taps (except center) → 0. */
    int c = M / 2;
    for (int n = 0; n < taps; ++n) {
        if ((n & 1) && n != c) h[n] = 0.0f;
    }

    /* Even phase: h[0], h[2], h[4], ... */
    for (int k = 0; k < phase_taps; ++k) {
        int n = k * 2;
        even_out[k] = (n < taps) ? h[n] : 0.0f;
    }
    *center_out = h[c];
    *center_delay_out = c / 2; /* in input-sample units for the odd polyphase */
}

/* One 2x upsample step via polyphase halfband (no memmove, no explicit zero-insert).
 * out[0] = even phase (FIR on even coeffs), out[1] = odd phase (center * delayed). */
static inline void dsdenc_interp_stage_push(DsdEncInterpStage* st, const float* even,
                                             float center, int center_delay,
                                             float in_sample, float out[2]) {
    unsigned idx = st->idx;
    st->delay[idx & DSDENC_DELAY_MASK] = in_sample;

    float acc = 0.0f;
    /* even[k] multiplies x[n - k] */
    for (int k = 0; k < DSDENC_PHASE_TAPS; ++k) {
        acc += even[k] * st->delay[(idx - (unsigned)k) & DSDENC_DELAY_MASK];
    }
    out[0] = acc;
    out[1] = center * st->delay[(idx - (unsigned)center_delay) & DSDENC_DELAY_MASK];

    st->idx = idx + 1;
}

static inline float dsdenc_modulate_bit(DsdEncModState* m, float u) {
    m->x1 += (u - m->y_prev);
    m->x2 += (m->x1 - m->y_prev);
    float y = (m->x2 >= 0.0f) ? 1.0f : -1.0f;
    m->y_prev = y;
    return y;
}

static inline float dsdenc_load_sample(const void* pcm_in, size_t index, int format) {
    switch (format) {
    case SND_PCM_FORMAT_FLOAT_LE:
        return ((const float*)pcm_in)[index];
    case SND_PCM_FORMAT_S32_LE:
        return (float)((const int32_t*)pcm_in)[index] * (1.0f / 2147483648.0f);
    case SND_PCM_FORMAT_S24_LE: {
        int32_t v = ((const int32_t*)pcm_in)[index];
        /* Sign-extend low 24 bits. */
        v = (v << 8) >> 8;
        return (float)v * (1.0f / 8388608.0f);
    }
    case SND_PCM_FORMAT_S24_3LE: {
        const uint8_t* p = (const uint8_t*)pcm_in + index * 3;
        int32_t v = (int32_t)p[0] | ((int32_t)p[1] << 8) | ((int32_t)p[2] << 16);
        if (v & 0x800000) v |= ~0xFFFFFF;
        return (float)v * (1.0f / 8388608.0f);
    }
    default: { /* S16_LE */
        return (float)((const int16_t*)pcm_in)[index] * (1.0f / 32768.0f);
    }
    }
}

static inline void dsdenc_emit_bit(DSDEncoder* enc, int ch, int bit,
                                   uint8_t* ch_out, size_t* byte_pos) {
    if (enc->layout == DSDENC_LAYOUT_DFF) {
        /* MSB-first packing */
        enc->bit_acc[ch] |= (uint8_t)(bit << (7 - enc->bit_acc_count[ch]));
    } else {
        /* LSB-first (DSF) */
        enc->bit_acc[ch] |= (uint8_t)(bit << enc->bit_acc_count[ch]);
    }
    enc->bit_acc_count[ch]++;
    if (enc->bit_acc_count[ch] == 8) {
        ch_out[(*byte_pos)++] = enc->bit_acc[ch];
        enc->bit_acc[ch] = 0;
        enc->bit_acc_count[ch] = 0;
    }
}

DSDEncoder* dsd_encoder_init(int channels, int pcm_rate, int dsd_rate) {
    if (channels <= 0 || channels > DSDENC_MAX_CHANNELS) return NULL;
    if (pcm_rate <= 0 || dsd_rate <= 0 || dsd_rate % pcm_rate != 0) return NULL;

    int ratio = dsd_rate / pcm_rate;
    if (ratio < 2 || ratio > (1 << DSDENC_MAX_STAGES)) return NULL;

    int stages = 0;
    int mode = DSDENC_MODE_LINEAR;
    while ((1 << stages) < ratio) stages++;
    if ((1 << stages) == ratio && stages <= DSDENC_MAX_STAGES) {
        mode = DSDENC_MODE_HALFBAND;
    } else {
        stages = 0; /* unused in linear mode */
    }

    DSDEncoder* enc = (DSDEncoder*)calloc(1, sizeof(DSDEncoder));
    if (!enc) return NULL;

    enc->channels = channels;
    enc->pcm_rate = pcm_rate;
    enc->dsd_rate = dsd_rate;
    enc->stages = stages;
    enc->osr = ratio;
    enc->mode = mode;
    enc->layout = DSDENC_LAYOUT_DSF;
    enc->input_scale = 0.5f;
    enc->dop_toggle = 0;

    dsdenc_design_halfband(enc->halfband_even, DSDENC_PHASE_TAPS,
                           &enc->halfband_center, &enc->center_delay, 7.0f);

    for (int ch = 0; ch < channels; ++ch) {
        enc->mod[ch].y_prev = -1.0f; /* idle mid-scale-ish */
        enc->prev_pcm[ch] = 0.0f;
    }

    return enc;
}

void dsd_encoder_free(DSDEncoder* enc) {
    free(enc);
}

void dsd_encoder_set_input_scale(DSDEncoder* enc, double scale) {
    if (!enc) return;
    if (scale < 0.1) scale = 0.1;
    if (scale > 0.95) scale = 0.95;
    enc->input_scale = (float)scale;
}

void dsd_encoder_set_layout(DSDEncoder* enc, int layout) {
    if (!enc) return;
    enc->layout = (layout == DSDENC_LAYOUT_DFF) ? DSDENC_LAYOUT_DFF : DSDENC_LAYOUT_DSF;
}

/* ---------------------------------------------------------------------
 * Process interleaved PCM of any supported ALSA format into raw DSD bits.
 *
 * layout DSF (default): ch0 bytes | ch1 bytes | ...  (LSB-first) — matches dsd.h
 * layout DFF:           byte-interleaved across channels (MSB-first)
 *
 * For DFF layout, out_bytes_per_channel is still reported per channel, and
 * dsd_out is sized as bytes_per_ch * channels with interleaved storage:
 *   out[frame_byte * channels + ch]
 * --------------------------------------------------------------------- */
int dsd_encoder_process(DSDEncoder* enc, const void* pcm_in, size_t frames, int format,
                         uint8_t* dsd_out, size_t dsd_out_cap,
                         size_t* out_bytes_per_channel) {
    if (!enc || !pcm_in || !dsd_out || frames == 0 || !out_bytes_per_channel) return -1;

    size_t osr = (size_t)enc->osr;
    size_t total_bits = frames * osr;
    size_t bytes_needed_per_ch = (total_bits + 7) / 8 + 1;
    /* Stride = floor(cap / channels). Callers such as aplay+ index with their
     * allocation stride (often exact_bytes+2), so honour the buffer layout
     * rather than packing channels tightly with bytes_needed_per_ch. */
    if (enc->channels <= 0) return -1;
    size_t stride = dsd_out_cap / (size_t)enc->channels;
    if (stride < bytes_needed_per_ch) return -2;

    uint8_t* ch_out[DSDENC_MAX_CHANNELS];
    size_t byte_pos[DSDENC_MAX_CHANNELS];
    memset(byte_pos, 0, sizeof(byte_pos));

    for (int ch = 0; ch < enc->channels; ++ch)
        ch_out[ch] = dsd_out + (size_t)ch * stride;

    float stage_buf_a[1 << DSDENC_MAX_STAGES];
    float stage_buf_b[1 << DSDENC_MAX_STAGES];

    for (size_t i = 0; i < frames; ++i) {
        for (int ch = 0; ch < enc->channels; ++ch) {
            float sample = dsdenc_load_sample(pcm_in, i * (size_t)enc->channels + (size_t)ch, format)
                         * enc->input_scale;

            if (enc->mode == DSDENC_MODE_HALFBAND) {
                float* cur = stage_buf_a;
                float* nxt = stage_buf_b;
                cur[0] = sample;
                size_t count = 1;
                for (int s = 0; s < enc->stages; ++s) {
                    for (size_t k = 0; k < count; ++k) {
                        float pair[2];
                        dsdenc_interp_stage_push(&enc->interp[ch][s], enc->halfband_even,
                                                 enc->halfband_center, enc->center_delay,
                                                 cur[k], pair);
                        nxt[2 * k]     = pair[0];
                        nxt[2 * k + 1] = pair[1];
                    }
                    count *= 2;
                    float* tmp = cur; cur = nxt; nxt = tmp;
                }
                for (size_t k = 0; k < osr; ++k) {
                    float bit_val = dsdenc_modulate_bit(&enc->mod[ch], cur[k]);
                    dsdenc_emit_bit(enc, ch, (bit_val > 0.0f) ? 1 : 0, ch_out[ch], &byte_pos[ch]);
                }
            } else {
                /* Linear interpolation between previous and current PCM sample. */
                float prev = enc->prev_pcm[ch];
                float inv_osr = 1.0f / (float)osr;
                for (size_t k = 0; k < osr; ++k) {
                    float t = ((float)k + 1.0f) * inv_osr;
                    float u = prev + (sample - prev) * t;
                    float bit_val = dsdenc_modulate_bit(&enc->mod[ch], u);
                    dsdenc_emit_bit(enc, ch, (bit_val > 0.0f) ? 1 : 0, ch_out[ch], &byte_pos[ch]);
                }
                enc->prev_pcm[ch] = sample;
            }
        }
    }

    size_t nbytes = byte_pos[0];
    for (int ch = 1; ch < enc->channels; ++ch) {
        if (byte_pos[ch] < nbytes) nbytes = byte_pos[ch];
    }

    if (enc->layout == DSDENC_LAYOUT_DFF && nbytes > 0) {
        /* Re-interleave: temp copy then write byte-interleaved into dsd_out. */
        size_t total = nbytes * (size_t)enc->channels;
        uint8_t* tmp = (uint8_t*)malloc(total);
        if (!tmp) return -3;
        for (size_t b = 0; b < nbytes; ++b) {
            for (int ch = 0; ch < enc->channels; ++ch)
                tmp[b * (size_t)enc->channels + (size_t)ch] = ch_out[ch][b];
        }
        memcpy(dsd_out, tmp, total);
        free(tmp);
    }

    *out_bytes_per_channel = nbytes;
    return 0;
}

/* Backward-compatible float32 wrapper. */
int dsd_encoder_process_raw(DSDEncoder* enc, const float* pcm_in, size_t frames,
                             uint8_t* dsd_out, size_t dsd_out_cap,
                             size_t* out_bytes_per_channel) {
    return dsd_encoder_process(enc, pcm_in, frames, SND_PCM_FORMAT_FLOAT_LE,
                               dsd_out, dsd_out_cap, out_bytes_per_channel);
}

/* DoP packing: 16 DSD bits + marker → 24-bit PCM sample (in int32_t low 24 bits).
 * Input is DSF-layout per-channel contiguous bytes (as produced by default process). */
int dsd_encoder_pack_dop(DSDEncoder* enc, const uint8_t* const dsd_bytes_per_ch[],
                          size_t n_bytes_per_channel, int32_t* out_pcm24,
                          size_t* out_frames) {
    if (!enc || !dsd_bytes_per_ch || !out_pcm24 || !out_frames) return -1;
    size_t n_frames = n_bytes_per_channel / 2;
    for (size_t f = 0; f < n_frames; ++f) {
        uint8_t marker = enc->dop_toggle ? 0xFA : 0x05;
        enc->dop_toggle = !enc->dop_toggle;
        for (int ch = 0; ch < enc->channels; ++ch) {
            uint8_t b0 = dsd_bytes_per_ch[ch][f * 2 + 0];
            uint8_t b1 = dsd_bytes_per_ch[ch][f * 2 + 1];
            int32_t v = ((int32_t)marker << 16) | ((int32_t)b0 << 8) | (int32_t)b1;
            out_pcm24[f * enc->channels + ch] = v;
        }
    }
    *out_frames = n_frames;
    return 0;
}

#endif /* DSD_ENCODER_IMPLEMENTATION */
#endif /* DSD_ENCODER_H */
