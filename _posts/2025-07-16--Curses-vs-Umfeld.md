---
layout: post
title:  "Curses vs Umfeld"
date:   2025-07-16 10:00:00 +0100
---

![2025-07-16--Curses-vs-Umfeld](/assets/2025-07-16--Curses-vs-Umfeld.gif){: style="width:100%;" }

another #weekendproject has been successfully completed: **TERMINAL RENDERER**

after i went deep into the Text-based User Interface (TUI) rabbit hole ( just saying: ncspot to replace Spotify and matterhorn to replace Mattermost ;) ), i thought it would be nice if _Umfeld_ could also be _text-based_.

so if you ever get tired of the same old pixel-based graphics and want to render lines, images, text, etcetera right into a terminal window, as some sort of weird ASCII representation, just do this in your code:

```c
void settings() {
    PGraphicsTerminal::enable_graphics_subsystem();
}
```

this will replace the default OpenGL renderer with an [ncurses](https://en.wikipedia.org/wiki/Ncurses)-based renderer that draws everything as characters. it is still very raw, but it can already draw points, lines ( <- [Bresenham](https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm) ), text, and images. it can also process user input from keyboard and mouse ( to some extent ).

some quick tests on macOS terminal emulators have shown that it runs ok on:

- `Terminal.app`
- `kitty.app`
- `iTerm.app`
