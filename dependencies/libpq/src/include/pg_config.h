/*
 *  pg_config.h: DV added, a hand written equivalent of the header the configure step of PostgreSQL 18.7 generates from
 *  src/include/pg_config.h.in, for building the client library libpq in GUCEF.
 *
 *  Upstream generates this header with configure (autoconf) or meson for the machine it runs on. Here the
 *  preprocessor selects the values per platform and compiler instead, for what libpq and the parts of src/common and
 *  src/port it uses need. Server only settings keep the upstream defaults.
 *
 *  Selections:
 *  - Linux (glibc, musl) and Android, macOS and iOS, Windows with Microsoft Visual C++ or MinGW
 *  - TLS: USE_OPENSSL on Linux (the OpenSSL of the system, the module links libssl and libcrypto there), no TLS on
 *    the other platforms (the source lists of the module follow the same selection)
 *  - No GSSAPI, no LDAP, no NLS, no libcurl (the built in OAuth device flow is not available)
 *  - The string functions that not every C library has (strlcat, strlcpy, timingsafe_bcmp, getpeereid) are always the
 *    replacements of src/port, so the result does not depend on the version of the C library
 */

#ifndef GUCEF_LIBPQ_PG_CONFIG_H
#define GUCEF_LIBPQ_PG_CONFIG_H

/*-------------------------------------------------------------------------*/
/*  Platform selection                                                     */
/*-------------------------------------------------------------------------*/

#if defined( _WIN32 )
  #define GUCEF_LIBPQ_PLATFORM_WINDOWS 1
  #if defined( _MSC_VER ) && !defined( __clang__ )
    #define GUCEF_LIBPQ_COMPILER_MSVC 1
  #endif
#elif defined( __APPLE__ )
  #define GUCEF_LIBPQ_PLATFORM_APPLE 1
#elif defined( __ANDROID__ )
  #define GUCEF_LIBPQ_PLATFORM_ANDROID 1
  #define GUCEF_LIBPQ_PLATFORM_LINUX 1
#elif defined( __linux__ )
  #define GUCEF_LIBPQ_PLATFORM_LINUX 1
#else
  #error "pg_config.h of libpq: this platform has no configuration yet"
#endif

#if defined( _WIN64 ) || defined( __LP64__ ) || defined( _LP64 ) || ( defined( __SIZEOF_POINTER__ ) && 8 == __SIZEOF_POINTER__ )
  #define GUCEF_LIBPQ_64BIT 1
#endif

/*-------------------------------------------------------------------------*/
/*  Version and package                                                    */
/*-------------------------------------------------------------------------*/

#define PACKAGE_BUGREPORT "pgsql-bugs@lists.postgresql.org"
#define PACKAGE_NAME "PostgreSQL"
#define PACKAGE_STRING "PostgreSQL 18.7"
#define PACKAGE_TARNAME "postgresql"
#define PACKAGE_URL "https://www.postgresql.org/"
#define PACKAGE_VERSION "18.7"
#define PG_MAJORVERSION "18"
#define PG_MAJORVERSION_NUM 18
#define PG_MINORVERSION_NUM 7
#define PG_VERSION "18.7"
#define PG_VERSION_NUM 180007
#define PG_VERSION_STR "PostgreSQL 18.7 (GUCEF libpq)"
#define CONFIGURE_ARGS ""

/*-------------------------------------------------------------------------*/
/*  Server settings, upstream defaults                                     */
/*-------------------------------------------------------------------------*/

#define BLCKSZ 8192
#define XLOG_BLCKSZ 8192
#define RELSEG_SIZE 131072
#define DEF_PGPORT 5432
#define DEF_PGPORT_STR "5432"
#define PG_KRB_SRVNAM "postgres"
#define MEMSET_LOOP_LIMIT 1024

/*-------------------------------------------------------------------------*/
/*  Sizes and alignment                                                    */
/*-------------------------------------------------------------------------*/

#define ALIGNOF_SHORT 2
#define ALIGNOF_INT 4

