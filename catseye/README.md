# Llama 2

### How to build

```
$ make
$ wget https://huggingface.co/karpathy/tinyllamas/resolve/main/stories42M.bin
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

```
$ ./llama2 -c out/model.bin -t 0.9 -s 256 -p "How are you? Are you fine?"
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

$ ./llama2 -m llama-2-7b-chat.ggmlv3.q4_0.bin -t 0.9 -a chat
seed: 1695190827

Enter system prompt (optional): You are a writer. You can write beautiful articles.
User: Help me to write an article about autumn.
Assistant: Of course! Here's an article about autumn:

Autumn, also known as fall, is the season that follows summer and precedes winter. It is a time of transition, when the warmth and growth of summer give way to the cooler temperatures and changing colors of the leaves. In many parts of the world, autumn is associated with harvest time, when crops are gathered and prepared for winter storage.

The colors of autumn are perhaps its most iconic feature. As the weather cools, the leaves on the trees change color, turning shades of orange, red, and yellow. This colorful display is caused by the changing levels of chlorophyll, the pigment that helps plants absorb sunlight, as the trees prepare for the winter months.

In addition to its vibrant colors, autumn is also known for its crisp, cool air and the smell of wood smoke. The scent of burning leaves and the sound of crackling firep
```

### ref.

* https://qiita.com/birdwatcher/items/b3e4428f63f708db37b7
* https://github.com/RahulSChand/llama2.c-for-dummies
* https://tips-memo.com/translation-jayalmmar-transformer
* https://zenn.dev/hijikix/articles/be49bc565aaa85
