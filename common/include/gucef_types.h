/*
 *  gucef common header: provides header based platform wide facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_TYPES_H
#define GUCEF_TYPES_H

/*
 *      calling convention macros for the GUCEF platform
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"        /* GUCEF configuration */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"      /* GUCEF platform compilation targets */
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
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

#else

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

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
  #define GUCEF_NS_FOR_BUILDINS ::GUCEF::
#else
  #define GUCEF_NS_FOR_BUILDINS
#endif
/*
 *      Maximum and minimal values for the build-in types 
 */
#define GUCEF_INT8MAX     ( (GUCEF_NS_FOR_BUILDINS Int8) 127 )
#define GUCEF_INT8MIN     ( (GUCEF_NS_FOR_BUILDINS Int8) (-128) )
#define GUCEF_UINT8MAX    ( (GUCEF_NS_FOR_BUILDINS UInt8) 256 )
#define GUCEF_UINT8MIN    ( (GUCEF_NS_FOR_BUILDINS UInt8) 0 )
#define GUCEF_INT16MAX    ( (GUCEF_NS_FOR_BUILDINS Int16) 32767 )
#define GUCEF_INT16MIN    ( (GUCEF_NS_FOR_BUILDINS Int16) (-32767-1) )
#define GUCEF_UINT16MAX   ( (GUCEF_NS_FOR_BUILDINS UInt16) 65535 )
#define GUCEF_UINT16MIN   ( (GUCEF_NS_FOR_BUILDINS UInt16) 0 )
#define GUCEF_INT32MAX    ( (GUCEF_NS_FOR_BUILDINS Int32) 2147483647 )
#define GUCEF_INT32MIN    ( (GUCEF_NS_FOR_BUILDINS Int32) (-2147483647-1) )
#define GUCEF_UINT32MAX   ( (GUCEF_NS_FOR_BUILDINS UInt32) 4294967295 )
#define GUCEF_UINT32MIN   ( (GUCEF_NS_FOR_BUILDINS UInt32) 0 )
#define GUCEF_INT64MIN    ( (GUCEF_NS_FOR_BUILDINS Int64) -9223372036854775808 )
#define GUCEF_INT64MAX    ( (GUCEF_NS_FOR_BUILDINS Int64) 9223372036854775807 )
#define GUCEF_UINT64MIN   ( (GUCEF_NS_FOR_BUILDINS UInt64) 0 )
#define GUCEF_UINT64MAX   ( (GUCEF_NS_FOR_BUILDINS UInt64) 18446744073709551615 )
#define GUCEF_FLOAT32MAX  ( (GUCEF_NS_FOR_BUILDINS Float32) (3.40282347E+38F) )
#define GUCEF_FLOAT32MIN  ( (GUCEF_NS_FOR_BUILDINS Float32) (-3.40282347E+38F-1) )

/*--------------------------------------------------------------------------*/

#define GUCEF_DATATYPE_UNKNOWN          0
#define GUCEF_DATATYPE_NAME_UNKNOWN     "UNKNOWN"


/*
 *  The following are C/C++ build-in data types
 *  Enumerated here for dynamic type referencing at runtime
 */
#define GUCEF_DATATYPE_UINT8           1
#define GUCEF_DATATYPE_INT8            2
#define GUCEF_DATATYPE_LE_UINT16       3
#define GUCEF_DATATYPE_BE_UINT16       4
#define GUCEF_DATATYPE_LE_INT16        5
#define GUCEF_DATATYPE_BE_INT16        6
#define GUCEF_DATATYPE_LE_UINT32       7
#define GUCEF_DATATYPE_BE_UINT32       8
#define GUCEF_DATATYPE_LE_INT32        9
#define GUCEF_DATATYPE_BE_INT32        10
#define GUCEF_DATATYPE_LE_UINT64       11
#define GUCEF_DATATYPE_BE_UINT64       12
#define GUCEF_DATATYPE_LE_INT64        13
#define GUCEF_DATATYPE_BE_INT64        14
#define GUCEF_DATATYPE_LE_FLOAT32      15
#define GUCEF_DATATYPE_BE_FLOAT32      16
#define GUCEF_DATATYPE_LE_FLOAT64      17
#define GUCEF_DATATYPE_BE_FLOAT64      18

