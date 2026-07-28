/* public domain Simple, Minimalistic, DSD library
 *	©2026 Yuichiro Nakada
 *
 * Basic usage:

#include <stdio.h>
#define DSD_DECODER_IMPLEMENTATION
#include "dsd.h"

int main() {
    FILE* file = fopen("sample.dsf", "rb");
    if (!file) return -1;
    DSDDecoder* decoder = dsd_decoder_init_file(file);
    if (!decoder) { fclose(file); return -1; }

    float pcm[256 * 2];
    size_t n;
    while ((n = dsd_decoder_read_pcm_frames(decoder, 256, pcm, SND_PCM_FORMAT_FLOAT_LE)) > 0) {
        // play pcm[0 .. n*channels)
    }
    dsd_decoder_free(decoder);
    fclose(file);
    return 0;
}
*/

#ifndef DSD_H
#define DSD_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ALSA format ids (fallbacks when <alsa/asoundlib.h> is not included). */
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

#define DSD_SAMPLES_PER_BYTE 8

/* Avoid clashing with other headers that also define MAX_CHANNELS (e.g. uwma.h). */
#ifndef DSD_MAX_CHANNELS
#define DSD_MAX_CHANNELS 8
#endif
#ifndef MAX_CHANNELS
#define MAX_CHANNELS DSD_MAX_CHANNELS
#endif

#define DSD_FILTER_STAGES 4

/* Source container format.
 * DSF (Sony):   little-endian ints, LSB-first bits, per-channel contiguous blocks.
 * DFF/DSDIFF:   big-endian ints, MSB-first bits, byte-interleaved channels. */
#define DSD_FILE_DSF 0
#define DSD_FILE_DFF 1

typedef struct {
    float x1, x2;
    float y1, y2;
} FilterState2;

typedef struct {
    float a0, a1, a2;
    float b1, b2;
} FilterCoeff2;

typedef struct {
    FILE* file;
    int file_type; /* DSD_FILE_DSF or DSD_FILE_DFF */
    uint64_t dsd_data_offset;
    uint64_t totalPCMFrameCount;
    uint64_t pcm_frames_processed;

    int sample_rate_dsd;
    int sample_rate_pcm;
    int channels;
    uint32_t block_size_bytes;

    uint8_t* block_buffer;
    size_t block_buffer_size;
    int owns_block_buffer; /* 1 = free in dsd_decoder_free, 0 = caller-owned (raw) */

    size_t current_dsd_bit_index;
    size_t valid_bits_in_block; /* bits available in current block (per channel) */

    FilterState2 filter_state[DSD_MAX_CHANNELS][DSD_FILTER_STAGES];
    FilterCoeff2 filter_coeff;

    int initial_rms_estimation_done;
    float current_scale_factor;
    long current_file_pos;
} DSDDecoder;

#ifdef DSD_DECODER_IMPLEMENTATION

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static uint32_t read_le32(const uint8_t* buf) {
    return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}
static uint64_t read_le64(const uint8_t* buf) {
    return (uint64_t)buf[0] | ((uint64_t)buf[1] << 8) | ((uint64_t)buf[2] << 16) | ((uint64_t)buf[3] << 24)
         | ((uint64_t)buf[4] << 32) | ((uint64_t)buf[5] << 40) | ((uint64_t)buf[6] << 48) | ((uint64_t)buf[7] << 56);
}
static uint32_t read_be32(const uint8_t* buf) {
    return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
}
static uint64_t read_be64(const uint8_t* buf) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v = (v << 8) | (uint64_t)buf[i];
    return v;
}

/* Pick a PCM rate that divides the DSD rate cleanly.
 * Prefer ~176.4/192 kHz (good realtime budget, matches DoP/DSD64 conventions);
 * fall back to higher/lower common rates for DSD512 / unusual clocks.
 * Covers both 44.1k and 48k families for DSD64..DSD512. */
