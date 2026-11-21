// stdafx.h : includefile for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
// DV edit: the original content of this header is used on Windows (the part before #else); the part
// after #else gives the other platforms the types and the MSVC intrinsics the decoder uses.
// The platform and CPU are detected with the macros of the GUCEF platform header (header only use of
// common/include, nothing of GUCEF is linked). The decoder uses SSE2 and is for x86 and x86-64 CPUs only.
#include "gucef_platform.h"

#if !defined( GUCEF_CPU_ARCHITECTURE ) || ( ( GUCEF_CPU_ARCHITECTURE != GUCEF_CPU_ARCHITECTURE_X86 ) && ( GUCEF_CPU_ARCHITECTURE != GUCEF_CPU_ARCHITECTURE_AMD64 ) )
#error ooz_extract: the decoder uses SSE2 intrinsics and is only available for x86 and x86-64 CPUs
#endif

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )


#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>
#include <assert.h>
#include <Windows.h>

#pragma warning (disable: 4244)

// TODO: reference additional headers your program requires here
typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned __int64 uint64;
typedef signed __int64 int64;
typedef signed int int32;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint;

#else /* DV edit: not Windows */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <emmintrin.h>
#include <xmmintrin.h>

// DV edit: the byte swaps come from the GUCEF endian header (header only, its functions are inline)
#include "gucef_endian.h"

#define __forceinline inline __attribute__((always_inline))

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef signed long long int64;
typedef signed int int32;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint;

// DV edit: the MSVC intrinsics used by the decoder that GUCEF does not offer, in terms of the GCC/Clang
// builtins (the bit scans store 0 for an input of 0, which the MSVC intrinsics leave undefined)
static __forceinline unsigned char _BitScanReverse(unsigned long *index, uint32 mask) {
  if (mask == 0) { *index = 0; return 0; }
  *index = (unsigned long)(31 - __builtin_clz(mask));
  return 1;
}
static __forceinline unsigned char _BitScanForward(unsigned long *index, uint32 mask) {
  if (mask == 0) { *index = 0; return 0; }
  *index = (unsigned long)__builtin_ctz(mask);
  return 1;
}
static __forceinline uint32 _rotl(uint32 value, int shift) {
  shift &= 31;
  return shift == 0 ? value : (value << shift) | (value >> (32 - shift));
}
static __forceinline uint16 _byteswap_ushort(uint16 value) { return GUCEF::SwapEndianUInt16(value); }
static __forceinline uint32 _byteswap_ulong(uint32 value) { return GUCEF::SwapEndianUInt32(value); }
static __forceinline uint64 _byteswap_uint64(uint64 value) { return GUCEF::SwapEndianUInt64(value); }

#endif