#define GUCEF_DATATYPE_NAME_UINT8           "UINT8"
#define GUCEF_DATATYPE_NAME_INT8            "INT8"
#define GUCEF_DATATYPE_NAME_LE_UINT16       "LE_UINT16"
#define GUCEF_DATATYPE_NAME_BE_UINT16       "BE_UINT16"
#define GUCEF_DATATYPE_NAME_LE_INT16        "LE_INT16"
#define GUCEF_DATATYPE_NAME_BE_INT16        "BE_INT16"
#define GUCEF_DATATYPE_NAME_LE_UINT32       "LE_UINT32"
#define GUCEF_DATATYPE_NAME_BE_UINT32       "BE_UINT32"
#define GUCEF_DATATYPE_NAME_LE_INT32        "LE_INT32"
#define GUCEF_DATATYPE_NAME_BE_INT32        "BE_INT32"
#define GUCEF_DATATYPE_NAME_LE_UINT64       "LE_UINT64"
#define GUCEF_DATATYPE_NAME_BE_UINT64       "BE_UINT64"
#define GUCEF_DATATYPE_NAME_LE_INT64        "LE_INT64"
#define GUCEF_DATATYPE_NAME_BE_INT64        "BE_INT64"
#define GUCEF_DATATYPE_NAME_LE_FLOAT32      "LE_FLOAT32"
#define GUCEF_DATATYPE_NAME_BE_FLOAT32      "BE_FLOAT32"
#define GUCEF_DATATYPE_NAME_LE_FLOAT64      "LE_FLOAT64"
#define GUCEF_DATATYPE_NAME_BE_FLOAT64      "BE_FLOAT64"

#if GUCEF_PLATFORM_BYTEORDER_ENDIAN == GUCEF_BYTEORDER_LITTLE_ENDIAN

    #define GUCEF_DATATYPE_UINT16       GUCEF_DATATYPE_LE_UINT16    
    #define GUCEF_DATATYPE_INT16        GUCEF_DATATYPE_LE_INT16
    #define GUCEF_DATATYPE_UINT32       GUCEF_DATATYPE_LE_UINT32
    #define GUCEF_DATATYPE_INT32        GUCEF_DATATYPE_LE_INT32
    #define GUCEF_DATATYPE_UINT64       GUCEF_DATATYPE_LE_UINT64
    #define GUCEF_DATATYPE_INT64        GUCEF_DATATYPE_LE_INT64
    #define GUCEF_DATATYPE_FLOAT32      GUCEF_DATATYPE_LE_FLOAT32
    #define GUCEF_DATATYPE_FLOAT64      GUCEF_DATATYPE_LE_FLOAT64

    #define GUCEF_DATATYPE_NAME_UINT16       GUCEF_DATATYPE_NAME_LE_UINT16    
    #define GUCEF_DATATYPE_NAME_INT16        GUCEF_DATATYPE_NAME_LE_INT16
    #define GUCEF_DATATYPE_NAME_UINT32       GUCEF_DATATYPE_NAME_LE_UINT32
    #define GUCEF_DATATYPE_NAME_INT32        GUCEF_DATATYPE_NAME_LE_INT32
    #define GUCEF_DATATYPE_NAME_UINT64       GUCEF_DATATYPE_NAME_LE_UINT64
    #define GUCEF_DATATYPE_NAME_INT64        GUCEF_DATATYPE_NAME_LE_INT64
    #define GUCEF_DATATYPE_NAME_FLOAT32      GUCEF_DATATYPE_NAME_LE_FLOAT32
    #define GUCEF_DATATYPE_NAME_FLOAT64      GUCEF_DATATYPE_NAME_LE_FLOAT64

