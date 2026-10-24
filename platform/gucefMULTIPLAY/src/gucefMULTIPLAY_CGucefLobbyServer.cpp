/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#include "gucefDRN_CDRNDataGrid.h"
#define GUCEF_DRN_CDRNDATAGRID_H
#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */

#ifndef GUCEF_DRN_CDRNDATASTREAM_H
#include "gucefDRN_CDRNDataStream.h"
#define GUCEF_DRN_CDRNDATASTREAM_H
#endif /* GUCEF_DRN_CDRNDATASTREAM_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CDRNVARIANTCODEC_H
#include "gucefDRN_CDRNVariantCodec.h"
#define GUCEF_DRN_CDRNVARIANTCODEC_H
#endif /* GUCEF_DRN_CDRNVARIANTCODEC_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#ifndef GUCEF_MULTIPLAY_DATANODEUTILS_H
#include "gucefMULTIPLAY_DataNodeUtils.h"
#define GUCEF_MULTIPLAY_DATANODEUTILS_H
#endif /* GUCEF_MULTIPLAY_DATANODEUTILS_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#include "gucefMULTIPLAY_CGucefLobbyProtocol.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#include "gucefMULTIPLAY_CGucefLobbyAuthenticator.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H
#include "gucefMULTIPLAY_CGucefLobbyRelayAuthenticator.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

#ifndef GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#include "gucefMULTIPLAY_CLaunchDescription.h"
#define GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#endif /* GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMEADVERTISER_H
#include "gucefMULTIPLAY_CGameAdvertiser.h"
#define GUCEF_MULTIPLAY_CGAMEADVERTISER_H
#endif /* GUCEF_MULTIPLAY_CGAMEADVERTISER_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#include "gucefMULTIPLAY_CGucefLobbyServer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CGucefLobbyServer::LobbyCreatedEvent = "GUCEF::MULTIPLAY::CGucefLobbyServer::LobbyCreatedEvent";
const CORE::CEvent CGucefLobbyServer::LobbyClosedEvent = "GUCEF::MULTIPLAY::CGucefLobbyServer::LobbyClosedEvent";
const CORE::CEvent CGucefLobbyServer::LobbyLaunchedEvent = "GUCEF::MULTIPLAY::CGucefLobbyServer::LobbyLaunchedEvent";
const CORE::CEvent CGucefLobbyServer::PlayerReportedEvent = "GUCEF::MULTIPLAY::CGucefLobbyServer::PlayerReportedEvent";

#define GUCEF_MULTIPLAY_LOBBYSERVER_TICK_MS     10
#define GUCEF_MULTIPLAY_LOBBYSERVER_CLOSE_MS    200
#define GUCEF_MULTIPLAY_LOBBYSERVER_MAX_REPORTS 10000
#define GUCEF_MULTIPLAY_LOBBYSERVER_MAX_REASON  500

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Writes a value when it differs from what the grid has */
void
Put( DRN::CDRNDataGrid* grid, const CORE::CString& key, const CORE::CVariant& value )
{GUCEF_TRACE;

    if ( GUCEF_NULL == grid )
        return;
    CORE::CVariant current;
    if ( grid->GetValue( key, current ) && DRN::CDRNVariantCodec::IsSame( current, value ) )
        return;
    grid->SetValue( key, value );
}

/*-------------------------------------------------------------------------*/

DRN::CDRNDataGrid*
CreateHostGrid( DRN::CDRNSession& session, const CORE::CString& name, DRN::TDRNAuthority authority )
{GUCEF_TRACE;

    DRN::CDRNDataGridSettings settings;
    settings.name = name;
    settings.authority = authority;
    settings.reliable = true;
    settings.sendRateHz = 0;
    settings.notify = DRN::DRN_NOTIFY_PER_PACKET;
    return session.CreateGrid( settings );
}

/*-------------------------------------------------------------------------*/

CORE::CString
JoinStrings( const CORE::CString::StringVector& strings )
{GUCEF_TRACE;

    CORE::CString joined;
    for ( size_t i=0; i<strings.size(); ++i )
    {
        if ( i > 0 )
            joined += ';';
        joined += strings[ i ];
    }
    return joined;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
SplitStrings( const CORE::CString& joined )
{GUCEF_TRACE;

    CORE::CString::StringVector strings;
    if ( joined.IsNULLOrEmpty() )
        return strings;
    CORE::CString::StringVector parts = joined.ParseElements( ';', false );
    for ( size_t i=0; i<parts.size(); ++i )
        if ( !parts[ i ].IsNULLOrEmpty() )
            strings.push_back( parts[ i ] );
    return strings;
}

/*-------------------------------------------------------------------------*/

/** The integrity report a request carries, an empty one when it carries none */
void
ReadIntegrity( const CORE::CVariant& blob, CORE::CValueList& integrity )
{GUCEF_TRACE;

    integrity.Clear();
    CORE::CDataNode tree;
    if ( blob.IsBinary() && CGucefLobbyProtocol::VariantToDataTree( blob, tree ) )
        ReadValueListFromChild( tree, "integrity", integrity );
}

/*-------------------------------------------------------------------------*/

/** Whether a report has every value expected, why not when it has not */
bool
CheckIntegrity( const CORE::CValueList& expected, const CORE::CValueList& report, CORE::CString& reason )
{GUCEF_TRACE;

    CORE::CString differing;
    UInt32 count = expected.GetKeyCount();
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CString name = expected.GetKey( i ).AsString();
        if ( !report.HasKey( name ) || report.GetValueAlways( name ).AsString() != expected.GetValue( i ).AsString() )
        {
            if ( !differing.IsNULLOrEmpty() )
                differing += ", ";
            differing += name;
        }
    }
    if ( differing.IsNULLOrEmpty() )
        return true;
    reason = "The game files differ: " + differing;
    return false;
}

/*-------------------------------------------------------------------------*/

