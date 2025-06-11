#!/bin/bash
# Fix for broken CMake on some Arch systems
export CMAKE_ROOT=/usr/share/cmake
/usr/bin/cmake "$@" 