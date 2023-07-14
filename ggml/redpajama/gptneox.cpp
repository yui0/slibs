// Defines fileno on msys:
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <cstdint>
#include <cstdio>
#endif

#include "gptneox-util.h"
#include "gptneox.h"

#include "../ggml.h"

#include <array>
#include <ctime>
#include <cinttypes>
#include <fstream>
#include <random>
#include <map>
#include <unordered_map>
#include <queue>
#include <cassert>
#include <cstring>
#include <climits>
#include <memory>
#include <algorithm>
#include <initializer_list>
#include <thread>
#include <atomic>
#include <mutex>
#include <sstream>

// TODO: Add back in n_ctx (max_position_embeddings) to ggml model, it is currently hard-coded to 2048 max for llama

#define GPTNEOX_USE_SCRATCH
#define GPTNEOX_MAX_SCRATCH_BUFFERS 16


enum e_model {
    MODEL_UNKNOWN,
    MODEL_3B, // StabilityAI Base Alpha 3B
    MODEL_7B,
    MODEL_12B,
    MODEL_20B,
};

static const size_t MiB = 1024*1024;

// computed for n_ctx == 2048
// TODO: dynamically determine these sizes
// TODO: To load the stablelm 3B model on my test XR will require some tricks, small ggml context size, mmap support, among others, but is maybe feasible, is a smaller n_ctx required? 512 instead of 2048/4096? Does mmap work as desired on iOS?
//       needs modifications in ggml

// TODO: Modify for gptneox, how are these values actually determined?
// TODO: This is now priority, 
static const std::map<e_model, size_t> & MEM_REQ_SCRATCH0()
{
    static std::map<e_model, size_t> _MEM_REQ_SCRATCH0 = {
        { MODEL_3B,    128ull * MiB },
        { MODEL_7B,    512ull * MiB },
        { MODEL_12B,   512ull * MiB },
        { MODEL_20B,   512ull * MiB },
    };
    return _MEM_REQ_SCRATCH0;
}

// TODO: Modify for gptneox, how are these values actually determined?
static const std::map<e_model, size_t> & MEM_REQ_SCRATCH1()
{
    static std::map<e_model, size_t> _MEM_REQ_SCRATCH1 = {
        { MODEL_3B,    128ull * MiB },
        { MODEL_7B,    512ull * MiB },
        { MODEL_12B,   512ull * MiB },
        { MODEL_20B,   512ull * MiB },
    };
    return _MEM_REQ_SCRATCH1;
}

// TODO: Modify for gptneox, how are these values actually determined?
// 2*n_embd*n_ctx*n_layer*sizeof(float16)
// llama 7B: 2 * 768 * 32 * 2 = 98304
static const std::map<e_model, size_t> & MEM_REQ_KV_SELF()
{
    static std::map<e_model, size_t> _MEM_REQ_KV_SELF = {
        { MODEL_3B,   512ull * MiB },
        { MODEL_7B,   1026ull * MiB },
        { MODEL_12B,  1608ull * MiB },
        { MODEL_20B,  1608ull * MiB },
    };
    return _MEM_REQ_KV_SELF;
}

// TODO: Modify for gptneox, how are these values actually determined?
// this is mostly needed for temporary mul_mat buffers to dequantize the data
// not actually needed if BLAS is disabled
static const std::map<e_model, size_t> & MEM_REQ_EVAL()
{
    static std::map<e_model, size_t> _MEM_REQ_EVAL = {
        { MODEL_3B,   512ull * MiB },
        { MODEL_7B,   768ull * MiB },
        { MODEL_12B, 1024ull * MiB },
        { MODEL_20B, 1024ull * MiB },
    };
    return _MEM_REQ_EVAL;
}

// default hparams (GPT-NeoX oasst 12B)
struct gptneox_hparams {
    uint32_t n_vocab = 50288;
    uint32_t n_ctx   = 4096;   // this is provided as user input?
    uint32_t n_embd  = 5120;
    uint32_t n_head  = 40;
    uint32_t n_layer = 36;
    uint32_t n_rot   = 32;
    uint32_t use_parallel_residual = 1; // 1 = true, 0 = false
    enum gptneox_ftype ftype = GPTNEOX_FTYPE_MOSTLY_F16;

    bool operator!=(const gptneox_hparams & other) const {
        return memcmp(this, &other, sizeof(gptneox_hparams));
    }
};

struct gptneox_layer {
    // input_layernorm
    struct ggml_tensor * ln_attn_g;
    struct ggml_tensor * ln_attn_b;

    // post_attention_layernorm
    struct ggml_tensor * ln_ff_g;
    struct ggml_tensor * ln_ff_b;

    // attention
    struct ggml_tensor * c_attn_attn_w;

    struct ggml_tensor * c_attn_attn_b;

    struct ggml_tensor * c_attn_proj_w;
    struct ggml_tensor * c_attn_proj_b;

    // ff
    struct ggml_tensor * c_mlp_fc_w;
    struct ggml_tensor * c_mlp_fc_b;

    struct ggml_tensor * c_mlp_proj_w;
    struct ggml_tensor * c_mlp_proj_b;
};

struct gptneox_kv_cache {
    struct ggml_tensor * k;
    struct ggml_tensor * v;

    struct ggml_context * ctx = NULL;

    gptneox_buffer buf;

    int n; // number of tokens currently in the cache

    ~gptneox_kv_cache() {
        if (ctx) {
            ggml_free(ctx);
        }
    }
};

struct gptneox_model {
    e_model type = MODEL_UNKNOWN;

    gptneox_hparams hparams;

    // final normalization
    struct ggml_tensor * ln_f_g;
    struct ggml_tensor * ln_f_b;

    // word embedding
    struct ggml_tensor * wte;

    // language model head
    struct ggml_tensor * lmh_g;

    std::vector<gptneox_layer> layers;

    // context
    struct ggml_context * ctx = NULL;

    // key + value cache for the self attention
    // TODO: move to gptneox_state
    struct gptneox_kv_cache kv_self;

    // the model memory buffer
    gptneox_buffer buf;

    // model memory mapped file
    std::unique_ptr<gptneox_mmap> mapping;

    // objects representing data potentially being locked in memory
    gptneox_mlock mlock_buf;
    gptneox_mlock mlock_mmap;

    // for quantize-stats only
    std::vector<std::pair<std::string, struct ggml_tensor *>> tensors_by_name;

    ~gptneox_model() {
        if (ctx) {
            ggml_free(ctx);
        }
    }
};

struct gptneox_vocab {
    using id    = int32_t;
    using token = std::string;

    struct token_score {
        token tok;
        float score;
    };

    std::unordered_map<token, id> token_to_id;
    std::vector<token_score> id_to_token;
};

struct gptneox_context {
    std::mt19937 rng;

    int64_t t_load_us = 0;
    int64_t t_start_us = 0;
    bool has_evaluated_once = false;

    int64_t t_sample_us = 0;
    int64_t t_eval_us   = 0;
    int64_t t_p_eval_us = 0;

    int32_t n_sample = 0; // number of tokens sampled
    int32_t n_eval   = 0; // number of eval calls
    int32_t n_p_eval = 0; // number of tokens in eval calls for the prompt (with batch size > 1)

    gptneox_model model;
    gptneox_vocab vocab;

    size_t mem_per_token = 0;

    // decode output (2-dimensional array: [n_tokens][n_vocab])
    std::vector<float> logits;
    bool logits_all = false;

    // input embedding (1-dimensional array: [n_embd])
    std::vector<float> embedding;

    // memory buffers used to evaluate the model
    // TODO: move in gptneox_state
    gptneox_buffer buf_compute;
    gptneox_buffer buf_scratch[GPTNEOX_MAX_SCRATCH_BUFFERS];

    int    buf_last = 0;
    size_t buf_max_size[GPTNEOX_MAX_SCRATCH_BUFFERS] = { 0 };

    void use_buf(struct ggml_context * ctx, int i) {
#if defined(GPTNEOX_USE_SCRATCH)
        size_t last_size = 0;

        if (i == -1) {
            last_size = ggml_set_scratch(ctx, { 0, 0, nullptr, });
        } else {
            auto & buf = buf_scratch[i];
            last_size = ggml_set_scratch(ctx, { 0, buf.size, buf.addr, });
        }

        if (buf_last >= 0) {
            buf_max_size[buf_last] = std::max(buf_max_size[buf_last], last_size);
        }

        buf_last = i;
#else
        (void) i;
        (void) ctx;
#endif
    }

    size_t get_buf_max_mem(int i) const {
#if defined(GPTNEOX_USE_SCRATCH)
        return buf_max_size[i];
#else
        (void) i;
        return 0;
#endif
    }
};

template <typename T>
static T checked_mul(T a, T b) {
    T ret = a * b;
    if (a != 0 && ret / a != b) {
        throw format("overflow multiplying %llu * %llu",
                     (unsigned long long) a, (unsigned long long) b);
    }
    return ret;
}

static size_t checked_div(size_t a, size_t b) {
    if (b == 0 || a % b != 0) {
        throw format("error dividing %zu / %zu", a, b);
    }
    return a / b;
}

static std::string gptneox_format_tensor_shape(const std::vector<uint32_t> & ne) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%5u", ne.at(0));
    for (size_t i = 1; i < ne.size(); i++) {
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " x %5u", ne.at(i));
    }
    return buf;
}

static size_t gptneox_calc_tensor_size(const std::vector<uint32_t> & ne, enum ggml_type type) {
    size_t size = ggml_type_size(type);
    for (uint32_t dim : ne) {
        size = checked_mul<size_t>(size, dim);
    }
    return size / ggml_blck_size(type);
}

struct gptneox_load_tensor_shard {
    std::vector<uint32_t> ne;
    size_t size;
    enum ggml_type type;
    size_t file_idx;
    size_t file_off;

    void calc_size() {
        size = gptneox_calc_tensor_size(ne, type);
    }
};

enum gptneox_split_type {
    SPLIT_NONE,
    SPLIT_BY_COLUMNS,
    SPLIT_BY_ROWS
};

struct gptneox_load_tensor {
    std::vector<gptneox_load_tensor_shard> shards;

    std::string name;
    enum ggml_type type = GGML_TYPE_F32;
    gptneox_split_type split_type = SPLIT_NONE;
    std::vector<uint32_t> ne;
    size_t size;
    struct ggml_tensor * ggml_tensor = NULL;
    uint8_t * data;

    gptneox_load_tensor(const std::string & name) : name(name) {}

    void calc_all() {
        calc_type();
        calc_split_type();
        calc_ne();
        calc_size();
    }

    void calc_type() {
        const auto & first_shard = shards.at(0);
        for (const auto & shard : shards) {
            if (shard.type != first_shard.type) {
                throw format("inconsistent tensor shard type in '%s'", name.c_str());
            }
        }
        type = first_shard.type;
    }

    void calc_split_type() {
        if (shards.at(0).ne.size() == 1 || // 1D tensors are just duplicated in every file
            shards.size() == 1) { // only one file?
            split_type = SPLIT_NONE;
        } else if (name.find("tok_embeddings.") == 0 ||
            name.find(".attention.wo.weight") != std::string::npos ||
            name.find(".feed_forward.w2.weight") != std::string::npos) {
            split_type = SPLIT_BY_COLUMNS;
        } else {
            split_type = SPLIT_BY_ROWS;
        }
    }

    void calc_ne() {
        const auto & first_shard = shards.at(0);
        for (const auto & shard : shards) {
            if (shard.ne != first_shard.ne) {
                throw format("inconsistent tensor shard shape in '%s': first was %s, other was %s",
                             name.c_str(), gptneox_format_tensor_shape(first_shard.ne).c_str(), gptneox_format_tensor_shape(shard.ne).c_str());
            }
        }
        ne = first_shard.ne;
        GPTNEOX_ASSERT(shards.size() <= UINT32_MAX);
        uint32_t n_shards = (uint32_t) shards.size();
        switch (split_type) {
            case SPLIT_NONE:
                ne = first_shard.ne;
                break;
            case SPLIT_BY_COLUMNS:
                ne = {checked_mul<uint32_t>(first_shard.ne[0], n_shards),
                      first_shard.ne[1]};
                break;
            case SPLIT_BY_ROWS:
                ne = {first_shard.ne[0],
                      checked_mul<uint32_t>(first_shard.ne[1], n_shards)};
                break;
        }
    }

    void calc_size() {
        size = gptneox_calc_tensor_size(ne, type);
    }
};

struct gptneox_load_tensors_map {
    // tensors is kept in a separate vector to preserve file order
    std::vector<gptneox_load_tensor> tensors;
    std::unordered_map<std::string, size_t> name_to_idx;
};

enum gptneox_file_version {
    GPTNEOX_FILE_VERSION_GGML,
    GPTNEOX_FILE_VERSION_GGMF_V1, // added version field and scores in vocab
    GPTNEOX_FILE_VERSION_GGJT_V1, // added padding
};

struct gptneox_file_loader {
    gptneox_file file;
    gptneox_file_version file_version;
    gptneox_hparams hparams;
    gptneox_vocab vocab;