static int dsd_choose_pcm_rate(int dsd_sample_rate) {
    static const int candidates[] = {
        176400, 192000, 88200, 96000, 352800, 384000, 44100, 48000
    };
    if (dsd_sample_rate <= 0) return 0;
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        int pcm = candidates[i];
        if (dsd_sample_rate % pcm == 0) {
            int dec = dsd_sample_rate / pcm;
            if (dec >= 8 && dec <= 512) return pcm;
        }
    }
    int dec = 32;
    while (dec > 8 && (dsd_sample_rate / dec) * dec != dsd_sample_rate) dec >>= 1;
    while (dec < 512 && (dsd_sample_rate / dec) * dec != dsd_sample_rate) dec <<= 1;
    if ((dsd_sample_rate / dec) * dec != dsd_sample_rate) dec = 32;
    return dsd_sample_rate / dec;
}

static inline float dsd_bit_lsb(uint8_t b, int bit) {
    return ((b >> bit) & 1) ? 1.0f : -1.0f;
}
static inline float dsd_bit_msb(uint8_t b, int bit) {
    return ((b >> (7 - bit)) & 1) ? 1.0f : -1.0f;
}

/* Resolve physical byte + bit order for channel `ch`, bit index within block. */
static inline float dsd_bit_value(const DSDDecoder* decoder, int ch, size_t bit_in_block) {
    size_t byte_in_channel = bit_in_block >> 3;
    int bit_in_byte = (int)(bit_in_block & 7);
    if (decoder->file_type == DSD_FILE_DFF) {
        size_t physical_byte = byte_in_channel * (size_t)decoder->channels + (size_t)ch;
        return dsd_bit_msb(decoder->block_buffer[physical_byte], bit_in_byte);
    } else {
        size_t physical_byte = (size_t)ch * decoder->block_size_bytes + byte_in_channel;
        return dsd_bit_lsb(decoder->block_buffer[physical_byte], bit_in_byte);
    }
}

static int dsd_load_next_block(DSDDecoder* decoder) {
    if (!decoder || !decoder->file || !decoder->block_buffer) return 0;
    size_t bytes_read = fread(decoder->block_buffer, 1, decoder->block_buffer_size, decoder->file);
    decoder->current_dsd_bit_index = 0;
    if (bytes_read == 0) {
        decoder->valid_bits_in_block = 0;
        return 0;
    }
    /* Partial trailing block: only fully-read per-channel bytes are valid. */
    size_t bytes_per_ch = bytes_read / (size_t)decoder->channels;
    if (bytes_per_ch > decoder->block_size_bytes) bytes_per_ch = decoder->block_size_bytes;
    decoder->valid_bits_in_block = bytes_per_ch * DSD_SAMPLES_PER_BYTE;
    return 1;
}

static void init_filter_coeff(FilterCoeff2* coeff, float cutoff_freq, float sample_rate_dsd) {
    float omega = tanf((float)M_PI * cutoff_freq / sample_rate_dsd);
    float k = 1.41421356237f; /* sqrt(2) */
    float denom = 1.0f + k * omega + omega * omega;
    coeff->a0 = omega * omega / denom;
    coeff->a1 = 2.0f * coeff->a0;
    coeff->a2 = coeff->a0;
    coeff->b1 = 2.0f * (omega * omega - 1.0f) / denom;
    coeff->b2 = (1.0f - k * omega + omega * omega) / denom;
}

static inline float apply_filter2(FilterState2* state, const FilterCoeff2* coeff, float input) {
    float output = coeff->a0 * input + coeff->a1 * state->x1 + coeff->a2 * state->x2
                 - coeff->b1 * state->y1 - coeff->b2 * state->y2;
    state->x2 = state->x1;
    state->x1 = input;
    state->y2 = state->y1;
    state->y1 = output;
    return output;
}

/* Run the 4-stage LPF on one bipolar DSD sample. */
static inline float dsd_filter_sample(DSDDecoder* decoder, int ch, float dsd_val) {
    float temp = dsd_val;
    for (int stage = 0; stage < DSD_FILTER_STAGES; ++stage) {
        temp = apply_filter2(&decoder->filter_state[ch][stage], &decoder->filter_coeff, temp);
    }
    return temp;
}

