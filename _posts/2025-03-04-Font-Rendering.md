---
layout: post
title:  "Font Rendering"
date:   2025-03-05 10:00:00 +0100
---

![2025-03-05-Font-Rendering--font-atlas.jpg](/assets/2025-03-05-Font-Rendering--font-atlas.jpg)

i totally reworked font rendering. i moved away from ftgl. it feels very deprecated and only works for OpenGL 2. i tried the built in SDL TTF font rendering library, however it fails at properly extracting kerning data. therefore *umfeld* is now using a so-called *atlas* approach ( i.e pre-rendering characters into a texture ). it uses *Freetype2* for rasterizing the characers and HarfBuzz to extract kerning data. it works really well and looks quite nice …

idea for a new feature: PFont can generate PImages of prerendered text ( should take approx 10min to implement ;) )
