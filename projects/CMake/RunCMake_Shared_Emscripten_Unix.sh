#!/bin/bash
#


# Turn on tracing, we want to see what's going on
#set -x

clear
echo "*** Invoking CMakeCommon ***"
. CMakeCommon.sh
echo "*** Invoking CMake ***"
EMS_OUTPUTDIR="$OUTPUTDIR/Emscripten"
echo "Output dir for this CMake target = $EMS_OUTPUTDIR"
echo "CMake will use source root: $SRCROOTDIR"


[ -d emscripten ] || mkdir emscripten
cd emscripten
[ -f Makefile ] || export VERBOSE="VERBOSE=1"
[ -f Makefile ] || cmake -DCMAKE_TOOLCHAIN_FILE=Emscripten.toolchain.cmake -G "Unix Makefiles" -H"$SRCROOTDIR" -B$EMS_OUTPUTDIR

#Do we want to actually build?
#make -j2 $VERBOSE $*

echo "Press enter to continue..."
line=""
read line