/** The host of a link's remote endpoint, what an address ban is keyed on */
CORE::CString
GetAddressOfLink( const DRN::CDRNPeerLink& link )
{GUCEF_TRACE;

    DRN::CDRNEndpoint remote = link.GetRemoteEndpoint();
    if ( remote.GetChannelType().Equals( DRN::CDRNEndpoint::ChannelTypeRelay, false ) )
        return remote.GetAddress();
    CORE::CString host = remote.GetHost();
    return host.IsNULLOrEmpty() ? remote.GetAddress() : host;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Lets a link join the directory, and a lobby when its token is for that lobby and the lobby admitted its user */
class CGucefLobbyServer::CAccessPolicy : public DRN::CIDRNAccessPolicy
{
    public:

    CGucefLobbyServer* m_server;

    CAccessPolicy( CGucefLobbyServer* server ) : DRN::CIDRNAccessPolicy(), m_server( server ) {}

    virtual bool MayJoin( const DRN::CDRNPeerLink& link, const CORE::CString& sessionName ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool MaySubscribe( UInt32 peerId, const CORE::CString& itemName ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool MayWrite( UInt32 peerId, const CORE::CString& itemName, UInt32 keyIndex ) GUCEF_VIRTUAL_OVERRIDE { return true; }
};

/*-------------------------------------------------------------------------*/

/** One lobby: its DRN session, its state and the rules that change it */
class CGucefLobbyServer::CHostedLobby : public CORE::CObserver ,
                                        public DRN::CIDRNWriteValidator
{
    public:

    struct SAdmission
    {
        CORE::CString name;
        bool spectator;
        CORE::CString::StringVector endpoints;
        CORE::CString publicKey;
        CORE::CValueList integrity;
    };
    typedef struct SAdmission TAdmission;
    typedef GUCEF::map< CORE::CString, TAdmission > TAdmissionMap;
    typedef GUCEF::map< UInt32, CORE::CString > TPeerMap;
    typedef GUCEF::map< CORE::CString, UInt32 > TMemberPeerMap;
    typedef GUCEF::map< CORE::CString, CORE::CString::StringVector > TEndpointMap;
    typedef GUCEF::map< CORE::CString, CORE::CString > TStringMap;
    typedef GUCEF::map< CORE::CString, CORE::CValueList > TIntegrityMap;
    typedef GUCEF::map< CORE::CString, CTokenBucket > TBucketMap;

    CGucefLobbyServer& m_server;
    CORE::CString m_lobbyId;
    CLobbyCreateParams m_params;
    CLobbyRules m_rules;
    CLobbyState m_state;
    CORE::CString m_creatorId;
    CORE::CString m_creatorName;
    DRN::CDRNSession* m_session;
    DRN::CDRNDataGrid* m_lobbyGrid;
    DRN::CDRNDataGrid* m_membersGrid;
    DRN::CDRNDataGrid* m_slotsGrid;
    DRN::CDRNDataGrid* m_schemaGrid;
    DRN::CDRNDataGrid* m_proposalsGrid;
    DRN::CDRNDataStream* m_chat;
    TAdmissionMap m_admissions;
    TPeerMap m_peerToMember;
    TMemberPeerMap m_memberToPeer;
    TEndpointMap m_memberEndpoints;
    TStringMap m_memberKeys;
    TIntegrityMap m_memberIntegrity;
    TBucketMap m_chatBuckets;
    CORE::CString::StringSet m_banned;
    CORE::CString::StringSet m_publishedMembers;
    DRN::CDRNSecurityPolicy m_gamePolicy;
    CORE::CString m_lastVoteId;
    CORE::CString m_lastVoteResult;
    Int32 m_countdownLeft;
    UInt64 m_nextCountdownAtMs;
    UInt64 m_nextLatencyAtMs;
    UInt64 m_closeAtMs;
    CORE::CString m_closeReason;
    bool m_closing;
    bool m_closed;
    bool m_dirty;
    UInt32 m_chatCount;
    CGameAdvertiser* m_advertiser;

    CHostedLobby( CGucefLobbyServer& server, const CORE::CString& lobbyId, const CLobbyCreateParams& params )
        : CORE::CObserver()
        , DRN::CIDRNWriteValidator()
        , m_server( server )
        , m_lobbyId( lobbyId )
        , m_params( params )
        , m_rules( server.m_settings.rules )
        , m_state()
        , m_creatorId()
        , m_creatorName()
        , m_session( GUCEF_NULL )
        , m_lobbyGrid( GUCEF_NULL )
        , m_membersGrid( GUCEF_NULL )
        , m_slotsGrid( GUCEF_NULL )
        , m_schemaGrid( GUCEF_NULL )
        , m_proposalsGrid( GUCEF_NULL )
        , m_chat( GUCEF_NULL )
        , m_admissions()
        , m_peerToMember()
        , m_memberToPeer()
        , m_memberEndpoints()
        , m_memberKeys()
        , m_memberIntegrity()
        , m_chatBuckets()
        , m_banned()
        , m_publishedMembers()
        , m_gamePolicy( MakeGamePolicy( params.securityPolicy ) )
        , m_lastVoteId()
        , m_lastVoteResult()
        , m_countdownLeft( -1 )
        , m_nextCountdownAtMs( 0 )
        , m_nextLatencyAtMs( 0 )
        , m_closeAtMs( 0 )
        , m_closeReason()
        , m_closing( false )
        , m_closed( false )
        , m_dirty( true )
        , m_chatCount( 0 )
        , m_advertiser( GUCEF_NULL )
    {GUCEF_TRACE;

        if ( 0 != params.maxPlayers )
            m_rules.maxPlayers = params.maxPlayers;
        if ( 0 != server.m_settings.maxMembersPerLobby && ( 0 == m_rules.maxPlayers || m_rules.maxPlayers > server.m_settings.maxMembersPerLobby ) )
            m_rules.maxPlayers = server.m_settings.maxMembersPerLobby;
        m_rules.allowSpectators = m_rules.allowSpectators && params.allowSpectators;

        m_state.lobbyId = lobbyId;
        m_state.name = params.name;
        m_state.phase = LOBBYPHASE_OPEN;
        m_state.slots = params.slots;
        if ( m_state.slots.empty() )
        {
            for ( UInt32 i=0; i<m_rules.maxPlayers; ++i )
            {
                CLobbySlot slot;
                slot.index = i;
                slot.team = i;
                m_state.slots.push_back( slot );
            }
        }
        params.GetStartSettings( m_state.settings );
        m_state.chatBackend = "lobby";
        m_state.chatChannel = "lobby:" + lobbyId;
    }

    virtual ~CHostedLobby()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
        GUCEF_DELETE m_advertiser;
        m_advertiser = GUCEF_NULL;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGucefLobbyServer::CHostedLobby";
        return classTypeName;
    }

    /*---------------------------------------------------------------------*/

    bool Start( void )
    {GUCEF_TRACE;

        m_session = m_server.m_node.CreateSession();
        if ( GUCEF_NULL == m_session )
            return false;

        DRN::CDRNSessionSettings settings;
        settings.sessionName = CGucefLobbyProtocol::MakeLobbySessionName( m_lobbyId );
        settings.topology = DRN::DRN_TOPOLOGY_CLIENT_SERVER;
        settings.publishLatency = true;
        settings.latencyPublishIntervalMs = m_server.m_settings.latencyIntervalInMs;
        if ( !m_session->Host( settings ) )
            return false;

        m_lobbyGrid = CreateHostGrid( *m_session, CGucefLobbyProtocol::LobbyGrid, DRN::DRN_AUTHORITY_HOST );
        m_membersGrid = CreateHostGrid( *m_session, CGucefLobbyProtocol::MembersGrid, DRN::DRN_AUTHORITY_HOST );
        m_slotsGrid = CreateHostGrid( *m_session, CGucefLobbyProtocol::SlotsGrid, DRN::DRN_AUTHORITY_HOST );
        m_schemaGrid = CreateHostGrid( *m_session, CGucefLobbyProtocol::SchemaGrid, DRN::DRN_AUTHORITY_HOST );

        DRN::CDRNDataGridSettings proposals;
        proposals.name = CGucefLobbyProtocol::ProposalsGrid;
        proposals.authority = DRN::DRN_AUTHORITY_PROPOSAL_VOTE;
        proposals.reliable = true;
        proposals.sendRateHz = 0;
        proposals.votePolicy.rule = DRN::CDRNVotePolicy::VOTERULE_MAJORITY;
        proposals.votePolicy.proposerVoteCounts = true;
        proposals.votePolicy.timeoutMs = m_rules.voteDurationInSecs * 1000;
        m_proposalsGrid = m_session->CreateGrid( proposals );

        DRN::CDRNDataStreamSettings chat;
        chat.name = CGucefLobbyProtocol::ChatStream;
        chat.retainedHistory = 100;
        chat.backlogLimit = 1000;
        m_chat = m_session->CreateStream( chat );

        if ( GUCEF_NULL == m_lobbyGrid || GUCEF_NULL == m_membersGrid || GUCEF_NULL == m_slotsGrid ||
             GUCEF_NULL == m_schemaGrid || GUCEF_NULL == m_proposalsGrid || GUCEF_NULL == m_chat )
            return false;

        m_lobbyGrid->SetWriteValidator( this );
        m_membersGrid->SetWriteValidator( this );
        m_slotsGrid->SetWriteValidator( this );
        m_schemaGrid->SetWriteValidator( this );
        m_proposalsGrid->SetWriteValidator( this );

        CORE::CDataNode schema( "schema" );
        m_params.schema.SaveConfig( schema );
        m_schemaGrid->SetValue( CORE::CVariant( "schema" ), CGucefLobbyProtocol::DataTreeToVariant( schema ) );

        SubscribeTo( m_session );
        SubscribeTo( m_proposalsGrid );
        m_session->Subscribe( CGucefLobbyProtocol::CommandsStream );

        if ( m_server.m_settings.advertiseOnLan )
        {
            m_advertiser = GUCEF_NEW CGameAdvertiser();
            m_advertiser->LoadConfig( m_server.m_settings.advertiserConfig );
        }
        Publish();
        return true;
    }

    /*---------------------------------------------------------------------*/

    /** A user may join, see the directory's "join" request */
    bool Admit( const CORE::CString& userId, const CORE::CString& name, const CORE::CString& password, bool asSpectator,
                const CORE::CString::StringVector& endpoints, const CORE::CString& publicKey, const CORE::CValueList& integrity,
                bool isCreator, CORE::CString& reason )
    {GUCEF_TRACE;

        if ( m_closing || m_closed )
        {
            reason = "The lobby is closed";
            return false;
        }
        if ( m_banned.find( userId ) != m_banned.end() )
        {
            reason = "You are banned from this lobby";
            return false;
        }
        // Compared as digests: neither the content nor the length of the password leaks through the time it takes
        if ( !isCreator && !m_params.password.IsNULLOrEmpty() &&
             !DRN::CDRNCrypto::ConstantTimeEqualsHashed( password.C_String(), password.ByteSize(), m_params.password.C_String(), m_params.password.ByteSize() ) )
        {
            reason = "The password is wrong";
            return false;
        }
        if ( GUCEF_NULL != m_state.FindMember( userId ) )
        {
            reason = "Already a member";
            return false;
        }
        if ( !m_rules.CheckJoin( m_state, asSpectator, reason ) )
            return false;

        TAdmission& admission = m_admissions[ userId ];
        admission.name = name;
        admission.spectator = asSpectator;
        admission.endpoints = endpoints;
        admission.publicKey = publicKey;
        admission.integrity = integrity;
        if ( isCreator )
        {
            m_creatorId = userId;
            m_creatorName = name;
        }
        return true;
    }

    bool HasAdmission( const CORE::CString& userId ) const
    {GUCEF_TRACE;

        return m_admissions.find( userId ) != m_admissions.end();
    }

    /** Tells the member why, takes it out of the lobby, and ends its link when closeLink */
    void KickMember( const CORE::CString& memberId, const CORE::CString& reason, bool closeLink )
    {GUCEF_TRACE;

        m_admissions.erase( memberId );
        UInt32 peerId = GetPeerOfMember( memberId );
        if ( 0 != peerId )
        {
            SendToMember( memberId, CGucefLobbyProtocol::MESSAGE_KICKED, reason, CORE::CString::Empty );
            m_peerToMember.erase( peerId );
            m_memberToPeer.erase( memberId );
        }
        RemoveMember( memberId, "The lobby is empty" );
        if ( 0 != peerId && closeLink && GUCEF_NULL != m_session )
        {
            DRN::CDRNPeerLinkPtr link = m_session->GetLinkToMember( peerId );
            if ( !link.IsNULL() )
                link->Close( DRN::DRN_LINKREASON_CLOSED_BY_APP, reason );
        }
    }

    /** Whether the user is in the lobby or on its way in */
    bool HasUser( const CORE::CString& userId ) const
    {GUCEF_TRACE;

        return GUCEF_NULL != m_state.FindMember( userId ) || HasAdmission( userId );
    }

    /*---------------------------------------------------------------------*/

    bool IsHost( const CORE::CString& memberId ) const
    {GUCEF_TRACE;

        return memberId == m_creatorId && !m_params.isPublic;
    }

    CORE::CString GetMemberOfPeer( UInt32 peerId ) const
    {GUCEF_TRACE;

        TPeerMap::const_iterator i = m_peerToMember.find( peerId );
        return i == m_peerToMember.end() ? CORE::CString::Empty : (*i).second;
    }

    UInt32 GetPeerOfMember( const CORE::CString& memberId ) const
    {GUCEF_TRACE;

        TMemberPeerMap::const_iterator i = m_memberToPeer.find( memberId );
        return i == m_memberToPeer.end() ? 0 : (*i).second;
    }

    /*---------------------------------------------------------------------*/

    bool SendToMember( const CORE::CString& memberId, UInt8 type, const CORE::CString& a, const CORE::CString& b )
    {GUCEF_TRACE;

        UInt32 peerId = GetPeerOfMember( memberId );
        if ( 0 == peerId || GUCEF_NULL == m_session )
            return false;
        CORE::CDynamicBuffer buffer;
        DRN::CDRNWireWriter writer( buffer );
        writer.WriteUInt8( type );
        writer.WriteString( a );
        writer.WriteString( b );
        return m_session->SendAppMessage( peerId, DRN::DRN_LANE_RELIABLE_ORDERED, buffer.GetConstBufferPtr(), writer.GetOffset() );
    }

    void Refuse( const CORE::CString& memberId, const CORE::CString& what, const CORE::CString& reason )
    {GUCEF_TRACE;

        SendToMember( memberId, CGucefLobbyProtocol::MESSAGE_REFUSED, what, reason );
    }

    /*---------------------------------------------------------------------*/

    /** The rules decide on every write to the lobby, the accepted ones change the state at once */
    virtual bool ValidateWrite( UInt32 proposerPeerId            ,
                                const DRN::CDRNDataGrid& grid    ,
                                const CORE::CVariant& key        ,
                                const CORE::CVariant& value      ,
                                CORE::CString& refusalReason     ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CORE::CString memberId = GetMemberOfPeer( proposerPeerId );
        if ( memberId.IsNULLOrEmpty() || GUCEF_NULL == m_state.FindMember( memberId ) )
        {
            refusalReason = "Not a member of the lobby";
            return false;
        }
        CORE::CString keyText = key.AsString();
        CORE::CString valueText = value.AsString();

        if ( &grid == m_lobbyGrid )
        {
            CORE::CString settingKey;
            if ( 0 != keyText.HasSubstr( "settings/", true ) )
            {
                refusalReason = "Only the server changes " + keyText;
                return false;
            }
            settingKey = keyText.CutChars( 9, true );
            bool needsVote = false;
            if ( !m_rules.CheckSetting( m_state, m_params.schema, memberId, settingKey, valueText, needsVote, refusalReason ) )
                return false;
            if ( needsVote )
            {
                refusalReason = "The setting " + settingKey + " is changed by a vote: propose it";
                return false;
            }
            m_state.SetSetting( settingKey, valueText );
            m_dirty = true;
            return true;
        }

        if ( &grid == m_membersGrid )
        {
            CORE::CString owner, field;
            if ( !CGucefLobbyProtocol::SplitMemberKey( keyText, owner, field ) || owner != memberId )
            {
                refusalReason = "A member changes only its own keys";
                return false;
            }
            if ( "ready" == field )
            {
                if ( !m_rules.CheckReady( m_state, memberId, refusalReason ) )
                    return false;
                bool countdownCancelled = false;
                m_rules.ApplyReady( m_state, memberId, value.AsBool( false, true ), countdownCancelled );
            }
            else
            if ( "slot" == field )
            {
                UInt32 slotIndex = (UInt32) value.AsInt32( -1, true );
                if ( !m_rules.CheckSlotRequest( m_state, memberId, slotIndex, refusalReason ) )
                    return false;
                m_rules.ApplySlot( m_state, memberId, slotIndex );
            }
            else
            if ( "faction" == field )
            {
                if ( !m_rules.CheckFaction( m_state, m_params.schema, memberId, valueText, refusalReason ) )
                    return false;
                m_state.FindSlotOfMember( memberId )->faction = valueText;
            }
            else
            if ( "colour" == field )
            {
                if ( !m_rules.CheckColour( m_state, m_params.schema, memberId, valueText, refusalReason ) )
                    return false;
                m_state.FindSlotOfMember( memberId )->colour = valueText;
            }
            else
            if ( "spectator" == field )
            {
                bool spectate = value.AsBool( false, true );
                if ( !m_rules.CheckSpectator( m_state, memberId, spectate, refusalReason ) )
                    return false;
                m_rules.ApplySpectator( m_state, memberId, spectate );
            }
            else
            {
                refusalReason = "Only the server changes " + field;
                return false;
            }
            m_dirty = true;
            return true;
        }

        if ( &grid == m_proposalsGrid )
        {
            if ( "start" == keyText )
            {
                bool needsVote = false;
                if ( !m_rules.CheckStart( m_state, memberId, false, needsVote, refusalReason ) )
                    return false;
                if ( !needsVote )
                {
                    refusalReason = "The boss starts the game directly";
                    return false;
                }
                return true;
            }
            if ( 0 != keyText.HasSubstr( "settings/", true ) )
            {
                refusalReason = "There is no such proposal";
                return false;
            }
            return m_rules.CheckProposal( m_state, m_params.schema, memberId, keyText.CutChars( 9, true ), valueText, refusalReason );
        }

        refusalReason = "Only the server changes " + grid.GetName();
        return false;
    }

    /*---------------------------------------------------------------------*/

    void OnVoteStarted( const DRN::CDRNVoteInfo& info )
    {GUCEF_TRACE;

        CORE::CString keyText = info.key.AsString();
        CLobbyVote& vote = m_state.activeVote;
        vote.Clear();
        vote.id = CORE::ToString( info.voteId );
        vote.proposer = GetMemberOfPeer( info.proposerPeerId );
        if ( "start" == keyText )
        {
            vote.what = "start";
        }
        else
        {
            vote.what = "setting";
            vote.key = keyText.CutChars( 9, true );
            vote.value = info.value.AsString();
        }
        vote.yes = 1;
        vote.needed = m_rules.GetVotesNeeded( m_state );
        vote.voters.insert( vote.proposer );
        vote.deadline = CORE::CDateTime::NowUTCDateTime();
        vote.deadline.AddSeconds( (Int32) m_rules.voteDurationInSecs );
        m_dirty = true;
    }

    void OnVoteEnded( const DRN::CDRNVoteInfo& info )
    {GUCEF_TRACE;

        // A vote that already ended here, for instance because its proposer left, has no say any more
        CLobbyVote vote = m_state.activeVote;
        if ( vote.id != CORE::ToString( info.voteId ) )
            return;

        bool passed = DRN::CDRNVotePolicy::VOTEOUTCOME_PASSED == info.outcome;
        m_state.activeVote.Clear();
        m_lastVoteId = vote.id;
        m_lastVoteResult = passed ? "passed" : "failed";
        m_dirty = true;
        if ( !passed )
            return;

        if ( "setting" == vote.what )
        {
            m_state.SetSetting( vote.key, vote.value );
        }
        else
        if ( "start" == vote.what )
        {
            CORE::CString reason;
            if ( m_rules.IsStartAllowed( m_state, reason ) )
                StartCountdown();
            else
                Refuse( vote.proposer, "start", reason );
        }
    }

    /*---------------------------------------------------------------------*/

    void OnMemberJoined( UInt32 peerId )
    {GUCEF_TRACE;

        DRN::CDRNPeerLinkPtr link = m_session->GetLinkToMember( peerId );
        if ( link.IsNULL() )
            return;
        CORE::CString userId = link->GetAuthResult().userId;
        TAdmissionMap::iterator a = m_admissions.find( userId );
        if ( a == m_admissions.end() )
            return;
        TAdmission admission = (*a).second;
        m_admissions.erase( a );

        CORE::CString reason;
        if ( GUCEF_NULL != m_state.FindMember( userId ) || !m_rules.CheckJoin( m_state, admission.spectator, reason ) )
        {
            m_peerToMember[ peerId ] = userId;
            m_memberToPeer[ userId ] = peerId;
            SendToMember( userId, CGucefLobbyProtocol::MESSAGE_CLOSED, reason.IsNULLOrEmpty() ? CORE::CString( "Already a member" ) : reason, CORE::CString::Empty );
            m_peerToMember.erase( peerId );
            m_memberToPeer.erase( userId );
            return;
        }

        m_rules.ApplyJoin( m_state, userId, admission.name, admission.spectator );
        m_peerToMember[ peerId ] = userId;
        m_memberToPeer[ userId ] = peerId;
        m_memberEndpoints[ userId ] = admission.endpoints;
        m_memberKeys[ userId ] = admission.publicKey;
        m_memberIntegrity[ userId ] = admission.integrity;
        m_chatBuckets[ userId ].SetRate( m_server.m_settings.chatMessagesPerSecond, m_server.m_settings.chatBurst );
        m_dirty = true;
        Publish();

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: " << admission.name << " joined lobby " << m_lobbyId << ( admission.spectator ? " as a spectator" : "" ) );
    }

    void OnMemberLeft( UInt32 peerId )
    {GUCEF_TRACE;

        CORE::CString memberId = GetMemberOfPeer( peerId );
        m_peerToMember.erase( peerId );
        if ( memberId.IsNULLOrEmpty() )
            return;
        m_memberToPeer.erase( memberId );
        RemoveMember( memberId, IsHost( memberId ) ? "The host left" : "The lobby is empty" );
    }

    void RemoveMember( const CORE::CString& memberId, const CORE::CString& closeReason )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_state.FindMember( memberId ) )
            return;
        CORE::CString voteId = m_state.activeVote.id;
        CLobbyRules::TLeaveOutcome outcome;
        m_rules.ApplyLeave( m_state, memberId, IsHost( memberId ), outcome );
        if ( outcome.voteCancelled )
        {
            m_lastVoteResult = "cancelled";
            m_lastVoteId = voteId;
        }
        m_dirty = true;
        if ( outcome.lobbyClosed )
            Close( closeReason );
    }

    /*---------------------------------------------------------------------*/

    void OnCommands( DRN::CDRNDataStream& stream )
    {GUCEF_TRACE;

        CORE::CString memberId = GetMemberOfPeer( stream.GetOwnerPeerId() );
        CORE::CVariantStream entry;
        while ( stream.ReadNextEntry( entry ) )
        {
            CORE::CString verb;
            entry >> verb;
            if ( !memberId.IsNULLOrEmpty() && GUCEF_NULL != m_state.FindMember( memberId ) )
                OnCommand( memberId, verb, entry );
            entry.Clear();
        }
    }

    void OnCommand( const CORE::CString& memberId, const CORE::CString& verb, CORE::CVariantStream& args )
    {GUCEF_TRACE;

        CORE::CString reason;
        if ( "say" == verb )
        {
            CORE::CString text;
            args >> text;
            if ( !m_chatBuckets[ memberId ].Take( GetTimeInMs() ) )
            {
                Refuse( memberId, "say", "Too many messages, wait a moment" );
                return;
            }
            const CLobbyMember* member = m_state.FindMember( memberId );
            CORE::CVariantStream& entry = m_chat->GetWriteStream();
            entry << memberId << ( GUCEF_NULL != member ? member->name : memberId ) << text << CGucefLobbyProtocol::GetUnixTimeInMs() << ( m_lobbyId + "-" + CORE::ToString( ++m_chatCount ) );
            entry.WriteSegmentEnd();
            if ( !m_chat->CommitEntry() )
                entry.Clear();
            return;
        }
        if ( "kick" == verb )
        {
            CORE::CString target;
            args >> target;
            if ( !m_rules.CheckKick( m_state, memberId, target, reason ) )
            {
                Refuse( memberId, "kick", reason );
                return;
            }
            KickMember( target, "You were kicked", false );
            return;
        }
        if ( "ban" == verb )
        {
            CORE::CString target;
            args >> target;
            if ( !m_rules.CheckKick( m_state, memberId, target, reason ) )
            {
                Refuse( memberId, "ban", reason );
                return;
            }
            m_banned.insert( target );
            KickMember( target, "You were banned from the lobby", false );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: " << memberId << " banned " << target << " from lobby " << m_lobbyId );
            return;
        }
        if ( "report" == verb )
        {
            CORE::CString target, why;
            args >> target >> why;
            if ( !m_chatBuckets[ memberId ].Take( GetTimeInMs() ) )
            {
                Refuse( memberId, "report", "Too many requests, wait a moment" );
                return;
            }
            const CLobbyMember* reported = m_state.FindMember( target );
            if ( GUCEF_NULL == reported || target == memberId )
            {
                Refuse( memberId, "report", "There is no such other member" );
                return;
            }
            const CLobbyMember* reporter = m_state.FindMember( memberId );
            CPlayerReport report;
            report.reporterId = memberId;
            report.reporterName = GUCEF_NULL != reporter ? reporter->name : memberId;
            report.reportedId = target;
            report.reportedName = reported->name;
            TStringMap::const_iterator k = m_memberKeys.find( target );
            if ( k != m_memberKeys.end() )
                report.reportedPublicKey = (*k).second;
            report.reason = why.Length() > GUCEF_MULTIPLAY_LOBBYSERVER_MAX_REASON ? why.SubstrFromRange( 0, GUCEF_MULTIPLAY_LOBBYSERVER_MAX_REASON ) : why;
            report.lobbyId = m_lobbyId;
            report.sessionName = LOBBYPHASE_IN_GAME == m_state.phase ? CGucefLobbyProtocol::MakeGameSessionName( m_lobbyId ) : CGucefLobbyProtocol::MakeLobbySessionName( m_lobbyId );
            report.time = CORE::CDateTime::NowUTCDateTime();
            if ( !m_server.AddReport( report ) )
                Refuse( memberId, "report", "The report could not be kept" );
            return;
        }
        if ( "boss" == verb )
        {
            CORE::CString target;
            args >> target;
            if ( !m_rules.CheckTransferBoss( m_state, memberId, target, reason ) )
            {
                Refuse( memberId, "transferBoss", reason );
                return;
            }
            m_rules.ApplyTransferBoss( m_state, target );
            m_dirty = true;
            return;
        }
        if ( "slotkind" == verb )
        {
            UInt32 slotIndex = 0;
            CORE::CString kind, aiType;
            args >> slotIndex >> kind >> aiType;
            TLobbySlotKind slotKind = StringToLobbySlotKind( kind );
            if ( !m_rules.CheckSlotKind( m_state, memberId, slotIndex, slotKind, reason ) )
            {
                Refuse( memberId, "slotKind", reason );
                return;
            }
            CLobbySlot* slot = m_state.FindSlot( slotIndex );
            slot->kind = slotKind;
            slot->aiType = LOBBYSLOTKIND_AI == slotKind ? aiType : CORE::CString::Empty;
            m_dirty = true;
            return;
        }
        if ( "start" == verb || "forcestart" == verb )
        {
            bool force = "forcestart" == verb;
            bool needsVote = false;
            if ( !m_rules.CheckStart( m_state, memberId, force, needsVote, reason ) )
            {
                Refuse( memberId, force ? "forceStart" : "start", reason );
                return;
            }
            if ( needsVote )
            {
                Refuse( memberId, "start", "Propose the start to the others" );
                return;
            }
            StartCountdown();
            return;
        }
        if ( "launch" == verb )
        {
            // A member that joins a game in progress asks for its launch once it has the lobby
            if ( LOBBYPHASE_IN_GAME == m_state.phase )
                SendLaunch( memberId );
            else
                Refuse( memberId, "launch", "The game has not started" );
            return;
        }
        Refuse( memberId, verb, "There is no such command" );
    }

    /*---------------------------------------------------------------------*/

    void StartCountdown( void )
    {GUCEF_TRACE;

        m_state.phase = LOBBYPHASE_COUNTDOWN;
        m_countdownLeft = (Int32) m_rules.countdownInSecs;
        m_nextCountdownAtMs = GetTimeInMs() + m_server.m_settings.secondInMs;
        m_dirty = true;
        if ( m_countdownLeft < 0 )
            m_countdownLeft = 0;
    }

    void Launch( void )
    {GUCEF_TRACE;

        m_countdownLeft = -1;
        m_state.phase = LOBBYPHASE_LAUNCHING;
        Publish();

        for ( size_t i=0; i<m_state.members.size(); ++i )
            SendLaunch( m_state.members[ i ].id );

        m_state.phase = LOBBYPHASE_IN_GAME;
        m_dirty = true;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: Lobby " << m_lobbyId << " launched its game" );
        CORE::TCloneableString lobbyId( m_lobbyId );
        m_server.NotifyObservers( LobbyLaunchedEvent, &lobbyId );
    }

    CLaunchDescription MakeLaunchDescription( const CORE::CString& memberId ) const
    {GUCEF_TRACE;

        CLaunchDescription launch;
        launch.gameId = m_params.gameId;
        launch.gameVersion = m_params.gameVersion;
        launch.lobbyId = m_lobbyId;
        launch.sessionName = CGucefLobbyProtocol::MakeGameSessionName( m_lobbyId );
        launch.topology = m_params.topology;

        // The host serves, or orders the peers; when it left the boss does
        CORE::CString hostId = GUCEF_NULL != m_state.FindMember( m_creatorId ) && !m_state.FindMember( m_creatorId )->IsSpectator() ? m_creatorId : m_state.bossId;
        if ( GAMETOPOLOGY_PEER_TO_PEER == m_params.topology )
            launch.sequencerMemberId = hostId;
        else
            launch.serverMemberId = hostId;

        for ( size_t i=0; i<m_state.members.size(); ++i )
        {
            const CLobbyMember& member = m_state.members[ i ];
            CLaunchMember launchMember;
            launchMember.id = member.id;
            launchMember.name = member.name;
            launchMember.role = member.role;
            launchMember.slot = member.slot;
            TEndpointMap::const_iterator e = m_memberEndpoints.find( member.id );
            if ( e != m_memberEndpoints.end() )
                launchMember.endpoints = (*e).second;
            TStringMap::const_iterator k = m_memberKeys.find( member.id );
            if ( k != m_memberKeys.end() )
                launchMember.publicKey = (*k).second;
            TIntegrityMap::const_iterator n = m_memberIntegrity.find( member.id );
            if ( n != m_memberIntegrity.end() )
                launchMember.integrity = (*n).second;
            launch.members.push_back( launchMember );
        }
        launch.settings = m_state.settings;
        launch.slots = m_state.slots;
        launch.localMemberId = memberId;

        const CLobbyMember* member = m_state.FindMember( memberId );
        launch.spectator = GUCEF_NULL != member && member->IsSpectator();
        CORE::CString role = launch.spectator ? "spectator" : ( memberId == hostId ? "host" : "player" );
        // Signed by the server: whoever checks them has the public key only, no member can make a token
        launch.securityPolicy = m_gamePolicy;
        launch.serverPublicKey = m_server.GetPublicKey();
        TStringMap::const_iterator key = m_memberKeys.find( memberId );
        launch.token = m_server.IssueGameToken( launch.sessionName, memberId, role, key != m_memberKeys.end() ? (*key).second : CORE::CString::Empty, m_gamePolicy );

        // The relay runs the server's game policy and publishes its hash and the relay's key with its endpoints: the
        // links with the relay run under that policy, the links of the game over it under the game's own
        if ( m_server.m_settings.node.relayEnabled )
        {
            DRN::CDRNEndpointList endpoints = m_server.GetRelayEndpoints();
            for ( UInt32 i=0; i<endpoints.GetCount() && launch.relay.IsNULLOrEmpty(); ++i )
                if ( DRN::CDRNEndpoint::ChannelTypeRelay != endpoints.Get( i ).GetChannelType() )
                    launch.relay = endpoints.Get( i ).ToString();
        }
        return launch;
    }

    void SendLaunch( const CORE::CString& memberId )
    {GUCEF_TRACE;

        UInt32 peerId = GetPeerOfMember( memberId );
        if ( 0 == peerId )
            return;
        CLaunchDescription launch = MakeLaunchDescription( memberId );
        CORE::CDataNode tree( "launch" );
        launch.Serialize( tree, CORE::CDataNodeSerializableSettings() );

        CORE::CDynamicBuffer buffer;
        DRN::CDRNWireWriter writer( buffer );
        writer.WriteUInt8( CGucefLobbyProtocol::MESSAGE_LAUNCH );
        CGucefLobbyProtocol::WriteDataTree( writer, tree );
        m_session->SendAppMessage( peerId, DRN::DRN_LANE_RELIABLE_ORDERED, buffer.GetConstBufferPtr(), writer.GetOffset() );
    }

    /*---------------------------------------------------------------------*/

    void Close( const CORE::CString& reason )
    {GUCEF_TRACE;

        if ( m_closing )
            return;
        m_closing = true;
        m_closeReason = reason;
        m_state.phase = LOBBYPHASE_CLOSED;
        m_closeAtMs = GetTimeInMs() + GUCEF_MULTIPLAY_LOBBYSERVER_CLOSE_MS;
        m_dirty = true;
        Publish();

        TMemberPeerMap members = m_memberToPeer;
        TMemberPeerMap::iterator i = members.begin();
        while ( i != members.end() )
        {
            SendToMember( (*i).first, CGucefLobbyProtocol::MESSAGE_CLOSED, reason, CORE::CString::Empty );
            ++i;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: Lobby " << m_lobbyId << " closes: " << reason );
    }

    /*---------------------------------------------------------------------*/

    CGameListingEntry MakeListing( void ) const
    {GUCEF_TRACE;

        CGameListingEntry listing;
        listing.listingId = m_lobbyId;
        listing.kind = LOBBYPHASE_IN_GAME == m_state.phase ? CGameListingEntry::KIND_GAME_IN_PROGRESS : CGameListingEntry::KIND_LOBBY;
        listing.name = m_state.name;
        listing.gameId = m_params.gameId;
        listing.gameVersion = m_params.gameVersion;
        listing.players = m_state.GetPlayerCount();
        listing.maxPlayers = m_rules.maxPlayers;
        listing.spectators = m_state.GetSpectatorCount();
        listing.passworded = !m_params.password.IsNULLOrEmpty();
        listing.hostName = m_creatorName;
        DRN::CDRNEndpointList endpoints = m_server.GetPinnedEndpoints();
        for ( UInt32 i=0; i<endpoints.GetCount(); ++i )
            listing.endpoints.push_back( endpoints.Get( i ).ToString() );
        listing.backendName = CGucefLobbyProtocol::ServiceName;
        listing.extra.SetAllowMultipleValues( false );
        listing.extra.Set( "lobbyService", CGucefLobbyProtocol::ServiceName );
        // For pinning: the key of the server's node (also the "key" parameter of the endpoints) and the token key
        listing.extra.Set( "serverKey", m_server.GetPublicKey() );
        listing.extra.Set( "nodeKey", m_server.GetNodePublicKey() );
        listing.extra.Set( "policyHash", m_gamePolicy.GetPolicyHashString() );
        listing.lastSeen = CORE::CDateTime::NowUTCDateTime();
        return listing;
    }

    /*---------------------------------------------------------------------*/

    /** Writes the state into the grids, only what changed travels */
    void Publish( void )
    {GUCEF_TRACE;

        m_dirty = false;
        if ( LOBBYPHASE_COUNTDOWN != m_state.phase )
            m_countdownLeft = -1;

        Put( m_lobbyGrid, "lobbyId", CORE::CVariant( m_lobbyId ) );
        Put( m_lobbyGrid, "name", CORE::CVariant( m_state.name ) );
        Put( m_lobbyGrid, "phase", CORE::CVariant( CORE::CString( LobbyPhaseToString( m_state.phase ) ) ) );
        Put( m_lobbyGrid, "boss", CORE::CVariant( m_state.bossId ) );
        Put( m_lobbyGrid, "countdown", CORE::CVariant( m_countdownLeft ) );
        Put( m_lobbyGrid, "gameId", CORE::CVariant( m_params.gameId ) );
        Put( m_lobbyGrid, "gameVersion", CORE::CVariant( m_params.gameVersion ) );
        Put( m_lobbyGrid, "topology", CORE::CVariant( CORE::CString( GameTopologyToString( m_params.topology ) ) ) );
        Put( m_lobbyGrid, "chat/backend", CORE::CVariant( m_state.chatBackend ) );
        Put( m_lobbyGrid, "chat/channel", CORE::CVariant( m_state.chatChannel ) );
        UInt32 settingCount = m_state.settings.GetKeyCount();
        for ( UInt32 i=0; i<settingCount; ++i )
            Put( m_lobbyGrid, "settings/" + m_state.settings.GetKey( i ).AsString(), CORE::CVariant( m_state.settings.GetValue( i ).AsString() ) );

        const CLobbyVote& vote = m_state.activeVote;
        Put( m_lobbyGrid, "vote/id", CORE::CVariant( vote.id ) );
        Put( m_lobbyGrid, "vote/what", CORE::CVariant( vote.what ) );
        Put( m_lobbyGrid, "vote/proposer", CORE::CVariant( vote.proposer ) );
        Put( m_lobbyGrid, "vote/key", CORE::CVariant( vote.key ) );
        Put( m_lobbyGrid, "vote/value", CORE::CVariant( vote.value ) );
        Put( m_lobbyGrid, "vote/yes", CORE::CVariant( vote.yes ) );
        Put( m_lobbyGrid, "vote/no", CORE::CVariant( vote.no ) );
        Put( m_lobbyGrid, "vote/needed", CORE::CVariant( vote.needed ) );
        Put( m_lobbyGrid, "vote/lastId", CORE::CVariant( m_lastVoteId ) );
        Put( m_lobbyGrid, "vote/lastResult", CORE::CVariant( m_lastVoteResult ) );

        CORE::CString::StringSet present;
        for ( size_t i=0; i<m_state.members.size(); ++i )
        {
            const CLobbyMember& member = m_state.members[ i ];
            const CLobbySlot* slot = m_state.FindSlotOfMember( member.id );
            const CORE::CString prefix = member.id + "/";
            present.insert( member.id );
            Put( m_membersGrid, prefix + "name", CORE::CVariant( member.name ) );
            Put( m_membersGrid, prefix + "role", CORE::CVariant( CORE::CString( LobbyMemberRoleToString( member.role ) ) ) );
            Put( m_membersGrid, prefix + "ready", CORE::CVariant( member.ready ) );
            Put( m_membersGrid, prefix + "slot", CORE::CVariant( member.slot ) );
            Put( m_membersGrid, prefix + "latency", CORE::CVariant( member.latencyMs ) );
            Put( m_membersGrid, prefix + "joinOrder", CORE::CVariant( member.joinOrder ) );
            Put( m_membersGrid, prefix + "spectator", CORE::CVariant( member.IsSpectator() ) );
            Put( m_membersGrid, prefix + "faction", CORE::CVariant( GUCEF_NULL != slot ? slot->faction : CORE::CString::Empty ) );
            Put( m_membersGrid, prefix + "colour", CORE::CVariant( GUCEF_NULL != slot ? slot->colour : CORE::CString::Empty ) );
        }
        CORE::CString::StringSet::iterator p = m_publishedMembers.begin();
        while ( p != m_publishedMembers.end() )
        {
            if ( present.find( (*p) ) == present.end() )
            {
                static const char* fields[] = { "name", "role", "ready", "slot", "latency", "joinOrder", "spectator", "faction", "colour" };
                for ( UInt32 f=0; f<sizeof( fields ) / sizeof( fields[ 0 ] ); ++f )
                    m_membersGrid->RemoveKey( CORE::CVariant( (*p) + "/" + fields[ f ] ) );
            }
            ++p;
        }
        m_publishedMembers = present;

        Put( m_slotsGrid, "count", CORE::CVariant( (UInt32) m_state.slots.size() ) );
        for ( size_t i=0; i<m_state.slots.size(); ++i )
        {
            const CLobbySlot& slot = m_state.slots[ i ];
            const CORE::CString prefix = CORE::ToString( (UInt32) i ) + "/";
            Put( m_slotsGrid, prefix + "index", CORE::CVariant( slot.index ) );
            Put( m_slotsGrid, prefix + "team", CORE::CVariant( slot.team ) );
            Put( m_slotsGrid, prefix + "faction", CORE::CVariant( slot.faction ) );
            Put( m_slotsGrid, prefix + "colour", CORE::CVariant( slot.colour ) );
            Put( m_slotsGrid, prefix + "kind", CORE::CVariant( CORE::CString( LobbySlotKindToString( slot.kind ) ) ) );
            Put( m_slotsGrid, prefix + "aiType", CORE::CVariant( slot.aiType ) );
            Put( m_slotsGrid, prefix + "member", CORE::CVariant( slot.memberId ) );
        }

        m_server.PublishListing( *this );
    }

    /*---------------------------------------------------------------------*/

    void OnTick( UInt64 nowMs )
    {GUCEF_TRACE;

        if ( m_closed )
            return;
        if ( m_closing )
        {
            if ( nowMs >= m_closeAtMs )
            {
                m_closed = true;
                if ( GUCEF_NULL != m_session )
                    m_session->Leave();
            }
            return;
        }

        if ( LOBBYPHASE_COUNTDOWN == m_state.phase && m_countdownLeft >= 0 && nowMs >= m_nextCountdownAtMs )
        {
            if ( 0 == m_countdownLeft )
            {
                // The 0 went out on the tick before, so every member saw it
                Launch();
            }
            else
            {
                --m_countdownLeft;
                UInt64 untilNext = m_server.m_settings.secondInMs;
                if ( 0 == m_countdownLeft && untilNext > 4 * GUCEF_MULTIPLAY_LOBBYSERVER_TICK_MS )
                    untilNext = 4 * GUCEF_MULTIPLAY_LOBBYSERVER_TICK_MS;
                m_nextCountdownAtMs = nowMs + untilNext;
                m_dirty = true;
            }
        }

        if ( nowMs >= m_nextLatencyAtMs )
        {
            m_nextLatencyAtMs = nowMs + m_server.m_settings.latencyIntervalInMs;
            DRN::CDRNDataGrid* latency = m_session->FindGrid( DRN::CDRNSession::LatencyGridName );
            if ( GUCEF_NULL != latency )
            {
                for ( size_t i=0; i<m_state.members.size(); ++i )
                {
                    CLobbyMember& member = m_state.members[ i ];
                    CORE::CVariant rtt;
                    UInt32 peerId = GetPeerOfMember( member.id );
                    if ( 0 != peerId && latency->GetValue( CORE::CVariant( peerId ), rtt ) )
                    {
                        Int32 latencyMs = (Int32) ( rtt.AsUInt32( 0, true ) / 1000 );
                        if ( latencyMs != member.latencyMs )
                        {
                            member.latencyMs = latencyMs;
                            m_dirty = true;
                        }
                    }
                }
            }
        }

        if ( m_dirty )
            Publish();
    }

    /*---------------------------------------------------------------------*/

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( notifier == m_session )
        {
            if ( DRN::CDRNSession::MemberJoinedEvent == eventId )
            {
                const DRN::CDRNSessionMemberInfo& info = static_cast< DRN::CDRNSessionMemberEventData* >( eventData )->GetData();
                if ( !info.isLocal )
                    OnMemberJoined( info.peerId );
            }
            else
            if ( DRN::CDRNSession::MemberLeftEvent == eventId )
            {
                const DRN::CDRNSessionMemberInfo& info = static_cast< DRN::CDRNSessionMemberEventData* >( eventData )->GetData();
                OnMemberLeft( info.peerId );
            }
            else
            if ( DRN::CDRNSession::ItemAnnouncedEvent == eventId )
            {
                const DRN::CDRNItemInfo& info = static_cast< DRN::CDRNItemInfoEventData* >( eventData )->GetData();
                if ( info.name == CGucefLobbyProtocol::CommandsStream )
                {
                    DRN::CDRNDataStream* stream = m_session->FindStream( info.name, info.ownerPeerId );
                    if ( GUCEF_NULL != stream )
                        SubscribeTo( stream );
                }
            }
            return;
        }

        if ( notifier == m_proposalsGrid )
        {
            if ( DRN::CDRNDataGrid::VoteStartedEvent == eventId )
                OnVoteStarted( static_cast< DRN::CDRNVoteEventData* >( eventData )->GetData() );
            else
            if ( DRN::CDRNDataGrid::VoteEndedEvent == eventId )
                OnVoteEnded( static_cast< DRN::CDRNVoteEventData* >( eventData )->GetData() );
            return;
        }

        if ( DRN::CDRNDataStream::EntriesAvailableEvent == eventId )
        {
            DRN::CDRNDataStream* stream = static_cast< DRN::CDRNDataStream* >( notifier );
            OnCommands( *stream );
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CGucefLobbyServer::CAccessPolicy::MayJoin( const DRN::CDRNPeerLink& link, const CORE::CString& sessionName )
{GUCEF_TRACE;

    if ( sessionName == CGucefLobbyProtocol::DirectorySessionName )
        return true;
    CHostedLobby* lobby = m_server->FindLobbyBySession( sessionName );
    if ( GUCEF_NULL == lobby )
        return false;
    const DRN::CDRNAuthResult& auth = link.GetAuthResult();
    return auth.sessionName == sessionName && lobby->HasAdmission( auth.userId );
}

/*-------------------------------------------------------------------------*/

CGucefLobbyServerSettings::CGucefLobbyServerSettings( void )
    : CORE::CIConfigurable()
    , node()
    , keyFile()
    , password()
    , requireIdentityProof( true )
    , banFile()
    , reportFile()
    , minimumSecurityPolicy()
    , chatMessagesPerSecond( 2 )
    , chatBurst( 10 )
    , requestsPerSecond( 10 )
    , requestsBurst( 40 )
    , expectedIntegrity()
    , maxLobbies( 0 )
    , maxMembersPerLobby( 0 )
    , tokenLifetimeInSecs( 3600 )
    , secondInMs( 1000 )
    , latencyIntervalInMs( 1000 )
    , advertiseOnLan( false )
    , advertiserConfig()
    , rules()
    , relayListen()
    , gameServiceName( DRN::CDRNNodeSettings().serviceName )
{GUCEF_TRACE;

    node.serviceName = CGucefLobbyProtocol::ServiceName;

    // The minimum asks for encryption only: a game sets the limits it needs
    minimumSecurityPolicy.limits.maxMessageSize = 0;
    minimumSecurityPolicy.limits.maxPreAuthPacketSize = 0;
    minimumSecurityPolicy.limits.maxMessagesPerSecond = 0;
    minimumSecurityPolicy.limits.maxBytesPerSecond = 0;
    minimumSecurityPolicy.limits.sustainedLimitSeconds = 0;
    minimumSecurityPolicy.limits.maxPendingHandshakes = 0;
    minimumSecurityPolicy.limits.maxHandshakesPerAddressPerMinute = 0;
    minimumSecurityPolicy.limits.maxFragmentGroups = 0;
    minimumSecurityPolicy.limits.maxFragmentBytes = 0;
    minimumSecurityPolicy.statelessCookies = false;
    minimumSecurityPolicy.ciphers.clear();
}

/*-------------------------------------------------------------------------*/

CGucefLobbyServerSettings::CGucefLobbyServerSettings( const CGucefLobbyServerSettings& src )
    : CORE::CIConfigurable( src )
    , node( src.node )
    , keyFile( src.keyFile )
    , password( src.password )
    , requireIdentityProof( src.requireIdentityProof )
    , banFile( src.banFile )
    , reportFile( src.reportFile )
    , minimumSecurityPolicy( src.minimumSecurityPolicy )
    , chatMessagesPerSecond( src.chatMessagesPerSecond )
    , chatBurst( src.chatBurst )
    , requestsPerSecond( src.requestsPerSecond )
    , requestsBurst( src.requestsBurst )
    , expectedIntegrity( src.expectedIntegrity )
    , maxLobbies( src.maxLobbies )
    , maxMembersPerLobby( src.maxMembersPerLobby )
    , tokenLifetimeInSecs( src.tokenLifetimeInSecs )
    , secondInMs( src.secondInMs )
    , latencyIntervalInMs( src.latencyIntervalInMs )
    , advertiseOnLan( src.advertiseOnLan )
    , advertiserConfig( src.advertiserConfig )
    , rules( src.rules )
    , relayListen( src.relayListen )
    , gameServiceName( src.gameServiceName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGucefLobbyServerSettings::~CGucefLobbyServerSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGucefLobbyServerSettings&
CGucefLobbyServerSettings::operator=( const CGucefLobbyServerSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        node = src.node;
        keyFile = src.keyFile;
        password = src.password;
        requireIdentityProof = src.requireIdentityProof;
        banFile = src.banFile;
        reportFile = src.reportFile;
        minimumSecurityPolicy = src.minimumSecurityPolicy;
        chatMessagesPerSecond = src.chatMessagesPerSecond;
        chatBurst = src.chatBurst;
        requestsPerSecond = src.requestsPerSecond;
        requestsBurst = src.requestsBurst;
        expectedIntegrity = src.expectedIntegrity;
        maxLobbies = src.maxLobbies;
        maxMembersPerLobby = src.maxMembersPerLobby;
        tokenLifetimeInSecs = src.tokenLifetimeInSecs;
        secondInMs = src.secondInMs;
        latencyIntervalInMs = src.latencyIntervalInMs;
        advertiseOnLan = src.advertiseOnLan;
        advertiserConfig = src.advertiserConfig;
        rules = src.rules;
        relayListen = src.relayListen;
        gameServiceName = src.gameServiceName;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServerSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "keyFile", keyFile );
    config.SetAttribute( "password", password );
    config.SetAttribute( "requireIdentityProof", requireIdentityProof );
    config.SetAttribute( "banFile", banFile );
    config.SetAttribute( "reportFile", reportFile );
    config.SetAttribute( "chatMessagesPerSecond", chatMessagesPerSecond );
    config.SetAttribute( "chatBurst", chatBurst );
    config.SetAttribute( "requestsPerSecond", requestsPerSecond );
    config.SetAttribute( "requestsBurst", requestsBurst );
    minimumSecurityPolicy.SaveConfig( *config.AddChild( "MinimumSecurityPolicy", GUCEF_DATATYPE_OBJECT ) );
    WriteValueListAsChild( config, "ExpectedIntegrity", expectedIntegrity );
    config.SetAttribute( "maxLobbies", maxLobbies );
    config.SetAttribute( "maxMembersPerLobby", maxMembersPerLobby );
    config.SetAttribute( "tokenLifetimeInSecs", tokenLifetimeInSecs );
    config.SetAttribute( "secondInMs", secondInMs );
    config.SetAttribute( "latencyIntervalInMs", latencyIntervalInMs );
    config.SetAttribute( "advertiseOnLan", advertiseOnLan );
    config.SetAttribute( "listen", node.listenEndpoints.ToString() );
    config.SetAttribute( "relayEnabled", node.relayEnabled );
    config.SetAttribute( "relayListen", relayListen.ToString() );
    config.SetAttribute( "gameServiceName", gameServiceName );
    node.SaveConfig( *config.AddChild( "Node", GUCEF_DATATYPE_OBJECT ) );
    rules.SaveConfig( *config.AddChild( "Rules", GUCEF_DATATYPE_OBJECT ) );
    CORE::CDataNode advertiser( advertiserConfig );
    advertiser.SetName( "Advertiser" );
    config.AddChild( advertiser );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServerSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    keyFile = config.GetAttributeValueOrChildValueByName( "keyFile" ).AsString( keyFile, true );
    password = config.GetAttributeValueOrChildValueByName( "password" ).AsString( password, true );
    requireIdentityProof = config.GetAttributeValueOrChildValueByName( "requireIdentityProof" ).AsBool( requireIdentityProof, true );
    banFile = config.GetAttributeValueOrChildValueByName( "banFile" ).AsString( banFile, true );
    reportFile = config.GetAttributeValueOrChildValueByName( "reportFile" ).AsString( reportFile, true );
    chatMessagesPerSecond = config.GetAttributeValueOrChildValueByName( "chatMessagesPerSecond" ).AsUInt32( chatMessagesPerSecond, true );
    chatBurst = config.GetAttributeValueOrChildValueByName( "chatBurst" ).AsUInt32( chatBurst, true );
    requestsPerSecond = config.GetAttributeValueOrChildValueByName( "requestsPerSecond" ).AsUInt32( requestsPerSecond, true );
    requestsBurst = config.GetAttributeValueOrChildValueByName( "requestsBurst" ).AsUInt32( requestsBurst, true );
    const CORE::CDataNode* minimumPolicy = config.FindChild( "MinimumSecurityPolicy" );
    if ( GUCEF_NULL != minimumPolicy && !minimumSecurityPolicy.LoadConfig( *minimumPolicy ) )
        return false;
    if ( GUCEF_NULL != config.FindChild( "ExpectedIntegrity" ) )
        ReadValueListFromChild( config, "ExpectedIntegrity", expectedIntegrity );
    maxLobbies = config.GetAttributeValueOrChildValueByName( "maxLobbies" ).AsUInt32( maxLobbies, true );
    maxMembersPerLobby = config.GetAttributeValueOrChildValueByName( "maxMembersPerLobby" ).AsUInt32( maxMembersPerLobby, true );
    tokenLifetimeInSecs = config.GetAttributeValueOrChildValueByName( "tokenLifetimeInSecs" ).AsUInt32( tokenLifetimeInSecs, true );
    secondInMs = config.GetAttributeValueOrChildValueByName( "secondInMs" ).AsUInt32( secondInMs, true );
    latencyIntervalInMs = config.GetAttributeValueOrChildValueByName( "latencyIntervalInMs" ).AsUInt32( latencyIntervalInMs, true );
    advertiseOnLan = config.GetAttributeValueOrChildValueByName( "advertiseOnLan" ).AsBool( advertiseOnLan, true );

    const CORE::CDataNode* nodeConfig = config.FindChild( "Node" );
    if ( GUCEF_NULL != nodeConfig && !node.LoadConfig( *nodeConfig ) )
        return false;
    // The short forms, easier to write in a config file than the node's own settings
    CORE::CString listen = config.GetAttributeValueOrChildValueByName( "listen" ).AsString( CORE::CString::Empty, true );
    if ( !listen.IsNULLOrEmpty() && !node.listenEndpoints.FromString( listen ) )
        return false;
    node.relayEnabled = config.GetAttributeValueOrChildValueByName( "relayEnabled" ).AsBool( node.relayEnabled, true );
    CORE::CString relayListenText = config.GetAttributeValueOrChildValueByName( "relayListen" ).AsString( CORE::CString::Empty, true );
    if ( !relayListenText.IsNULLOrEmpty() && !relayListen.FromString( relayListenText ) )
        return false;
    gameServiceName = config.GetAttributeValueOrChildValueByName( "gameServiceName" ).AsString( gameServiceName, true );
    if ( node.serviceName.IsNULLOrEmpty() || "drn" == node.serviceName )
        node.serviceName = CGucefLobbyProtocol::ServiceName;
    const CORE::CDataNode* rulesConfig = config.FindChild( "Rules" );
    if ( GUCEF_NULL != rulesConfig )
        rules.LoadConfig( *rulesConfig );
    const CORE::CDataNode* advertiser = config.FindChild( "Advertiser" );
    if ( GUCEF_NULL != advertiser )
        advertiserConfig = *advertiser;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobbyServerSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGucefLobbyServerSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::RegisterEvents( void )
{GUCEF_TRACE;

    LobbyCreatedEvent.Initialize();
    LobbyClosedEvent.Initialize();
    LobbyLaunchedEvent.Initialize();
    PlayerReportedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CGucefLobbyServer::CGucefLobbyServer( void )
    : CORE::CObservingNotifier()
    , m_settings()
    , m_node()
    , m_relayNode()
    , m_accessPolicy( GUCEF_NULL )
    , m_directory( GUCEF_NULL )
    , m_listings( GUCEF_NULL )
    , m_timer( GUCEF_MULTIPLAY_LOBBYSERVER_TICK_MS )
    , m_lobbies()
    , m_closedLobbies()
    , m_clients()
    , m_channels()
    , m_lobbyCount( 0 )
    , m_messageCount( 0 )
    , m_running( false )
    , m_haveKeys( false )
    , m_banList()
    , m_reports()
{GUCEF_TRACE;

    memset( m_privateKey, 0, sizeof( m_privateKey ) );
    memset( m_publicKey, 0, sizeof( m_publicKey ) );
    memset( m_nodePrivateKey, 0, sizeof( m_nodePrivateKey ) );
    memset( m_nodePublicKey, 0, sizeof( m_nodePublicKey ) );

    // The authenticator of the directory and the lobbies is registered by the global systems of the module
    CMultiplayGlobal::Instance();

    RegisterEvents();
    m_accessPolicy = GUCEF_NEW CAccessPolicy( this );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CGucefLobbyServer::~CGucefLobbyServer()
{GUCEF_TRACE;

    Stop();
    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_accessPolicy;
    m_accessPolicy = GUCEF_NULL;
    memset( m_privateKey, 0, sizeof( m_privateKey ) );
    memset( m_nodePrivateKey, 0, sizeof( m_nodePrivateKey ) );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::SetSettings( const CGucefLobbyServerSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const CGucefLobbyServerSettings&
CGucefLobbyServer::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNNode&
CGucefLobbyServer::GetNode( void )
{GUCEF_TRACE;

    return m_node;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServer::Start( void )
{GUCEF_TRACE;

    if ( m_running )
        return true;

    if ( !LoadKeys() )
        return false;

    m_banList.SetFilePath( CORE::CString::Empty );
    if ( !m_settings.banFile.IsNULLOrEmpty() )
    {
        CORE::CString banFile = CORE::RelativePath( m_settings.banFile );
        if ( !m_banList.LoadFromFile( banFile ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: The bans in " << banFile << " cannot be read" );
            return false;
        }
        m_banList.SetFilePath( banFile );
    }
    LoadReports();

    DRN::CDRNNodeSettings nodeSettings( m_settings.node );
    if ( nodeSettings.serviceName.IsNULLOrEmpty() )
        nodeSettings.serviceName = CGucefLobbyProtocol::ServiceName;
    // The node's static key follows from the server's secret, unless the node settings name one of their own
    if ( nodeSettings.staticPrivateKey.IsNULLOrEmpty() && nodeSettings.staticKeyFile.IsNULLOrEmpty() )
        nodeSettings.staticPrivateKey = DRN::CDRNCrypto::ToHex( m_nodePrivateKey, sizeof( m_nodePrivateKey ) );
    CGucefLobbyAuthenticator::MakeServerSettings( GetPublicKey(), m_settings.password, m_settings.requireIdentityProof, nodeSettings.auth );
    nodeSettings.relayEnabled = false;      // the relay for the game sessions is a node of its own, see StartRelay()
    if ( !m_node.SetSettings( nodeSettings ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: The node settings were not accepted" );
        return false;
    }
    m_node.SetAccessPolicy( m_accessPolicy );
    m_node.SetBanList( &m_banList );
    if ( !m_node.Start() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: Could not listen on " << m_settings.node.listenEndpoints.ToString() );
        return false;
    }
    if ( m_settings.node.relayEnabled && !StartRelay() )
    {
        m_node.Stop();
        return false;
    }

    m_directory = m_node.CreateSession();
    DRN::CDRNSessionSettings directorySettings;
    directorySettings.sessionName = CGucefLobbyProtocol::DirectorySessionName;
    directorySettings.topology = DRN::DRN_TOPOLOGY_CLIENT_SERVER;
    if ( GUCEF_NULL == m_directory || !m_directory->Host( directorySettings ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: Could not host the directory" );
        m_relayNode.Stop();
        m_node.Stop();
        return false;
    }
    m_listings = CreateHostGrid( *m_directory, CGucefLobbyProtocol::ListingsGrid, DRN::DRN_AUTHORITY_HOST );
    m_directory->Subscribe( CGucefLobbyProtocol::RequestsStream );
    SubscribeTo( m_directory );

    m_running = true;
    m_timer.SetEnabled( true );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: Serving at " << GetEndpoints().ToString() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::Stop( void )
{GUCEF_TRACE;

    if ( !m_running )
        return;
    m_running = false;
    m_timer.SetEnabled( false );

    TLobbyMap::iterator i = m_lobbies.begin();
    while ( i != m_lobbies.end() )
    {
        GUCEF_DELETE (*i).second;
        ++i;
    }
    m_lobbies.clear();
    for ( size_t n=0; n<m_closedLobbies.size(); ++n )
        GUCEF_DELETE m_closedLobbies[ n ];
    m_closedLobbies.clear();
    m_clients.clear();
    m_channels.clear();
    if ( GUCEF_NULL != m_directory )
        UnsubscribeFrom( m_directory );
    m_directory = GUCEF_NULL;
    m_listings = GUCEF_NULL;
    m_relayNode.Stop();
    m_node.Stop();
    m_node.SetAccessPolicy( GUCEF_NULL );
    m_node.SetBanList( GUCEF_NULL );
    m_relayNode.SetBanList( GUCEF_NULL );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: Stopped" );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServer::IsRunning( void ) const
{GUCEF_TRACE;

    return m_running;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpointList
CGucefLobbyServer::GetEndpoints( void ) const
{GUCEF_TRACE;

    return m_node.GetAdvertisedEndpoints();
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpointList
CGucefLobbyServer::GetRelayEndpoints( void ) const
{GUCEF_TRACE;

    if ( !m_running || !m_settings.node.relayEnabled )
        return DRN::CDRNEndpointList();
    return m_relayNode.GetPublishedEndpoints();
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServer::StartRelay( void )
{GUCEF_TRACE;

    // The nodes of the game sessions are of the game service and authenticate with the tokens of their launch
    // descriptions; every one of them may bind, the realm is its game session
    DRN::CDRNNodeSettings relaySettings( m_settings.node );
    relaySettings.nodeId = DRN::CDRNNodeSettings::GenerateNodeId();
    relaySettings.serviceName = m_settings.gameServiceName.IsNULLOrEmpty() ? DRN::CDRNNodeSettings().serviceName : m_settings.gameServiceName;
    relaySettings.relayEnabled = true;
    relaySettings.relayToken.Clear();
    relaySettings.securityPolicy = MakeGamePolicy( m_settings.node.securityPolicy );
    relaySettings.staticPrivateKey.Clear();
    relaySettings.staticKeyFile.Clear();
    CGucefLobbyRelayAuthenticator::MakeRelaySettings( GetPublicKey(), relaySettings.auth );

    relaySettings.listenEndpoints = m_settings.relayListen;
    if ( relaySettings.listenEndpoints.IsEmpty() )
    {
        // By default next to the server: on the port after each of its own
        DRN::CDRNEndpointList endpoints = m_node.GetListenEndpoints();
        for ( UInt32 i=0; i<endpoints.GetCount(); ++i )
        {
            const DRN::CDRNEndpoint& endpoint = endpoints.Get( i );
            if ( DRN::CDRNEndpoint::ChannelTypeRelay == endpoint.GetChannelType() )
                continue;
            CORE::CString address = endpoint.GetAddress();
            Int32 colon = address.HasChar( ':', false );
            if ( colon <= 0 )
                continue;
            UInt32 port = CORE::StringToUInt32( address.SubstrFromRange( (UInt32) colon + 1, address.Length() ), 0 );
            if ( 0 == port || port >= 65535 )
                continue;
            relaySettings.listenEndpoints.Add( DRN::CDRNEndpoint( endpoint.GetChannelType(), address.SubstrFromRange( 0, (UInt32) colon ) + ":" + CORE::ToString( port + 1 ) ) );
        }
    }

    m_relayNode.SetBanList( &m_banList );
    if ( !m_relayNode.SetSettings( relaySettings ) || !m_relayNode.Start() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: Could not offer the relay at " << relaySettings.listenEndpoints.ToString() );
        m_relayNode.Stop();
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: The relay for the game sessions is at " << m_relayNode.GetListenEndpoints().ToString() );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CGucefLobbyServer::GetLobbyCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_lobbies.size();
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::GetLobbyIds( CORE::CString::StringVector& lobbyIds ) const
{GUCEF_TRACE;

    TLobbyMap::const_iterator i = m_lobbies.begin();
    while ( i != m_lobbies.end() )
    {
        lobbyIds.push_back( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServer::GetLobbyState( const CORE::CString& lobbyId, CLobbyState& state ) const
{GUCEF_TRACE;

    CHostedLobby* lobby = FindLobby( lobbyId );
    if ( GUCEF_NULL == lobby )
        return false;
    state = lobby->m_state;
    return true;
}

/*-------------------------------------------------------------------------*/

CGucefLobbyServer::CHostedLobby*
CGucefLobbyServer::FindLobby( const CORE::CString& lobbyId ) const
{GUCEF_TRACE;

    TLobbyMap::const_iterator i = m_lobbies.find( lobbyId );
    return i == m_lobbies.end() ? GUCEF_NULL : (*i).second;
}

/*-------------------------------------------------------------------------*/

CGucefLobbyServer::CHostedLobby*
CGucefLobbyServer::FindLobbyBySession( const CORE::CString& sessionName ) const
{GUCEF_TRACE;

    if ( 0 != sessionName.HasSubstr( CGucefLobbyProtocol::LobbySessionPrefix, true ) )
        return GUCEF_NULL;
    return FindLobby( sessionName.CutChars( CGucefLobbyProtocol::LobbySessionPrefix.Length(), true ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGucefLobbyServer::IssueToken( const CORE::CString& sessionName, const CORE::CString& userId, const CORE::CString& role, const CORE::CString& userPublicKey ) const
{GUCEF_TRACE;

    // A lobby session is on the server's own node, of its policy
    return IssueGameToken( sessionName, userId, role, userPublicKey, m_settings.node.securityPolicy );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGucefLobbyServer::IssueGameToken( const CORE::CString& sessionName  ,
                                   const CORE::CString& userId       ,
                                   const CORE::CString& role         ,
                                   const CORE::CString& userPublicKey,
                                   const DRN::CDRNSecurityPolicy& policy ) const
{GUCEF_TRACE;

    if ( !m_haveKeys )
        return CORE::CString::Empty;
    UInt8 userKey[ DRN::CDRNCrypto::KeySize ];
    bool haveUserKey = DRN::CDRNCrypto::FromHex( userPublicKey, userKey, sizeof( userKey ) );
    UInt8 policyHash[ DRN::CDRNSecurityPolicy::PolicyHashSize ];
    policy.GetPolicyHash( policyHash );
    return DRN::CDRNTokenAuthenticator::IssueSignedToken( m_privateKey, sessionName, userId, role,
                                                          DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + m_settings.tokenLifetimeInSecs,
                                                          haveUserKey ? userKey : GUCEF_NULL, policyHash );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServer::LoadKeys( void )
{GUCEF_TRACE;

    if ( !DRN::CDRNCrypto::IsSignatureAvailable() || !DRN::CDRNCrypto::IsKeyAgreementAvailable() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: The server signs its tokens with \"ed25519\" and its node needs \"x25519\": load a codec plugin that has them, such as codecspluginMONOCYPHER" );
        return false;
    }

    // One secret, from which the key that signs the tokens and the node's static key follow
    UInt8 secret[ DRN::CDRNCrypto::KeySize ];
    bool haveSecret = false;
    CORE::CString keyFile = m_settings.keyFile.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::RelativePath( m_settings.keyFile );
    if ( !keyFile.IsNULLOrEmpty() && CORE::FileExists( keyFile ) )
    {
        CORE::CString content;
        haveSecret = CORE::LoadTextFileAsString( keyFile, content ) && DRN::CDRNCrypto::FromHex( content.RemoveChar( '\r' ).RemoveChar( '\n' ).Trim( true ).Trim( false ), secret, sizeof( secret ) );
        if ( !haveSecret )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: " << keyFile << " holds no key" );
            return false;
        }
    }
    if ( !haveSecret )
    {
        if ( !DRN::CDRNCrypto::RandomBytes( secret, sizeof( secret ) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: There is no randomness to make a key with" );
            return false;
        }
        if ( !keyFile.IsNULLOrEmpty() )
        {
            CORE::CreateDirs( CORE::StripFilename( keyFile ) );
            if ( !CORE::WriteStringAsTextFile( keyFile, DRN::CDRNCrypto::ToHex( secret, sizeof( secret ) ) + "\n" ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: The key cannot be written to " << keyFile );
                memset( secret, 0, sizeof( secret ) );
                return false;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: Made a new key in " << keyFile );
        }
    }

    static const char salt[] = "gucef-lobby-server";
    static const char signingInfo[] = "ed25519 token key";
    static const char nodeInfo[] = "x25519 node key";
    bool derived = DRN::CDRNCrypto::Hkdf( salt, sizeof( salt ) - 1, secret, sizeof( secret ), signingInfo, sizeof( signingInfo ) - 1, m_privateKey, sizeof( m_privateKey ) ) &&
                   DRN::CDRNCrypto::Hkdf( salt, sizeof( salt ) - 1, secret, sizeof( secret ), nodeInfo, sizeof( nodeInfo ) - 1, m_nodePrivateKey, sizeof( m_nodePrivateKey ) ) &&
                   DRN::CDRNCrypto::GetSignaturePublicKey( m_privateKey, m_publicKey ) &&
                   DRN::CDRNCrypto::GetKeyAgreementPublicKey( m_nodePrivateKey, m_nodePublicKey );
    memset( secret, 0, sizeof( secret ) );
    m_haveKeys = derived;
    if ( !derived )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: The keys of the server could not be made" );
    return derived;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGucefLobbyServer::GetPublicKey( void ) const
{GUCEF_TRACE;

    return m_haveKeys ? DRN::CDRNCrypto::ToHex( m_publicKey, sizeof( m_publicKey ) ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGucefLobbyServer::GetNodePublicKey( void ) const
{GUCEF_TRACE;

    // The node takes the key of its own settings when they name one, that is the key it has then
    if ( !m_haveKeys || !m_settings.node.staticPrivateKey.IsNULLOrEmpty() || !m_settings.node.staticKeyFile.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    return DRN::CDRNCrypto::ToHex( m_nodePublicKey, sizeof( m_nodePublicKey ) );
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpointList
CGucefLobbyServer::GetPinnedEndpoints( void ) const
{GUCEF_TRACE;

    DRN::CDRNEndpointList endpoints = GetEndpoints();
    CORE::CString nodeKey = GetNodePublicKey();
    if ( nodeKey.IsNULLOrEmpty() )
        return endpoints;
    DRN::CDRNEndpointList pinned;
    for ( UInt32 i=0; i<endpoints.GetCount(); ++i )
    {
        DRN::CDRNEndpoint endpoint = endpoints.Get( i );
        endpoint.GetParams().SetAllowMultipleValues( false );
        endpoint.GetParams().Set( "key", nodeKey );
        pinned.Add( endpoint );
    }
    return pinned;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNBanList&
CGucefLobbyServer::GetBanList( void )
{GUCEF_TRACE;

    return m_banList;
}

/*-------------------------------------------------------------------------*/

CGucefLobbyServer::TDirectoryClient*
CGucefLobbyServer::FindClientOfUser( const CORE::CString& userId, UInt32* peerId )
{GUCEF_TRACE;

    TClientMap::iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        if ( (*i).second.userId == userId )
        {
            if ( GUCEF_NULL != peerId )
                *peerId = (*i).first;
            return &(*i).second;
        }
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServer::KickAndBan( const CORE::CString& userId, const CORE::CString& reason, bool byAddress, UInt64 expiryUnixSeconds )
{GUCEF_TRACE;

    if ( userId.IsNULLOrEmpty() )
        return false;

    m_banList.Ban( DRN::DRN_BAN_USER_ID, userId, expiryUnixSeconds, reason );
    UInt32 peerId = 0;
    TDirectoryClient* client = FindClientOfUser( userId, &peerId );
    if ( GUCEF_NULL != client )
    {
        if ( !client->publicKey.IsNULLOrEmpty() )
            m_banList.Ban( DRN::DRN_BAN_PUBLIC_KEY, client->publicKey.Lowercase(), expiryUnixSeconds, reason );
        if ( byAddress && !client->address.IsNULLOrEmpty() )
            m_banList.Ban( DRN::DRN_BAN_ADDRESS, client->address, expiryUnixSeconds, reason );
    }

    // Out of every lobby, and off the server
    TLobbyMap::iterator i = m_lobbies.begin();
    while ( i != m_lobbies.end() )
    {
        if ( (*i).second->HasUser( userId ) )
            (*i).second->KickMember( userId, "You were banned: " + reason, true );
        ++i;
    }
    if ( GUCEF_NULL != client && GUCEF_NULL != m_directory )
    {
        DRN::CDRNPeerLinkPtr link = m_directory->GetLinkToMember( peerId );
        if ( !link.IsNULL() )
            link->Close( DRN::DRN_LINKREASON_CLOSED_BY_APP, "banned: " + reason );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: Banned " << userId << ( byAddress ? " and its address" : "" ) << ": " << reason );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::GetReports( TPlayerReportVector& reports ) const
{GUCEF_TRACE;

    reports = m_reports;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::LoadReports( void )
{GUCEF_TRACE;

    m_reports.clear();
    if ( m_settings.reportFile.IsNULLOrEmpty() )
        return;
    CORE::CString path = CORE::RelativePath( m_settings.reportFile );
    if ( !CORE::FileExists( path ) )
        return;
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    CORE::CString codecName = CORE::ExtractFileExtention( path ).Lowercase();
    CORE::CDataNode root;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() ||
         !codec->BuildDataTree( &root, path ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: The report log " << path << " cannot be read" );
        return;
    }
    const CORE::CDataNode* list = root.FindChild( "reports" );
    if ( GUCEF_NULL == list && root.GetName() == "reports" )
        list = &root;
    if ( GUCEF_NULL == list )
        return;
    CORE::CDataNode::const_iterator i = list->ConstBegin();
    while ( i != list->ConstEnd() )
    {
        CPlayerReport report;
        if ( report.Deserialize( *(*i), CORE::CDataNodeSerializableSettings() ) )
            m_reports.push_back( report );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServer::AddReport( const CPlayerReport& report )
{GUCEF_TRACE;

    m_reports.push_back( report );
    if ( m_reports.size() > GUCEF_MULTIPLAY_LOBBYSERVER_MAX_REPORTS )
        m_reports.erase( m_reports.begin() );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: " << report.reporterName << " reported " << report.reportedName << " (" << report.reportedId << ") in lobby " << report.lobbyId << ": " << report.reason );

    bool kept = true;
    if ( !m_settings.reportFile.IsNULLOrEmpty() )
    {
        CORE::CString path = CORE::RelativePath( m_settings.reportFile );
        CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
        CORE::CString codecName = CORE::ExtractFileExtention( path ).Lowercase();
        kept = CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) && !codec.IsNULL();
        if ( kept )
        {
            CORE::CDataNode root( "ReportLog", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* list = root.AddChild( "reports", GUCEF_DATATYPE_ARRAY );
            for ( size_t i=0; i<m_reports.size(); ++i )
                m_reports[ i ].Serialize( *list->AddChild( "report", GUCEF_DATATYPE_OBJECT ), CORE::CDataNodeSerializableSettings() );
            CORE::CreateDirs( CORE::StripFilename( path ) );
            kept = codec->StoreDataTree( &root, path );
        }
        if ( !kept )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: The report log " << path << " cannot be written" );
    }

    CPlayerReport eventData( report );
    NotifyObservers( PlayerReportedEvent, &eventData );
    return kept;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSecurityPolicy
CGucefLobbyServer::MakeGamePolicy( const DRN::CDRNSecurityPolicy& lobbyPolicy )
{GUCEF_TRACE;

    DRN::CDRNSecurityPolicy policy( lobbyPolicy );
    policy.requireSignedTokens = true;
    policy.requireTokenPolicyHash = true;
    return policy;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::PublishListing( CHostedLobby& lobby )
{GUCEF_TRACE;

    CGameListingEntry listing = lobby.MakeListing();
    if ( LOBBYPHASE_CLOSED == lobby.m_state.phase )
    {
        WithdrawListing( listing.listingId );
        if ( GUCEF_NULL != lobby.m_advertiser )
            lobby.m_advertiser->Withdraw();
        return;
    }

    CORE::CDataNode tree( "listing" );
    listing.Serialize( tree, CORE::CDataNodeSerializableSettings() );
    if ( GUCEF_NULL != m_listings )
        Put( m_listings, listing.listingId, CGucefLobbyProtocol::DataTreeToVariant( tree ) );

    if ( GUCEF_NULL != lobby.m_advertiser )
    {
        if ( lobby.m_advertiser->IsAdvertising() )
            lobby.m_advertiser->UpdateListing( listing );
        else
            lobby.m_advertiser->Advertise( listing );
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::WithdrawListing( const CORE::CString& listingId )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_listings && m_listings->HasKey( CORE::CVariant( listingId ) ) )
        m_listings->RemoveKey( CORE::CVariant( listingId ) );
}

/*-------------------------------------------------------------------------*/

DRN::CDRNDataStream*
CGucefLobbyServer::GetChannel( const CORE::CString& channel )
{GUCEF_TRACE;

    TChannelMap::iterator i = m_channels.find( channel );
    if ( i != m_channels.end() )
        return (*i).second;
    if ( GUCEF_NULL == m_directory )
        return GUCEF_NULL;

    DRN::CDRNDataStreamSettings settings;
    settings.name = CGucefLobbyProtocol::ChannelStreamPrefix + channel;
    settings.retainedHistory = 50;
    settings.backlogLimit = 1000;
    DRN::CDRNDataStream* stream = m_directory->CreateStream( settings );
    if ( GUCEF_NULL != stream )
        m_channels[ channel ] = stream;
    return stream;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::PostToChannel( const CORE::CString& channel, const CORE::CString& kind, const CORE::CString& from, const CORE::CString& text )
{GUCEF_TRACE;

    DRN::CDRNDataStream* stream = GetChannel( channel );
    if ( GUCEF_NULL == stream )
        return;
    CORE::CVariantStream& entry = stream->GetWriteStream();
    entry << kind << from << text << CGucefLobbyProtocol::GetUnixTimeInMs() << ( "m" + CORE::ToString( ++m_messageCount ) );
    entry.WriteSegmentEnd();
    if ( !stream->CommitEntry() )
        entry.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyServer::Reply( UInt32 peerId, UInt32 requestId, bool ok, const CORE::CString& reason, const CORE::CVariant* arg1, const CORE::CVariant* arg2, const CORE::CVariant* arg3 )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_directory )
        return false;
    if ( !ok )
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: Refused request " << requestId << " of peer " << peerId << ": " << reason );
    CORE::CDynamicBuffer buffer;
    DRN::CDRNWireWriter writer( buffer );
    writer.WriteUInt8( CGucefLobbyProtocol::MESSAGE_REPLY );
    writer.WriteVarUInt32( requestId );
    writer.WriteBool( ok );
    writer.WriteString( reason );
    const CORE::CVariant* args[ 3 ] = { arg1, arg2, arg3 };
    for ( UInt32 i=0; i<3; ++i )
        DRN::CDRNVariantCodec::Write( writer, GUCEF_NULL != args[ i ] ? *args[ i ] : CORE::CVariant::Empty );
    return m_directory->SendAppMessage( peerId, DRN::DRN_LANE_RELIABLE_ORDERED, buffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::OnRequests( DRN::CDRNDataStream& stream )
{GUCEF_TRACE;

    UInt32 peerId = stream.GetOwnerPeerId();
    CORE::CVariantStream entry;
    while ( stream.ReadNextEntry( entry ) )
    {
        UInt32 requestId = 0;
        CORE::CString verb;
        entry >> requestId >> verb;
        if ( entry.IsValid() )
            OnRequest( peerId, requestId, verb, entry );
        entry.Clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::OnRequest( UInt32 peerId, UInt32 requestId, const CORE::CString& verb, CORE::CVariantStream& args )
{GUCEF_TRACE;

    if ( "hello" == verb )
    {
        CORE::CString userId, name;
        args >> userId >> name;
        DRN::CDRNPeerLinkPtr link = GUCEF_NULL != m_directory ? m_directory->GetLinkToMember( peerId ) : DRN::CDRNPeerLinkPtr();
        if ( link.IsNULL() )
            return;
        const DRN::CDRNAuthResult& auth = link->GetAuthResult();
        CORE::CString publicKey;
        if ( CPlayerIdentity::KeySize == auth.publicKey.GetDataSize() )
        {
            // The user is who its key proved: a user id of another key is a forgery
            publicKey = DRN::CDRNCrypto::ToHex( auth.publicKey.GetConstBufferPtr(), auth.publicKey.GetDataSize() );
            if ( !userId.IsNULLOrEmpty() && userId != auth.userId )
            {
                Reply( peerId, requestId, false, "The user id is not the one of your identity key" );
                return;
            }
            userId = auth.userId;
        }
        else
        if ( m_settings.requireIdentityProof )
        {
            Reply( peerId, requestId, false, "The server wants proof of an identity key" );
            return;
        }
        if ( userId.IsNULLOrEmpty() )
        {
            Reply( peerId, requestId, false, "A user id is needed" );
            return;
        }
        DRN::CDRNBanEntry ban;
        if ( m_banList.IsUserBanned( userId, &ban ) )
        {
            Reply( peerId, requestId, false, "You are banned: " + ban.reason );
            return;
        }
        TDirectoryClient& client = m_clients[ peerId ];
        client.userId = userId;
        client.name = name.IsNULLOrEmpty() ? userId : name;
        client.publicKey = publicKey;
        client.address = GetAddressOfLink( *link );
        client.requests.SetRate( m_settings.requestsPerSecond, m_settings.requestsBurst );
        client.chat.SetRate( m_settings.chatMessagesPerSecond, m_settings.chatBurst );
        CORE::CVariant userIdArg( userId );
        Reply( peerId, requestId, true, CORE::CString::Empty, &userIdArg );
        return;
    }

    TClientMap::iterator c = m_clients.find( peerId );
    if ( c == m_clients.end() )
    {
        Reply( peerId, requestId, false, "Say hello first" );
        return;
    }
    TDirectoryClient& client = (*c).second;
    if ( !client.requests.Take( GetTimeInMs() ) )
    {
        Reply( peerId, requestId, false, "Too many requests, wait a moment" );
        return;
    }

    if ( "create" == verb )
    {
        CORE::CVariant paramsBlob;
        CORE::CString endpoints;
        CORE::CVariant integrityBlob;
        args >> paramsBlob >> endpoints;
        args >> integrityBlob;
        CORE::CDataNode paramsTree;
        CLobbyCreateParams params;
        if ( !CGucefLobbyProtocol::VariantToDataTree( paramsBlob, paramsTree ) || !params.LoadConfig( paramsTree ) )
        {
            Reply( peerId, requestId, false, "The lobby parameters cannot be read" );
            return;
        }
        CORE::CString why;
        if ( MakeGamePolicy( params.securityPolicy ).IsWeakerThan( m_settings.minimumSecurityPolicy, why ) )
        {
            Reply( peerId, requestId, false, "The security policy of the game is weaker than the server allows: " + why );
            return;
        }
        CORE::CValueList integrity;
        ReadIntegrity( integrityBlob, integrity );
        if ( !CheckIntegrity( m_settings.expectedIntegrity, integrity, why ) )
        {
            Reply( peerId, requestId, false, why );
            return;
        }
        if ( 0 != m_settings.maxLobbies && m_lobbies.size() >= m_settings.maxLobbies )
        {
            Reply( peerId, requestId, false, "The server has as many lobbies as it takes" );
            return;
        }

        CORE::CString lobbyId = "lobby-" + CORE::ToString( ++m_lobbyCount );
        CHostedLobby* lobby = GUCEF_NEW CHostedLobby( *this, lobbyId, params );
        CORE::CString reason;
        if ( !lobby->Start() || !lobby->Admit( client.userId, client.name, CORE::CString::Empty, false, SplitStrings( endpoints ), client.publicKey, integrity, true, reason ) )
        {
            if ( GUCEF_NULL != lobby->m_session )
                m_node.DestroySession( lobby->m_session );
            GUCEF_DELETE lobby;
            Reply( peerId, requestId, false, reason.IsNULLOrEmpty() ? CORE::CString( "The lobby could not be made" ) : reason );
            return;
        }
        m_lobbies[ lobbyId ] = lobby;
        lobby->Publish();

        CORE::CVariant idArg( lobbyId );
        CORE::CVariant tokenArg( IssueToken( CGucefLobbyProtocol::MakeLobbySessionName( lobbyId ), client.userId, "player", client.publicKey ) );
        CORE::CVariant endpointsArg( GetPinnedEndpoints().ToString() );
        Reply( peerId, requestId, true, CORE::CString::Empty, &idArg, &tokenArg, &endpointsArg );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyServer: " << client.name << " created lobby " << lobbyId << " \"" << params.name << "\"" );
        CORE::TCloneableString eventData( lobbyId );
        NotifyObservers( LobbyCreatedEvent, &eventData );
        return;
    }

    if ( "join" == verb )
    {
        CORE::CString lobbyId, password, endpoints;
        bool asSpectator = false;
        CORE::CVariant integrityBlob;
        args >> lobbyId >> password >> asSpectator >> endpoints;
        args >> integrityBlob;
        CORE::CValueList integrity;
        ReadIntegrity( integrityBlob, integrity );
        CHostedLobby* lobby = FindLobby( lobbyId );
        CORE::CString reason;
        if ( GUCEF_NULL == lobby )
        {
            Reply( peerId, requestId, false, "There is no lobby " + lobbyId );
            return;
        }
        if ( !CheckIntegrity( m_settings.expectedIntegrity, integrity, reason ) ||
             !lobby->Admit( client.userId, client.name, password, asSpectator, SplitStrings( endpoints ), client.publicKey, integrity, false, reason ) )
        {
            Reply( peerId, requestId, false, reason );
            return;
        }
        CORE::CVariant idArg( lobbyId );
        CORE::CVariant tokenArg( IssueToken( CGucefLobbyProtocol::MakeLobbySessionName( lobbyId ), client.userId, asSpectator ? "spectator" : "player", client.publicKey ) );
        CORE::CVariant endpointsArg( GetPinnedEndpoints().ToString() );
        Reply( peerId, requestId, true, CORE::CString::Empty, &idArg, &tokenArg, &endpointsArg );
        return;
    }

    if ( "publish" == verb )
    {
        CORE::CVariant listingBlob;
        args >> listingBlob;
        CORE::CDataNode tree;
        CGameListingEntry listing;
        if ( !CGucefLobbyProtocol::VariantToDataTree( listingBlob, tree ) || !listing.Deserialize( tree, CORE::CDataNodeSerializableSettings() ) || listing.listingId.IsNULLOrEmpty() )
        {
            Reply( peerId, requestId, false, "The listing cannot be read" );
            return;
        }
        if ( GUCEF_NULL != FindLobby( listing.listingId ) )
        {
            Reply( peerId, requestId, false, "The listing id is taken" );
            return;
        }
        Put( m_listings, listing.listingId, listingBlob );
        Reply( peerId, requestId, true, CORE::CString::Empty );
        return;
    }

    if ( "withdraw" == verb )
    {
        CORE::CString listingId;
        args >> listingId;
        bool known = GUCEF_NULL == FindLobby( listingId ) && GUCEF_NULL != m_listings && m_listings->HasKey( CORE::CVariant( listingId ) );
        if ( known )
            WithdrawListing( listingId );
        Reply( peerId, requestId, known, known ? CORE::CString::Empty : CORE::CString( "There is no such listing" ) );
        return;
    }

    if ( "chatjoin" == verb || "chatleave" == verb )
    {
        CORE::CString channel;
        args >> channel;
        if ( channel.IsNULLOrEmpty() )
        {
            Reply( peerId, requestId, false, "A channel needs a name" );
            return;
        }
        bool join = "chatjoin" == verb;
        if ( join )
        {
            client.channels.insert( channel );
            PostToChannel( channel, "joined", client.name, CORE::CString::Empty );
        }
        else
        {
            client.channels.erase( channel );
            PostToChannel( channel, "left", client.name, CORE::CString::Empty );
        }
        Reply( peerId, requestId, true, CORE::CString::Empty );
        return;
    }

    if ( "say" == verb )
    {
        CORE::CString channel, text;
        args >> channel >> text;
        if ( client.channels.find( channel ) == client.channels.end() )
        {
            Reply( peerId, requestId, false, "Not in the channel " + channel );
            return;
        }
        if ( !client.chat.Take( GetTimeInMs() ) )
        {
            Reply( peerId, requestId, false, "Too many messages, wait a moment" );
            return;
        }
        PostToChannel( channel, "msg", client.name, text );
        Reply( peerId, requestId, true, CORE::CString::Empty );
        return;
    }

    if ( "dm" == verb )
    {
        CORE::CString to, text;
        args >> to >> text;
        if ( !client.chat.Take( GetTimeInMs() ) )
        {
            Reply( peerId, requestId, false, "Too many messages, wait a moment" );
            return;
        }
        TClientMap::iterator t = m_clients.begin();
        while ( t != m_clients.end() && (*t).second.name != to && (*t).second.userId != to )
            ++t;
        if ( t == m_clients.end() )
        {
            Reply( peerId, requestId, false, "There is no user " + to );
            return;
        }
        CORE::CDynamicBuffer buffer;
        DRN::CDRNWireWriter writer( buffer );
        writer.WriteUInt8( CGucefLobbyProtocol::MESSAGE_DIRECT_MESSAGE );
        writer.WriteString( client.name );
        writer.WriteString( text );
        writer.WriteVarUInt64( CGucefLobbyProtocol::GetUnixTimeInMs() );
        writer.WriteString( "d" + CORE::ToString( ++m_messageCount ) );
        m_directory->SendAppMessage( (*t).first, DRN::DRN_LANE_RELIABLE_ORDERED, buffer.GetConstBufferPtr(), writer.GetOffset() );
        Reply( peerId, requestId, true, CORE::CString::Empty );
        return;
    }

    Reply( peerId, requestId, false, "There is no such request: " + verb );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::OnDirectoryMemberLeft( UInt32 peerId )
{GUCEF_TRACE;

    TClientMap::iterator c = m_clients.find( peerId );
    if ( c == m_clients.end() )
        return;
    TDirectoryClient client = (*c).second;
    m_clients.erase( c );
    CORE::CString::StringSet::iterator i = client.channels.begin();
    while ( i != client.channels.end() )
    {
        PostToChannel( (*i), "left", client.name, CORE::CString::Empty );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::OnTimer( void )
{GUCEF_TRACE;

    if ( !m_running )
        return;

    // The sessions of closed lobbies go once they had the time to end
    for ( size_t i=0; i<m_closedLobbies.size(); ++i )
    {
        if ( GUCEF_NULL != m_closedLobbies[ i ]->m_session )
            m_node.DestroySession( m_closedLobbies[ i ]->m_session );
        GUCEF_DELETE m_closedLobbies[ i ];
    }
    m_closedLobbies.clear();

    UInt64 nowMs = GetTimeInMs();
    TLobbyMap::iterator i = m_lobbies.begin();
    while ( i != m_lobbies.end() )
    {
        CHostedLobby* lobby = (*i).second;
        lobby->OnTick( nowMs );
        if ( lobby->m_closed )
        {
            CORE::CString lobbyId = (*i).first;
            WithdrawListing( lobbyId );
            m_closedLobbies.push_back( lobby );
            m_lobbies.erase( i++ );
            CORE::TCloneableString eventData( lobbyId );
            NotifyObservers( LobbyClosedEvent, &eventData );
            continue;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyServer::OnNotify( CORE::CNotifier* notifier                 ,
                             const CORE::CEvent& eventId               ,
                             CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( notifier == &m_timer )
    {
        if ( CORE::CTimer::TimerUpdateEvent == eventId )
            OnTimer();
        return;
    }

    if ( GUCEF_NULL != m_directory && notifier == m_directory )
    {
        if ( DRN::CDRNSession::ItemAnnouncedEvent == eventId )
        {
            const DRN::CDRNItemInfo& info = static_cast< DRN::CDRNItemInfoEventData* >( eventData )->GetData();
            if ( info.name == CGucefLobbyProtocol::RequestsStream )
            {
                DRN::CDRNDataStream* stream = m_directory->FindStream( info.name, info.ownerPeerId );
                if ( GUCEF_NULL != stream )
                    SubscribeTo( stream );
            }
        }
        else
        if ( DRN::CDRNSession::MemberLeftEvent == eventId )
        {
            OnDirectoryMemberLeft( static_cast< DRN::CDRNSessionMemberEventData* >( eventData )->GetData().peerId );
        }
        return;
    }

    if ( DRN::CDRNDataStream::EntriesAvailableEvent == eventId )
    {
        DRN::CDRNDataStream* stream = static_cast< DRN::CDRNDataStream* >( notifier );
        if ( GUCEF_NULL != m_directory && &stream->GetSession() == m_directory && stream->GetName() == CGucefLobbyProtocol::RequestsStream )
            OnRequests( *stream );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobbyServer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGucefLobbyServer";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
