/*
#include <stdio.h>
#define DSD_DECODER_IMPLEMENTATION
#include "dsd_decoder.h"

int main() {
    FILE* file = fopen("sample.dsf", "rb");
    if (!file) {
        printf("Failed to open file\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t* data = (uint8_t*)malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    // デコーダ初期化（初期サイズは仮）
    DSDDecoder* decoder = dsd_decoder_init(size, DSD_DEFAULT_SAMPLE_RATE, PCM_DEFAULT_SAMPLE_RATE, 2);
    if (!decoder) {
        free(data);
        return -1;
    }

    if (dsd_decoder_load_dsf(decoder, data, size) != 0) {
        printf("Failed to load DSF data\n");
        dsd_decoder_free(decoder);
        free(data);
        return -1;
    }

    // PCMに変換
    if (dsd_decoder_convert_to_pcm(decoder) != 0) {
        printf("Failed to convert to PCM\n");
        dsd_decoder_free(decoder);
        free(data);
        return -1;
    }

    // PCMデータ取得
    size_t pcm_size;
    const int32_t* pcm_data = dsd_decoder_get_pcm_data(decoder, &pcm_size);

    FILE* out = fopen("output.pcm", "wb");
    fwrite(pcm_data, sizeof(int32_t), pcm_size, out);
    fclose(out);

    dsd_decoder_free(decoder);
    free(data);
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

// ALSA format definitions
#ifndef SND_PCM_FORMAT_S16_LE
#define SND_PCM_FORMAT_S16_LE 2
#endif
#ifndef SND_PCM_FORMAT_FLOAT_LE
#define SND_PCM_FORMAT_FLOAT_LE 14
#endif

#define DSD_SAMPLES_PER_BYTE 8
#define MAX_CHANNELS 2

// Source container format. DSF (Sony) is little-endian, LSB-first within a
// byte, and stores each channel's DSD bits in its own contiguous sub-block.
// DFF/DSDIFF (Philips) is big-endian, MSB-first within a byte, and
// interleaves channels byte-by-byte instead of block-by-block.
#define DSD_FILE_DSF 0
#define DSD_FILE_DFF 1

// 2次フィルタの状態と係数を定義
typedef struct {
    double x1, x2; // 入力の過去値
    double y1, y2; // 出力の過去値
} FilterState2;

typedef struct {
    double a0, a1, a2; // フィードフォワード係数
    double b1, b2;     // フィードバック係数
} FilterCoeff2;

typedef struct {
    FILE* file;
    int file_type; // DSD_FILE_DSF or DSD_FILE_DFF
    uint64_t dsd_data_offset;
    uint64_t totalPCMFrameCount;
    uint64_t pcm_frames_processed;

    int sample_rate_dsd;
    int sample_rate_pcm;
    int channels;
    uint32_t block_size_bytes;

    uint8_t* block_buffer;
    size_t block_buffer_size;
    
    // Indexing and filter state
    size_t current_dsd_bit_index;

    // 4段階の2次フィルタ
    FilterState2 filter_state[MAX_CHANNELS][4];
    FilterCoeff2 filter_coeff;

    // --- RMS 推定のための追加フィールド ---
    int initial_rms_estimation_done; // RMS推定が完了したかどうかのフラグ
    double current_scale_factor;     // 適用するスケーリング係数
    long current_file_pos;           // RMS推定後のファイルポインタを保存
    // ------------------------------------

} DSDDecoder;

#ifdef DSD_DECODER_IMPLEMENTATION

// Helper functions (no changes)
static uint32_t read_le32(const uint8_t* buf) { return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24); }
static uint64_t read_le64(const uint8_t* buf) { return (uint64_t)buf[0] | ((uint64_t)buf[1] << 8) | ((uint64_t)buf[2] << 16) | ((uint64_t)buf[3] << 24) | ((uint64_t)buf[4] << 32) | ((uint64_t)buf[5] << 40) | ((uint64_t)buf[6] << 48) | ((uint64_t)buf[7] << 56); }

// DFF/DSDIFF chunk sizes and integers are big-endian.
static uint32_t read_be32(const uint8_t* buf) { return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3]; }
static uint64_t read_be64(const uint8_t* buf) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v = (v << 8) | (uint64_t)buf[i];
    return v;
}

// DSD64/128/256 -> a common 176.4kHz PCM rate (decimation factor 16/32/64);
// anything else falls back to a straight /32 decimation.
static int dsd_choose_pcm_rate(int dsd_sample_rate) {
    if (dsd_sample_rate == 2822400) return 176400;
    if (dsd_sample_rate == 5644800) return 176400;
    if (dsd_sample_rate == 11289600) return 176400;
    return dsd_sample_rate / 32;
}

// Returns the DSD sample (+1.0 / -1.0) for channel `ch` at bit position
// `bit_in_block` (0-based, counted from the start of the current
// block_buffer, per channel). Abstracts away the two container layouts:
//   DSF: each channel occupies its own contiguous run of block_size_bytes,
//        bits read LSB-first.
//   DFF: channels are interleaved byte-by-byte across the buffer, bits read
//        MSB-first.
static inline double dsd_bit_value(const DSDDecoder* decoder, int ch, size_t bit_in_block) {
    size_t byte_in_channel = bit_in_block / DSD_SAMPLES_PER_BYTE;
    int bit_in_byte = (int)(bit_in_block % DSD_SAMPLES_PER_BYTE);
    size_t physical_byte;
    int bit_pos;
    if (decoder->file_type == DSD_FILE_DFF) {
        physical_byte = byte_in_channel * (size_t)decoder->channels + (size_t)ch;
        bit_pos = 7 - bit_in_byte; // MSB-first
    } else {
        physical_byte = (size_t)ch * decoder->block_size_bytes + byte_in_channel;
        bit_pos = bit_in_byte; // LSB-first
    }
    return ((decoder->block_buffer[physical_byte] >> bit_pos) & 1) ? 1.0 : -1.0;
}

static int dsd_load_next_block(DSDDecoder* decoder) {
    if (!decoder || !decoder->file) return 0;
    size_t bytes_read = fread(decoder->block_buffer, 1, decoder->block_buffer_size, decoder->file);
    decoder->current_dsd_bit_index = 0;
    return bytes_read > 0;
}

// フィルタ係数の初期化（簡易バターワースフィルタ）
static void init_filter_coeff(FilterCoeff2* coeff, double cutoff_freq, double sample_rate_dsd) {
    double omega = tan(M_PI * cutoff_freq / sample_rate_dsd);
    double denom = 1.0 + sqrt(2.0) * omega + omega * omega;
    coeff->a0 = omega * omega / denom;
    coeff->a1 = 2.0 * coeff->a0;
    coeff->a2 = coeff->a0;
    coeff->b1 = 2.0 * (omega * omega - 1.0) / denom;
    coeff->b2 = (1.0 - sqrt(2.0) * omega + omega * omega) / denom;
}

// 2次フィルタの適用
static double apply_filter2(FilterState2* state, FilterCoeff2* coeff, double input) {
    double output = coeff->a0 * input + coeff->a1 * state->x1 + coeff->a2 * state->x2
                    - coeff->b1 * state->y1 - coeff->b2 * state->y2;
    state->x2 = state->x1;
    state->x1 = input;
    state->y2 = state->y1;
    state->y1 = output;
    return output;
}

// Parses a DSDIFF (.dff) file. On success, leaves `file` positioned at the
// first byte of raw DSD audio data and fills in decoder->channels and
// decoder->sample_rate_dsd, plus *out_total_dsd_samples (per-channel sample
// count) and *out_data_size (raw audio byte count, all channels combined).
// Returns 0 on success, -1 if the file is malformed or uses compressed
// (DST) audio, which this decoder does not support.
static int dsd_parse_dff_header(DSDDecoder* decoder, FILE* file, uint64_t* out_total_dsd_samples, uint64_t* out_data_size) {
    uint8_t hdr[12];
    if (fread(hdr, 1, 12, file) != 12 || strncmp((char*)hdr, "FRM8", 4) != 0) return -1;
    uint64_t frm8_size = read_be64(hdr + 4);

    uint8_t form_type[4];
    if (fread(form_type, 1, 4, file) != 4 || strncmp((char*)form_type, "DSD ", 4) != 0) return -1;

    uint64_t end_offset = 12 + frm8_size; // absolute end of the FRM8 payload

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

                    if (strncmp((char*)shdr, "FS ", 3) == 0 && ssize >= 4) {
                        uint8_t buf4[4];
                        if (fread(buf4, 1, 4, file) == 4) { sample_rate = read_be32(buf4); got_fs = 1; }
                    } else if (strncmp((char*)shdr, "CHNL", 4) == 0 && ssize >= 2) {
                        uint8_t buf2[2];
                        if (fread(buf2, 1, 2, file) == 2) { channels = ((int)buf2[0] << 8) | (int)buf2[1]; got_chnl = 1; }
                    } else if (strncmp((char*)shdr, "CMPR", 4) == 0 && ssize >= 4) {
                        uint8_t buf4[4];
                        if (fread(buf4, 1, 4, file) == 4 && strncmp((char*)buf4, "DSD ", 4) != 0) {
                            return -1; // compressed (e.g. DST) audio not supported
                        }
                    }
                    uint64_t padded = ssize + (ssize & 1); // chunks pad to an even boundary
                    fseek(file, sub_data_start + (long)padded, SEEK_SET);
                }
            }
        } else if (strncmp((char*)chdr, "DSD ", 4) == 0) {
            data_offset = (uint64_t)chunk_data_start;
            data_size = size;
            got_data = 1;
            break; // raw audio found; any trailing metadata chunks are ignored
        } else if (strncmp((char*)chdr, "DST ", 4) == 0) {
            return -1; // compressed audio not supported
        }

        uint64_t padded = size + (size & 1);
        fseek(file, chunk_data_start + (long)padded, SEEK_SET);
    }

    if (!got_fs || !got_chnl || !got_data || channels < 1 || channels > MAX_CHANNELS || data_size == 0) return -1;

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

    // --- Header Parsing: sniff the magic to tell DSF (Sony) from DFF/DSDIFF (Philips) ---
    uint8_t magic[4];
    if (fread(magic, 1, 4, file) != 4) { free(decoder); return NULL; }
    fseek(file, 0, SEEK_SET);

    uint64_t total_dsd_samples = 0;

    if (strncmp((char*)magic, "FRM8", 4) == 0) {
        // --- DFF/DSDIFF ---
        decoder->file_type = DSD_FILE_DFF;
        uint64_t data_size = 0;
        if (dsd_parse_dff_header(decoder, file, &total_dsd_samples, &data_size) != 0) { free(decoder); return NULL; }
        // DSDIFF has no per-channel block layout of its own (channels are
        // byte-interleaved throughout), so we pick our own read-ahead
        // granularity (bytes per channel) for the decoder's block_buffer.
        decoder->block_size_bytes = 4096;
    } else if (strncmp((char*)magic, "DSD ", 4) == 0) {
        // --- DSF ---
        decoder->file_type = DSD_FILE_DSF;
        uint8_t header_buf[80];
        if (fread(header_buf, 1, 28, file) != 28 || strncmp((char*)header_buf, "DSD ", 4) != 0) { free(decoder); return NULL; }
        uint64_t fmt_chunk_offset = 28;
        fseek(file, fmt_chunk_offset, SEEK_SET);
        if (fread(header_buf, 1, 52, file) != 52 || strncmp((char*)header_buf, "fmt ", 4) != 0) { free(decoder); return NULL; }

        uint64_t fmt_chunk_size = read_le64(header_buf + 4);
        decoder->channels = read_le32(header_buf + 24);
        decoder->sample_rate_dsd = read_le32(header_buf + 28);
        total_dsd_samples = read_le64(header_buf + 36);
        decoder->block_size_bytes = read_le32(header_buf + 44);

        if (read_le32(header_buf + 32) != 1 || (decoder->channels < 1 || decoder->channels > MAX_CHANNELS) || decoder->block_size_bytes == 0) { free(decoder); return NULL; }

        fseek(file, fmt_chunk_offset + fmt_chunk_size, SEEK_SET);
        char chunk_id[12];
        if (fread(chunk_id, 1, 12, file) != 12 || strncmp(chunk_id, "data", 4) != 0) { free(decoder); return NULL; }
        // file position is now at the first byte of raw DSD audio data
    } else {
        free(decoder);
        return NULL;
    }

    // PCMサンプルレートの計算 (デシメーションファクターの調整を検討)
    decoder->sample_rate_pcm = dsd_choose_pcm_rate(decoder->sample_rate_dsd);
    if (decoder->sample_rate_pcm <= 0) { free(decoder); return NULL; }

    size_t decimation_factor = decoder->sample_rate_dsd / decoder->sample_rate_pcm;
    if (decimation_factor == 0) { free(decoder); return NULL; }
    decoder->totalPCMFrameCount = total_dsd_samples / decimation_factor;

    decoder->block_buffer_size = decoder->block_size_bytes * decoder->channels;
    decoder->block_buffer = (uint8_t*)malloc(decoder->block_buffer_size);
    if (!decoder->block_buffer) { free(decoder); return NULL; }

    // --- Initialize filter ---
    double cutoff_freq = decoder->sample_rate_pcm / 2.0; // PCMサンプリングレートの半分
    init_filter_coeff(&decoder->filter_coeff, cutoff_freq, decoder->sample_rate_dsd);
    // フィルタ状態の初期化
    memset(decoder->filter_state, 0, sizeof(decoder->filter_state));
    
    // 初期ブロックをロード
    dsd_load_next_block(decoder);

    // --- RMS推定の初期化 ---
    decoder->initial_rms_estimation_done = 0;
    decoder->current_scale_factor = 1.0; // デフォルトは1.0 (後で計算)
    decoder->current_file_pos = ftell(file); // 現在のファイル位置を保存
    // ----------------------

    return decoder;
}

void dsd_decoder_free(DSDDecoder* decoder) {
    if (decoder) {
        free(decoder->block_buffer);
        free(decoder);
    }
}

// RMS推定
static void dsd_decoder_estimate_rms(DSDDecoder* decoder, int format) {
    if (decoder->initial_rms_estimation_done) return;

    // ファイルポインタを先頭に戻す
    fseek(decoder->file, decoder->current_file_pos, SEEK_SET);
    // フィルタ状態もリセット
    memset(decoder->filter_state, 0, sizeof(decoder->filter_state));
    decoder->current_dsd_bit_index = 0;
    decoder->pcm_frames_processed = 0;
    dsd_load_next_block(decoder); // 最初のブロックを再ロード

    const int ESTIMATION_FRAMES = decoder->sample_rate_pcm * 2; // 2秒分のPCMフレームで推定
    // 推定用のバッファは、float型で確保すると計算が楽
    float* temp_pcm_buffer = (float*)malloc(ESTIMATION_FRAMES * decoder->channels * sizeof(float));
    if (!temp_pcm_buffer) {
        // メモリ確保失敗の場合は、デフォルトのスケーリング係数を使用
        decoder->current_scale_factor = 1.0; // もしくはより適切なデフォルト値
        decoder->initial_rms_estimation_done = 1;
        return;
    }

    size_t decimation_factor = decoder->sample_rate_dsd / decoder->sample_rate_pcm;
    size_t block_size_bits = decoder->block_size_bytes * DSD_SAMPLES_PER_BYTE;

    double sum_squares[MAX_CHANNELS] = {0.0};
    size_t actual_frames_processed = 0;

    for (size_t i = 0; i < ESTIMATION_FRAMES; ++i) {
        for (int ch = 0; ch < decoder->channels; ++ch) {
            double accum = 0.0;
            size_t start_bit = decoder->current_dsd_bit_index;

            for (size_t k = 0; k < decimation_factor; ++k) {
                size_t current_bit = start_bit + k;
                if (current_bit >= block_size_bits) {
                    // 推定中にファイルの終わりに来た場合
                    if (!dsd_load_next_block(decoder)) goto end_estimation_loop;
                    start_bit = 0;
                    current_bit = k;
                }
                double dsd_val = dsd_bit_value(decoder, ch, current_bit);

                double temp = dsd_val;
                for (int stage = 0; stage < 4; ++stage) {
                    temp = apply_filter2(&decoder->filter_state[ch][stage], &decoder->filter_coeff, temp);
                }
                accum += temp;
            }
            // ここではまだ最終的なスケーリングは行わない
            temp_pcm_buffer[i * decoder->channels + ch] = (float)(accum / decimation_factor);
            sum_squares[ch] += pow(temp_pcm_buffer[i * decoder->channels + ch], 2);
        }

        decoder->current_dsd_bit_index += decimation_factor;
        if (decoder->current_dsd_bit_index >= block_size_bits) {
            if (!dsd_load_next_block(decoder)) goto end_estimation_loop;
        }
        actual_frames_processed++;
    }

end_estimation_loop:
    {
      // RMS値を計算し、スケーリング係数を決定
      double average_rms_sq = 0.0;
      if (actual_frames_processed > 0) {
          for (int ch = 0; ch < decoder->channels; ++ch) {
              average_rms_sq += sum_squares[ch];
          }
          average_rms_sq /= (actual_frames_processed * decoder->channels);
      }
      double estimated_rms = sqrt(average_rms_sq);

      // 目標とするRMS値 (例: PCMフルスケールの-12dBFSに相当)
      // 0dBFS = 1.0, -6dBFS = 0.5, -12dBFS = 0.25
      const double TARGET_RMS = 0.25; // 経験的に良いとされる値

      if (estimated_rms > 1e-9) {
          decoder->current_scale_factor = TARGET_RMS / estimated_rms;

        if (decoder->current_scale_factor < 2.0) {
            decoder->current_scale_factor = 2.0;
        }
        if (decoder->current_scale_factor > 4.0) {
            decoder->current_scale_factor = 4.0;
        }
      } else {
          decoder->current_scale_factor = 1.0;
      }
    }

    free(temp_pcm_buffer);
    decoder->initial_rms_estimation_done = 1;

    // ファイルポインタを実際の再生開始位置に戻す
    fseek(decoder->file, decoder->current_file_pos, SEEK_SET);
    // フィルタ状態もリセット
    memset(decoder->filter_state, 0, sizeof(decoder->filter_state));
    decoder->current_dsd_bit_index = 0;
    decoder->pcm_frames_processed = 0;
    dsd_load_next_block(decoder); // 最初のブロックを再ロード
}

size_t dsd_decoder_read_pcm_frames(DSDDecoder* decoder, size_t frames_to_read, void* buffer, int format) {
    if (!decoder || !buffer || frames_to_read == 0 || decoder->pcm_frames_processed >= decoder->totalPCMFrameCount) return 0;

    // RMS推定がまだ行われていなければ実行
    if (!decoder->initial_rms_estimation_done) {
        dsd_decoder_estimate_rms(decoder, format);
    }

    size_t decimation_factor = decoder->sample_rate_dsd / decoder->sample_rate_pcm;
    if (decimation_factor == 0) return 0;

    size_t frames_read = 0;
    int16_t* buffer_s16 = (int16_t*)buffer;
    float* buffer_f32 = (float*)buffer;
    size_t block_size_bits = decoder->block_size_bytes * DSD_SAMPLES_PER_BYTE;

    double filtered[MAX_CHANNELS]; // 各チャンネルのフィルタ出力
    for (size_t i = 0; i < frames_to_read; ++i) {
        for (int ch = 0; ch < decoder->channels; ++ch) {
            size_t start_bit = decoder->current_dsd_bit_index;

            double accum = 0.0;
            for (size_t k = 0; k < decimation_factor; ++k) {
                size_t current_bit = start_bit + k;
                if (current_bit >= block_size_bits) {
                    if (!dsd_load_next_block(decoder)) goto end_loop;
                    start_bit = 0;
                    current_bit = k;
                }
                // Bit order/layout (LSB-first blocked for DSF, MSB-first
                // interleaved for DFF) is handled inside dsd_bit_value().
                double dsd_val = dsd_bit_value(decoder, ch, current_bit);

                // RMS推定時と同様に、1ビットごとにローパスフィルタを通してから平均化(間引き)する
                double temp = dsd_val;
                for (int stage = 0; stage < 4; ++stage) {
                    temp = apply_filter2(&decoder->filter_state[ch][stage], &decoder->filter_coeff, temp);
                }
                accum += temp;
            }

            filtered[ch] = (accum / decimation_factor) * decoder->current_scale_factor;
        }

        // PCMバッファに書き込み
        for (int ch = 0; ch < decoder->channels; ++ch) {
            double pcm_val = filtered[ch];
            if (format == SND_PCM_FORMAT_FLOAT_LE) {
                if (pcm_val > 1.0) pcm_val = 1.0;
                if (pcm_val < -1.0) pcm_val = -1.0;
                buffer_f32[i * decoder->channels + ch] = (float)pcm_val;
            } else {
                int32_t s16_val = (int32_t)(pcm_val * 32767.0);
                if (s16_val > 32767) s16_val = 32767;
                if (s16_val < -32768) s16_val = -32768;
                buffer_s16[i * decoder->channels + ch] = (int16_t)s16_val;
            }
        }

        decoder->current_dsd_bit_index += decimation_factor;
        if (decoder->current_dsd_bit_index >= block_size_bits) {
            if (!dsd_load_next_block(decoder)) {
                frames_read++;
                goto end_loop;
            }
        }

        decoder->pcm_frames_processed++;
        frames_read++;
        if (decoder->pcm_frames_processed >= decoder->totalPCMFrameCount) goto end_loop;
    }

end_loop:
    return frames_read;
}

#endif // DSD_DECODER_IMPLEMENTATION
#endif // DSD_H
