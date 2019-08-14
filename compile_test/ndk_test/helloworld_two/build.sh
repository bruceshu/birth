#! /bin/sh

# clear project
rm -rf libs/ obj/

# build project
ndk-build APP_ABI=armeabi-v7a NDK_PROJECT_PATH=./ APP_BUILD_SCRIPT=./Android.mk APP_STL=stlport_static
