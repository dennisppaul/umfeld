---
layout: post
title:  "Umfeld OpenGL Support Table"
date:   2025-04-04 10:00:00 +0100
---

this is a rundown of the renderers available in *Umfeld* and how they are supported on various platforms:

- `OPENGL_3_3`, shader-based version with core profile ( no fixed-function pipeline )
- `OPENGL_2_0`, legacy version with fixed-function pipeline  
- `OPENGL_ES_3_0` :: used for iOS + Android + RPI4b+5 @note( has not yet been implemented in *Umfeld* )
- `SDL_2D` :: built in software-renderer @note( has not yet been implemented in *Umfeld* )

note, that only `OPENGL_3_3` and `OPENGL_2_0` are half decently implemented. all other renderers are stil VERY much work in progress and lack most features or do not exist at all ( e.g `OPENGL_ES_3_0` + `SDL_2D` )

| System    | Max. OpenGL version        | Umfeld OpenGL version      | Umfeld Token                 |
| --------- | -------------------------- | ---------------------------- | ------------------------------ |
| macOS     | OpenGL 4.1 [^1]            | OpenGL 3.3 core + OpenGL 2.0 | `OPENGL_3_3` + `OPENGL_2_0`    |
| Linux     | OpenGL 4.6 [^2]            | OpenGL 3.3 core + OpenGL 2.0 | `OPENGL_3_3` + `OPENGL_2_0`    |
| RPI 4b    | OpenGL ES 3.1 + OpenGL 2.1 | OpenGL ES 3.0 + OpenGL 2.0   | `OPENGL_ES_3_0` + `OPENGL_2_0` |
| RPI 5     | OpenGL ES 3.1 + OpenGL 2.1 | OpenGL ES 3.0 + OpenGL 2.0   | `OPENGL_ES_3_0` + `OPENGL_2_0` |
| Windows   | OpenGL 4.6 [^2]            | OpenGL 3.3 core + OpenGL 2.0 | `OPENGL_3_3` + `OPENGL_2_0`    |
| iOS       | OpenGL ES 3.0 [^3]         | OpenGL ES 3.0                | `OPENGL_ES_3_0`                |
| WebGL 2.0 | OpenGL ES 3.0              | OpenGL ES 3.0                | `OPENGL_ES_3_0`                |

[^1]: deprecated on macOS since 10.14+ in favor of Metal
[^2]: may vary depending on OS, GPU + driver
[^3]: deprecated in favor of Metal
