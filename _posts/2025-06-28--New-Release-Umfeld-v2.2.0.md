---
layout: post
title:  "New Release: Umfeld v2.2.0"
date:   2025-06-28 10:00:00 +0100
---

![2025-06-28--New-Release-Umfeld-v2.2.0](/assets/2025-06-28--New-Release-Umfeld-v2.2.0.png){: style="width:100%;" }

## RELEASE_NOTES Umfeld v2.2.0 (2025-06-284)

- `size()` can now interpret `DISPLAY_WIDTH, DISPLAY_HEIGHT` to set window to full display width and height
- audio devices can now run in their own thread ( e.g by setting `audio_threaded = true;` in `settings()` )
- shapes can now be exported as PDF or OBJ
- lights are now working
- window location and window title can now be set by application
- `loadBytes()`, `loadStrings()`, and `loadImage()` now support loading from local files as well as URLs and URIs (with base64 + percent + html encoding)
- added `data` folder concept. folder will be automatically copied upon building
- `load...` functions now look for files in `data` folder #breakingchange
- fixed vertex buffer on Windows and Linux
- added dedicated point and line shaders
- added a CLI-based renderer ( WIP )
- added `noLoop()` and `redraw()`
- updated documentation
- improved build scripts. *Umfeld* can now be installed via scripts from repository ( see [Quickstart](README.md#Quickstart) )

... and many more bug fixes and minor improvements.

[https://github.com/dennisppaul/umfeld/releases/tag/v2.2.0](https://github.com/dennisppaul/umfeld/releases/tag/v2.2.0)
