#!/bin/bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/../../.."
echo "✓ Build root: $PWD"

BUILD_DIR="${BUILD_DIR:-build/linux}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"
PARALLEL="${PARALLEL:-$(nproc 2>/dev/null || echo 4)}"

echo "==> Radioform Linux Build"
echo "    Build dir:  $BUILD_DIR"
echo "    Type:       $BUILD_TYPE"
echo "    Prefix:     $INSTALL_PREFIX"
echo "    Parallel:   $PARALLEL"

# Install deps
echo "==> Installing build dependencies..."
if command -v apt-get >/dev/null 2>&1; then
    sudo apt-get update -qq
    sudo apt-get install -y -qq \
        build-essential cmake pkg-config \
        libpipewire-0.3-dev libspa-0.2-dev \
        libgtk-3-dev libayatana-appindicator3-dev \
        libjson-c-dev \
        git curl wget file
elif command -v pacman >/dev/null 2>&1; then
    sudo pacman -S --needed --noconfirm \
        base-devel cmake pkg-config \
        pipewire libpipewire \
        gtk3 libappindicator-gtk3 \
        json-c
elif command -v dnf >/dev/null 2>&1; then
    sudo dnf install -y \
        gcc-c++ cmake pkgconfig \
        pipewire-devel spaudio-devel \
        gtk3-devel libappindicator-gtk3-devel \
        json-c-devel
else
    echo "⚠ No supported package manager found. Install deps manually:"
    echo "   build-essential, cmake, pkg-config, libpipewire-0.3-dev,"
    echo "   libspa-0.2-dev, libgtk-3-dev, libayatana-appindicator3-dev, libjson-c-dev"
fi

echo "==> Configuring..."
cmake -S packages/linux -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"

echo "==> Building..."
cmake --build "$BUILD_DIR" -j "$PARALLEL"

echo "==> Installing..."
cmake --install "$BUILD_DIR"

echo ""
echo "✓ Radioform Linux build complete!"
echo ""
echo "Run:"
echo "  radioform-filter       # PipeWire EQ filter daemon"
echo "  radioform-ui           # GTK3 system tray UI"
echo ""
echo "Or as a service:"
echo "  radioform-filter &"
echo "  radioform-ui &"
