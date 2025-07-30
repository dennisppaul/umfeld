---
layout: post
title:  "New Release: Umfeld v2.3.0"
date:   2025-07-29 10:00:00 +0100
---

![2025-07-29--New-Release-Umfeld-v2.3.0](/assets/2025-07-29--New-Release-Umfeld-v2.3.0.png){: style="width:100%;" }

hej community,

there is a new release of [Umfeld](https://github.com/dennisppaul/umfeld/releases/tag/v2.3.0) available.

this release introduces a series of improvements and a fe new features. texture behave a bit better, terminal renderer received a bit of care, and pixel operations got fixed. however, the biggest, under the hood change, was the improvement of the mechanism handling package finding for the projects the *Umfeld* depends upon.

## RELEASE_NOTES Umfeld v2.3.0 (2025-07-24)

- textures now have filtering and wrapping options with `texture_filter` and `texture_wrap`
- application now sets automatically window title to project title
- added option `flip_y_texcoords` to `PImage` to flip y-axis when drawn ( addressing the bottom-left vs bottom-right dilema )
- terminal renderer has been improved, can draw images now, handles colors, and has been added officially
- pixel operations `loadPixels()`, `pixels[]`, and `updatePixel()` now work
- improved install scripts
- Arch Linux now works again and is detected by installation script
- improved random functions; mode can be set with `set_random_mode()`
- added additional console and warning function that only emit a message once: `console_once()` and `warning_in_function_once()`
- default window name is now deduced from CMake script
- made package and library finding in CMake script more robust 
- improved terminal ( CLI ) renderer
- fixed threaded audio

… and many more bug fixes and minor improvements.

[https://github.com/dennisppaul/umfeld/releases/tag/v2.3.0](https://github.com/dennisppaul/umfeld/releases/tag/v2.3.0)
