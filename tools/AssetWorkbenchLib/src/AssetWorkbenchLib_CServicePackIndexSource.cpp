/*
 *  AssetWorkbenchLib: the library of the asset workbench
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "AssetWorkbenchLib_CServicePackIndexSource.h"

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

CServicePackIndexSource::CServicePackIndexSource( ASSETSTORE::CIAssetStoreService& service )
    : IPackIndexSource()
    , CORE::CObservingNotifier()
    , m_service( service )
    , m_cache( service.GetType() )
    , m_learning( false )
    , m_servingFromCache( false )
    , m_calls( 0 )
    , m_lastMessage()
    , m_lock()
{GUCEF_TRACE;

    TEventCallback callback( this, &CServicePackIndexSource::OnCatalogueChanged );
    SubscribeTo( &m_service, ASSETSTORE::CIAssetStoreService::CatalogueChangedEvent, callback );
}

/*-------------------------------------------------------------------------*/

CServicePackIndexSource::~CServicePackIndexSource()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CServicePackIndexSource::SetDiskCacheFolder( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    m_cache.SetDiskFolder( vfsFolder );
}

/*-------------------------------------------------------------------------*/

void
CServicePackIndexSource::SetLearning( bool learning )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_learning != learning )
    {
        // what a consumer of the other kind was shown is not what this one is shown
        m_learning = learning;
        m_cache.DropAllPages();
        m_cache.MarkAllForCheck();
    }
}

/*-------------------------------------------------------------------------*/

bool
CServicePackIndexSource::IsLearning( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_learning;
}

/*-------------------------------------------------------------------------*/

const CPackIndexClientCache&
CServicePackIndexSource::GetCache( void ) const
{GUCEF_TRACE;

    return m_cache;
}

/*-------------------------------------------------------------------------*/

UInt32
CServicePackIndexSource::GetServiceCallCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_calls;
}

/*-------------------------------------------------------------------------*/

bool
CServicePackIndexSource::IsServingFromCache( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_servingFromCache;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServicePackIndexSource::GetName( void ) const
{GUCEF_TRACE;

    return "the asset store service (" + m_service.GetType() + ")";
}

/*-------------------------------------------------------------------------*/

bool
CServicePackIndexSource::IsCheap( void ) const
{GUCEF_TRACE;

    return m_service.IsInProcess();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServicePackIndexSource::GetLastMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastMessage;
}

/*-------------------------------------------------------------------------*/

IPackIndexSource::EResult
CServicePackIndexSource::Fail( EResult result, const CORE::CString& message )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_lastMessage = message;
    return result;
}

/*-------------------------------------------------------------------------*/

IPackIndexSource::EResult
CServicePackIndexSource::ResultOf( ASSETSTORE::TAssetStoreStatus status, const CORE::CString& what )
{GUCEF_TRACE;

    if ( ASSETSTORE::ASSETSTORE_STATUS_OK == status )
        return RESULT_OK;
    const CORE::CString text = what + ": " + CORE::CString( ASSETSTORE::AssetStoreStatusToString( status ) );
    switch ( status )
    {
        case ASSETSTORE::ASSETSTORE_STATUS_NOT_FOUND:
        case ASSETSTORE::ASSETSTORE_STATUS_INVALID_ARGUMENT:
            return Fail( RESULT_NOT_FOUND, text );
        case ASSETSTORE::ASSETSTORE_STATUS_UNAVAILABLE:
        case ASSETSTORE::ASSETSTORE_STATUS_NETWORK_ERROR:
        case ASSETSTORE::ASSETSTORE_STATUS_TIMED_OUT:
            return Fail( RESULT_UNAVAILABLE, text );
        default:
            return Fail( RESULT_FAILED, text );
    }
}

/*-------------------------------------------------------------------------*/

void
CServicePackIndexSource::ResourceFromRecord( const ASSETSTORE::CAssetPackResource& record, ASSETCONVERTER::CPackIndexResource& resource )
{GUCEF_TRACE;

    resource = ASSETCONVERTER::CPackIndexResource();
    resource.id = record.resourceId;
    resource.path = record.path;
    resource.name = record.name;
    resource.title = record.title;
    resource.kind = record.kind;
    resource.storeKind = record.storeKind;
    resource.group = record.groupId;
    resource.subGroup = record.subGroup;
    resource.groupRule = record.groupRule;
    resource.format = record.format;
    resource.size = record.sizeBytes;
    resource.digest = record.digest;
    resource.presentedUnder = record.presentedUnder;
    resource.status.state = record.status;
    if ( record.analysed )
        resource.status.SetStep( ASSETCONVERTER::CPackIndexStatus::StepAnalyse, ASSETCONVERTER::CPackIndexStepStatus( ASSETCONVERTER::CPackIndexStepStatus::Fresh ) );
    if ( record.converted )
        resource.status.SetStep( ASSETCONVERTER::CPackIndexStatus::StepConvert, ASSETCONVERTER::CPackIndexStepStatus( ASSETCONVERTER::CPackIndexStepStatus::Fresh ) );
}

