# stable-diffusion

### How to build

```
$ gcc -O3 -DNDEBUG -march=native -mavx -mf16c -msse3 -ffast-math -fno-omit-frame-pointer -c ggml.c
$ g++ -o sd -O3 main.cpp stable-diffusion.cpp ggml.o
```

### How to convert the model

```
$ python3 convert.py model.safetensors --out_type f16
```

### How to use

```
$ ./sd 
error: the following arguments are required: prompt
usage: ./sd [arguments]

arguments:
  -h, --help                         show this help message and exit
  -t, --threads N                    number of threads to use during computation (default: -1).
                                     If threads <= 0, then threads will be set to the number of CPU cores
  -m, --model [MODEL]                path to model
  -o, --output OUTPUT                path to write result image to (default: .\output.png)
  -p, --prompt [PROMPT]              the prompt to render
  -n, --negative-prompt PROMPT       the negative prompt (default: "")
  --cfg-scale SCALE                  unconditional guidance scale: (default: 7.0)
  -H, --height H                     image height, in pixel space (default: 512)
  -W, --width W                      image width, in pixel space (default: 512)
  --sample-method SAMPLE_METHOD      sample method (default: "eular a")
  --steps  STEPS                     number of sample steps (default: 20)
  -s SEED, --seed SEED               RNG seed (default: 42, use random seed for < 0)
  -v, --verbose                      print extra info

$ ./sd -p "masterpiece, best quality, ultra-detailed, illustration, 1girl, solo, outdoors, camping, night, mountains, nature, stars, moon, tent, twin ponytails, green eyes, cheerful, happy, backpack, sleeping bag, camping stove, water bottle, mountain boots, gloves, sweater, hat, flashlight, forest, rocks, river, wood, smoke, shadows, contrast, clear sky, constellations, Milky Way, peaceful, serene, quiet, tranquil, remote, secluded, adventurous, exploration, escape, independence, survival, resourcefulness, challenge, perseverance, stamina, endurance, observation, intuition, adaptability, creativity, imagination, artistry, inspiration, beauty, awe, wonder, gratitude, appreciation, relaxation, enjoyment, rejuvenation, mindfulness, awareness, connection, harmony, balance, texture, detail, realism, depth, perspective, composition, color, light, shadow, reflection, refraction, tone, contrast, foreground, middle ground, background, naturalistic, figurative, representational, impressionistic, expressionistic, abstract, innovative, experimental, unique" -n "paintings, sketches, (worst quality:2), (low quality:2), (normal quality:2), lowres, ((monochrome)), ((grayscale)), skin spots, acnes, skin blemishes, age spot, manboobs, (backlight:1.2), double navel, mutad arms, hused arms, neck lace, analog, analog effects, (sunglass:1.4), bad architecture, text, (logo), (watermark)" --steps 20 --cfg-scale 7 -W 512 -H 768 -m ambientmix-half-ggml-model-f16.bin -o mountain.png
```

![Mountain Girl](mountain.png)

###

* https://github.com/leejet/stable-diffusion.cpp
