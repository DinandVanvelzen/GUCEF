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

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#include "gucefVFS_CVfsFileSystemAccess.h"
#define GUCEF_VFS_CVFSFILESYSTEMACCESS_H
#endif /* GUCEF_VFS_CVFSFILESYSTEMACCESS_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSHAREDECISION_H
#include "gucefASSETSTORE_CAssetShareDecision.h"
#define GUCEF_ASSETSTORE_CASSETSHAREDECISION_H
#endif /* GUCEF_ASSETSTORE_CASSETSHAREDECISION_H ? */

#include "gucefASSETSTORE_CAssetShareProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The file system access of the VFS the share reads from */
CORE::TFileSystemAccessPtr
MakeVfsFileSystem( void )
{GUCEF_TRACE;

    return ( GUCEF_NEW VFS::CVfsFileSystemAccess() )->CreateSharedPtr();
}

}

/*-------------------------------------------------------------------------*/

CAssetShareProvider::CAssetShareProvider( const CORE::CString& vfsRoot, const TAssetStoreServicePtr& service, CAssetShareDecision* decision )
    : DRN::CDRNFileSystemAccessResourceProvider( MakeVfsFileSystem(), "/" + vfsRoot, CORE::CUri( "vfs://" + vfsRoot ) )
    , m_vfsRoot( vfsRoot )
    , m_service( service )
    , m_decision( decision )
    , m_digests( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetShareProvider::~CAssetShareProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetShareProvider::GetVfsRoot( void ) const
{GUCEF_TRACE;

    return m_vfsRoot;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetShareProvider::GetCatalogueDigestCount( void ) const
{GUCEF_TRACE;

    return m_digests.Load();
}

/*-------------------------------------------------------------------------*/

bool
CAssetShareProvider::GetKnownSha256( const CORE::CString& name, const DRN::CDRNNamespaceEntry& entry, UInt8* digest ) const
{GUCEF_TRACE;

    if ( m_service.IsNULL() || name.IsNULLOrEmpty() )
        return false;
    const CORE::CString servicePath = m_vfsRoot + "/" + name;
    if ( GUCEF_NULL != m_decision )
        m_decision->Warm( servicePath );

    // the digest of the catalogue is the digest of the file only when the file is the size the catalogue has of it
    UInt64 size = 0;
    CORE::CString hex;
    if ( !m_service->GetCatalogueDigestOfPath( servicePath, size, hex ) || size != entry.size || 64 != hex.Length() )
        return false;
    if ( !DRN::CDRNCrypto::FromHex( hex, digest, 32 ) )
        return false;
    ++m_digests;
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
