#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."

BUILD_DIR="${BUILD_DIR:-build/linux}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"
PARALLEL="${PARALLEL:-$(nproc)}"

echo "==> Radioform Linux Build"
echo "    Build dir:  $BUILD_DIR"
echo "    Type:       $BUILD_TYPE"
echo "    Prefix:     $INSTALL_PREFIX"
echo "    Parallel:   $PARALLEL"

# Install deps (Debian/Ubuntu)
if command -v apt-get &>/dev/null; then
    echo "==> Installing build dependencies..."
    sudo apt-get update -qq
    sudo apt-get install -y -qq \
        build-essential cmake pkg-config \
        libpipewire-0.3-dev libspa-0.2-dev \
        libgtk-4-dev libayatana-appindicator3-dev \
        libjson-c-dev \
        git curl wget file
fi

# Arch
if command -v pacman &>/dev/null; then
    echo "==> Installing build dependencies..."
    sudo pacman -S --needed --noconfirm \
        base-devel cmake pkg-config \
        pipewire libpipewire \
        gtk4 libappindicator-gtk3 \
        json-c
fi

# Fedora/RHEL
if command -v dnf &>/dev/null; then
    echo "==> Installing build dependencies..."
    sudo dnf install -y \
        gcc-c++ cmake pkgconfig \
        pipewire-devel spaudio-devel \
        gtk4-devel libappindicator-gtk3-devel \
        json-c-devel
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
echo "  radioform-ui           # GTK4 system tray UI"
echo ""
echo "Or as a service:"
echo "  radioform-filter &"
echo "  radioform-ui &"
