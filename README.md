mkdir /vendor
mkdir /vendor/lib64
mkdir /vendor/etc
mkdir /vendor/etc/camera


scp Z:\for_docker\david\rk_build\app\SpadisQT\libadaps_swift_decode.so root@[fe80::5047:afff:fe7a:1234]:/vendor/lib64/libadaps_swift_decode.so
scp Z:\for_docker\david\rk_build\app\SpadisQT\adapsdepthsettings.xml root@[fe80::5047:afff:fe7a:1234]:/vendor/etc/camera/adapsdepthsettings.xml
scp Z:\for_docker\david\rk_build\app\SpadisQT\SpadisQT root@[fe80::5047:afff:fe7a:1234]:/usr/bin/SpadisQT
scp Z:\for_docker\david\rk_build\app\AdapsLib\libadaps_swift_decode.so root@[fe80::5047:afff:fe7a:1234]:/vendor/lib64/libadaps_swift_decode.so

adb push Z:\for_docker\david\rk_build\app\AdapsLib\libadaps_swift_decode.so /vendor/lib64/.
adb push Z:\for_docker\david\rk_build\app\SpadisQT\libadaps_swift_decode.so /vendor/lib64/.
adb push Z:\for_docker\david\rk_build\app\SpadisQT\adapsdepthsettings.xml /vendor/etc/camera/.
adb push Z:\for_docker\david\rk_build\app\SpadisQT\SpadisQT /usr/bin/.

chmod +x /usr/bin/SpadisQT


4. 因为rk3568的屏是类似于手机屏，原本是纵向显示的，SpadisQT界面显示好小，秦宇建议旋转90度，通过下列命令
   可将屏幕旋转270度，同时调整mainwindow.ui的布局大小，大部分控件调大后，比原来更好看一些；
     rk3568开发板屏幕桌面如何旋转90度？
        cat /sys/kernel/debug/dri/0/summary
        查看Connector名字，旋转如：
        echo "output:DSI-1:rotate270" > /tmp/.weston_drm.conf
        等待界面刷新

FHR test pattern的raw data一致性验证：

export expected_frame_md5sum="85f24805d05ed40d63426bc193094e84"
echo 0x8 > /sys/kernel/debug/adaps/dbg_ctrl


adb push Z:\for_docker\david\rk_build.QT_swift\buildroot/output/rockchip_rk3568/target/usr/lib/libstdc++.so.6.0.29 /usr/lib/.

root@ok3588:/usr/lib# ls -l libstdc++.so*
lrwxrwxrwx 1 root root      19 Mar  9  2023 libstdc++.so -> libstdc++.so.6.0.28
lrwxrwxrwx 1 root root      19 Mar  9  2023 libstdc++.so.6 -> libstdc++.so.6.0.28
-rwxr-xr-x 1 root root 1556632 Mar  9  2023 libstdc++.so.6.0.28
-rw-r--r-- 1 root root    2582 Mar  9  2023 libstdc++.so.6.0.28-gdb.py
-rw-rw-rw- 1 root root 1749440 Feb  2  2024 libstdc++.so.6.0.29
root@ok3588:/usr/lib# rm libstdc++.so.6
root@ok3588:/usr/lib# ln -s libstdc++.so.6.0.29 libstdc++.so.6
root@ok3588:/usr/lib# rm libstdc++.so
root@ok3588:/usr/lib# ln -s libstdc++.so.6.0.29 libstdc++.so