#else

    #define GUCEF_DATATYPE_UINT16       GUCEF_DATATYPE_BE_UINT16    
    #define GUCEF_DATATYPE_INT16        GUCEF_DATATYPE_BE_INT16
    #define GUCEF_DATATYPE_UINT32       GUCEF_DATATYPE_BE_UINT32
    #define GUCEF_DATATYPE_INT32        GUCEF_DATATYPE_BE_INT32
    #define GUCEF_DATATYPE_UINT64       GUCEF_DATATYPE_BE_UINT64
    #define GUCEF_DATATYPE_INT64        GUCEF_DATATYPE_BE_INT64
    #define GUCEF_DATATYPE_FLOAT32      GUCEF_DATATYPE_BE_FLOAT32
    #define GUCEF_DATATYPE_FLOAT64      GUCEF_DATATYPE_BE_FLOAT64

    #define GUCEF_DATATYPE_NAME_UINT16       GUCEF_DATATYPE_NAME_BE_UINT16    
    #define GUCEF_DATATYPE_NAME_INT16        GUCEF_DATATYPE_NAME_BE_INT16
    #define GUCEF_DATATYPE_NAME_UINT32       GUCEF_DATATYPE_NAME_BE_UINT32
    #define GUCEF_DATATYPE_NAME_INT32        GUCEF_DATATYPE_NAME_BE_INT32
    #define GUCEF_DATATYPE_NAME_UINT64       GUCEF_DATATYPE_NAME_BE_UINT64
    #define GUCEF_DATATYPE_NAME_INT64        GUCEF_DATATYPE_NAME_BE_INT64
    #define GUCEF_DATATYPE_NAME_FLOAT32      GUCEF_DATATYPE_NAME_BE_FLOAT32
    #define GUCEF_DATATYPE_NAME_FLOAT64      GUCEF_DATATYPE_NAME_BE_FLOAT64

#endif

/*
 *  The following are higher level data types non-build-in or non-primative
 *  Enumerated here for dynamic type referencing at runtime
 */
#define GUCEF_DATATYPE_NUMERIC                  20      /**< number of undefined range. Suggested to always interpret using the largest range signed storage available */
#define GUCEF_DATATYPE_ASCII_STRING             21      /**< string utilizing the ASCII character set exclusively */
#define GUCEF_DATATYPE_UTF8_STRING              22      /**< string utilizing the UTF8 character set exclusively */
#define GUCEF_DATATYPE_UTF16_LE_STRING          23      /**< string utilizing the UTF16 character set exclusively. Little Endian variant of UTF16 */
#define GUCEF_DATATYPE_UTF16_BE_STRING          24      /**< string utilizing the UTF16 character set exclusively. Big Endian variant of UTF16 */
#define GUCEF_DATATYPE_UTF32_LE_STRING          25      /**< string utilizing the UTF32 character set exclusively. Little Endian variant of UTF32 */
#define GUCEF_DATATYPE_UTF32_BE_STRING          26      /**< string utilizing the UTF32 character set exclusively. Big Endian variant of UTF32 */

#define GUCEF_DATATYPE_BOOLEAN_INT32            30      /**< integer value actually representing a boolean (zero or non-zero ) */
#define GUCEF_DATATYPE_BOOLEAN_ASCII_STRING     31      /**< ASCII character set string value actually representing a boolean using reserved words or digits */
#define GUCEF_DATATYPE_BOOLEAN_UTF8_STRING      32      /**< UTF8 character set string value actually representing a boolean using reserved words or digits */
#define GUCEF_DATATYPE_BOOLEAN_UTF16_LE_STRING  33      /**< UTF16 Little Endian character set string value actually representing a boolean using reserved words or digits */
#define GUCEF_DATATYPE_BOOLEAN_UTF16_BE_STRING  34      /**< UTF16 Big Endian character set string value actually representing a boolean using reserved words or digits */
#define GUCEF_DATATYPE_BOOLEAN_UTF32_LE_STRING  35      /**< UTF32 Little Endian character set string value actually representing a boolean using reserved words or digits */
#define GUCEF_DATATYPE_BOOLEAN_UTF32_BE_STRING  36      /**< UTF32 Big Endian character set string value actually representing a boolean using reserved words or digits */

