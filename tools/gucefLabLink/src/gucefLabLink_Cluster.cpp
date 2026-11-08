/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYENTRY_H
#include "gucefCOMCORE_CDiscoveryEntry.h"
#define GUCEF_COMCORE_CDISCOVERYENTRY_H
#endif /* GUCEF_COMCORE_CDISCOVERYENTRY_H ? */

#ifndef GUCEF_COMCORE_CRPCHOST_H
#include "gucefCOMCORE_CRpcHost.h"
#define GUCEF_COMCORE_CRPCHOST_H
#endif /* GUCEF_COMCORE_CRPCHOST_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICEIMPL_H
#include "gucefCOMCORE_CRpcServiceImpl.h"
#define GUCEF_COMCORE_CRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CRPCSERVICEIMPL_H ? */

#ifndef GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#include "gucefCOMCORE_CTRpcMethodHandler.h"
#define GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#endif /* GUCEF_COMCORE_CTRPCMETHODHANDLER_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CIDRNACCESSPOLICY_H
#include "gucefDRN_CIDRNAccessPolicy.h"
#define GUCEF_DRN_CIDRNACCESSPOLICY_H
#endif /* GUCEF_DRN_CIDRNACCESSPOLICY_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#include "gucefDRN_CDRNRpcTransportListener.h"
#define GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#include "gucefLabLink_Cluster.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The content of a text file, empty when it cannot be read */
static CORE::CString
ReadText( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    if ( !CORE::FileExists( path ) || !content.LoadContentFromFile( path ) )
        return CORE::CString::Empty;
    return ( 0 == content.GetDataSize() ? CORE::CString::Empty : CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) );
}

/*-------------------------------------------------------------------------*/

