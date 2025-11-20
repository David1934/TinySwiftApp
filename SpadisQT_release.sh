#!/bin/bash

CURRENT_DIR=`pwd`
# Get the current date and time
RELEASE_TIME=$(date +"%Y%m%d%H%M%S")

RELEASE_TIME2=$(date +"%Y/%m/%d %H:%M:%S")
CURRENT_USER=$(whoami)

# Define the list of files to be copied
FILES_LST=(
    "adaps_dtof.cpp"
    "adaps_dtof.h"
    "adaps_dtof_uapi.h"
    "adaps_types.h"
    "adapsdepthsettings.xml"
    "CMakeLists.txt"
    "common.h"
    "depthmapwrapper.h"
    "dtof_main.cpp"
    "dtof_main.h"
    "libadaps_swift_decode.so"
    "LICENSE"
    "main.cpp"
    "Makefile"
    "misc_device.cpp"
    "misc_device.h"
    "README.md"
    "README_zh_CN.md"
    "utils.cpp"
    "utils.h"
    "v4l2.cpp"
    "v4l2.h"
)

Release_content="TinySwiftApp for adaps ADS6401 dToF sensor
Release version: $1
Release time: $RELEASE_TIME2
Released by: $CURRENT_USER

  This is an demo application for ads6401 dToF sensor, named 'TinySwiftApp', run on embedded Linux system;
  We had tested on rk3568 Linux 5.10 system.

  Since the app uses some Linux v4l2 apis, the app can't run on Windows.

  Contact us (https://adapsphotonics.com/).
"

# Function to copy files
copy_file_list_from_src_to_dest() {
    local SOURCE_DIR=$1
    local TARGET_DIR=$2
    local FILES=("${@:3}")  # Get all arguments starting from the 3rd as the file list

    for FILE in "${FILES[@]}"; do
        # Check if the source file exists
        if [ -f "$SOURCE_DIR/$FILE" ]; then
            # Create the target directory (if it doesn't exist)
            TARGET_FILE_DIR=$(dirname "$TARGET_DIR/$FILE")
            mkdir -p "$TARGET_FILE_DIR"
            
            # Copy the file
            cp --preserve=mode,timestamps "$SOURCE_DIR/$FILE" "$TARGET_DIR/$FILE"
            echo "Copied: $SOURCE_DIR/$FILE -> $TARGET_DIR/$FILE"
        else
            echo "Source file not found: $SOURCE_DIR/$FILE"
        fi
    done

    echo "Copy completed!"
}

# Check if a command-line argument is provided
if [ $# -eq 0 ]; then
    echo "Error: Please provide the Release version number as a command-line argument."
    exit 1
fi

# Get the command-line argument as the firmware version
FW_VERSION="$1"

# Define the release folder name
RELEASE_FOLDER="TinySwiftApp_${FW_VERSION}_Release${RELEASE_TIME}"

# Create the release folder
mkdir -p "$RELEASE_FOLDER"

# copy original files to the release folder
copy_file_list_from_src_to_dest "$CURRENT_DIR" "$RELEASE_FOLDER" "${FILES_LST[@]}"

# generate Release file
echo "$Release_content" > "$RELEASE_FOLDER/Release.txt"

# Use the zip command to package the release folder into a zip file
zip -r "${RELEASE_FOLDER}.zip" "$RELEASE_FOLDER"

# Optional: Delete the release folder
rm -r "$RELEASE_FOLDER"

echo "The release package has been generated at: $CURRENT_DIR/${RELEASE_FOLDER}.zip"
echo "Done!"