#define GUCEF_DATATYPE_BINARY_BLOB              40      /**< binary large object of dynamically determined size */
#define GUCEF_DATATYPE_BINARY_BSOB              41      /**< binary small object of size not exceeding multi-purpose fixed storage */

#define GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING    50  /**< ASCII character set string value actually representing a date time value per the ISO 8601 convention */ 
#define GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING     51  /**< UTF8 character set string value actually representing a date time value per the ISO 8601 convention */ 

#define GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH        55  /**< Little Endian UInt64 based timestamp using seconds since unix epoch */
#define GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH        56  /**< Big Endian UInt64 based timestamp using seconds since unix epoch */
#define GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH          57  /**< Little Endian UInt64 based timestamp using milliseconds since unix epoch */
#define GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH          58  /**< Little Endian UInt64 based timestamp using milliseconds since unix epoch */

#define GUCEF_DATATYPE_ARRAY                    250      /**< data type representing an array container concept */
#define GUCEF_DATATYPE_OBJECT                   251      /**< data type representing an object container concept */
#define GUCEF_DATATYPE_NIL                      252      /**< representing data that is zero or nought. The word nil started to be used in the mid 19th century as a contraction of nihil, a Latin word that means nothing */
#define GUCEF_DATATYPE_NULL                     253      /**< representing data that is null and void */
#define GUCEF_DATATYPE_SET                      254      /**< data type representing a set container concept */

#define GUCEF_DATATYPE_LAST_TYPE_ID         GUCEF_DATATYPE_SET

#define GUCEF_DATATYPE_NAME_NUMERIC                  "NUMERIC"
#define GUCEF_DATATYPE_NAME_ASCII_STRING             "ASCII_STRING"
#define GUCEF_DATATYPE_NAME_UTF8_STRING              "UTF8_STRING"
#define GUCEF_DATATYPE_NAME_UTF16_LE_STRING          "UTF16_LE_STRING"
#define GUCEF_DATATYPE_NAME_UTF16_BE_STRING          "UTF16_BE_STRING"
#define GUCEF_DATATYPE_NAME_UTF32_LE_STRING          "UTF32_LE_STRING"
#define GUCEF_DATATYPE_NAME_UTF32_BE_STRING          "UTF32_BE_STRING"

#define GUCEF_DATATYPE_NAME_BOOLEAN_INT32            "BOOLEAN_INT32"
#define GUCEF_DATATYPE_NAME_BOOLEAN_ASCII_STRING     "BOOLEAN_ASCII_STRING"
#define GUCEF_DATATYPE_NAME_BOOLEAN_UTF8_STRING      "BOOLEAN_UTF8_STRING"
#define GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_LE_STRING  "BOOLEAN_UTF16_LE_STRING"
#define GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_BE_STRING  "BOOLEAN_UTF16_BE_STRING"
#define GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_LE_STRING  "BOOLEAN_UTF32_LE_STRING"
#define GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_BE_STRING  "BOOLEAN_UTF32_BE_STRING"

#define GUCEF_DATATYPE_NAME_BINARY_BLOB              "BLOB"
#define GUCEF_DATATYPE_NAME_BINARY_BSOB              "BSOB"

#define GUCEF_DATATYPE_NAME_DATETIME_ISO8601_ASCII_STRING   "DATETIME_ISO8601_ASCII_STRING"
#define GUCEF_DATATYPE_NAME_DATETIME_ISO8601_UTF8_STRING    "DATETIME_ISO8601_UTF8_STRING"

#define GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH  "LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH"
#define GUCEF_DATATYPE_NAME_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH  "BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH"
#define GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH    "LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH"
#define GUCEF_DATATYPE_NAME_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH    "BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH"