/** The machine id in the subject of a token peer, "drn:<machine id>" */
static CORE::CString
MachineIdOf( const CORE::CString& subject )
{GUCEF_TRACE;

    return subject.StartsWith( "drn:" ) ? subject.CutChars( 4, true ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Who may join the sessions of an endpoint: addresses in the allowed ranges; on the lab endpoint only peers whose
 *  token is of the lab secret or of a machine that is still approved. Neither endpoint publishes data items
 */
class CJoinPolicy : public DRN::CIDRNAccessPolicy
{
    public:

    CJoinPolicy( CClusterRole& role, bool lab ) : DRN::CIDRNAccessPolicy(), m_role( role ), m_lab( lab ) {}

    virtual bool MayJoin( const DRN::CDRNPeerLink& link, const CORE::CString& sessionName ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CORE::CString address = link.GetRemoteEndpoint().GetHost();
        if ( !m_role.GetSettings().allowedRanges.Allows( address ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Refused " << address << ": not in the allowed ranges (" << m_role.GetSettings().allowedRanges.Describe() << ")" );
            return false;
        }
        if ( !m_lab )
            return sessionName == AccessSessionName;

        const DRN::CDRNAuthResult& auth = link.GetAuthResult();
        CORE::CString::StringSet roles;
        roles.insert( auth.role );
        if ( sessionName != LabSessionName || !m_role.IsAllowed( "drn:" + auth.userId, roles ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Refused " << address << ": its token is not of the lab secret or of an approved machine" );
            return false;
        }
        return true;
    }

    virtual bool MaySubscribe( UInt32 peerId, const CORE::CString& itemName ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool MayWrite( UInt32 peerId, const CORE::CString& itemName, UInt32 keyIndex ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    private:

    CClusterRole& m_role;
    bool m_lab;
};

/*-------------------------------------------------------------------------*/

/** The service of the lab endpoint */
class CLabService : public COMCORE::CRpcServiceImpl
{
    public:

    typedef COMCORE::CTRpcMethodHandler< CLabService > THandler;

    explicit CLabService( CClusterRole& role )
        : COMCORE::CRpcServiceImpl()
        , m_role( role )
    {
        BindMethod( "Hello", GUCEF_NEW THandler( this, &CLabService::OnHello ) );
        BindMethod( "Join", GUCEF_NEW THandler( this, &CLabService::OnJoin ) );
        BindMethod( "ClientConfig", GUCEF_NEW THandler( this, &CLabService::OnClientConfig ) );
    }

    /** A call of a peer that may not use the service any more is refused: the approval was taken back */
    bool Refuse( COMCORE::CRpcServerCall& call )
    {
        const COMCORE::CRpcPrincipal& principal = call.GetPrincipal();
        if ( principal.isAuthenticated && m_role.IsAllowed( principal.subject, principal.roles ) )
            return false;
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, "not a machine of this lab" ) );
        return true;
    }

    COMCORE::TRpcHandlerResult OnHello( COMCORE::CRpcServerCall& call )
    {
        if ( Refuse( call ) )
            return COMCORE::RPC_HANDLER_DONE;
        COMCORE::CRpcMessage response = call.NewResponse();
        response.SetString( 1, m_role.GetSettings().clusterName );
        response.SetString( 2, m_role.GetK3sVersion() );
        response.SetString( 3, MakeClusterProof( m_role.GetSecret(), m_role.GetLabKey() ) );
        call.Reply( response );
        return COMCORE::RPC_HANDLER_DONE;
    }

    COMCORE::TRpcHandlerResult OnJoin( COMCORE::CRpcServerCall& call )
    {
        if ( Refuse( call ) )
            return COMCORE::RPC_HANDLER_DONE;
        const COMCORE::CRpcMessage& request = call.GetRequest();
        const CORE::CString name = request.GetString( 1 );
        const bool ephemeral = request.GetBool( 4 );
        if ( m_role.GetSettings().joinCommand.IsNULLOrEmpty() )
        {
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_FAILED_PRECONDITION, "this cluster does not let machines join" ) );
            return COMCORE::RPC_HANDLER_DONE;
        }

        // a bootstrap token for this one join, valid for an hour
        CORE::CProcess::TStringVector args;
        args.push_back( "--values" );
        args.push_back( "--ttl" );
        args.push_back( "1h" );
        if ( ephemeral )
            args.push_back( "--ephemeral" );
        // the node name the machine joins with: what a join of it that did not complete left behind is cleaned up
        if ( !name.IsNULLOrEmpty() )
        {
            args.push_back( "--node" );
            args.push_back( name.Lowercase() );
        }
        std::string output;
        Int32 exitCode = -1;
        if ( !RunProgram( m_role.GetSettings().joinCommand, args, CORE::CProcess::TStringMap(), 60000, output, exitCode ) || 0 != exitCode ||
             ValueOfLine( output, "token" ).IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << m_role.GetSettings().joinCommand << " failed for " << name << ": " << output.c_str() );
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the cluster could not make a join token" ) );
            return COMCORE::RPC_HANDLER_DONE;
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Join of " << name << " (" << request.GetString( 2 ) << ", " << request.GetString( 3 ) <<
                          ( ephemeral ? ", ephemeral" : "" ) << ") for " << call.GetPrincipal().subject );

        COMCORE::CRpcMessage response = call.NewResponse();
        response.SetString( 1, ValueOfLine( output, "server" ) );
        response.SetString( 2, ValueOfLine( output, "version" ) );
        response.SetString( 3, ValueOfLine( output, "token" ) );
        response.SetBool( 4, ValueOfLine( output, "ephemeral" ) == "true" );
        call.Reply( response );
        return COMCORE::RPC_HANDLER_DONE;
    }

    COMCORE::TRpcHandlerResult OnClientConfig( COMCORE::CRpcServerCall& call )
    {
        if ( Refuse( call ) )
            return COMCORE::RPC_HANDLER_DONE;
        const CORE::CString& mcpDir = m_role.GetSettings().mcpDir;
        COMCORE::CRpcMessage response = call.NewResponse();
        response.SetString( 1, ReadText( CORE::CombinePath( mcpDir, "tls/ca.crt" ) ) );
        response.SetString( 2, StripText( ReadText( CORE::CombinePath( mcpDir, "lab-token" ) ) ) );
        CORE::CString::StringVector lines = ReadText( CORE::CombinePath( mcpDir, "mcp-nodes" ) ).ParseElements( '\n', false );
        for ( size_t i=0; i<lines.size(); ++i )
        {
            const CORE::CString line = StripText( lines[ i ] );
            if ( !line.IsNULLOrEmpty() && '#' != line[ 0 ] )
                response.Add( 3, CORE::CVariant( line ) );
        }
        call.Reply( response );
        return COMCORE::RPC_HANDLER_DONE;
    }

    private:

    CClusterRole& m_role;
};

/*-------------------------------------------------------------------------*/

/** The service of the access endpoint: machines without the lab secret ask for access, and wait for the approval */
class CAccessService : public COMCORE::CRpcServiceImpl
{
    public:

    typedef COMCORE::CTRpcMethodHandler< CAccessService > THandler;

    explicit CAccessService( CClusterRole& role )
        : COMCORE::CRpcServiceImpl()
        , m_role( role )
    {
        BindMethod( "RequestAccess", GUCEF_NEW THandler( this, &CAccessService::OnRequestAccess ) );
    }

    COMCORE::TRpcHandlerResult OnRequestAccess( COMCORE::CRpcServerCall& call )
    {
        const COMCORE::CRpcMessage& request = call.GetRequest();
        const CORE::CString machineId = StripText( request.GetString( 1 ) );
        const CORE::CString name = StripText( request.GetString( 2 ) );
        if ( machineId.Length() < 16 || machineId.Length() > 64 || name.IsNULLOrEmpty() || name.Length() > 64 )
        {
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT, "a machine id and a name are needed" ) );
            return COMCORE::RPC_HANDLER_DONE;
        }

        CORE::CString problem;
        GUCEF::vector< SApproval > approvals;
        if ( !m_role.GetStore().LoadApprovals( approvals, problem ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << problem );
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the cluster cannot read its approvals" ) );
            return COMCORE::RPC_HANDLER_DONE;
        }

        const CORE::CString address = m_role.GetAccessPeerAddress( call.GetPrincipal() );
        SApproval* known = GUCEF_NULL;
        for ( size_t i=0; i<approvals.size(); ++i )
            if ( approvals[ i ].machineId == machineId )
                known = &approvals[ i ];

        COMCORE::CRpcMessage response = call.NewResponse();
        response.SetString( 4, m_role.GetSettings().clusterName );
        if ( GUCEF_NULL == known )
        {
            SApproval approval;
            approval.state = AccessPending;
            approval.machineId = machineId;
            approval.name = name;
            approval.address = address;
            approval.os = request.GetString( 3 );
            approvals.push_back( approval );
            if ( !m_role.GetStore().SaveApprovals( approvals, problem ) )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << problem );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << name << " (" << approval.os << ") asks for access; approve it with: gucefLabLink --approve " << name );
            response.SetString( 1, AccessPending );
        }
        else
        if ( known->state == AccessApproved )
        {
            // the token is made when it is first collected, for the key of the moment
            if ( known->token.IsNULLOrEmpty() )
            {
                known->token = DRN::CDRNTokenAuthenticator::IssueToken( m_role.GetTokenKey(), LabSessionName, machineId, RoleApproved,
                                                                        DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + ApprovedTokenLifetimeInSecs );
                if ( !m_role.GetStore().SaveApprovals( approvals, problem ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << problem );
            }
            response.SetString( 1, AccessApproved );
            response.SetString( 2, known->token );
            response.SetString( 3, m_role.GetLabKey() );
        }
        else
        {
            response.SetString( 1, known->state == AccessRefused ? AccessRefused : AccessPending );
        }
        call.Reply( response );
        return COMCORE::RPC_HANDLER_DONE;
    }

    private:

    CClusterRole& m_role;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
