#ifndef __Custom_Config_H_
#define __Custom_Config_H_

/* Build configuration of the Ogre modules in this code base.
   This is the equivalent of the OgreBuildSettings.h which the upstream CMake build
   generates from CMake/Templates/OgreBuildSettings.h.in. The values that depend on the
   platform are chosen with the preprocessor so one file serves all platforms. */

// Define ogre version
#define OGRE_VERSION_MAJOR 14
#define OGRE_VERSION_MINOR 6
#define OGRE_VERSION_PATCH 3
#define OGRE_VERSION_SUFFIX ""
#define OGRE_VERSION_NAME "Tsathoggua"

/* Static library builds: the modules of WebAssembly builds are static libraries, which are linked
   into the application together with the plugins that are used. All other platforms use shared libraries. */
#if defined(__EMSCRIPTEN__)
#   define OGRE_STATIC_LIB
#endif

/* Builds without NDEBUG are debug builds. Windows overrides this in OgrePlatform.h with _DEBUG. */
#if defined(NDEBUG)
#   define OGRE_DEBUG_MODE 0
#else
#   define OGRE_DEBUG_MODE 1
#endif

/** There are three modes for resource lookup
0 - LEGACY - search in all groups twice - for case sensitive and insensitive lookup
1 - PEDANTIC - require an explicit resource group. Case sensitive lookup.
2 - STRICT - search in default group if not specified otherwise. Case sensitive lookup.
*/
#define OGRE_RESOURCEMANAGER_STRICT 2

/* #undef OGRE_NODELESS_POSITIONING */

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#   define OGRE_CONFIG_BIG_ENDIAN
#else
#   define OGRE_CONFIG_LITTLE_ENDIAN
#endif

/** If set to 1, Real is typedef'ed to double. Otherwise, Real is typedef'ed
    to float. Setting this allows you to perform mathematical operations in the
    CPU (Quaternion, Vector3 etc) with more precision, but bear in mind that the
    GPU still operates in single-precision mode.
*/
#define OGRE_DOUBLE_PRECISION 0

#define OGRE_NODE_INHERIT_TRANSFORM 0

/** There are three modes for handling asserts in OGRE:
0 - STANDARD - Standard asserts in debug builds, nothing in release builds
1 - RELEASE_EXCEPTIONS - Standard asserts in debug builds, exceptions in release builds
2 - EXCEPTIONS - Exceptions in debug builds, exceptions in release builds
*/
#define OGRE_ASSERT_MODE 2

/** Support for multithreading, there are 3 options

OGRE_THREAD_SUPPORT = 0
    No support for threading.
OGRE_THREAD_SUPPORT = 1
    Thread support for background loading, by both loading and constructing resources
    in a background thread. Resource management and SharedPtr handling becomes
    thread-safe, and resources may be completely loaded in the background.
    The places where threading is available are clearly
    marked, you should assume state is NOT thread safe unless otherwise
    stated in relation to this flag.
OGRE_THREAD_SUPPORT = 2
    Thread support for background resource preparation. This means that resource
    data can streamed into memory in the background, but the final resource
    construction (including RenderSystem dependencies) is still done in the primary
    thread. Has a lower synchronisation primitive overhead than full threading
    while still allowing the major blocking aspects of resource management (I/O)
    to be done in the background.
OGRE_THREAD_SUPPORT = 3
    No synchronisation primitive overhead. WorkQueue is threaded.

WebAssembly builds are without threads, upstream considers threading unstable there.
*/
/** Provider for threading functionality, there are 4 options.

OGRE_THREAD_PROVIDER = 0
    No support for threading.
OGRE_THREAD_PROVIDER = 1
    Boost libraries provide threading functionality.
OGRE_THREAD_PROVIDER = 2
    Poco libraries provide threading functionality.
OGRE_THREAD_PROVIDER = 4
    Standard library provides threading functionality. (requires compiler support)
*/
#if defined(__EMSCRIPTEN__)
#   define OGRE_THREAD_SUPPORT 0
#   define OGRE_THREAD_PROVIDER 0
#else
#   define OGRE_THREAD_SUPPORT 3
#   define OGRE_THREAD_PROVIDER 4
#endif

#define OGRE_NO_MESHLOD 0

/** Disables use of the internal image codec for loading DDS files. */
#define OGRE_NO_DDS_CODEC 0

/** Disables use of the internal image codec for loading PVRTC files. */
#define OGRE_NO_PVRTC_CODEC 1

/** Disables use of the internal image codec for loading ETC files. */
#define OGRE_NO_ETC_CODEC 0

#define OGRE_NO_ASTC_CODEC 0

/* DV edit: Ogre does not read or write files by itself in this code base. The archive factories for the
   file system and ZIP files are not registered by Root, Root has no default plugin, configuration and log
   file names and the default log has no file. Applications read through the GucefVFS archive factories.
   The Ogre modules also get this define from their ModuleInfo.xml. */
#ifndef GUCEF_OGRE_NO_NATIVE_ARCHIVES
#   define GUCEF_OGRE_NO_NATIVE_ARCHIVES
#endif

/** Disables use of the ZIP archive support. */
#define OGRE_NO_ZIP_ARCHIVE 0

#define OGRE_NO_TBB_SCHEDULER 0

/** If set to 1, profiling code will be included in the application. When you
    are deploying your application you will probably want to set this to 0 */
#define OGRE_PROFILING 0

#define OGRE_NO_QUAD_BUFFER_STEREO 1

/* strtol_l is not available with the Microsoft C runtime and with Emscripten */
#if defined(_WIN32) || defined(__EMSCRIPTEN__)
#   define OGRE_NO_LOCALE_STRCONVERT 1
#else
#   define OGRE_NO_LOCALE_STRCONVERT 0
#endif

/** If set to 1, all file system paths are expected to be UTF-8 encoded and
    the wide character (wchar_t) file system routines are used on Windows.
*/
#if defined(_WIN32) && !defined(__ANDROID__)
#   define OGRE_CONFIG_FILESYSTEM_UNICODE 1
#else
#   define OGRE_CONFIG_FILESYSTEM_UNICODE 0
#endif

#endif