#define GUCEF_DATATYPE_NAME_ARRAY                    "ARRAY"
#define GUCEF_DATATYPE_NAME_OBJECT                   "OBJECT"
#define GUCEF_DATATYPE_NAME_NIL                      "NIL"
#define GUCEF_DATATYPE_NAME_NULL                     "NULL"
#define GUCEF_DATATYPE_NAME_SET                      "SET"

#if GUCEF_PLATFORM_BYTEORDER_ENDIAN == GUCEF_BYTEORDER_LITTLE_ENDIAN

    #define GUCEF_DATATYPE_UTF16_STRING                             GUCEF_DATATYPE_UTF16_LE_STRING    
    #define GUCEF_DATATYPE_UTF32_STRING                             GUCEF_DATATYPE_UTF32_LE_STRING
    #define GUCEF_DATATYPE_BOOLEAN_UTF16_STRING                     GUCEF_DATATYPE_BOOLEAN_UTF16_LE_STRING
    #define GUCEF_DATATYPE_BOOLEAN_UTF32_STRING                     GUCEF_DATATYPE_BOOLEAN_UTF32_LE_STRING
    #define GUCEF_DATATYPE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH       GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH
    #define GUCEF_DATATYPE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH         GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH

    #define GUCEF_DATATYPE_NAME_UTF16_STRING                        GUCEF_DATATYPE_NAME_UTF16_LE_STRING    
    #define GUCEF_DATATYPE_NAME_UTF32_STRING                        GUCEF_DATATYPE_NAME_UTF32_LE_STRING
    #define GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_STRING                GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_LE_STRING
    #define GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_STRING                GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_LE_STRING
    #define GUCEF_DATATYPE_NAME_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH  GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH
    #define GUCEF_DATATYPE_NAME_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH    GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH

#else

    #define GUCEF_DATATYPE_UTF16_STRING                             GUCEF_DATATYPE_UTF16_BE_STRING    
    #define GUCEF_DATATYPE_UTF32_STRING                             GUCEF_DATATYPE_UTF32_BE_STRING
    #define GUCEF_DATATYPE_BOOLEAN_UTF16_STRING                     GUCEF_DATATYPE_BOOLEAN_UTF16_BE_STRING
    #define GUCEF_DATATYPE_BOOLEAN_UTF32_STRING                     GUCEF_DATATYPE_BOOLEAN_UTF32_BE_STRING
    #define GUCEF_DATATYPE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH       GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH
    #define GUCEF_DATATYPE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH         GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH

    #define GUCEF_DATATYPE_NAME_UTF16_STRING                        GUCEF_DATATYPE_NAME_UTF16_BE_STRING    
    #define GUCEF_DATATYPE_NAME_UTF32_STRING                        GUCEF_DATATYPE_NAME_UTF32_BE_STRING
    #define GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_STRING                GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_BE_STRING
    #define GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_STRING                GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_BE_STRING
    #define GUCEF_DATATYPE_NAME_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH  GUCEF_DATATYPE_NAME_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH
    #define GUCEF_DATATYPE_NAME_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH    GUCEF_DATATYPE_NAME_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH

#endif

#ifdef __cplusplus

/**
 *      Build-in data types as part of an enumeration set
 */
enum EBuildinDataType
{
    DATATYPE_UINT8   = GUCEF_DATATYPE_UINT8   ,
    DATATYPE_INT8    = GUCEF_DATATYPE_INT8    ,

    DATATYPE_LE_UINT16  = GUCEF_DATATYPE_LE_UINT16  ,
    DATATYPE_LE_INT16   = GUCEF_DATATYPE_LE_INT16   ,
    DATATYPE_LE_UINT32  = GUCEF_DATATYPE_LE_UINT32  ,
    DATATYPE_LE_INT32   = GUCEF_DATATYPE_LE_INT32   ,
    DATATYPE_LE_UINT64  = GUCEF_DATATYPE_LE_UINT64  ,
    DATATYPE_LE_INT64   = GUCEF_DATATYPE_LE_INT64   ,
    DATATYPE_LE_FLOAT32 = GUCEF_DATATYPE_LE_FLOAT32 ,
    DATATYPE_LE_FLOAT64 = GUCEF_DATATYPE_LE_FLOAT64 ,

