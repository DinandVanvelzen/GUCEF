CALL CMakeCommon.bat

ECHO *** Setup emscripten and run CMake ***

REM Set up environment variables for this shell
CALL "%EMSDK%\emsdk_env.bat"

SET MAINCMAKE=%SRCROOTDIR%\projects\CMake\targets\GUCEF_tag_platform
SET EM_OUTPUTDIR=%OUTPUTDIR%\Emscripten_Platform
SET TOOLCHAIN=%SRCROOTDIR%\projects\CMake\Emscripten.toolchain.cmake

cmake -DCMAKE_TOOLCHAIN_FILE="%SRCROOTDIR%\projects\CMake\Emscripten.toolchain.cmake" ^
      -G "Ninja" ^
      -DPLATFORM=wasm ^
      -S "%MAINCMAKE%" -B "%EM_OUTPUTDIR%"

cmake --build "%EM_OUTPUTDIR%"

PAUSE