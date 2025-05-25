---
layout: post
title:  "New Release: Umfeld v2.1.0"
date:   2025-05-26 10:00:00 +0100
---

![2025-05-26--New-Release-Umfeld-v2.1.0](/assets/2025-05-26--New-Release-Umfeld-v2.1.0.png){: style="width:100%;" }

## RELEASE_NOTES Umfeld v2.1.0 (2025-05-24)

- *Umfeld* no runs on macOS, Linux ( including RPI ), and Windows ( still a lot of glitches )
- OpenGL ES 3.0 now works ( as a subset of OpenGL 3.3 core ) ( + on macOS ( M3 ) ANGLE (Almost Native Graphics Layer Engine) works now to develop OpenGL ES )
- KMSDRM now works on RPIs ( i.e running applications without desktop environment )
- added support for gamepad
- replaced GLEW with GLAD ( OpenGL function loading library, prerequisite for OpenGL ES 3.0 )
- added support for serial communinication with `Serial`
- updated RPI image at http://dm-hb.de/umfeld-rpi
- updated documentation

[https://github.com/dennisppaul/umfeld/releases/tag/v2.1.0](https://github.com/dennisppaul/umfeld/releases/tag/v2.1.0)
