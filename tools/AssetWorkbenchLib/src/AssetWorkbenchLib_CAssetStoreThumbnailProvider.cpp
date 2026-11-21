/*
 *  AssetWorkbenchLib: the asset sources of the workbench: online stores, the primary store and the transfers
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSTORETHUMBNAILPROVIDER_H
#include "AssetWorkbenchLib_CAssetStoreThumbnailProvider.h"
#define ASSETWORKBENCHLIB_CASSETSTORETHUMBNAILPROVIDER_H
#endif /* ASSETWORKBENCHLIB_CASSETSTORETHUMBNAILPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


CAssetStoreThumbnailProvider::CAssetStoreThumbnailProvider( ASSETSTORE::CIAssetStoreService& service, ASSETSTORE::CAssetStoreLibraryFileSystemAccess& library )
    : GUI::CIThumbnailProvider()
    , m_service( service )
    , m_library( library )
    , m_waiting()
{GUCEF_TRACE;

    TEventCallback callback( this, &CAssetStoreThumbnailProvider::OnImageReady );
    SubscribeTo( &m_service, ASSETSTORE::CIAssetStoreService::ListingImageReadyEvent, callback );
}

/*-------------------------------------------------------------------------*/

CAssetStoreThumbnailProvider::~CAssetStoreThumbnailProvider()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreThumbnailProvider::Answer( const CORE::CString& path, UInt32 size, const IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    TThumbnail thumbnail;
    thumbnail.path = path;
    thumbnail.size = size;
    thumbnail.pixels = pixels;
    TThumbnailReadyEventData data( thumbnail );
    NotifyObservers( ThumbnailReadyEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreThumbnailProvider::RequestThumbnail( const CORE::CString& path, UInt32 size )
{GUCEF_TRACE;

    const CORE::CString url = m_library.GetThumbnailUrlOfPath( path );
    if ( url.IsNULLOrEmpty() )
    {
        Answer( path, size, IMAGE::TPixelMapPtr() );
        return;
    }
    ASSETSTORE::CAssetThumbnail image;
    if ( m_service.TryGetListingImage( url, size, image ) )
    {
        Answer( path, size, image.pixels );
        return;
    }
    for ( size_t i=0; i<m_waiting.size(); ++i )
    {
        if ( m_waiting[ i ].url == url && m_waiting[ i ].size == size )
        {
            m_waiting[ i ].paths.push_back( path );
            return;
        }
    }
    SWaiting waiting;
    waiting.url = url;
    waiting.size = size;
    waiting.paths.push_back( path );
    m_waiting.push_back( waiting );
    if ( !m_service.RequestListingImage( url, size ) )
    {
        // the service cannot ask for it: the paths are answered without a picture
        m_waiting.pop_back();
        Answer( path, size, IMAGE::TPixelMapPtr() );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreThumbnailProvider::CancelThumbnail( const CORE::CString& path, UInt32 size )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_waiting.size(); ++i )
    {
        if ( m_waiting[ i ].size != size )
            continue;
        CORE::CString::StringVector& paths = m_waiting[ i ].paths;
        for ( size_t p=0; p<paths.size(); ++p )
        {
            if ( paths[ p ] == path )
            {
                paths.erase( paths.begin() + p );
                break;
            }
        }
        if ( paths.empty() )
        {
            m_service.CancelListingImage( m_waiting[ i ].url, size );
            m_waiting.erase( m_waiting.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreThumbnailProvider::GetWaitingCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<m_waiting.size(); ++i )
        count += (UInt32) m_waiting[ i ].paths.size();
    return count;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreThumbnailProvider::OnImageReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CAssetThumbnail* data = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetThumbnail >( eventData );
    if ( GUCEF_NULL == data )
        return;
    for ( size_t i=0; i<m_waiting.size(); ++i )
    {
        if ( m_waiting[ i ].url == data->url && m_waiting[ i ].size == data->size )
        {
            const SWaiting waiting = m_waiting[ i ];
            m_waiting.erase( m_waiting.begin() + i );
            for ( size_t p=0; p<waiting.paths.size(); ++p )
                Answer( waiting.paths[ p ], waiting.size, data->pixels );
            return;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
