#! /bin/sh


rm -rf libs/ obj

ndk-build APP_ABI=armeabi-v7a NDK_PROJECT_PATH=./ APP_BUILD_SCRIPT=./Android.mk