    gptneox_file_loader(const char * fname, size_t file_idx, gptneox_load_tensors_map & tensors_map)
        : file(fname, "rb") {
        fprintf(stderr, "gptneox.cpp: loading model from %s\n", fname);
        read_magic();
        read_hparams();
        read_vocab();
        read_tensor_metadata(file_idx, tensors_map);
    }
    void read_magic() {
        uint32_t magic = file.read_u32();
        uint32_t version = 0;

        if (magic != 'ggml') {
            version = file.read_u32();
        }

        if (magic == 'ggml' && version == 0) {
            file_version = GPTNEOX_FILE_VERSION_GGML;
        } else if (magic == 'ggmf' && version == 1) {
            file_version = GPTNEOX_FILE_VERSION_GGMF_V1;
        } else if (magic == 'ggjt' && version == 1) {
            file_version = GPTNEOX_FILE_VERSION_GGJT_V1;
        } else {
            throw format("unknown (magic, version) combination: %08x, %08x; is this really a GGML file?",
                         magic, version);
        }
    }
    void read_hparams() {
        hparams.n_vocab = file.read_u32();
        hparams.n_ctx = file.read_u32();
        hparams.n_embd = file.read_u32();
        hparams.n_head = file.read_u32();
        hparams.n_layer = file.read_u32();
        hparams.n_rot = file.read_u32();
        hparams.use_parallel_residual = file.read_u32();
        hparams.ftype = (enum gptneox_ftype) file.read_u32();
    }
    void read_vocab() {
        vocab.id_to_token.resize(hparams.n_vocab);

        for (uint32_t i = 0; i < hparams.n_vocab; i++) {
            uint32_t len = file.read_u32();
            std::string word = file.read_string(len);

            float score = 0.0f;
            // TODO: Implement scores in gptneox
            /*if (file_version >= GPTNEOX_FILE_VERSION_GGMF_V1) {
                file.read_raw(&score, sizeof(score));
            }*/

            vocab.token_to_id[word] = i;

            auto & tok_score = vocab.id_to_token[i];
            tok_score.tok = std::move(word);
            tok_score.score = score;
        }
    }
    void read_tensor_metadata(size_t file_idx, gptneox_load_tensors_map & tensors_map) {
        while (file.tell() < file.size) {
            gptneox_load_tensor_shard shard;
            uint32_t n_dims = file.read_u32();
            uint32_t name_len = file.read_u32();
            shard.type = (enum ggml_type) file.read_u32();
            shard.ne.resize(n_dims);
            file.read_raw(shard.ne.data(), sizeof(shard.ne[0]) * n_dims);
            std::string name = file.read_string(name_len);
            if (n_dims < 1 || n_dims > 2) {
                throw format("gptneox.cpp: tensor '%s' should not be %u-dimensional", name.c_str(), n_dims);
            }
            switch (shard.type) {
                case GGML_TYPE_F32:
                case GGML_TYPE_F16:
                case GGML_TYPE_Q4_0:
                case GGML_TYPE_Q4_1:
                //case GGML_TYPE_Q4_2:
                case GGML_TYPE_Q5_0:
                case GGML_TYPE_Q5_1:
                case GGML_TYPE_Q8_0:
                    break;
                default: {
                    throw format("unrecognized tensor type %u\n", shard.type);
                }
            }

            if (file_version >= GPTNEOX_FILE_VERSION_GGJT_V1) {
                // skip to the next multiple of 32 bytes
                file.seek(-file.tell() & 31, SEEK_CUR);
            }
            shard.file_idx = file_idx;
            shard.file_off = file.tell();

            shard.calc_size();
            file.seek(shard.size, SEEK_CUR);

            auto it = tensors_map.name_to_idx.find(name);
            size_t idx;
            if (it != tensors_map.name_to_idx.end()) {
                idx = it->second;
            } else {
                tensors_map.tensors.emplace_back(name);
                idx = tensors_map.tensors.size() - 1;
                tensors_map.name_to_idx.emplace(name, idx);
            }
            tensors_map.tensors.at(idx).shards.push_back(shard);
        }
    }
};

struct gptneox_file_saver {
    gptneox_file file;
    gptneox_file_loader * any_file_loader;
    gptneox_file_saver(const char * fname, gptneox_file_loader * any_file_loader, enum gptneox_ftype new_ftype)
        : file(fname, "wb"), any_file_loader(any_file_loader) {
        fprintf(stderr, "gptneox.cpp: saving model to %s\n", fname);
        write_magic();
        write_hparams(new_ftype);
        write_vocab();
    }
    void write_magic() {
        file.write_u32('ggjt'); // magic
        file.write_u32(1); // version
    }
    void write_hparams(enum gptneox_ftype new_ftype) {
        const gptneox_hparams & hparams = any_file_loader->hparams;
        file.write_u32(hparams.n_vocab);
        file.write_u32(hparams.n_ctx);
        file.write_u32(hparams.n_embd);
        file.write_u32(hparams.n_head);
        file.write_u32(hparams.n_layer);
        file.write_u32(hparams.n_rot);
        file.write_u32(hparams.use_parallel_residual);
        file.write_u32(new_ftype);
    }
    void write_vocab() {
        if (any_file_loader->file_version == GPTNEOX_FILE_VERSION_GGML) {
            fprintf(stderr, "gptneox.cpp: WARNING: input is an old file that doesn't have scores; will add dummy scores\n");
        }
        uint32_t n_vocab = any_file_loader->hparams.n_vocab;
        for (uint32_t i = 0; i < n_vocab; i++) {
            const auto & token_score = any_file_loader->vocab.id_to_token.at(i);
            file.write_u32((uint32_t) token_score.tok.size());
            file.write_raw(token_score.tok.data(), token_score.tok.size());
            // TODO: Implement scores in gptneox?
            //file.write_raw(&token_score.score, sizeof(token_score.score));
        }
    }
    void write_tensor(gptneox_load_tensor & tensor, enum ggml_type new_type, const void * new_data, size_t new_size) {
        switch (new_type) {
            case GGML_TYPE_F32:
            case GGML_TYPE_F16:
            case GGML_TYPE_Q4_0:
            case GGML_TYPE_Q4_1:
            //case GGML_TYPE_Q4_2:
            case GGML_TYPE_Q5_0:
            case GGML_TYPE_Q5_1:
            case GGML_TYPE_Q8_0:
                break;
            default: GPTNEOX_ASSERT(false);
        }
        file.write_u32((uint32_t) tensor.ne.size());
        file.write_u32((uint32_t) tensor.name.size());
        file.write_u32(new_type);
        file.write_raw(tensor.ne.data(), sizeof(tensor.ne[0]) * tensor.ne.size());
        file.write_raw(tensor.name.data(), tensor.name.size());
        file.seek(-file.tell() & 31, SEEK_CUR);
        GPTNEOX_ASSERT(new_size == gptneox_calc_tensor_size(tensor.ne, new_type));
        file.write_raw(new_data, new_size);
    }
};

struct gptneox_model_loader {
    std::vector<std::unique_ptr<gptneox_file_loader>> file_loaders;
    gptneox_load_tensors_map tensors_map;
    bool use_mmap;
    size_t num_ggml_tensors_created = 0;
    struct ggml_context * ggml_ctx = NULL;
    std::unique_ptr<gptneox_mmap> mapping;

    gptneox_model_loader(const std::string & fname_base, bool use_mmap, bool vocab_only) {
        auto first_file = new gptneox_file_loader(fname_base.c_str(), 0, tensors_map);
        file_loaders.emplace_back(first_file);
        uint32_t n_parts = vocab_only ? 1 : guess_n_parts();
        for (uint32_t i = 1; i < n_parts; i++) {
            std::string fname = fname_base + "." + std::to_string(i);
            auto ith_file = new gptneox_file_loader(fname.c_str(), i, tensors_map);
            file_loaders.emplace_back(ith_file);
            if (ith_file->hparams != first_file->hparams) {
                throw format("gptneox.cpp: hparams inconsistent between files");
            }
        }
        if (!gptneox_mmap::SUPPORTED) {
            use_mmap = false;
        }
        if (use_mmap && alignment_prevents_mmap()) {
            fprintf(stderr, "gptneox.cpp: can't use mmap because tensors are not aligned; convert to new format to avoid this\n");
            use_mmap = false;
        }
        this->use_mmap = use_mmap;
        for (gptneox_load_tensor & lt : tensors_map.tensors) {
            lt.calc_all();
        }
    }

    bool alignment_prevents_mmap() {
        for (const gptneox_load_tensor & lt : tensors_map.tensors) {
            for (const gptneox_load_tensor_shard & shard : lt.shards) {
                if (shard.file_off & 3) {
                    return true;
                }
            }
        }
        return false;
    }

    uint32_t guess_n_parts() const {
        auto it = tensors_map.name_to_idx.find("gpt_neox.embed_in.weight");
        if (it == tensors_map.name_to_idx.end()) {
            throw std::string("missing gpt_neox.embed_in.weight");
        }
        const gptneox_load_tensor & lt = tensors_map.tensors.at(it->second);
        return file_loaders.at(0)->hparams.n_embd / lt.shards.at(0).ne.at(0);
    }

    void calc_sizes(size_t * ctx_size_p, size_t * mmapped_size_p) const {
        *ctx_size_p = *mmapped_size_p = 0;
        for (const gptneox_load_tensor & lt : tensors_map.tensors) {
            *ctx_size_p += sizeof(struct ggml_tensor) + GGML_OBJECT_SIZE;
            *(use_mmap ? mmapped_size_p : ctx_size_p) += lt.size;
        }
    }

    struct ggml_tensor * get_tensor(const std::string & name, std::vector<uint32_t> ne) {
        auto it = tensors_map.name_to_idx.find(name);
        if (it == tensors_map.name_to_idx.end()) {
            throw format("gptneox.cpp: tensor '%s' is missing from model", name.c_str());
        }
        gptneox_load_tensor & lt = tensors_map.tensors.at(it->second);
        if (lt.ne != ne) {
            throw format("gptneox.cpp: tensor '%s' has wrong shape; expected %s, got %s",
                         name.c_str(), gptneox_format_tensor_shape(ne).c_str(), gptneox_format_tensor_shape(lt.ne).c_str());
        }

        return get_tensor_for(lt);
    }

    struct ggml_tensor * get_tensor_for(gptneox_load_tensor & lt) {
        struct ggml_tensor * tensor;
        if (lt.ne.size() == 2) {
            tensor = ggml_new_tensor_2d(ggml_ctx, lt.type, lt.ne.at(0), lt.ne.at(1));
        } else {
            GPTNEOX_ASSERT(lt.ne.size() == 1);
            tensor = ggml_new_tensor_1d(ggml_ctx, lt.type, lt.ne.at(0));
        }
        GPTNEOX_ASSERT(lt.ggml_tensor == NULL); // if this fails, we called get_tensor twice on the same tensor
        lt.ggml_tensor = tensor;
        num_ggml_tensors_created++;
        return tensor;
    }

    void done_getting_tensors() {
        if (num_ggml_tensors_created != tensors_map.tensors.size()) {
            throw std::string("gptneox.cpp: file contained more tensors than expected");
        }
    }

    void load_all_data(gptneox_progress_callback progress_callback, void *  progress_callback_user_data, gptneox_mlock * lmlock) {
        size_t data_size = 0;
        for (const gptneox_load_tensor & lt : tensors_map.tensors) {
            data_size += lt.size;
        }

        if (use_mmap) {
            mapping.reset(new gptneox_mmap(&file_loaders.at(0)->file));
            if (!lmlock) {
                // Don't call the callback since the actual loading will be lazy
                // and we can't measure it.
                progress_callback = NULL;
            }
            if (lmlock) {
                lmlock->init(mapping->addr);
            }
        }

        size_t done_size = 0;
        for (gptneox_load_tensor & lt : tensors_map.tensors) {
            if (progress_callback) {
                progress_callback((float) done_size / data_size, progress_callback_user_data);
            }
            GPTNEOX_ASSERT(lt.ggml_tensor); // unused tensors should have been caught by load_data already
            lt.data = (uint8_t *) lt.ggml_tensor->data;
            load_data_for(lt);
            lt.ggml_tensor->data = lt.data;
            done_size += lt.size;
            if (use_mmap && lmlock) {
                lmlock->grow_to(done_size);
            }
        }
        if (progress_callback) {
            progress_callback(1.0f, progress_callback_user_data);
        }
    }

    void load_data_for(gptneox_load_tensor & lt) {
        if (use_mmap) {
            GPTNEOX_ASSERT(lt.shards.size() == 1);
            lt.data = (uint8_t *) mapping->addr + lt.shards.at(0).file_off;
        } else if (lt.split_type == SPLIT_NONE) {
            gptneox_file & file = file_loaders.at(lt.shards.at(0).file_idx)->file;
            file.seek(lt.shards.at(0).file_off, SEEK_SET);
            file.read_raw(lt.data, lt.size);
        } else if (lt.split_type == SPLIT_BY_ROWS) {
            size_t offset = 0;
            for (gptneox_load_tensor_shard & shard : lt.shards) {
                gptneox_file & file = file_loaders.at(shard.file_idx)->file;
                file.seek(shard.file_off, SEEK_SET);
                file.read_raw(lt.data + offset, shard.size);
                offset += shard.size;
            }
            GPTNEOX_ASSERT(offset == lt.size);
        } else if (lt.split_type == SPLIT_BY_COLUMNS) {
            // Let's load the data into temporary buffers to ensure the OS performs large loads.
            std::vector<gptneox_buffer> tmp_bufs;
            tmp_bufs.resize(lt.shards.size());
            for (size_t i = 0; i < lt.shards.size(); i++) {
                gptneox_load_tensor_shard & shard = lt.shards.at(i);
                gptneox_file & file = file_loaders.at(shard.file_idx)->file;
                file.seek(shard.file_off, SEEK_SET);
                tmp_bufs.at(i).resize(shard.size);
                file.read_raw(tmp_bufs.at(i).addr, shard.size);
            }
            // Then reshape.
            size_t num_rows = lt.ne.at(1);
            size_t per_shard_row_size = lt.shards.at(0).size / num_rows;
            size_t out_offset = 0;
            for (size_t row = 0; row < num_rows; row++) {
                for (gptneox_buffer & tmp_buf : tmp_bufs) {
                    memcpy(lt.data + out_offset,
                           tmp_buf.addr + row * per_shard_row_size,
                           per_shard_row_size);
                    out_offset += per_shard_row_size;
                }
            }
            GPTNEOX_ASSERT(out_offset == lt.size);
        }
        if (0) {
            print_checksum(lt);
        }
    }

