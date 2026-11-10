//
// Define and set to 1 if the target system supports a proc filesystem
// compatible with the Linux kernel's proc filesystem.  Note that this
// is only used by a program in the IlmImfTest test suite, it's not
// used by any OpenEXR library or application code.
//

#undef OPENEXR_IMF_HAVE_LINUX_PROCFS

//
// Define and set to 1 if the target system is a Darwin-based system
// (e.g., OS X).
//

/* #undef OPENEXR_IMF_HAVE_DARWIN */

//
// Define and set to 1 if the target system has a complete <iomanip>
// implementation, specifically if it supports the std::right
// formatter.
//

#define OPENEXR_IMF_HAVE_COMPLETE_IOMANIP 1

//
// Define and set to 1 if the target system has support for large
// stack sizes.
//

#define OPENEXR_IMF_HAVE_LARGE_STACK 1

//
// Define if we can support GCC style inline asm with AVX instructions
//

#undef OPENEXR_IMF_HAVE_GCC_INLINE_ASM_AVX

//
// Define if we can use sysconf(_SC_NPROCESSORS_ONLN) to get CPU count
//

#if !defined(_MSC_VER) && !defined(__MINGW32__)
// DV edit: POSIX systems can tell the number of processors
#define OPENEXR_IMF_HAVE_SYSCONF_NPROCESSORS_ONLN 1
#else
#undef OPENEXR_IMF_HAVE_SYSCONF_NPROCESSORS_ONLN
#endif

//
// Current internal library namepace name
//
#define OPENEXR_IMF_INTERNAL_NAMESPACE_CUSTOM 1
#define OPENEXR_IMF_INTERNAL_NAMESPACE Imf_2_2

//
// Current public user namepace name
//

/* #undef OPENEXR_IMF_NAMESPACE_CUSTOM */
#define OPENEXR_IMF_NAMESPACE Imf

//
// Version string for runtime access
//

#define OPENEXR_VERSION_STRING "2.2.0"
#define OPENEXR_PACKAGE_STRING "OpenEXR 2.2.0"

#define OPENEXR_VERSION_MAJOR 2
#define OPENEXR_VERSION_MINOR 2
#define OPENEXR_VERSION_PATCH 0

// Version as a single hex number, e.g. 0x01000300 == 1.0.3
#define OPENEXR_VERSION_HEX ((OPENEXR_VERSION_MAJOR << 24) | \
                             (OPENEXR_VERSION_MINOR << 16) | \
                             (OPENEXR_VERSION_PATCH <<  8))

//
// This is a hard-coded config file for Windows platforms.  Don't
// change any of these settings.
//

//
// Define and set to 1 if the target system has POSIX thread support
// and you want OpenEXR to use it for multithreaded file I/O.
//

/* #undef HAVE_PTHREAD */

//
// Define and set to 1 if the target system supports POSIX semaphores
// and you want OpenEXR to use them; otherwise, OpenEXR will use its
// own semaphore implementation.
//

/* #undef HAVE_POSIX_SEMAPHORES */

//
// Define and set to 1 if the target system is a Darwin-based system
// (e.g., OS X).
//

/* #undef HAVE_DARWIN */

//
// Define and set to 1 if the target system supports a proc filesystem
// compatible with the Linux kernel's proc filesystem.  Note that this
// is only used by a program in the IlmImfTest test suite, it's not
// used by any OpenEXR library or application code.
//

/* #undef HAVE_LINUX_PROCFS */

//
// Define and set to 1 if the target system has a complete <iomanip>
// implementation, specifically if it supports the std::right
// formatter.
//

#define HAVE_COMPLETE_IOMANIP 1
