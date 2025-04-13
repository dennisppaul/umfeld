---
layout: post
title:  "Umfeld in Visual Studio Code"
date:   2025-04-13 18:00:00 +0100
---

to run an *Umfeld* application as a *CMake* project in Visual Studio Code ( VS Code ), follow these steps:

## install prerequisites

- install *Umfeld* ( see [Umfeld / Documentation / Installing Umfeld](https://github.com/dennisppaul/umfeld/blob/main/documentation/DOCUMENTATION.md) )
- install [Visual Studio Code](https://code.visualstudio.com)
- install *C/C++ Extension Pack* for VS Code: go to the extensions panel and search for `C/C++ Extension Pack` ( Microsoft ), then install it.
    
## open your project folder

open the root folder of your cmake project ( the one containing the `CMakeLists.txt` ) in VS Code ( e.g `./umgebung-examples/Basics/minimal` ).

## configure the project

after opening the folder:

- press `CMD+SHIFT+P` ( or `CTRL+SHIFT+P` on linux and windows )
- type `cmake: configure` and run the command ( you can let the CMake decide which compiler to use or pick one of you choice )

this will configure your project and create a build folder ( default is `build` ).

## build the project

- press `CMD+SHIFT+P`
- type `cmake: build` and run the command

your code will be compiled.

## run your executable

after building, you can:

- press `CMD+SHIFT+P`
- type `cmake: run` ( or use `cmake: debug to launch with debugger` )

alternatively, find the output binary in your build/ folder and run it manually in a terminal.
