/*
 *  AssetWorkbenchLib: gives the resources of the resource view the thumbnails that exist for the files of the pack
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

#include "AssetWorkbenchLib_CResourceViewThumbnailProvider.h"

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

CResourceViewThumbnailProvider::CResourceViewThumbnailProvider( CThumbnailProvider& inner, const CResourceViewFileSystemAccess& view )
    : GUI::CIThumbnailProvider()
    , m_inner( inner )
    , m_view( view )
    , m_resourceMode( false )
    , m_pending()
{GUCEF_TRACE;

    TEventCallback readyCallback( this, &CResourceViewThumbnailProvider::OnInnerThumbnailReady );
    SubscribeTo( &m_inner, ThumbnailReadyEvent, readyCallback );
}

/*-------------------------------------------------------------------------*/

CResourceViewThumbnailProvider::~CResourceViewThumbnailProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void CResourceViewThumbnailProvider::SetResourceMode( bool resourceMode ) { m_resourceMode = resourceMode; }
bool CResourceViewThumbnailProvider::IsResourceMode( void ) const { return m_resourceMode; }

/*-------------------------------------------------------------------------*/

void
CResourceViewThumbnailProvider::RequestThumbnail( const CORE::CString& path, UInt32 size )
{GUCEF_TRACE;

    ASSETCONVERTER::CPackIndexResource resource;
    if ( !m_resourceMode )
    {
        m_inner.RequestThumbnail( path, size );
        return;
    }
    if ( !m_view.TryResolve( path, resource ) )
    {
        // a group: nothing to show
        GUI::CIThumbnailProvider::TThumbnail thumbnail;
        thumbnail.path = path;
        thumbnail.size = size;
        TThumbnailReadyEventData eventData( thumbnail );
        NotifyObservers( ThumbnailReadyEvent, &eventData );
        return;
    }
    const CORE::CString file = m_view.GetVfsPathOf( resource );
    CORE::CString::StringVector& askers = m_pending[ file + "\n" + CORE::UInt32ToString( size ) ];
    for ( size_t i=0; i<askers.size(); ++i )
        if ( askers[ i ] == path )
            return;
    askers.push_back( path );
    m_inner.RequestExistingThumbnail( file, size );
}

/*-------------------------------------------------------------------------*/

void
CResourceViewThumbnailProvider::CancelThumbnail( const CORE::CString& path, UInt32 size )
{GUCEF_TRACE;

    if ( !m_resourceMode )
    {
        m_inner.CancelThumbnail( path, size );
        return;
    }
    // a request for what exists is answered within a frame or two: it is let go of when the answer comes
}

/*-------------------------------------------------------------------------*/

void
CResourceViewThumbnailProvider::OnInnerThumbnailReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const TThumbnailReadyEventData* ready = static_cast< const TThumbnailReadyEventData* >( eventData );
    const TThumbnail& thumbnail = ready->GetData();
    TPendingMap::iterator found = m_pending.find( thumbnail.path + "\n" + CORE::UInt32ToString( thumbnail.size ) );
    if ( found == m_pending.end() )
    {
        // not asked for through the resource view: told on as it is
        TThumbnailReadyEventData passed( thumbnail );
        NotifyObservers( ThumbnailReadyEvent, &passed );
        return;
    }
    const CORE::CString::StringVector askers = (*found).second;
    m_pending.erase( found );
    for ( size_t i=0; i<askers.size(); ++i )
    {
        TThumbnail answer = thumbnail;
        answer.path = askers[ i ];
        TThumbnailReadyEventData answerData( answer );
        NotifyObservers( ThumbnailReadyEvent, &answerData );
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
