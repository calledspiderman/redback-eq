#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

VERSION="${VERSION:-1.0.0}"
DEB_DIR="$SCRIPT_DIR/debian"
OUTPUT_DIR="${OUTPUT_DIR:-$SCRIPT_DIR/../../dist}"

mkdir -p "$OUTPUT_DIR"

echo "==> Building Radioform .deb package (version $VERSION)..."

# Build the binaries first
"$SCRIPT_DIR/build.sh"

# Copy binaries into packaging dir
BUILD_DIR="${BUILD_DIR:-$SCRIPT_DIR/../../../build/linux}"
BIN_SRC="$BUILD_DIR/src/filter/radioform-filter"
UI_SRC="$BUILD_DIR/linux-ui/radioform-ui"

if [ -f "$BIN_SRC" ]; then
    cp "$BIN_SRC" "$DEB_DIR/usr/bin/"
fi
if [ -f "$UI_SRC" ]; then
    cp "$UI_SRC" "$DEB_DIR/usr/bin/"
fi

# Update version in control file
sed -i "s/Version: .*/Version: $VERSION/" "$DEB_DIR/DEBIAN/control"

# Build .deb
dpkg-deb --build "$DEB_DIR" "$OUTPUT_DIR/radioform_${VERSION}_amd64.deb"

echo "✓ Created: $OUTPUT_DIR/radioform_${VERSION}_amd64.deb"
