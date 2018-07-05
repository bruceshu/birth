#! /bin/sh

# build file system
# depend on log_system,before you build file system,please clone log_system code

./../log_system/build_static.sh

gcc main.c -L./../log_system/ -llog -lpthread
