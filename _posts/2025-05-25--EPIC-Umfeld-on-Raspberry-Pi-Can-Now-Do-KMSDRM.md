---
layout: post
title:  "EPIC Umfeld on Raspberry Pi Can Now Do KMSDRM"
date:   2025-05-25 18:00:00 +0100
---

![2025-05-25--EPIC-Umfeld-on-Raspberry-Pi-Can-Now-Do-KMSDRM](/assets/2025-05-25--EPIC-Umfeld-on-Raspberry-Pi-Can-Now-Do-KMSDRM.gif){: style="width:100%;" }

*Umfeld* can now run via KMSDRM on RPIs. this is EPIC![^1] but … yeah, why?

> KMSDRM (Kernel Mode Setting + Direct Rendering Manager) is a video driver backend that allows rendering graphics directly to the screen without needing a windowing system like X11 or Wayland. on Raspberry Pi devices, it provides better performance, lower latency, and more direct control over the GPU, making it ideal for full-screen, embedded, or real-time applications like games, kiosks, or media players.

is what ChatGPT says … in other words: a nice, lightweight, high-performance way to run fullscreen applications.

i am extremely excited about this. on the one hand, this was one of the initial reasons to start *Umfeld* in the first place [^2] and on the other hand, i feel it’s also a small-technology-style statement: thin linux distribution + umfeld application … perfection!

a few notes for this to work:

1. `SDL3` must be compiled with the KMSDRM backend enabled:
    ```sh
    cmake -B build -DSDL_VIDEO_KMSDRM=ON ...
    ```
2. if need be force the driver via environment variables before runnning the application:
    ```sh
    export SDL_VIDEODRIVER=kmsdrm
    export SDL_RENDER_DRIVER=opengles2
    ```

[^1]: at least to me ;)
[^2]: ever tried running Processing (java-based or javascript) on a Raspberry Pi? … it is SLOW!