#if defined( GUCEF_LIBPQ_PLATFORM_WINDOWS )
  #define SIZEOF_LONG 4
  #define ALIGNOF_LONG 4
  #define SIZEOF_LONG_LONG 8
  #define SIZEOF_OFF_T 4
  #if defined( _WIN64 )
    #define SIZEOF_SIZE_T 8
    #define SIZEOF_VOID_P 8
  #else
    #define SIZEOF_SIZE_T 4
    #define SIZEOF_VOID_P 4
  #endif
  #define ALIGNOF_DOUBLE 8
  #define ALIGNOF_INT64_T 8
  #define MAXIMUM_ALIGNOF 8
#else
  #if defined( GUCEF_LIBPQ_64BIT )
    #define SIZEOF_LONG 8
    #define ALIGNOF_LONG 8
    #define SIZEOF_SIZE_T 8
    #define SIZEOF_VOID_P 8
  #else
    #define SIZEOF_LONG 4
    #define ALIGNOF_LONG 4
    #define SIZEOF_SIZE_T 4
    #define SIZEOF_VOID_P 4
  #endif
  #define SIZEOF_LONG_LONG 8
  #define SIZEOF_OFF_T 8
  /* 32 bit x86 aligns double and 64 bit integers inside structures at 4 bytes */
  #if defined( __i386__ )
    #define ALIGNOF_DOUBLE 4
    #define ALIGNOF_INT64_T 4
    #define MAXIMUM_ALIGNOF 4
  #else
    #define ALIGNOF_DOUBLE 8
    #define ALIGNOF_INT64_T 8
    #define MAXIMUM_ALIGNOF 8
  #endif
  /* large file support on 32 bit platforms (set before any system header) */
  #if !defined( GUCEF_LIBPQ_64BIT ) && !defined( GUCEF_LIBPQ_PLATFORM_APPLE ) && !defined( _FILE_OFFSET_BITS )
    #define _FILE_OFFSET_BITS 64
  #endif
#endif

/*-------------------------------------------------------------------------*/
/*  Compiler                                                               */
/*-------------------------------------------------------------------------*/

#if defined( __GNUC__ ) || defined( __clang__ )
  #define HAVE__BUILTIN_BSWAP16 1
  #define HAVE__BUILTIN_BSWAP32 1
  #define HAVE__BUILTIN_BSWAP64 1
  #define HAVE__BUILTIN_CLZ 1
  #define HAVE__BUILTIN_CTZ 1
  #define HAVE__BUILTIN_POPCOUNT 1
  #define HAVE__BUILTIN_CONSTANT_P 1
  #define HAVE__BUILTIN_FRAME_ADDRESS 1
  #define HAVE__BUILTIN_OP_OVERFLOW 1
  #define HAVE__BUILTIN_TYPES_COMPATIBLE_P 1
  #define HAVE__BUILTIN_UNREACHABLE 1
  #define HAVE_COMPUTED_GOTO 1
  #define HAVE_TYPEOF 1
  #define HAVE__STATIC_ASSERT 1
  #define HAVE_VISIBILITY_ATTRIBUTE 1
  #define HAVE_GCC__ATOMIC_INT32_CAS 1
  #define HAVE_GCC__ATOMIC_INT64_CAS 1
  #define HAVE_GCC__SYNC_CHAR_TAS 1
  #define HAVE_GCC__SYNC_INT32_CAS 1
  #define HAVE_GCC__SYNC_INT32_TAS 1
  #define HAVE_GCC__SYNC_INT64_CAS 1
  #if defined( __clang__ )
    #define PG_C_PRINTF_ATTRIBUTE printf
    #define PG_CXX_PRINTF_ATTRIBUTE printf
  #else
    #define PG_C_PRINTF_ATTRIBUTE gnu_printf
    #define PG_CXX_PRINTF_ATTRIBUTE gnu_printf
  #endif
  #define pg_restrict __restrict
  #if defined( __SIZEOF_INT128__ )
    #define PG_INT128_TYPE __int128
    #define ALIGNOF_PG_INT128_TYPE 16
  #endif
  #if defined( __x86_64__ ) || defined( __i386__ )
    #define HAVE__GET_CPUID 1
    #define HAVE__GET_CPUID_COUNT 1
  #endif
