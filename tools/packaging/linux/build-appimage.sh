#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

APP_DIR="$SCRIPT_DIR/AppDir"
OUTPUT_DIR="${OUTPUT_DIR:-$SCRIPT_DIR/../../dist}"
VERSION="${VERSION:-1.0.0}"

echo "==> Building Radioform AppImage (version $VERSION)..."

# Build binaries
"$SCRIPT_DIR/build.sh"

mkdir -p "$APP_DIR/usr/bin"
mkdir -p "$APP_DIR/usr/share/applications"
mkdir -p "$APP_DIR/usr/share/icons/hicolor/scalable/apps"
mkdir -p "$APP_DIR/usr/share/radioform/presets"

BUILD_DIR="${BUILD_DIR:-$SCRIPT_DIR/../../../build/linux}"
BIN_SRC="$BUILD_DIR/src/filter/radioform-filter"
UI_SRC="$BUILD_DIR/linux-ui/radioform-ui"

[ -f "$BIN_SRC" ] && cp "$BIN_SRC" "$APP_DIR/usr/bin/"
[ -f "$UI_SRC" ] && cp "$UI_SRC" "$APP_DIR/usr/bin/"

cp "$SCRIPT_DIR/debian/usr/share/applications/radioform.desktop" "$APP_DIR/"
cp "$SCRIPT_DIR/debian/usr/share/icons/hicolor/scalable/apps/radioform.svg" "$APP_DIR/"
cp "$SCRIPT_DIR/debian/usr/share/radioform/presets/"*.json "$APP_DIR/usr/share/radioform/presets/"

cat > "$APP_DIR/AppRun" << 'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "$0")")"
export PATH="$HERE/usr/bin:$PATH"
export XDG_DATA_DIRS="$HERE/usr/share:$XDG_DATA_DIRS"

if [ "$1" = "--filter" ] || [ "$1" = "-f" ]; then
    exec radioform-filter "${@:2}"
else
    exec radioform-ui "$@"
fi
EOF
chmod +x "$APP_DIR/AppRun"

# Download appimagetool if not present
if ! command -v appimagetool &>/dev/null; then
    echo "==> Downloading appimagetool..."
    wget -q -O /tmp/appimagetool \
        "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
    chmod +x /tmp/appimagetool
    APPIMAGETOOL="/tmp/appimagetool"
else
    APPIMAGETOOL="appimagetool"
fi

mkdir -p "$OUTPUT_DIR"
$APPIMAGETOOL "$APP_DIR" "$OUTPUT_DIR/Radioform-${VERSION}-x86_64.AppImage"

echo "✓ Created: $OUTPUT_DIR/Radioform-${VERSION}-x86_64.AppImage"
