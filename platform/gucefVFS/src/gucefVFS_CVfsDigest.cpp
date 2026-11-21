/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefVFS_CVfsDigest.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsDigest::CVfsDigest( void )
    : algorithm( ALGORITHM_UNKNOWN )
    , value()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsDigest::CVfsDigest( EAlgorithm digestAlgorithm, const CORE::CString& digestValue )
    : algorithm( digestAlgorithm )
    , value( ALGORITHM_UNKNOWN == digestAlgorithm ? digestValue : NormalizeValue( digestValue ) )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsDigest::CVfsDigest( const CVfsDigest& src )
    : algorithm( src.algorithm )
    , value( src.value )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsDigest&
CVfsDigest::operator=( const CVfsDigest& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        algorithm = src.algorithm;
        value = src.value;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVfsDigest
CVfsDigest::FromBytes( EAlgorithm digestAlgorithm, const void* digestBytes, UInt32 digestByteCount )
{GUCEF_TRACE;

    if ( ALGORITHM_UNKNOWN == digestAlgorithm || GUCEF_NULL == digestBytes || digestByteCount != GetByteLength( digestAlgorithm ) )
        return CVfsDigest();

    return CVfsDigest( digestAlgorithm, CORE::Base16Encode( digestBytes, digestByteCount ) );
}

/*-------------------------------------------------------------------------*/

bool
CVfsDigest::IsEmpty( void ) const
{GUCEF_TRACE;

    return value.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CVfsDigest::IsWellFormed( void ) const
{GUCEF_TRACE;

    if ( ALGORITHM_UNKNOWN == algorithm || value.Length() != GetHexLength( algorithm ) )
        return false;

    const char* chars = value.C_String();
    for ( UInt32 i=0; i<value.Length(); ++i )
    {
        const char c = chars[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVfsDigest::Matches( const CVfsDigest& other ) const
{GUCEF_TRACE;

    // an unknown algorithm is not a digest anyone can compare; a value of the wrong form is no digest of the algorithm
    if ( algorithm != other.algorithm || !IsWellFormed() || !other.IsWellFormed() )
        return false;

    return value == other.value;
}

/*-------------------------------------------------------------------------*/

bool
CVfsDigest::ToBytes( UInt8* bytes, UInt32& byteCount ) const
{GUCEF_TRACE;

    byteCount = 0;
    if ( GUCEF_NULL == bytes || !IsWellFormed() )
        return false;

    UInt32 used = 0;
    if ( !CORE::Base16Decode( value, false, bytes, GetByteLength( algorithm ), used ) || used != GetByteLength( algorithm ) )
        return false;

    byteCount = used;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsDigest::NormalizeValue( const CORE::CString& digestValue )
{GUCEF_TRACE;

    const char* chars = digestValue.C_String();
    UInt32 first = 0;
    UInt32 last = digestValue.Length();
    while ( first < last && ( ' ' == chars[ first ] || '\t' == chars[ first ] || '\r' == chars[ first ] || '\n' == chars[ first ] ) )
        ++first;
    while ( last > first && ( ' ' == chars[ last - 1 ] || '\t' == chars[ last - 1 ] || '\r' == chars[ last - 1 ] || '\n' == chars[ last - 1 ] ) )
        --last;

    return CORE::CString( chars + first, last - first ).Lowercase();
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsDigest::GetHexLength( EAlgorithm digestAlgorithm )
{GUCEF_TRACE;

    return GetByteLength( digestAlgorithm ) * 2;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsDigest::GetByteLength( EAlgorithm digestAlgorithm )
{GUCEF_TRACE;

    switch ( digestAlgorithm )
    {
        case ALGORITHM_MD5:     return 16;
        case ALGORITHM_SHA1:    return 20;
        case ALGORITHM_SHA256:  return 32;
        default:                return 0;
    }
}

/*-------------------------------------------------------------------------*/

const char*
CVfsDigest::GetAlgorithmName( EAlgorithm digestAlgorithm )
{GUCEF_TRACE;

    switch ( digestAlgorithm )
    {
        case ALGORITHM_MD5:     return "md5";
        case ALGORITHM_SHA1:    return "sha1";
        case ALGORITHM_SHA256:  return "sha256";
        default:                return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

CVfsDigest::EAlgorithm
CVfsDigest::GetAlgorithmFromName( const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CString lowered = NormalizeValue( name );
    if ( lowered == "md5" )
        return ALGORITHM_MD5;
    if ( lowered == "sha1" || lowered == "sha-1" )
        return ALGORITHM_SHA1;
    if ( lowered == "sha256" || lowered == "sha-256" )
        return ALGORITHM_SHA256;
    return ALGORITHM_UNKNOWN;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