/* Cheap soft-knee limiter (avoids libm tanh in the hot path). */
static inline float dsd_soft_limit(float x) {
    const float KNEE = 0.9f;
    float av = fabsf(x);
    if (av <= KNEE) return x;
    float over = av - KNEE;
    /* tanh(u) ≈ u*(27+u^2)/(27+9*u^2), u = over/(1-KNEE) */
    float u = over / (1.0f - KNEE);
    float u2 = u * u;
    float th = u * (27.0f + u2) / (27.0f + 9.0f * u2);
    float compressed = KNEE + (1.0f - KNEE) * th;
    if (compressed > 1.0f) compressed = 1.0f;
    return (x < 0.0f) ? -compressed : compressed;
}

static inline void dsd_store_sample(void* buffer, size_t index, float pcm_val, int format) {
    pcm_val = dsd_soft_limit(pcm_val);
    switch (format) {
    case SND_PCM_FORMAT_FLOAT_LE:
        if (pcm_val > 1.0f) pcm_val = 1.0f;
        if (pcm_val < -1.0f) pcm_val = -1.0f;
        ((float*)buffer)[index] = pcm_val;
        break;
    case SND_PCM_FORMAT_S32_LE: {
        double s = (double)pcm_val * 2147483647.0;
        if (s > 2147483647.0) s = 2147483647.0;
        if (s < -2147483648.0) s = -2147483648.0;
        ((int32_t*)buffer)[index] = (int32_t)s;
        break;
    }
    case SND_PCM_FORMAT_S24_LE: {
        /* 24-bit audio in the low 24 bits of a 32-bit word (ALSA S24_LE). */
        double s = (double)pcm_val * 8388607.0;
        if (s > 8388607.0) s = 8388607.0;
        if (s < -8388608.0) s = -8388608.0;
        ((int32_t*)buffer)[index] = (int32_t)s;
        break;
    }
    case SND_PCM_FORMAT_S24_3LE: {
        double s = (double)pcm_val * 8388607.0;
        if (s > 8388607.0) s = 8388607.0;
        if (s < -8388608.0) s = -8388608.0;
        int32_t v = (int32_t)s;
        uint8_t* p = (uint8_t*)buffer + index * 3;
        p[0] = (uint8_t)(v & 0xFF);
        p[1] = (uint8_t)((v >> 8) & 0xFF);
        p[2] = (uint8_t)((v >> 16) & 0xFF);
        break;
    }
    default: { /* SND_PCM_FORMAT_S16_LE and anything unknown */
        int32_t s16_val = (int32_t)(pcm_val * 32767.0f);
        if (s16_val > 32767) s16_val = 32767;
        if (s16_val < -32768) s16_val = -32768;
        ((int16_t*)buffer)[index] = (int16_t)s16_val;
        break;
    }
    }
}

