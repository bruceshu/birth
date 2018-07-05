#! /bin/sh

# build static log system

gcc -c log.c
ar -r liblog.a log.o