    DATATYPE_BE_UINT16  = GUCEF_DATATYPE_BE_UINT16  ,
    DATATYPE_BE_INT16   = GUCEF_DATATYPE_BE_INT16   ,
    DATATYPE_BE_UINT32  = GUCEF_DATATYPE_BE_UINT32  ,
    DATATYPE_BE_INT32   = GUCEF_DATATYPE_BE_INT32   ,
    DATATYPE_BE_UINT64  = GUCEF_DATATYPE_BE_UINT64  ,
    DATATYPE_BE_INT64   = GUCEF_DATATYPE_BE_INT64   ,
    DATATYPE_BE_FLOAT32 = GUCEF_DATATYPE_BE_FLOAT32 ,
    DATATYPE_BE_FLOAT64 = GUCEF_DATATYPE_BE_FLOAT64 ,

    DATATYPE_UNKNOWN    = GUCEF_DATATYPE_UNKNOWN
};
typedef enum EBuildinDataType TBuildinDataType;

enum EBasicDataType
{
    DATATYPE_BINARY_BLOB                = GUCEF_DATATYPE_BINARY_BLOB                ,
    DATATYPE_BINARY_BSOB                = GUCEF_DATATYPE_BINARY_BSOB                ,
    DATATYPE_NUMERIC                    = GUCEF_DATATYPE_NUMERIC                    ,
    DATATYPE_ASCII_STRING               = GUCEF_DATATYPE_ASCII_STRING               ,
    DATATYPE_UTF8_STRING                = GUCEF_DATATYPE_UTF8_STRING                ,
    DATATYPE_UTF16_LE_STRING            = GUCEF_DATATYPE_UTF16_LE_STRING            ,
    DATATYPE_UTF16_BE_STRING            = GUCEF_DATATYPE_UTF16_BE_STRING            ,
    DATATYPE_UTF32_LE_STRING            = GUCEF_DATATYPE_UTF32_LE_STRING            ,
    DATATYPE_UTF32_BE_STRING            = GUCEF_DATATYPE_UTF32_BE_STRING            ,
    DATATYPE_ARRAY                      = GUCEF_DATATYPE_ARRAY                      ,
    DATATYPE_OBJECT                     = GUCEF_DATATYPE_OBJECT                     ,
    DATATYPE_SET                        = GUCEF_DATATYPE_SET                        ,
    DATATYPE_BOOLEAN_INT32              = GUCEF_DATATYPE_BOOLEAN_INT32              ,
    DATATYPE_BOOLEAN_ASCII_STRING       = GUCEF_DATATYPE_BOOLEAN_ASCII_STRING       ,
    DATATYPE_BOOLEAN_UTF8_STRING        = GUCEF_DATATYPE_BOOLEAN_UTF8_STRING        ,
    DATATYPE_BOOLEAN_UTF16_LE_STRING    = GUCEF_DATATYPE_BOOLEAN_UTF16_LE_STRING    ,
    DATATYPE_BOOLEAN_UTF16_BE_STRING    = GUCEF_DATATYPE_BOOLEAN_UTF16_BE_STRING    ,
    DATATYPE_BOOLEAN_UTF32_LE_STRING    = GUCEF_DATATYPE_BOOLEAN_UTF32_LE_STRING    ,
    DATATYPE_BOOLEAN_UTF32_BE_STRING    = GUCEF_DATATYPE_BOOLEAN_UTF32_BE_STRING 
};
typedef enum EBasicDataType TBasicDataType;

#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

#define GUCEF_FLOAT32_COMPARISON_EPSILON   ( (Float32) 0.00001f )
#define GUCEF_FLOAT64_COMPARISON_EPSILON   ( (Float64) 0.00001 )

#define GUCEF_FLOAT64_PI    3.1415926535897931

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_TYPES_H ? */

