#!/bin/sh

workspace=`pwd`

rm -rf ${workspace}/build
mkdir ${workspace}/build
cd ${workspace}/build
cmake ..

cd src/helloLib
make

cd ..
make
