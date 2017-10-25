#! /usr/bin/env bash

set -e
ROOT_PATH=`pwd`
IJK_BUILD_PATH=$ROOT_PATH/android
BUILD_PATH=$IJK_BUILD_PATH/contrib

echo "========================="
echo "build ffmpeg begin"
echo "========================="
cd $BUILD_PATH
./compile-ffmpeg.sh all
cd -

echo "========================="
echo "build ijkplayer begin"
echo "========================="
cd $IJK_BUILD_PATH
./compile-ijk.sh all

echo "========================="
echo "build ijkplayer end"
echo "========================="

echo "========================="
echo "mkdir output"
echo "========================="
cd $ROOT_PATH
mkdir -p output/arm64 
cp $IJK_BUILD_PATH/ijkplayer/ijkplayer-arm64/src/main/libs/arm64-v8a/* $ROOT_PATH/output/arm64

mkdir -p output/armv5
cp $IJK_BUILD_PATH/ijkplayer/ijkplayer-armv5/src/main/libs/armeabi/* $ROOT_PATH/output/armv5

mkdir -p output/armv7a
cp $IJK_BUILD_PATH/ijkplayer/ijkplayer-armv7a/src/main/libs/armeabi-v7a/* $ROOT_PATH/output/armv7a

mkdir -p output/x86
cp $IJK_BUILD_PATH/ijkplayer/ijkplayer-x86/src/main/libs/x86/* $ROOT_PATH/output/x86

mkdir -p output/x86_64
cp $IJK_BUILD_PATH/ijkplayer/ijkplayer-x86_64/src/main/libs/x86_64/* $ROOT_PATH/output/x86_64

tar zcvf ijkplayer_release.tar.gz output
. ./ijkmedia/ijkplayer/ijk_version.sh
BuildTime=$(date +"%Y%m%d")
mv ijkplayer_release.tar.gz Android_ijkplayer_${VERSION}_${BuildTime}_release.tar.gz







