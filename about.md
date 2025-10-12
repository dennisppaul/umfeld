---
layout: page
title: About
permalink: /about/
---

{% include_relative /assets/abstract.md %}

well, *Umfeld* is actually imitating [Processing.org](https://processing.org) quite bluntly ;)

this project aims to supply a framework that allows writing applications that more or less look like original [Processing.org](https://processing.org) sketches. note the character of this project is that features will be added as they are needed. it might partly replicate but not fully emulate the original [Processing.org](https://processing.org) environment.

*Umfeld* may also function as a statement on what is good and valuable about the [Processing.org](https://processing.org) *idiom* while at the same time suggesting disconnecting it from one specific group, application or project … maybe a bit like markdown in that sense.

see [DOCUMENTATION](https://github.com/dennisppaul/umfeld/blob/main/documentation/DOCUMENTATION.md) for usage information and [examples](https://github.com/dennisppaul/umfeld-examples) for applications.

## Why *Umfeld*?

*Umfeld* is yet another programming environment for designers, makers, and artists. so why does it exist? and why not use [Processing.org](https://processing.org), [OpenFrameworks](https://openframeworks.cc), [Cinder](https://libcinder.org) or any of the other frameworks out there?

well, the main reason why we even created *Umfeld* was basically to have a C++ derivative of Processing.org that stays very close to the *original* idiom. while Cinder and OpenFrameworks, for example, are fantastic projects, they differ quite a lot from the *Processing.org Idiom* ( i.e a `setup()-draw()` structure, `stroke()+fill()` and shape-based drawing, etcetera ). *Umfeld*, however, literally imitates core Processing classes and functions.

why not use the original Java-based *Processing.org* then? there are a few key aspects that just cannot and probably will never be resolved due to limitations imposed by Java. while Java is a very well-designed language and does many things better than C/C++, it has some drawbacks.

the most important benefit of moving to C/C++ is the availability of many extremely powerful and widespread libraries and APIs ( e.g OpenGL, OpenCV, FFmpeg, PortAudio ) that can be natively used and integrated into applications and sketches without the need for a native binding library. while many of these libraries have been fully or partly made accessible in Processing through native bindings, oftentimes these libraries or their bindings introduce significant overhead ( technically, administratively, etcetera ), are not at the current version, are incompatible with newer versions of OSes, or do not expose all functionality ( e.g unstable webcam support on macOS ). while this also happens with C/C++ libraries and APIs, the problems are greatly reduced. also, there is still a significant number of libraries and APIs not available for Java-based Processing or that are impossible to port because of limitations posed by Java VM implementations ( e.g multi-channel audio ).

in addition to this, due to the nature of C/C++ and the way *Umfeld* is implemented, applications and sketches developed with *Umfeld* can be built and deployed with a very(!) small memory and CPU footprint. this means that applications and sketches can potentially run on smaller or older hardware like, e.g Raspberry Pi. some modules can even be excluded from a build to reduce the footprint even more. *Umfeld* can even run *truly* headless ( i.e it does not require any virtual offscreen graphics driver if no window is required ).

furthermore, *Umfeld* uses CMake as a build system. CMake is very well-designed and more or less easy to learn and extend. this also means that *Umfeld* applications and sketches can be developed in more or less any text editor or IDE ( e.g Visual Studio Code, Zed, CLion, Vim ). it does not require proprietary build systems like Xcode.

and finally, although C and especially C++ is known for its horrific errors, confusing error messages, and volatile behavior at times, with the advent of LLM-based programming assistants, the entry threshold can be greatly reduced. trust the process. PS ( if *vibe coding* is even a thing this should be easy ;) )

*Umfeld* is developed and maintained by and [Dennis P Paul](https://dennisppaul.de). Please get in contact if you are interested to join.