    static void print_checksum(gptneox_load_tensor & lt) {
        uint32_t sum = 0;
        for (size_t i = 0; i < lt.size; i++) {
            uint8_t byte = lt.data[i];
            sum = byte + (sum << 6) + (sum << 16) - sum; // sdbm hash
        }
        fprintf(stderr, "%s checksum: %#08x (%s, size %zu)\n", lt.name.c_str(), sum,
                gptneox_format_tensor_shape(lt.ne).c_str(), lt.size);
    }

};


//
// kv cache
//

static bool kv_cache_init(
        const struct gptneox_hparams & hparams,
             struct gptneox_kv_cache & cache,
                         ggml_type   wtype,
                               int   n_ctx) {
    const int n_embd  = hparams.n_embd;
    const int n_layer = hparams.n_layer;

    const int64_t n_mem      = (int64_t)n_layer*n_ctx;
    const int64_t n_elements = n_embd*n_mem;

    cache.buf.resize(2u*n_elements*ggml_type_size(wtype) + 2u*MiB);

    struct ggml_init_params params;
    params.mem_size   = cache.buf.size;
    params.mem_buffer = cache.buf.addr;
    params.no_alloc   = false;

    cache.ctx = ggml_init(params);

    if (!cache.ctx) {
        fprintf(stderr, "%s: failed to allocate memory for kv cache\n", __func__);
        return false;
    }

    cache.k = ggml_new_tensor_1d(cache.ctx, wtype, n_elements);
    cache.v = ggml_new_tensor_1d(cache.ctx, wtype, n_elements);

    return true;
}

struct gptneox_context_params gptneox_context_default_params() {
    struct gptneox_context_params result = {
        /*.n_ctx                       =*/ 512,
        /*.n_parts                     =*/ -1,
        /*.seed                        =*/ 0,
        /*.f16_kv                      =*/ false,
        /*.logits_all                  =*/ false,
        /*.vocab_only                  =*/ false,
        /*.use_mmap                    =*/ true,
        /*.use_mlock                   =*/ false,
        /*.embedding                   =*/ false,
        /*.progress_callback           =*/ nullptr,
        /*.progress_callback_user_data =*/ nullptr,
    };

    return result;
}

bool gptneox_mmap_supported() {
    return gptneox_mmap::SUPPORTED;
}

bool gptneox_mlock_supported() {
    return gptneox_mlock::SUPPORTED;
}

//
// model loading
//

static const char *gptneox_file_version_name(gptneox_file_version version) {
    switch (version) {
        case GPTNEOX_FILE_VERSION_GGML: return "'ggml' (old version with low tokenizer quality and no mmap support)";
        case GPTNEOX_FILE_VERSION_GGMF_V1: return "ggmf v1 (old version with no mmap support)";
        case GPTNEOX_FILE_VERSION_GGJT_V1: return "ggjt v1 (latest)";
        default: GPTNEOX_ASSERT(false);
    }
}

static const char *gptneox_ftype_name(enum gptneox_ftype ftype) {
    switch (ftype) {
        case GPTNEOX_FTYPE_ALL_F32:     return "all F32";
        case GPTNEOX_FTYPE_MOSTLY_F16:  return "mostly F16";
        case GPTNEOX_FTYPE_MOSTLY_Q4_0: return "mostly Q4_0";
        case GPTNEOX_FTYPE_MOSTLY_Q4_1: return "mostly Q4_1";
        case GPTNEOX_FTYPE_MOSTLY_Q4_1_SOME_F16:
                                      return "mostly Q4_1, some F16";
        //case GPTNEOX_FTYPE_MOSTLY_Q4_2: return "mostly Q4_2";
        //case GPTNEOX_FTYPE_MOSTLY_Q4_3: return "mostly Q4_3";
        case GPTNEOX_FTYPE_MOSTLY_Q5_0: return "mostly Q5_0";
        case GPTNEOX_FTYPE_MOSTLY_Q5_1: return "mostly Q5_1";
        case GPTNEOX_FTYPE_MOSTLY_Q8_0: return "mostly Q8_0";
        default:                      return "unknown, may not work";
    }
}

static const char *gptneox_model_type_name(e_model type) {
    switch (type) {
        case MODEL_3B: return "3B";
        case MODEL_7B: return "7B";
        case MODEL_12B: return "12B";
        case MODEL_20B: return "20B";
        case MODEL_UNKNOWN: return "UNKNOWN";
        default: GPTNEOX_ASSERT(false);
    }
}

static void gptneox_model_load_internal(
        const std::string & fname,
        gptneox_context & lctx,
        int n_ctx,
        ggml_type memory_type,
        bool use_mmap,
        bool use_mlock,
        bool vocab_only,
        gptneox_progress_callback progress_callback,
        void * progress_callback_user_data) {

    lctx.t_start_us = ggml_time_us();

    std::unique_ptr<gptneox_model_loader> ml(new gptneox_model_loader(fname, use_mmap, vocab_only));

    lctx.vocab = std::move(ml->file_loaders.at(0)->vocab);
    auto & model = lctx.model;
    model.hparams = ml->file_loaders.at(0)->hparams;
    gptneox_file_version file_version = ml->file_loaders.at(0)->file_version;
    auto & hparams = model.hparams;
    
    {
        switch (hparams.n_layer) {
            case 16: {
                if (hparams.n_embd < 6144) {
                    model.type = e_model::MODEL_3B;
                } else {
                    model.type = e_model::MODEL_7B;
                }
                break;
            }
            // # <RedPajama>: we extend the model type settings for RedPajama models.  
            case 32:{
                if (hparams.n_embd == 2560) {
                    model.type = e_model::MODEL_3B;
                } else if (hparams.n_embd == 4096) {
                    model.type = e_model::MODEL_7B;
                }
                else {
                    model.type = e_model::MODEL_UNKNOWN;
                }
                break;
            }
            case 36: model.type = e_model::MODEL_12B; break;
            case 44: model.type = e_model::MODEL_20B; break;
        }

        hparams.n_ctx = n_ctx;
    }

    {
        fprintf(stderr, "%s: format     = %s\n",  __func__, gptneox_file_version_name(file_version));
        fprintf(stderr, "%s: n_vocab    = %u\n",  __func__, hparams.n_vocab);
        fprintf(stderr, "%s: n_ctx      = %u\n",  __func__, hparams.n_ctx);
        fprintf(stderr, "%s: n_embd     = %u\n",  __func__, hparams.n_embd);
        fprintf(stderr, "%s: n_head     = %u\n",  __func__, hparams.n_head);
        fprintf(stderr, "%s: n_layer    = %u\n",  __func__, hparams.n_layer);
        fprintf(stderr, "%s: n_rot      = %u\n",  __func__, hparams.n_rot);
        fprintf(stderr, "%s: use_parallel_residual = %d\n", __func__, hparams.use_parallel_residual);
        fprintf(stderr, "%s: ftype      = %u (%s)\n", __func__, hparams.ftype, gptneox_ftype_name(hparams.ftype));
        fprintf(stderr, "%s: n_parts    = %zu\n", __func__, ml->file_loaders.size());
        fprintf(stderr, "%s: model size = %s\n",  __func__, gptneox_model_type_name(model.type));
    }

    if (vocab_only) {
        return;
    }

    auto & ctx = model.ctx;

    size_t ctx_size, mmapped_size;
    ml->calc_sizes(&ctx_size, &mmapped_size);
    fprintf(stderr, "%s: ggml ctx size = %6.2f KiB\n", __func__, ctx_size/1024.0);

    // print memory requirements
    {
        const size_t scale = memory_type == GGML_TYPE_F32 ? 2 : 1;

        // this is the total memory required to run the inference
        const size_t mem_required =
            ctx_size +
            mmapped_size +
            MEM_REQ_SCRATCH0().at(model.type) +
            MEM_REQ_SCRATCH1().at(model.type) +
            MEM_REQ_EVAL().at(model.type);

        // this is the memory required by one gptneox_state
        const size_t mem_required_state =
            scale*MEM_REQ_KV_SELF().at(model.type);

        fprintf(stderr, "%s: mem required  = %7.2f MiB (+ %7.2f MiB per state)\n", __func__,
                mem_required / 1024.0 / 1024.0, mem_required_state / 1024.0 / 1024.0);
    }

    // create the ggml context
    {
        lctx.model.buf.resize(ctx_size);
        if (use_mlock) {
            lctx.model.mlock_buf.init(lctx.model.buf.addr);
            lctx.model.mlock_buf.grow_to(lctx.model.buf.size);
        }

        struct ggml_init_params params = {
            /*.mem_size   =*/ lctx.model.buf.size,
            /*.mem_buffer =*/ lctx.model.buf.addr,
            /*.no_alloc   =*/ ml->use_mmap,
        };

        model.ctx = ggml_init(params);
        if (!model.ctx) {
            throw format("ggml_init() failed");
        }
    }

    // prepare memory for the weights
    {
        const auto & hparams = model.hparams;

        const uint32_t n_embd  = hparams.n_embd;
        const uint32_t n_layer = hparams.n_layer;
        const uint32_t n_vocab = hparams.n_vocab;

        ml->ggml_ctx = ctx;

        model.wte       = ml->get_tensor("gpt_neox.embed_in.weight",            {n_embd, n_vocab});
        model.ln_f_g    = ml->get_tensor("gpt_neox.final_layer_norm.weight",    {n_embd});
        model.ln_f_b    = ml->get_tensor("gpt_neox.final_layer_norm.bias",      {n_embd});
        model.lmh_g     = ml->get_tensor("embed_out.weight",                    {n_embd, n_vocab});

        model.layers.resize(n_layer);
        for (uint32_t i = 0; i < n_layer; ++i) {
            auto & layer = model.layers[i];

            std::string layers_i = "gpt_neox.layers." + std::to_string(i);

            layer.ln_attn_g = ml->get_tensor(layers_i + ".input_layernorm.weight", {n_embd});
            layer.ln_attn_b = ml->get_tensor(layers_i + ".input_layernorm.bias", {n_embd});

            layer.c_attn_attn_w = ml->get_tensor(layers_i + ".attention.query_key_value.weight", {n_embd, n_embd * 3});
            layer.c_attn_attn_b = ml->get_tensor(layers_i + ".attention.query_key_value.bias", {n_embd * 3});
            layer.c_attn_proj_w = ml->get_tensor(layers_i + ".attention.dense.weight", {n_embd, n_embd});
            layer.c_attn_proj_b = ml->get_tensor(layers_i + ".attention.dense.bias", {n_embd});

            layer.ln_ff_g = ml->get_tensor(layers_i + ".post_attention_layernorm.weight", {n_embd});
            layer.ln_ff_b = ml->get_tensor(layers_i + ".post_attention_layernorm.bias", {n_embd});

            layer.c_mlp_fc_w =   ml->get_tensor(layers_i + ".mlp.dense_h_to_4h.weight", {n_embd,   n_embd * 4});
            layer.c_mlp_fc_b =   ml->get_tensor(layers_i + ".mlp.dense_h_to_4h.bias",   {n_embd * 4});
            layer.c_mlp_proj_w = ml->get_tensor(layers_i + ".mlp.dense_4h_to_h.weight", {n_embd * 4,   n_embd});
            layer.c_mlp_proj_b = ml->get_tensor(layers_i + ".mlp.dense_4h_to_h.bias",   {n_embd});
        }
    }

    ml->done_getting_tensors();

    // populate `tensors_by_name`
    for (gptneox_load_tensor & lt : ml->tensors_map.tensors) {
        model.tensors_by_name.emplace_back(lt.name, lt.ggml_tensor);
    }

    ml->load_all_data(progress_callback, progress_callback_user_data, use_mlock ? &lctx.model.mlock_mmap : NULL);

    model.mapping = std::move(ml->mapping);

    // loading time will be recalculate after the first eval, so
    // we take page faults deferred by mmap() into consideration
    lctx.t_load_us = ggml_time_us() - lctx.t_start_us;
}

static bool gptneox_model_load(
        const std::string & fname,
        gptneox_context & lctx,
        int n_ctx,
        ggml_type memory_type,
        bool use_mmap,
        bool use_mlock,
        bool vocab_only,
        gptneox_progress_callback progress_callback,
        void *progress_callback_user_data) {
    try {
        gptneox_model_load_internal(fname, lctx, n_ctx, memory_type, use_mmap, use_mlock,
                                  vocab_only, progress_callback, progress_callback_user_data);
        return true;
    } catch (const std::string & err) {
        fprintf(stderr, "error loading model: %s\n", err.c_str());
        return false;
    }
}

