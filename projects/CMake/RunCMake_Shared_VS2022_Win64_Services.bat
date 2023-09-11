CALL CMakeCommon.bat

ECHO *** Set VS2022 specifics and run CMake ***

SET MAINCMAKE=%SRCROOTDIR%\projects\CMake\targets\GUCEF_tag_services
SET VS19_OUTPUTDIR="%OUTPUTDIR%\VS2022_x64_Services"

CMake.exe -DBUILD_SHARED_LIBS=ON -G"Visual Studio 17 2022" -A x64 -H"%MAINCMAKE%" -B%VS19_OUTPUTDIR%
PAUSE