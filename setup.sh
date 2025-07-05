#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ -z "$1" ]; then
    echo "Usage: $(basename "$0") <architecture>"
    echo "- architectures:"
    echo "       - x86_64"
    echo "       - arm64"
    exit 1
fi

ARCH="$1"

# ========== HANDLE PARAMETER ==========
if [[ "$ARCH" == "arm64" ]]; then
    BASE_PATH_64BITS="$SCRIPT_DIR/Liho/app/src/main/libs/arm64-v8a"
    BASE_PATH_32BITS="$SCRIPT_DIR/Liho/app/src/main/libs/armeabi-v7a"

    DESTINATION_MODULE_PATH_64BITS="$SCRIPT_DIR/LihoMagiskModule/zygisk/arm64-v8a.so"
    DESTINATION_MODULE_PATH_32BITS="$SCRIPT_DIR/LihoMagiskModule/zygisk/armeabi-v7a.so"
elif [[ "$ARCH" == "x86_64" ]]; then
    BASE_PATH_64BITS="$SCRIPT_DIR/Liho/app/src/main/libs/x86_64"
    BASE_PATH_32BITS="$SCRIPT_DIR/Liho/app/src/main/libs/x86"

    DESTINATION_MODULE_PATH_64BITS="$SCRIPT_DIR/LihoMagiskModule/zygisk/x86_64.so"
    DESTINATION_MODULE_PATH_32BITS="$SCRIPT_DIR/LihoMagiskModule/zygisk/x86.so"
else
    echo "Invalid architecture: $ARCH"
    exit 1
fi

# ========== BUILD .so ==========
echo "Running ndk-build in: $SCRIPT_DIR/Liho/app/src/main"
pushd "$SCRIPT_DIR/Liho/app/src/main" > /dev/null
ndk-build
popd > /dev/null

# ========== COPY .SO TO MODULE ==========
echo

SOURCE_LIHO="libliho.so"

cp "$BASE_PATH_64BITS/$SOURCE_LIHO" "$DESTINATION_MODULE_PATH_64BITS"
cp "$BASE_PATH_32BITS/$SOURCE_LIHO" "$DESTINATION_MODULE_PATH_32BITS"

# ========== ZIP MODULE ==========
echo
echo "Zipping Liho Magisk Module..."

MODULE_DIR="$SCRIPT_DIR/LihoMagiskModule"
ZIP_PATH="$SCRIPT_DIR/LihoMagiskModule.zip"

rm -f "$ZIP_PATH"
cd "$MODULE_DIR"
zip -r "$ZIP_PATH" * > /dev/null
cd - > /dev/null

echo "Zip created at: $ZIP_PATH"