// evaluate the transformer
//
//   - lctx:      llama context
//   - tokens:    new batch of tokens to process
//   - n_past:    the context size so far
//   - n_threads: number of threads to use
//
static bool gptneox_eval_internal(
        gptneox_context & lctx,
    const gptneox_token * tokens,
            const int   n_tokens,
            const int   n_past,
            const int   n_threads) {
    const int64_t t_start_us = ggml_time_us();

    const int N = n_tokens;

    const auto & model   = lctx.model;
    const auto & hparams = model.hparams;

    auto & kv_self = model.kv_self;

    GPTNEOX_ASSERT(!!kv_self.ctx);

    const int n_embd  = hparams.n_embd;
    const int n_layer = hparams.n_layer;
    const int n_ctx   = hparams.n_ctx;
    const int n_head  = hparams.n_head;
    const int n_vocab = hparams.n_vocab;
    const int n_rot   = hparams.n_rot;

    auto & mem_per_token = lctx.mem_per_token;
    auto & buf_compute   = lctx.buf_compute;

    struct ggml_init_params params = {
        /*.mem_size   =*/ buf_compute.size,
        /*.mem_buffer =*/ buf_compute.addr,
        /*.no_alloc   =*/ false,
    };

    struct ggml_context * ctx0 = ggml_init(params);

    // for big prompts, if BLAS is enabled, it is better to use only one thread
    // otherwise, the threads are spin-lock waiting for the BLAS calls and are degrading the performance
    ggml_cgraph gf = {};

    struct ggml_tensor * embd = ggml_new_tensor_1d(ctx0, GGML_TYPE_I32, N);
    memcpy(embd->data, tokens, N*ggml_element_size(embd));

    struct ggml_tensor * inpL = ggml_get_rows(ctx0, model.wte, embd);

    for (int il = 0; il < n_layer; ++il) {
        struct ggml_tensor * cur;

        lctx.use_buf(ctx0, 0);

        // input norm
        {
            cur = ggml_norm(ctx0, inpL);

            // cur = ln_attn_g*cur + ln_attn_b
            cur = ggml_add(ctx0,
                    ggml_mul(ctx0,
                        ggml_repeat(ctx0, model.layers[il].ln_attn_g, cur),
                        cur),
                    ggml_repeat(ctx0, model.layers[il].ln_attn_b, cur));
        }

        // self-attention
        {
            // attn
            // [3*n_embd, n_embd] - model.layers[il].c_attn_attn_w
            // [3*n_embd,      1] - model.layers[il].c_attn_attn_b
            // [  n_embd,      N] - cur (in)
            // [3*n_embd,      N] - cur (out)
            //
            // cur = attn_w*cur + attn_b
            // [3*n_embd, N]
            {
                cur = ggml_mul_mat(ctx0, model.layers[il].c_attn_attn_w, cur);
                cur = ggml_add(ctx0,
                        ggml_repeat(ctx0,
                                    model.layers[il].c_attn_attn_b, cur),
                        cur);
            }
             
            // Split QKV and make contiguous
            struct ggml_tensor * Qcur = ggml_view_3d(ctx0, cur,
                                            n_embd/n_head,
                                            n_head,
                                            N,
                                            ggml_element_size(cur) * 3 * n_embd/n_head,
                                            ggml_element_size(cur) * 3 * n_embd,
                                            ggml_element_size(cur) * n_embd/n_head * 0);
            struct ggml_tensor * Kcur = ggml_view_3d(ctx0, cur,
                                            n_embd/n_head,
                                            n_head,
                                            N,
                                            ggml_element_size(cur) * 3 * n_embd/n_head,
                                            ggml_element_size(cur) * 3 * n_embd,
                                            ggml_element_size(cur) * n_embd/n_head * 1);
            struct ggml_tensor * Vcur = ggml_view_3d(ctx0, cur,
                                            n_embd/n_head,
                                            n_head,
                                            N,
                                            ggml_element_size(cur) * 3 * n_embd/n_head,
                                            ggml_element_size(cur) * 3 * n_embd,
                                            ggml_element_size(cur) * n_embd/n_head * 2);
            // TODO: Flatten without copying, or see if non-contiguous can be used for any of QKV.
            Qcur = ggml_cpy(ctx0, Qcur,
                        ggml_new_tensor_3d(ctx0, GGML_TYPE_F32, n_embd/n_head, n_head, N));
            Kcur = ggml_cpy(ctx0, Kcur,
                        ggml_new_tensor_3d(ctx0, GGML_TYPE_F32, n_embd/n_head, n_head, N));
            Vcur = ggml_cpy(ctx0, Vcur,
                        ggml_new_tensor_3d(ctx0, GGML_TYPE_F32, n_embd/n_head, n_head, N));
            
            // MARK: gptneox RoPE Q and K, before cache
            // Bit 2 for gptneox style (2)
            // Bit 1 is zero for dont skip n_past +(0), use (2+1) = (3) if rope is applied to cache of k (after cache only)
            Qcur = ggml_rope(ctx0, Qcur, n_past, n_rot, 2, 0);
            Kcur = ggml_rope(ctx0, Kcur, n_past, n_rot, 2, 0); //3);

            // store key and value to memory, not required if prompt if only a single token (not practical or likely)
            //if (N >= 1) {
                // Each entry in kv_self has byte size of (ggml_element_size * n_embd * n_ctx * n_layer)
                Vcur = ggml_view_2d(ctx0, Vcur,
                            n_embd,
                            N,
                            ggml_element_size(Vcur) * n_embd,
                            0);
                Vcur = ggml_transpose(ctx0, Vcur);
            
                struct ggml_tensor * k = ggml_view_1d(ctx0, kv_self.k,
                                            n_embd * N, // num elements in current context (up to n_embd*n_ctx but usually less)
                                            ggml_element_size(kv_self.k) * n_embd * (il * n_ctx + n_past));
                struct ggml_tensor * v = ggml_view_2d(ctx0, kv_self.v,
                                            N,
                                            n_embd,
                                            ggml_element_size(kv_self.v) * n_ctx,
                                            ggml_element_size(kv_self.v) * ((il * n_ctx * n_embd) + n_past));
            
                // important: storing RoPE-ed version of K in the KV cache!
                ggml_build_forward_expand(&gf, ggml_cpy(ctx0, Kcur, k));
                ggml_build_forward_expand(&gf, ggml_cpy(ctx0, Vcur, v));
            //}
            
            // Q = Qcur.contiguous().view(n_embd/n_head, n_head, N).permute(0, 2, 1, 3)
            struct ggml_tensor * Q =
                ggml_permute(ctx0,
                        Qcur,
                        0, 2, 1, 3);

            // K = Kmem.view(n_embd/n_head, n_head, n_past + N).permute(0, 2, 1, 3)
            struct ggml_tensor * K =
                ggml_permute(ctx0,
                        ggml_reshape_3d(ctx0,
                            ggml_view_1d(ctx0, kv_self.k,
                                (n_past + N) * n_embd,
                                ggml_element_size(kv_self.k) * il * n_ctx * n_embd),
                            n_embd/n_head, n_head, n_past + N),
                        0, 2, 1, 3);

            // K * Q
            // Will use internally ggml_compute_forward_mul_mat_f16_f32 because K is f16 (cache) and Q is f32 (from q4_0)
            // Outputs [N, N, H, B], so it seems like this is correct for "scores"
            // K is internally transposed by ggml_mul_mat
            struct ggml_tensor * KQ = ggml_mul_mat(ctx0, K, Q);
            // KQ_scaled = KQ / sqrt(n_embd/n_head)
            struct ggml_tensor * KQ_scaled = ggml_scale(ctx0, KQ,
                                                ggml_new_f32(ctx0, 1.0f/sqrt(float(n_embd)/n_head)));
            // KQ_masked = mask_past(KQ_scaled)
            struct ggml_tensor * KQ_masked = ggml_diag_mask_inf(ctx0, KQ_scaled, n_past);
            // KQ = soft_max(KQ_masked)
            struct ggml_tensor * KQ_soft_max = ggml_soft_max(ctx0, KQ_masked);
            
            // V_trans = Vmem.view(n_embd/n_head, n_head, n_past + N).permute(1, 2, 0, 3).contiguous()
            struct ggml_tensor * V_trans = ggml_view_3d(ctx0, kv_self.v,
                                                n_past + N,
                                                n_embd/n_head,
                                                n_head,
                                                ggml_element_size(kv_self.v) * n_ctx,
                                                ggml_element_size(kv_self.v) * n_ctx * n_embd/n_head,
                                                ggml_element_size(kv_self.v) * il * n_ctx * n_embd);

            // KQV = transpose(V) * KQ_soft_max
            struct ggml_tensor * KQV = ggml_mul_mat(ctx0, V_trans, KQ_soft_max);

            // KQV_merged = KQV.permute(0, 2, 1, 3)
            struct ggml_tensor * KQV_merged = ggml_permute(ctx0, KQV, 0, 2, 1, 3);

            // cur = KQV_merged.contiguous().view(n_embd, N)
            cur = ggml_cpy(ctx0, KQV_merged,
                        ggml_new_tensor_2d(ctx0, GGML_TYPE_F32, n_embd, N));

            // projection (first weight)
            cur = ggml_mul_mat(ctx0, model.layers[il].c_attn_proj_w, cur);

            // projection (then bias)
            cur = ggml_add(ctx0, ggml_repeat(ctx0, model.layers[il].c_attn_proj_b, cur), cur);
        }

        lctx.use_buf(ctx0, 1);
        
        if (hparams.use_parallel_residual == 1) {
            //printf("use_parallel_residual == 1\n");
            
            // This is independent of the self-attention result, so it could be done in parallel to the self-attention
            struct ggml_tensor * outAttn = cur;

            // post attention layer norm
            {
                cur = ggml_norm(ctx0, inpL);

                // cur = ln_attn_g*inpFF + ln_attn_b
                cur = ggml_add(ctx0,
                    ggml_mul(ctx0,
                        ggml_repeat(ctx0, model.layers[il].ln_ff_g, cur),
                        cur),
                    ggml_repeat(ctx0, model.layers[il].ln_ff_b, cur));
            }


            // feed-forward network
            {
                // note here we pass inpFF instead of cur
                cur = ggml_mul_mat(ctx0, model.layers[il].c_mlp_fc_w, cur);

                cur = ggml_add(ctx0,
                            ggml_repeat(ctx0, model.layers[il].c_mlp_fc_b, cur),
                            cur);

                // GELU activation
                cur = ggml_gelu(ctx0, cur);

                // projection
                // cur = proj_w*inpFF + proj_b
                cur = ggml_mul_mat(ctx0, model.layers[il].c_mlp_proj_w, cur);

                cur = ggml_add(ctx0,
                            ggml_repeat(ctx0, model.layers[il].c_mlp_proj_b, cur),
                            cur);
            }
            //# pseudocode:
            //# x = x + attn(ln1(x)) + mlp(ln2(x))
            // inpL = inpL + outAttn + cur
            cur = ggml_add(ctx0, outAttn, cur);
            inpL = ggml_add(ctx0, inpL, cur);
        } else if (hparams.use_parallel_residual == 0) {
            //printf("use_parallel_residual == 0\n");
            
            // This takes the self-attention residual output as input to Feedforward
            struct ggml_tensor * outAttn = cur;
            struct ggml_tensor * inpFF = ggml_add(ctx0, outAttn, inpL);

            // post attention layer norm
            {
                cur = ggml_norm(ctx0, inpFF);

                // inpFF = ln_attn_g*inpFF + ln_attn_b
                cur = ggml_add(ctx0,
                    ggml_mul(ctx0,
                        ggml_repeat(ctx0, model.layers[il].ln_ff_g, cur),
                        cur),
                    ggml_repeat(ctx0, model.layers[il].ln_ff_b, cur));
            }

            // feed-forward network
            {
                // note here we pass inpFF instead of cur
                cur = ggml_mul_mat(ctx0, model.layers[il].c_mlp_fc_w, cur);

                cur = ggml_add(ctx0, ggml_repeat(ctx0, model.layers[il].c_mlp_fc_b, cur), cur);

                cur = ggml_gelu(ctx0, cur);

                cur = ggml_mul_mat(ctx0, model.layers[il].c_mlp_proj_w, cur);

                cur = ggml_add(ctx0, ggml_repeat(ctx0, model.layers[il].c_mlp_proj_b, cur), cur);
            }

            //# pseudocode:
            //# x = x + attn(ln1(x)) (residual above as input to mlp)
            //# x = x + mlp(ln2(x)) (residual after mlp aka inpL + cur)
            //# <RedPajama>: we fixed a small issue in the gptneox.cpp fork when setting use_parallel_residual to False;
            inpL = ggml_add(ctx0, inpFF, cur);
        } else {
            printf("use_parallel_residual == %d\n", hparams.use_parallel_residual);
            assert(0);
        }
    }

    lctx.use_buf(ctx0, 0);

    // used at the end to optionally extract the embeddings
    struct ggml_tensor * embeddings = NULL;

    // norm
    {
        inpL = ggml_norm(ctx0, inpL);

        // inpL = ln_f_g*inpL + ln_f_b
        inpL = ggml_add(ctx0,
                ggml_mul(ctx0,
                    ggml_repeat(ctx0, model.ln_f_g, inpL),
                    inpL),
                ggml_repeat(ctx0, model.ln_f_b, inpL));

        embeddings = inpL;
    }

    // lm_head
    inpL = ggml_mul_mat(ctx0, model.lmh_g, inpL);

    lctx.use_buf(ctx0, -1);

    // logits -> probs
    //inpL = ggml_soft_max(ctx0, inpL);

    // run the computation
    ggml_build_forward_expand(&gf, inpL);
    ggml_graph_compute_with_ctx(ctx0, &gf, N >= 32 && ggml_cpu_has_blas() && !ggml_cpu_has_cublas() ? 1 : n_threads);

#ifdef GGML_PERF
    // print timing information per ggml operation (for debugging purposes)
    // requires GGML_PERF to be defined
    ggml_graph_print(&gf);
#endif

    // plot the computation graph in dot format (for debugging purposes)
    //if (n_past%100 == 0) {
    //    ggml_graph_dump_dot(&gf, NULL, "llama.dot");
    //}

    //embd_w.resize(n_vocab*N);
    //memcpy(embd_w.data(), ggml_get_data(inpL), sizeof(float)*n_vocab*N);

    // extract logits
    {
        auto & logits_out = lctx.logits;

        if (lctx.logits_all) {
            logits_out.resize(n_vocab * N);
            memcpy(logits_out.data(), (float *) ggml_get_data(inpL), sizeof(float)*n_vocab*N);
        } else {
            // return result for just the last token
            logits_out.resize(n_vocab);
            memcpy(logits_out.data(), (float *) ggml_get_data(inpL) + (n_vocab*(N-1)), sizeof(float)*n_vocab);
        }
    }

    // extract embeddings
    if (lctx.embedding.size()) {
        auto & embedding_out = lctx.embedding;

        embedding_out.resize(n_embd);
        memcpy(embedding_out.data(), (float *) ggml_get_data(embeddings) + (n_embd*(N - 1)), sizeof(float)*n_embd);
    }

    if (mem_per_token == 0) {
        mem_per_token = ggml_used_mem(ctx0)/N;
    }

#if 0
    printf("\n%s: used_mem = %.3f MiB, scratch -- %.3f MiB %.3f MiB\n", __func__,
            ggml_used_mem(ctx0)/1024.0/1024.0,
            lctx.get_buf_max_mem(0)/1024.0/1024.0,
            lctx.get_buf_max_mem(1)/1024.0/1024.0);
#endif

    ggml_free(ctx0);

    // measure the performance only for the single-token evals
    if (N == 1) {
        lctx.t_eval_us += ggml_time_us() - t_start_us;
        lctx.n_eval++;
    }
    else if (N > 1) {
        lctx.t_p_eval_us += ggml_time_us() - t_start_us;
        lctx.n_p_eval += N;
    }

    return true;
}

