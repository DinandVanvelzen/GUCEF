/*
 *  AssetWorkbenchLib: what to open for an item of an online store or a pack of the library
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

#ifndef GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#include "gucefASSETSTORE_CAssetNameHints.h"
#define GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#endif /* GUCEF_ASSETSTORE_CASSETNAMEHINTS_H ? */

#include "AssetWorkbenchLib_CStoreItemOpenLookup.h"

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

CStoreItemOpenLookup::CStoreItemOpenLookup( void )
    : m_service( GUCEF_NULL )
    , m_state( STATE_IDLE )
    , m_forItem( false )
    , m_storeId()
    , m_itemId()
    , m_itemTitle()
    , m_candidates()
    , m_nextCandidate( 0 )
    , m_packId()
    , m_pack()
    , m_assetId()
    , m_asset()
    , m_message()
    , m_future( CORE::TASKSTATUS_UNDEFINED )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CStoreItemOpenLookup::StartForItem( ASSETSTORE::CIAssetStoreService* service, const CORE::CString& storeId, const CORE::CString& itemId, const CORE::CString& itemTitle )
{GUCEF_TRACE;

    Cancel();
    if ( GUCEF_NULL == service || storeId.IsNULLOrEmpty() || itemId.IsNULLOrEmpty() )
        return false;
    m_service = service;
    m_forItem = true;
    m_storeId = storeId;
    m_itemId = itemId;
    m_itemTitle = itemTitle;

    // the packs of the store that came from the item; a service that does not know the filter of the item gives the packs of
    // the store, which the check of the candidates sorts out
    ASSETSTORE::CAssetSearchQuery query;
    query.resultKind = "packs";
    query.sort = "newest";
    query.sources.insert( storeId.Lowercase() );
    query.sourceItemIds.insert( itemId );
    query.pageSize = MaxCandidates;
    query.withFacets = false;
    m_future = m_service->SearchAsync( query );
    m_state = STATE_FINDING_PACK;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStoreItemOpenLookup::StartForPack( ASSETSTORE::CIAssetStoreService* service, const CORE::CString& packId, const CORE::CString& packTitle )
{GUCEF_TRACE;

    Cancel();
    if ( GUCEF_NULL == service || packId.IsNULLOrEmpty() )
        return false;
    m_service = service;
    m_forItem = false;
    m_packId = packId;
    m_itemTitle = packTitle;
    m_future = m_service->GetPackAsync( packId );
    m_candidates.push_back( packId );
    m_nextCandidate = 1;
    m_state = STATE_CHECKING_PACK;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CStoreItemOpenLookup::Cancel( void )
{GUCEF_TRACE;

    m_service = GUCEF_NULL;
    m_state = STATE_IDLE;
    m_forItem = false;
    m_storeId.Clear();
    m_itemId.Clear();
    m_itemTitle.Clear();
    m_candidates.clear();
    m_nextCandidate = 0;
    m_packId.Clear();
    m_pack = ASSETSTORE::CAssetPackRecord();
    m_assetId.Clear();
    m_asset = ASSETSTORE::CAssetRecord();
    m_message.Clear();
    m_future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
}

/*-------------------------------------------------------------------------*/

void
CStoreItemOpenLookup::Fail( const CORE::CString& message )
{GUCEF_TRACE;

    m_state = STATE_FAILED;
    m_message = message;
    m_future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
}

/*-------------------------------------------------------------------------*/

void
CStoreItemOpenLookup::CheckNextCandidate( void )
{GUCEF_TRACE;

    if ( m_nextCandidate >= (UInt32) m_candidates.size() )
    {
        m_state = STATE_NOT_IN_LIBRARY;
        m_future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        return;
    }
    m_future = m_service->GetPackAsync( m_candidates[ m_nextCandidate++ ] );
    m_state = STATE_CHECKING_PACK;
}

/*-------------------------------------------------------------------------*/

void
CStoreItemOpenLookup::AskAssets( void )
{GUCEF_TRACE;

    m_future = m_service->GetAssetsAsync( m_packId, CORE::CString::Empty, MaxAssets );
    m_state = STATE_FINDING_ASSET;
}

/*-------------------------------------------------------------------------*/

bool
CStoreItemOpenLookup::Update( void )
{GUCEF_TRACE;

    // one call is out while the lookup is busy; a call that failed has no future any more (HasAFuture() is false) and is taken all the same
    if ( !IsBusy() || GUCEF_NULL == m_service || !m_future.IsReady() )
        return false;

    const ASSETSTORE::TAssetStoreStatus status = ASSETSTORE::CIAssetStoreService::GetStatusOfFuture( m_future );
    switch ( m_state )
    {
        case STATE_FINDING_PACK:
        {
            ASSETSTORE::CAssetSearchResultPage page;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status || !ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_future, page ) )
            {
                Fail( "The library cannot be searched for " + m_itemTitle + ": " + ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( m_future ) );
                return true;
            }
            m_candidates.clear();
            for ( size_t i=0; i<page.hits.size() && m_candidates.size()<MaxCandidates; ++i )
                if ( !page.hits[ i ].packId.IsNULLOrEmpty() )
                    m_candidates.push_back( page.hits[ i ].packId );
            m_nextCandidate = 0;
            CheckNextCandidate();
            return STATE_NOT_IN_LIBRARY == m_state;
        }
        case STATE_CHECKING_PACK:
        {
            ASSETSTORE::CAssetPackRecord pack;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status || !ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_future, pack ) )
            {
                if ( !m_forItem )
                {
                    Fail( "The pack " + m_itemTitle + " cannot be read from the store: " + ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( m_future ) );
                    return true;
                }
                // a candidate that went meanwhile: the next one
                CheckNextCandidate();
                return STATE_NOT_IN_LIBRARY == m_state;
            }
            if ( !m_forItem && pack.packId.IsNULLOrEmpty() )
            {
                Fail( "The pack " + m_itemTitle + " is not in the library of the store" );
                return true;
            }
            if ( m_forItem && ( pack.packId.IsNULLOrEmpty() || pack.sourceItemId != m_itemId || pack.sourceId.Lowercase() != m_storeId.Lowercase() ) )
            {
                CheckNextCandidate();
                return STATE_NOT_IN_LIBRARY == m_state;
            }
            m_pack = pack;
            m_packId = pack.packId;
            if ( m_itemTitle.IsNULLOrEmpty() )
                m_itemTitle = pack.title;
            AskAssets();
            return false;
        }
        case STATE_FINDING_ASSET:
        {
            ASSETSTORE::CAssetRecordPage assets;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status || !ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_future, assets ) )
            {
                Fail( "The assets of the pack " + m_pack.title + " cannot be read from the store: " + ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( m_future ) );
                return true;
            }
            const Int32 chosen = ChooseAssetToShow( assets.entries );
            if ( chosen >= 0 )
            {
                m_asset = assets.entries[ (size_t) chosen ];
                m_assetId = m_asset.assetId;
            }
            m_state = STATE_FOUND;
            m_future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
            return true;
        }
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CStoreItemOpenLookup::ChooseAssetToShow( const ASSETSTORE::TAssetRecordVector& assets )
{GUCEF_TRACE;

    Int32 convertedModel = -1;
    Int32 model = -1;
    for ( size_t i=0; i<assets.size(); ++i )
    {
        if ( !ASSETSTORE::CAssetNameHints::IsModelKind( assets[ i ].kind ) )
            continue;
        if ( !assets[ i ].convertedPath.IsNULLOrEmpty() && ( convertedModel < 0 || assets[ i ].name < assets[ (size_t) convertedModel ].name ) )
            convertedModel = (Int32) i;
        if ( model < 0 || assets[ i ].name < assets[ (size_t) model ].name )
            model = (Int32) i;
    }
    if ( convertedModel >= 0 )
        return convertedModel;
    if ( model >= 0 )
        return model;
    return assets.empty() ? -1 : 0;
}

