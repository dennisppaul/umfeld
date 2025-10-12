---
layout: post
title:  "New Release: Umfeld v2.2.0"
date:   2025-06-28 10:00:00 +0100
---

![2025-06-28--New-Release-Umfeld-v2.2.0](/assets/2025-06-28--New-Release-Umfeld-v2.2.0.png){: style="width:100%;" }

hej community,

there is a new release of [Umfeld](https://github.com/dennisppaul/umfeld).

i really do think you should try it out. we have invested a lot of energy in making it more accessible and easier to get started with … on macOS, Linux, Raspberry Pi, and even Windows ( UCRT64 ). if you are running a more or less friendly system, it does not require much more effort than pasting this into your terminal:

```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/dennisppaul/umfeld/refs/heads/main/install.sh)"
```

for more detailed information on how to install and what to do after installation, head over to [https://github.com/dennisppaul/umfeld](https://github.com/dennisppaul/umfeld) or read the [release notes](https://github.com/dennisppaul/umfeld/releases/tag/v2.2.0).

apart from hopefully having made it more accessible, [Umfeld](https://github.com/dennisppaul/umfeld) has become so much more stable. also, we have added a few [libraries](https://github.com/dennisppaul/umfeld-libraries) that interface with Ollama, DaisySP, Dear ImGui, or OpenCV ( WIP ).

i really hope you get into explorer mode and give [Umfeld](https://github.com/dennisppaul/umfeld) a try. any suggestions, bug reports, or comments, either send me a DM or file an issue in the GitHub repository.

## RELEASE_NOTES Umfeld v2.2.0 (2025-06-28)

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
