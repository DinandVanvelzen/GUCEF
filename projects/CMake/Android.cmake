# Android.cmake - Thin wrapper delegating to the NDK's official toolchain
#
# Requires ANDROID_NDK_ROOT environment variable to be set.
# Set ANDROID_ABI and ANDROID_PLATFORM before including, or rely on the
# defaults set here. The CMake preset (_comp-Android-*) sets these for you.

if(NOT DEFINED ANDROID_NDK_ROOT)
    if(DEFINED ENV{ANDROID_NDK_ROOT})
        set(ANDROID_NDK_ROOT "$ENV{ANDROID_NDK_ROOT}")
    else()
        message(FATAL_ERROR "ANDROID_NDK_ROOT environment variable is not set. "
            "Install the Android NDK and set ANDROID_NDK_ROOT to its root directory.")
    endif()
endif()

if(NOT DEFINED ANDROID_ABI)
    set(ANDROID_ABI "arm64-v8a" CACHE STRING "Android ABI to target" FORCE)
endif()

if(NOT DEFINED ANDROID_PLATFORM)
    set(ANDROID_PLATFORM "android-21" CACHE STRING "Minimum Android API level" FORCE)
endif()

include("${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake")
