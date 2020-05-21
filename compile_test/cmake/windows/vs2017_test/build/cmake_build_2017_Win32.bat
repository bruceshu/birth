@echo off

cd ..
mkdir Win32_2017
cd Win32_2017

cmake .. -G"Visual Studio 15 2017" -DWIN32_2017=1 -DCMAKE_CONFIGURATION_TYPES=Debug
