## **TinySwiftApp for ADAPS ADS6401 dToF Sensor**
([Chinese version](README_zh_CN.md))

This is a demo application for the ADS6401 dToF sensor from [ADAPS Photonics](https://adapsphotonics.com/), 
named 'TinySwiftApp', designed to run on embedded Linux systems.

Using the V4L2 framework, the raw MIPI data from our Swift dToF sensor is captured, processed by our proprietary algorithm library, 
and converted into depth or point-cloud data.

It has been tested on RK3568 with Linux 5.10 kernel.

Since the app uses some Linux v4l2 apis, the app can't run on Windows.


There are four types of dToF modules based on the ADS6401 chip:  
- **Spot module**  
- **Small-Flood module**  
- **Big FoV Flood module**  
- **Big FoV V2 Flood module**  

The Linux kernel driver (`ads6401`) reports the module type when the `ADAPS_GET_DTOF_MODULE_STATIC_DATA` ioctl command is executed.

### **Deployment Steps**  
Follow these steps to deploy TinySwiftApp to your development board:

#### **Step 1: Create Required Directories**  
Run the following commands:  
```bash
mkdir -p /vendor/lib64
mkdir -p /vendor/etc/camera
mkdir -p /data/vendor/camera
```

#### **Step 2A: Copy Files via SSH**
If your board supports SSH, use these commands (replace your real build path and IP address):
```
scp /your/build/path/libadaps_swift_decode.so root@[your_board_ip]:/vendor/lib64/
scp /your/build/path/adapsdepthsettings.xml root@[your_board_ip]:/vendor/etc/camera/
scp /your/build/path/TinySwiftApp root@[your_board_ip]:/usr/bin/
```

#### **Step 2B: Copy Files via ADB**
If your board supports ADB, use these commands (replace your real build path):
```
adb push /your/build/path/libadaps_swift_decode.so /vendor/lib64/
adb push /your/build/path/adapsdepthsettings.xml /vendor/etc/camera/
adb push /your/build/path/TinySwiftApp /usr/bin/
```

#### **Step 3: Set Executable Permissions**

```
chmod +x /usr/bin/TinySwiftApp
```

For questions, contact us: [ADAPS Photonics](https://adapsphotonics.com/).

### **License**
TinySwiftApp is licensed under:

[GNU LGPLv3](https://opensource.org/licenses/LGPL-3.0)
