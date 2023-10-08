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
...
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
...
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
* Mistral
  * https://github.com/mistralai/mistral-src
  * https://huggingface.co/TheBloke/Mistral-7B-Instruct-v0.1-GGUF
* ELYZA-japanese-Llama-2
  * https://huggingface.co/mmnga/ELYZA-japanese-Llama-2-7b-fast-instruct-gguf/resolve/main/ELYZA-japanese-Llama-2-7b-fast-instruct-q4_0.gguf
* CodeLlama
  * https://huggingface.co/TheBloke/CodeLlama-7B-Instruct-GGML/resolve/main/codellama-7b-instruct.ggmlv3.Q4_0.bin

### ref.

* https://qiita.com/birdwatcher/items/b3e4428f63f708db37b7
* https://github.com/RahulSChand/llama2.c-for-dummies
* https://tips-memo.com/translation-jayalmmar-transformer
* https://zenn.dev/hijikix/articles/be49bc565aaa85