//
// tokenizer
//

static size_t utf8_len(char src) {
    const size_t lookup[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4 };
    uint8_t highbits = static_cast<uint8_t>(src) >> 4;
    return lookup[highbits];
}

struct gptneox_sp_symbol {
    using index = int;
    index prev;
    index next;
    const char * text;
    size_t n;
};

struct gptneox_sp_bigram {
    struct comparator {
        bool operator()(gptneox_sp_bigram & l, gptneox_sp_bigram & r) {
            return (l.score < r.score) || (l.score == r.score && l.left > r.left);
        }
    };
    using queue_storage = std::vector<gptneox_sp_bigram>;
    using queue = std::priority_queue<gptneox_sp_bigram, queue_storage, comparator>;
    gptneox_sp_symbol::index left;
    gptneox_sp_symbol::index right;
    float score;
    size_t size;
};

// original implementation:
// https://github.com/ggerganov/llama.cpp/commit/074bea2eb1f1349a0118239c4152914aecaa1be4
struct gptneox_tokenizer {
    gptneox_tokenizer(const gptneox_vocab & vocab): vocab_(vocab) {}

    void tokenize(const std::string & text, std::vector<gptneox_vocab::id> & output) {
        // split string into utf8 chars
        int index = 0;
        size_t offs = 0;
        while (offs < text.size()) {
            gptneox_sp_symbol sym;
            size_t char_len = std::min(text.size() - offs, utf8_len(text[offs]));
            sym.text = text.c_str() + offs;
            sym.n = char_len;
            offs += char_len;
            sym.prev = index - 1;
            sym.next = offs == text.size() ? -1 : index + 1;
            index++;
            symbols_.emplace_back(std::move(sym));
        }

        // seed the work queue with all possible 2-character tokens.
        for (size_t i = 1; i < symbols_.size(); ++i) {
            try_add_bigram(i - 1, i);
        }

        // keep substituting the highest frequency pairs for as long as we can.
        while (!work_queue_.empty()) {
            auto bigram = work_queue_.top();
            work_queue_.pop();

            auto & left_sym = symbols_[bigram.left];
            auto & right_sym = symbols_[bigram.right];

            // if one of the symbols already got merged, skip it.
            if (left_sym.n == 0 || right_sym.n == 0 ||
                left_sym.n + right_sym.n != bigram.size) {
                continue;
            }

            // merge the right sym into the left one
            left_sym.n += right_sym.n;
            right_sym.n = 0;

            //printf("left = '%*s' size = %zu\n", (int) left_sym.n, left_sym.text, bigram.size);

            // remove the right sym from the chain
            left_sym.next = right_sym.next;
            if (right_sym.next >= 0) {
                symbols_[right_sym.next].prev = bigram.left;
            }

            // find more substitutions
            try_add_bigram(left_sym.prev, bigram.left);
            try_add_bigram(bigram.left, left_sym.next);
        }

        for (int i = 0; i != -1; i = symbols_[i].next) {
            auto & symbol = symbols_[i];
            auto token = vocab_.token_to_id.find(std::string(symbol.text, symbol.n));

            if (token == vocab_.token_to_id.end()) {
                // output any symbols that did not form tokens as bytes.
                for (int j = 0; j < (int) symbol.n; ++j) {
                    gptneox_vocab::id token_id = static_cast<uint8_t>(symbol.text[j]) + 3;
                    output.push_back(token_id);
                }
            } else {
                output.push_back((*token).second);
            }
        }
    }

private:
    void try_add_bigram(int left, int right) {
        if (left == -1 || right == -1) {
            return;
        }

        const std::string text = std::string(symbols_[left].text, symbols_[left].n + symbols_[right].n);
        auto token = vocab_.token_to_id.find(text);

        if (token == vocab_.token_to_id.end()) {
            return;
        }

        if (static_cast<size_t>((*token).second) >= vocab_.id_to_token.size()) {
            return;
        }

        const auto &tok_score = vocab_.id_to_token[(*token).second];

        gptneox_sp_bigram bigram;
        bigram.left = left;
        bigram.right = right;
        bigram.score = tok_score.score;
        bigram.size = text.size();
        work_queue_.push(bigram);
    }

    const gptneox_vocab & vocab_;
    std::vector<gptneox_sp_symbol> symbols_;
    gptneox_sp_bigram::queue work_queue_;
};

static std::vector<gptneox_vocab::id> gptneox_tokenize(const gptneox_vocab & vocab, const std::string & text, bool bos) {
    gptneox_tokenizer tokenizer(vocab);
    std::vector<gptneox_vocab::id> output;

    if (text.size() == 0) {
        return output;
    }

    if (bos) {
        output.push_back(gptneox_token_bos());
    }

    tokenizer.tokenize(text, output);
    return output;
}

//
// sampling
//

void gptneox_sample_softmax(struct gptneox_context * ctx, gptneox_token_data_array * candidates) {
    assert(candidates->size > 0);

    const int64_t t_start_sample_us = ggml_time_us();

    // Sort the logits in descending order
    if (!candidates->sorted) {
        std::sort(candidates->data, candidates->data + candidates->size, [](const gptneox_token_data & a, const gptneox_token_data & b) {
            return a.logit > b.logit;
        });
        candidates->sorted = true;
    }

    float max_l = candidates->data[0].logit;
    float cum_sum = 0.0f;
    for (size_t i = 0; i < candidates->size; ++i) {
        float p = expf(candidates->data[i].logit - max_l);
        candidates->data[i].p = p;
        cum_sum += p;
    }
    for (size_t i = 0; i < candidates->size; ++i) {
        candidates->data[i].p /= cum_sum;
    }

    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
}

void gptneox_sample_top_k(struct gptneox_context * ctx, gptneox_token_data_array * candidates, int k, size_t min_keep) {
    const int64_t t_start_sample_us = ggml_time_us();

    k = std::max(k, (int) min_keep);
    k = std::min(k, (int) candidates->size);

    // Sort scores in descending order
    if (!candidates->sorted) {
        auto comp = [](const gptneox_token_data & a, const gptneox_token_data & b) {
            return a.logit > b.logit;
        };
        if (k == (int) candidates->size) {
            std::sort(candidates->data, candidates->data + candidates->size, comp);
        } else {
            std::partial_sort(candidates->data, candidates->data + k, candidates->data + candidates->size, comp);
        }
        candidates->sorted = true;
    }
    candidates->size = k;

    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
}

void gptneox_sample_top_p(struct gptneox_context * ctx, gptneox_token_data_array * candidates, float p, size_t min_keep) {
    if (p >= 1.0f) {
        return;
    }

    const int64_t t_start_sample_us = ggml_time_us();

    gptneox_sample_softmax(ctx, candidates);

    // Compute the cumulative probabilities
    float cum_sum = 0.0f;
    size_t last_idx = candidates->size;

    for (size_t i = 0; i < candidates->size; ++i) {
        cum_sum += candidates->data[i].p;

        // Check if the running sum is greater than p or if we have kept at least min_keep tokens
        if (cum_sum > p && i >= min_keep) {
            last_idx = i;
            break;
        }
    }

    // Resize the output vector to keep only the top-p tokens
    candidates->size = last_idx;

    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
}

void gptneox_sample_tail_free(struct gptneox_context * ctx, gptneox_token_data_array * candidates, float z, size_t min_keep) {
    if (z >= 1.0f || candidates->size <= 2) {
        return;
    }

    const int64_t t_start_sample_us = ggml_time_us();

    gptneox_sample_softmax(nullptr, candidates);

    // Compute the first and second derivatives
    std::vector<float> first_derivatives(candidates->size - 1);
    std::vector<float> second_derivatives(candidates->size - 2);

    for (size_t i = 0; i < first_derivatives.size(); ++i) {
        first_derivatives[i] = candidates->data[i].p - candidates->data[i + 1].p;
    }
    for (size_t i = 0; i < second_derivatives.size(); ++i) {
        second_derivatives[i] = first_derivatives[i] - first_derivatives[i + 1];
    }

    // Calculate absolute value of second derivatives
    for (size_t i = 0; i < second_derivatives.size(); ++i) {
        second_derivatives[i] = abs(second_derivatives[i]);
    }

    // Normalize the second derivatives
    float second_derivatives_sum = std::accumulate(second_derivatives.begin(), second_derivatives.end(), 0.0f);
    for (float & value : second_derivatives) {
        value /= second_derivatives_sum;
    }

    float cum_sum = 0.0f;
    size_t last_idx = candidates->size;
    for (size_t i = 0; i < second_derivatives.size(); ++i) {
        cum_sum += second_derivatives[i];

        // Check if the running sum is greater than z or if we have kept at least min_keep tokens
        if (cum_sum > z && i >= min_keep) {
            last_idx = i;
            break;
        }
    }

    // Resize the output vector to keep only the tokens above the tail location
    candidates->size = last_idx;

    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
}


void gptneox_sample_typical(struct gptneox_context * ctx, gptneox_token_data_array * candidates, float p, size_t min_keep) {
    // Reference implementation:
    // https://github.com/huggingface/transformers/compare/main...cimeister:typical-sampling:typical-pr
    if (p >= 1.0f) {
        return;
    }

    const int64_t t_start_sample_us = ggml_time_us();

    // Compute the softmax of logits and calculate entropy
    gptneox_sample_softmax(nullptr, candidates);

    float entropy = 0.0f;
    for (size_t i = 0; i < candidates->size; ++i) {
        entropy += -candidates->data[i].p * logf(candidates->data[i].p);
    }

    // Compute the absolute difference between negative log probability and entropy for each candidate
    std::vector<float> shifted_scores;
    for (size_t i = 0; i < candidates->size; ++i) {
        float shifted_score = fabsf(-logf(candidates->data[i].p) - entropy);
        shifted_scores.push_back(shifted_score);
    }

    // Sort tokens based on the shifted_scores and their corresponding indices
    std::vector<size_t> indices(candidates->size);
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        return shifted_scores[a] < shifted_scores[b];
    });

    // Compute the cumulative probabilities
    float cum_sum = 0.0f;
    size_t last_idx = indices.size();

    for (size_t i = 0; i < indices.size(); ++i) {
        size_t idx = indices[i];
        cum_sum += candidates->data[idx].p;

        // Check if the running sum is greater than typical or if we have kept at least min_keep tokens
        if (cum_sum > p && i >= min_keep - 1) {
            last_idx = i + 1;
            break;
        }
    }

    // Resize the output vector to keep only the locally typical tokens
    std::vector<gptneox_token_data> new_candidates;
    for (size_t i = 0; i < last_idx; ++i) {
        size_t idx = indices[i];
        new_candidates.push_back(candidates->data[idx]);
    }

    // Replace the data in candidates with the new_candidates data
    std::copy(new_candidates.begin(), new_candidates.end(), candidates->data);
    candidates->size = new_candidates.size();

    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
}

void gptneox_sample_temperature(struct gptneox_context * ctx, gptneox_token_data_array * candidates_p, float temp) {
    const int64_t t_start_sample_us = ggml_time_us();

    for (size_t i = 0; i < candidates_p->size; ++i) {
        candidates_p->data[i].logit /= temp;
    }

    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
}

void gptneox_sample_repetition_penalty(struct gptneox_context * ctx, gptneox_token_data_array * candidates, gptneox_token * last_tokens, size_t last_tokens_size, float penalty) {
    if (last_tokens_size == 0 || penalty == 1.0f) {
        return;
    }

    const int64_t t_start_sample_us = ggml_time_us();

    for (size_t i = 0; i < candidates->size; ++i) {
        auto token_iter = std::find(last_tokens, last_tokens + last_tokens_size, candidates->data[i].id);
        if (token_iter == last_tokens + last_tokens_size) {
            continue;
        }

        // The academic publication that described this technique actually just only divided, but that would cause tokens with negative logits to become more likely, which is obviously wrong.
        // This is common fix for this problem, which is to multiply by the penalty instead of dividing.
        if (candidates->data[i].logit <= 0) {
            candidates->data[i].logit *= penalty;
        } else {
            candidates->data[i].logit /= penalty;
        }
    }

    candidates->sorted = false;

    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
}

