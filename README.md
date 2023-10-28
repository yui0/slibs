# slibs

![GitHub Repo stars](https://img.shields.io/github/stars/yui0/slibs?style=social)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/yui0/slibs)
![Lines of code](https://img.shields.io/tokei/lines/github/yui0/slibs)

Single file libraries for C/C++.

|library | category | LoC | license | description
| --------------------- | -------- | --- | --- | --------------------------------
|**[alsa.h](alsa.h)** | audio | 138 | public domain | [ALSA library](https://github.com/yui0/aplay-)
|**[uaac.h](uaac.h)** | audio | 13438 | RPSL | [AAC audio decoder](https://github.com/yui0/aplay-)
|**[uflac_enc.h](uflac_enc.h)** | audio | 2843 | LGPL-2.1 | FLAC audio encoder based on Flake
|**[wav.h](wav.h)** | audio | 61 | public domain | Wave format
|**[wave.h](wave.h)** | audio | 304 | public domain | utility for wave format
|**[minimp3.h](minimp3.h)** | audio | 2671 | LGPL | [MP3 decoder](https://oldforum.puppylinux.com/viewtopic.php?t=59417)
| --------------------- | -------- | --- | --- | --------------------------------
|**[v4l2.h](v4l2.h)** | video | 614 | public domain | video for linux
|**[mp4parser.h](mp4parser.h)** | video | 6015 | LGPL-3.0 | mp4 parser
|**[jo_mpeg.h](jo_mpeg.h)** | video | 317 | public domain | [MPEG1/2 encoder](http://www.jonolick.com/home/mpeg-video-writer)
| --------------------- | -------- | --- | --- | --------------------------------
|**[imgp.h](imgp.h)** | image | 441 | public domain | mini image processing library
|**[potracelib.h](potracelib.h)** | <details><summary>image</summary>[Document](http://potrace.sourceforge.net/potracelib.pdf), [Example](https://github.com/yui0/img2vec)</details> | 2466 | GPL | [Transforming bitmaps into vector graphics](http://potrace.sourceforge.net/)
|**[pssub.h](pssub.h)** | image | 25689 | unknown | Postscript library
|**[svg.h](svg.h)** | image | 21735 | Apache-2.0 | SVG library
| --------------------- | -------- | --- | --- | --------------------------------
|**[kms-glsl.h](kms-glsl.h)** | OpenGL | 2426 | MIT | [OpenGL using the DRM/KMS](https://ttt.io/glsl-raspberry-pi)
| --------------------- | -------- | --- | --- | --------------------------------
|**[catseye_llama2.h](catseye/catseye_llama2.h)** | math | 1874 | MIT | Llama2 for CPU
|**[fft.h](fft.h)** | math | 2244 | public domain | Fast Fourier Transform
|**[ifft.h](fft.h)** | math | 19875 | unknown | Fixed-point in-place Fast Fourier Transform
|**[fmath.h](fmath.h)** | math | 142 | public domain | Fast math library
|**[random.h](random.h)** | math | 33 | public domain | XOR128 random function
|**[ggml.h](ggml/ggml.h)** | <details><summary>math</summary>Llama 2, RedPajama, Stable Diffusion</details> | 20352 | MIT | [Tensor library for machine learning](https://github.com/ggerganov/ggml)
| --------------------- | -------- | --- | --- | --------------------------------
|**[sgemm_sse.h](sgemm_sse.h)** | math | 451 | public domain | [Fast GEMM function for SSE](https://github.com/yui0/ugemm)
|**[sgemm_ocl.h](sgemm_ocl.h)** | GPGPU/math | 662 | public domain | [Fast GEMM function for OpenCL](https://github.com/yui0/ugemm)
|**[sgemm_gl.h](sgemm_gl.h)** | GPGPU/math | 110 | public domain | [Fast GEMM function for OpenGL4](https://github.com/yui0/ugemm)
|**[gpgpu_glsl.h](gpgpu_glsl.h)** | GPGPU | 399 | public domain | [GLSL frameworks](https://github.com/yui0/waifu2x-glsl)
|**[gpgpu_gles3.h](gpgpu_gles3.h)** | GPGPU | 264 | public domain | [GLES frameworks with TransformFeedback](https://github.com/yui0/waifu2x-glsl)
|**[gpgpu_gl4.h](gpgpu_gl4.h)** | GPGPU | 170 | public domain | [OpenGL compute shader frameworks](https://github.com/yui0/waifu2x-glsl)
|**[ocl.h](ocl.h)** | GPGPU | 190 | public domain | [OpenCL frameworks](https://github.com/yui0/waifu2x-ocl)
| --------------------- | -------- | --- | --- | --------------------------------
|**[alloc.h](alloc.h)** | utils | 116 | MIT | Safety alloc
|**[clock.h](clock.h)** | <details><summary>utils</summary>[ref.](https://github.com/yui0/waifu2x-glsl)</details> | 31 | public domain | clock function for linux
|**[ht.h](ht.h)** | utils | 330 | MIT | hash table
|**[kbhit.h](kbhit.h)** | <details><summary>utils</summary>[ref.](https://github.com/yui0/aplay-)</details> | 64 | public domain | kbhit function for linux
|**[ls.h](ls.h)** | utils | 155 | <details><summary>public domain</summary>[ref.](https://github.com/yui0/aplay-)</details> | making list of files and directories
| --------------------- | -------- | --- | --- | --------------------------------
|**[aimage.h](aimage.h)** | utils | 321 | public domain | Image Viewer for Console
|**[mcurses.h](mcurses.h)** | <details><summary>utils</summary>[ref.](https://github.com/yui0/aplay-)</details> | 1481 | GPL-2 | "[mcurses](https://github.com/ChrisMicro/mcurses)" is a minimized version of programming library ncurses
|**[termbox.h](termbox.h)** | <details><summary>utils</summary>[ref.](https://github.com/nsf/termbox)</details> | 1719 | MIT | Library for writing text-based user interfaces
| --------------------- | -------- | --- | --- | --------------------------------
|**[csv.h](csv.h)** | utils | 77 | public domain | CSV file parser
|**[ini.h](ini.h)** | utils | 250 | BSD | ini file parser
|**[parg.h](parg.h)** | <details><summary>utils</summary>[ref.](https://github.com/jibsen/parg)</details> | 551 | CC0 | Parser for argv that works similarly to getopt
|**[parson.h](parson.h)** | <details><summary>utils</summary>[ref.](https://github.com/kgabis/parson)</details> | 2239 | MIT | Lightweight JSON library
|**[utf8.h](utf8.h)** | <details><summary>utils</summary>[ref.](http://www.ne.jp/asahi/maoyam/hp/UTF-8)</details> | 1123 | GPL2 | UTF8 decoder and encoder library

## References
- [A single-header ANSI C gui library](https://github.com/vurtun/nuklear)
- [Sean Barretts single header libraries](https://github.com/nothings/single_file_libs)
- [Single header libraries for C/C++](https://github.com/vurtun/mmx)
- [enkiTS: source implementation for mm_sched.h](https://github.com/dougbinks/enkiTS)
- [Webby: source implementation for mm_web.h](https://github.com/deplinenoise/webby)
- [GPGPU made simple.](https://github.com/turbo/js)
- http://apoorvaj.io/
