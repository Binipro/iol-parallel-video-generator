## iol-parallel-video-generator

A parallel c++ project mainly responsible to reconstruct or generate videos from Adobe After Effect farm rendering
tools such as Nexrender. When such tools are used to render a single job on multiple worker nodes in a network, 
the outputs of those workers are image sequences. This program is able to convert the image sequences in parallel
to high quality videos using ffmpeg. The program can be also used for any image sequences to video conversion.

The parallel program uses high performance parallel programming framework called Fastflow with a farm parallel pattern.

## Prerequisites

```
gcc, cmake, ffmpeg, libx264
```
 ## Build
 ```
 git clone https://github.com/Binipro/iol-parallel-video-generator.git
 cd iol-parallel-video-generator
 cmake .
 make
 ./iol-parallel-video-generator --help
 ```
