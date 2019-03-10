#!/bin/bash
#make clean

export NDK=/home/pierce/Android/Sdk/ndk-bundle16
export SYSROOT=$NDK/platforms/android-21/arch-arm/
export TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
export TMPDIR=/home/pierce/disk/AndroidProject/ffmpeg-2.6.9/build
export CPU=arm
export PREFIX=$(pwd)/android/$CPU
export ADDI_CFLAGS="-marm"

./configure --target-os=linux \
--prefix=$PREFIX --arch=arm \
--disable-doc \
--enable-shared \
--disable-static \
--disable-yasm \
--disable-symver \
--enable-gpl \
--disable-ffmpeg \
--disable-ffplay \
--disable-ffprobe \
--disable-ffserver \
--disable-doc \
--disable-symver \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
--enable-cross-compile \
--sysroot=$SYSROOT \
--extra-cflags="-Os -fpic $ADDI_CFLAGS" \
--extra-ldflags="$ADDI_LDFLAGS" \
$ADDITIONAL_CONFIGURE_FLAG
make
make install
./configure \ --prefix=$PREFIX \ --enable-shared \ --disable-static \ --disable-doc \ --disable-ffmpeg \ --disable-ffplay \ --disable-ffprobe \ --disable-ffserver \ --disable-avdevice \ --disable-doc \ --disable-symver \ --cross-prefix=${TOOLCHAIN}/bin/arm-linux-androideabi- \ --target-os=linux \ --arch=arm \ --enable-cross-compile \ --sysroot=$SYSROOT \ --extra-cflags="-Os -fpic $ADDI_CFLAGS" \ --extra-ldflags="$ADDI_LDFLAGS" \ $ADDITIONAL_CONFIGURE_FLAG
