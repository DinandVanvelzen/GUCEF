CALL CMakeCommon.bat

ECHO *** Set VS2022 specifics and run CMake ***

SET MAINCMAKE=%SRCROOTDIR%\projects\CMake\targets\GUCEF_exe_FileSorter
SET VS22_OUTPUTDIR="%OUTPUTDIR%\VS2022_x64_FileSorter"

CMake.exe -DBUILD_SHARED_LIBS=ON -G"Visual Studio 17 2022" -A x64 -H"%MAINCMAKE%" -B%VS22_OUTPUTDIR%
PAUSE