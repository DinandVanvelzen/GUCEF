/*
 * DV edit: GUCEF builds libarchive with the project files made by ProjectGenerator, without running
 * configure or CMake which would normally generate this file. Of the optional third party libraries
 * only zlib is used.
 */
#if defined( _WIN32 ) && !defined( __CYGWIN__ )
  /* The Windows host configuration libarchive ships for its Android NDK build, with zlib enabled */
  #define __LIBARCHIVE_CONFIG_H_INCLUDED 1
  #include "../contrib/android/config/windows_host.h"
  #undef HAVE_UNISTD_H
  #undef HAVE_DECL_SSIZE_MAX
  #define HAVE_DECL_SSIZE_MAX 0
  #undef HAVE_DECL_SIZE_MAX
  #define HAVE_DECL_SIZE_MAX 0
  #undef HAVE_DECL_TIMEZONE
  #define HAVE_DECL_TIMEZONE 0
  #define HAVE__GET_TIMEZONE 1
  #ifndef HAVE_ZLIB_H
  #define HAVE_ZLIB_H 1
  #endif
  #ifndef HAVE_LIBZ
  #define HAVE_LIBZ 1
  #endif
#elif defined( __ANDROID__ )
  #include "../contrib/android/config/android.h"
#elif defined( __linux__ )
  #include "config_linux.h"
#elif defined( __APPLE__ )
  #include "config_macos.h"
#else
  #error "libarchive: no configuration available for this platform, see config.h"
#endif