static int dsd_parse_dff_header(DSDDecoder* decoder, FILE* file, uint64_t* out_total_dsd_samples, uint64_t* out_data_size) {
    uint8_t hdr[12];
    if (fread(hdr, 1, 12, file) != 12 || strncmp((char*)hdr, "FRM8", 4) != 0) return -1;
    uint64_t frm8_size = read_be64(hdr + 4);

    uint8_t form_type[4];
    if (fread(form_type, 1, 4, file) != 4 || strncmp((char*)form_type, "DSD ", 4) != 0) return -1;

    uint64_t end_offset = 12 + frm8_size;

    int got_fs = 0, got_chnl = 0, got_data = 0;
    uint32_t sample_rate = 0;
    int channels = 0;
    uint64_t data_offset = 0, data_size = 0;

    while ((uint64_t)ftell(file) + 12 <= end_offset) {
        uint8_t chdr[12];
        if (fread(chdr, 1, 12, file) != 12) break;
        uint64_t size = read_be64(chdr + 4);
        long chunk_data_start = ftell(file);

        if (strncmp((char*)chdr, "PROP", 4) == 0) {
            uint8_t local_id[4];
            if (fread(local_id, 1, 4, file) != 4) return -1;
            if (strncmp((char*)local_id, "SND ", 4) == 0) {
                uint64_t prop_end = (uint64_t)chunk_data_start + size;
                while ((uint64_t)ftell(file) + 12 <= prop_end) {
                    uint8_t shdr[12];
                    if (fread(shdr, 1, 12, file) != 12) break;
                    uint64_t ssize = read_be64(shdr + 4);
                    long sub_data_start = ftell(file);

                    if (strncmp((char*)shdr, "FS  ", 4) == 0 && ssize >= 4) {
                        uint8_t buf4[4];
                        if (fread(buf4, 1, 4, file) == 4) { sample_rate = read_be32(buf4); got_fs = 1; }
                    } else if (strncmp((char*)shdr, "FS ", 3) == 0 && ssize >= 4) {
                        /* Some writers pad the id as "FS " (3 chars + space already in id). */
                        uint8_t buf4[4];
                        if (fread(buf4, 1, 4, file) == 4) { sample_rate = read_be32(buf4); got_fs = 1; }
                    } else if (strncmp((char*)shdr, "CHNL", 4) == 0 && ssize >= 2) {
                        uint8_t buf2[2];
                        if (fread(buf2, 1, 2, file) == 2) { channels = ((int)buf2[0] << 8) | (int)buf2[1]; got_chnl = 1; }
                    } else if (strncmp((char*)shdr, "CMPR", 4) == 0 && ssize >= 4) {
                        uint8_t buf4[4];
                        if (fread(buf4, 1, 4, file) == 4 && strncmp((char*)buf4, "DSD ", 4) != 0) {
                            return -1; /* DST etc. not supported */
                        }
                    }
                    uint64_t padded = ssize + (ssize & 1);
                    fseek(file, sub_data_start + (long)padded, SEEK_SET);
                }
            }
        } else if (strncmp((char*)chdr, "DSD ", 4) == 0) {
            data_offset = (uint64_t)chunk_data_start;
            data_size = size;
            got_data = 1;
            break;
        } else if (strncmp((char*)chdr, "DST ", 4) == 0) {
            return -1;
        }

        uint64_t padded = size + (size & 1);
        fseek(file, chunk_data_start + (long)padded, SEEK_SET);
    }

    if (!got_fs || !got_chnl || !got_data || channels < 1 || channels > DSD_MAX_CHANNELS || data_size == 0) return -1;

    decoder->sample_rate_dsd = (int)sample_rate;
    decoder->channels = channels;
    fseek(file, (long)data_offset, SEEK_SET);

    *out_data_size = data_size;
    *out_total_dsd_samples = (data_size / (uint64_t)channels) * 8ULL;
    return 0;
}

DSDDecoder* dsd_decoder_init_file(FILE* file) {
    if (!file) return NULL;
    DSDDecoder* decoder = (DSDDecoder*)calloc(1, sizeof(DSDDecoder));
    if (!decoder) return NULL;
    decoder->file = file;

    uint8_t magic[4];
    if (fread(magic, 1, 4, file) != 4) { free(decoder); return NULL; }
    fseek(file, 0, SEEK_SET);

    uint64_t total_dsd_samples = 0;

    if (strncmp((char*)magic, "FRM8", 4) == 0) {
        decoder->file_type = DSD_FILE_DFF;
        uint64_t data_size = 0;
        if (dsd_parse_dff_header(decoder, file, &total_dsd_samples, &data_size) != 0) { free(decoder); return NULL; }
        decoder->block_size_bytes = 4096;
    } else if (strncmp((char*)magic, "DSD ", 4) == 0) {
        decoder->file_type = DSD_FILE_DSF;
        uint8_t header_buf[80];
        if (fread(header_buf, 1, 28, file) != 28 || strncmp((char*)header_buf, "DSD ", 4) != 0) { free(decoder); return NULL; }
        uint64_t fmt_chunk_offset = 28;
        fseek(file, (long)fmt_chunk_offset, SEEK_SET);
        if (fread(header_buf, 1, 52, file) != 52 || strncmp((char*)header_buf, "fmt ", 4) != 0) { free(decoder); return NULL; }

        uint64_t fmt_chunk_size = read_le64(header_buf + 4);
        decoder->channels = (int)read_le32(header_buf + 24);
        decoder->sample_rate_dsd = (int)read_le32(header_buf + 28);
        uint32_t bits_per_sample = read_le32(header_buf + 32); /* must be 1 for raw DSD */
        total_dsd_samples = read_le64(header_buf + 36);
        decoder->block_size_bytes = read_le32(header_buf + 44);

        if (bits_per_sample != 1 || decoder->channels < 1 || decoder->channels > DSD_MAX_CHANNELS || decoder->block_size_bytes == 0) {
            free(decoder);
            return NULL;
        }

        fseek(file, (long)(fmt_chunk_offset + fmt_chunk_size), SEEK_SET);
        char chunk_id[12];
        if (fread(chunk_id, 1, 12, file) != 12 || strncmp(chunk_id, "data", 4) != 0) { free(decoder); return NULL; }
    } else {
        free(decoder);
        return NULL;
    }

    decoder->sample_rate_pcm = dsd_choose_pcm_rate(decoder->sample_rate_dsd);
    if (decoder->sample_rate_pcm <= 0) { free(decoder); return NULL; }

    size_t decimation_factor = (size_t)decoder->sample_rate_dsd / (size_t)decoder->sample_rate_pcm;
    if (decimation_factor == 0) { free(decoder); return NULL; }
    decoder->totalPCMFrameCount = total_dsd_samples / decimation_factor;

    decoder->block_buffer_size = (size_t)decoder->block_size_bytes * (size_t)decoder->channels;
    decoder->block_buffer = (uint8_t*)malloc(decoder->block_buffer_size);
    decoder->owns_block_buffer = 1;
    if (!decoder->block_buffer) { free(decoder); return NULL; }

    init_filter_coeff(&decoder->filter_coeff, (float)decoder->sample_rate_pcm * 0.5f, (float)decoder->sample_rate_dsd);
    memset(decoder->filter_state, 0, sizeof(decoder->filter_state));

    dsd_load_next_block(decoder);

    decoder->initial_rms_estimation_done = 0;
    decoder->current_scale_factor = 1.0f;
    decoder->current_file_pos = ftell(file);

    return decoder;
}