#elif defined( GUCEF_LIBPQ_COMPILER_MSVC )
  #define pg_restrict __restrict
  #define HAVE__CPUID 1
  #define HAVE__CPUIDEX 1
  #define HAVE__STATIC_ASSERT 1
#endif

/*-------------------------------------------------------------------------*/
/*  C library and operating system                                         */
/*-------------------------------------------------------------------------*/

#define STDC_HEADERS 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_MEMORY_H 1
#define HAVE_LIBM 1

/* replacements of src/port are used on every platform for these, see the top of the file */
#define HAVE_DECL_STRLCAT 0
#define HAVE_DECL_STRLCPY 0
#define HAVE_DECL_TIMINGSAFE_BCMP 0

#if defined( GUCEF_LIBPQ_PLATFORM_WINDOWS )

  #define DLSUFFIX ".dll"
  #define HAVE_DECL_FDATASYNC 0
  #define HAVE_DECL_F_FULLFSYNC 0
  #define HAVE_DECL_MEMSET_S 0
  #define HAVE_DECL_POSIX_FADVISE 0
  #define HAVE_DECL_PREADV 0
  #define HAVE_DECL_PWRITEV 0
  #define HAVE_DECL_STRCHRNUL 0
  #define HAVE_DECL_STRNLEN 1
  #define HAVE_DECL_STRSEP 0
  #define HAVE_STRNLEN 1
  #define HAVE_INET_PTON 1
  #define HAVE_SOCKLEN_T 1
  #if defined( GUCEF_LIBPQ_COMPILER_MSVC )
    #define HAVE_CRTDEFS_H 1
  #else
    #define HAVE_GETOPT 1
    #define HAVE_GETOPT_H 1
    #define HAVE_GETOPT_LONG 1
    #define HAVE_INT_OPTERR 1
    #define HAVE_STRUCT_OPTION 1
    #define HAVE_STRINGS_H 1
    #define HAVE_UNISTD_H 1
  #endif

