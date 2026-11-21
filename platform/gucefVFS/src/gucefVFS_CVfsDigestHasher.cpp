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

#include "gucefVFS_CVfsDigestHasher.h"

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

CVfsDigestHasher::CVfsDigestHasher( CVfsDigest::EAlgorithm algorithm )
    : m_algorithm( algorithm )
    , m_md5()
    , m_sha1()
    , m_sha256()
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

bool
CVfsDigestHasher::IsSupported( void ) const
{GUCEF_TRACE;

    return CVfsDigest::ALGORITHM_MD5 == m_algorithm || CVfsDigest::ALGORITHM_SHA1 == m_algorithm || CVfsDigest::ALGORITHM_SHA256 == m_algorithm;
}

/*-------------------------------------------------------------------------*/

CVfsDigest::EAlgorithm
CVfsDigestHasher::GetAlgorithm( void ) const
{GUCEF_TRACE;

    return m_algorithm;
}

/*-------------------------------------------------------------------------*/

void
CVfsDigestHasher::Reset( void )
{GUCEF_TRACE;

    switch ( m_algorithm )
    {
        case CVfsDigest::ALGORITHM_MD5:     md5_init( &m_md5 ); break;
        case CVfsDigest::ALGORITHM_SHA1:    m_sha1.Reset(); break;
        case CVfsDigest::ALGORITHM_SHA256:  m_sha256.Reset(); break;
        default: break;
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsDigestHasher::Update( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( 0 == size || GUCEF_NULL == data )
        return;

    switch ( m_algorithm )
    {
        case CVfsDigest::ALGORITHM_MD5:
        {
            // md5_append takes an int: a piece is fed in parts that fit
            const md5_byte_t* bytes = static_cast< const md5_byte_t* >( data );
            UInt32 left = size;
            while ( left > 0 )
            {
                const UInt32 part = left > 0x40000000U ? 0x40000000U : left;
                md5_append( &m_md5, bytes, (int) part );
                bytes += part;
                left -= part;
            }
            break;
        }
        case CVfsDigest::ALGORITHM_SHA1:    m_sha1.Update( data, size ); break;
        case CVfsDigest::ALGORITHM_SHA256:  m_sha256.Update( data, size ); break;
        default: break;
    }
}

/*-------------------------------------------------------------------------*/

CVfsDigest
CVfsDigestHasher::Final( void )
{GUCEF_TRACE;

    UInt8 digest[ 32 ];
    switch ( m_algorithm )
    {
        case CVfsDigest::ALGORITHM_MD5:
        {
            md5_finish( &m_md5, digest );
            return CVfsDigest::FromBytes( m_algorithm, digest, 16 );
        }
        case CVfsDigest::ALGORITHM_SHA1:
        {
            m_sha1.Final( digest );
            return CVfsDigest::FromBytes( m_algorithm, digest, 20 );
        }
        case CVfsDigest::ALGORITHM_SHA256:
        {
            m_sha256.Final( digest );
            return CVfsDigest::FromBytes( m_algorithm, digest, 32 );
        }
        default:
            return CVfsDigest();
    }
}

/*-------------------------------------------------------------------------*/

CVfsDigest
CVfsDigestHasher::Compute( CVfsDigest::EAlgorithm algorithm, const void* data, UInt32 size )
{GUCEF_TRACE;

    CVfsDigestHasher hasher( algorithm );
    hasher.Update( data, size );
    return hasher.Final();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
