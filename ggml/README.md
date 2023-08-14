# ggml

### How to build and use

```
$make

Downloading ggml model 117M ...
Done! Model '117M' saved in 'models/gpt-2-117M/ggml-model.bin'
You can now use it like this:

  $ ./bin/gpt-2 -m models/gpt-2-117M/ggml-model.bin -p "This is an example"

main: seed = 1692020226
gpt2_model_load: loading model from 'models/gpt-2-117M/ggml-model.bin'
gpt2_model_load: n_vocab = 50257
gpt2_model_load: n_ctx   = 1024
gpt2_model_load: n_embd  = 768
gpt2_model_load: n_head  = 12
gpt2_model_load: n_layer = 12
gpt2_model_load: ftype   = 1
gpt2_model_load: qntvr   = 0
gpt2_model_load: ggml tensor size = 240 bytes
gpt2_model_load: ggml ctx size = 384.77 MB
gpt2_model_load: memory size =    72.00 MB, n_mem = 12288
gpt2_model_load: model size  =   239.08 MB
extract_tests_from_file : No test file found.
test_gpt_tokenizer : 0 tests failed out of 0 tests.
main: prompt: 'She'
main: number of tokens in prompt = 1, first 8 tokens: 3347 

She, a graduate of the University of North Carolina-Charlotte and a member of the state's First Lady's Council on Women, was a political scientist at George Washington University who was elected president of the Women's Forum in 1992.

Kemp, a graduate of George Washington University, was a student at the University of Virginia's College of William and Mary and a member of the Board of Governors of the National Association of College Republicans and the National Republican Senatorial Committee.

On Wednesday, the state's attorney general's office confirmed to the Associated Press that an investigation by the Washington Post revealed that the Trump campaign tried to have sex with Kemp, who is also a lobbyist, on a hotel room in the Trump International Hotel in Bedminster, N.J., in March.

Kemp's attorney, Matthew J. Lipsky, told AP that Trump campaign officials tried to get Kemp to come to his room at Trump Tower in New York City to have sex with him

main: mem per token =  2016924 bytes
main:     load time =   118.06 ms
main:   sample time =    79.81 ms
main:  predict time =  7110.90 ms / 35.55 ms per token
main:    total time =  7442.36 ms
```