/*-------------------------------------------------------------------------*/

CStoreItemOpenLookup::TState CStoreItemOpenLookup::GetState( void ) const { return m_state; }
bool CStoreItemOpenLookup::IsBusy( void ) const { return STATE_FINDING_PACK == m_state || STATE_CHECKING_PACK == m_state || STATE_FINDING_ASSET == m_state; }
bool CStoreItemOpenLookup::IsForItem( void ) const { return m_forItem; }
const CORE::CString& CStoreItemOpenLookup::GetStoreId( void ) const { return m_storeId; }
const CORE::CString& CStoreItemOpenLookup::GetItemId( void ) const { return m_itemId; }
const CORE::CString& CStoreItemOpenLookup::GetItemTitle( void ) const { return m_itemTitle; }
const CORE::CString& CStoreItemOpenLookup::GetPackId( void ) const { return m_packId; }
const ASSETSTORE::CAssetPackRecord& CStoreItemOpenLookup::GetPack( void ) const { return m_pack; }
const CORE::CString& CStoreItemOpenLookup::GetAssetId( void ) const { return m_assetId; }
const ASSETSTORE::CAssetRecord& CStoreItemOpenLookup::GetAsset( void ) const { return m_asset; }
const CORE::CString& CStoreItemOpenLookup::GetProblem( void ) const { return m_message; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
