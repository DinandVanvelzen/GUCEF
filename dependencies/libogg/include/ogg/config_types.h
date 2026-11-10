#ifndef __CONFIG_TYPES_H__
#define __CONFIG_TYPES_H__

/*
 * DV edit: GUCEF builds libogg with the project files made by ProjectGenerator, without running
 * configure or CMake which would normally generate this file from config_types.h.in. The values
 * are those of a platform with C99 <stdint.h> (Linux, Android, Emscripten). os_types.h defines
 * the types itself for Windows and Apple platforms and does not include this file for those.
 */
#define INCLUDE_INTTYPES_H 1
#define INCLUDE_STDINT_H 1
#define INCLUDE_SYS_TYPES_H 1

#if INCLUDE_INTTYPES_H
#  include <inttypes.h>
#endif
#if INCLUDE_STDINT_H
#  include <stdint.h>
#endif
#if INCLUDE_SYS_TYPES_H
#  include <sys/types.h>
#endif

typedef int16_t ogg_int16_t;
typedef uint16_t ogg_uint16_t;
typedef int32_t ogg_int32_t;
typedef uint32_t ogg_uint32_t;
typedef int64_t ogg_int64_t;
typedef uint64_t ogg_uint64_t;

#endif
