#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

VERSION="${VERSION:-1.0.0}"
DEB_DIR="debian"
OUTPUT_DIR="${OUTPUT_DIR:-../../dist}"

mkdir -p "$OUTPUT_DIR"

echo "==> Building Radioform .deb package (version $VERSION)..."

# Build the binaries first
"$(dirname "$0")/build.sh"

# Copy binaries into packaging dir
BIN_SRC="${BUILD_DIR:-build/linux}/src/filter/radioform-filter"
UI_SRC="${BUILD_DIR:-build/linux}/linux-ui/radioform-ui"

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
