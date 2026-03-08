/*
 *  gucefMT: GUCEF module providing multithreading solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GUCEF_MLF_ETYPES_H
#define GUCEF_MLF_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Maximum and minimal values for the simple types which we are about to
 *      define.
 */
#define GUCEFMLF_INT8MAX     127
#define GUCEFMLF_INT8MIN     (-128)
#define GUCEFMLF_UINT8MAX    256
#define GUCEFMLF_UINT8MIN    0
#define GUCEFMLF_INT16MAX    32767
#define GUCEFMLF_INT16MIN    (-32767-1)
#define GUCEFMLF_UINT16MAX   65535
#define GUCEFMLF_UINT16MIN   0
#define GUCEFMLF_INT32MAX    2147483647
#define GUCEFMLF_INT32MIN    (-2147483647-1)
#define GUCEFMLF_UINT32MAX   4294967295
#define GUCEFMLF_UINT32MIN   0
#define GUCEFMLF_FLOAT32MAX  (3.40282347E+38F)
#define GUCEFMLF_FLOAT32MIN  (-3.40282347E+38F-1)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace MLF {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

#if GUCEF_COMPILER == GUCEF_COMPILER_MSVC

#ifndef Byte
typedef signed __int8       Byte;     /* 1 byte, signed */
#endif

typedef unsigned __int8     Ubyte;    /* 1 byte, unsigned */

#undef UByte
#define UByte               Ubyte

typedef unsigned __int8     UInt8;    /* 1 byte, unsigned */
typedef signed __int8       Int8;     /* 1 byte, signed */

typedef unsigned __int16    UInt16;   /* 2 bytes, unsigned */
typedef signed __int16      Int16;    /* 2 bytes, signed */

typedef unsigned __int32    UInt32;   /* 4 bytes, unsigned */
typedef signed __int32      Int32;    /* 4 bytes, signed */

typedef signed __int64      Int64;   /* 8 bytes, signed */
typedef unsigned __int64    UInt64;  /* 8 bytes, unsigned */

typedef float               Float32;  /* 4 bytes, signed, decimal */
typedef double              Float64;  /* 8 bytes, signed, decimal */

#ifdef _WIN64
typedef unsigned __int64    UIntPtr;  /* pointer-sized unsigned int */
typedef signed __int64      IntPtr;   /* pointer-sized signed int */
#else
typedef unsigned __int32    UIntPtr;  /* pointer-sized unsigned int */
typedef signed __int32      IntPtr;   /* pointer-sized signed int */
#endif

#else
                       
#ifndef Byte
typedef signed char     Byte;     /* 1 byte, signed */
#endif

typedef unsigned char   Ubyte;    /* 1 byte, unsigned */

#undef UByte
#define UByte           Ubyte

typedef unsigned char   UInt8;    /* 1 byte, unsigned */
typedef signed char     Int8;     /* 1 byte, signed */

typedef unsigned short  UInt16;   /* 2 bytes, unsigned */
typedef signed short    Int16;    /* 2 bytes, signed */

#if defined(__alpha) || (defined(_MIPS_SZLONG) && _MIPS_SZLONG == 64) || defined(__LP64__)
typedef unsigned int    UInt32;   /* 4 bytes, unsigned */
typedef signed int      Int32;    /* 4 bytes, signed */
#else
typedef unsigned long   UInt32;   /* 4 bytes, unsigned */
typedef signed long     Int32;    /* 4 bytes, signed */
#endif

#ifdef __GNUC__
typedef signed long long   Int64;   /* 8 bytes, signed */
typedef unsigned long long UInt64;  /* 8 bytes, unsigned */
#else
typedef signed __int64   Int64;   /* 8 bytes, signed */
typedef unsigned __int64 UInt64;  /* 8 bytes, unsigned */
#endif

typedef float           Float32;  /* 4 bytes, signed, decimal */
typedef double          Float64;  /* 8 bytes, signed, decimal */

#include <stdint.h>
typedef uintptr_t       UIntPtr;  /* pointer-sized unsigned int */
typedef intptr_t        IntPtr;   /* pointer-sized signed int */

#endif

/*--------------------------------------------------------------------------*/

/**
 *      Build-in data types as part of an enumeration set
 */
enum EBuildinDataType
{
        DATATYPE_FLOAT32 = 1 ,
        DATATYPE_FLOAT64     ,
        DATATYPE_UINT8       ,
        DATATYPE_INT8        ,
        DATATYPE_UINT16      ,
        DATATYPE_INT16       ,
        DATATYPE_UINT32      ,
        DATATYPE_INT32       ,
        DATATYPE_UINT64      ,
        DATATYPE_INT64       ,

        DATATYPE_UNKNOWN
};
typedef enum EBuildinDataType TBuildinDataType;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace MLF */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MLF_ETYPES_H ? */





