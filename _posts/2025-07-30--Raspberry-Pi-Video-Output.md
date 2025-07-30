---
layout: post
title:  "Raspberry Pi Video Output"
date:   2025-07-16 10:00:00 +0100
---

below is an autoi-generated table showing video outputs and OpenGL versions for all Raspberry Pi boards:

| Model            | Max OpenGL Version | Max OpenGL ES Version | Video Output        | Best Performing Setup               | GPU                                                                                             |
| ---------------- | ------------------ | --------------------- | ------------------- | ----------------------------------- | ----------------------------------------------------------------------------------------------- |
| Zero             | n.A                | ES 2.0                | Mini-HDMI           |                                     | BCM2835, VideoCore IV @ 250 MHz                                                                 |
| Zero W / WH      | n.A                | ES 2.0                | Mini-HDMI           |                                     | BCM2835, VideoCore IV @ 250 MHz                                                                 |
| Zero 2 W         | n.A                | ES 2.0                | Mini-HDMI           |                                     | BCM2836 (v1.1) or BCM2837 (v1.2), VideoCore IV @ 250–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan) |
| 1 Model A        | n.A                | ES 2.0                | Composite, HDMI     |                                     | BCM2835, VideoCore IV @ 250 MHz                                                                 |
| 1 Model A+       | n.A                | ES 2.0                | Composite, HDMI     |                                     | BCM2835, VideoCore IV @ 250 MHz                                                                 |
| 1 Model B        | n.A                | ES 2.0                | Composite, HDMI     |                                     | BCM2835, VideoCore IV @ 250 MHz                                                                 |
| 1 Model B+       | n.A                | ES 2.0                | Composite, HDMI     |                                     | BCM2835, VideoCore IV @ 250 MHz                                                                 |
| 2 Model B        | n.A                | ES 2.0                | Composite, HDMI     |                                     | BCM2836 (v1.1) or BCM2837 (v1.2), VideoCore IV @ 250–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan) |
| 2 Model B v1.2   | 2.1                | ES 2.0                | Composite, HDMI     |                                     | BCM2836 (v1.1) or BCM2837 (v1.2), VideoCore IV @ 250–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan) |
| 3 Model A+       | 2.1                | ES 2.0                | Composite, HDMI     |                                     | BCM2837, VideoCore IV @ 300–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan)                          |
| 3 Model B        | 2.1                | ES 2.0                | Composite, HDMI     |                                     | BCM2837, VideoCore IV @ 300–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan)                          |
| 3 Model B+       | 2.1                | ES 2.0                | Composite, HDMI     |                                     | BCM2837, VideoCore IV @ 300–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan)                          |
| 4 Model B        | 3.1                | ES 3.1                | 2x Micro-HDMI       |                                     | BCM2711, VideoCore VI @ 500 MHz (OpenGL ES 3.1, Vulkan 1.2, GL 3.1)                             |
| 5                | 4.6                | ES 3.2                | 2x Micro-HDMI       |                                     | BCM2712, VideoCore VII @ 800 MHz (OpenGL ES 3.1, Vulkan 1.2, GL 3.1)                            |
| CM1              | n.A                | ES 2.0                | DSI/CSI only        | OpenGL 2.1 (KMS driver)             | BCM2835, VideoCore IV @ 250 MHz                                                                 |
| CM3              | 2.1                | ES 2.0                | DSI/CSI only        | OpenGL 2.1 (KMS driver)             | BCM2837, VideoCore IV @ 300–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan)                          |
| CM3 Lite         | 2.1                | ES 2.0                | DSI/CSI only        | OpenGL 2.1 (KMS driver)             | BCM2837, VideoCore IV @ 300–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan)                          |
| CM3+             | 2.1                | ES 2.0                | DSI/CSI only        | OpenGL 2.1 (KMS driver)             | BCM2837, VideoCore IV @ 300–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan)                          |
| CM3+ Lite        | 2.1                | ES 2.0                | DSI/CSI only        | OpenGL 2.1 (KMS driver)             | BCM2837, VideoCore IV @ 300–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan)                          |
| CM4              | 3.1                | ES 3.1                | 2x HDMI via carrier | OpenGL ES 3.1 + Vulkan (KMS driver) | BCM2711, VideoCore VI @ 500 MHz (OpenGL ES 3.1, Vulkan 1.2, GL 3.1)                             |
| CM5              | 4.6                | ES 3.2                | 2x HDMI via carrier |                                     | BCM2712, VideoCore VII @ 800 MHz (OpenGL ES 3.1, Vulkan 1.2, GL 3.1)                            |
| 2 Model B (v1.1) | 2.1                | ES 2.0                | HDMI                | OpenGL 2.1 (KMS driver)             | BCM2836 (v1.1) or BCM2837 (v1.2), VideoCore IV @ 250–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan) |
| 3 Model B        | 2.1                | ES 2.0                | HDMI                | OpenGL 2.1 (KMS driver)             | BCM2837, VideoCore IV @ 300–400 MHz (OpenGL ES 2.0, GL 2.1, no Vulkan)                          |
| 4 Model B        | 3.1                | ES 3.1                | Dual micro-HDMI     | OpenGL ES 3.1 + Vulkan (KMS driver) | BCM2711, VideoCore VI @ 500 MHz (OpenGL ES 3.1, Vulkan 1.2, GL 3.1)                             |
| 5 Model B        | 3.1                | ES 3.1                | Dual micro-HDMI     | OpenGL ES 3.1 + Vulkan (KMS driver) | BCM2712, VideoCore VII @ 800 MHz (OpenGL ES 3.1, Vulkan 1.2, GL 3.1)                            |

*Umfeld* supports OpenGL 2.0, OpenGL 3.3 core and OpenGL ES 3.0 ( despite the specs above Umfeld runs on a Raspberry Pi 1 Mod B [[2025-06-30--Umfeld-on-Raspberry-Pi-1-Mod-B]] with only OpenGL 2.0 support … hmmm ).

<!--EOF-->
