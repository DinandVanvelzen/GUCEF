/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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
#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */
#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#include "AssetWorkbenchLib_CServerSettingsController.h"

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

const CORE::CEvent CServerSettingsController::SettingsReadEvent = "GUCEF::ASSETWORKBENCH::CServerSettingsController::SettingsReadEvent";
const CORE::CEvent CServerSettingsController::OutcomeEvent = "GUCEF::ASSETWORKBENCH::CServerSettingsController::OutcomeEvent";

/*-------------------------------------------------------------------------*/

void
CServerSettingsController::RegisterEvents( void )
{GUCEF_TRACE;

    SettingsReadEvent.Initialize();
    OutcomeEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CServerSettingsController::SStoreProfile::SStoreProfile( void )
    : storeId()
    , displayName()
    , known( false )
    , profile()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServerSettingsController::SPending::SPending( void )
    : future( CORE::TASKSTATUS_UNDEFINED )
    , kind( PENDING_DESCRIBE )
    , storeId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServerSettingsController::CServerSettingsController( ASSETSTORE::CIAssetStoreService& service )
    : CORE::CObservingNotifier()
    , m_service( service )
    , m_description()
    , m_hasDescription( false )
    , m_profiles()
    , m_pending()
    , m_problem()
    , m_lastOutcome()
    , m_descriptionCount( 0 )
    , m_commandCounter( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    TEventCallback callback( this, &CServerSettingsController::OnSettingsChanged );
    SubscribeTo( &m_service, ASSETSTORE::CIAssetStoreService::SettingsChangedEvent, callback );
}

/*-------------------------------------------------------------------------*/

CServerSettingsController::~CServerSettingsController()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsController::Ask( const CORE::CFutureResult& future, EPendingKind kind, const CORE::CString& storeId )
{GUCEF_TRACE;

    if ( future.HasNoFuture() )
    {
        m_problem = "The asset store service could not be asked";
        return false;
    }
    SPending pending;
    pending.future = future;
    pending.kind = kind;
    pending.storeId = storeId;
    m_pending.push_back( pending );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsController::IsPending( EPendingKind kind ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_pending.size(); ++i )
        if ( m_pending[ i ].kind == kind )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerSettingsController::NextCommandId( void )
{GUCEF_TRACE;

    return "wbsettings-" + CORE::ToString( (UInt64) MT::PrecisionTickCount() ) + "-" + CORE::ToString( ++m_commandCounter );
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsController::Refresh( void )
{GUCEF_TRACE;

    m_problem.Clear();
    // one read of each at a time: a burst of changes reads the description once more after the reads under way
    bool asked = true;
    if ( !IsPending( PENDING_DESCRIBE ) )
        asked = Ask( m_service.DescribeSettingsAsync( ASSETSTORE::CAssetSettingsRequest() ), PENDING_DESCRIBE, CORE::CString::Empty ) && asked;
    if ( !IsPending( PENDING_STORES ) )
        asked = Ask( m_service.ListStoresAsync(), PENDING_STORES, CORE::CString::Empty ) && asked;
    return asked;
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsController::SetValue( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    m_problem.Clear();
    ASSETSTORE::CAssetSettingsRequest request;
    request.values[ key ] = value;
    return Ask( m_service.SetSettingsAsync( NextCommandId(), request ), PENDING_CHANGE, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsController::ResetValue( const CORE::CString& key )
{GUCEF_TRACE;

    m_problem.Clear();
    ASSETSTORE::CAssetSettingsRequest request;
    request.keys.push_back( key );
    return Ask( m_service.ResetSettingsAsync( NextCommandId(), request ), PENDING_CHANGE, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsController::SetProfileMember( const CORE::CString& storeId, const CORE::CString& member, const CORE::CString& value )
{GUCEF_TRACE;

    m_problem.Clear();
    const SStoreProfile* store = FindProfile( storeId );
    if ( GUCEF_NULL == store || !store->known )
    {
        m_problem = "The sync profile of the store " + storeId + " is not read yet";
        return false;
    }
    // the profile as it is, with the one member changed: the members the node does not have keep their value
    ASSETSTORE::CAssetLibrarySyncProfile profile = store->profile;
    CORE::CDataNode change( "profile", GUCEF_DATATYPE_OBJECT );
    change.SetAttribute( member, value );
    CORE::CDataNodeSerializableSettings settings;
    profile.Deserialize( change, settings );
    CORE::CString reason;
    if ( !profile.IsValid( reason ) )
    {
        m_problem = "The sync profile of " + storeId + " would not be valid: " + reason;
        return false;
    }
    return Ask( m_service.SetSyncProfileAsync( NextCommandId(), storeId, profile ), PENDING_PROFILE_CHANGE, storeId );
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsController::TakeOutcome( const CORE::CString& text )
{GUCEF_TRACE;

    m_lastOutcome = text;
    CORE::TCloneableString data( text );
    NotifyObservers( OutcomeEvent, &data );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServerSettingsController::DescribeOutcome( const ASSETSTORE::CAssetSettingsChangeResult& outcome )
{GUCEF_TRACE;

    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != outcome.status )
    {
        CORE::CString text = outcome.message.IsNULLOrEmpty() ? CORE::CString( "The change was refused" ) : outcome.message;
        // the reason per key is in the message of the service already; a record without a message gets them here
        if ( outcome.message.IsNULLOrEmpty() )
            for ( CORE::CString::StringMap::const_iterator e = outcome.errors.begin(); e != outcome.errors.end(); ++e )
                text += "; " + (*e).first + ": " + (*e).second;
        return text;
    }
    if ( outcome.noChange )
        return outcome.message.IsNULLOrEmpty() ? CORE::CString( "Nothing changed" ) : outcome.message;
    CORE::CString text;
    for ( size_t c=0; c<outcome.changes.size(); ++c )
    {
        const ASSETSTORE::CAssetSettingChange& change = outcome.changes[ c ];
        if ( ASSETSTORE::CAssetSettingChange::AppliedUnchanged == change.applied )
            continue;
        text += ( text.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( "; " ) ) + change.key +
                ( change.valuesShown ? " is " + change.newValue : CORE::CString( " changed" ) ) +
                ( ASSETSTORE::CAssetSettingChange::AppliedLive == change.applied ? ", applied at once" : ", applies after a restart of the service" );
    }
    return text;
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsController::Update( void )
{GUCEF_TRACE;

    // the answers that are there, in the order they were asked; the others wait for the next frame
    TPendingVector waiting;
    TPendingVector ready;
    for ( size_t i=0; i<m_pending.size(); ++i )
    {
        if ( m_pending[ i ].future.IsReady() || m_pending[ i ].future.HasNoFuture() )
            ready.push_back( m_pending[ i ] );
        else
            waiting.push_back( m_pending[ i ] );
    }
    m_pending = waiting;
    bool read = false;
    bool again = false;
    for ( size_t i=0; i<ready.size(); ++i )
    {
        const SPending& pending = ready[ i ];
        switch ( pending.kind )
        {
            case PENDING_DESCRIBE:
            {
                ASSETSTORE::CAssetSettingsDescription description;
                if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( pending.future, description ) )
                {
                    m_description = description;
                    m_hasDescription = true;
                    ++m_descriptionCount;
                    read = true;
                }
                else
                    m_problem = "The settings of the service could not be read: " + ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( pending.future );
                break;
            }
            case PENDING_STORES:
            {
                ASSETSTORE::CAssetStoreInfoPage page;
                if ( !ASSETSTORE::CIAssetStoreService::TryGetFutureResult( pending.future, page ) )
                    break;
                TStoreProfileVector profiles;
                for ( size_t s=0; s<page.entries.size(); ++s )
                {
                    SStoreProfile store;
                    store.storeId = page.entries[ s ].storeId;
                    store.displayName = page.entries[ s ].displayName.IsNULLOrEmpty() ? store.storeId : page.entries[ s ].displayName;
                    const SStoreProfile* known = FindProfile( store.storeId );
                    if ( GUCEF_NULL != known )
                    {
                        store.known = known->known;
                        store.profile = known->profile;
                    }
                    profiles.push_back( store );
                    Ask( m_service.GetSyncProfileAsync( store.storeId ), PENDING_PROFILE, store.storeId );
                }
                m_profiles = profiles;
                read = true;
                break;
            }
            case PENDING_PROFILE:
            {
                ASSETSTORE::CAssetLibrarySyncProfile profile;
                if ( !ASSETSTORE::CIAssetStoreService::TryGetFutureResult( pending.future, profile ) )
                    break;
                for ( size_t s=0; s<m_profiles.size(); ++s )
                {
                    if ( m_profiles[ s ].storeId != pending.storeId )
                        continue;
                    m_profiles[ s ].profile = profile;
                    m_profiles[ s ].known = true;
                    read = true;
                }
                break;
            }
            case PENDING_CHANGE:
            {
                // the record of a change comes also when it is refused: it has the status and the reason per key
                ASSETSTORE::CAssetSettingsChangeResult outcome;
                if ( !ASSETSTORE::CIAssetStoreService::TryGetFutureResult( pending.future, outcome ) )
                {
                    outcome.status = ASSETSTORE::CIAssetStoreService::GetStatusOfFuture( pending.future );
                    outcome.message = ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( pending.future );
                    if ( ASSETSTORE::ASSETSTORE_STATUS_OK == outcome.status )
                        outcome.status = ASSETSTORE::ASSETSTORE_STATUS_FAILED;
                }
                if ( ASSETSTORE::ASSETSTORE_STATUS_OK != outcome.status )
                    m_problem = DescribeOutcome( outcome );
                TakeOutcome( DescribeOutcome( outcome ) );
                again = true;
                break;
            }
            case PENDING_PROFILE_CHANGE:
            default:
            {
                const ASSETSTORE::TAssetStoreStatus status = ASSETSTORE::CIAssetStoreService::GetStatusOfFuture( pending.future );
                if ( ASSETSTORE::ASSETSTORE_STATUS_OK == status )
                {
                    TakeOutcome( "The sync profile of " + pending.storeId + " is changed" );
                    Ask( m_service.GetSyncProfileAsync( pending.storeId ), PENDING_PROFILE, pending.storeId );
                }
                else
                {
                    m_problem = "The sync profile of " + pending.storeId + " was not changed: " + ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( pending.future );
                    TakeOutcome( m_problem );
                    again = true;
                }
                break;
            }
        }
    }
    if ( again && !IsPending( PENDING_DESCRIBE ) )
        Ask( m_service.DescribeSettingsAsync( ASSETSTORE::CAssetSettingsRequest() ), PENDING_DESCRIBE, CORE::CString::Empty );
    if ( read )
        NotifyObservers( SettingsReadEvent );
}

/*-------------------------------------------------------------------------*/

void
CServerSettingsController::OnSettingsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a change by any client of the service, this one included: the description is read again
    if ( !IsPending( PENDING_DESCRIBE ) )
        Ask( m_service.DescribeSettingsAsync( ASSETSTORE::CAssetSettingsRequest() ), PENDING_DESCRIBE, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsController::IsWaiting( void ) const
{GUCEF_TRACE;

    return !m_pending.empty();
}

/*-------------------------------------------------------------------------*/

bool
CServerSettingsController::HasDescription( void ) const
{GUCEF_TRACE;

    return m_hasDescription;
}

/*-------------------------------------------------------------------------*/

const ASSETSTORE::CAssetSettingsDescription&
CServerSettingsController::GetDescription( void ) const
{GUCEF_TRACE;

    return m_description;
}

/*-------------------------------------------------------------------------*/

const CServerSettingsController::TStoreProfileVector&
CServerSettingsController::GetProfiles( void ) const
{GUCEF_TRACE;

    return m_profiles;
}

/*-------------------------------------------------------------------------*/

const CServerSettingsController::SStoreProfile*
CServerSettingsController::FindProfile( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    for ( size_t s=0; s<m_profiles.size(); ++s )
        if ( m_profiles[ s ].storeId == storeId )
            return &m_profiles[ s ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServerSettingsController::GetProblem( void ) const
{GUCEF_TRACE;

    return m_problem;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServerSettingsController::GetLastOutcome( void ) const
{GUCEF_TRACE;

    return m_lastOutcome;
}

/*-------------------------------------------------------------------------*/

UInt32
CServerSettingsController::GetDescriptionCount( void ) const
{GUCEF_TRACE;

    return m_descriptionCount;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServerSettingsController::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CServerSettingsController";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
