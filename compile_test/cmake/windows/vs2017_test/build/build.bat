set WORKSPACE="%cd%/.."

cd %WORKSPACE%
rmdir /s /q Win32_2017
rmdir /s /q bin

cd build
"%cd%/utf8bom.exe" "%cd%/../src"
call cmake_build_2017_Win32.bat

cd %WORKSPACE%/Win32_2017
"D:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\devenv.com" YouMeCommon.sln /build "Debug|Win32" /project src\YouMeCommon.vcxproj || exit /b 1

