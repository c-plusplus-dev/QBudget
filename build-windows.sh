#!/usr/bin/env bash

set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

TOOLCHAIN_FILE="$ROOT/cmake/mingw-x64.cmake"
DEPENDENCIES_DIR="$ROOT/Windows-Dependencies"

VCPKG_ROOT="${VCPKG_ROOT:-$HOME/GitHub/vcpkg}"
VCPKG_TRIPLET="x64-mingw-dynamic"
VCPKG_INSTALLED="$VCPKG_ROOT/installed/$VCPKG_TRIPLET"

COMPILER="x86_64-w64-mingw32-g++"

BUILD_TYPE="Release"
BUILD_DIR="$ROOT/build-windows"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -debug)
            BUILD_TYPE="Debug"
            BUILD_DIR="$ROOT/build-windows-debug"
            ;;
        -h|--help)
            echo "Usage: ./build-windows.sh [-debug]"
            echo
            echo "Options:"
            echo "  -debug    Build with debug symbols and no optimizations"
            echo "  -h        Show this help"
            exit 0
            ;;
        *)
            echo "ERROR: Unknown argument: $1"
            echo "Usage: ./build-windows.sh [-debug]"
            exit 1
            ;;
    esac
    shift
done

echo "==> Checking build environment"

for command in cmake "$COMPILER"; do
    if ! command -v "$command" >/dev/null 2>&1; then
        echo "ERROR: Required command not found: $command"
        exit 1
    fi
done

if [[ ! -f "$TOOLCHAIN_FILE" ]]; then
    echo "ERROR: MinGW toolchain file not found:"
    echo "  $TOOLCHAIN_FILE"
    exit 1
fi

if [[ ! -d "$DEPENDENCIES_DIR" ]]; then
    echo "ERROR: Windows dependencies directory not found:"
    echo "  $DEPENDENCIES_DIR"
    exit 1
fi

if [[ ! -d "$VCPKG_INSTALLED" ]]; then
    echo "ERROR: vcpkg installation not found:"
    echo "  $VCPKG_INSTALLED"
    exit 1
fi

echo
echo "==> Configuring QBudget for Windows x64 ($BUILD_TYPE)"

CMAKE_ARGS=(
    -S "$ROOT"
    -B "$BUILD_DIR"
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE"
    "-DCMAKE_PREFIX_PATH=$DEPENDENCIES_DIR;$VCPKG_INSTALLED"
    "-DCMAKE_FIND_ROOT_PATH=$DEPENDENCIES_DIR;$VCPKG_INSTALLED"
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
)

if [[ "$BUILD_TYPE" == "Debug" ]]; then
    CMAKE_ARGS+=(
        -DCMAKE_CXX_FLAGS_DEBUG="-g3 -O0"
    )
fi

cmake "${CMAKE_ARGS[@]}"

echo
echo "==> Building QBudget ($BUILD_TYPE)"

cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" --parallel

if [[ ! -f "$BUILD_DIR/QBudget.exe" ]]; then
    echo "ERROR: Build completed, but QBudget.exe was not found."
    exit 1
fi

echo
echo "==> Windows $BUILD_TYPE build complete!"
echo
echo "Executable:"
echo "  $BUILD_DIR/QBudget.exe"
