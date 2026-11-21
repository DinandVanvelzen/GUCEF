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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#include "gucefASSETSTORE_CRemoteAssetStoreProvider.h"

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

CRemoteAssetStoreProvider::CRemoteAssetStoreProvider( CIAssetStoreService* service, const CORE::CString& storeId, const CORE::CString& displayName, bool signedIn )
    : CIAssetStoreProvider()
    , m_service( service )
    , m_storeId( storeId )
    , m_displayName( displayName )
    , m_signedIn( signedIn )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteAssetStoreProvider::~CRemoteAssetStoreProvider()
{GUCEF_TRACE;

    WaitForAsyncOperations();
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreProvider::Update( const CORE::CString& displayName, bool signedIn )
{GUCEF_TRACE;

    bool changed = false;
    {
        MT::CScopeMutex lock( m_lock );
        changed = m_signedIn != signedIn;
        if ( !displayName.IsNULLOrEmpty() )
            m_displayName = displayName;
        m_signedIn = signedIn;
    }
    if ( changed )
        NotifySessionStateChanged();
}

/*-------------------------------------------------------------------------*/

void
CRemoteAssetStoreProvider::Detach( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_service = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteAssetStoreProvider::GetProviderId( void ) const
{GUCEF_TRACE;

    return m_storeId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteAssetStoreProvider::GetDisplayName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_displayName.IsNULLOrEmpty() ? m_storeId : m_displayName;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteAssetStoreProvider::IsSignedIn( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_signedIn;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreProvider::QueryLibrary( const CAssetStoreQuery& query, CAssetStoreLibraryPage& page )
{GUCEF_TRACE;

    CIAssetStoreService* service = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        service = m_service;
    }
    if ( GUCEF_NULL == service )
        return ASSETSTORE_STATUS_UNAVAILABLE;
    return service->QueryLibrary( m_storeId, query, page );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreProvider::GetItem( const CORE::CString& itemId, CAssetStoreItem& item )
{GUCEF_TRACE;

    CIAssetStoreService* service = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        service = m_service;
    }
    if ( GUCEF_NULL == service )
        return ASSETSTORE_STATUS_UNAVAILABLE;
    return service->GetStoreItem( m_storeId, itemId, item );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteAssetStoreProvider::PlanDownload( const CORE::CString& itemId              ,
                                         const CORE::CString& formatId            ,
                                         const CORE::CString::StringVector& fileIds,
                                         CAssetStoreDownloadPlan& plan            )
{GUCEF_TRACE;

    // the transfers of a remote store are made by the server: EnqueueDownload() of the service
    return ASSETSTORE_STATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteAssetStoreProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CRemoteAssetStoreProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