/*-------------------------------------------------------------------------*/

void
CServicePackIndexSource::GroupsFromRows( const ASSETSTORE::TAssetPackGroupVector& rows, ASSETCONVERTER::CPackIndexGroup::TGroupVector& groups )
{GUCEF_TRACE;

    groups.clear();
    for ( size_t r=0; r<rows.size(); ++r )
    {
        if ( rows[ r ].subGroup.IsNULLOrEmpty() )
        {
            ASSETCONVERTER::CPackIndexGroup group;
            group.id = rows[ r ].groupId;
            group.name = rows[ r ].name;
            group.count = (UInt32) rows[ r ].resourceCount;
            group.bytes = rows[ r ].sizeBytes;
            groups.push_back( group );
        }
    }
    for ( size_t r=0; r<rows.size(); ++r )
    {
        if ( rows[ r ].subGroup.IsNULLOrEmpty() )
            continue;
        for ( size_t g=0; g<groups.size(); ++g )
        {
            if ( groups[ g ].id != rows[ r ].groupId )
                continue;
            ASSETCONVERTER::CPackIndexSubGroup sub;
            sub.id = rows[ r ].subGroup;
            sub.name = rows[ r ].name;
            sub.count = (UInt32) rows[ r ].resourceCount;
            sub.bytes = rows[ r ].sizeBytes;
            groups[ g ].subGroups.push_back( sub );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CServicePackIndexSource::DocumentFromInfo( const ASSETSTORE::CAssetPackIndexInfo& info, ASSETCONVERTER::CPackIndexDocument& document )
{GUCEF_TRACE;

    document.Clear();
    if ( info.hasManifest && document.DeserializeManifest( info.manifest ) )
    {
        document.resources.clear();
        return;
    }
    // the document could not be read by the service: what the catalogue holds of it
    document.identity.packKey = info.packKey;
    document.producer.indexKey = info.indexKey;
    document.documentHash = info.documentHash;
    document.state = ASSETCONVERTER::CPackIndexDocument::StateComplete;
    document.levels = info.levels;
    document.counts.resources = (UInt32) info.resourceCount;
    document.counts.noise = (UInt32) info.noiseCount;
    GroupsFromRows( info.groups, document.groups );
}

/*-------------------------------------------------------------------------*/

IPackIndexSource::EResult
CServicePackIndexSource::GetManifest( const CORE::CString& packId, ASSETCONVERTER::CPackIndexDocument& manifest, bool& stale )
{GUCEF_TRACE;

    stale = false;
    CPackIndexClientCache::SManifest cached;
    const bool inMemory = m_cache.FindManifest( packId, cached );
    if ( inMemory && !cached.needsCheck )
    {
        manifest = cached.document;
        stale = cached.stale;
        MT::CScopeMutex lock( m_lock );
        m_servingFromCache = false;
        return RESULT_OK;
    }

    bool learning;
    {
        MT::CScopeMutex lock( m_lock );
        learning = m_learning;
        ++m_calls;
    }
    ASSETSTORE::CAssetPackIndexInfo info;
    const ASSETSTORE::TAssetStoreStatus status = m_service.GetPackIndex( packId, learning, info );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK == status )
    {
        {
            MT::CScopeMutex lock( m_lock );
            m_servingFromCache = false;
        }
        CPackIndexClientCache::SManifest entry;
        if ( inMemory && cached.indexKey == info.indexKey && cached.documentHash == info.documentHash )
        {
            // the index key is the same: the document is the same, the cached manifest stands
            entry = cached;
            entry.stale = info.stale;
            entry.needsCheck = false;
        }
        else
        {
            entry.indexKey = info.indexKey;
            entry.documentHash = info.documentHash;
            entry.stale = info.stale;
            DocumentFromInfo( info, entry.document );
        }
        m_cache.PutManifest( packId, entry );
        manifest = entry.document;
        stale = entry.stale;
        return RESULT_OK;
    }

    const EResult result = ResultOf( status, "the pack index of " + packId );
    if ( RESULT_NOT_FOUND == result )
    {
        m_cache.RemovePack( packId );
        return result;
    }

    // the service cannot be reached: what was known of the pack is shown
    CPackIndexClientCache::SManifest known = cached;
    if ( inMemory || m_cache.LoadManifestFromDisk( packId, known ) )
    {
        manifest = known.document;
        stale = true;
        MT::CScopeMutex lock( m_lock );
        m_servingFromCache = true;
        return RESULT_OK;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

IPackIndexSource::EResult
CServicePackIndexSource::GetGroups( const CORE::CString& packId, ASSETCONVERTER::CPackIndexGroup::TGroupVector& groups )
{GUCEF_TRACE;

    groups.clear();
    const CORE::CString key = CPackIndexClientCache::MakeGroupsKey( packId );
    CPackIndexClientCache::SPage cached;
    if ( m_cache.FindPage( key, cached ) )
    {
        groups = cached.groups;
        return RESULT_OK;
    }
    bool learning;
    {
        MT::CScopeMutex lock( m_lock );
        learning = m_learning;
        ++m_calls;
    }
    ASSETSTORE::CAssetPackGroupPage page;
    const ASSETSTORE::TAssetStoreStatus status = m_service.GetResourceGroups( packId, learning, page );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
        return ResultOf( status, "the groups of the pack index of " + packId );
    GroupsFromRows( page.entries, groups );
    CPackIndexClientCache::SPage entry;
    entry.key = key;
    entry.packId = packId;
    entry.revision = page.revision;
    entry.groups = groups;
    m_cache.PutPage( entry );
    return RESULT_OK;
}

/*-------------------------------------------------------------------------*/

IPackIndexSource::EResult
CServicePackIndexSource::GetResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup,
                                       const CORE::CString& pageToken, UInt32 maxEntries,
                                       ASSETCONVERTER::CPackIndexResource::TResourceVector& page, CORE::CString& nextPageToken, UInt64& total )
{GUCEF_TRACE;

    page.clear();
    nextPageToken.Clear();
    total = 0;
    const CORE::CString key = CPackIndexClientCache::MakeResourcesKey( packId, groupId, subGroup, pageToken, maxEntries );
    CPackIndexClientCache::SPage cached;
    if ( m_cache.FindPage( key, cached ) )
    {
        page = cached.resources;
        nextPageToken = cached.nextPageToken;
        total = cached.total;
        return RESULT_OK;
    }
    bool learning;
    {
        MT::CScopeMutex lock( m_lock );
        learning = m_learning;
        ++m_calls;
    }
    ASSETSTORE::CAssetPackResourcePage records;
    const ASSETSTORE::TAssetStoreStatus status = m_service.GetResources( packId, groupId, subGroup, pageToken, maxEntries, learning, records );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
        return ResultOf( status, "the resources of the pack index of " + packId );
    page.resize( records.entries.size() );
    for ( size_t r=0; r<records.entries.size(); ++r )
        ResourceFromRecord( records.entries[ r ], page[ r ] );
    nextPageToken = records.nextPageToken;
    total = records.total;
    CPackIndexClientCache::SPage entry;
    entry.key = key;
    entry.packId = packId;
    entry.revision = records.revision;
    entry.total = total;
    entry.nextPageToken = nextPageToken;
    entry.resources = page;
    m_cache.PutPage( entry );
    return RESULT_OK;
}

/*-------------------------------------------------------------------------*/

void
CServicePackIndexSource::OnCatalogueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CAssetCatalogueChange* change = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetCatalogueChange >( eventData );
    if ( GUCEF_NULL == change )
        return;
    switch ( change->kind )
    {
        case ASSETSTORE::ASSETCHANGE_RESYNC:
        {
            // what the journal no longer holds: everything is asked again
            m_cache.DropAllPages();
            m_cache.MarkAllForCheck();
            break;
        }
        case ASSETSTORE::ASSETCHANGE_PACK_INDEX_UPDATED:
        case ASSETSTORE::ASSETCHANGE_PACK_UPDATED:
        {
            for ( size_t i=0; i<change->ids.size(); ++i )
            {
                m_cache.DropPagesOfPack( change->ids[ i ] );
                m_cache.MarkForCheck( change->ids[ i ] );
            }
            break;
        }
        case ASSETSTORE::ASSETCHANGE_PACK_REMOVED:
        {
            for ( size_t i=0; i<change->ids.size(); ++i )
                m_cache.RemovePack( change->ids[ i ] );
            break;
        }
        default:
        {
            break;
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