void dsd_decoder_free(DSDDecoder* decoder) {
    if (decoder) {
        if (decoder->owns_block_buffer) free(decoder->block_buffer);
        free(decoder);
    }
}

/* Optional: override the auto-chosen PCM rate before the first read.
 * `pcm_rate` must evenly divide sample_rate_dsd. Returns 0 on success. */
int dsd_decoder_set_pcm_rate(DSDDecoder* decoder, int pcm_rate) {
    if (!decoder || pcm_rate <= 0) return -1;
    if (decoder->sample_rate_dsd % pcm_rate != 0) return -1;
    int dec = decoder->sample_rate_dsd / pcm_rate;
    if (dec < 2) return -1;

    int was_streaming = (decoder->totalPCMFrameCount == (uint64_t)-1);
    if (!was_streaming && decoder->sample_rate_pcm > 0) {
        /* Preserve total DSD sample count across the rate change. */
        uint64_t total_dsd = decoder->totalPCMFrameCount *
            ((uint64_t)decoder->sample_rate_dsd / (uint64_t)decoder->sample_rate_pcm);
        decoder->totalPCMFrameCount = total_dsd / (uint64_t)dec;
    }

    decoder->sample_rate_pcm = pcm_rate;
    init_filter_coeff(&decoder->filter_coeff, (float)pcm_rate * 0.5f, (float)decoder->sample_rate_dsd);
    memset(decoder->filter_state, 0, sizeof(decoder->filter_state));
    decoder->pcm_frames_processed = 0;
    decoder->current_dsd_bit_index = 0;
    return 0;
}

/* Filter `nbits` contiguous DSD bits for one channel starting at `start_bit`.
 * Caller guarantees start_bit .. start_bit+nbits-1 lie inside the current block. */
