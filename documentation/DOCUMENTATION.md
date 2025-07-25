# Umfeld / Documentation

## Installing Umfeld

### Prerequisite

#### macOS

install the required packages with [Homebrew](https://brew.sh).

either run the install script:

```sh
./installation/install-macOS.sh
```

or install manually step by step:

```sh
brew update
brew install \
  cmake \
  pkgconfig \
  sdl3 \
  harfbuzz \
  freetype \
  ffmpeg \
  rtmidi \
  glm \
  dylibbundler \
  portaudio \
  cairo
```

#### Linux

install the required packages with [APT](https://en.wikipedia.org/wiki/APT_(software)) ( for Raspberry Pi OS, see detailed instructions below ).

either run the install script:

```sh
./installation/install-linux.sh
```

or install manually step by step:

```sh
sudo apt-get update -y
sudo apt-get upgrade -y
sudo apt-get install -y \
  git \
  clang \
  cmake \
  curl \
  libx11-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  mesa-utils \
  libxext-dev \
  libxrandr-dev \
  libxcursor-dev \
  libxi-dev \
  libxinerama-dev \
  libwayland-dev \
  libxkbcommon-dev \
  wayland-protocols \
  pkg-config \
  ffmpeg \
  libavcodec-dev \
  libavformat-dev \
  libavutil-dev \
  libswscale-dev \
  libavdevice-dev \
  libharfbuzz-dev \
  libfreetype6-dev \
  librtmidi-dev \
  libglm-dev \
  portaudio19-dev \
  libcairo2-dev \
  libcurl4-openssl-dev \
  libncurses-dev
# sudo apt-get install libsdl3-dev # currently (2025-06-27) not available
```

##### Build SDL3 from source

⚠️ WARNING ⚠️ as of (2025-06-27) SDL3 is not available via apt and must be built from source ( see `install-linux.sh` or below )

```sh
git clone --depth=1 https://github.com/libsdl-org/SDL.git
cd SDL
cmake -S . -B build \
  -DSDL_KMSDRM=ON \
  -DSDL_OPENGL=ON \
  -DSDL_OPENGLES=ON \
  -DSDL_WAYLAND=ON \
  -DSDL_X11=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
sudo cmake --install build
```

#### Raspberry Pi OS (RPI)

*Umfeld* can run on Raspberry Pi ( e.g RPI 4 Model B and RPI 5 ). see [Umfeld-on-RPI](Umfeld-on-RPI.md) for detailed information.

open a terminal and either run the install script:

```sh
./installation/install-raspberry-pi.sh
```

or install manually step by step:

```sh
sudo apt-get update -y
sudo apt-get dist-upgrade -y
sudo apt-get install -y \
  build-essential \
  git \
  cmake \
  curl \
  libx11-dev \
  libegl1-mesa-dev \
  libgles2-mesa-dev \
  pkg-config \
  ffmpeg \
  libavcodec-dev \
  libavformat-dev \
  libavutil-dev \
  libswscale-dev \
  libavdevice-dev \
  libharfbuzz-dev \
  libfreetype6-dev \
  librtmidi-dev \
  libglm-dev \
  portaudio19-dev \
  libcairo2-dev \
  libcurl4-openssl-dev \
  libncurses-dev
```

##### Build SDL from source

note, currently SDL3 is not available via `apt` and needs to be build from source. the following steps should work without modification:

```sh
sudo apt-get install -y \
  libdrm-dev \
  libgbm-dev \
  libudev-dev \
  libegl-dev \
  libgl-dev
git clone --depth=1 https://github.com/libsdl-org/SDL.git
cd SDL
USE_CONSOLE_BUILD=OFF # if running headless (no X11/Wayland) set to 'USE_CONSOLE_BUILD=ON'
cmake -S . -B build \
  -DSDL_KMSDRM=ON \
  -DSDL_OPENGL=ON \
  -DSDL_OPENGLES=ON \
  -DSDL_ALSA=ON \
  -DSDL_PULSEAUDIO=ON \
  -DSDL_PIPEWIRE=ON \
  -DSDL_JACK=ON \
  -DSDL_UNIX_CONSOLE_BUILD=$USE_CONSOLE_BUILD \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build --prefix /usr/local
```

#### Windows

- install [MSYS2](https://www.msys2.org/)
- open `MSYS2 UCRT64`

either run the install script:

```sh
./installation/install-windows-ucrt64.sh
```

or install manually step by step:

```sh
pacman -Syu --noconfirm
pacman -Syu --noconfirm # Repeat after first upgrade to ensure core tools are also updated
pacman -S --noconfirm \
  mingw-w64-ucrt-x86_64-toolchain \
  git \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-curl \
  mingw-w64-ucrt-x86_64-mesa \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-ffmpeg \
  mingw-w64-ucrt-x86_64-harfbuzz \
  mingw-w64-ucrt-x86_64-freetype \
  mingw-w64-ucrt-x86_64-rtmidi \
  mingw-w64-ucrt-x86_64-glm \
  mingw-w64-ucrt-x86_64-portaudio pkgconf \
  mingw-w64-ucrt-x86_64-cairo \
  mingw-w64-ucrt-x86_64-cairomm \
  mingw-w64-ucrt-x86_64-pdcurses \
  mingw-w64-ucrt-x86_64-sdl3 \
  mingw-w64-ucrt-x86_64-sdl3-image \
  mingw-w64-ucrt-x86_64-sdl3-ttf
```

the setup is exclusively for the `MSYS2 UCRT64` branch ( and not for `MSYS2 MINGW64` etcetera ). also it uses `ninja` as a build system instead of `make` ( which is the default on Linux + macOS ).

### Cloning *Umfeld* Repositories

clone the *Umfeld* repositories from GitHub into a desired folder ( e.g `~/Documents/dev` ):

either run the install script:

```sh
./installation/install-macOS.sh
```

or clone manually step by step:

```sh
git clone https://github.com/dennisppaul/umfeld
git clone https://github.com/dennisppaul/umfeld-examples
git clone --recurse-submodules https://github.com/dennisppaul/umfeld-libraries.git # clone with submodules
```

## Building *Umfeld* Example Applications

see [Developing-Applications](Developing-Applications.md) 

## Known Differences

see [Known-Differences-Between-Processing-and-Umfeld](Known-Differences-Between-Processing-and-Umfeld.md)

## Known Issues

see [Known-Issues](Known-Issues.md)
