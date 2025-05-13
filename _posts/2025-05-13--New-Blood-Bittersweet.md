---
layout: post
title:  "New Blood Bittersweet"
date:   2025-05-13 10:00:00 +0100
---

![2025-05-13--New-Blood-Bittersweet](/assets/2025-05-13--New-Blood-Bittersweet.gif){: style="width:100%;" }

*Umfeld* now runs on *Windows* … and it feels a bit dirty. 

somehow, i never cared about porting *Umfeld* … but then i looked into those eyes … and felt sorry.

i don’t want to start a discussion but for me *Windows* is such a nasty environment for developers and getting *Umfeld* to compile was a bit of a pain. this included WEIRD glitches in the tesselation library, a bizzar absence of the *weak* attribute[^1], and a compiler toolchain that can only be described as *frankenstein*-esque. no offense dear people from MinGW you are doing an AMAZING job, but it feels so much less complete, coherent and *wanted* than developing e.g on Ubuntu or macOS.

well, *Umfeld* works now more or less on *Windows* … with a few glitches and inconsistencies. at least SDL3 seems to be pretty solid! let’s see where this all goes … welcome *Windows*.

[^1]: this is actually a longer story and took me quite a while to figure out. long story short: MSVC the main compiler for *Windows* does not support the weak attribute at all i.e there is no way to replicate the overriding of functions as used in Processing ( e.g `keyPressed()` ). MinGW understands the weak attribute, well, a little bit, but does not AT ALL behave the same way as it does in GCC or Clang. check out `UmfeldCallbacks` for a working example.