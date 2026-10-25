/*
 *  lobbyservicepluginEOS: gucefMULTIPLAY lobby service plugin for lobbies on Epic Online Services
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

#include <string.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#ifndef GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#include "gucefMULTIPLAY_CLaunchDescription.h"
#define GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#endif /* GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H
#include "lobbyservicepluginEOS_CEosLobbyService.h"
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H ? */

#include "lobbyservicepluginEOS_CEosLobby.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace EOS {

using namespace PLUGINGLUE::EOS;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
NoSeparator( const CORE::CString& text )
{GUCEF_TRACE;

    return CEosLobbyCodec::SanitizeText( text, 200 ).ReplaceChar( '|', ' ' );
}

/*-------------------------------------------------------------------------*/

static bool
ProofsEqual( const CORE::CString& a, const CORE::CString& b )
{GUCEF_TRACE;

    // Time that does not depend on where they differ
    if ( a.ByteSize() != b.ByteSize() )
        return false;
    return CORE::CSha256::ConstantTimeEquals( a.Lowercase().C_String(), b.Lowercase().C_String(), a.ByteSize() - 1 );
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobby::GetMaxMembers( const CEosLobbyServiceSettings& settings, const MULTIPLAY::CLobbyCreateParams& params )
{GUCEF_TRACE;

    UInt32 members = params.maxPlayers + ( params.allowSpectators ? settings.spectatorSlots : 0 );
    if ( members > settings.limits.maxLobbyMembers )
        members = settings.limits.maxLobbyMembers;
    if ( members < 2 )
        members = 2;
    return members;
}

/*-------------------------------------------------------------------------*/

EEosLobbyPermission
CEosLobby::GetPermission( const CEosLobbyServiceSettings& settings, bool isPublic, bool visible )
{GUCEF_TRACE;

    if ( !visible )
        return EOSLOBBYPERM_INVITE_ONLY;
    if ( "public" == settings.permission )
        return EOSLOBBYPERM_PUBLIC_ADVERTISED;
    if ( "presence" == settings.permission )
        return EOSLOBBYPERM_JOIN_VIA_PRESENCE;
    if ( "invite" == settings.permission )
        return EOSLOBBYPERM_INVITE_ONLY;
    return isPublic ? EOSLOBBYPERM_PUBLIC_ADVERTISED : EOSLOBBYPERM_INVITE_ONLY;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::Validate( const CEosLobbyService& service, const MULTIPLAY::CLobbyCreateParams& params, CORE::CString& reason )
{GUCEF_TRACE;

    // The lobby is built without being made: what does not fit the attributes of a lobby is found out before EOS is asked for one
    CEosLobby probe( const_cast< CEosLobbyService& >( service ), "validate", "validate", false );
    return probe.PrepareOwner( params, CORE::CString::Empty, reason );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::PrepareOwner( const MULTIPLAY::CLobbyCreateParams& params, const CORE::CString& password, CORE::CString& reason )
{GUCEF_TRACE;

    const CEosLobbyServiceSettings& settings = m_service->GetSettings();

    CORE::CString name = CEosLobbyCodec::SanitizeText( params.name, 120 );
    if ( name.IsNULLOrEmpty() )
    {
        reason = "A lobby needs a name";
        return false;
    }
    if ( params.maxPlayers < 1 || params.maxPlayers > settings.limits.maxLobbyMembers )
    {
        reason = "The number of players has to be 1 to " + CORE::UInt32ToString( settings.limits.maxLobbyMembers );
        return false;
    }
    if ( params.gameId.ByteSize() > 65 || params.gameVersion.ByteSize() > 65 )
    {
        reason = "The game id and version are limited to 64 bytes";
        return false;
    }

    m_params = params;
    m_params.name = name;
    m_params.password.Clear();
    m_schema = params.schema;
    m_password = password;
    m_isOwner = true;
    m_ownerUserId = m_localUser;

    m_rules = settings.rules;
    m_rules.maxPlayers = params.maxPlayers;
    m_rules.allowSpectators = m_rules.allowSpectators && params.allowSpectators;
    m_rules.closeWhenHostLeaves = false;

    m_state = MULTIPLAY::CLobbyState();
    m_state.lobbyId = m_lobbyId;
    m_state.name = name;
    m_state.phase = MULTIPLAY::LOBBYPHASE_OPEN;
    m_state.slots = params.slots;
    if ( m_state.slots.empty() )
    {
        for ( UInt32 i=0; i<params.maxPlayers; ++i )
        {
            MULTIPLAY::CLobbySlot slot;
            slot.index = i;
            slot.team = i;
            m_state.slots.push_back( slot );
        }
    }
    params.GetStartSettings( m_state.settings );

    if ( !password.IsNULLOrEmpty() )
    {
        m_summary.passwordSalt = CEosLobbyCodec::MakeSalt();
        if ( m_summary.passwordSalt.IsNULLOrEmpty() )
        {
            reason = "There is no randomness to make the salt of the password with";
            return false;
        }
        m_summary.passworded = true;
    }

    CORE::CString hostName = CEosLobbyCodec::SanitizeText( m_service->GetIdentity().displayName, 64 );
    if ( hostName.IsNULLOrEmpty() )
        hostName = "Player";
    m_rules.ApplyJoin( m_state, m_localUser, hostName, false );
    m_lastGoodState = m_state;
    m_hasDefinition = true;
    m_visible = true;

    // Does it fit?
    TEosAttributeVector attributes;
    if ( !HostBuildAttributes( attributes, reason, 1 ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::StartAsOwner( const MULTIPLAY::CLobbyCreateParams& params, const CORE::CString& password, CORE::CString& reason )
{GUCEF_TRACE;

    if ( !PrepareOwner( params, password, reason ) )
        return false;

    m_stage = STAGE_STARTING;
    m_forceFull = true;
    m_revision = 0;
    m_updateBucket.SetRate( 0, 0 );

    // The lobby was made hidden (invite only) and without data: the data and the permission level it was meant to have go in one update
    SEosLobbyUpdateOptions options;
    options.lobbyId = m_lobbyId;
    options.localUser = m_localUser;
    TEosAttributeVector attributes;
    ++m_revision;
    if ( !HostBuildAttributes( attributes, reason, m_revision ) )
        return false;
    options.setAttributes = attributes;
    options.setPermission = true;
    options.permission = GetPermission( m_service->GetSettings(), m_params.isPublic, true );
    options.setMemberAttributes.push_back( CEosLobbyCodec::MakeString( CEosLobbyCodec::MemberKeyName, m_state.members[ 0 ].name ) );
    options.setMemberAttributes.push_back( CEosLobbyCodec::MakeInt( CEosLobbyCodec::KeyVersion, CEosLobbyCodec::WireVersion ) );

    UInt32 requestId = m_service->RequestUpdate( m_lobbyId, options );
    if ( 0 == requestId )
    {
        reason = "The lobby could not be published";
        return false;
    }
    m_updateRequestId = requestId;
    m_updateInFlight = true;
    m_wireAttributes = attributes;
    m_forceFull = false;
    ++m_updateCount;
    m_updateBucket.SetRate( m_service->GetSettings().updatesPerSecond, m_service->GetSettings().updateBurst );
    m_lastSecondInMs = PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs();
    m_joinDeadlineInMs = m_lastSecondInMs + m_service->GetSettings().requestTimeoutInMs;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::HostDefinitionAttributes( TEosAttributeVector& attributes, CORE::CString& reason )
{GUCEF_TRACE;

    // The definition of a lobby does not change: made once. It is what a member needs to take the lobby over
    if ( m_definitionAttributes.empty() )
    {
        CORE::CDataNode tree( "df" );
        CORE::CDataNode* paramsNode = tree.AddChild( "params", GUCEF_DATATYPE_OBJECT );
        MULTIPLAY::CLobbyCreateParams publicParams( m_params );
        publicParams.password.Clear();
        publicParams.SaveConfig( *paramsNode );
        CORE::CDataNode* rulesNode = tree.AddChild( "rules", GUCEF_DATATYPE_OBJECT );
        m_rules.SaveConfig( *rulesNode );
        TEosAttributeVector made;
        if ( !CEosLobbyCodec::EncodeBlob( CEosLobbyCodec::BlobDefinition, tree, m_service->GetSettings().limits, made, reason ) )
            return false;
        m_definitionAttributes = made;
    }
    attributes.insert( attributes.end(), m_definitionAttributes.begin(), m_definitionAttributes.end() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::HostBuildAttributes( TEosAttributeVector& attributes, CORE::CString& reason, UInt32 revision )
{GUCEF_TRACE;

    const CEosLobbyServiceSettings& settings = m_service->GetSettings();
    const CEosLobbyLimits& limits = settings.limits;

    CEosLobbySummary summary;
    summary.stamp = m_localUser;
    summary.name = m_state.name;
    const MULTIPLAY::CLobbyMember* host = m_state.FindMember( m_localUser );
    summary.hostName = GUCEF_NULL != host ? host->name : CORE::CString::Empty;
    summary.gameId = m_params.gameId;
    summary.gameVersion = m_params.gameVersion;
    summary.topology = m_params.topology;
    summary.passworded = !m_summary.passwordSalt.IsNULLOrEmpty();
    summary.passwordSalt = m_summary.passwordSalt;
    summary.allowSpectators = m_rules.allowSpectators;
    summary.maxPlayers = m_rules.maxPlayers;
    summary.spectatorSlots = m_rules.allowSpectators ? settings.spectatorSlots : 0;
    summary.phase = m_state.phase;
    summary.players = m_state.GetPlayerCount();
    summary.spectators = m_state.GetSpectatorCount();
    summary.isPublic = m_visible && m_params.isPublic;
    summary.revision = revision;
    CEosLobbyCodec::EncodeSummary( summary, attributes );

    // The settings that are searched on
    for ( size_t i=0; i<settings.mirrorSettings.size(); ++i )
    {
        const CORE::CString& key = settings.mirrorSettings[ i ];
        CORE::CString attributeKey = CEosLobbyCodec::MirrorPrefix + key;
        if ( !m_state.settings.HasKey( key ) )
            continue;
        if ( !CEosLobbyCodec::IsValidKey( attributeKey, limits ) )
        {
            reason = "The setting " + key + " cannot be mirrored as an attribute";
            return false;
        }
        attributes.push_back( CEosLobbyCodec::MakeString( attributeKey, CEosLobbyCodec::SanitizeText( m_state.settings.GetValueAlways( key ).AsString( CORE::CString::Empty, true ), 120 ) ) );
    }

    if ( !HostDefinitionAttributes( attributes, reason ) )
        return false;

    // The state, and with it the notes to the members
    CORE::CDataNode stateTree( "st" );
    stateTree.SetAttribute( "rv", revision );
    if ( m_countdownLeft >= 0 )
        stateTree.SetAttribute( "cd", m_countdownLeft );
    if ( !m_closeReason.IsNULLOrEmpty() )
        stateTree.SetAttribute( "cr", m_closeReason );
    CORE::CDataNode* stateNode = stateTree.AddChild( "state", GUCEF_DATATYPE_OBJECT );
    m_state.Serialize( *stateNode, CORE::CDataNodeSerializableSettings() );
    if ( !m_lastEndedVote.id.IsNULLOrEmpty() )
    {
        CORE::CDataNode* voteEnd = stateTree.AddChild( "ve", GUCEF_DATATYPE_OBJECT );
        voteEnd->SetAttribute( "id", m_lastEndedVote.id );
        voteEnd->SetAttribute( "what", m_lastEndedVote.what );
        voteEnd->SetAttribute( "proposer", m_lastEndedVote.proposer );
        voteEnd->SetAttribute( "key", m_lastEndedVote.key );
        voteEnd->SetAttribute( "value", m_lastEndedVote.value );
        voteEnd->SetAttribute( "yes", m_lastEndedVote.yes );
        voteEnd->SetAttribute( "no", m_lastEndedVote.no );
        voteEnd->SetAttribute( "passed", m_lastEndedPassed );
    }
    if ( !m_acks.empty() )
    {
        CORE::CDataNode* acks = stateTree.AddChild( "ak", GUCEF_DATATYPE_OBJECT );
        for ( TSequenceMap::const_iterator i = m_acks.begin(); i != m_acks.end(); ++i )
            acks->SetAttribute( (*i).first, (*i).second );
    }
    if ( !m_refusals.empty() )
    {
        CORE::CDataNode* refusals = stateTree.AddChild( "rf", GUCEF_DATATYPE_OBJECT );
        for ( TTextMap::const_iterator i = m_refusals.begin(); i != m_refusals.end(); ++i )
            refusals->SetAttribute( (*i).first, (*i).second );
    }
    if ( !m_rejections.empty() )
    {
        CORE::CDataNode* rejections = stateTree.AddChild( "rj", GUCEF_DATATYPE_OBJECT );
        for ( TTextMap::const_iterator i = m_rejections.begin(); i != m_rejections.end(); ++i )
            rejections->SetAttribute( (*i).first, (*i).second );
    }
    if ( !CEosLobbyCodec::EncodeBlob( CEosLobbyCodec::BlobState, stateTree, limits, attributes, reason ) )
        return false;

    if ( m_haveLaunch )
    {
        CORE::CDataNode launchTree( "la" );
        MULTIPLAY::CLaunchDescription publicLaunch( m_launch );
        publicLaunch.token.Clear();
        publicLaunch.localMemberId.Clear();
        publicLaunch.Serialize( launchTree, CORE::CDataNodeSerializableSettings() );
        if ( !CEosLobbyCodec::EncodeBlob( CEosLobbyCodec::BlobLaunch, launchTree, limits, attributes, reason ) )
            return false;
    }

    if ( attributes.size() > limits.maxLobbyAttributes )
    {
        reason = "The lobby would need " + CORE::UInt32ToString( (UInt32) attributes.size() ) + " attributes, EOS lobbies have " + CORE::UInt32ToString( limits.maxLobbyAttributes );
        return false;
    }
    for ( size_t i=0; i<attributes.size(); ++i )
    {
        if ( !CEosLobbyCodec::IsValidKey( attributes[ i ].key, limits ) || attributes[ i ].stringValue.ByteSize() > limits.maxAttributeLength + 1 )
        {
            reason = "The attribute " + attributes[ i ].key + " does not fit";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::HostCommit( const CORE::CString& path, const CORE::CString& requester, const CORE::CString& what )
{GUCEF_TRACE;

    // What the change would publish has to fit the lobby; if not it is not made
    TEosAttributeVector probe;
    CORE::CString reason;
    if ( !HostBuildAttributes( probe, reason, m_revision + 1 ) )
    {
        m_state = m_lastGoodState;
        if ( !requester.IsNULLOrEmpty() )
            HostRefuse( requester, what, "That does not fit the lobby: " + reason, 0 );
        return false;
    }
    m_lastGoodState = m_state;
    m_dirty = true;
    if ( !path.IsNULLOrEmpty() )
        StateChanged( path );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::HostPublish( void )
{GUCEF_TRACE;

    if ( m_updateInFlight || GUCEF_NULL == m_service )
        return false;

    ++m_revision;
    TEosAttributeVector attributes;
    CORE::CString reason;
    if ( !HostBuildAttributes( attributes, reason, m_revision ) )
    {
        --m_revision;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosLobby: The lobby cannot be published: " << reason );
        m_dirty = false;
        return false;
    }

    SEosLobbyUpdateOptions options;
    options.lobbyId = m_lobbyId;
    options.localUser = m_localUser;
    for ( size_t i=0; i<attributes.size(); ++i )
    {
        const SEosAttribute* known = m_forceFull ? GUCEF_NULL : CEosLobbyCodec::FindAttribute( m_wireAttributes, attributes[ i ].key );
        bool same = GUCEF_NULL != known && known->type == attributes[ i ].type && known->stringValue == attributes[ i ].stringValue && known->intValue == attributes[ i ].intValue;
        if ( !same )
            options.setAttributes.push_back( attributes[ i ] );
    }
    // What was published and is no longer there, such as the pieces of data that became smaller
    for ( size_t i=0; i<m_wireAttributes.size(); ++i )
        if ( GUCEF_NULL == CEosLobbyCodec::FindAttribute( attributes, m_wireAttributes[ i ].key ) )
            options.removeAttributes.push_back( m_wireAttributes[ i ].key );
    if ( m_visibilityChange )
    {
        options.setPermission = true;
        options.permission = GetPermission( m_service->GetSettings(), m_params.isPublic, m_visible );
    }

    UInt32 requestId = m_service->RequestUpdate( m_lobbyId, options );
    if ( 0 == requestId )
    {
        --m_revision;
        return false;
    }
    m_updateRequestId = requestId;
    m_updateInFlight = true;
    m_dirty = false;
    m_forceFull = false;
    m_visibilityChange = false;
    m_wireAttributes = attributes;
    m_publishedRevision = m_revision;
    ++m_updateCount;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostOnUpdateEnded( EEosResult result )
{GUCEF_TRACE;

    m_updateInFlight = false;
    UInt64 nowInMs = PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs();

    if ( EOSRESULT_SUCCESS != result )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "EosLobby: The update of the lobby failed: " << EosResultToString( result ) );
        m_forceFull = true;
        ++m_updateFailures;
        if ( STAGE_STARTING == m_stage )
        {
            m_service->OnLobbyStarted( this, false, CORE::CString( "The lobby could not be published: " ) + EosResultToString( result ) );
            return;
        }
        if ( m_updateFailures < 3 )
        {
            m_dirty = true;
            m_nextPublishInMs = nowInMs + 1000 * m_updateFailures;
        }
        else
        {
            // It stays as it was published last; the next change tries again
            m_updateFailures = 0;
            m_dirty = false;
            // Whoever waits for a publication does not wait for ever
            m_confirmedRevision = m_publishedRevision;
        }
    }
    else
    {
        m_updateFailures = 0;
        m_confirmedRevision = m_publishedRevision;
        if ( STAGE_STARTING == m_stage )
        {
            m_stage = STAGE_ACTIVE;
            m_service->OnLobbyStarted( this, true, CORE::CString::Empty );
            if ( STAGE_DETACHED == m_stage )
                return;
        }
    }

    // The launch is published: the game is under way
    if ( m_haveLaunch && !m_launchPublished && EOSRESULT_SUCCESS == result && MULTIPLAY::LOBBYPHASE_LAUNCHING == m_state.phase )
    {
        m_launchPublished = true;
        m_state.phase = MULTIPLAY::LOBBYPHASE_IN_GAME;
        HostCommit( "phase", CORE::CString::Empty, CORE::CString::Empty );
    }

    HostProcessQueues( nowInMs );

    if ( m_closing && !m_updateInFlight && ( EOSRESULT_SUCCESS != result || m_confirmedRevision >= m_closeRevision ) )
    {
        // The members get time to read why before the lobby is gone
        UInt32 grace = m_service->GetSettings().departureGraceInMs;
        if ( EOSRESULT_SUCCESS == result && 0 != grace )
        {
            if ( 0 == m_closeDestroyAtInMs )
                m_closeDestroyAtInMs = nowInMs + grace;
        }
        else
        {
            HostFinishClose();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostFinishClose( void )
{GUCEF_TRACE;

    m_service->RequestDestroy( m_lobbyId );
    m_closing = false;
    m_closeDestroyAtInMs = 0;
    Detach( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostProcessQueues( UInt64 nowInMs )
{GUCEF_TRACE;

    // Whoever is to be kicked is kicked when the publication that tells why has been made and the member had time to leave by itself
    for ( size_t i=0; i<m_kickQueue.size(); )
    {
        if ( m_confirmedRevision >= m_kickQueue[ i ].minRevision && nowInMs >= m_kickQueue[ i ].notBeforeInMs && !m_updateInFlight )
        {
            m_service->RequestKick( m_lobbyId, m_kickQueue[ i ].userId );
            m_kickQueue.erase( m_kickQueue.begin() + i );
        }
        else
        {
            ++i;
        }
    }

    // The boss is handed over when the state that says so is out
    if ( !m_promoteTarget.IsNULLOrEmpty() && 0 == m_promoteRequestId && !m_updateInFlight && !m_dirty )
    {
        m_promoteRequestId = m_service->RequestPromote( m_lobbyId, m_promoteTarget );
        if ( 0 == m_promoteRequestId )
        {
            m_rules.ApplyTransferBoss( m_state, m_localUser );
            HostCommit( "boss", m_localUser, "transferBoss" );
            Refuse( "transferBoss", "EOS did not take the request to hand the lobby over" );
        }
        m_promoteTarget.Clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostTick( UInt64 nowInMs )
{GUCEF_TRACE;

    const CEosLobbyServiceSettings& settings = m_service->GetSettings();

    // The seconds of the countdown and of the votes
    UInt32 guard = 0;
    while ( 0 != m_lastSecondInMs && nowInMs >= m_lastSecondInMs + settings.secondInMs && guard < 5 )
    {
        m_lastSecondInMs += settings.secondInMs;
        HostSecond();
        ++guard;
        if ( STAGE_DETACHED == m_stage )
            return;
    }
    if ( nowInMs > m_lastSecondInMs + 5 * settings.secondInMs )
        m_lastSecondInMs = nowInMs;

    if ( m_closing && 0 != m_closeDestroyAtInMs && nowInMs >= m_closeDestroyAtInMs )
    {
        HostFinishClose();
        return;
    }

    // Members that joined and said nothing in time
    for ( size_t i=0; i<m_pendingJoins.size(); )
    {
        if ( nowInMs >= m_pendingJoins[ i ].deadlineInMs )
        {
            CORE::CString userId = m_pendingJoins[ i ].userId;
            HostEvaluateJoin( userId, true );
            if ( i < m_pendingJoins.size() && m_pendingJoins[ i ].userId == userId )
                m_pendingJoins.erase( m_pendingJoins.begin() + i );
        }
        else
        {
            ++i;
        }
    }

    if ( m_dirty && !m_updateInFlight && nowInMs >= m_nextPublishInMs && STAGE_ACTIVE == m_stage )
    {
        if ( m_updateBucket.Take( nowInMs ) )
            HostPublish();
    }
    HostProcessQueues( nowInMs );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostRefuse( const CORE::CString& memberId, const CORE::CString& what, const CORE::CString& reason, UInt32 sequence )
{GUCEF_TRACE;

    if ( memberId == m_localUser )
    {
        Refuse( what, reason );
        return;
    }
    m_refusals[ memberId ] = CORE::UInt32ToString( sequence ) + "|" + NoSeparator( what ) + "|" + NoSeparator( reason );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostRequestLocal( const CORE::CString& what, const CORE::CString& verb, const CORE::CString& arg1, const CORE::CString& arg2 )
{GUCEF_TRACE;

    HostHandle( m_localUser, verb, arg1, arg2, 0 );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostSetReady( const CORE::CString& memberId, bool ready, UInt32 sequence )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckReady( m_state, memberId, reason ) )
    {
        HostRefuse( memberId, "ready", reason, sequence );
        return;
    }
    bool countdownCancelled = false;
    m_rules.ApplyReady( m_state, memberId, ready, countdownCancelled );
    HostCommit( "members", memberId, "ready" );
    if ( countdownCancelled )
        HostCancelCountdown();
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostHandle( const CORE::CString& memberId, const CORE::CString& verb, const CORE::CString& arg1, const CORE::CString& arg2, UInt32 sequence )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_state.FindMember( memberId ) )
    {
        HostRefuse( memberId, verb, "Not a member of the lobby", sequence );
        return;
    }
    CORE::CString reason;

    if ( "set" == verb )
    {
        bool needsVote = false;
        if ( !m_rules.CheckSetting( m_state, m_schema, memberId, arg1, arg2, needsVote, reason ) )
            HostRefuse( memberId, "setting", reason, sequence );
        else
        if ( needsVote )
            HostStartVote( memberId, "setting", arg1, arg2 );
        else
        {
            m_state.SetSetting( arg1, arg2 );
            HostCommit( "settings/" + arg1, memberId, "setting" );
        }
    }
    else
    if ( "prop" == verb )
    {
        if ( !m_rules.CheckProposal( m_state, m_schema, memberId, arg1, arg2, reason ) )
            HostRefuse( memberId, "proposal", reason, sequence );
        else
            HostStartVote( memberId, "setting", arg1, arg2 );
    }
    else
    if ( "vote" == verb )
    {
        if ( !m_rules.CheckVote( m_state, memberId, arg1, reason ) )
            HostRefuse( memberId, "vote", reason, sequence );
        else
        {
            if ( "1" == arg2 )
                ++m_state.activeVote.yes;
            else
                ++m_state.activeVote.no;
            m_state.activeVote.voters.insert( memberId );
            HostCommit( "vote", memberId, "vote" );
            HostEvaluateVote();
        }
    }
    else
    if ( "spec" == verb )
    {
        bool spectate = "1" == arg1;
        if ( !m_rules.CheckSpectator( m_state, memberId, spectate, reason ) )
            HostRefuse( memberId, "spectator", reason, sequence );
        else
        {
            m_rules.ApplySpectator( m_state, memberId, spectate );
            if ( HostCommit( "members", memberId, "spectator" ) )
                StateChanged( "slots" );
        }
    }
    else
    if ( "slot" == verb )
    {
        UInt32 slotIndex = CORE::StringToUInt32( arg1, 0xFFFFFFFF );
        if ( !m_rules.CheckSlotRequest( m_state, memberId, slotIndex, reason ) )
            HostRefuse( memberId, "slot", reason, sequence );
        else
        {
            m_rules.ApplySlot( m_state, memberId, slotIndex );
            HostCommit( "slots", memberId, "slot" );
        }
    }
    else
    if ( "fac" == verb )
    {
        if ( !m_rules.CheckFaction( m_state, m_schema, memberId, arg1, reason ) )
            HostRefuse( memberId, "faction", reason, sequence );
        else
        {
            m_state.FindSlotOfMember( memberId )->faction = arg1;
            HostCommit( "slots", memberId, "faction" );
        }
    }
    else
    if ( "col" == verb )
    {
        if ( !m_rules.CheckColour( m_state, m_schema, memberId, arg1, reason ) )
            HostRefuse( memberId, "colour", reason, sequence );
        else
        {
            m_state.FindSlotOfMember( memberId )->colour = arg1;
            HostCommit( "slots", memberId, "colour" );
        }
    }
    else
    if ( "kick" == verb || "ban" == verb )
    {
        if ( !m_rules.CheckKick( m_state, memberId, arg1, reason ) )
            HostRefuse( memberId, verb, reason, sequence );
        else
            HostKickMember( arg1, "ban" == verb ? "You are banned from the lobby" : "You were kicked", "ban" == verb );
    }
    else
    if ( "boss" == verb )
    {
        if ( !m_rules.CheckTransferBoss( m_state, memberId, arg1, reason ) )
            HostRefuse( memberId, "transferBoss", reason, sequence );
        else
        {
            m_rules.ApplyTransferBoss( m_state, arg1 );
            if ( HostCommit( "boss", memberId, "transferBoss" ) )
                m_promoteTarget = arg1;
        }
    }
    else
    if ( "slotkind" == verb )
    {
        UInt32 slotIndex = CORE::StringToUInt32( arg1, 0xFFFFFFFF );
        CORE::CString::StringVector kindParts = arg2.ParseElements( ':', true );
        UInt32 kindNumber = kindParts.empty() ? 0xFFFFFFFF : CORE::StringToUInt32( kindParts[ 0 ], 0xFFFFFFFF );
        if ( kindNumber > (UInt32) MULTIPLAY::LOBBYSLOTKIND_AI )
        {
            HostRefuse( memberId, "slotKind", "That is not a kind of slot", sequence );
        }
        else
        {
            MULTIPLAY::TLobbySlotKind kind = (MULTIPLAY::TLobbySlotKind) kindNumber;
            if ( !m_rules.CheckSlotKind( m_state, memberId, slotIndex, kind, reason ) )
                HostRefuse( memberId, "slotKind", reason, sequence );
            else
            {
                MULTIPLAY::CLobbySlot* slot = m_state.FindSlot( slotIndex );
                slot->kind = kind;
                slot->aiType = MULTIPLAY::LOBBYSLOTKIND_AI == kind && kindParts.size() > 1 ? kindParts[ 1 ] : CORE::CString::Empty;
                HostCommit( "slots", memberId, "slotKind" );
            }
        }
    }
    else
    if ( "start" == verb || "fstart" == verb )
    {
        bool force = "fstart" == verb;
        bool needsVote = false;
        if ( !m_rules.CheckStart( m_state, memberId, force, needsVote, reason ) )
            HostRefuse( memberId, force ? "forceStart" : "start", reason, sequence );
        else
        if ( needsVote )
            HostStartVote( memberId, "start", CORE::CString::Empty, CORE::CString::Empty );
        else
            HostStartCountdown();
    }
    else
    {
        HostRefuse( memberId, verb, "The owner does not know that request", sequence );
    }

    if ( 0 != sequence )
    {
        m_acks[ memberId ] = sequence;
        m_dirty = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostStartVote( const CORE::CString& memberId, const CORE::CString& what, const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    MULTIPLAY::CLobbyVote& vote = m_state.activeVote;
    vote.Clear();
    vote.id = "vote-" + CORE::UInt32ToString( ++m_voteCount );
    vote.what = what;
    vote.proposer = memberId;
    vote.key = key;
    vote.value = value;
    vote.yes = 1;
    vote.no = 0;
    vote.needed = m_rules.GetVotesNeeded( m_state );
    vote.voters.insert( memberId );
    vote.deadline = CORE::CDateTime::NowUTCDateTime();
    vote.deadline.AddSeconds( (Int32) m_rules.voteDurationInSecs );
    m_voteSecondsLeft = (Int32) m_rules.voteDurationInSecs;

    MULTIPLAY::CLobbyVote started = vote;
    if ( !HostCommit( "vote", memberId, what ) )
    {
        m_voteSecondsLeft = -1;
        return;
    }
    m_lastVoteStarted = started.id;
    MULTIPLAY::CLobbyVoteEventData startedData( started, false );
    NotifyObservers( VoteStartedEvent, &startedData );
    HostEvaluateVote();
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostEvaluateVote( void )
{GUCEF_TRACE;

    MULTIPLAY::CLobbyRules::TVoteOutcome outcome = m_rules.EvaluateVote( m_state );
    if ( MULTIPLAY::CLobbyRules::VOTEOUTCOME_PASSED == outcome )
        HostEndVote( true );
    else
    if ( MULTIPLAY::CLobbyRules::VOTEOUTCOME_FAILED == outcome )
        HostEndVote( false );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostEndVote( bool passed )
{GUCEF_TRACE;

    MULTIPLAY::CLobbyVote vote = m_state.activeVote;
    m_state.activeVote.Clear();
    m_voteSecondsLeft = -1;
    m_lastEndedVote = vote;
    m_lastEndedPassed = passed;
    m_lastVoteEndedSeen = vote.id;

    MULTIPLAY::CLobbyVoteEventData ended( vote, passed );
    HostCommit( "vote", CORE::CString::Empty, CORE::CString::Empty );
    NotifyObservers( VoteEndedEvent, &ended );
    if ( !passed )
        return;

    if ( "setting" == vote.what )
    {
        m_state.SetSetting( vote.key, vote.value );
        HostCommit( "settings/" + vote.key, vote.proposer, "setting" );
    }
    else
    if ( "start" == vote.what )
    {
        CORE::CString reason;
        if ( m_rules.IsStartAllowed( m_state, reason ) )
            HostStartCountdown();
        else
            HostRefuse( vote.proposer, "start", reason, 0 );
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostStartCountdown( void )
{GUCEF_TRACE;

    m_state.phase = MULTIPLAY::LOBBYPHASE_COUNTDOWN;
    m_countdownLeft = (Int32) m_rules.countdownInSecs;
    if ( !HostCommit( "phase", CORE::CString::Empty, CORE::CString::Empty ) )
    {
        m_countdownLeft = -1;
        return;
    }
    CORE::TCloneableInt32 left( m_countdownLeft );
    NotifyObservers( CountdownEvent, &left );
    if ( 0 >= m_countdownLeft )
        HostLaunch();
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostCancelCountdown( void )
{GUCEF_TRACE;

    m_countdownLeft = -1;
    CORE::TCloneableInt32 cancelled( -1 );
    NotifyObservers( CountdownEvent, &cancelled );
    StateChanged( "phase" );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostSecond( void )
{GUCEF_TRACE;

    if ( MULTIPLAY::LOBBYPHASE_COUNTDOWN == m_state.phase && m_countdownLeft > 0 )
    {
        --m_countdownLeft;
        m_dirty = true;
        CORE::TCloneableInt32 left( m_countdownLeft );
        NotifyObservers( CountdownEvent, &left );
        if ( 0 == m_countdownLeft )
            HostLaunch();
    }
    if ( m_state.activeVote.IsActive() && m_voteSecondsLeft > 0 )
    {
        --m_voteSecondsLeft;
        if ( 0 == m_voteSecondsLeft )
            HostEndVote( false );
    }
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::HostPrepareLaunch( CORE::CString& reason )
{GUCEF_TRACE;

    const CEosLobbyServiceSettings& settings = m_service->GetSettings();
    CEosP2PRouter* router = m_service->GetRouter();
    if ( GUCEF_NULL == router )
    {
        reason = "EOS P2P is not available, the game cannot be handed to the members";
        return false;
    }

    // The tokens of the game: signed by a key of this lobby, which no member has. Without a signature codec there are none
    if ( !m_haveLaunchKey )
    {
        UInt8 publicKey[ DRN::CDRNCrypto::KeySize ];
        if ( DRN::CDRNCrypto::IsSignatureAvailable() && DRN::CDRNCrypto::GenerateSignatureKeyPair( m_launchKey, publicKey ) )
        {
            m_haveLaunchKey = true;
            m_launchPublicKey = DRN::CDRNCrypto::ToHex( publicKey, sizeof( publicKey ) );
        }
        else
        if ( !settings.allowUnsignedLaunch )
        {
            reason = "The tokens of the game need an \"ed25519\" signature codec, is codecspluginMONOCYPHER loaded?";
            return false;
        }
    }

    // The game runs on a socket of EOS P2P that only the members of the lobby can reach
    if ( !OpenGameSocket() || !OpenControlSocket() )
    {
        reason = "The sockets of the game could not be opened";
        return false;
    }
    SEosLobbyDetails details;
    m_service->CopyDetails( m_lobbyId, details );
    UpdateSockets( details );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostMakeLaunch( MULTIPLAY::CLaunchDescription& launch ) const
{GUCEF_TRACE;

    launch = MULTIPLAY::CLaunchDescription();
    launch.gameId = m_params.gameId;
    launch.gameVersion = m_params.gameVersion;
    launch.lobbyId = m_lobbyId;
    launch.sessionName = "eos-" + m_lobbyId;
    launch.topology = m_params.topology;
    if ( MULTIPLAY::GAMETOPOLOGY_PEER_TO_PEER == m_params.topology )
        launch.sequencerMemberId = m_localUser;
    else
        launch.serverMemberId = m_localUser;

    CORE::CString socketName = CEosLobbyCodec::MakeSocketName( "gg", m_lobbyId );
    for ( size_t i=0; i<m_state.members.size(); ++i )
    {
        const MULTIPLAY::CLobbyMember& member = m_state.members[ i ];
        MULTIPLAY::CLaunchMember launchMember;
        launchMember.id = member.id;
        launchMember.name = member.name;
        launchMember.role = member.role;
        launchMember.slot = member.slot;
        launchMember.endpoints.push_back( "eos-p2p://" + member.id + "/" + socketName );
        launch.members.push_back( launchMember );
    }
    launch.settings = m_state.settings;
    launch.slots = m_state.slots;

    // The tokens are checked by every node with the public key alone, so no member can make one; the policy they are for is the game's
    launch.securityPolicy = m_params.securityPolicy;
    if ( m_haveLaunchKey )
    {
        launch.securityPolicy.requireSignedTokens = true;
        launch.securityPolicy.requireTokenPolicyHash = true;
        launch.serverPublicKey = m_launchPublicKey;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosLobby::HostIssueToken( const CORE::CString& userId, const CORE::CString& role ) const
{GUCEF_TRACE;

    if ( !m_haveLaunchKey )
        return CORE::CString::Empty;
    UInt8 policyHash[ DRN::CDRNSecurityPolicy::PolicyHashSize ];
    m_launch.securityPolicy.GetPolicyHash( policyHash );
    return DRN::CDRNTokenAuthenticator::IssueSignedToken( m_launchKey, m_launch.sessionName, userId, role,
                                                          DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + m_service->GetSettings().tokenLifetimeInSecs,
                                                          GUCEF_NULL, policyHash );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostSendTokenTo( const CORE::CString& userId )
{GUCEF_TRACE;

    if ( !m_haveLaunch || GUCEF_NULL == m_controlSocket )
        return;
    const MULTIPLAY::CLobbyMember* member = m_state.FindMember( userId );
    if ( GUCEF_NULL == member || userId == m_localUser )
        return;

    // A token is not given out again and again
    UInt64 nowInMs = PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs();
    GUCEF::map< CORE::CString, UInt64 >::iterator last = m_lastTokenSent.find( userId );
    if ( last != m_lastTokenSent.end() && nowInMs < (*last).second + 250 )
        return;
    m_lastTokenSent[ userId ] = nowInMs;

    CORE::CString role = member->IsSpectator() ? "spectator" : "player";
    CORE::CString token = HostIssueToken( userId, role );
    if ( token.IsNULLOrEmpty() || token.ByteSize() > 1000 )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosLobby: No token for a member of the game" );
        return;
    }
    GUCEF::vector< char > packet;
    packet.push_back( 'G' ); packet.push_back( 'T' ); packet.push_back( 'K' ); packet.push_back( '1' );
    const char* chars = token.C_String();
    for ( UInt32 i=0; i<token.ByteSize()-1; ++i )
        packet.push_back( chars[ i ] );
    m_controlSocket->SendPacket( userId, &packet[ 0 ], (UInt32) packet.size(), EOSRELIABILITY_RELIABLE_ORDERED );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostLaunch( void )
{GUCEF_TRACE;

    m_countdownLeft = -1;
    CORE::CString reason;
    if ( !HostPrepareLaunch( reason ) )
    {
        // No game: the lobby is open again
        m_state.phase = MULTIPLAY::LOBBYPHASE_OPEN;
        HostCommit( "phase", CORE::CString::Empty, CORE::CString::Empty );
        CORE::TCloneableInt32 cancelled( -1 );
        NotifyObservers( CountdownEvent, &cancelled );
        Refuse( "start", reason );
        return;
    }

    m_state.phase = MULTIPLAY::LOBBYPHASE_LAUNCHING;
    HostMakeLaunch( m_launch );
    m_haveLaunch = true;
    m_launchPublished = false;
    if ( !HostCommit( "phase", CORE::CString::Empty, CORE::CString::Empty ) )
    {
        // The game data does not fit the lobby
        m_haveLaunch = false;
        m_state.phase = MULTIPLAY::LOBBYPHASE_OPEN;
        HostCommit( "phase", CORE::CString::Empty, CORE::CString::Empty );
        Refuse( "start", "The data of the game does not fit the lobby" );
        return;
    }

    // The owner is in the game too
    MULTIPLAY::CLaunchDescription own( m_launch );
    own.localMemberId = m_localUser;
    own.token = HostIssueToken( m_localUser, "host" );
    own.spectator = false;
    m_launchRaised = true;
    NotifyObservers( LaunchEvent, &own );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostFixBoss( void )
{GUCEF_TRACE;

    // The boss of the lobby is the one that owns it in EOS
    if ( m_state.bossId != m_localUser && GUCEF_NULL != m_state.FindMember( m_localUser ) )
        m_rules.ApplyTransferBoss( m_state, m_localUser );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostApplyLeaveOutcome( const CORE::CString& userId, const MULTIPLAY::CLobbyVote& voteBefore, const MULTIPLAY::CLobbyRules::TLeaveOutcome& outcome )
{GUCEF_TRACE;

    CORE::TCloneableString left( userId );
    NotifyObservers( MemberLeftEvent, &left );

    if ( outcome.voteCancelled )
    {
        m_voteSecondsLeft = -1;
        m_lastEndedVote = voteBefore;
        m_lastEndedPassed = false;
        m_lastVoteEndedSeen = voteBefore.id;
        MULTIPLAY::CLobbyVoteEventData ended( voteBefore, false );
        NotifyObservers( VoteEndedEvent, &ended );
        StateChanged( "vote" );
    }
    if ( outcome.countdownCancelled )
        HostCancelCountdown();
    if ( !outcome.newBossId.IsNULLOrEmpty() )
        StateChanged( "boss" );
    StateChanged( "members" );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostMemberGone( const CORE::CString& userId )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_pendingJoins.size(); ++i )
    {
        if ( m_pendingJoins[ i ].userId == userId )
        {
            m_pendingJoins.erase( m_pendingJoins.begin() + i );
            break;
        }
    }
    m_processedSequences.erase( userId );
    m_acks.erase( userId );
    m_refusals.erase( userId );
    m_memberBuckets.erase( userId );
    m_lastTokenSent.erase( userId );

    // A member that is gone needs neither a kick nor the reason of one: whoever joins under its id later starts fresh
    for ( size_t i=0; i<m_kickQueue.size(); )
    {
        if ( m_kickQueue[ i ].userId == userId )
            m_kickQueue.erase( m_kickQueue.begin() + i );
        else
            ++i;
    }
    if ( m_rejections.erase( userId ) > 0 )
        m_dirty = true;

    if ( GUCEF_NULL == m_state.FindMember( userId ) )
        return;
    MULTIPLAY::CLobbyVote voteBefore = m_state.activeVote;
    MULTIPLAY::CLobbyRules::TLeaveOutcome outcome;
    m_rules.ApplyLeave( m_state, userId, false, outcome );
    HostFixBoss();
    HostApplyLeaveOutcome( userId, voteBefore, outcome );
    HostCommit( CORE::CString::Empty, CORE::CString::Empty, CORE::CString::Empty );

    SEosLobbyDetails details;
    if ( m_service->CopyDetails( m_lobbyId, details ) )
        UpdateSockets( details );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostKickMember( const CORE::CString& userId, const CORE::CString& reason, bool ban )
{GUCEF_TRACE;

    if ( ban )
        m_bans.insert( userId );

    // Out of the state first, and the reason in it: the member reads why before EOS removes it
    m_rejections[ userId ] = NoSeparator( reason );
    SKick kick;
    kick.userId = userId;
    kick.minRevision = m_revision + 1;
    kick.notBeforeInMs = PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs() + m_service->GetSettings().departureGraceInMs;
    m_kickQueue.push_back( kick );

    if ( GUCEF_NULL != m_state.FindMember( userId ) )
    {
        MULTIPLAY::CLobbyVote voteBefore = m_state.activeVote;
        MULTIPLAY::CLobbyRules::TLeaveOutcome outcome;
        m_rules.ApplyLeave( m_state, userId, false, outcome );
        HostFixBoss();
        HostApplyLeaveOutcome( userId, voteBefore, outcome );
    }
    m_acks.erase( userId );
    m_refusals.erase( userId );
    m_processedSequences.erase( userId );
    HostCommit( CORE::CString::Empty, CORE::CString::Empty, CORE::CString::Empty );
    m_dirty = true;

    SEosLobbyDetails details;
    if ( m_service->CopyDetails( m_lobbyId, details ) )
        UpdateSockets( details );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostMemberJoined( const CORE::CString& userId )
{GUCEF_TRACE;

    if ( userId == m_localUser || !CEosLobbyCodec::IsValidUserId( userId ) )
        return;
    if ( GUCEF_NULL != m_state.FindMember( userId ) )
        return;
    for ( size_t i=0; i<m_pendingJoins.size(); ++i )
        if ( m_pendingJoins[ i ].userId == userId )
            return;

    // A reason that was given to an earlier membership is not for this one
    m_rejections.erase( userId );

    // Banned ones do not come back
    if ( m_bans.find( userId ) != m_bans.end() )
    {
        HostKickMember( userId, "You are banned from the lobby", true );
        return;
    }

    SPendingJoin pending;
    pending.userId = userId;
    pending.deadlineInMs = PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs() + m_service->GetSettings().joinGraceInMs;
    m_pendingJoins.push_back( pending );
    HostEvaluateJoin( userId, false );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostEvaluateJoin( const CORE::CString& userId, bool deadlinePassed )
{GUCEF_TRACE;

    size_t pendingIndex = m_pendingJoins.size();
    for ( size_t i=0; i<m_pendingJoins.size(); ++i )
        if ( m_pendingJoins[ i ].userId == userId )
            pendingIndex = i;
    if ( pendingIndex >= m_pendingJoins.size() )
        return;

    SEosLobbyDetails details;
    if ( !m_service->CopyDetails( m_lobbyId, details ) )
        return;
    const SEosLobbyMember* member = GUCEF_NULL;
    for ( size_t i=0; i<details.members.size(); ++i )
        if ( details.members[ i ].puid == userId )
            member = &details.members[ i ];
    if ( GUCEF_NULL == member )
    {
        m_pendingJoins.erase( m_pendingJoins.begin() + pendingIndex );
        return;
    }

    // What the member says of itself: nothing of it is trusted beyond its bounds
    CORE::CString rawName;
    CORE::CString proof;
    Int64 spectatorFlag = 0;
    bool haveName = CEosLobbyCodec::GetString( member->attributes, CEosLobbyCodec::MemberKeyName, rawName );
    bool haveProof = CEosLobbyCodec::GetString( member->attributes, CEosLobbyCodec::MemberKeyPasswordProof, proof );
    CEosLobbyCodec::GetInt( member->attributes, CEosLobbyCodec::MemberKeySpectator, spectatorFlag );
    bool needProof = !m_password.IsNULLOrEmpty();

    if ( ( !haveName || ( needProof && !haveProof ) ) && !deadlinePassed )
        return;

    CORE::CString reason;
    bool accepted = true;
    CORE::CString name = CEosLobbyCodec::SanitizeText( rawName, 64 );
    if ( !haveName || name.IsNULLOrEmpty() )
    {
        accepted = false;
        reason = "No name was given in time";
    }
    else
    if ( needProof )
    {
        if ( !haveProof )
        {
            accepted = false;
            reason = "No password was given in time";
        }
        else
        if ( !ProofsEqual( proof, CEosLobbyCodec::MakePasswordProof( m_summary.passwordSalt, userId, m_password ) ) )
        {
            accepted = false;
            reason = "The password is wrong";
        }
    }
    if ( accepted )
        accepted = m_rules.CheckJoin( m_state, 0 != spectatorFlag, reason );

    m_pendingJoins.erase( m_pendingJoins.begin() + pendingIndex );
    if ( !accepted )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosLobby: A member that joined is refused: " << reason );
        HostKickMember( userId, reason, false );
        return;
    }

    m_rules.ApplyJoin( m_state, userId, name, 0 != spectatorFlag );
    if ( !HostCommit( "members", CORE::CString::Empty, CORE::CString::Empty ) )
    {
        HostKickMember( userId, "The lobby has no room for more data", false );
        return;
    }
    CORE::TCloneableString joined( userId );
    NotifyObservers( MemberJoinedEvent, &joined );
    StateChanged( "slots" );

    // A game under way takes the newcomer in: a spectator gets the data of the game
    if ( m_haveLaunch )
    {
        HostMakeLaunch( m_launch );
        HostCommit( CORE::CString::Empty, CORE::CString::Empty, CORE::CString::Empty );
    }
    UpdateSockets( details );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostOnMemberUpdated( const CORE::CString& userId )
{GUCEF_TRACE;

    if ( userId == m_localUser )
        return;
    if ( GUCEF_NULL == m_state.FindMember( userId ) )
    {
        HostEvaluateJoin( userId, false );
        return;
    }
    HostProcessMember( userId );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostProcessMember( const CORE::CString& userId )
{GUCEF_TRACE;

    SEosLobbyDetails details;
    if ( !m_service->CopyDetails( m_lobbyId, details ) )
        return;
    const SEosLobbyMember* member = GUCEF_NULL;
    for ( size_t i=0; i<details.members.size(); ++i )
        if ( details.members[ i ].puid == userId )
            member = &details.members[ i ];
    if ( GUCEF_NULL == member )
        return;

    UInt32 processed = 0;
    TSequenceMap::const_iterator p = m_processedSequences.find( userId );
    if ( p != m_processedSequences.end() )
        processed = (*p).second;

    // The name of the member, when it changed
    CORE::CString rawName;
    if ( CEosLobbyCodec::GetString( member->attributes, CEosLobbyCodec::MemberKeyName, rawName ) )
    {
        CORE::CString name = CEosLobbyCodec::SanitizeText( rawName, 64 );
        MULTIPLAY::CLobbyMember* inState = m_state.FindMember( userId );
        if ( !name.IsNULLOrEmpty() && GUCEF_NULL != inState && inState->name != name )
        {
            inState->name = name;
            HostCommit( "members", userId, "name" );
        }
    }

    // The request and the readiness it has; the one with the lower sequence first
    UInt32 requestSequence = 0, readySequence = 0;
    CORE::CString verb, arg1, arg2, readyText;
    bool ready = false;
    bool haveRequest = false, haveReady = false;
    CORE::CString text;
    if ( CEosLobbyCodec::GetString( member->attributes, CEosLobbyCodec::MemberKeyRequest, text ) )
        haveRequest = CEosLobbyCodec::ParseRequest( text, requestSequence, verb, arg1, arg2 ) && requestSequence > processed;
    if ( CEosLobbyCodec::GetString( member->attributes, CEosLobbyCodec::MemberKeyReady, text ) )
        haveReady = CEosLobbyCodec::ParseReady( text, readySequence, ready ) && readySequence > processed;

    UInt64 nowInMs = PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs();
    for ( int round=0; round<2; ++round )
    {
        bool takeReady = haveReady && ( !haveRequest || readySequence < requestSequence );
        if ( !takeReady && !haveRequest )
            break;
        UInt32 sequence = takeReady ? readySequence : requestSequence;

        // A member does not flood the owner
        TBucketMap::iterator bucket = m_memberBuckets.find( userId );
        if ( bucket == m_memberBuckets.end() )
        {
            MULTIPLAY::CTokenBucket fresh;
            fresh.SetRate( m_service->GetSettings().memberRequestsPerSecond, m_service->GetSettings().memberRequestBurst );
            bucket = m_memberBuckets.insert( TBucketMap::value_type( userId, fresh ) ).first;
        }
        if ( !(*bucket).second.Take( nowInMs ) )
        {
            HostRefuse( userId, takeReady ? "ready" : verb, "Too many requests", sequence );
            m_acks[ userId ] = sequence;
            m_dirty = true;
        }
        else
        if ( takeReady )
        {
            HostSetReady( userId, ready, sequence );
            m_acks[ userId ] = sequence;
            m_dirty = true;
        }
        else
        {
            HostHandle( userId, verb, arg1, arg2, sequence );
        }
        m_processedSequences[ userId ] = sequence;
        if ( takeReady )
            haveReady = false;
        else
            haveRequest = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::HostClose( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( m_closing )
        return;

    // The members read why before the lobby is gone: the publication with the reason goes first, rate limit or not
    m_closing = true;
    m_closeReason = reason;
    m_state.phase = MULTIPLAY::LOBBYPHASE_CLOSED;
    m_pendingJoins.clear();
    m_dirty = true;
    if ( m_state.members.size() <= 1 || STAGE_ACTIVE != m_stage )
    {
        // Nobody to tell
        m_service->RequestDestroy( m_lobbyId );
        m_closing = false;
        Detach( CORE::CString::Empty, false );
        return;
    }
    m_closeRevision = m_revision + 1;
    if ( !m_updateInFlight )
        HostPublish();
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::Takeover( const SEosLobbyDetails& details )
{GUCEF_TRACE;

    // EOS made the local user the owner: the lobby is run from here from now on
    if ( !m_hasDefinition || !m_haveOwnerState )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosLobby: The lobby was handed to the local user without its definition, it is left" );
        m_service->RequestLeave( m_lobbyId );
        Detach( "The lobby could not be taken over", true );
        return;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosLobby: The local user became the owner of the lobby" );

    m_isOwner = true;
    m_ownerUserId = m_localUser;
    m_password.Clear();
    m_requests.clear();
    m_requestInFlight = false;
    m_state.lobbyId = m_lobbyId;

    // A countdown or a vote was the old owner's: both end
    if ( MULTIPLAY::LOBBYPHASE_COUNTDOWN == m_state.phase )
        m_state.phase = MULTIPLAY::LOBBYPHASE_OPEN;
    m_countdownLeft = -1;
    m_state.activeVote.Clear();
    m_voteSecondsLeft = -1;

    // Whoever is not in the lobby any more leaves it, whoever is in it and not in the state is a newcomer
    TEosAttributeVector dummy;
    for ( size_t i=0; i<m_state.members.size(); )
    {
        if ( !IsInEos( details, m_state.members[ i ].id ) )
        {
            MULTIPLAY::CLobbyRules::TLeaveOutcome outcome;
            CORE::CString id = m_state.members[ i ].id;
            m_rules.ApplyLeave( m_state, id, false, outcome );
            CORE::TCloneableString left( id );
            NotifyObservers( MemberLeftEvent, &left );
        }
        else
        {
            ++i;
        }
    }
    if ( GUCEF_NULL == m_state.FindMember( m_localUser ) )
    {
        m_rules.ApplyJoin( m_state, m_localUser, "Player", false );
    }
    HostFixBoss();
    m_lastGoodState = m_state;

    // Everything is written again with the stamp of the new owner, and what the old owner left that is not needed goes
    m_revision = m_lastRevisionSeen;
    m_wireAttributes.clear();
    for ( size_t i=0; i<details.attributes.size(); ++i )
        m_wireAttributes.push_back( details.attributes[ i ] );
    m_forceFull = true;
    m_definitionAttributes.clear();
    m_dirty = true;
    m_lastSecondInMs = PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs();
    m_updateBucket.SetRate( m_service->GetSettings().updatesPerSecond, m_service->GetSettings().updateBurst );

    // The game: the sockets stay, the state says whether there is a launch
    if ( m_launchWanted || m_launchRaised )
        m_haveLaunch = true;
    StateChanged( "boss" );
    StateChanged( "members" );
    StateChanged( "phase" );

    for ( size_t i=0; i<details.members.size(); ++i )
        if ( details.members[ i ].puid != m_localUser && GUCEF_NULL == m_state.FindMember( details.members[ i ].puid ) )
            HostMemberJoined( details.members[ i ].puid );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::SetVisibility( bool visible )
{GUCEF_TRACE;

    if ( !IsMember() || !m_isOwner )
        return false;
    if ( visible == m_visible )
        return true;
    m_visible = visible;
    m_visibilityChange = true;
    m_dirty = true;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
