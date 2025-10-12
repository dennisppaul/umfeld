---
layout: post
title:  "Umfeld on Raspberry Pi"
date:   2025-04-03 10:00:00 +0100
---

![2025-04-03-Umfeld-on-Raspberry-Pi.gif](/assets/2025-04-03-Umfeld-on-Raspberry-Pi.gif){: style="width:100%;" }

so this is exciting! Umfeld can run on a Raspberry Pi 4 Model B or Raspberry Pi 5.

next stop: get *Kernel Mode Setting with Direct Rendering Manager* (KMSDRM) up and runnning! i have already compiled and ran a minimal demo with ImGui … nice!

the following step-by-step guid is also published and will be updated in the repository at [Umfeld on Raspberry Pi](https://github.com/dennisppaul/umfeld/blob/main/documentation/Umfeld-on-RPI.md):

# Umfeld on Raspberry Pi

RPI currently uses X11 as the rendering system when in desktop environment. however, there is also an allegedly much faster KMS ( or KMSDRM ) rendering system which can start fullscreen windows without(!) a GUI i.e from the command-line ( and even from a remote machine via SSH ). i have tested this already, it does work but requires some extra development. stay tuned.

*Umfeld* was tested on *Raspberry Pi 4 Model B* with Raspberry Pi OS (64-bit), *Debian Bookworm* ( Released: 2024-11-19 ).

however, it has not been tested carefully. there might be glitches …

PS RPI does not support antialiasing i.e make sure to set the value to `0` in `settings`:

```c++
void settings(){
    antialiasing = 0;
}
```

this step by step guide has been tested on a *Raspberry Pi 4 Model B* with Raspberry Pi OS (64-bit), *Debian Bookworm* ( Released: 2024-11-19 ) installed.

## Quick Start

for a quick start an image with *Umfeld* and all dependencies can be download from: http://dm-hb.de/umfeld-rpi

the image has been tested on *Raspberry Pi 4 Model B* with Raspberry Pi OS (64-bit), *Debian Bookworm* ( Released: 2024-11-19 ).

the credentials are:

- name ....... : `umfeld.local` 
- user ....... : `umfeld` 
- password ... : `umfeld123`

## Preparing the Build Environment

install all packages:

```sh
sudo apt-get update -y
sudo apt-get upgrade -y
sudo apt-get install cmake libglew-dev libharfbuzz-dev libfreetype6-dev ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavdevice-dev librtmidi-dev libglm-dev portaudio19-dev -y
#sudo apt-get install libsdl3-dev # currently (2025-04-01) not available
```

note, the following packagae are also required but come pre-installed with the OS distribution:

- `gcc` (v12.2.0) 
- `git` (v2.39.5)
- `pkg-config` (v1.8.1)

since SDL3 is currently not available via `apt` ( i.e `apt-get install libsdl3-dev` ) we need to build SDL3 from source and install it.

### Build SDL3 from Source

```sh
sudo apt-get update -y ; sudo apt-get upgrade -y
sudo apt-get install cmake
git clone https://github.com/libsdl-org/SDL.git
cd SDL
# cmake -S . -B build -DSDL_ALSA=ON -DSDL_PULSEAUDIO=ON -DSDL_PIPEWIRE=ON -DSDL_JACK=ON -DCMAKE_BUILD_TYPE=Release
# cmake -S . -B build -DVIDEO_KMSDRM=ON -DVIDEO_X11=ON -DVIDEO_WAYLAND=ON -DRENDER_OPENGL=ON -DRENDER_OPENGLES=ON -DCMAKE_BUILD_TYPE=Release
cmake -S . -B build -DVIDEO_KMSDRM=ON -DVIDEO_X11=ON -DVIDEO_WAYLAND=ON -DRENDER_OPENGL=ON -DRENDER_OPENGLES=ON -DSDL_ALSA=ON -DSDL_PULSEAUDIO=ON -DSDL_PIPEWIRE=ON -DSDL_JACK=ON -DCMAKE_BUILD_TYPE=Release ## TODO check if all build flags are correct and needed
cmake --build build # `--parallel` is optional for building with multiple cores but currently kills the RPI
sudo cmake --install build --prefix /usr/local
```

now SDL3 is properly installed and can be used for build ( i.e `pkg-config --libs --cflags sdl3` provides all compile flags ).

## Setting up Umfeld

first clone [umfeld](https://github.com/dennisppaul/umfeld) and the [umfeld-examples](https://github.com/dennisppaul/umfeld-examples) repositories with submodules from GitHub:

```sh
git clone --recurse-submodules https://github.com/dennisppaul/umfeld.git
git clone --recurse-submodules https://github.com/dennisppaul/umfeld-examples.git
```

now enter the example directory, to build and run an example e.g a basic minimal example:

```sh
cd umfeld-examples/Basics/minimal
cmake -B build
cmake --build build # --parallel
./build/minimal
```

note, the first time might take a bit on a small machine. note, examples can not be run from `ssh` sessions ( without *X11 forwarding* ).

## X11 Forwarding

to run GUI applications from another machine via `ssh` the following steps must be taken:

- check that remote RPI has *X11 forwarding* enabled. in `/etc/ssh/sshd_config` confirm that `X11Forwarding yes`. if not change it an restart `ssh` with `sudo systemctl restart ssh`
- make sure local machine has an *X server* running ( e.g for macOS install XQuartz with `brew install xquartz` )
- start `ssh` session from local machine with `-Y` option e.g `ssh -Y umfeld@umfeld.local`
- run application with `DISPLAY` set to remote screen e.g `DISPLAY=:0 ./umgegbung-application`
