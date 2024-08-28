/root/SpadisQT 

adb push Z:\for_docker\david\rk_build\app\AdapsLib\libadaps_swift_decode.so /vendor/lib64/.
adb push Z:\for_docker\david\rk_build\app\SpadisQT\libadaps_swift_decode.so /vendor/lib64/.
adb push Z:\for_docker\david\rk_build\app\SpadisQT\adapsdepthsettings.xml /vendor/etc/camera/.
adb push Z:\for_docker\david\rk_build\app\SpadisQT\SpadisQT /root/.

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

# QSTCamera
基于v4l2的简易照相机，不显示实时摄像头，拍照以yuv格式存储
路径自己在v4l2.h中设置