#else /* POSIX */

  #define HAVE_PTHREAD 1
  #define HAVE_STRINGS_H 1
  #define HAVE_UNISTD_H 1
  #define HAVE_TERMIOS_H 1
  #define HAVE_IFADDRS_H 1
  #define HAVE_GETIFADDRS 1
  #define HAVE_INET_ATON 1
  #define HAVE_INET_PTON 1
  #define HAVE_MKDTEMP 1
  #define HAVE_STRNLEN 1
  #define HAVE_DECL_STRNLEN 1
  #define HAVE_STRSEP 1
  #define HAVE_DECL_STRSEP 1
  #define HAVE_STRERROR_R 1
  #define HAVE_STRSIGNAL 1
  #define HAVE_SOCKLEN_T 1
  #define HAVE_GETOPT 1
  #define HAVE_GETOPT_H 1
  #define HAVE_GETOPT_LONG 1
  #define HAVE_INT_OPTERR 1
  #define HAVE_STRUCT_OPTION 1
  #define HAVE_STRUCT_TM_TM_ZONE 1
  #define HAVE_USELOCALE 1
  #define HAVE_SYSLOG 1
  #define HAVE_FSEEKO 1
  #define HAVE_EXECINFO_H 1
  #define HAVE_BACKTRACE_SYMBOLS 1
  #define HAVE_SYS_PRCTL_H 1

  #if defined( GUCEF_LIBPQ_PLATFORM_APPLE )
    #define DLSUFFIX ".dylib"
    #undef HAVE_SYS_PRCTL_H
    #define HAVE_GETPEEREID 1
    #define HAVE_SYS_UCRED_H 1
    #define HAVE_STRUCT_SOCKADDR_SA_LEN 1
    #define HAVE_XLOCALE_H 1
    #define HAVE_LOCALECONV_L 1
    #define HAVE_MBSTOWCS_L 1
    #define HAVE_WCSTOMBS_L 1
    #define HAVE_INT_OPTRESET 1
    #define HAVE_DECL_F_FULLFSYNC 1
    #define HAVE_DECL_FDATASYNC 0
    #define HAVE_DECL_MEMSET_S 1
    #define HAVE_DECL_POSIX_FADVISE 0
    #define HAVE_DECL_PREADV 1
    #define HAVE_DECL_PWRITEV 1
    #define HAVE_DECL_STRCHRNUL 0
    #define HAVE_KQUEUE 1
    #define HAVE_SYS_EVENT_H 1
    #define HAVE_COPYFILE 1
    #define HAVE_COPYFILE_H 1
    #define STRERROR_R_INT 1
  #else /* Linux and Android */
    #define DLSUFFIX ".so"
    #define HAVE_DECL_FDATASYNC 1
    #define HAVE_DECL_F_FULLFSYNC 0
    #define HAVE_DECL_MEMSET_S 0
    #define HAVE_DECL_POSIX_FADVISE 1
    #define HAVE_DECL_PREADV 1
    #define HAVE_DECL_PWRITEV 1
    #define HAVE_DECL_STRCHRNUL 1
    #define HAVE_EXPLICIT_BZERO 1
    #define HAVE_POSIX_FADVISE 1
    #define HAVE_POSIX_FALLOCATE 1
    #define HAVE_PPOLL 1
    #define HAVE_SYNC_FILE_RANGE 1
    #define HAVE_SYS_EPOLL_H 1
    #define HAVE_SYS_SIGNALFD_H 1
    #define HAVE_SYS_PERSONALITY_H 1
    #define HAVE_GETAUXVAL 1
    #define HAVE_COPY_FILE_RANGE 1
    #define HAVE_PTHREAD_BARRIER_WAIT 1
    #define HAVE_INT_TIMEZONE 1
    #if defined( GUCEF_LIBPQ_PLATFORM_ANDROID )
      /* bionic: XSI strerror_r, no execinfo */
      #define STRERROR_R_INT 1
      #undef HAVE_EXECINFO_H
      #undef HAVE_BACKTRACE_SYMBOLS
    #elif defined( __GLIBC__ ) || !defined( __has_include )
      /* glibc: the GNU strerror_r that returns a char*, STRERROR_R_INT stays undefined */
    #elif __has_include( <gnu/libc-version.h> )
      /* glibc whose features.h was not included yet */
    #else
      /* musl: XSI strerror_r, no execinfo */
      #define STRERROR_R_INT 1
      #undef HAVE_EXECINFO_H
      #undef HAVE_BACKTRACE_SYMBOLS
    #endif
  #endif

#endif

/*-------------------------------------------------------------------------*/
/*  Features                                                               */
/*-------------------------------------------------------------------------*/

#if defined( GUCEF_LIBPQ_PLATFORM_LINUX ) && !defined( GUCEF_LIBPQ_PLATFORM_ANDROID )
  #define USE_OPENSSL 1
  #define HAVE_LIBSSL 1
  #define HAVE_LIBCRYPTO 1
  #define OPENSSL_API_COMPAT 0x10101000L
  #define HAVE_SSL_CTX_SET_CERT_CB 1
  #define HAVE_SSL_CTX_SET_CIPHERSUITES 1
  #define HAVE_SSL_CTX_SET_KEYLOG_CALLBACK 1
  #define HAVE_SSL_CTX_SET_NUM_TICKETS 1
  #define HAVE_X509_GET_SIGNATURE_INFO 1
#endif

/* CRC-32C, not used by libpq: the portable implementation */
#define USE_SLICING_BY_8_CRC32C 1

#endif /* GUCEF_LIBPQ_PG_CONFIG_H ? */
