/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#include "gucefASSETSTORE_CAssetDigest.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetDigest::Sha1 = "sha1";
const CORE::CString CAssetDigest::Sha256 = "sha256";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetDigest::CAssetDigest( const CORE::CString& algorithm )
    : m_algorithm( algorithm.Lowercase() == Sha1 ? Sha1 : Sha256 )
    , m_sha1()
    , m_sha256()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetDigest::Reset( void )
{GUCEF_TRACE;

    m_sha1.Reset();
    m_sha256.Reset();
}

/*-------------------------------------------------------------------------*/

void
CAssetDigest::Update( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( m_algorithm == Sha1 )
        m_sha1.Update( data, size );
    else
        m_sha256.Update( data, size );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetDigest::FinalHex( void )
{GUCEF_TRACE;

    CORE::CString hex;
    if ( m_algorithm == Sha1 )
    {
        hex = m_sha1.FinalHex();
    }
    else
    {
        UInt8 digest[ 32 ];
        m_sha256.Final( digest );
        static const char digits[] = "0123456789abcdef";
        char text[ 65 ];
        for ( UInt32 i=0; i<32; ++i )
        {
            text[ i*2 ] = digits[ digest[ i ] >> 4 ];
            text[ i*2+1 ] = digits[ digest[ i ] & 0x0F ];
        }
        text[ 64 ] = '\0';
        hex = text;
    }
    Reset();
    return hex;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetDigest::GetAlgorithm( void ) const
{GUCEF_TRACE;

    return m_algorithm;
}

/*-------------------------------------------------------------------------*/

bool
CAssetDigest::IsSupportedAlgorithm( const CORE::CString& algorithm )
{GUCEF_TRACE;

    const CORE::CString lower = algorithm.Lowercase();
    return lower == Sha1 || lower == Sha256;
}

/*-------------------------------------------------------------------------*/

bool
CAssetDigest::AreEqual( const CORE::CString& digestA, const CORE::CString& digestB )
{GUCEF_TRACE;

    return !digestA.IsNULLOrEmpty() && digestA.Equals( digestB, false );
}

/*-------------------------------------------------------------------------*/

bool
CAssetDigest::ComputeVfsFileDigest( const CORE::CString& vfsPath, const CORE::CString& algorithm, CORE::CString& digestHex, UInt64* fileSize )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::TBasicVfsResourcePtr file = vfs.GetFile( vfsPath, "rb", false );
    if ( file.IsNULL() )
        return false;
    CORE::IOAccessPtr access = file->GetAccess();
    if ( access.IsNULL() )
        return false;

    CAssetDigest digest( algorithm );
    UInt64 total = 0;
    char buffer[ 65536 ];
    while ( !access->Eof() )
    {
        const UInt32 read = access->Read( buffer, 1, sizeof( buffer ) );
        if ( 0 == read )
            break;
        digest.Update( buffer, read );
        total += read;
    }
    digestHex = digest.FinalHex();
    if ( GUCEF_NULL != fileSize )
        *fileSize = total;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
