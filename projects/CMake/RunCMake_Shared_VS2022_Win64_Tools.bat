CALL CMakeCommon.bat

ECHO *** Set VS2022 specifics and run CMake ***

SET MAINCMAKE=%SRCROOTDIR%\projects\CMake\targets\GUCEF_tag_tools
SET VS22_OUTPUTDIR="%OUTPUTDIR%\VS2022_x64_Tools"

CMake.exe -DBUILD_SHARED_LIBS=ON -G"Visual Studio 17 2022" -A x64 -H"%MAINCMAKE%" -B%VS22_OUTPUTDIR%
PAUSE