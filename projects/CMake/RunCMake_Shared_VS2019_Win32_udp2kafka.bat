CALL CMakeCommon.bat

ECHO *** Set VS2019 specifics and run CMake ***

SET MAINCMAKE=%SRCROOTDIR%\projects\CMake\targets\GUCEF_exe_udp2kafka
SET VS19_OUTPUTDIR="%OUTPUTDIR%\VS2019_Win32_udp2kafka"

CMake.exe -DBUILD_SHARED_LIBS=ON -G"Visual Studio 16 2019" -A Win32 -H"%MAINCMAKE%" -B%VS19_OUTPUTDIR%
PAUSE