AddDiscoveryClients( const SSettings& settings, const CORE::CString& namePrefix, CORE::CString::StringVector& names )
{GUCEF_TRACE;

    COMCORE::CDiscoveryManager& manager = COMCORE::CComCoreGlobal::Instance()->GetDiscoveryManager();
    bool any = false;
    if ( settings.useMdns )
    {
        CORE::CDataNode config;
        config.SetAttribute( "name", namePrefix + "-mdns" );
        if ( !manager.CreateDiscoveryClient( "mdns", &config ).IsNULL() )
        {
            names.push_back( namePrefix + "-mdns" );
            any = true;
        }
        else
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: mDNS discovery is not available" );
    }
    if ( settings.useBroadcast )
    {
        CORE::CDataNode config;
        config.SetAttribute( "name", namePrefix + "-broadcast" );
        config.SetAttribute( "port", (UInt32) settings.broadcastPort );
        if ( !manager.CreateDiscoveryClient( "broadcast", &config ).IsNULL() )
        {
            names.push_back( namePrefix + "-broadcast" );
            any = true;
        }
        else
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: UDP broadcast discovery is not available" );
    }
    return any;
}

/*-------------------------------------------------------------------------*/

CClusterRole::CClusterRole( const SSettings& settings, CStore& store )
    : m_settings( settings )
    , m_store( store )
    , m_secret()
    , m_tokenKey()
    , m_k3sVersion()
    , m_labNode( GUCEF_NULL )
    , m_accessNode( GUCEF_NULL )
    , m_labSession( GUCEF_NULL )
    , m_accessSession( GUCEF_NULL )
    , m_labPolicy( GUCEF_NULL )
    , m_accessPolicy( GUCEF_NULL )
    , m_labHost( GUCEF_NULL )
    , m_accessHost( GUCEF_NULL )
    , m_labService( GUCEF_NULL )
    , m_accessService( GUCEF_NULL )
    , m_entry()
    , m_discoveryClients()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CClusterRole::~CClusterRole()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