void gptneox_sample_frequency_and_presence_penalties(struct gptneox_context * ctx, gptneox_token_data_array * candidates, gptneox_token * last_tokens_p, size_t last_tokens_size, float alpha_frequency, float alpha_presence) {
    if (last_tokens_size == 0 || (alpha_frequency == 0.0f && alpha_presence == 0.0f)) {
        return;
    }

    const int64_t t_start_sample_us = ggml_time_us();

    // Create a frequency map to count occurrences of each token in last_tokens
    std::unordered_map<gptneox_token, int> token_count;
    for (size_t i = 0; i < last_tokens_size; ++i) {
        token_count[last_tokens_p[i]]++;
    }

    // Apply frequency and presence penalties to the candidates
    for (size_t i = 0; i < candidates->size; ++i) {
        auto token_iter = token_count.find(candidates->data[i].id);
        if (token_iter == token_count.end()) {
            continue;
        }

        int count = token_iter->second;
        candidates->data[i].logit -= float(count) * alpha_frequency + float(count > 0) * alpha_presence;
    }

    candidates->sorted = false;

    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
}


gptneox_token gptneox_sample_token_mirostat(struct gptneox_context * ctx, gptneox_token_data_array * candidates, float tau, float eta, int m, float * mu) {
    assert(ctx);
    auto N = float(gptneox_n_vocab(ctx));
    int64_t t_start_sample_us;
    t_start_sample_us = ggml_time_us();

    gptneox_sample_softmax(nullptr, candidates);

    // Estimate s_hat using the most probable m tokens
    float s_hat = 0.0;
    float sum_ti_bi = 0.0;
    float sum_ti_sq = 0.0;
    for (size_t i = 0; i < size_t(m - 1) && i < candidates->size - 1; ++i) {
        float t_i = logf(float(i + 2) / float(i + 1));
        float b_i = logf(candidates->data[i].p / candidates->data[i + 1].p);
        sum_ti_bi += t_i * b_i;
        sum_ti_sq += t_i * t_i;
    }
    s_hat = sum_ti_bi / sum_ti_sq;

    // Compute k from the estimated s_hat and target surprise value
    float epsilon_hat = s_hat - 1;
    float k = powf((epsilon_hat * powf(2, *mu)) / (1 - powf(N, -epsilon_hat)), 1 / s_hat);

    // Sample the next word X using top-k sampling
    gptneox_sample_top_k(nullptr, candidates, int(k), 1);
    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
    gptneox_token X = gptneox_sample_token(ctx, candidates);
    t_start_sample_us = ggml_time_us();

    // Compute error as the difference between observed surprise and target surprise value
    size_t X_idx = std::distance(candidates->data, std::find_if(candidates->data, candidates->data + candidates->size, [&](const gptneox_token_data & candidate) {
        return candidate.id == X;
    }));
    float observed_surprise = -log2f(candidates->data[X_idx].p);
    float e = observed_surprise - tau;

    // Update mu using the learning rate and error
    *mu = *mu - eta * e;

    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
        ctx->n_sample++;
    }
    return X;
}

gptneox_token gptneox_sample_token_mirostat_v2(struct gptneox_context * ctx, gptneox_token_data_array * candidates, float tau, float eta, float * mu) {
    assert(ctx);
    int64_t t_start_sample_us;
    t_start_sample_us = ggml_time_us();

    gptneox_sample_softmax(ctx, candidates);

    // Truncate the words with surprise values greater than mu
    candidates->size = std::distance(candidates->data, std::find_if(candidates->data, candidates->data + candidates->size, [&](const gptneox_token_data & candidate) {
        return -log2f(candidate.p) > *mu;
    }));

    // Normalize the probabilities of the remaining words
    gptneox_sample_softmax(ctx, candidates);

    // Sample the next word X from the remaining words
    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
    gptneox_token X = gptneox_sample_token(ctx, candidates);
    t_start_sample_us = ggml_time_us();

    // Compute error as the difference between observed surprise and target surprise value
    size_t X_idx = std::distance(candidates->data, std::find_if(candidates->data, candidates->data + candidates->size, [&](const gptneox_token_data & candidate) {
        return candidate.id == X;
    }));
    float observed_surprise = -log2f(candidates->data[X_idx].p);
    float e = observed_surprise - tau;

    // Update mu using the learning rate and error
    *mu = *mu - eta * e;

    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
    return X;
}

gptneox_token gptneox_sample_token_greedy(struct gptneox_context * ctx, gptneox_token_data_array * candidates) {
    const int64_t t_start_sample_us = ggml_time_us();

    // Find max element
    auto max_iter = std::max_element(candidates->data, candidates->data + candidates->size, [](const gptneox_token_data & a, const gptneox_token_data & b) {
        return a.logit < b.logit;
    });

    gptneox_token result = max_iter->id;
    if (ctx) {
        ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
        ctx->n_sample++;
    }
    return result;
}

gptneox_token gptneox_sample_token(struct gptneox_context * ctx, gptneox_token_data_array * candidates) {
    assert(ctx);
    const int64_t t_start_sample_us = ggml_time_us();
    gptneox_sample_softmax(nullptr, candidates);

    std::vector<float> probs;
    probs.reserve(candidates->size);
    for (size_t i = 0; i < candidates->size; ++i) {
        probs.push_back(candidates->data[i].p);
    }

    std::discrete_distribution<> dist(probs.begin(), probs.end());
    auto & rng = ctx->rng;
    int idx = dist(rng);

    gptneox_token result = candidates->data[idx].id;

    ctx->t_sample_us += ggml_time_us() - t_start_sample_us;
    ctx->n_sample++;
    return result;
}

//
// quantization
//

// temp - load then save model, allows for load and save to be different
static void gptneox_model_copy_internal(const std::string & fname_inp, const std::string & fname_out, enum gptneox_ftype ftype) {
    std::unique_ptr<gptneox_model_loader> model_loader(new gptneox_model_loader(fname_inp.c_str(),
                                                            /*use_mmap*/ false,
                                                            /*vocab_only*/ false));
    gptneox_file_saver file_saver(fname_out.c_str(), model_loader->file_loaders.at(0).get(), ftype);

    size_t idx = 0;
    for (gptneox_load_tensor & tensor : model_loader->tensors_map.tensors) {
        gptneox_buffer read_data;
        read_data.resize(tensor.size);
        tensor.data = read_data.addr;
        model_loader->load_data_for(tensor);

        printf("[%4zu/%4zu] %36s - %16s, type = %6s, ",
               ++idx, model_loader->tensors_map.tensors.size(),
               tensor.name.c_str(), gptneox_format_tensor_shape(tensor.ne).c_str(),
               ggml_type_name(tensor.type));

        file_saver.write_tensor(tensor, tensor.type, tensor.data, tensor.size);
    }
}

int gptneox_model_copy(
        const char * fname_inp,
        const char * fname_out,
  enum gptneox_ftype   ftype) {
    try {
        gptneox_model_copy_internal(fname_inp, fname_out, ftype);
        return 0;
    } catch (const std::string & err) {
        fprintf(stderr, "%s: failed to copy: %s\n", __func__, err.c_str());
        return 1;
    }
}


static void gptneox_model_quantize_internal(const std::string & fname_inp, const std::string & fname_out, enum gptneox_ftype ftype, int nthread) {
    ggml_type quantized_type;
    switch (ftype) {
        case GPTNEOX_FTYPE_MOSTLY_Q4_0: quantized_type = GGML_TYPE_Q4_0; break;
        case GPTNEOX_FTYPE_MOSTLY_Q4_1: quantized_type = GGML_TYPE_Q4_1; break;
        //case GPTNEOX_FTYPE_MOSTLY_Q4_2: quantized_type = GGML_TYPE_Q4_2; break;
        case GPTNEOX_FTYPE_MOSTLY_Q5_0: quantized_type = GGML_TYPE_Q5_0; break;
        case GPTNEOX_FTYPE_MOSTLY_Q5_1: quantized_type = GGML_TYPE_Q5_1; break;
        case GPTNEOX_FTYPE_MOSTLY_Q8_0: quantized_type = GGML_TYPE_Q8_0; break;
        default: throw format("invalid output file type %d\n", ftype);
    };

    if (nthread <= 0) {
        nthread = std::thread::hardware_concurrency();
    }

    std::unique_ptr<gptneox_model_loader> model_loader(new gptneox_model_loader(fname_inp.c_str(), /*use_mmap*/ false,
                                                                            /*vocab_only*/ false));
    gptneox_file_saver file_saver(fname_out.c_str(), model_loader->file_loaders.at(0).get(), ftype);

    size_t total_size_org = 0;
    size_t total_size_new = 0;
    std::vector<int64_t> hist_all(1 << 4, 0);

    std::vector<std::thread> workers;
    std::mutex mutex;

    size_t idx = 0;
    for (gptneox_load_tensor & tensor : model_loader->tensors_map.tensors) {
        gptneox_buffer read_data;
        read_data.resize(tensor.size);
        tensor.data = read_data.addr;
        model_loader->load_data_for(tensor);

        printf("[%4zu/%4zu] %36s - %16s, type = %6s, ",
               ++idx, model_loader->tensors_map.tensors.size(),
               tensor.name.c_str(), gptneox_format_tensor_shape(tensor.ne).c_str(),
               ggml_type_name(tensor.type));

        // This used to be a regex, but <regex> has an extreme cost to compile times.
        bool quantize = tensor.name.rfind("weight") == tensor.name.size() - 6; // ends with 'weight'?

        // quantize only 2D tensors
        quantize &= (tensor.ne.size() == 2);

        // uncomment this to keep the output layer in FP16
        //if (tensor.name == "output.weight") {
        //    quantize = false;
        //}

        enum ggml_type new_type;
        void * new_data;
        size_t new_size;
        gptneox_buffer work;

        if (!quantize) {
            new_type = tensor.type;
            new_data = tensor.data;
            new_size = tensor.size;
            printf("size = %8.3f MiB\n", tensor.size/1024.0/1024.0);
        } else {
            new_type = quantized_type;
            float * f32_data;
            size_t nelements = tensor.ne.at(0) * tensor.ne.at(1);
            gptneox_buffer f32_conv_buf;
            if (tensor.type == GGML_TYPE_F32) {
                f32_data = (float *) tensor.data;
            } else if (tensor.type == GGML_TYPE_F16) {
                f32_conv_buf.resize(nelements * sizeof(float));
                f32_data = (float *) f32_conv_buf.addr;
                auto f16_data = (const ggml_fp16_t *) tensor.data;
                for (size_t i = 0; i < nelements; i++) {
                    f32_data[i] = ggml_fp16_to_fp32(f16_data[i]);
                }
            } else {
                throw format("type %s unsupported for integer quantization", ggml_type_name(tensor.type));
            }

            printf("quantizing .. ");
            fflush(stdout);

            work.resize(nelements * 4); // upper bound on size
            new_data = work.addr;
            std::vector<int64_t> hist_cur(1 << 4, 0);

            int chunk_size = 32 * 512;
            const int nchunk = (nelements + chunk_size - 1)/chunk_size;
            const int nthread_use = nthread > 1 ? std::max(1, std::min(nthread, nchunk)) : 1;
            if (nthread_use < 2) {
                new_size = ggml_quantize_chunk(new_type, f32_data, new_data, 0, nelements, hist_cur.data());
            } else {
                size_t counter = 0;
                new_size = 0;
                auto compute = [&mutex, &counter, &hist_cur, &new_size, new_type, f32_data, new_data, nelements, chunk_size] () {
                    std::vector<int64_t> local_hist;
                    size_t local_size = 0;
                    while (true) {
                        std::unique_lock<std::mutex> lock(mutex);
                        size_t first = counter; counter += chunk_size;
                        if (first >= nelements) {
                            if (!local_hist.empty()) {
                                for (int j=0; j<int(local_hist.size()); ++j) hist_cur[j] += local_hist[j];
                                new_size += local_size;
                            }
                            break;
                        }
                        lock.unlock();
                        size_t last = std::min(nelements, first + chunk_size);
                        if (local_hist.empty()) local_hist.resize(hist_cur.size(), 0);
                        local_size += ggml_quantize_chunk(new_type, f32_data, new_data, first, last - first, local_hist.data());
                    }
                };
                if (int(workers.size()) < nthread_use - 1) workers.resize(nthread_use - 1);
                for (int it = 0; it < nthread_use - 1; ++it) workers[it] = std::thread(compute);
                compute();
                for (int it = 0; it < nthread_use - 1; ++it) workers[it].join();
            }

            printf("size = %8.2f MiB -> %8.2f MiB | hist: ", tensor.size/1024.0/1024.0, new_size/1024.0/1024.0);
            for (size_t i = 0; i < hist_cur.size(); i++) {
                hist_all[i] += hist_cur[i];
            }

            for (size_t i = 0; i < hist_cur.size(); i++) {
                printf("%5.3f ", hist_cur[i] / float(nelements));
            }
            printf("\n");
        }
        total_size_org += tensor.size;
        total_size_new += new_size;
        file_saver.write_tensor(tensor, new_type, new_data, new_size);
    }

    printf("%s: model size  = %8.2f MiB\n", __func__, total_size_org/1024.0/1024.0);
    printf("%s: quant size  = %8.2f MiB\n", __func__, total_size_new/1024.0/1024.0);

    {
        int64_t sum_all = 0;
        for (size_t i = 0; i < hist_all.size(); i++) {
            sum_all += hist_all[i];
        }

        printf("%s: hist: ", __func__);
        for (size_t i = 0; i < hist_all.size(); i++) {
            printf("%5.3f ", hist_all[i] / float(sum_all));
        }
        printf("\n");
    }
}

