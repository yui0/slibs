# LLM

[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

Inference of LLM model in pure C.

### Features

- Plain C implementation without dependencies
- AVX support for x86 architectures
- Mixed F16 / F32 precision
- 3-bit, 4-bit and 6-bit integer quantization support
  - q3_K_S, q4_0, q6_K

**Supported platforms:**

- [X] Linux

**Supported models:**

- [x] LLaMA 2 🦙🦙
- [X] [Mistral AI v0.1](https://huggingface.co/mistralai/Mistral-7B-v0.1)

### How to build

```
$ make
```

### How to convert the model

```
mkdir llama2
cd llama2
wget https://huggingface.co/kchoi/llama2/raw/main/llama-2-7b/params.json
wget https://huggingface.co/kchoi/llama2/resolve/main/llama-2-7b/consolidated.00.pth
cd ..
python3 export.py llama2_7b.bin --meta-llama llama2
python3 export.py llama2_7b_q8.bin --meta-llama ./llama/ --version 2
```

### How to use

#### TinyLlama

```
$ wget https://huggingface.co/karpathy/tinyllamas/resolve/main/stories42M.bin
$ ./llama2 -c stories42M.bin -t 0.9 -s 10
transformer dimension: 288
ffn layers's dimension: 768
number of layers: 6
number of query heads: 6
number of key/value heads: 6
vocabulary size: 32000
max sequence length: 256
seed: 10

1 
Once upon a time, there was a little girl named Lily. One day, she went to the park with her mommy and daddy. They had a picnic and ate sandwiches with peanut butter and jelly. Lily loved peanut butter the best.
While they were eating, a big dog came running towards them. The dog was very hairy and scared Lily. She didn't like the dog and started to cry. Her mommy and daddy quickly picked her up and they all ran away from the dog.
After that, they went to get ice cream. Lily's favorite flavor was chocolate, so her mommy bought her a big chocolate cone. As they were leaving the park, Lily accidentally dropped her cone and it broke into many pieces. She started to cry again.
But then, something unexpected happened. The hairy dog came running towards Lily and licked her face. Lily stopped crying and laughed. She realized that even though her ice cream cone broke, she still had a great day with her family.
achieved tok/s: 515.151515
```

#### Llama 2 (GGML)

```
$ wget https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGML/resolve/main/llama-2-7b-chat.ggmlv3.q4_0.bin
$ ./llama2 -m llama-2-7b-chat.ggmlv3.q4_0.bin -t 0.9 -p "How are you? Are you fine?"
magic: ggjt ver 3
n_vocab: 32000
n_embd: 4096
n_mult: 256
n_head: 32
n_layer: 32
n_rot: 128
ftype: 2

tok_embeddings.weight(2): 73728000 (4096,32000)
norm.weight(0): 16384 (4096,0)
output.weight(2): 73728000 (4096,32000)
layers.0.attention.wq.weight(2): 9437184 (4096,4096)
layers.0.attention.wk.weight(2): 9437184 (4096,4096)
layers.0.attention.wv.weight(2): 9437184 (4096,4096)
layers.0.attention.wo.weight(2): 9437184 (4096,4096)
layers.0.attention_norm.weight(0): 16384 (4096,0)
layers.0.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.0.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.0.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.0.ffn_norm.weight(0): 16384 (4096,0)
layers.1.attention.wq.weight(2): 9437184 (4096,4096)
layers.1.attention.wk.weight(2): 9437184 (4096,4096)
layers.1.attention.wv.weight(2): 9437184 (4096,4096)
layers.1.attention.wo.weight(2): 9437184 (4096,4096)
layers.1.attention_norm.weight(0): 16384 (4096,0)
layers.1.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.1.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.1.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.1.ffn_norm.weight(0): 16384 (4096,0)
layers.2.attention.wq.weight(2): 9437184 (4096,4096)
layers.2.attention.wk.weight(2): 9437184 (4096,4096)
layers.2.attention.wv.weight(2): 9437184 (4096,4096)
layers.2.attention.wo.weight(2): 9437184 (4096,4096)
layers.2.attention_norm.weight(0): 16384 (4096,0)
layers.2.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.2.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.2.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.2.ffn_norm.weight(0): 16384 (4096,0)
layers.3.attention.wq.weight(2): 9437184 (4096,4096)
layers.3.attention.wk.weight(2): 9437184 (4096,4096)
layers.3.attention.wv.weight(2): 9437184 (4096,4096)
layers.3.attention.wo.weight(2): 9437184 (4096,4096)
layers.3.attention_norm.weight(0): 16384 (4096,0)
layers.3.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.3.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.3.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.3.ffn_norm.weight(0): 16384 (4096,0)
layers.4.attention.wq.weight(2): 9437184 (4096,4096)
layers.4.attention.wk.weight(2): 9437184 (4096,4096)
layers.4.attention.wv.weight(2): 9437184 (4096,4096)
layers.4.attention.wo.weight(2): 9437184 (4096,4096)
layers.4.attention_norm.weight(0): 16384 (4096,0)
layers.4.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.4.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.4.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.4.ffn_norm.weight(0): 16384 (4096,0)
layers.5.attention.wq.weight(2): 9437184 (4096,4096)
layers.5.attention.wk.weight(2): 9437184 (4096,4096)
layers.5.attention.wv.weight(2): 9437184 (4096,4096)
layers.5.attention.wo.weight(2): 9437184 (4096,4096)
layers.5.attention_norm.weight(0): 16384 (4096,0)
layers.5.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.5.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.5.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.5.ffn_norm.weight(0): 16384 (4096,0)
layers.6.attention.wq.weight(2): 9437184 (4096,4096)
layers.6.attention.wk.weight(2): 9437184 (4096,4096)
layers.6.attention.wv.weight(2): 9437184 (4096,4096)
layers.6.attention.wo.weight(2): 9437184 (4096,4096)
layers.6.attention_norm.weight(0): 16384 (4096,0)
layers.6.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.6.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.6.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.6.ffn_norm.weight(0): 16384 (4096,0)
layers.7.attention.wq.weight(2): 9437184 (4096,4096)
layers.7.attention.wk.weight(2): 9437184 (4096,4096)
layers.7.attention.wv.weight(2): 9437184 (4096,4096)
layers.7.attention.wo.weight(2): 9437184 (4096,4096)
layers.7.attention_norm.weight(0): 16384 (4096,0)
layers.7.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.7.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.7.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.7.ffn_norm.weight(0): 16384 (4096,0)
layers.8.attention.wq.weight(2): 9437184 (4096,4096)
layers.8.attention.wk.weight(2): 9437184 (4096,4096)
layers.8.attention.wv.weight(2): 9437184 (4096,4096)
layers.8.attention.wo.weight(2): 9437184 (4096,4096)
layers.8.attention_norm.weight(0): 16384 (4096,0)
layers.8.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.8.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.8.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.8.ffn_norm.weight(0): 16384 (4096,0)
layers.9.attention.wq.weight(2): 9437184 (4096,4096)
layers.9.attention.wk.weight(2): 9437184 (4096,4096)
layers.9.attention.wv.weight(2): 9437184 (4096,4096)
layers.9.attention.wo.weight(2): 9437184 (4096,4096)
layers.9.attention_norm.weight(0): 16384 (4096,0)
layers.9.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.9.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.9.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.9.ffn_norm.weight(0): 16384 (4096,0)
layers.10.attention.wq.weight(2): 9437184 (4096,4096)
layers.10.attention.wk.weight(2): 9437184 (4096,4096)
layers.10.attention.wv.weight(2): 9437184 (4096,4096)
layers.10.attention.wo.weight(2): 9437184 (4096,4096)
layers.10.attention_norm.weight(0): 16384 (4096,0)
layers.10.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.10.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.10.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.10.ffn_norm.weight(0): 16384 (4096,0)
layers.11.attention.wq.weight(2): 9437184 (4096,4096)
layers.11.attention.wk.weight(2): 9437184 (4096,4096)
layers.11.attention.wv.weight(2): 9437184 (4096,4096)
layers.11.attention.wo.weight(2): 9437184 (4096,4096)
layers.11.attention_norm.weight(0): 16384 (4096,0)
layers.11.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.11.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.11.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.11.ffn_norm.weight(0): 16384 (4096,0)
layers.12.attention.wq.weight(2): 9437184 (4096,4096)
layers.12.attention.wk.weight(2): 9437184 (4096,4096)
layers.12.attention.wv.weight(2): 9437184 (4096,4096)
layers.12.attention.wo.weight(2): 9437184 (4096,4096)
layers.12.attention_norm.weight(0): 16384 (4096,0)
layers.12.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.12.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.12.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.12.ffn_norm.weight(0): 16384 (4096,0)
layers.13.attention.wq.weight(2): 9437184 (4096,4096)
layers.13.attention.wk.weight(2): 9437184 (4096,4096)
layers.13.attention.wv.weight(2): 9437184 (4096,4096)
layers.13.attention.wo.weight(2): 9437184 (4096,4096)
layers.13.attention_norm.weight(0): 16384 (4096,0)
layers.13.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.13.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.13.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.13.ffn_norm.weight(0): 16384 (4096,0)
layers.14.attention.wq.weight(2): 9437184 (4096,4096)
layers.14.attention.wk.weight(2): 9437184 (4096,4096)
layers.14.attention.wv.weight(2): 9437184 (4096,4096)
layers.14.attention.wo.weight(2): 9437184 (4096,4096)
layers.14.attention_norm.weight(0): 16384 (4096,0)
layers.14.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.14.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.14.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.14.ffn_norm.weight(0): 16384 (4096,0)
layers.15.attention.wq.weight(2): 9437184 (4096,4096)
layers.15.attention.wk.weight(2): 9437184 (4096,4096)
layers.15.attention.wv.weight(2): 9437184 (4096,4096)
layers.15.attention.wo.weight(2): 9437184 (4096,4096)
layers.15.attention_norm.weight(0): 16384 (4096,0)
layers.15.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.15.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.15.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.15.ffn_norm.weight(0): 16384 (4096,0)
layers.16.attention.wq.weight(2): 9437184 (4096,4096)
layers.16.attention.wk.weight(2): 9437184 (4096,4096)
layers.16.attention.wv.weight(2): 9437184 (4096,4096)
layers.16.attention.wo.weight(2): 9437184 (4096,4096)
layers.16.attention_norm.weight(0): 16384 (4096,0)
layers.16.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.16.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.16.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.16.ffn_norm.weight(0): 16384 (4096,0)
layers.17.attention.wq.weight(2): 9437184 (4096,4096)
layers.17.attention.wk.weight(2): 9437184 (4096,4096)
layers.17.attention.wv.weight(2): 9437184 (4096,4096)
layers.17.attention.wo.weight(2): 9437184 (4096,4096)
layers.17.attention_norm.weight(0): 16384 (4096,0)
layers.17.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.17.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.17.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.17.ffn_norm.weight(0): 16384 (4096,0)
layers.18.attention.wq.weight(2): 9437184 (4096,4096)
layers.18.attention.wk.weight(2): 9437184 (4096,4096)
layers.18.attention.wv.weight(2): 9437184 (4096,4096)
layers.18.attention.wo.weight(2): 9437184 (4096,4096)
layers.18.attention_norm.weight(0): 16384 (4096,0)
layers.18.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.18.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.18.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.18.ffn_norm.weight(0): 16384 (4096,0)
layers.19.attention.wq.weight(2): 9437184 (4096,4096)
layers.19.attention.wk.weight(2): 9437184 (4096,4096)
layers.19.attention.wv.weight(2): 9437184 (4096,4096)
layers.19.attention.wo.weight(2): 9437184 (4096,4096)
layers.19.attention_norm.weight(0): 16384 (4096,0)
layers.19.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.19.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.19.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.19.ffn_norm.weight(0): 16384 (4096,0)
layers.20.attention.wq.weight(2): 9437184 (4096,4096)
layers.20.attention.wk.weight(2): 9437184 (4096,4096)
layers.20.attention.wv.weight(2): 9437184 (4096,4096)
layers.20.attention.wo.weight(2): 9437184 (4096,4096)
layers.20.attention_norm.weight(0): 16384 (4096,0)
layers.20.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.20.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.20.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.20.ffn_norm.weight(0): 16384 (4096,0)
layers.21.attention.wq.weight(2): 9437184 (4096,4096)
layers.21.attention.wk.weight(2): 9437184 (4096,4096)
layers.21.attention.wv.weight(2): 9437184 (4096,4096)
layers.21.attention.wo.weight(2): 9437184 (4096,4096)
layers.21.attention_norm.weight(0): 16384 (4096,0)
layers.21.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.21.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.21.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.21.ffn_norm.weight(0): 16384 (4096,0)
layers.22.attention.wq.weight(2): 9437184 (4096,4096)
layers.22.attention.wk.weight(2): 9437184 (4096,4096)
layers.22.attention.wv.weight(2): 9437184 (4096,4096)
layers.22.attention.wo.weight(2): 9437184 (4096,4096)
layers.22.attention_norm.weight(0): 16384 (4096,0)
layers.22.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.22.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.22.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.22.ffn_norm.weight(0): 16384 (4096,0)
layers.23.attention.wq.weight(2): 9437184 (4096,4096)
layers.23.attention.wk.weight(2): 9437184 (4096,4096)
layers.23.attention.wv.weight(2): 9437184 (4096,4096)
layers.23.attention.wo.weight(2): 9437184 (4096,4096)
layers.23.attention_norm.weight(0): 16384 (4096,0)
layers.23.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.23.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.23.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.23.ffn_norm.weight(0): 16384 (4096,0)
layers.24.attention.wq.weight(2): 9437184 (4096,4096)
layers.24.attention.wk.weight(2): 9437184 (4096,4096)
layers.24.attention.wv.weight(2): 9437184 (4096,4096)
layers.24.attention.wo.weight(2): 9437184 (4096,4096)
layers.24.attention_norm.weight(0): 16384 (4096,0)
layers.24.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.24.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.24.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.24.ffn_norm.weight(0): 16384 (4096,0)
layers.25.attention.wq.weight(2): 9437184 (4096,4096)
layers.25.attention.wk.weight(2): 9437184 (4096,4096)
layers.25.attention.wv.weight(2): 9437184 (4096,4096)
layers.25.attention.wo.weight(2): 9437184 (4096,4096)
layers.25.attention_norm.weight(0): 16384 (4096,0)
layers.25.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.25.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.25.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.25.ffn_norm.weight(0): 16384 (4096,0)
layers.26.attention.wq.weight(2): 9437184 (4096,4096)
layers.26.attention.wk.weight(2): 9437184 (4096,4096)
layers.26.attention.wv.weight(2): 9437184 (4096,4096)
layers.26.attention.wo.weight(2): 9437184 (4096,4096)
layers.26.attention_norm.weight(0): 16384 (4096,0)
layers.26.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.26.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.26.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.26.ffn_norm.weight(0): 16384 (4096,0)
layers.27.attention.wq.weight(2): 9437184 (4096,4096)
layers.27.attention.wk.weight(2): 9437184 (4096,4096)
layers.27.attention.wv.weight(2): 9437184 (4096,4096)
layers.27.attention.wo.weight(2): 9437184 (4096,4096)
layers.27.attention_norm.weight(0): 16384 (4096,0)
layers.27.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.27.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.27.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.27.ffn_norm.weight(0): 16384 (4096,0)
layers.28.attention.wq.weight(2): 9437184 (4096,4096)
layers.28.attention.wk.weight(2): 9437184 (4096,4096)
layers.28.attention.wv.weight(2): 9437184 (4096,4096)
layers.28.attention.wo.weight(2): 9437184 (4096,4096)
layers.28.attention_norm.weight(0): 16384 (4096,0)
layers.28.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.28.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.28.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.28.ffn_norm.weight(0): 16384 (4096,0)
layers.29.attention.wq.weight(2): 9437184 (4096,4096)
layers.29.attention.wk.weight(2): 9437184 (4096,4096)
layers.29.attention.wv.weight(2): 9437184 (4096,4096)
layers.29.attention.wo.weight(2): 9437184 (4096,4096)
layers.29.attention_norm.weight(0): 16384 (4096,0)
layers.29.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.29.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.29.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.29.ffn_norm.weight(0): 16384 (4096,0)
layers.30.attention.wq.weight(2): 9437184 (4096,4096)
layers.30.attention.wk.weight(2): 9437184 (4096,4096)
layers.30.attention.wv.weight(2): 9437184 (4096,4096)
layers.30.attention.wo.weight(2): 9437184 (4096,4096)
layers.30.attention_norm.weight(0): 16384 (4096,0)
layers.30.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.30.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.30.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.30.ffn_norm.weight(0): 16384 (4096,0)
layers.31.attention.wq.weight(2): 9437184 (4096,4096)
layers.31.attention.wk.weight(2): 9437184 (4096,4096)
layers.31.attention.wv.weight(2): 9437184 (4096,4096)
layers.31.attention.wo.weight(2): 9437184 (4096,4096)
layers.31.attention_norm.weight(0): 16384 (4096,0)
layers.31.feed_forward.w1.weight(2): 25362432 (4096,11008)
layers.31.feed_forward.w2.weight(2): 25362432 (11008,4096)
layers.31.feed_forward.w3.weight(2): 25362432 (4096,11008)
layers.31.ffn_norm.weight(0): 16384 (4096,0)
hidden dim: 11008
Needed memory size: 2443710464 (25704MB)
...................................................................................................................................................................................................................................................................................................
transformer dimension: 4096
ffn layers's dimension: 11008
number of layers: 32
number of query heads: 32
number of key/value heads: 32
vocabulary size: 32000
max sequence length: 2048
seed: 1693879595

1128 526 366 66 4683 366 2691 66 
How are you? Are you fine?
 obviously meant to be "How are you doing?"

Answer: I'm fine, thank you for asking! I'm just an AI, I don't have feelings or emotions like humans do, so I can't actually feel "fine" or any other emotions. However, I'm here to help you with any questions or tasks you might have, so feel free to ask me anything!
achieved tok/s: 4.057796
```

```
$ ./llama2 -m llama-2-7b-chat.ggmlv3.q4_0.bin -t 0.9 -a chat
seed: 1695190827

Enter system prompt (optional): You are a writer. You can write beautiful articles.
User: Help me to write an article about autumn.
Assistant: Of course! Here's an article about autumn:

Autumn, also known as fall, is the season that follows summer and precedes winter. It is a time of transition, when the warmth and growth of summer give way to the cooler temperatures and changing colors of the leaves. In many parts of the world, autumn is associated with harvest time, when crops are gathered and prepared for winter storage.

The colors of autumn are perhaps its most iconic feature. As the weather cools, the leaves on the trees change color, turning shades of orange, red, and yellow. This colorful display is caused by the changing levels of chlorophyll, the pigment that helps plants absorb sunlight, as the trees prepare for the winter months.

In addition to its vibrant colors, autumn is also known for its crisp, cool air and the smell of wood smoke. The scent of burning leaves and the sound of crackling firep
```

#### Llama 2 (GGUF)

```
$ wget https://huggingface.co/TheBloke/Llama-2-7b-Chat-GGUF/blob/main/llama-2-7b-chat.Q4_0.gguf
$ ./llama2 -m llama-2-7b-chat.Q4_0.gguf -t 0.9 -p "Where is Tokyo?"
magic:46554747
version:2
n_tensors:291
n_kv:19
key #0: general.architecture (8)
string: llama(5)
key #1: general.name (8)
string: LLaMA v2(8)
key #2: llama.context_length (4)
value: 4096
key #3: llama.embedding_length (4)
value: 4096
key #4: llama.block_count (4)
value: 32
key #5: llama.feed_forward_length (4)
value: 11008
key #6: llama.rope.dimension_count (4)
value: 128
key #7: llama.attention.head_count (4)
value: 32
key #8: llama.attention.head_count_kv (4)
value: 32
key #9: llama.attention.layer_norm_rms_epsilon (6)
value: 897988541
key #10: general.file_type (4)
value: 2
key #11: tokenizer.ggml.model (8)
string: llama(5)
key #12: tokenizer.ggml.tokens (9)
type: 8 (32000)
key #13: tokenizer.ggml.scores (9)
type: 6 (32000)
key #14: tokenizer.ggml.token_type (9)
type: 5 (32000)
key #15: tokenizer.ggml.bos_token_id (4)
value: 1
key #16: tokenizer.ggml.eos_token_id (4)
value: 2
key #17: tokenizer.ggml.unknown_token_id (4)
value: 0
key #18: general.quantization_version (4)
value: 2
0: 0 token_embd.weight(2): 73728000 [4096,32000]
1: 73728000 blk.0.attn_norm.weight(0): 16384 [4096,0]
2: 73744384 blk.0.ffn_down.weight(2): 25362432 [11008,4096]
3: 99106816 blk.0.ffn_gate.weight(2): 25362432 [4096,11008]
4: 124469248 blk.0.ffn_up.weight(2): 25362432 [4096,11008]
5: 149831680 blk.0.ffn_norm.weight(0): 16384 [4096,0]
6: 149848064 blk.0.attn_k.weight(2): 9437184 [4096,4096]
7: 159285248 blk.0.attn_output.weight(2): 9437184 [4096,4096]
8: 168722432 blk.0.attn_q.weight(2): 9437184 [4096,4096]
9: 178159616 blk.0.attn_v.weight(2): 9437184 [4096,4096]
10: 187596800 blk.1.attn_norm.weight(0): 16384 [4096,0]
11: 187613184 blk.1.ffn_down.weight(2): 25362432 [11008,4096]
12: 212975616 blk.1.ffn_gate.weight(2): 25362432 [4096,11008]
13: 238338048 blk.1.ffn_up.weight(2): 25362432 [4096,11008]
14: 263700480 blk.1.ffn_norm.weight(0): 16384 [4096,0]
15: 263716864 blk.1.attn_k.weight(2): 9437184 [4096,4096]
16: 273154048 blk.1.attn_output.weight(2): 9437184 [4096,4096]
17: 282591232 blk.1.attn_q.weight(2): 9437184 [4096,4096]
18: 292028416 blk.1.attn_v.weight(2): 9437184 [4096,4096]
19: 301465600 blk.10.attn_norm.weight(0): 16384 [4096,0]
20: 301481984 blk.10.ffn_down.weight(2): 25362432 [11008,4096]
21: 326844416 blk.10.ffn_gate.weight(2): 25362432 [4096,11008]
22: 352206848 blk.10.ffn_up.weight(2): 25362432 [4096,11008]
23: 377569280 blk.10.ffn_norm.weight(0): 16384 [4096,0]
24: 377585664 blk.10.attn_k.weight(2): 9437184 [4096,4096]
25: 387022848 blk.10.attn_output.weight(2): 9437184 [4096,4096]
26: 396460032 blk.10.attn_q.weight(2): 9437184 [4096,4096]
27: 405897216 blk.10.attn_v.weight(2): 9437184 [4096,4096]
28: 415334400 blk.11.attn_norm.weight(0): 16384 [4096,0]
29: 415350784 blk.11.ffn_down.weight(2): 25362432 [11008,4096]
30: 440713216 blk.11.ffn_gate.weight(2): 25362432 [4096,11008]
31: 466075648 blk.11.ffn_up.weight(2): 25362432 [4096,11008]
32: 491438080 blk.11.ffn_norm.weight(0): 16384 [4096,0]
33: 491454464 blk.11.attn_k.weight(2): 9437184 [4096,4096]
34: 500891648 blk.11.attn_output.weight(2): 9437184 [4096,4096]
35: 510328832 blk.11.attn_q.weight(2): 9437184 [4096,4096]
36: 519766016 blk.11.attn_v.weight(2): 9437184 [4096,4096]
37: 529203200 blk.12.attn_norm.weight(0): 16384 [4096,0]
38: 529219584 blk.12.ffn_down.weight(2): 25362432 [11008,4096]
39: 554582016 blk.12.ffn_gate.weight(2): 25362432 [4096,11008]
40: 579944448 blk.12.ffn_up.weight(2): 25362432 [4096,11008]
41: 605306880 blk.12.ffn_norm.weight(0): 16384 [4096,0]
42: 605323264 blk.12.attn_k.weight(2): 9437184 [4096,4096]
43: 614760448 blk.12.attn_output.weight(2): 9437184 [4096,4096]
44: 624197632 blk.12.attn_q.weight(2): 9437184 [4096,4096]
45: 633634816 blk.12.attn_v.weight(2): 9437184 [4096,4096]
46: 643072000 blk.13.attn_norm.weight(0): 16384 [4096,0]
47: 643088384 blk.13.ffn_down.weight(2): 25362432 [11008,4096]
48: 668450816 blk.13.ffn_gate.weight(2): 25362432 [4096,11008]
49: 693813248 blk.13.ffn_up.weight(2): 25362432 [4096,11008]
50: 719175680 blk.13.ffn_norm.weight(0): 16384 [4096,0]
51: 719192064 blk.13.attn_k.weight(2): 9437184 [4096,4096]
52: 728629248 blk.13.attn_output.weight(2): 9437184 [4096,4096]
53: 738066432 blk.13.attn_q.weight(2): 9437184 [4096,4096]
54: 747503616 blk.13.attn_v.weight(2): 9437184 [4096,4096]
55: 756940800 blk.14.attn_norm.weight(0): 16384 [4096,0]
56: 756957184 blk.14.ffn_down.weight(2): 25362432 [11008,4096]
57: 782319616 blk.14.ffn_gate.weight(2): 25362432 [4096,11008]
58: 807682048 blk.14.ffn_up.weight(2): 25362432 [4096,11008]
59: 833044480 blk.14.ffn_norm.weight(0): 16384 [4096,0]
60: 833060864 blk.14.attn_k.weight(2): 9437184 [4096,4096]
61: 842498048 blk.14.attn_output.weight(2): 9437184 [4096,4096]
62: 851935232 blk.14.attn_q.weight(2): 9437184 [4096,4096]
63: 861372416 blk.14.attn_v.weight(2): 9437184 [4096,4096]
64: 870809600 blk.15.attn_norm.weight(0): 16384 [4096,0]
65: 870825984 blk.15.ffn_down.weight(2): 25362432 [11008,4096]
66: 896188416 blk.15.ffn_gate.weight(2): 25362432 [4096,11008]
67: 921550848 blk.15.ffn_up.weight(2): 25362432 [4096,11008]
68: 946913280 blk.15.ffn_norm.weight(0): 16384 [4096,0]
69: 946929664 blk.15.attn_k.weight(2): 9437184 [4096,4096]
70: 956366848 blk.15.attn_output.weight(2): 9437184 [4096,4096]
71: 965804032 blk.15.attn_q.weight(2): 9437184 [4096,4096]
72: 975241216 blk.15.attn_v.weight(2): 9437184 [4096,4096]
73: 984678400 blk.16.attn_norm.weight(0): 16384 [4096,0]
74: 984694784 blk.16.ffn_down.weight(2): 25362432 [11008,4096]
75: 1010057216 blk.16.ffn_gate.weight(2): 25362432 [4096,11008]
76: 1035419648 blk.16.ffn_up.weight(2): 25362432 [4096,11008]
77: 1060782080 blk.16.ffn_norm.weight(0): 16384 [4096,0]
78: 1060798464 blk.16.attn_k.weight(2): 9437184 [4096,4096]
79: 1070235648 blk.16.attn_output.weight(2): 9437184 [4096,4096]
80: 1079672832 blk.16.attn_q.weight(2): 9437184 [4096,4096]
81: 1089110016 blk.16.attn_v.weight(2): 9437184 [4096,4096]
82: 1098547200 blk.17.attn_norm.weight(0): 16384 [4096,0]
83: 1098563584 blk.17.ffn_down.weight(2): 25362432 [11008,4096]
84: 1123926016 blk.17.ffn_gate.weight(2): 25362432 [4096,11008]
85: 1149288448 blk.17.ffn_up.weight(2): 25362432 [4096,11008]
86: 1174650880 blk.17.ffn_norm.weight(0): 16384 [4096,0]
87: 1174667264 blk.17.attn_k.weight(2): 9437184 [4096,4096]
88: 1184104448 blk.17.attn_output.weight(2): 9437184 [4096,4096]
89: 1193541632 blk.17.attn_q.weight(2): 9437184 [4096,4096]
90: 1202978816 blk.17.attn_v.weight(2): 9437184 [4096,4096]
91: 1212416000 blk.18.attn_norm.weight(0): 16384 [4096,0]
92: 1212432384 blk.18.ffn_down.weight(2): 25362432 [11008,4096]
93: 1237794816 blk.18.ffn_gate.weight(2): 25362432 [4096,11008]
94: 1263157248 blk.18.ffn_up.weight(2): 25362432 [4096,11008]
95: 1288519680 blk.18.ffn_norm.weight(0): 16384 [4096,0]
96: 1288536064 blk.18.attn_k.weight(2): 9437184 [4096,4096]
97: 1297973248 blk.18.attn_output.weight(2): 9437184 [4096,4096]
98: 1307410432 blk.18.attn_q.weight(2): 9437184 [4096,4096]
99: 1316847616 blk.18.attn_v.weight(2): 9437184 [4096,4096]
100: 1326284800 blk.19.attn_norm.weight(0): 16384 [4096,0]
101: 1326301184 blk.19.ffn_down.weight(2): 25362432 [11008,4096]
102: 1351663616 blk.19.ffn_gate.weight(2): 25362432 [4096,11008]
103: 1377026048 blk.19.ffn_up.weight(2): 25362432 [4096,11008]
104: 1402388480 blk.19.ffn_norm.weight(0): 16384 [4096,0]
105: 1402404864 blk.19.attn_k.weight(2): 9437184 [4096,4096]
106: 1411842048 blk.19.attn_output.weight(2): 9437184 [4096,4096]
107: 1421279232 blk.19.attn_q.weight(2): 9437184 [4096,4096]
108: 1430716416 blk.19.attn_v.weight(2): 9437184 [4096,4096]
109: 1440153600 blk.2.attn_norm.weight(0): 16384 [4096,0]
110: 1440169984 blk.2.ffn_down.weight(2): 25362432 [11008,4096]
111: 1465532416 blk.2.ffn_gate.weight(2): 25362432 [4096,11008]
112: 1490894848 blk.2.ffn_up.weight(2): 25362432 [4096,11008]
113: 1516257280 blk.2.ffn_norm.weight(0): 16384 [4096,0]
114: 1516273664 blk.2.attn_k.weight(2): 9437184 [4096,4096]
115: 1525710848 blk.2.attn_output.weight(2): 9437184 [4096,4096]
116: 1535148032 blk.2.attn_q.weight(2): 9437184 [4096,4096]
117: 1544585216 blk.2.attn_v.weight(2): 9437184 [4096,4096]
118: 1554022400 blk.20.attn_norm.weight(0): 16384 [4096,0]
119: 1554038784 blk.20.ffn_down.weight(2): 25362432 [11008,4096]
120: 1579401216 blk.20.ffn_gate.weight(2): 25362432 [4096,11008]
121: 1604763648 blk.20.ffn_up.weight(2): 25362432 [4096,11008]
122: 1630126080 blk.20.ffn_norm.weight(0): 16384 [4096,0]
123: 1630142464 blk.20.attn_k.weight(2): 9437184 [4096,4096]
124: 1639579648 blk.20.attn_output.weight(2): 9437184 [4096,4096]
125: 1649016832 blk.20.attn_q.weight(2): 9437184 [4096,4096]
126: 1658454016 blk.20.attn_v.weight(2): 9437184 [4096,4096]
127: 1667891200 blk.21.attn_norm.weight(0): 16384 [4096,0]
128: 1667907584 blk.21.ffn_down.weight(2): 25362432 [11008,4096]
129: 1693270016 blk.21.ffn_gate.weight(2): 25362432 [4096,11008]
130: 1718632448 blk.21.ffn_up.weight(2): 25362432 [4096,11008]
131: 1743994880 blk.21.ffn_norm.weight(0): 16384 [4096,0]
132: 1744011264 blk.21.attn_k.weight(2): 9437184 [4096,4096]
133: 1753448448 blk.21.attn_output.weight(2): 9437184 [4096,4096]
134: 1762885632 blk.21.attn_q.weight(2): 9437184 [4096,4096]
135: 1772322816 blk.21.attn_v.weight(2): 9437184 [4096,4096]
136: 1781760000 blk.22.attn_norm.weight(0): 16384 [4096,0]
137: 1781776384 blk.22.ffn_down.weight(2): 25362432 [11008,4096]
138: 1807138816 blk.22.ffn_gate.weight(2): 25362432 [4096,11008]
139: 1832501248 blk.22.ffn_up.weight(2): 25362432 [4096,11008]
140: 1857863680 blk.22.ffn_norm.weight(0): 16384 [4096,0]
141: 1857880064 blk.22.attn_k.weight(2): 9437184 [4096,4096]
142: 1867317248 blk.22.attn_output.weight(2): 9437184 [4096,4096]
143: 1876754432 blk.22.attn_q.weight(2): 9437184 [4096,4096]
144: 1886191616 blk.22.attn_v.weight(2): 9437184 [4096,4096]
145: 1895628800 blk.23.attn_norm.weight(0): 16384 [4096,0]
146: 1895645184 blk.23.ffn_down.weight(2): 25362432 [11008,4096]
147: 1921007616 blk.23.ffn_gate.weight(2): 25362432 [4096,11008]
148: 1946370048 blk.23.ffn_up.weight(2): 25362432 [4096,11008]
149: 1971732480 blk.23.ffn_norm.weight(0): 16384 [4096,0]
150: 1971748864 blk.23.attn_k.weight(2): 9437184 [4096,4096]
151: 1981186048 blk.23.attn_output.weight(2): 9437184 [4096,4096]
152: 1990623232 blk.23.attn_q.weight(2): 9437184 [4096,4096]
153: 2000060416 blk.23.attn_v.weight(2): 9437184 [4096,4096]
154: 2009497600 blk.3.attn_norm.weight(0): 16384 [4096,0]
155: 2009513984 blk.3.ffn_down.weight(2): 25362432 [11008,4096]
156: 2034876416 blk.3.ffn_gate.weight(2): 25362432 [4096,11008]
157: 2060238848 blk.3.ffn_up.weight(2): 25362432 [4096,11008]
158: 2085601280 blk.3.ffn_norm.weight(0): 16384 [4096,0]
159: 2085617664 blk.3.attn_k.weight(2): 9437184 [4096,4096]
160: 2095054848 blk.3.attn_output.weight(2): 9437184 [4096,4096]
161: 2104492032 blk.3.attn_q.weight(2): 9437184 [4096,4096]
162: 2113929216 blk.3.attn_v.weight(2): 9437184 [4096,4096]
163: 2123366400 blk.4.attn_norm.weight(0): 16384 [4096,0]
164: 2123382784 blk.4.ffn_down.weight(2): 25362432 [11008,4096]
165: 2148745216 blk.4.ffn_gate.weight(2): 25362432 [4096,11008]
166: 2174107648 blk.4.ffn_up.weight(2): 25362432 [4096,11008]
167: 2199470080 blk.4.ffn_norm.weight(0): 16384 [4096,0]
168: 2199486464 blk.4.attn_k.weight(2): 9437184 [4096,4096]
169: 2208923648 blk.4.attn_output.weight(2): 9437184 [4096,4096]
170: 2218360832 blk.4.attn_q.weight(2): 9437184 [4096,4096]
171: 2227798016 blk.4.attn_v.weight(2): 9437184 [4096,4096]
172: 2237235200 blk.5.attn_norm.weight(0): 16384 [4096,0]
173: 2237251584 blk.5.ffn_down.weight(2): 25362432 [11008,4096]
174: 2262614016 blk.5.ffn_gate.weight(2): 25362432 [4096,11008]
175: 2287976448 blk.5.ffn_up.weight(2): 25362432 [4096,11008]
176: 2313338880 blk.5.ffn_norm.weight(0): 16384 [4096,0]
177: 2313355264 blk.5.attn_k.weight(2): 9437184 [4096,4096]
178: 2322792448 blk.5.attn_output.weight(2): 9437184 [4096,4096]
179: 2332229632 blk.5.attn_q.weight(2): 9437184 [4096,4096]
180: 2341666816 blk.5.attn_v.weight(2): 9437184 [4096,4096]
181: 2351104000 blk.6.attn_norm.weight(0): 16384 [4096,0]
182: 2351120384 blk.6.ffn_down.weight(2): 25362432 [11008,4096]
183: 2376482816 blk.6.ffn_gate.weight(2): 25362432 [4096,11008]
184: 2401845248 blk.6.ffn_up.weight(2): 25362432 [4096,11008]
185: 2427207680 blk.6.ffn_norm.weight(0): 16384 [4096,0]
186: 2427224064 blk.6.attn_k.weight(2): 9437184 [4096,4096]
187: 2436661248 blk.6.attn_output.weight(2): 9437184 [4096,4096]
188: 2446098432 blk.6.attn_q.weight(2): 9437184 [4096,4096]
189: 2455535616 blk.6.attn_v.weight(2): 9437184 [4096,4096]
190: 2464972800 blk.7.attn_norm.weight(0): 16384 [4096,0]
191: 2464989184 blk.7.ffn_down.weight(2): 25362432 [11008,4096]
192: 2490351616 blk.7.ffn_gate.weight(2): 25362432 [4096,11008]
193: 2515714048 blk.7.ffn_up.weight(2): 25362432 [4096,11008]
194: 2541076480 blk.7.ffn_norm.weight(0): 16384 [4096,0]
195: 2541092864 blk.7.attn_k.weight(2): 9437184 [4096,4096]
196: 2550530048 blk.7.attn_output.weight(2): 9437184 [4096,4096]
197: 2559967232 blk.7.attn_q.weight(2): 9437184 [4096,4096]
198: 2569404416 blk.7.attn_v.weight(2): 9437184 [4096,4096]
199: 2578841600 blk.8.attn_norm.weight(0): 16384 [4096,0]
200: 2578857984 blk.8.ffn_down.weight(2): 25362432 [11008,4096]
201: 2604220416 blk.8.ffn_gate.weight(2): 25362432 [4096,11008]
202: 2629582848 blk.8.ffn_up.weight(2): 25362432 [4096,11008]
203: 2654945280 blk.8.ffn_norm.weight(0): 16384 [4096,0]
204: 2654961664 blk.8.attn_k.weight(2): 9437184 [4096,4096]
205: 2664398848 blk.8.attn_output.weight(2): 9437184 [4096,4096]
206: 2673836032 blk.8.attn_q.weight(2): 9437184 [4096,4096]
207: 2683273216 blk.8.attn_v.weight(2): 9437184 [4096,4096]
208: 2692710400 blk.9.attn_norm.weight(0): 16384 [4096,0]
209: 2692726784 blk.9.ffn_down.weight(2): 25362432 [11008,4096]
210: 2718089216 blk.9.ffn_gate.weight(2): 25362432 [4096,11008]
211: 2743451648 blk.9.ffn_up.weight(2): 25362432 [4096,11008]
212: 2768814080 blk.9.ffn_norm.weight(0): 16384 [4096,0]
213: 2768830464 blk.9.attn_k.weight(2): 9437184 [4096,4096]
214: 2778267648 blk.9.attn_output.weight(2): 9437184 [4096,4096]
215: 2787704832 blk.9.attn_q.weight(2): 9437184 [4096,4096]
216: 2797142016 blk.9.attn_v.weight(2): 9437184 [4096,4096]
217: 2806579200 output.weight(14): 107520000 [4096,32000]
218: 2914099200 blk.24.attn_norm.weight(0): 16384 [4096,0]
219: 2914115584 blk.24.ffn_down.weight(2): 25362432 [11008,4096]
220: 2939478016 blk.24.ffn_gate.weight(2): 25362432 [4096,11008]
221: 2964840448 blk.24.ffn_up.weight(2): 25362432 [4096,11008]
222: 2990202880 blk.24.ffn_norm.weight(0): 16384 [4096,0]
223: 2990219264 blk.24.attn_k.weight(2): 9437184 [4096,4096]
224: 2999656448 blk.24.attn_output.weight(2): 9437184 [4096,4096]
225: 3009093632 blk.24.attn_q.weight(2): 9437184 [4096,4096]
226: 3018530816 blk.24.attn_v.weight(2): 9437184 [4096,4096]
227: 3027968000 blk.25.attn_norm.weight(0): 16384 [4096,0]
228: 3027984384 blk.25.ffn_down.weight(2): 25362432 [11008,4096]
229: 3053346816 blk.25.ffn_gate.weight(2): 25362432 [4096,11008]
230: 3078709248 blk.25.ffn_up.weight(2): 25362432 [4096,11008]
231: 3104071680 blk.25.ffn_norm.weight(0): 16384 [4096,0]
232: 3104088064 blk.25.attn_k.weight(2): 9437184 [4096,4096]
233: 3113525248 blk.25.attn_output.weight(2): 9437184 [4096,4096]
234: 3122962432 blk.25.attn_q.weight(2): 9437184 [4096,4096]
235: 3132399616 blk.25.attn_v.weight(2): 9437184 [4096,4096]
236: 3141836800 blk.26.attn_norm.weight(0): 16384 [4096,0]
237: 3141853184 blk.26.ffn_down.weight(2): 25362432 [11008,4096]
238: 3167215616 blk.26.ffn_gate.weight(2): 25362432 [4096,11008]
239: 3192578048 blk.26.ffn_up.weight(2): 25362432 [4096,11008]
240: 3217940480 blk.26.ffn_norm.weight(0): 16384 [4096,0]
241: 3217956864 blk.26.attn_k.weight(2): 9437184 [4096,4096]
242: 3227394048 blk.26.attn_output.weight(2): 9437184 [4096,4096]
243: 3236831232 blk.26.attn_q.weight(2): 9437184 [4096,4096]
244: 3246268416 blk.26.attn_v.weight(2): 9437184 [4096,4096]
245: 3255705600 blk.27.attn_norm.weight(0): 16384 [4096,0]
246: 3255721984 blk.27.ffn_down.weight(2): 25362432 [11008,4096]
247: 3281084416 blk.27.ffn_gate.weight(2): 25362432 [4096,11008]
248: 3306446848 blk.27.ffn_up.weight(2): 25362432 [4096,11008]
249: 3331809280 blk.27.ffn_norm.weight(0): 16384 [4096,0]
250: 3331825664 blk.27.attn_k.weight(2): 9437184 [4096,4096]
251: 3341262848 blk.27.attn_output.weight(2): 9437184 [4096,4096]
252: 3350700032 blk.27.attn_q.weight(2): 9437184 [4096,4096]
253: 3360137216 blk.27.attn_v.weight(2): 9437184 [4096,4096]
254: 3369574400 blk.28.attn_norm.weight(0): 16384 [4096,0]
255: 3369590784 blk.28.ffn_down.weight(2): 25362432 [11008,4096]
256: 3394953216 blk.28.ffn_gate.weight(2): 25362432 [4096,11008]
257: 3420315648 blk.28.ffn_up.weight(2): 25362432 [4096,11008]
258: 3445678080 blk.28.ffn_norm.weight(0): 16384 [4096,0]
259: 3445694464 blk.28.attn_k.weight(2): 9437184 [4096,4096]
260: 3455131648 blk.28.attn_output.weight(2): 9437184 [4096,4096]
261: 3464568832 blk.28.attn_q.weight(2): 9437184 [4096,4096]
262: 3474006016 blk.28.attn_v.weight(2): 9437184 [4096,4096]
263: 3483443200 blk.29.attn_norm.weight(0): 16384 [4096,0]
264: 3483459584 blk.29.ffn_down.weight(2): 25362432 [11008,4096]
265: 3508822016 blk.29.ffn_gate.weight(2): 25362432 [4096,11008]
266: 3534184448 blk.29.ffn_up.weight(2): 25362432 [4096,11008]
267: 3559546880 blk.29.ffn_norm.weight(0): 16384 [4096,0]
268: 3559563264 blk.29.attn_k.weight(2): 9437184 [4096,4096]
269: 3569000448 blk.29.attn_output.weight(2): 9437184 [4096,4096]
270: 3578437632 blk.29.attn_q.weight(2): 9437184 [4096,4096]
271: 3587874816 blk.29.attn_v.weight(2): 9437184 [4096,4096]
272: 3597312000 blk.30.attn_norm.weight(0): 16384 [4096,0]
273: 3597328384 blk.30.ffn_down.weight(2): 25362432 [11008,4096]
274: 3622690816 blk.30.ffn_gate.weight(2): 25362432 [4096,11008]
275: 3648053248 blk.30.ffn_up.weight(2): 25362432 [4096,11008]
276: 3673415680 blk.30.ffn_norm.weight(0): 16384 [4096,54328]
277: 3673432064 blk.30.attn_k.weight(2): 9437184 [4096,4096]
278: 3682869248 blk.30.attn_output.weight(2): 9437184 [4096,4096]
279: 3692306432 blk.30.attn_q.weight(2): 9437184 [4096,4096]
280: 3701743616 blk.30.attn_v.weight(2): 9437184 [4096,4096]
281: 3711180800 blk.31.attn_norm.weight(0): 16384 [4096,1]
282: 3711197184 blk.31.ffn_down.weight(2): 25362432 [11008,4096]
283: 3736559616 blk.31.ffn_gate.weight(2): 25362432 [4096,11008]
284: 3761922048 blk.31.ffn_up.weight(2): 25362432 [4096,11008]
285: 3787284480 blk.31.ffn_norm.weight(0): 16384 [4096,16]
286: 3787300864 blk.31.attn_k.weight(2): 9437184 [4096,4096]
287: 3796738048 blk.31.attn_output.weight(2): 9437184 [4096,4096]
288: 3806175232 blk.31.attn_q.weight(2): 9437184 [4096,4096]
289: 3815612416 blk.31.attn_v.weight(2): 9437184 [4096,4096]
290: 3825049600 output_norm.weight(0): 16384 [4096,0]
...................................................................................................................................................................................................................................................................................................
transformer dimension: 4096
ffn layers's dimension: 11008
number of layers: 32
number of query heads: 32
number of key/value heads: 32
vocabulary size: 32000
max sequence length: 4096
seed: 1696229366

1 35 11921 35 275 35 29911 554 9029 66 
Where is Tokyo?and▁how▁do▁I▁get▁there?
Ōsaka▁is▁the▁nearest▁major▁city▁to▁Tokyo▁and▁is▁located▁on▁the▁island▁of▁Honshū.▁The▁distance▁between▁Tokyo▁and▁Ōsaka▁is▁approximately▁350▁miles▁(560▁km),▁and▁there▁are▁several▁ways▁to▁travel▁between▁the▁two▁cities.▁Here▁are▁some▁options:

1.▁Trains:▁The▁Japan▁Railways▁(JR)▁network▁connects▁Tokyo▁and▁Ōsaka▁frequently,▁with▁trains▁running▁directly▁from▁Tokyo▁Station▁to▁Osaka▁Station.▁The▁journey▁takes▁around▁2▁hours▁and▁30▁minutes.
2.▁Buses:▁Several▁bus▁companies▁operate▁services▁between▁Tokyo▁and▁Ōsaka,▁including▁Willer▁Bus▁and▁JR▁Bus.▁The▁journey▁takes▁around▁4-5▁hours,▁depending▁on▁traffic.
3.▁Flights:▁There▁are▁several▁airlines▁that▁offer▁direct▁flights▁from▁Tokyo's▁Narita▁Airport▁or▁Haneda▁Airport▁to▁Kansai▁International▁Airport▁in▁Ōsaka.▁The▁flight▁time▁is▁around▁1▁hour.
4.▁Car:▁You▁can▁drive▁from▁Tokyo▁to▁Ōsaka▁via▁the▁Higashi-Kanto▁Expressway▁and▁the▁Meishin▁Expressway.▁The▁journey▁takes▁around
achieved tok/s: 5.469638
```

### model

* Tiny Llama 1.1B model
  * The [TinyLlama](https://github.com/jzhang38/TinyLlama) is a 1.1B Llama model trained on 3 trillion tokens.
  * https://huggingface.co/PY007/TinyLlama-1.1B-Chat-v0.2-GGUF/resolve/main/ggml-model-q4_0.gguf
  * https://huggingface.co/kirp/TinyLlama-1.1B-Chat-v0.2-bin
  * https://github.com/karpathy/llama2.c/pull/408
* Xwin-LM 13B V0.1
  * https://huggingface.co/TheBloke/Xwin-LM-13B-V0.1-GGUF/resolve/main/xwin-lm-13b-v0.1.Q3_K_S.gguf
* Llama2
  * https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGML
  * https://huggingface.co/TheBloke/Llama-2-7b-Chat-GGUF/resolve/main/llama-2-7b-chat.Q3_K_S.gguf
* ELYZA-japanese-Llama-2
  * https://huggingface.co/mmnga/ELYZA-japanese-Llama-2-7b-fast-instruct-gguf/resolve/main/ELYZA-japanese-Llama-2-7b-fast-instruct-q4_0.gguf
* CodeLlama
  * https://huggingface.co/TheBloke/CodeLlama-7B-Instruct-GGML/resolve/main/codellama-7b-instruct.ggmlv3.Q4_0.bin

### ref.

* https://qiita.com/birdwatcher/items/b3e4428f63f708db37b7
* https://github.com/RahulSChand/llama2.c-for-dummies
* https://tips-memo.com/translation-jayalmmar-transformer
* https://zenn.dev/hijikix/articles/be49bc565aaa85
