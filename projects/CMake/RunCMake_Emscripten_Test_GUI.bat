CALL CMakeCommon.bat

ECHO *** Setup emscripten and run CMake ***

REM Set up environment variables for this shell
CALL "%EMSDK%\emsdk_env.bat"

SET MAINCMAKE=%SRCROOTDIR%\projects\CMake\targets\GUCEF_exe_gucefGUI_TestApp
SET EM_OUTPUTDIR=%OUTPUTDIR%\Emscripten_Test_GUI
SET TOOLCHAIN=%SRCROOTDIR%\projects\CMake\Emscripten.toolchain.cmake

cmake -DCMAKE_TOOLCHAIN_FILE="%SRCROOTDIR%\projects\CMake\Emscripten.toolchain.cmake" ^
      -G "Ninja" ^
      -DPLATFORM=wasm ^
      -S "%MAINCMAKE%" -B "%EM_OUTPUTDIR%" -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build "%EM_OUTPUTDIR%"

PAUSE