//
// interface implementation
//

struct gptneox_context * gptneox_init_from_file(
                             const char * path_model,
            struct gptneox_context_params   params) {
    ggml_time_init();

    gptneox_context * ctx = new gptneox_context;

    if (params.seed <= 0) {
        params.seed = time(NULL);
    }

    unsigned cur_percentage = 0;
    if (params.progress_callback == NULL) {
        params.progress_callback_user_data = &cur_percentage;
        params.progress_callback = [](float progress, void * ctx) {
            unsigned * cur_percentage_p = (unsigned *) ctx;
            unsigned percentage = (unsigned) (100 * progress);
            while (percentage > *cur_percentage_p) {
                ++*cur_percentage_p;
                fprintf(stderr, ".");
                fflush(stderr);
                if (percentage >= 100) {
                    fprintf(stderr, "\n");
                }
            }
        };
    }

    ctx->rng = std::mt19937(params.seed);
    ctx->logits_all = params.logits_all;

    ggml_type memory_type = params.f16_kv ? GGML_TYPE_F16 : GGML_TYPE_F32;

    if (!gptneox_model_load(path_model, *ctx, params.n_ctx, memory_type,
                          params.use_mmap, params.use_mlock, params.vocab_only,
                          params.progress_callback, params.progress_callback_user_data)) {
        fprintf(stderr, "%s: failed to load model\n", __func__);
        gptneox_free(ctx);
        return nullptr;
    }

    // reserve memory for context buffers
    if (!params.vocab_only) {
        if (!kv_cache_init(ctx->model.hparams, ctx->model.kv_self, memory_type, ctx->model.hparams.n_ctx)) {
            fprintf(stderr, "%s: kv_cache_init() failed for self-attention cache\n", __func__);
            gptneox_free(ctx);
            return nullptr;
        }

        {
            const size_t memory_size = ggml_nbytes(ctx->model.kv_self.k) + ggml_nbytes(ctx->model.kv_self.v);
            fprintf(stderr, "%s: kv self size  = %7.2f MiB\n", __func__, memory_size / 1024.0 / 1024.0);
        }

        const auto & hparams = ctx->model.hparams;

        // resized during inference
        if (params.logits_all) {
            ctx->logits.reserve(hparams.n_ctx*hparams.n_vocab);
        } else {
            ctx->logits.reserve(hparams.n_vocab);
        }

        if (params.embedding){
            ctx->embedding.resize(hparams.n_embd);
        }

        ctx->buf_compute.resize(MEM_REQ_EVAL().at(ctx->model.type));

        ctx->buf_scratch[0].resize(MEM_REQ_SCRATCH0().at(ctx->model.type));
        ctx->buf_scratch[1].resize(MEM_REQ_SCRATCH1().at(ctx->model.type));
    }

    return ctx;
}

void gptneox_free(struct gptneox_context * ctx) {
    delete ctx;
}

int gptneox_model_quantize(
        const char * fname_inp,
        const char * fname_out,
  enum gptneox_ftype   ftype,
        int          nthread) {
    try {
        gptneox_model_quantize_internal(fname_inp, fname_out, ftype, nthread);
        return 0;
    } catch (const std::string & err) {
        fprintf(stderr, "%s: failed to quantize: %s\n", __func__, err.c_str());
        return 1;
    }
}

int gptneox_apply_lora_from_file_internal(struct gptneox_context * ctx, const char * path_lora, const char * path_base_model, int n_threads) {
    fprintf(stderr, "%s: applying lora adapter from '%s' - please wait ...\n", __func__, path_lora);

    auto & model = ctx->model;

    const int64_t t_start_lora_us = ggml_time_us();

    auto fin = std::ifstream(path_lora, std::ios::binary);
    if (!fin) {
        fprintf(stderr, "%s: failed to open '%s'\n", __func__, path_lora);
        return 1;
    }

    // verify magic and version
    {
        uint32_t magic;
        fin.read((char *) &magic, sizeof(magic));
        if (magic != 'ggla') {
            fprintf(stderr, "%s: bad file magic\n", __func__);
            return 1;
        }
        uint32_t format_version;
        fin.read((char *) &format_version, sizeof(format_version));

        if (format_version != 1) {
            fprintf(stderr, "%s: unsupported file version\n", __func__ );
            return 1;
        }
    }

    int32_t lora_r;
    int32_t lora_alpha;
    fin.read((char *) &lora_r, sizeof(lora_r));
    fin.read((char *) &lora_alpha, sizeof(lora_alpha));
    float scaling = (float)lora_alpha / (float)lora_r;

    fprintf(stderr, "%s: r = %d, alpha = %d, scaling = %.2f\n", __func__, lora_r, lora_alpha, scaling);


    // create a temporary ggml context to store the lora tensors
    // todo: calculate size from biggest possible tensor
    std::vector<uint8_t> lora_buf(1024ull * 1024ull * 1024ull);
    struct ggml_init_params params;
    params.mem_size   = lora_buf.size();
    params.mem_buffer = lora_buf.data();
    params.no_alloc   = false;

    ggml_context * lora_ctx = ggml_init(params);
    std::unordered_map<std::string, struct ggml_tensor *> lora_tensors;

    // create a name -> tensor map of the model to accelerate lookups
    std::unordered_map<std::string, struct ggml_tensor*> model_tensors;
    for (auto & kv: model.tensors_by_name) {
        model_tensors.insert(kv);
    }


    // load base model
    std::unique_ptr<gptneox_model_loader> model_loader;
    ggml_context * base_ctx = NULL;
    gptneox_buffer base_buf;
    if (path_base_model) {
        fprintf(stderr, "%s: loading base model from '%s'\n", __func__, path_base_model);
        model_loader.reset(new gptneox_model_loader(path_base_model, /*use_mmap*/ true, /*vocab_only*/ false));

        size_t ctx_size, mmapped_size;
        model_loader->calc_sizes(&ctx_size, &mmapped_size);
        base_buf.resize(ctx_size);

        ggml_init_params base_params;
        base_params.mem_size   = base_buf.size;
        base_params.mem_buffer = base_buf.addr;
        base_params.no_alloc   = model_loader->use_mmap;

        base_ctx = ggml_init(base_params);

        model_loader->ggml_ctx = base_ctx;

        // maybe this should in gptneox_model_loader
        if (model_loader->use_mmap) {
            model_loader->mapping.reset(new gptneox_mmap(&model_loader->file_loaders.at(0)->file, /* prefetch */ false));
        }
    }

    // read tensors and apply
    bool warned = false;
    int n_tensors = 0;
    while (true) {
        int32_t n_dims;
        int32_t length;
        int32_t ftype;

        fin.read(reinterpret_cast<char *>(&n_dims), sizeof(n_dims));
        fin.read(reinterpret_cast<char *>(&length), sizeof(length));
        fin.read(reinterpret_cast<char *>(&ftype),  sizeof(ftype));
        if (fin.eof()) {
            break;
        }

        int32_t ne[2] = { 1, 1 };
        for (int i = 0; i < n_dims; ++i) {
            fin.read(reinterpret_cast<char *>(&ne[i]), sizeof(ne[i]));
        }

        std::string name(length, 0);
        fin.read(&name[0], length);

        // check for lora suffix and get the type of tensor
        const std::string lora_suffix = ".lora";
        size_t pos = name.rfind(lora_suffix);
        if (pos == std::string::npos) {
            fprintf(stderr, "%s: error: '%s' is not a lora tensor\n", __func__, name.c_str());
            return 1;
        }

        std::string lora_type = name.substr(pos + lora_suffix.length());
        std::string base_name = name;
        base_name.erase(pos);
        // fprintf(stderr, "%s: %s => %s (lora type %s) ", __func__, name.c_str(),base_name.c_str(), lora_type.c_str());

        if (model_tensors.find(base_name.data()) == model_tensors.end()) {
            fprintf(stderr, "%s: unknown tensor '%s' in lora adapter\n", __func__, name.data());
            return 1;
        }

        // create ggml tensor
        ggml_type wtype;
        switch (ftype) {
            case 0: wtype = GGML_TYPE_F32;  break;
            case 1: wtype = GGML_TYPE_F16;  break;
            default:
                    {
                        fprintf(stderr, "%s: invalid tensor data type '%d'\n",
                                __func__, ftype);
                        return false;
                    }
        }
        ggml_tensor* lora_tensor;
        if (n_dims == 2) {
            lora_tensor = ggml_new_tensor_2d(lora_ctx, wtype, ne[0], ne[1]);
        }
        else {
            fprintf(stderr, "%s: unsupported tensor dimension %d\n", __func__, n_dims);
            return 1;
        }

        // load tensor data
        size_t offset = fin.tellg();
        size_t tensor_data_size = ggml_nbytes(lora_tensor);
        offset = (offset + 31) & -32;
        fin.seekg(offset);
        fin.read((char*)lora_tensor->data, tensor_data_size);

        lora_tensors[name] = lora_tensor;

        // check if we have both A and B tensors and apply
        if (lora_tensors.find(base_name + ".loraA") != lora_tensors.end() &&
            lora_tensors.find(base_name + ".loraB") != lora_tensors.end()) {

            ggml_tensor * dest_t = model_tensors[base_name];
            ggml_tensor * base_t;
            if (model_loader) {
                // load from base model
                if (model_loader->tensors_map.name_to_idx.find(base_name) == model_loader->tensors_map.name_to_idx.end()) {
                    fprintf(stderr, "%s: error: tensor '%s' not found in base model\n", __func__, base_name.c_str());
                    return 1;
                }
                size_t idx = model_loader->tensors_map.name_to_idx[base_name];
                gptneox_load_tensor & lt = model_loader->tensors_map.tensors[idx];
                base_t = model_loader->get_tensor(base_name, { (uint32_t)dest_t->ne[0], (uint32_t)dest_t->ne[1] });
                lt.data = (uint8_t *) lt.ggml_tensor->data;
                model_loader->load_data_for(lt);
                lt.ggml_tensor->data = lt.data;
            }
            else {
                base_t = dest_t;
            }

            if (ggml_is_quantized(base_t->type)) {
                if (!warned) {
                    fprintf(stderr, "%s: warning: using a lora adapter with a quantized model may result in poor quality, "
                                    "use a f16 or f32 base model with --lora-base\n", __func__);
                    warned = true;
                }
            }

            ggml_tensor * loraA = lora_tensors[base_name + ".loraA"];
            ggml_tensor * loraB = lora_tensors[base_name + ".loraB"];

            if (base_t->ne[0] != loraA->ne[1] || base_t->ne[1] != loraB->ne[1]) {
                fprintf(stderr, "%s: incompatible tensor dimensions (%" PRId64 " and %" PRId64 ");"
                               " are you sure that this adapter is for this model?\n", __func__, base_t->ne[0], loraA->ne[1]);
                return 1;
            }

            // w = w + BA*s
            ggml_tensor * BA = ggml_mul_mat(lora_ctx, loraA, loraB);

            if (scaling != 1.0f) {
                ggml_tensor * scale_tensor = ggml_new_f32(lora_ctx, scaling);
                BA = ggml_scale(lora_ctx, BA, scale_tensor);
            }

            ggml_tensor * r;
            if (base_t == dest_t) {
                r = ggml_add_inplace(lora_ctx, dest_t, BA);
            }
            else {
                r = ggml_add(lora_ctx, base_t, BA);
                r = ggml_cpy(lora_ctx, r, dest_t);
            }

            struct ggml_cgraph gf = ggml_build_forward(r);
            ggml_graph_compute_with_ctx(lora_ctx, &gf, n_threads);

            // we won't need these tensors again, reset the context to save memory
            ggml_free(lora_ctx);
            lora_ctx = ggml_init(params);
            lora_tensors.clear();

            n_tensors++;
            if (n_tensors % 4 == 0)
                fprintf(stderr, ".");
        }
    }

    // TODO: this should be in a destructor, it will leak on failure
    ggml_free(lora_ctx);
    if (base_ctx) {
        ggml_free(base_ctx);
    }

    const int64_t t_lora_us = ggml_time_us() - t_start_lora_us;
    fprintf(stderr, " done (%.2f ms)\n", t_lora_us / 1000.0);

    return 0;
}

int gptneox_apply_lora_from_file(struct gptneox_context * ctx, const char * path_lora, const char * path_base_model, int n_threads) {
    try {
        return gptneox_apply_lora_from_file_internal(ctx, path_lora, path_base_model, n_threads);
    } catch (const std::string & err) {
        fprintf(stderr, "%s: failed to apply lora adapter: %s\n", __func__, err.c_str());
        return 1;
    }
}

int gptneox_get_kv_cache_token_count(struct gptneox_context * ctx) {
    return ctx->model.kv_self.n;
}

#define GPTNEOX_MAX_RNG_STATE 64*1024

void gptneox_set_rng_seed(struct gptneox_context * ctx, int seed) {
    if (seed <= 0) {
        seed = time(NULL);
    }
    ctx->rng.seed(seed);
}

// Returns the size of the state
size_t gptneox_get_state_size(struct gptneox_context * ctx) {
    // we don't know size of rng until we actually serialize it. so reserve more than enough memory for its serialized state.
    // for reference, std::mt19937(1337) serializes to 6701 bytes.
    const size_t s_rng_size        = sizeof(size_t);
    const size_t s_rng             = GPTNEOX_MAX_RNG_STATE;
    const size_t s_logits_capacity = sizeof(size_t);
    const size_t s_logits_size     = sizeof(size_t);
    const size_t s_logits          = ctx->logits.capacity() * sizeof(float);
    const size_t s_embedding_size  = sizeof(size_t);
    const size_t s_embedding       = ctx->embedding.size() * sizeof(float);
    const size_t s_kv_size         = sizeof(size_t);
    const size_t s_kv_ntok         = sizeof(int);
    const size_t s_kv              = ctx->model.kv_self.buf.size;

    const size_t s_total = (
        + s_rng_size
        + s_rng
        + s_logits_capacity
        + s_logits_size
        + s_logits
        + s_embedding_size
        + s_embedding
        + s_kv_size
        + s_kv_ntok
        + s_kv
    );

    return s_total;
}

