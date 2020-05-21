@echo off

cd ..
mkdir Win64_2017
cd Win64_2017

cmake .. -G"Visual Studio 15 2017 Win64" -DYOUME_MINI=0 -DWIN64=1 -DCMAKE_CONFIGURATION_TYPES=Debug
