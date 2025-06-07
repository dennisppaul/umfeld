#!/bin/bash

# Umfeld installation script for Arch Linux
# Fixes CMake and harfbuzz issues specific to Arch

set -e

echo "=== Umfeld Installation Script for Arch Linux ==="
echo ""

# Make sure we're actually on Arch
if ! command -v pacman &> /dev/null; then
    echo "Error: This script is for Arch Linux (needs pacman/yay)."
    echo "Use the regular install script for other distros."
    exit 1
fi

# Get sudo access upfront so we don't get interrupted later
echo "--- Getting sudo access ---"
sudo -v
# Keep sudo alive in background
while true; do sudo -n true; sleep 60; done 2>/dev/null &
KEEP_SUDO_ALIVE_PID="$!"
trap 'kill "$KEEP_SUDO_ALIVE_PID" 2>/dev/null' EXIT

# Update packages first
echo "--- Updating packages ---"
sudo pacman -Syu --noconfirm

# Basic build tools
echo "--- Installing build tools ---"
sudo pacman -S --noconfirm \
    base-devel \
    git \
    cmake \
    ninja \
    pkg-config \
    mesa

# Main dependencies for Umfeld
echo "--- Installing Umfeld dependencies ---"
sudo pacman -S --noconfirm \
    sdl3 \
    harfbuzz \
    freetype2 \
    ffmpeg \
    rtmidi \
    glm \
    portaudio

# Display stuff (X11/Wayland)
echo "--- Installing display dependencies ---"
sudo pacman -S --noconfirm \
    libx11 \
    libxext \
    libxrandr \
    libxcursor \
    libxi \
    libxinerama \
    wayland \
    libxkbcommon \
    wayland-protocols

# Fix CMake being broken on some Arch installs
echo "--- Fixing CMake issues ---"

# Check if CMake is broken (has "dirty" in version or CMAKE_ROOT errors)
CMAKE_VERSION_OUTPUT=$(cmake --version 2>&1 || true)
if echo "$CMAKE_VERSION_OUTPUT" | grep -q "CMAKE_ROOT" || echo "$CMAKE_VERSION_OUTPUT" | grep -q "dirty"; then
    echo "CMake is broken, creating wrapper script..."
    
    # Make a wrapper that fixes the CMAKE_ROOT issue
    cat > cmake_wrapper.sh << 'EOF'
#!/bin/bash
export CMAKE_ROOT=/usr/share/cmake
/usr/bin/cmake "$@"
EOF
    chmod +x cmake_wrapper.sh
    
    echo "Created cmake_wrapper.sh - use this instead of cmake"
else
    echo "CMake looks fine, no wrapper needed"
fi

# Some CMake installs look for /share/cmake instead of /usr/share/cmake
if [ ! -d "/share/cmake" ] && [ -d "/usr/share/cmake" ]; then
    echo "Creating symlink for CMake modules..."
    sudo mkdir -p /share
    sudo ln -sf /usr/share/cmake /share/cmake
    echo "Fixed: /share/cmake -> /usr/share/cmake"
fi

# Double-check everything installed correctly
echo ""
echo "--- Checking what we installed ---"

# Check pkg-config stuff
DEPS=("sdl3" "harfbuzz" "freetype2" "rtmidi" "portaudio-2.0")
FFMPEG_DEPS=("libavcodec" "libavformat" "libavutil" "libswscale" "libswresample" "libavdevice")

echo "pkg-config libraries:"
for dep in "${DEPS[@]}"; do
    if pkg-config --exists "$dep"; then
        VERSION=$(pkg-config --modversion "$dep")
        echo "  ✓ $dep ($VERSION)"
    else
        echo "  ✗ $dep (missing!)"
    fi
done

echo "FFmpeg libraries:"
for dep in "${FFMPEG_DEPS[@]}"; do
    if pkg-config --exists "$dep"; then
        VERSION=$(pkg-config --modversion "$dep")
        echo "  ✓ $dep ($VERSION)"
    else
        echo "  ✗ $dep (missing!)"
    fi
done

# Check GLM
if [ -d "/usr/include/glm" ]; then
    echo "  ✓ GLM headers found"
else
    echo "  ✗ GLM headers missing"
fi

# Check CMake
if [ -f "cmake_wrapper.sh" ]; then
    CMAKE_TEST=$(./cmake_wrapper.sh --version 2>&1 | head -1 || true)
    echo "  ✓ CMake (using wrapper): $CMAKE_TEST"
else
    CMAKE_TEST=$(cmake --version 2>&1 | head -1 || true)
    if echo "$CMAKE_TEST" | grep -q "cmake version"; then
        echo "  ✓ CMake: $CMAKE_TEST"
    else
        echo "  ✗ CMake still broken: $CMAKE_TEST"
    fi
fi

echo ""
echo "=== All done! ==="
echo ""
echo "Everything should be installed now."
echo ""
echo "Arch-specific fixes applied:"
echo "  • CMake wrapper (if needed)"
echo "  • Symlink for CMake modules"
echo "  • All packages from pacman"
echo ""
echo "To build projects:"
echo "  1. Get the umfeld-examples repo"
echo "  2. cd into an example"
if [ -f "cmake_wrapper.sh" ]; then
    echo "  3. Run: /path/to/umfeld/cmake_wrapper.sh -B build ."
    echo "  4. Run: /path/to/umfeld/cmake_wrapper.sh --build build"
else
    echo "  3. Run: cmake -B build ."
    echo "  4. Run: cmake --build build"
fi
echo "  5. Run: ./build/your_app"
echo ""
echo "If CMake gives you trouble, use cmake_wrapper.sh instead."
echo ""
echo "Done! 🎉" 