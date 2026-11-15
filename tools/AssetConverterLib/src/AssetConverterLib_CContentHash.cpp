/*
 *  AssetConverterLib: SHA-256 content hashes of files and buffers
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

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "AssetConverterLib_CContentHash.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
DigestToHex( const UInt8* digest )
{
    return CORE::ConvertBytesToHexString( digest, CORE::CSha256::DigestSize, false, false ).Lowercase();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CContentHash::HashFile( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    VFS::TBasicVfsResourcePtr resource = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( vfsPath, "rb", false );
    if ( resource.IsNULL() )
        return CORE::CString::Empty;
    CORE::IOAccessPtr access = resource->GetAccess();
    if ( access.IsNULL() )
        return CORE::CString::Empty;

    CORE::CSha256 sha;
    UInt8 piece[ 65536 ];
    while ( true )
    {
        const UInt32 read = access->Read( piece, 1, sizeof( piece ) );
        if ( 0 == read )
            break;
        sha.Update( piece, read );
    }
    UInt8 digest[ CORE::CSha256::DigestSize ];
    sha.Final( digest );
    return DigestToHex( digest );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CContentHash::HashBuffer( const void* data, UInt32 size )
{GUCEF_TRACE;

    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( data, size, digest );
    return DigestToHex( digest );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CContentHash::HashText( const CORE::CString& text )
{GUCEF_TRACE;

    return HashBuffer( text.C_String(), text.ByteSize() > 0 ? text.ByteSize()-1 : 0 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