const SSettings& CClusterRole::GetSettings( void ) const { return m_settings; }
CStore& CClusterRole::GetStore( void ) { return m_store; }
const CORE::CString& CClusterRole::GetSecret( void ) const { return m_secret; }
const CORE::CString& CClusterRole::GetTokenKey( void ) const { return m_tokenKey; }
const CORE::CString& CClusterRole::GetK3sVersion( void ) const { return m_k3sVersion; }

/*-------------------------------------------------------------------------*/

CORE::CString
CClusterRole::GetLabKey( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_labNode ? m_labNode->GetPublicKeyString() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CClusterRole::GetAccessPeerAddress( const COMCORE::CRpcPrincipal& principal ) const
{GUCEF_TRACE;

    CORE::CValueList attributes = principal.attributes;
    CORE::CVariant peerId;
    if ( GUCEF_NULL == m_accessSession || !attributes.TryGetValue( "drn.peerId", peerId ) )
        return CORE::CString::Empty;
    // the session is the node's: read under its lock, the call runs on a thread of the pool
    MT::CScopeMutex lock( m_accessNode->GetLock() );
    DRN::CDRNPeerLinkPtr link = m_accessSession->GetLinkToMember( peerId.AsUInt32() );
    return link.IsNULL() ? CORE::CString::Empty : link->GetRemoteEndpoint().GetHost();
}

/*-------------------------------------------------------------------------*/

bool
CClusterRole::IsAllowed( const CORE::CString& subject, const CORE::CString::StringSet& roles ) const
{GUCEF_TRACE;

    if ( roles.find( RoleLab ) != roles.end() )
        return true;
    if ( roles.find( RoleApproved ) == roles.end() )
        return false;

    // an approval that was taken back refuses its token
    const CORE::CString machineId = MachineIdOf( subject );
    CORE::CString problem;
    GUCEF::vector< SApproval > approvals;
    if ( machineId.IsNULLOrEmpty() || !m_store.LoadApprovals( approvals, problem ) )
        return false;
    for ( size_t i=0; i<approvals.size(); ++i )
        if ( approvals[ i ].machineId == machineId )
            return approvals[ i ].state == AccessApproved;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CClusterRole::StartEndpoint( bool lab, CORE::CString& problem )
{GUCEF_TRACE;

    const UInt16 port = lab ? m_settings.labPort : m_settings.accessPort;
    DRN::CDRNNodeSettings nodeSettings;
    nodeSettings.serviceName = "gucef-lab";
    nodeSettings.securityPolicy.requireEncryption = true;
    if ( lab )
    {
        // HMAC tokens of the lab secret, for this session only; a key of its own that stays, so machines can pin it
        nodeSettings.auth.authenticatorType = "token";
        nodeSettings.auth.SetSetting( "key", CORE::CVariant( m_tokenKey ) );
        nodeSettings.auth.SetSetting( "expectedSession", CORE::CVariant( LabSessionName ) );
        nodeSettings.staticKeyFile = m_store.GetPath( "lab.key" );
    }
    else
    {
        nodeSettings.auth.authenticatorType = "anonymous";
    }

    DRN::CDRNNode* node = GUCEF_NEW DRN::CDRNNode();
    if ( !node->SetSettings( nodeSettings ) || !node->Start() )
    {
        problem = CORE::CString( "the DRN node of the " ) + ( lab ? "lab" : "access" ) + " endpoint cannot start (is codecspluginMONOCYPHER there?)";
        GUCEF_DELETE node;
        return false;
    }
    CJoinPolicy* policy = GUCEF_NEW CJoinPolicy( *this, lab );
    node->SetAccessPolicy( policy );
    if ( !node->Listen( DRN::CDRNEndpoint( "tcp://0.0.0.0:" + CORE::ToString( port ) ) ) )
    {
        problem = "cannot listen on port " + CORE::ToString( port );
        node->Stop();
        GUCEF_DELETE node;
        GUCEF_DELETE policy;
        return false;
    }

    DRN::CDRNSession* session = node->CreateSession();
    DRN::CDRNSessionSettings sessionSettings;
    sessionSettings.sessionName = lab ? LabSessionName : AccessSessionName;
    sessionSettings.topology = DRN::DRN_TOPOLOGY_CLIENT_SERVER;
    if ( GUCEF_NULL == session || !session->Host( sessionSettings ) )
    {
        problem = "cannot host the DRN session " + sessionSettings.sessionName;
        node->Stop();
        GUCEF_DELETE node;
        GUCEF_DELETE policy;
        return false;
    }

    COMCORE::CRpcHostSettings hostSettings;
    hostSettings.endpointSettings.nodeName = m_settings.clusterName;
    hostSettings.endpointSettings.requireSecureTransport = true;
    COMCORE::CRpcHost* host = GUCEF_NEW COMCORE::CRpcHost( node->GetPulseGenerator(), hostSettings );
    COMCORE::CRpcServiceImpl* service = GUCEF_NULL;
    CORE::CString error;
    COMCORE::CRpcContractPtr contract = lab ? BuildLabContract( error ) : BuildAccessContract( error );
    if ( lab )
        service = m_labService = GUCEF_NEW CLabService( *this );
    else
        service = m_accessService = GUCEF_NEW CAccessService( *this );
    CORE::CTSharedPtr< DRN::CDRNRpcTransportListener, MT::CMutex > drnListener( GUCEF_NEW DRN::CDRNRpcTransportListener( *session ) );
    COMCORE::CIRpcTransportListenerPtr listener;
    listener = drnListener;
    if ( contract.IsNULL() || !host->RegisterService( contract, lab ? "Lab" : "Access", service, COMCORE::RPC_EXECUTION_THREAD_POOL ) ||
         !host->Start() || !host->AddListener( listener ) || !listener->Listen( CORE::CUri() ) )
    {
        problem = "the RPC service of the " + CORE::CString( lab ? "lab" : "access" ) + " endpoint cannot start " + error;
        GUCEF_DELETE host;
        node->Stop();
        GUCEF_DELETE node;
        GUCEF_DELETE policy;
        return false;
    }

    if ( lab )
    {
        m_labNode = node; m_labSession = session; m_labPolicy = policy; m_labHost = host;
    }
    else
    {
        m_accessNode = node; m_accessSession = session; m_accessPolicy = policy; m_accessHost = host;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The " << ( lab ? "lab" : "access" ) << " endpoint listens on port " << port );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CClusterRole::Advertise( void )
{GUCEF_TRACE;

    COMCORE::CDiscoveryEntry* entry = GUCEF_NEW COMCORE::CDiscoveryEntry();
    entry->SetServiceType( ServiceType );
    entry->SetApplicationInstanceId( m_settings.clusterName );
    entry->SetInstanceName( m_settings.clusterName );
    entry->SetTtlInSecs( 120 );
    // the host of an endpoint is the address the entry is seen from
    entry->GetEndpoints().push_back( "tcp://0.0.0.0:" + CORE::ToString( m_settings.labPort ) );
    entry->GetAttributes().Set( AttributeCluster, CORE::CVariant( m_settings.clusterName ) );
    entry->GetAttributes().Set( AttributeKey, CORE::CVariant( GetLabKey() ) );
    entry->GetAttributes().Set( AttributeLabPort, CORE::CVariant( CORE::ToString( m_settings.labPort ) ) );
    entry->GetAttributes().Set( AttributeAccessPort, CORE::CVariant( CORE::ToString( m_settings.accessPort ) ) );
    m_entry = COMCORE::CIDiscoveryEntryPtr( entry );
    COMCORE::CComCoreGlobal::Instance()->GetDiscoveryManager().Advertise( m_entry );
}

/*-------------------------------------------------------------------------*/

bool
CClusterRole::Start( CORE::CString& problem )
{GUCEF_TRACE;

    if ( !m_store.ReadSecret( m_secret, problem ) )
    {
        problem += ": make one with gucefLabLink --init";
        return false;
    }
    m_tokenKey = DeriveTokenKey( m_secret );
    if ( m_tokenKey.IsNULLOrEmpty() )
    {
        problem = "the key of the tokens cannot be derived (HKDF)";
        return false;
    }

    // the version machines join with is that of the k3s of this node
    std::string output;
    Int32 exitCode = -1;
    CORE::CProcess::TStringVector args;
    args.push_back( "--version" );
    if ( RunProgram( m_settings.k3sBin, args, CORE::CProcess::TStringMap(), 10000, output, exitCode ) && 0 == exitCode )
    {
        CORE::CString::StringVector words = CORE::CString( output.c_str() ).ParseElements( ' ', false );
        if ( words.size() >= 3 )
            m_k3sVersion = words[ 2 ];
    }

    if ( !StartEndpoint( true, problem ) || !StartEndpoint( false, problem ) )
    {
        Stop();
        return false;
    }
    if ( !AddDiscoveryClients( m_settings, "gucef-lablink", m_discoveryClients ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: No discovery is available: machines find the cluster only when they are told where it is" );
    Advertise();
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Cluster " << m_settings.clusterName << " (k3s " << m_k3sVersion << ") answers machines in " <<
                      m_settings.allowedRanges.Describe() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CClusterRole::Stop( void )
{GUCEF_TRACE;

    COMCORE::CDiscoveryManager& manager = COMCORE::CComCoreGlobal::Instance()->GetDiscoveryManager();
    if ( !m_entry.IsNULL() )
    {
        manager.Withdraw( m_entry );
        m_entry.Unlink();
    }
    for ( size_t i=0; i<m_discoveryClients.size(); ++i )
        manager.UnregisterDiscoveryClient( m_discoveryClients[ i ] );
    m_discoveryClients.clear();

    COMCORE::CRpcHost** hosts[ 2 ] = { &m_labHost, &m_accessHost };
    DRN::CDRNNode** nodes[ 2 ] = { &m_labNode, &m_accessNode };
    CJoinPolicy** policies[ 2 ] = { &m_labPolicy, &m_accessPolicy };
    for ( int i=0; i<2; ++i )
    {
        if ( GUCEF_NULL != *hosts[ i ] )
        {
            (*hosts[ i ])->Stop();
            GUCEF_DELETE *hosts[ i ];
            *hosts[ i ] = GUCEF_NULL;
        }
        if ( GUCEF_NULL != *nodes[ i ] )
        {
            (*nodes[ i ])->Stop();
            (*nodes[ i ])->SetAccessPolicy( GUCEF_NULL );
            GUCEF_DELETE *nodes[ i ];
            *nodes[ i ] = GUCEF_NULL;
        }
        GUCEF_DELETE *policies[ i ];
        *policies[ i ] = GUCEF_NULL;
    }
    m_labSession = m_accessSession = GUCEF_NULL;
    GUCEF_DELETE m_labService;
    m_labService = GUCEF_NULL;
    GUCEF_DELETE m_accessService;
    m_accessService = GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
