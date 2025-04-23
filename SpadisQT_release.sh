#!/bin/bash

CURRENT_DIR=`pwd`
# Get the current date and time
RELEASE_TIME=$(date +"%Y%m%d%H%M%S")

RELEASE_TIME2=$(date +"%Y/%m/%d %H:%M:%S")
CURRENT_USER=$(whoami)

# Define the list of files to be copied
FILES_LST=(
#    "ads6401_roi_sram_test_data.h"
    "adapsdepthsettings.xml"
    "adaps_dtof.cpp"
    "adaps_dtof.h"
    "host_comm.cpp"
    "host_comm.h"
    "adaps_dtof_uapi.h"
    "adaps_types.h"
    "common.h"
    "depthmapwrapper.h"
    "FrameProcessThread.cpp"
    "FrameProcessThread.h"
    "globalapplication.cpp"
    "globalapplication.h"
    "libadaps_swift_decode.so"
    "libAdapsSender.so"
    "main.cpp"
    "mainwindow.cpp"
    "mainwindow.h"
    "mainwindow.ui"
#    "Makefile"
    "SpadisQT.pro"
    "SpadisQT_console.pro"
    "utils.cpp"
    "utils.h"
    "v4l2.cpp"
    "v4l2.h"
)

Readme_content="SpadisQT App for adaps ADS6401 dToF sensor
Release version: $1
Release time: $RELEASE_TIME2
Released by: $CURRENT_USER

  This is an demo application for ads6401 dToF sensor, named 'SpadisQT', run on embedded Linux system;
  We had tested on rk3568 Linux 5.10 system.

  It can be built with QT 5.xx for Linux. (Since the app uses some v4l2 apis, the app can't run on Windows).
  
  Please make sure your develop board support QT before building and running the SpadisQT app,
  you should study it yourself, we can't answer this kind of questions,-^-;

  There are two kinds of dToF modules from ads6401 chip, one is SPOT module, the other is FLOOD module;

  You can select the proper type in SpadisQT.pro:
    please comment 'DEFINES += CONFIG_ADAPS_SWIFT_FLOOD' line if you'd like to use ads6401 as a SPOT module;
    please uncomment 'DEFINES += CONFIG_ADAPS_SWIFT_FLOOD' line if you'd like to use ads6401 as a FLOOD module;

  Once you built the app sucessfully, you can use the following steps to deployment SpadisQT to your develop board:

  Step 1: create some directories with the below commands
    mkdir /vendor
    mkdir /vendor/lib64
    mkdir /vendor/etc
    mkdir /vendor/etc/camera
    mkdir /data
    mkdir /data/vendor
    mkdir /data/vendor/camera

  Step 2A: copy some files to develop board from the build directory if you develop board support ssh login;
    scp Z:\rk_build\app\SpadisQT\libadaps_swift_decode.so root@[fe80::5047:afff:fe7a:1234]:/vendor/lib64/libadaps_swift_decode.so
    scp Z:\rk_build\app\SpadisQT\libAdapsSender.so root@[fe80::5047:afff:fe7a:1234]:/vendor/lib64/libAdapsSender.so
    scp Z:\rk_build\app\SpadisQT\adapsdepthsettings.xml root@[fe80::5047:afff:fe7a:1234]:/vendor/etc/camera/adapsdepthsettings.xml
    scp Z:\rk_build\app\SpadisQT\SpadisQT root@[fe80::5047:afff:fe7a:1234]:/usr/bin/SpadisQT

  (Note: Replace 'Z:\rk_build\app\SpadisQT' with your real path, 
      and replace '[fe80::5047:afff:fe7a:1234]' with your real ip address;)

  Step 2B: copy some files to develop board from the build directory if you develop board support adb push;
    adb push Z:\rk_build\app\SpadisQT\libadaps_swift_decode.so /vendor/lib64/.
    adb push Z:\rk_build\app\SpadisQT\libAdapsSender.so /vendor/lib64/.
    adb push Z:\rk_build\app\SpadisQT\adapsdepthsettings.xml /vendor/etc/camera/.
    adb push Z:\rk_build\app\SpadisQT\SpadisQT /usr/bin/.

  (Note: Replace 'Z:\rk_build\app\SpadisQT' with your real path;)

  Step 3: add the executable attribute for SpadisQT file
    chmod +x /usr/bin/SpadisQT
  
  If you have any questions, please feel free to contact us (https://adapsphotonics.com/).
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
RELEASE_FOLDER="SpadisQT_${FW_VERSION}_Release${RELEASE_TIME}"

# Create the release folder
mkdir -p "$RELEASE_FOLDER"

# copy original files to the release folder
copy_file_list_from_src_to_dest "$CURRENT_DIR" "$RELEASE_FOLDER" "${FILES_LST[@]}"

# generate Readme file
echo "$Readme_content" > "$RELEASE_FOLDER/Readme.txt"

# Use the zip command to package the release folder into a zip file
zip -r "${RELEASE_FOLDER}.zip" "$RELEASE_FOLDER"

# Optional: Delete the release folder
rm -r "$RELEASE_FOLDER"

echo "The release package has been generated at: $CURRENT_DIR/${RELEASE_FOLDER}.zip"
echo "Done!"

