/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#define TABLECODECPLUGIN_PARQUET_BYTEORDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_BYTEORDER_ENDIAN_COMPILE_TIME == GUCEF_BYTEORDER_LITTLE_ENDIAN )
  #define TABLECODECPLUGIN_PARQUET_HOST_IS_LITTLE_ENDIAN 1
#else
  #define TABLECODECPLUGIN_PARQUET_HOST_IS_LITTLE_ENDIAN 0
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace PARQUET {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Parquet is little endian. These read and write little endian values at any address (no alignment needed);
 *  on little endian hosts they compile to plain loads and stores.
 */

inline UInt16 LoadLE16( const UInt8* p ) { return (UInt16) ( (UInt16) p[ 0 ] | ( (UInt16) p[ 1 ] << 8 ) ); }

inline UInt32 LoadLE32( const UInt8* p )
{
    #if TABLECODECPLUGIN_PARQUET_HOST_IS_LITTLE_ENDIAN
    UInt32 v; memcpy( &v, p, 4 ); return v;
    #else
    return (UInt32) p[ 0 ] | ( (UInt32) p[ 1 ] << 8 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 3 ] << 24 );
    #endif
}

inline UInt64 LoadLE64( const UInt8* p )
{
    #if TABLECODECPLUGIN_PARQUET_HOST_IS_LITTLE_ENDIAN
    UInt64 v; memcpy( &v, p, 8 ); return v;
    #else
    return (UInt64) LoadLE32( p ) | ( (UInt64) LoadLE32( p + 4 ) << 32 );
    #endif
}

inline void StoreLE32( UInt8* p, UInt32 v )
{
    #if TABLECODECPLUGIN_PARQUET_HOST_IS_LITTLE_ENDIAN
    memcpy( p, &v, 4 );
    #else
    p[ 0 ] = (UInt8) v; p[ 1 ] = (UInt8) ( v >> 8 ); p[ 2 ] = (UInt8) ( v >> 16 ); p[ 3 ] = (UInt8) ( v >> 24 );
    #endif
}

inline void StoreLE64( UInt8* p, UInt64 v )
{
    #if TABLECODECPLUGIN_PARQUET_HOST_IS_LITTLE_ENDIAN
    memcpy( p, &v, 8 );
    #else
    StoreLE32( p, (UInt32) v ); StoreLE32( p + 4, (UInt32) ( v >> 32 ) );
    #endif
}

inline Float32 LoadLEFloat32( const UInt8* p ) { UInt32 bits = LoadLE32( p ); Float32 v; memcpy( &v, &bits, 4 ); return v; }

inline Float64 LoadLEFloat64( const UInt8* p ) { UInt64 bits = LoadLE64( p ); Float64 v; memcpy( &v, &bits, 8 ); return v; }

inline void StoreLEFloat32( UInt8* p, Float32 v ) { UInt32 bits; memcpy( &bits, &v, 4 ); StoreLE32( p, bits ); }

inline void StoreLEFloat64( UInt8* p, Float64 v ) { UInt64 bits; memcpy( &bits, &v, 8 ); StoreLE64( p, bits ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_PARQUET_BYTEORDER_H ? */
