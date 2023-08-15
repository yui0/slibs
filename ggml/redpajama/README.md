# RedPajama

### How to build

```
$ make
$ ./redpajama -m ggml-japanese-gpt-neox-3.6b-instruction-ppo-q4_0.bin 
main: seed = 1692022816
gptneox.cpp: loading model from ggml-japanese-gpt-neox-3.6b-instruction-ppo-q4_0.bin
gptneox_model_load_internal: format     = ggjt v1 (latest)
gptneox_model_load_internal: n_vocab    = 32000
gptneox_model_load_internal: n_ctx      = 512
gptneox_model_load_internal: n_embd     = 2816
gptneox_model_load_internal: n_head     = 22
gptneox_model_load_internal: n_layer    = 36
gptneox_model_load_internal: n_rot      = 128
gptneox_model_load_internal: use_parallel_residual = 0
gptneox_model_load_internal: ftype      = 2 (mostly Q4_0)
gptneox_model_load_internal: n_parts    = 1
gptneox_model_load_internal: model size = 12B
gptneox_model_load_internal: ggml ctx size = 115.81 KiB
gptneox_model_load_internal: mem required  = 3987.53 MiB (+ 1608.00 MiB per state)
gptneox_init_from_file: kv self size  =  198.00 MiB

system_info: n_threads = 12 / 12 | AVX = 0 | AVX2 = 0 | AVX512 = 0 | AVX512_VBMI = 0 | AVX512_VNNI = 0 | FMA = 0 | NEON = 0 | ARM_FMA = 0 | F16C = 0 | FP16_VA = 0 | WASM_SIMD = 0 | BLAS = 0 | SSE3 = 0 | VSX = 0 | 
sampling: repeat_last_n = 64, repeat_penalty = 1.100000, presence_penalty = 0.000000, frequency_penalty = 0.000000, top_k = 40, tfs_z = 1.000000, top_p = 0.950000, typical_p = 1.000000, temp = 0.800000, mirostat = 0, mirostat_lr = 0.100000, mirostat_ent = 5.000000
generate: n_ctx = 512, n_batch = 512, n_predict = 128, n_keep = 0


<0x12>西海に面した半島は、中国や朝鮮半島との海上交通の重要な拠点であり、古代から中世にかけて大いに栄えました。近世以降、鎖国政策により貿易の中心が長崎へと移った後も、朝鮮半島や東シナ海の重要な玄関口として重要な役割を果たしています。</s>「はい」と答える人も少なくないでしょう。しかし、本当にそうなのでしょうか?確かに、多くの要因が関係しています。一般的に、健康的な体重を維持するには、定期的な運動、高カロリー摂取の制限、十分な睡眠をとることが重要です。また、定期的に健康診断を受けることも大切です。さらに、定期的に


gptneox_print_timings:        load time =   554.25 ms
gptneox_print_timings:      sample time =   105.10 ms /   128 runs   (    0.82 ms per run)
gptneox_print_timings: prompt eval time =     0.00 ms /     1 tokens (    0.00 ms per token)
gptneox_print_timings:        eval time = 50753.77 ms /   127 runs   (  399.64 ms per run)
gptneox_print_timings:       total time = 51006.33 ms
```
