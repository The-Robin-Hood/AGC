#!/bin/bash

echo "WebRTC-Audio-Processing Builder Started"
echo "========================"

git clone https://gitlab.freedesktop.org/pulseaudio/webrtc-audio-processing.git

# Configuration variables
OUTPUT_FOLDER=$PWD/"agc"
REPO="webrtc-audio-processing"
BUILD_DIR=$REPO"/build"
WEBRTC_SOURCE=$REPO"/webrtc"
WEBRTC_DEST="$OUTPUT_FOLDER/include/webrtc"

# Build WebRTC using Meson
echo "Building WebRTC..."
cd webrtc-audio-processing
meson . $BUILD_DIR -Dprefix=$OUTPUT_FOLDER
ninja -C $BUILD_DIR
ninja -C $BUILD_DIR install
cd ..
echo "Build files created in $OUTPUT_FOLDER folder"
echo

# Find the Abseil directory dynamically
ABSL_DIR=$(find $REPO/subprojects -maxdepth 1 -type d -name "abseil-cpp-*" | head -n 1)
if [ -z "$ABSL_DIR" ]; then
    echo "Error: Could not find Abseil directory in subprojects/"
    exit 1
fi

if [ -z "$OUTPUT_FOLDER" ]; then
    mkdir $OUTPUT_FOLDER
fi

ABSL_SOURCE="$ABSL_DIR/absl"
ABSL_DEST="$OUTPUT_FOLDER/include/absl"

echo "Using Abseil from: $ABSL_DIR"

# Function to copy header files
copy_headers() {
    local source=$1
    local dest=$2
    
    echo "Creating include files:"
    echo "Source: $source"
    echo "Destination: $dest"
    
    mkdir -p "$dest"
    
    find "$source" -type f \( -name "*.h" -o -name "*.inc" \) -exec bash -c \
        'dest="${0/$1/$2}"; mkdir -p "$(dirname "$dest")"; cp "$0" "$dest"' \
        {} "$source" "$dest" \;
        
    echo "Header files and inc files copied successfully"
    echo
}

# Generate WebRTC include files
echo "Generating WebRTC include files..."
copy_headers "$WEBRTC_SOURCE" "$WEBRTC_DEST"

# Generate Abseil include files
echo "Generating Abseil include files..."
copy_headers "$ABSL_SOURCE" "$ABSL_DEST"

echo "Build process completed successfully"