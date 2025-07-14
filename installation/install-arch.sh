#!/bin/bash

# fail fast if any command fails and elevate privileges
set -e
echo "--- requesting sudo access once"
sudo -v
# keep-alive: update existing `sudo` time stamp until script is done
# (this runs in background and exits when this script finishes)
while true; do sudo -n true; sleep 60; done 2>/dev/null &
KEEP_SUDO_ALIVE_PID="$!"
# trap exit to clean up background sudo keeper
trap 'kill "$KEEP_SUDO_ALIVE_PID"' EXIT

echo "--- updating pacman"
sudo pacman -Syu --noconfirm

echo "--- installing dependencies (Arch Linux)"
sudo pacman -S --noconfirm --needed \
  git \
  clang \
  cmake \
  curl \
  mesa \
  libxext \
  libxrandr \
  libxcursor \
  libxi \
  libxinerama \
  wayland \
  libxkbcommon \
  wayland-protocols

echo "--- installing umfeld dependencies"
sudo pacman -S --noconfirm --needed \
  pkg-config \
  ffmpeg \
  harfbuzz \
  freetype2 \
  rtmidi \
  glm \
  portaudio \
  cairo \
  libcurl-gnutls \
  ncurses

# install SDL3 from source into system
echo "--- building SDL3 from source..."

TMP_DIR=$(mktemp -d)
cd "$TMP_DIR"

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

echo "--- testing SDL3 pkg-config:"
pkg-config --libs --cflags sdl3

cd ~
rm -rf "$TMP_DIR"
echo "--- installed SDL3 successfully"

echo "--- Arch Linux environment set up for 'umfeld'."
echo
