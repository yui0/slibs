# slibs

![GitHub Repo stars](https://img.shields.io/github/stars/yui0/slibs?style=social)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/yui0/slibs)
![Lines of code](https://img.shields.io/tokei/lines/github/yui0/slibs)

Single file libraries for C/C++.

![Logo](slibs.jpeg)

## AI ([llm](https://github.com/yui0/slibs/tree/master/catseye), [Stable Diffusion](https://github.com/yui0/slibs/tree/master/ggml/stable-diffusion))
| library                                                               | license              |  LoC  | description
| --------------------------------------------------------------------- |:--------------------:|:-----:| -----------
| **[catseye_llm.h](catseye/catseye_llm.h)**                            | MIT                  |  1874 | <details><summary>Llama2 for CPU</summary>[Llama2](https://github.com/yui0/slibs/tree/master/catseye), [catseye](https://github.com/yui0/catseye)</details>
| **[ggml.h](ggml/ggml.h)**                                             | MIT                  | 20352 | [Tensor library for machine learning](https://github.com/ggerganov/ggml), <details><summary>details</summary>[GGML](https://github.com/yui0/slibs/tree/master/ggml), [Llama 2](https://github.com/yui0/slibs/tree/master/ggml/llama), [RedPajama](https://github.com/yui0/slibs/tree/master/ggml/redpajama), [Stable Diffusion](https://github.com/yui0/slibs/tree/master/ggml/stable-diffusion)</details>

## audio
| library                                                               | license              |  LoC  | description
| --------------------------------------------------------------------- |:--------------------:|:-----:| -----------
| **[alsa.h](alsa.h)**                                                  | public domain        |   138 | [ALSA library](https://github.com/yui0/aplay-)
| **[uaac.h](uaac.h)**                                                  | RPSL                 | 13438 | [AAC audio decoder](https://github.com/yui0/aplay-)
| **[uflac_enc.h](uflac_enc.h)**                                        | LGPL-2.1             |  2843 | FLAC audio encoder
| **[wav.h](wav.h)**                                                    | public domain        |    61 | Wave format
| **[wave.h](wave.h)**                                                  | public domain        |   304 | utility for wave format
| **[minimp3.h](minimp3.h)**                                            | LGPL                 |  2671 | [MP3 decoder](https://oldforum.puppylinux.com/viewtopic.php?t=59417)

## video
| library                                                               | license              |  LoC  | description
| --------------------------------------------------------------------- |:--------------------:|:-----:| -----------
| **[v4l2.h](v4l2.h)**                                                  | public domain        |   614 | video for linux
| **[mp4parser.h](mp4parser.h)**                                        | LGPL-3.0             |  6015 | mp4 parser
| **[jo_mpeg.h](jo_mpeg.h)**                                            | public domain        |   317 | [MPEG1/2 encoder](http://www.jonolick.com/home/mpeg-video-writer)

## image
| library                                                               | license              |  LoC  | description
| --------------------------------------------------------------------- |:--------------------:|:-----:| -----------
| **[imgp.h](imgp.h)**                                                  | public domain        |   441 | mini image processing library
| **[potracelib.h](potracelib.h)**                                      | GPL                  |  2466 | [Transforming bitmaps into vector graphics](http://potrace.sourceforge.net/), <details><summary>details</summary>[Document](http://potrace.sourceforge.net/potracelib.pdf), [Example](https://github.com/yui0/img2vec)</details>
| **[pssub.h](pssub.h)**                                                | unknown              |  1153 | Postscript library
| **[svg.h](svg.h)**                                                    | Apache-2.0           |   348 | SVG library

## OpenGL
| library                                                               | license              |  LoC  | description
| --------------------------------------------------------------------- |:--------------------:|:-----:| -----------
| **[kms-glsl.h](kms-glsl.h)**                                          | MIT                  |  2426 | [OpenGL using the DRM/KMS](https://ttt.io/glsl-raspberry-pi)

## math
| library                                                               | license              |  LoC  | description
| --------------------------------------------------------------------- |:--------------------:|:-----:| -----------
| **[fft.h](fft.h)**                                                    | public domain        |   136 | Fast Fourier Transform
| **[ifft.h](ifft.h)**                                                  | unknown              |   550 | Fixed-point in-place Fast Fourier Transform
| **[fmath.h](fmath.h)**                                                | public domain        |   142 | Fast math library
| **[random.h](random.h)**                                              | public domain        |    33 | XOR128 random function

## math / GEMM / GPGPU
| library                                                               | license              |  LoC  | description
| --------------------------------------------------------------------- |:--------------------:|:-----:| -----------
| **[sgemm_sse.h](sgemm_sse.h)**                                        | public domain        |   451 | [Fast GEMM function for SSE](https://github.com/yui0/ugemm)
| **[sgemm_ocl.h](sgemm_ocl.h)**                                        | public domain        |   662 | [Fast GEMM function for OpenCL](https://github.com/yui0/ugemm)
| **[sgemm_gl.h](sgemm_gl.h)**                                          | public domain        |   110 | [Fast GEMM function for OpenGL4](https://github.com/yui0/ugemm)
| **[gpgpu_glsl.h](gpgpu_glsl.h)**                                      | public domain        |   399 | [GLSL frameworks](https://github.com/yui0/waifu2x-glsl)
| **[gpgpu_gles.h](gpgpu_gles.h)**                                      | public domain        |   443 | <details><summary>GLES 3.0 frameworks</summary>[Example](gpgpu_gles_matmul.c)</details>
| **[gpgpu_gles3.h](gpgpu_gles3.h)**                                    | public domain        |   264 | GLES 3.0 frameworks with TransformFeedback
| **[gpgpu_gl4.h](gpgpu_gl4.h)**                                        | public domain        |   170 | [OpenGL compute shader frameworks](https://github.com/yui0/waifu2x-glsl)
| **[ocl.h](ocl.h)**                                                    | public domain        |   190 | [OpenCL frameworks](https://github.com/yui0/waifu2x-ocl)

## utils
| library                                                               | license              |  LoC  | description
| --------------------------------------------------------------------- |:--------------------:|:-----:| -----------
| **[alloc.h](alloc.h)**                                                | MIT                  |   116 | Safety alloc
| **[clock.h](clock.h)**                                                | MIT                  |    31 | <details><summary>clock function for linux</summary>[ref.](https://github.com/yui0/waifu2x-glsl)</details>
| **[ht.h](ht.h)**                                                      | MIT                  |   330 | hash table
| **[kbhit.h](kbhit.h)**                                                | MIT                  |    64 | <details><summary>kbhit function for linux</summary>[ref.](https://github.com/yui0/aplay-)</details>
| **[ls.h](ls.h)**                                                      | MIT                  |   155 | <details><summary>making list of files and directories</summary>[ref.](https://github.com/yui0/aplay-)</details>

## console
| library                                                               | license              |  LoC  | description
| --------------------------------------------------------------------- |:--------------------:|:-----:| -----------
| **[aimage.h](aimage.h)**                                              | public domain        |   321 | Image Viewer for Console
| **[mcurses.h](mcurses.h)**                                            | GPL-2                |  1481 | <details><summary>"[mcurses](https://github.com/ChrisMicro/mcurses)" is a minimized version of programming library ncurses</summary>[ref.](https://github.com/yui0/aplay-)</details>
| **[termbox.h](termbox.h)**                                            | MIT                  |  1719 | <details><summary>Library for writing text-based user interfaces</summary>[ref.](https://github.com/nsf/termbox)</details>

## parser
| library                                                               | license              |  LoC  | description
| --------------------------------------------------------------------- |:--------------------:|:-----:| -----------
| **[csv.h](csv.h)**                                                    | public domain        |    77 | CSV file parser
| **[ini.h](ini.h)**                                                    | BSD                  |   250 | ini file parser
| **[parg.h](parg.h)**                                                  | CC0                  |   551 | <details><summary>Parser for argv that works similarly to getopt</summary>[ref.](https://github.com/jibsen/parg)</details>
| **[parson.h](parson.h)**                                              | MIT                  |  2239 | <details><summary>Lightweight JSON library</summary>[ref.](https://github.com/kgabis/parson)</details>
| **[utf8.h](utf8.h)**                                                  | GPL2                 |  1123 | <details><summary>UTF8 decoder and encoder library</summary>[ref.](http://www.ne.jp/asahi/maoyam/hp/UTF-8)</details>

## References
- [A single-header ANSI C gui library](https://github.com/vurtun/nuklear)
- [Sean Barretts single header libraries](https://github.com/nothings/single_file_libs)
- [Single header libraries for C/C++](https://github.com/vurtun/mmx)
- [enkiTS: source implementation for mm_sched.h](https://github.com/dougbinks/enkiTS)
- [Webby: source implementation for mm_web.h](https://github.com/deplinenoise/webby)
- [GPGPU made simple.](https://github.com/turbo/js)
- http://apoorvaj.io/
