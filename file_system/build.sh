#! /bin/sh

# build file system
# depend on log system,before you build file system,please clone log system code

gcc main.c -L./../log_system/ -llog -lpthread
