---
layout: post
title:  "Umfeld on Raspberry Pi 1 Mod. B"
date:   2025-06-30 10:00:00 +0100
---

![2025-06-30--Umfeld-on-Raspberry-Pi-1-Mod-B](/assets/2025-06-30--Umfeld-on-Raspberry-Pi-1-Mod-B.gif){: style="width:100%;" }

one #weekendproject later i can now confirm that _Umfeld_ compiles and runs on a *Raspberry Pi 1 Model B* ( one of the very first RPIs released ).

i started off with a fresh install of *Raspberry Pi OS Bookworm ( 32-bit, 2025-05-13 Release )*. from there, i used the _Umfeld_ install script to pull in all required dependencies. that took care of most things, but a few tweaks were needed to get it running.

- i forced the renderer to use *OpenGL 2.0*, which currently isn’t in a great state on this hardware. expect quirks.
- i hit a linker error related to atomics. to fix it, i had to explicitly link against libatomic. that meant adding this line to the CMakeLists.txt:

```
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -latomic" )
```

- or alternatively:

```
target_link_libraries( minimal atomic )
```

after that, i was able to compile and run the minimal sketch in fullscreen at *1280×720 @ 60 FPS*. that’s pretty impressive for a machine with a single-core ARM11 CPU and 14 years of age.

however, this is just a proof of concept* and i am not sure if this is production-ready. i only tested the `minimal` example. the whole process took *forever*; installing packages, compiling SDL3, and building a sketch for the first time easily took *7–8 hours* ( i didn’t time it exactly ). after that recompilimg just the sketch still required approx. 3 min.

### What’s Next?

i haven’t tested KMSDRM yet. it might work, but would likely require switching from desktop *OpenGL 2.0* to an *OpenGL ES 2.0* backend ( and *Umfeld* currently defaults to the former ).
