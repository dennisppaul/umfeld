---
layout: post
title:  "Shape Render Pipeline"
date:   2025-03-29 10:00:00 +0100
---

the main concept of the *Umfeld* graphic render is centered around the idea of shapes as used in *Processing*:

```c
beginShape(TRIANGLES);
vertex(50,0);
vertex(100, 100);
vertex(0, 100);
endShape();
```

to make it more consistent  implementing a renderer ( i.e derivatives of `PGraphics` ) all primitives ( e.g `rect`, `ellipse`, `point`, `bezier` etcetera ) are implement with *begin-end-shapes*. `PGraphics` in turn separates shapes into stroke and fill shapes, where stroke shapes are arranged as *line strips* and fill shapes as a list of triangles:

```
                    + -> emit_shape_stroke_line_strip -> triangulate, transform to world/screen space
begin-end-shape ->  |
                    + -> emit_shape_fill_triangles    -> buffer, transform to world/screen space
```

a derived renderer ( e.g `PGraphicsOpenGLv33`) can then receive, transform and draw line-strips ( `emit_shape_stroke_line_strip` ) and triangles as required. while this approach offers a consistent and concicse entry point for implementing other renderers ( e.g SDL_gpu, SDL_renderer, Vulkan or Metal ) it is probably not the most performant approach. there is room for numerous renderer-specific improvements here ( which may be applied over time ). however, for now this seems like a reasonable approach.

it also opens up interesting approaches that involve buffering of shapes ( AAMOF this one major optimization idea ).