static float dsd_accumulate_range(DSDDecoder* decoder, int ch, size_t start_bit, size_t nbits) {
    float accum = 0.0f;
    if (nbits == 0) return 0.0f;

    if (decoder->file_type != DSD_FILE_DFF && (start_bit & 7) == 0) {
        const uint8_t* base = decoder->block_buffer
            + (size_t)ch * decoder->block_size_bytes + (start_bit >> 3);
        size_t bits_left = nbits;
        size_t bi = 0;
        while (bits_left >= 8) {
            uint8_t b = base[bi++];
            accum += dsd_filter_sample(decoder, ch, dsd_bit_lsb(b, 0));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_lsb(b, 1));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_lsb(b, 2));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_lsb(b, 3));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_lsb(b, 4));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_lsb(b, 5));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_lsb(b, 6));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_lsb(b, 7));
            bits_left -= 8;
        }
        if (bits_left) {
            uint8_t b = base[bi];
            for (size_t k = 0; k < bits_left; ++k)
                accum += dsd_filter_sample(decoder, ch, dsd_bit_lsb(b, (int)k));
        }
        return accum;
    }

    if (decoder->file_type == DSD_FILE_DFF && (start_bit & 7) == 0) {
        size_t byte_i = start_bit >> 3;
        size_t bits_left = nbits;
        while (bits_left >= 8) {
            uint8_t b = decoder->block_buffer[byte_i * (size_t)decoder->channels + (size_t)ch];
            ++byte_i;
            accum += dsd_filter_sample(decoder, ch, dsd_bit_msb(b, 0));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_msb(b, 1));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_msb(b, 2));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_msb(b, 3));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_msb(b, 4));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_msb(b, 5));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_msb(b, 6));
            accum += dsd_filter_sample(decoder, ch, dsd_bit_msb(b, 7));
            bits_left -= 8;
        }
        if (bits_left) {
            uint8_t b = decoder->block_buffer[byte_i * (size_t)decoder->channels + (size_t)ch];
            for (size_t k = 0; k < bits_left; ++k)
                accum += dsd_filter_sample(decoder, ch, dsd_bit_msb(b, (int)k));
        }
        return accum;
    }

    for (size_t k = 0; k < nbits; ++k)
        accum += dsd_filter_sample(decoder, ch, dsd_bit_value(decoder, ch, start_bit + k));
    return accum;
}

static void dsd_decoder_estimate_rms(DSDDecoder* decoder, int format) {
    (void)format;
    if (decoder->initial_rms_estimation_done) return;
    if (!decoder->file) {
        decoder->current_scale_factor = 1.0f;
        decoder->initial_rms_estimation_done = 1;
        return;
    }

    fseek(decoder->file, decoder->current_file_pos, SEEK_SET);
    memset(decoder->filter_state, 0, sizeof(decoder->filter_state));
    decoder->current_dsd_bit_index = 0;
    decoder->pcm_frames_processed = 0;
    dsd_load_next_block(decoder);

    const int ESTIMATION_FRAMES = decoder->sample_rate_pcm * 2;
    float* temp_pcm_buffer = (float*)malloc((size_t)ESTIMATION_FRAMES * (size_t)decoder->channels * sizeof(float));
    if (!temp_pcm_buffer) {
        decoder->current_scale_factor = 1.0f;
        decoder->initial_rms_estimation_done = 1;
        return;
    }

    size_t decimation_factor = (size_t)decoder->sample_rate_dsd / (size_t)decoder->sample_rate_pcm;
    size_t block_bits = decoder->valid_bits_in_block;
    if (block_bits == 0) block_bits = (size_t)decoder->block_size_bytes * DSD_SAMPLES_PER_BYTE;

    double sum_squares = 0.0;
    float peak_abs = 0.0f;
    size_t actual_frames_processed = 0;
    float inv_dec = 1.0f / (float)decimation_factor;

    for (int i = 0; i < ESTIMATION_FRAMES; ++i) {
        size_t start = decoder->current_dsd_bit_index;
        size_t rem = (start < block_bits) ? (block_bits - start) : 0;

        if (rem >= decimation_factor) {
            for (int ch = 0; ch < decoder->channels; ++ch) {
                float sample = dsd_accumulate_range(decoder, ch, start, decimation_factor) * inv_dec;
                temp_pcm_buffer[i * decoder->channels + ch] = sample;
                sum_squares += (double)sample * (double)sample;
                float av = fabsf(sample);
                if (av > peak_abs) peak_abs = av;
            }
            decoder->current_dsd_bit_index = start + decimation_factor;
            if (decoder->current_dsd_bit_index >= block_bits) {
                if (!dsd_load_next_block(decoder)) goto end_estimation_loop;
                block_bits = decoder->valid_bits_in_block;
            }
        } else {
            /* Cross block boundary: finish rem bits for every channel, then load. */
            float partial[DSD_MAX_CHANNELS];
            for (int ch = 0; ch < decoder->channels; ++ch)
                partial[ch] = dsd_accumulate_range(decoder, ch, start, rem);
            if (!dsd_load_next_block(decoder)) goto end_estimation_loop;
            block_bits = decoder->valid_bits_in_block;
            size_t need = decimation_factor - rem;
            if (need > block_bits) goto end_estimation_loop;
            for (int ch = 0; ch < decoder->channels; ++ch) {
                float sample = (partial[ch] + dsd_accumulate_range(decoder, ch, 0, need)) * inv_dec;
                temp_pcm_buffer[i * decoder->channels + ch] = sample;
                sum_squares += (double)sample * (double)sample;
                float av = fabsf(sample);
                if (av > peak_abs) peak_abs = av;
            }
            decoder->current_dsd_bit_index = need;
        }
        actual_frames_processed++;
    }

end_estimation_loop:
    {
        double average_rms_sq = 0.0;
        if (actual_frames_processed > 0) {
            average_rms_sq = sum_squares / (double)(actual_frames_processed * (size_t)decoder->channels);
        }
        double estimated_rms = sqrt(average_rms_sq);

        const double TARGET_RMS = 0.25;
        const double PEAK_HEADROOM = 0.9;

        double scale_from_rms = 1.0;
        if (estimated_rms > 1e-9) scale_from_rms = TARGET_RMS / estimated_rms;

        double scale_from_peak = 4.0;
        if (peak_abs > 1e-9f) scale_from_peak = PEAK_HEADROOM / (double)peak_abs;

        double scale = scale_from_rms;
        if (scale > scale_from_peak) scale = scale_from_peak;
        if (scale > 4.0) scale = 4.0;
        if (scale < 0.05) scale = 0.05;

        decoder->current_scale_factor = (float)scale;
    }

    free(temp_pcm_buffer);
    decoder->initial_rms_estimation_done = 1;

    fseek(decoder->file, decoder->current_file_pos, SEEK_SET);
    memset(decoder->filter_state, 0, sizeof(decoder->filter_state));
    decoder->current_dsd_bit_index = 0;
    decoder->pcm_frames_processed = 0;
    dsd_load_next_block(decoder);
}

