#!/bin/bash

set -euo pipefail

# Always run relative to the project root.
ROOT="$(cd "$(dirname "$0")" && pwd)"

command -v rsvg-convert >/dev/null || {
    echo "Error: Install librsvg (brew install librsvg)"
    exit 1
}

command -v magick >/dev/null || {
    echo "Error: Install ImageMagick (brew install imagemagick)"
    exit 1
}

command -v iconutil >/dev/null || {
    echo "Error: iconutil is required (run this on macOS)"
    exit 1
}

generate_icons() {
    local SVG="$1"
    local NAME="$2"
    local DIRECTORY="$3"

    local ICONSET="$DIRECTORY/$NAME.iconset"
    local ICNS="$DIRECTORY/$NAME.icns"
    local ICO="$DIRECTORY/$NAME.ico"

    local TEMP
    TEMP="$(mktemp -d)"

    echo "Generating $NAME..."

    mkdir -p "$DIRECTORY"
    rm -rf "$ICONSET"
    mkdir -p "$ICONSET"

    # macOS iconset resolutions.
    local SIZES=(
        "16:icon_16x16"
        "32:icon_16x16@2x"
        "32:icon_32x32"
        "64:icon_32x32@2x"
        "128:icon_128x128"
        "256:icon_128x128@2x"
        "256:icon_256x256"
        "512:icon_256x256@2x"
        "512:icon_512x512"
        "1024:icon_512x512@2x"
    )

    for ENTRY in "${SIZES[@]}"; do
        SIZE="${ENTRY%%:*}"
        FILENAME="${ENTRY#*:}"

        rsvg-convert \
            -w "$SIZE" \
            -h "$SIZE" \
            "$SVG" \
            -o "$ICONSET/$FILENAME.png"
    done

    # macOS ICNS.
    iconutil -c icns \
        "$ICONSET" \
        -o "$ICNS"

    # Windows ICO.
    rsvg-convert \
        -w 1024 \
        -h 1024 \
        "$SVG" \
        -o "$TEMP/icon.png"

    magick "$TEMP/icon.png" \
        -define icon:auto-resize=256,128,64,48,32,24,16 \
        "$ICO"

    rm -rf "$TEMP"

    echo "Generated:"
    echo "  $ICNS"
    echo "  $ICO"
    echo "  $ICONSET"
}

generate_icons \
    "$ROOT/QBudget.svg" \
    "QBudget" \
    "$ROOT"

generate_icons \
    "$ROOT/installer/QBudget-Installer.svg" \
    "QBudget-Installer" \
    "$ROOT/installer"

echo "All icons generated successfully!"
