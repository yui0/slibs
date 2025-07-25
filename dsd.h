/*
#include <stdio.h>
#include "dsd_decoder.h"

int main() {
    #define DSD_DECODER_IMPLEMENTATION
    #include "dsd_decoder.h"

    // DSFファイルの読み込み（例: ファイルから）
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

    // DSFデータロード
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

    // PCMデータ処理（例: ファイルに保存）
    FILE* out = fopen("output.pcm", "wb");
    fwrite(pcm_data, sizeof(int32_t), pcm_size, out);
    fclose(out);

    // 解放
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

DSDDecoder* dsd_decoder_init_file(FILE* file) {
    if (!file) return NULL;
    DSDDecoder* decoder = (DSDDecoder*)calloc(1, sizeof(DSDDecoder));
    if (!decoder) return NULL;
    decoder->file = file;

    // --- Header Parsing ---
    uint8_t header_buf[80];
    if (fread(header_buf, 1, 28, file) != 28 || strncmp((char*)header_buf, "DSD ", 4) != 0) { free(decoder); return NULL; }
    uint64_t fmt_chunk_offset = 28;
    fseek(file, fmt_chunk_offset, SEEK_SET);
    if (fread(header_buf, 1, 52, file) != 52 || strncmp((char*)header_buf, "fmt ", 4) != 0) { free(decoder); return NULL; }

    uint64_t fmt_chunk_size = read_le64(header_buf + 4);
    decoder->channels = read_le32(header_buf + 24);
    decoder->sample_rate_dsd = read_le32(header_buf + 28);
    uint64_t total_dsd_samples = read_le64(header_buf + 36);
    decoder->block_size_bytes = read_le32(header_buf + 44);

    if (read_le32(header_buf + 32) != 1 || (decoder->channels < 1 || decoder->channels > MAX_CHANNELS) || decoder->block_size_bytes == 0) { free(decoder); return NULL; }
    
    // PCMサンプルレートの計算 (デシメーションファクターの調整を検討)
    /*if (decoder->sample_rate_dsd == 2822400) decoder->sample_rate_pcm = 88200; // DSD64 -> PCM 88.2kHz (factor 32)
    else if (decoder->sample_rate_dsd == 5644800) decoder->sample_rate_pcm = 176400; // DSD128 -> PCM 176.4kHz (factor 32)
    else if (decoder->sample_rate_dsd == 11289600) decoder->sample_rate_pcm = 352800; // DSD256 -> PCM 352.8kHz (factor 32)
    else decoder->sample_rate_pcm = decoder->sample_rate_dsd / 32; // Fallback, adjust if needed*/
    if (decoder->sample_rate_dsd == 2822400) decoder->sample_rate_pcm = 176400;
    else if (decoder->sample_rate_dsd == 5644800) decoder->sample_rate_pcm = 176400;
    else if (decoder->sample_rate_dsd == 11289600) decoder->sample_rate_pcm = 176400;
    else decoder->sample_rate_pcm = decoder->sample_rate_dsd / 32; // Fallback, adjust if needed

    size_t decimation_factor = decoder->sample_rate_dsd / decoder->sample_rate_pcm;
    decoder->totalPCMFrameCount = total_dsd_samples / decimation_factor;

    decoder->block_buffer_size = decoder->block_size_bytes * decoder->channels;
    decoder->block_buffer = (uint8_t*)malloc(decoder->block_buffer_size);
    if (!decoder->block_buffer) { free(decoder); return NULL; }

    fseek(file, fmt_chunk_offset + fmt_chunk_size, SEEK_SET);
    char chunk_id[12];
    if (fread(chunk_id, 1, 12, file) != 12 || strncmp(chunk_id, "data", 4) != 0) { free(decoder->block_buffer); free(decoder); return NULL; }
    fseek(file, ftell(file) - 12 + 12, SEEK_SET); // dataチャンクの先頭へ

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

// RMS推定を実行する新しいヘルパー関数
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
            const uint8_t* dsd_channel_data = decoder->block_buffer + (ch * decoder->block_size_bytes);
            double accum = 0.0;
            size_t start_bit = decoder->current_dsd_bit_index;

            for (size_t k = 0; k < decimation_factor; ++k) {
                size_t current_bit = start_bit + k;
                if (current_bit >= block_size_bits) {
                    // 推定中にファイルの終わりに来た場合
                    if (!dsd_load_next_block(decoder)) goto end_estimation_loop;
                    dsd_channel_data = decoder->block_buffer + (ch * decoder->block_size_bytes);
                    start_bit = 0;
                    current_bit = k;
                }
                size_t byte_idx = current_bit / DSD_SAMPLES_PER_BYTE;
                int bit_pos = 7 - (current_bit % DSD_SAMPLES_PER_BYTE);
                double dsd_val = ((dsd_channel_data[byte_idx] >> bit_pos) & 1) ? 1.0 : -1.0;

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

    double scale = decoder->current_scale_factor / decimation_factor;
    double filtered[MAX_CHANNELS]; // 各チャンネルのフィルタ出力
    for (size_t i = 0; i < frames_to_read; ++i) {
        for (int ch = 0; ch < decoder->channels; ++ch) {
            const uint8_t* dsd_channel_data = decoder->block_buffer + (ch * decoder->block_size_bytes);
            size_t start_bit = decoder->current_dsd_bit_index;

            int sum = 0;
            for (size_t k = 0; k < decimation_factor; ++k) {
                size_t current_bit = start_bit + k;
                if (current_bit >= block_size_bits) {
                    if (!dsd_load_next_block(decoder)) goto end_loop;
                    dsd_channel_data = decoder->block_buffer + (ch * decoder->block_size_bytes);
                    start_bit = 0;
                    current_bit = k;
                }
                size_t byte_idx = current_bit / DSD_SAMPLES_PER_BYTE;
                int bit_pos = 7 - (current_bit % DSD_SAMPLES_PER_BYTE);

                sum += ((dsd_channel_data[byte_idx] >> bit_pos) & 1) ? 1 : 0;
            }

            //filtered[ch] = ((double)sum / decimation_factor)*2.0-1.0;
            filtered[ch] = ((double)sum / decimation_factor)*decoder->current_scale_factor-decoder->current_scale_factor/2;
        }

        // PCMバッファに書き込み
        for (int ch = 0; ch < decoder->channels; ++ch) {
                /*double pcm_val = filtered[ch];
                for (int stage = 0; stage < 4; ++stage) {
                    pcm_val = apply_filter2(&decoder->filter_state[ch][stage], &decoder->filter_coeff, pcm_val);
                }*/
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