size_t dsd_decoder_read_pcm_frames(DSDDecoder* decoder, size_t frames_to_read, void* buffer, int format) {
    if (!decoder || !buffer || frames_to_read == 0) return 0;
    if (decoder->pcm_frames_processed >= decoder->totalPCMFrameCount) return 0;
    if (!decoder->block_buffer) return 0;

    if (!decoder->initial_rms_estimation_done) {
        dsd_decoder_estimate_rms(decoder, format);
    }

    size_t decimation_factor = (size_t)decoder->sample_rate_dsd / (size_t)decoder->sample_rate_pcm;
    if (decimation_factor == 0) return 0;

    size_t frames_read = 0;
    size_t block_bits = decoder->valid_bits_in_block;
    if (block_bits == 0) block_bits = (size_t)decoder->block_size_bytes * DSD_SAMPLES_PER_BYTE;

    float inv_dec = 1.0f / (float)decimation_factor;
    float scale = decoder->current_scale_factor;

    for (size_t i = 0; i < frames_to_read; ++i) {
        size_t start = decoder->current_dsd_bit_index;
        size_t rem = (start < block_bits) ? (block_bits - start) : 0;

        if (rem >= decimation_factor) {
            for (int ch = 0; ch < decoder->channels; ++ch) {
                float pcm_val = dsd_accumulate_range(decoder, ch, start, decimation_factor) * inv_dec * scale;
                dsd_store_sample(buffer, i * (size_t)decoder->channels + (size_t)ch, pcm_val, format);
            }
            decoder->current_dsd_bit_index = start + decimation_factor;
            if (decoder->current_dsd_bit_index >= block_bits) {
                if (decoder->file) {
                    if (!dsd_load_next_block(decoder)) {
                        frames_read++;
                        decoder->pcm_frames_processed++;
                        goto end_loop;
                    }
                    block_bits = decoder->valid_bits_in_block;
                } else {
                    frames_read++;
                    decoder->pcm_frames_processed++;
                    goto end_loop;
                }
            }
        } else if (rem > 0 && decoder->file) {
            float partial[DSD_MAX_CHANNELS];
            for (int ch = 0; ch < decoder->channels; ++ch)
                partial[ch] = dsd_accumulate_range(decoder, ch, start, rem);
            if (!dsd_load_next_block(decoder)) goto end_loop;
            block_bits = decoder->valid_bits_in_block;
            size_t need = decimation_factor - rem;
            if (need > block_bits) goto end_loop;
            for (int ch = 0; ch < decoder->channels; ++ch) {
                float pcm_val = (partial[ch] + dsd_accumulate_range(decoder, ch, 0, need)) * inv_dec * scale;
                dsd_store_sample(buffer, i * (size_t)decoder->channels + (size_t)ch, pcm_val, format);
            }
            decoder->current_dsd_bit_index = need;
        } else {
            /* No bits left (raw EOF or empty block). */
            goto end_loop;
        }

        decoder->pcm_frames_processed++;
        frames_read++;
        if (decoder->pcm_frames_processed >= decoder->totalPCMFrameCount) goto end_loop;
    }

end_loop:
    return frames_read;
}

