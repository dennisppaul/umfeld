#!/usr/bin/env bash

# Ensure the script is being run from ç
if [[ "$MSYSTEM" != "UCRT64" ]]; then
  echo "Error: This script must be run from the MSYS2 UCRT64 environment."
  echo "Start MSYS2 using the 'MSYS2 UCRT64' shortcut and run this script again."
  exit 1
fi

echo "--- updating MSYS2 package database and core system packages..."
echo "--- NOTE the update might quit this console! in that case just re-open and re-run this script if that happens ..."
pacman -Syu --noconfirm
# Repeat after first upgrade to ensure core tools are also updated
pacman -Syu --noconfirm

echo "--- installing dependencies (Windows MSYS2 UCRT64)"
pacman -S --noconfirm \
  mingw-w64-ucrt-x86_64-toolchain \
  git \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-curl \
  mingw-w64-ucrt-x86_64-mesa
echo "--- installing Umfeld dependencies"
pacman -S --noconfirm \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-ffmpeg \
  mingw-w64-ucrt-x86_64-harfbuzz \
  mingw-w64-ucrt-x86_64-freetype \
  mingw-w64-ucrt-x86_64-rtmidi \
  mingw-w64-ucrt-x86_64-glm \
  mingw-w64-ucrt-x86_64-portaudio pkgconf \
  mingw-w64-ucrt-x86_64-cairo \
  mingw-w64-ucrt-x86_64-cairomm \
  mingw-w64-ucrt-x86_64-pdcurses
echo "--- installing SDL3"
pacman -S --noconfirm \
  mingw-w64-ucrt-x86_64-sdl3 \
  mingw-w64-ucrt-x86_64-sdl3-image \
  mingw-w64-ucrt-x86_64-sdl3-ttf

echo "--- Windows MSYS2 UCRT64 environment set up for 'Umfeld'."
echo