// Copies the state to the specified destination address
size_t gptneox_copy_state_data(struct gptneox_context * ctx, uint8_t * dest) {
    uint8_t * out = dest;

    // copy rng
    {
        std::stringstream rng_ss;
        rng_ss << ctx->rng;

        const size_t rng_size = rng_ss.str().size();
        char rng_buf[GPTNEOX_MAX_RNG_STATE];

        memset(&rng_buf[0], 0, GPTNEOX_MAX_RNG_STATE);
        memcpy(&rng_buf[0], rng_ss.str().data(), rng_ss.str().size());

        memcpy(out, &rng_size,   sizeof(rng_size));    out += sizeof(rng_size);
        memcpy(out, &rng_buf[0], GPTNEOX_MAX_RNG_STATE); out += GPTNEOX_MAX_RNG_STATE;
    }

    // copy logits
    {
        const size_t logits_cap  = ctx->logits.capacity();
        const size_t logits_size = ctx->logits.size();

        memcpy(out, &logits_cap,  sizeof(logits_cap));  out += sizeof(logits_cap);
        memcpy(out, &logits_size, sizeof(logits_size)); out += sizeof(logits_size);

        if (logits_size) {
            memcpy(out, ctx->logits.data(), logits_size * sizeof(float));
        }

        out += logits_cap * sizeof(float);
    }

    // copy embeddings
    {
        const size_t embedding_size = ctx->embedding.size();

        memcpy(out, &embedding_size, sizeof(embedding_size)); out += sizeof(embedding_size);

        if (embedding_size) {
            memcpy(out, ctx->embedding.data(), embedding_size * sizeof(float));
            out += embedding_size * sizeof(float);
        }
    }

    // copy kv cache
    {
        const size_t kv_size = ctx->model.kv_self.buf.size;
        const int    kv_ntok = gptneox_get_kv_cache_token_count(ctx);

        memcpy(out, &kv_size, sizeof(kv_size)); out += sizeof(kv_size);
        memcpy(out, &kv_ntok, sizeof(kv_ntok)); out += sizeof(kv_ntok);

        if (kv_size) {
            memcpy(out, ctx->model.kv_self.buf.addr, kv_size); out += kv_size;
        }
    }

    const size_t written  = out - dest;
    const size_t expected = gptneox_get_state_size(ctx);

    GPTNEOX_ASSERT(written == expected);

    return written;
}

// Sets the state reading from the specified source address
size_t gptneox_set_state_data(struct gptneox_context * ctx, const uint8_t * src) {
    const uint8_t * in = src;

    // set rng
    {
        size_t rng_size;
        char   rng_buf[GPTNEOX_MAX_RNG_STATE];

        memcpy(&rng_size,   in, sizeof(rng_size));    in += sizeof(rng_size);
        memcpy(&rng_buf[0], in, GPTNEOX_MAX_RNG_STATE); in += GPTNEOX_MAX_RNG_STATE;

        std::stringstream rng_ss;
        rng_ss.str(std::string(&rng_buf[0], rng_size));
        rng_ss >> ctx->rng;

        GPTNEOX_ASSERT(rng_ss.fail() == false);
    }

    // set logits
    {
        size_t logits_cap;
        size_t logits_size;

        memcpy(&logits_cap,  in, sizeof(logits_cap));  in += sizeof(logits_cap);
        memcpy(&logits_size, in, sizeof(logits_size)); in += sizeof(logits_size);

        GPTNEOX_ASSERT(ctx->logits.capacity() == logits_cap);

        if (logits_size) {
            ctx->logits.resize(logits_size);
            memcpy(ctx->logits.data(), in, logits_size * sizeof(float));
        }

        in += logits_cap * sizeof(float);
    }

    // set embeddings
    {
        size_t embedding_size;

        memcpy(&embedding_size, in, sizeof(embedding_size)); in += sizeof(embedding_size);

        GPTNEOX_ASSERT(ctx->embedding.capacity() == embedding_size);

        if (embedding_size) {
            memcpy(ctx->embedding.data(), in, embedding_size * sizeof(float));
            in += embedding_size * sizeof(float);
        }
    }

    // set kv cache
    {
        size_t kv_size;
        int kv_ntok;

        memcpy(&kv_size, in, sizeof(kv_size)); in += sizeof(kv_size);
        memcpy(&kv_ntok, in, sizeof(kv_ntok)); in += sizeof(kv_ntok);

        if (kv_size) {
            GPTNEOX_ASSERT(ctx->model.kv_self.buf.size == kv_size);

            void * k_data = ctx->model.kv_self.k->data; // remember data pointers
            void * v_data = ctx->model.kv_self.v->data; // because their value is stored in buf and overwritten by memcpy

            memcpy(ctx->model.kv_self.buf.addr, in, kv_size); in += kv_size;

            ctx->model.kv_self.k->data = k_data; // restore correct data pointers
            ctx->model.kv_self.v->data = v_data;

        }

        ctx->model.kv_self.n = kv_ntok;
    }

    const size_t nread    = in - src;
    const size_t expected = gptneox_get_state_size(ctx);

    GPTNEOX_ASSERT(nread == expected);

    return nread;
}

int gptneox_eval(
        struct gptneox_context * ctx,
           const gptneox_token * tokens,
                         int   n_tokens,
                         int   n_past,
                         int   n_threads) {
    if (!gptneox_eval_internal(*ctx, tokens, n_tokens, n_past, n_threads)) {
        fprintf(stderr, "%s: failed to eval\n", __func__);
        return 1;
    }
    // get a more accurate load time, upon first eval
    if (!ctx->has_evaluated_once) {
        ctx->t_load_us = ggml_time_us() - ctx->t_start_us;
        ctx->has_evaluated_once = true;
    }
    return 0;
}

int gptneox_tokenize(
        struct gptneox_context * ctx,
                  const char * text,
                 gptneox_token * tokens,
                         int   n_max_tokens,
                        bool   add_bos) {
    auto res = gptneox_tokenize(ctx->vocab, text, add_bos);

    if (n_max_tokens < (int) res.size()) {
        fprintf(stderr, "%s: too many tokens\n", __func__);
        return -((int) res.size());
    }

    for (size_t i = 0; i < res.size(); i++) {
        tokens[i] = res[i];
    }

    return res.size();
}

int gptneox_n_vocab(struct gptneox_context * ctx) {
    return ctx->vocab.id_to_token.size();
}

int gptneox_n_ctx(struct gptneox_context * ctx) {
    return ctx->model.hparams.n_ctx;
}

int gptneox_n_embd(struct gptneox_context * ctx) {
    return ctx->model.hparams.n_embd;
}

float * gptneox_get_logits(struct gptneox_context * ctx) {
    return ctx->logits.data();
}

float * gptneox_get_embeddings(struct gptneox_context * ctx) {
    return ctx->embedding.data();
}

const char * gptneox_token_to_str(struct gptneox_context * ctx, gptneox_token token) {
    if (token >= gptneox_n_vocab(ctx)) {
        return nullptr;
    }

    return ctx->vocab.id_to_token[token].tok.c_str();
}

gptneox_token gptneox_str_to_token(struct gptneox_context * ctx, const char * str) {
    return ctx->vocab.token_to_id[str];
}

gptneox_token gptneox_token_bos() {
    return 0;
}

gptneox_token gptneox_token_eos() {
    return 0;
}

// Varies depending on gptneox model, use gptneox_str_to_token instead
gptneox_token gptneox_token_nl() {
    return 13;
}


void gptneox_print_timings(struct gptneox_context * ctx) {
    const int64_t t_end_us = ggml_time_us();

    const int32_t n_sample = std::max(1, ctx->n_sample);
    const int32_t n_eval   = std::max(1, ctx->n_eval);
    const int32_t n_p_eval = std::max(1, ctx->n_p_eval);

    fprintf(stderr, "\n");
    fprintf(stderr, "%s:        load time = %8.2f ms\n", __func__, ctx->t_load_us / 1000.0);
    fprintf(stderr, "%s:      sample time = %8.2f ms / %5d runs   (%8.2f ms per run)\n",   __func__, 1e-3 * ctx->t_sample_us, n_sample, 1e-3 * ctx->t_sample_us / n_sample);
    fprintf(stderr, "%s: prompt eval time = %8.2f ms / %5d tokens (%8.2f ms per token)\n", __func__, 1e-3 * ctx->t_p_eval_us, n_p_eval, 1e-3 * ctx->t_p_eval_us / n_p_eval);
    fprintf(stderr, "%s:        eval time = %8.2f ms / %5d runs   (%8.2f ms per run)\n",   __func__, 1e-3 * ctx->t_eval_us,   n_eval,   1e-3 * ctx->t_eval_us   / n_eval);
    fprintf(stderr, "%s:       total time = %8.2f ms\n", __func__, (t_end_us - ctx->t_start_us)/1000.0);
}

void gptneox_reset_timings(struct gptneox_context * ctx) {
    ctx->t_start_us = ggml_time_us();
    ctx->t_sample_us = ctx->n_sample = 0;
    ctx->t_eval_us   = ctx->n_eval   = 0;
    ctx->t_p_eval_us = ctx->n_p_eval = 0;
}

const char * gptneox_print_system_info(void) {
    static std::string s;

    s  = "";
    s += "AVX = "         + std::to_string(ggml_cpu_has_avx())         + " | ";
    s += "AVX2 = "        + std::to_string(ggml_cpu_has_avx2())        + " | ";
    s += "AVX512 = "      + std::to_string(ggml_cpu_has_avx512())      + " | ";
    s += "AVX512_VBMI = " + std::to_string(ggml_cpu_has_avx512_vbmi()) + " | ";
    s += "AVX512_VNNI = " + std::to_string(ggml_cpu_has_avx512_vnni()) + " | ";
    s += "FMA = "         + std::to_string(ggml_cpu_has_fma())         + " | ";
    s += "NEON = "        + std::to_string(ggml_cpu_has_neon())        + " | ";
    s += "ARM_FMA = "     + std::to_string(ggml_cpu_has_arm_fma())     + " | ";
    s += "F16C = "        + std::to_string(ggml_cpu_has_f16c())        + " | ";
    s += "FP16_VA = "     + std::to_string(ggml_cpu_has_fp16_va())     + " | ";
    s += "WASM_SIMD = "   + std::to_string(ggml_cpu_has_wasm_simd())   + " | ";
    s += "BLAS = "        + std::to_string(ggml_cpu_has_blas())        + " | ";
    s += "SSE3 = "        + std::to_string(ggml_cpu_has_sse3())        + " | ";
    s += "VSX = "         + std::to_string(ggml_cpu_has_vsx())         + " | ";

    return s.c_str();
}

// For internal test use
std::vector<std::pair<std::string, struct ggml_tensor *>>& gptneox_internal_get_tensor_map(struct gptneox_context * ctx) {
    return ctx->model.tensors_by_name;
}

size_t gptneox_load_session_file(struct gptneox_context * ctx, const char * path_session, gptneox_token * tokens_out, size_t n_token_capacity, size_t * n_token_count_out) {
    // TODO leverage mmap
    gptneox_file file(path_session, "rb");
    const uint32_t magic = file.read_u32();
    const uint32_t version = file.read_u32();

    if (!(magic == 'ggsn' && version == 0)) {
        fprintf(stderr, "%s : unknown (magic, version) for session file: %08x, %08x\n", __func__, magic, version);
        return 0;
    }

    gptneox_hparams session_hparams;
    file.read_raw(&session_hparams, sizeof(gptneox_hparams));

    // REVIEW
    if (session_hparams != ctx->model.hparams) {
        fprintf(stderr, "%s : model hparams didn't match from session file!\n", __func__);
        return 0;
    }

    const uint32_t n_token_count = file.read_u32();
    GPTNEOX_ASSERT(n_token_capacity >= n_token_count);
    file.read_raw(tokens_out, sizeof(gptneox_token) * n_token_count);
    *n_token_count_out = n_token_count;

    const size_t n_state_size = file.size - file.tell();
    const size_t n_orig_state_size = gptneox_get_state_size(ctx);
    if (n_state_size != n_orig_state_size) {
        fprintf(stderr, "%s : failed to validate state size\n", __func__);
    }
    std::unique_ptr<uint8_t[]> state_data(new uint8_t[n_state_size]);
    file.read_raw(state_data.get(), n_state_size);
    return gptneox_set_state_data(ctx, state_data.get());
}

size_t gptneox_save_session_file(struct gptneox_context * ctx, const char * path_session, const gptneox_token * tokens, size_t n_token_count) {
    // TODO save temp & swap
    gptneox_file file(path_session, "wb");

    const size_t n_state_size = gptneox_get_state_size(ctx);
    std::unique_ptr<uint8_t[]> state_data(new uint8_t[n_state_size]);
    gptneox_copy_state_data(ctx, state_data.get());

    file.write_u32('ggsn'); // magic
    file.write_u32(0); // version
    file.write_raw(&ctx->model.hparams, sizeof(gptneox_hparams));

    file.write_u32((uint32_t) n_token_count); // REVIEW
    file.write_raw(tokens, sizeof(gptneox_token) * n_token_count);

    file.write_raw(state_data.get(), n_state_size);
    return n_state_size; // REVIEW
}