/* ---------------------------------------------------------------------
 * In-memory (non-file) decode API — same filter / limiter pipeline as
 * file playback. Layout must match DSF: per-channel contiguous, LSB-first.
 * --------------------------------------------------------------------- */

#define DSD_RAW_DECODER_DEFAULT_SCALE 2.0f

DSDDecoder* dsd_decoder_init_raw(int channels, int dsd_sample_rate, int block_size_bytes_per_ch) {
    if (channels < 1 || channels > DSD_MAX_CHANNELS || dsd_sample_rate <= 0 || block_size_bytes_per_ch <= 0) return NULL;

    DSDDecoder* decoder = (DSDDecoder*)calloc(1, sizeof(DSDDecoder));
    if (!decoder) return NULL;

    decoder->file = NULL;
    decoder->file_type = DSD_FILE_DSF;
    decoder->channels = channels;
    decoder->sample_rate_dsd = dsd_sample_rate;
    decoder->sample_rate_pcm = dsd_choose_pcm_rate(dsd_sample_rate);
    if (decoder->sample_rate_pcm <= 0) { free(decoder); return NULL; }

    decoder->block_size_bytes = (uint32_t)block_size_bytes_per_ch;
    decoder->block_buffer = NULL;
    decoder->block_buffer_size = (size_t)block_size_bytes_per_ch * (size_t)channels;
    decoder->owns_block_buffer = 0;
    decoder->current_dsd_bit_index = 0;
    decoder->valid_bits_in_block = (size_t)block_size_bytes_per_ch * DSD_SAMPLES_PER_BYTE;
    decoder->totalPCMFrameCount = (uint64_t)-1;
    decoder->pcm_frames_processed = 0;

    init_filter_coeff(&decoder->filter_coeff, (float)decoder->sample_rate_pcm * 0.5f, (float)dsd_sample_rate);
    memset(decoder->filter_state, 0, sizeof(decoder->filter_state));

    decoder->initial_rms_estimation_done = 1;
    decoder->current_scale_factor = DSD_RAW_DECODER_DEFAULT_SCALE;

    return decoder;
}

void dsd_decoder_feed_block(DSDDecoder* decoder, uint8_t* block_buffer) {
    if (!decoder) return;
    decoder->block_buffer = block_buffer;
    decoder->current_dsd_bit_index = 0;
    decoder->valid_bits_in_block = (size_t)decoder->block_size_bytes * DSD_SAMPLES_PER_BYTE;
}

size_t dsd_decoder_frames_per_block(const DSDDecoder* decoder) {
    if (!decoder || decoder->sample_rate_pcm <= 0) return 0;
    size_t decimation_factor = (size_t)decoder->sample_rate_dsd / (size_t)decoder->sample_rate_pcm;
    if (decimation_factor == 0) return 0;
    return ((size_t)decoder->block_size_bytes * DSD_SAMPLES_PER_BYTE) / decimation_factor;
}

#endif /* DSD_DECODER_IMPLEMENTATION */
#endif /* DSD_H */
