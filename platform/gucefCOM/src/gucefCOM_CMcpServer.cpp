/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CJSONSCHEMAVALIDATOR_H
#include "gucefCORE_CJsonSchemaValidator.h"
#define GUCEF_CORE_CJSONSCHEMAVALIDATOR_H
#endif /* GUCEF_CORE_CJSONSCHEMAVALIDATOR_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CMCPOPERATIONTASK_H
#include "gucefCOM_CMcpOperationTask.h"

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H
#include "gucefCOM_CMcpConfigurableAccessPolicy.h"
#define GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H
#endif /* GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */
#define GUCEF_COM_CMCPOPERATIONTASK_H
#endif /* GUCEF_COM_CMCPOPERATIONTASK_H ? */

#include "gucefCOM_CMcpServer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CMcpServer::ChannelOpenedEvent = "GUCEF::COM::CMcpServer::ChannelOpenedEvent";
const CORE::CEvent CMcpServer::ChannelClosedEvent = "GUCEF::COM::CMcpServer::ChannelClosedEvent";
const CORE::CEvent CMcpServer::RequestStartedEvent = "GUCEF::COM::CMcpServer::RequestStartedEvent";
const CORE::CEvent CMcpServer::RequestFinishedEvent = "GUCEF::COM::CMcpServer::RequestFinishedEvent";
const CORE::CEvent CMcpServer::TransportErrorEvent = "GUCEF::COM::CMcpServer::TransportErrorEvent";
const CORE::CEvent CMcpServer::ShutdownRequestedEvent = "GUCEF::COM::CMcpServer::ShutdownRequestedEvent";
const CORE::CEvent CMcpServer::AuditEvent = "GUCEF::COM::CMcpServer::AuditEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const LegacyInputRequiredText = "this operation needs client input, which requires MCP protocol 2026-07-28";
const char* const RequestStateRefusedText = "Invalid params: requestState invalid or expired";
const UInt32 MaxArgumentErrors = 16;

/** A scalar attribute or an object, array or null child: whether the member is there at all */
bool
HasMember( const CORE::CDataNode& object, const CORE::CString& name )
{GUCEF_TRACE;

    return CJsonRpcEndpoint::HasJsonMember( object, name );
}

/** An optional string member: false when it is there but not a string */
bool
GetOptionalStringMember( const CORE::CDataNode& object, const char* name, CORE::CString& value, bool& present )
{GUCEF_TRACE;

    present = false;
    const CORE::CDataNode::TKeyValuePair* attribute = object.GetAttribute( name );
    if ( GUCEF_NULL != attribute )
    {
        if ( !(*attribute).second.IsString() )
            return false;
        value = (*attribute).second.AsString();
        present = true;
        return true;
    }
    if ( GUCEF_NULL != object.FindChild( name ) )
        return false;
    return true;
}

/** An optional object member: false when it is there but not an object */
bool
GetOptionalObjectMember( const CORE::CDataNode& object, const char* name, const CORE::CDataNode*& member )
{GUCEF_TRACE;

    member = GUCEF_NULL;
    if ( GUCEF_NULL != object.GetAttribute( name ) )
        return false;
    const CORE::CDataNode* child = object.FindChild( name );
    if ( GUCEF_NULL == child )
        return true;
    if ( GUCEF_DATATYPE_OBJECT != child->GetNodeType() || child->HasValue() )
        return false;
    member = child;
    return true;
}

/** An optional boolean member: false when it is there but not a boolean */
bool
GetOptionalBoolMember( const CORE::CDataNode& object, const char* name, bool& value )
{GUCEF_TRACE;

    value = false;
    const CORE::CDataNode::TKeyValuePair* attribute = object.GetAttribute( name );
    if ( GUCEF_NULL != attribute )
    {
        if ( !(*attribute).second.IsBoolean() )
            return false;
        value = (*attribute).second.AsBool();
        return true;
    }
    return GUCEF_NULL == object.FindChild( name );
}

/** The key of a list entry, the order of the registries */
const CORE::CString& KeyOf( const CMcpToolEntryPtr& entry ) { return entry->descriptor.name; }
const CORE::CString& KeyOf( const CMcpResourceEntryPtr& entry ) { return entry->descriptor.uri; }
const CORE::CString& KeyOf( const CMcpResourceTemplateEntryPtr& entry ) { return entry->descriptor.name; }
const CORE::CString& KeyOf( const CMcpPromptEntryPtr& entry ) { return entry->descriptor.name; }

/** The index of the first entry whose key sorts after 'key', in a vector sorted by key */
template < class TEntryVector >
size_t
FirstAfter( const TEntryVector& entries, const CORE::CString& key )
{GUCEF_TRACE;

    size_t low = 0;
    size_t high = entries.size();
    while ( low < high )
    {
        size_t middle = low + ( high - low ) / 2;
        if ( key < KeyOf( entries[ middle ] ) )
            high = middle;
        else
            low = middle + 1;
    }
    return low;
}

CORE::CString
FormatValidationErrors( const CORE::CJsonSchemaValidator::TErrorVector& errors )
{GUCEF_TRACE;

    CORE::CString text;
    for ( size_t i=0; i<errors.size(); ++i )
    {
        if ( i > 0 )
            text += '\n';
        text += errors[ i ].instancePath.IsNULLOrEmpty() ? CORE::CString( "(root)" ) : errors[ i ].instancePath;
        text += ": ";
        text += errors[ i ].message;
    }
    return text;
}

void
SetUriData( CORE::CDataNode& data, const CORE::CString& uri )
{GUCEF_TRACE;

    data = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
    data.SetAttribute( "uri", CORE::CVariant( uri ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      INTERNAL TYPES                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Everything about one request that the steps of the dispatch share */
struct CMcpServer::SDispatch
{
    CMcpChannelPtr channel;                     /**< empty for in-process calls */
    UInt64 channelId;
    CORE::CVariant id;
    CORE::CString idKey;
    CORE::CString method;
    CORE::CString target;
    EMcpProtocolEra era;
    CMcpRequestMeta meta;
    CMcpPrincipal principal;
    CMcpRequestContextPtr context;
    CMcpToolEntryPtr tool;
    CMcpResourceEntryPtr resource;
    CMcpResourceTemplateEntryPtr resourceTemplate;
    CMcpPromptEntryPtr prompt;
    UInt8 argumentsDigest[ 32 ];
    bool inProcess;
    bool finished;
    UInt64 startInMs;

    // what the audit record will say
    bool auditHasArguments;
    CORE::CString auditArgumentsDigest;
    CORE::CString auditArgumentsPreview;
    bool auditHasDecision;
    EMcpAccessDecision auditDecision;
    CORE::CString auditReason;

    SDispatch( void )
        : channel()
        , channelId( 0 )
        , id()
        , idKey()
        , method()
        , target()
        , era( MCP_ERA_MODERN )
        , meta()
        , principal()
        , context()
        , tool()
        , resource()
        , resourceTemplate()
        , prompt()
        , inProcess( false )
        , finished( false )
        , startInMs( 0 )
        , auditHasArguments( false )
        , auditArgumentsDigest()
        , auditArgumentsPreview()
        , auditHasDecision( false )
        , auditDecision( MCP_ACCESS_ALLOW )
        , auditReason()
    {
        ::memset( argumentsDigest, 0, sizeof( argumentsDigest ) );
    }
};

/*-------------------------------------------------------------------------*/

/** A request whose answer comes later: a pool task or a deferred handler */
struct CMcpServer::SInFlight
{
    SDispatch dispatch;
    CORE::CFutureResult future;
    bool hasFuture;
    UInt64 deadlineInMs;
    bool hasSentProgress;
    UInt64 lastProgressInMs;
    bool hasHeldProgress;
    CMcpRequestContext::SProgress heldProgress;

    SInFlight( void )
        : dispatch()
        , future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED )
        , hasFuture( false )
        , deadlineInMs( 0 )
        , hasSentProgress( false )
        , lastProgressInMs( 0 )
        , hasHeldProgress( false )
        , heldProgress()
    {
    }
};

/*-------------------------------------------------------------------------*/

struct CMcpServer::SChannelState
{
    CMcpChannelPtr channel;
    TInFlightMap inFlight;
    TSubscriptionMap subscriptions;
    bool closed;

    SChannelState( void )
        : channel()
        , inFlight()
        , subscriptions()
        , closed( false )
    {
    }
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SMcpChannelEventInfo::SMcpChannelEventInfo( void )
    : channelId( 0 )
    , transport()
    , subject()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

SMcpRequestEventInfo::SMcpRequestEventInfo( void )
    : method()
    , target()
    , requestId()
    , transport()
    , subject()
    , channelId( 0 )
    , era( MCP_ERA_MODERN )
    , protocolVersion()
    , outcome( MCP_OUTCOME_COMPLETE )
    , errorCode( 0 )
    , durationInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMcpServer::RegisterEvents( void )
{GUCEF_TRACE;

    ChannelOpenedEvent.Initialize();
    ChannelClosedEvent.Initialize();
    RequestStartedEvent.Initialize();
    RequestFinishedEvent.Initialize();
    TransportErrorEvent.Initialize();
    ShutdownRequestedEvent.Initialize();
    AuditEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMcpServer::CMcpServer( CORE::PulseGeneratorPtr pulseGenerator ,
                        CORE::ThreadPoolPtr threadPool         )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , CIMcpRequestContextListener()
    , m_pulseGenerator( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
    , m_threadPool( threadPool )
    , m_settings()
    , m_serverInfo()
    , m_serverInfoNode()
    , m_configLock()
    , m_policy()
    , m_defaultPolicy( GUCEF_NEW CMcpConfigurableAccessPolicy() )
    , m_toolRegistry( m_pulseGenerator )
    , m_resourceRegistry( m_pulseGenerator )
    , m_promptRegistry( m_pulseGenerator )
    , m_sealer()
    , m_transports()
    , m_configuredTransports()
    , m_started( false )
    , m_requestingPulses( false )
    , m_channels()
    , m_nextChannelId( 0 )
    , m_inFlightByContext()
    , m_inFlightTotal( 0 )
    , m_subscriptionsTotal( 0 )
    , m_heldProgress()
    , m_mailboxLock()
    , m_mailbox()
    , m_mailboxWork()
    , m_updatesLock()
    , m_pendingResourceUpdates()
    , m_rateLock()
    , m_tokenBuckets()
    , m_refusalWindowStartInMs( 0 )
    , m_refusalsInWindow( 0 )
    , m_refusalsSuppressed( 0 )
    , m_suppressedRefusalTransport()
    , m_suppressedRefusalReason()
    , m_inProcessLock()
    , m_inProcessFutures()
    , m_eventQueueLock()
    , m_queuedRequestEvents()
    , m_protocolThreadId( 0 )
    GUCEF_SYSTEM_THREAD_ID_MEMBER_INIT( m_protocolSystemThreadId, 0 )
    , m_toolSnapshotGeneration( 0 )
    , m_toolSnapshot()
    , m_resourceSnapshotGeneration( 0 )
    , m_resourceSnapshot()
    , m_templateSnapshot()
    , m_promptSnapshotGeneration( 0 )
    , m_promptSnapshot()
{GUCEF_TRACE;

    RegisterEvents();
    m_protocolThreadId.Store( m_pulseGenerator->GetPulseDriverThreadId() );
    GUCEF_SYSTEM_THREAD_ID_CODE( m_protocolSystemThreadId.Store( m_pulseGenerator->GetPulseDriverSystemThreadId() ); )
    ApplySettings( m_settings );

    typedef CORE::CTEventHandlerFunctor< CMcpServer > TEventCallback;
    TEventCallback pulseCallback( this, &CMcpServer::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulseCallback );
    TEventCallback toolsCallback( this, &CMcpServer::OnToolsListChanged );
    SubscribeTo( &m_toolRegistry, CMcpRegistryBase::ListChangedEvent, toolsCallback );
    TEventCallback resourcesCallback( this, &CMcpServer::OnResourcesListChanged );
    SubscribeTo( &m_resourceRegistry, CMcpRegistryBase::ListChangedEvent, resourcesCallback );
    TEventCallback promptsCallback( this, &CMcpServer::OnPromptsListChanged );
    SubscribeTo( &m_promptRegistry, CMcpRegistryBase::ListChangedEvent, promptsCallback );
}

/*-------------------------------------------------------------------------*/

CMcpServer::~CMcpServer()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    UnsubscribeAllFromObserver();

    Stop( false );

    // nothing may tell this server about news any more
    for ( TContextMap::iterator i = m_inFlightByContext.begin(); i != m_inFlightByContext.end(); ++i )
        (*i).first->SetListener( GUCEF_NULL );

    // close what transports left open
    GUCEF::vector< CMcpChannelPtr > channels;
    for ( TChannelStateMap::iterator i = m_channels.begin(); i != m_channels.end(); ++i )
        channels.push_back( (*i).second->channel );
    for ( size_t i=0; i<channels.size(); ++i )
        CloseChannel( channels[ i ] );
    for ( TChannelStateMap::iterator i = m_channels.begin(); i != m_channels.end(); ++i )
    {
        SChannelState* state = (*i).second;
        for ( TInFlightMap::iterator n = state->inFlight.begin(); n != state->inFlight.end(); ++n )
        {
            SInFlight* inFlight = (*n).second;
            inFlight->dispatch.context->SetListener( GUCEF_NULL );
            inFlight->dispatch.context->RequestCancellation();
            inFlight->dispatch.context->MarkEnded();
            if ( inFlight->hasFuture )
                inFlight->future.RequestCancellation();
            GUCEF_DELETE inFlight;
        }
        GUCEF_DELETE state;
    }
    m_channels.clear();
    m_inFlightByContext.clear();
    m_heldProgress.clear();

    // the asynchronous in-process calls run this server's code on pool threads: wait for them
    TFutureVector futures;
    {
        MT::CScopeMutex lock( m_inProcessLock );
        futures.swap( m_inProcessFutures );
    }
    for ( size_t i=0; i<futures.size(); ++i )
    {
        futures[ i ].RequestCancellation();
        futures[ i ].Await();
    }

    if ( m_requestingPulses )
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpServer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpServer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::ApplySettings( const CMcpServerSettings& settings )
{GUCEF_TRACE;

    bool sealerSettingsChanged = !m_sealer.IsInitialized() ||
                                 settings.requestStateProtection != m_settings.requestStateProtection ||
                                 settings.requestStateKeyFile != m_settings.requestStateKeyFile ||
                                 settings.requestStateAeadCodecs != m_settings.requestStateAeadCodecs;

    MT::CScopeMutex lock( m_configLock );
    if ( &settings != &m_settings )
        m_settings = settings;
    if ( !m_settings.serverName.IsNULLOrEmpty() )
        m_serverInfo.name = m_settings.serverName;
    if ( !m_settings.serverTitle.IsNULLOrEmpty() )
        m_serverInfo.title = m_settings.serverTitle;
    if ( !m_settings.serverVersion.IsNULLOrEmpty() )
        m_serverInfo.version = m_settings.serverVersion;
    m_serverInfo.ToDataNode( m_serverInfoNode );

    if ( !sealerSettingsChanged )
        return true;

    CMcpRequestStateSealer::EProtection protection = CMcpRequestStateSealer::PROTECTION_AUTO;
    if ( !CMcpRequestStateSealer::ParseProtection( m_settings.requestStateProtection, protection ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: Unknown requestStateProtection \"" << m_settings.requestStateProtection << "\", using \"auto\"" );
        protection = CMcpRequestStateSealer::PROTECTION_AUTO;
    }
    CORE::CString error;
    CORE::CDynamicBuffer key;
    bool haveKey = false;
    if ( !m_settings.requestStateKeyFile.IsNULLOrEmpty() )
    {
        if ( !CMcpRequestStateSealer::LoadKeyFile( m_settings.requestStateKeyFile, key, error ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: " << error << ": " << m_settings.requestStateKeyFile );
            return false;
        }
        haveKey = true;
    }
    if ( !m_sealer.Initialize( protection, m_settings.requestStateAeadCodecs, haveKey ? &key : GUCEF_NULL, error ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: The requestState sealer could not be prepared: " << error );
        return false;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "McpServer: requestState protection is " <<
        CMcpRequestStateSealer::ProtectionToString( m_sealer.GetProtection() ) <<
        ( m_sealer.GetAeadCodecType().IsNULLOrEmpty() ? CORE::CString::Empty : " with " + m_sealer.GetAeadCodecType() ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::SetSettings( const CMcpServerSettings& settings )
{GUCEF_TRACE;

    return ApplySettings( settings );
}

/*-------------------------------------------------------------------------*/

const CMcpServerSettings&
CMcpServer::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

const CMcpRequestStateSealer&
CMcpServer::GetRequestStateSealer( void ) const
{GUCEF_TRACE;

    return m_sealer;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = cfg.FindChild( CMcpServerSettings::ConfigSectionName );
    if ( GUCEF_NULL == section )
        section = cfg.AddChild( CMcpServerSettings::ConfigSectionName, GUCEF_DATATYPE_OBJECT );
    MT::CScopeMutex lock( m_configLock );
    return m_settings.SaveConfig( *section );
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    const CORE::CDataNode* section = cfg.FindChild( CMcpServerSettings::ConfigSectionName );
    CMcpServerSettings settings( m_settings );
    if ( !settings.LoadConfig( GUCEF_NULL != section ? *section : cfg ) )
        return false;
    if ( !ApplySettings( settings ) )
        return false;

    // The configured transports are made through the factory. Those of an earlier load are replaced
    if ( !settings.transports.HasChildren() )
        return true;

    const TTransportPtrVector earlier = m_configuredTransports;
    m_configuredTransports.clear();
    for ( size_t n=0; n<earlier.size(); ++n )
    {
        for ( size_t i=0; i<m_transports.size(); ++i )
        {
            if ( m_transports[ i ].GetPointerAlways() == earlier[ n ] )
            {
                CIMcpTransportPtr transport = m_transports[ i ];
                RemoveTransport( transport );
                break;
            }
        }
    }

    bool allMade = true;
    for ( CORE::CDataNode::const_iterator i = settings.transports.ConstBegin(); i != settings.transports.ConstEnd(); ++i )
    {
        const CORE::CDataNode& entry = *(*i);
        const CORE::CString type = entry.GetAttributeValueOrChildValueByName( "type" ).AsString( CORE::CString::Empty, true );
        CIMcpTransportPtr transport = CComGlobal::Instance()->GetMcpTransportFactory().Create( type );
        if ( transport.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: There is no transport of type \"" << type << "\"" );
            allMade = false;
            continue;
        }
        if ( !transport->LoadConfig( entry ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: The configuration of the \"" << type << "\" transport is not valid" );
            allMade = false;
            continue;
        }
        if ( AddTransport( transport ) )
            m_configuredTransports.push_back( transport.GetPointerAlways() );
        else
            allMade = false;
    }
    return allMade;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::SetServerInfo( const CMcpImplementationInfo& info )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_configLock );
    m_serverInfo = info;
    m_settings.serverName = info.name;
    m_settings.serverTitle = info.title;
    m_settings.serverVersion = info.version;
    m_serverInfo.ToDataNode( m_serverInfoNode );
}

/*-------------------------------------------------------------------------*/

const CMcpImplementationInfo&
CMcpServer::GetServerInfo( void ) const
{GUCEF_TRACE;

    return m_serverInfo;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::SetInstructions( const CORE::CString& instructions )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_configLock );
    m_settings.instructions = instructions;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::SetAccessPolicy( const CIMcpAccessPolicyPtr& policy )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_configLock );
    m_policy = policy;
}

/*-------------------------------------------------------------------------*/

CIMcpAccessPolicyPtr
CMcpServer::GetAccessPolicy( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_configLock );
    return m_policy.IsNULL() ? m_defaultPolicy : m_policy;
}

/*-------------------------------------------------------------------------*/

const CIMcpAccessPolicy&
CMcpServer::GetPolicy( CIMcpAccessPolicyPtr& holder ) const
{GUCEF_TRACE;

    holder = GetAccessPolicy();
    return *holder;
}

/*-------------------------------------------------------------------------*/

CMcpToolRegistry&
CMcpServer::GetToolRegistry( void )
{GUCEF_TRACE;

    return m_toolRegistry;
}

/*-------------------------------------------------------------------------*/

CMcpResourceRegistry&
CMcpServer::GetResourceRegistry( void )
{GUCEF_TRACE;

    return m_resourceRegistry;
}

/*-------------------------------------------------------------------------*/

CMcpPromptRegistry&
CMcpServer::GetPromptRegistry( void )
{GUCEF_TRACE;

    return m_promptRegistry;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CMcpServer::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpServer::NowUnixMs( void ) const
{GUCEF_TRACE;

    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpServer::NowMonotonicMs( void )
{GUCEF_TRACE;

    return CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::IsValidRequestId( const CORE::CVariant& id )
{GUCEF_TRACE;

    // MCP: a string or an integer, never null. An integer beyond Int64 arrives as a float and is refused with the rest
    return id.IsInitialized() && ( id.IsString() || id.IsInteger() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMcpServer::IdKey( const CORE::CVariant& id )
{GUCEF_TRACE;

    // "1" and 1 are different ids
    if ( id.IsString() )
        return "s:" + id.AsString();
    if ( id.IsUnsignedInteger() )
        return "i:" + CORE::UInt64ToString( id.AsUInt64() );
    return "i:" + CORE::Int64ToString( id.AsInt64() );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::OnTransportError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( TransportErrorEvent, eventData );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::RequestShutdown( const CORE::CString& reason )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: A shutdown is requested: " << reason );
    CORE::TCloneableString eData( reason );
    NotifyObservers( ShutdownRequestedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::AddTransport( const CIMcpTransportPtr& transportToAdd )
{GUCEF_TRACE;

    CIMcpTransportPtr transport( transportToAdd );

    if ( transport.IsNULL() )
        return false;
    for ( size_t i=0; i<m_transports.size(); ++i )
        if ( m_transports[ i ] == transport )
            return false;
    m_transports.push_back( transport );

    // what goes wrong in a transport is reported by the server as well
    typedef CORE::CTEventHandlerFunctor< CMcpServer > TEventCallback;
    TEventCallback errorCallback( this, &CMcpServer::OnTransportError );
    SubscribeTo( transport.GetPointerAlways(), CIMcpTransport::TransportErrorEvent, errorCallback );

    if ( m_started )
        return transport->Start( *this );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::RemoveTransport( const CIMcpTransportPtr& transportToRemove )
{GUCEF_TRACE;

    CIMcpTransportPtr transport( transportToRemove );

    for ( TTransportVector::iterator i = m_transports.begin(); i != m_transports.end(); ++i )
    {
        if ( (*i) == transport )
        {
            transport->Stop( true );
            UnsubscribeFrom( transport.GetPointerAlways() );
            for ( TTransportPtrVector::iterator c = m_configuredTransports.begin(); c != m_configuredTransports.end(); ++c )
            {
                if ( (*c) == transport.GetPointerAlways() )
                {
                    m_configuredTransports.erase( c );
                    break;
                }
            }
            m_transports.erase( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::Start( void )
{GUCEF_TRACE;

    if ( m_started )
        return true;
    m_started = true;
    UpdatePeriodicPulses();
    bool allStarted = true;
    for ( size_t i=0; i<m_transports.size(); ++i )
    {
        if ( !m_transports[ i ]->Start( *this ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: The transport \"" << m_transports[ i ]->GetTransportName() << "\" did not start" );
            allStarted = false;
        }
    }
    return allStarted;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::IsStarted( void ) const
{GUCEF_TRACE;

    return m_started;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::Stop( bool graceful )
{GUCEF_TRACE;

    FlushRefusalSummary( NowMonotonicMs(), true );

    // subscriptions end first: graceful closure before the transports go
    for ( TChannelStateMap::iterator i = m_channels.begin(); i != m_channels.end(); ++i )
    {
        SChannelState& state = *(*i).second;
        for ( TSubscriptionMap::iterator n = state.subscriptions.begin(); n != state.subscriptions.end(); ++n )
            EndSubscription( state, (*n).second, graceful && !state.closed );
        state.subscriptions.clear();
    }

    // requests in flight are cancelled; their pool tasks get the grace time to end
    GUCEF::vector< SInFlight* > inFlight;
    for ( TContextMap::iterator i = m_inFlightByContext.begin(); i != m_inFlightByContext.end(); ++i )
        inFlight.push_back( (*i).second );
    TFutureVector futures;
    for ( size_t i=0; i<inFlight.size(); ++i )
    {
        if ( inFlight[ i ]->hasFuture )
            futures.push_back( inFlight[ i ]->future );
        CancelInFlight( inFlight[ i ], MCP_OUTCOME_CANCELLED );
    }
    if ( graceful )
    {
        UInt64 deadline = NowMonotonicMs() + m_settings.cancelGraceMs;
        for ( size_t i=0; i<futures.size(); ++i )
        {
            UInt64 now = NowMonotonicMs();
            if ( now >= deadline )
                break;
            futures[ i ].Await( (Int32) ( deadline - now ) );
        }
    }

    for ( size_t i=0; i<m_transports.size(); ++i )
        m_transports[ i ]->Stop( graceful );

    m_started = false;
    UpdatePeriodicPulses();
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpServer::GetChannelCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TChannelStateMap::const_iterator i = m_channels.begin(); i != m_channels.end(); ++i )
        if ( !(*i).second->closed )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpServer::GetInFlightCount( void ) const
{GUCEF_TRACE;

    return m_inFlightTotal;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpServer::GetSubscriptionCount( void ) const
{GUCEF_TRACE;

    return m_subscriptionsTotal;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::OpenChannel( const CMcpChannelPtr& channelToOpen )
{GUCEF_TRACE;

    CMcpChannelPtr channel( channelToOpen );

    if ( channel.IsNULL() || channel->IsOpen() )
        return false;

    UInt64 channelId = ++m_nextChannelId;
    SChannelState* state = GUCEF_NEW SChannelState();
    state->channel = channel;
    m_channels[ channelId ] = state;
    channel->AttachToServer( this, channelId, m_settings.maxMessageBytes, m_settings.maxJsonDepth );

    SMcpChannelEventInfo info;
    info.channelId = channelId;
    info.transport = channel->GetPrincipal().transport;
    info.subject = channel->GetPrincipal().subject;
    TMcpChannelEventData eData( info );
    NotifyObservers( ChannelOpenedEvent, &eData );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::CloseChannel( const CMcpChannelPtr& channelToClose )
{GUCEF_TRACE;

    CMcpChannelPtr channel( channelToClose );

    if ( channel.IsNULL() )
        return;
    const UInt64 channelId = channel->GetChannelId();
    TChannelStateMap::iterator found = m_channels.find( channelId );
    if ( found == m_channels.end() || (*found).second->closed )
        return;
    SChannelState* state = (*found).second;
    state->closed = true;

    // the subscriptions end without a message: there is no one to send it to
    for ( TSubscriptionMap::iterator i = state->subscriptions.begin(); i != state->subscriptions.end(); ++i )
        EndSubscription( *state, (*i).second, false );
    state->subscriptions.clear();

    // a modern request, or one on a stream channel, is cancelled by the close. A legacy request over HTTP runs to
    // its end, as 2025-11-25 does not take a disconnect for a cancellation; its result is dropped then
    GUCEF::vector< SInFlight* > cancelled;
    for ( TInFlightMap::iterator i = state->inFlight.begin(); i != state->inFlight.end(); ++i )
    {
        SInFlight* inFlight = (*i).second;
        if ( !( CMcpChannel::CHANNEL_EXCHANGE == channel->GetKind() && MCP_ERA_LEGACY == inFlight->dispatch.era ) )
            cancelled.push_back( inFlight );
    }
    channel->DetachFromServer();
    for ( size_t i=0; i<cancelled.size(); ++i )
        CancelInFlight( cancelled[ i ], MCP_OUTCOME_CANCELLED );

    SMcpChannelEventInfo info;
    info.channelId = channelId;
    info.transport = channel->GetPrincipal().transport;
    info.subject = channel->GetPrincipal().subject;
    TMcpChannelEventData eData( info );
    NotifyObservers( ChannelClosedEvent, &eData );

    found = m_channels.find( channelId );
    if ( found != m_channels.end() && (*found).second->inFlight.empty() )
    {
        GUCEF_DELETE (*found).second;
        m_channels.erase( found );
    }
}

/*-------------------------------------------------------------------------*/

CMcpServer::SChannelState*
CMcpServer::GetChannelState( UInt64 channelId )
{GUCEF_TRACE;

    TChannelStateMap::iterator i = m_channels.find( channelId );
    return i == m_channels.end() ? GUCEF_NULL : (*i).second;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::RaiseRequestStarted( const SDispatch& dispatch )
{GUCEF_TRACE;

    SMcpRequestEventInfo info;
    info.method = dispatch.method;
    info.target = dispatch.target;
    info.requestId = dispatch.inProcess ? CORE::CString::Empty : dispatch.id.AsString();
    info.transport = dispatch.principal.transport;
    info.subject = dispatch.principal.subject;
    info.channelId = dispatch.channelId;
    info.era = dispatch.era;
    info.protocolVersion = dispatch.meta.GetProtocolVersion();
    RaiseRequestEvent( false, info );
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::IsProtocolThread( void ) const
{GUCEF_TRACE;

    // kept by the server itself: the pulse generator does not offer its driver thread id to other threads safely
    TThreadId protocolThreadId = m_protocolThreadId.Load();
    return 0 != protocolThreadId && MT::GetCurrentThreadID() == protocolThreadId;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::RaiseRequestEvent( bool finished, const SMcpRequestEventInfo& info, const CMcpAuditRecord* audit )
{GUCEF_TRACE;

    if ( !IsProtocolThread() )
    {
        SQueuedRequestEvent queued;
        queued.finished = finished;
        queued.info = info;
        queued.hasAudit = GUCEF_NULL != audit;
        if ( GUCEF_NULL != audit )
            queued.audit = *audit;
        MT::CScopeMutex lock( m_eventQueueLock );
        m_queuedRequestEvents.push_back( queued );
        return;
    }
    RaiseQueuedRequestEvents();
    TMcpRequestEventData eData( info );
    NotifyObservers( finished ? RequestFinishedEvent : RequestStartedEvent, &eData );
    if ( GUCEF_NULL != audit )
    {
        TMcpAuditEventData auditData( *audit );
        NotifyObservers( AuditEvent, &auditData );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::RaiseQueuedRequestEvents( void )
{GUCEF_TRACE;

    GUCEF::vector< SQueuedRequestEvent > queued;
    {
        MT::CScopeMutex lock( m_eventQueueLock );
        if ( m_queuedRequestEvents.empty() )
            return;
        queued.swap( m_queuedRequestEvents );
    }
    for ( size_t i=0; i<queued.size(); ++i )
    {
        TMcpRequestEventData eData( queued[ i ].info );
        NotifyObservers( queued[ i ].finished ? RequestFinishedEvent : RequestStartedEvent, &eData );
        if ( queued[ i ].hasAudit )
        {
            TMcpAuditEventData auditData( queued[ i ].audit );
            NotifyObservers( AuditEvent, &auditData );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::RaiseRequestFinished( const SDispatch& dispatch, EMcpRequestOutcome outcome, Int32 errorCode )
{GUCEF_TRACE;

    SMcpRequestEventInfo info;
    info.method = dispatch.method;
    info.target = dispatch.target;
    info.requestId = dispatch.inProcess ? CORE::CString::Empty : dispatch.id.AsString();
    info.transport = dispatch.principal.transport;
    info.subject = dispatch.principal.subject;
    info.channelId = dispatch.channelId;
    info.era = dispatch.era;
    info.protocolVersion = dispatch.meta.GetProtocolVersion();
    info.outcome = outcome;
    info.errorCode = errorCode;
    UInt64 now = NowMonotonicMs();
    info.durationInMs = now > dispatch.startInMs ? now - dispatch.startInMs : 0;

    CMcpAuditRecord audit;
    BuildAuditRecord( dispatch, outcome, errorCode, info, audit );
    RaiseRequestEvent( true, info, &audit );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::BuildAuditRecord( const SDispatch& dispatch, EMcpRequestOutcome outcome, Int32 errorCode, const SMcpRequestEventInfo& info, CMcpAuditRecord& record ) const
{GUCEF_TRACE;

    // everything a client supplied passes the redactor before it can reach a log
    const CORE::CRedactor& lineRedactor = CORE::CRedactor::ForLogLine();
    record.timestampUtc = CORE::CDateTime::NowUTCDateTime();
    record.durationInMs = info.durationInMs;
    record.transport = dispatch.principal.transport;
    record.channelId = dispatch.channelId;
    record.era = dispatch.era;
    record.protocolVersion = info.protocolVersion;
    record.requestId = info.requestId;
    record.subject = dispatch.principal.subject;
    record.isAuthenticated = dispatch.principal.isAuthenticated;
    record.scopes = dispatch.principal.scopes;
    if ( dispatch.meta.HasClientInfo() )
    {
        record.clientName = lineRedactor.RedactText( dispatch.meta.GetClientInfo().name );
        record.clientVersion = lineRedactor.RedactText( dispatch.meta.GetClientInfo().version );
    }
    record.method = dispatch.method;
    record.target = CMcpMethods::ResourcesRead == dispatch.method ? CORE::CRedactor::ForUrl().RedactUrl( dispatch.target )
                                                                  : lineRedactor.RedactText( dispatch.target );
    record.hasDecision = dispatch.auditHasDecision;
    record.decision = dispatch.auditDecision;
    record.decisionReason = dispatch.auditReason;
    record.outcome = outcome;
    record.errorCode = errorCode;
    record.hasArguments = dispatch.auditHasArguments;
    record.argumentsDigest = dispatch.auditArgumentsDigest;
    record.argumentsPreview = dispatch.auditArgumentsPreview;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::CaptureAuditArguments( SDispatch& dispatch, const CORE::CDataNode* arguments, const CMcpToolEntry* tool ) const
{GUCEF_TRACE;

    // the digest is over the real arguments, the preview over the arguments with the secrets of the tool masked and the rest redacted
    UInt8 digest[ 32 ];
    CMcpRequestStateSealer::DigestArguments( arguments, digest );
    static const CORE::CString::StringSet noSecrets;
    dispatch.auditHasArguments = true;
    dispatch.auditArgumentsDigest = CMcpAuditRecord::DigestToHex( digest );
    dispatch.auditArgumentsPreview = CMcpAuditRecord::MakeArgumentsPreview( arguments, GUCEF_NULL != tool ? tool->descriptor.secretArguments : noSecrets );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::NoteDecision( SDispatch& dispatch, EMcpAccessDecision decision, const CORE::CString& reason )
{GUCEF_TRACE;

    dispatch.auditHasDecision = true;
    dispatch.auditDecision = decision;
    dispatch.auditReason = reason;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::NoteNotVisible( SDispatch& dispatch, bool found )
{GUCEF_TRACE;

    // an object that does not exist is no decision of the policy; one that exists and is not visible to the principal is
    if ( found )
        NoteDecision( dispatch, MCP_ACCESS_HIDE, "not visible to the principal under the access policy" );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::AddResultMeta( const SDispatch& dispatch, CORE::CDataNode& result ) const
{GUCEF_TRACE;

    // results of legacy requests carry no resultType and no serverInfo
    if ( MCP_ERA_MODERN != dispatch.era )
        return;
    if ( GUCEF_NULL == result.GetAttribute( "resultType" ) )
        result.SetAttribute( "resultType", CORE::CVariant( CMcpResultType::Complete ) );
    if ( !m_settings.includeServerInfo )
        return;

    CORE::CDataNode* meta = result.FindChild( "_meta" );
    if ( GUCEF_NULL == meta )
        meta = result.AddChild( "_meta", GUCEF_DATATYPE_OBJECT );
    meta->DelChild( CMcpMetaKeys::ServerInfo );
    CORE::CDataNode* serverInfo = meta->AddChild( m_serverInfoNode );
    serverInfo->SetName( CMcpMetaKeys::ServerInfo );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::AddCacheHints( const SDispatch& dispatch, CORE::CDataNode& result, UInt32 ttlMs, EMcpCacheScope scope ) const
{GUCEF_TRACE;

    if ( MCP_ERA_MODERN != dispatch.era )
        return;
    result.SetAttribute( "ttlMs", (UInt64) ttlMs );
    result.SetAttribute( "cacheScope", CORE::CVariant( McpCacheScopeToString( scope ) ) );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::SendResult( SDispatch& dispatch, CORE::CDataNode& result, EMcpRequestOutcome outcome )
{GUCEF_TRACE;

    if ( dispatch.finished )
        return;
    dispatch.finished = true;
    if ( !dispatch.channel.IsNULL() )
        dispatch.channel->SendResult( dispatch.id, result );
    RaiseRequestFinished( dispatch, outcome, 0 );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::SendError( SDispatch& dispatch, Int32 code, const CORE::CString& message, const CORE::CDataNode* data )
{GUCEF_TRACE;

    if ( dispatch.finished )
        return;
    dispatch.finished = true;
    if ( !dispatch.channel.IsNULL() )
        dispatch.channel->SendError( dispatch.id, code, message, data );
    RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, code );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::SendIdInUseError( SDispatch& dispatch )
{GUCEF_TRACE;

    // the answer to the request in progress is the one response with that id: the client would not know which of two
    // responses belongs to which request. JSON-RPC answers an error it cannot tie to a request with a null id
    if ( dispatch.finished )
        return;
    dispatch.finished = true;
    if ( !dispatch.channel.IsNULL() )
        dispatch.channel->SendError( CORE::CVariant(), MCP_ERROR_INVALID_REQUEST, "Invalid request: the id is in use by a request in progress" );
    RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_INVALID_REQUEST );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::OnChannelRequest( CMcpChannel& channel, const SJsonRpcRequest& request )
{GUCEF_TRACE;

    SChannelState* state = GetChannelState( channel.GetChannelId() );
    if ( GUCEF_NULL == state || state->closed )
        return;

    // 1. the envelope: the endpoint checked JSON-RPC, MCP wants a string or integer id
    if ( !IsValidRequestId( request.id ) )
    {
        channel.SendError( CORE::CVariant(), MCP_ERROR_INVALID_REQUEST, "Invalid request: the id must be a string or an integer" );
        return;
    }

    SDispatch dispatch;
    dispatch.channel = state->channel;
    dispatch.channelId = channel.GetChannelId();
    dispatch.id = request.id;
    dispatch.idKey = IdKey( request.id );
    dispatch.method = request.method;
    dispatch.principal = channel.GetPrincipal();
    dispatch.startInMs = NowMonotonicMs();

    const CORE::CDataNode* params = request.hasParams ? &request.params : GUCEF_NULL;

    // 2. the era of the request
    const CORE::CDataNode* metaNode = GUCEF_NULL;
    bool metaIsScalar = false;
    bool hasModernVersion = false;
    if ( GUCEF_NULL != params && GUCEF_DATATYPE_OBJECT == params->GetNodeType() )
    {
        metaNode = params->FindChild( "_meta" );
        metaIsScalar = GUCEF_NULL != params->GetAttribute( "_meta" );
        hasModernVersion = GUCEF_NULL != metaNode &&
                           ( GUCEF_NULL != metaNode->GetAttribute( CMcpMetaKeys::ProtocolVersion ) || GUCEF_NULL != metaNode->FindChild( CMcpMetaKeys::ProtocolVersion ) );
    }
    bool isLegacyChannel = m_settings.supportLegacy && channel.IsLegacyRequest( request );
    if ( hasModernVersion )
        dispatch.era = MCP_ERA_MODERN;
    else
    if ( request.method == CMcpMethods::Initialize || isLegacyChannel )
        dispatch.era = MCP_ERA_LEGACY;
    else
        dispatch.era = MCP_ERA_MODERN;

    if ( MCP_ERA_LEGACY == dispatch.era )
    {
        const CMcpLegacySession* session = channel.GetLegacySession();
        if ( GUCEF_NULL != session && request.method != CMcpMethods::Initialize )
            dispatch.meta.SetForLegacy( session->protocolVersion, session->clientCapabilities, session->hasClientInfo ? &session->clientInfo : GUCEF_NULL );
        else
            dispatch.meta.SetForLegacy( CMcpProtocolVersions::Version_2025_11_25, CORE::CDataNode( GUCEF_DATATYPE_OBJECT ), GUCEF_NULL );
        if ( GUCEF_NULL != metaNode )
        {
            const CORE::CDataNode::TKeyValuePair* token = metaNode->GetAttribute( CMcpMetaKeys::ProgressToken );
            if ( GUCEF_NULL != token )
                dispatch.meta.SetProgressToken( (*token).second );
        }
    }
    else
    {
        // 3. the _meta of a modern request
        CORE::CString metaError;
        CMcpRequestMeta::EParseResult parsed = metaIsScalar ? CMcpRequestMeta::META_MALFORMED : dispatch.meta.Parse( metaNode, metaError );
        if ( metaIsScalar )
            metaError = "_meta must be an object";
        if ( CMcpRequestMeta::META_OK != parsed )
        {
            RaiseRequestStarted( dispatch );
            if ( state->inFlight.end() != state->inFlight.find( dispatch.idKey ) )
            {
                SendIdInUseError( dispatch );
                return;
            }
            if ( CMcpRequestMeta::META_NO_PROTOCOL_VERSION == parsed )
                SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: _meta with \"" + CMcpMetaKeys::ProtocolVersion + "\" and \"" + CMcpMetaKeys::ClientCapabilities + "\" is required" );
            else
                SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: " + metaError );
            return;
        }
    }

    RaiseRequestStarted( dispatch );

    if ( state->inFlight.end() != state->inFlight.find( dispatch.idKey ) || state->subscriptions.end() != state->subscriptions.find( dispatch.idKey ) )
    {
        SendIdInUseError( dispatch );
        return;
    }
    if ( GUCEF_NULL != params && GUCEF_DATATYPE_OBJECT != params->GetNodeType() )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: params must be an object" );
        return;
    }

    // 4. what the transport checks, the HTTP headers
    Int32 headerErrorCode = 0;
    CORE::CString headerErrorMessage;
    if ( !channel.ValidateRequestHeaders( request, MCP_ERA_MODERN == dispatch.era ? &dispatch.meta : GUCEF_NULL, dispatch.era, headerErrorCode, headerErrorMessage ) )
    {
        SendError( dispatch, headerErrorCode, headerErrorMessage );
        return;
    }
    channel.CompleteRequestPrincipal( dispatch.principal );

    if ( MCP_ERA_MODERN == dispatch.era )
    {
        // 5. the version
        if ( !CMcpProtocolVersions::IsModernVersion( dispatch.meta.GetProtocolVersion() ) )
        {
            CORE::CDataNode data( GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* supported = data.AddChild( "supported", GUCEF_DATATYPE_ARRAY );
            CORE::CString::StringVector versions;
            CMcpProtocolVersions::GetModernVersions( versions );
            for ( size_t i=0; i<versions.size(); ++i )
                supported->AddValueAsChild( CORE::CVariant( versions[ i ] ) );
            if ( m_settings.supportLegacy )
            {
                CMcpProtocolVersions::GetLegacyVersions( versions );
                for ( size_t i=0; i<versions.size(); ++i )
                    supported->AddValueAsChild( CORE::CVariant( versions[ i ] ) );
            }
            data.SetAttribute( "requested", CORE::CVariant( dispatch.meta.GetProtocolVersion() ) );
            SendError( dispatch, MCP_ERROR_UNSUPPORTED_PROTOCOL_VERSION, "Unsupported protocol version", &data );
            return;
        }
        DispatchModern( dispatch, request, params );
    }
    else
    {
        DispatchLegacy( dispatch, request, params );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::DispatchModern( SDispatch& dispatch, const SJsonRpcRequest& request, const CORE::CDataNode* params )
{GUCEF_TRACE;

    // 6. the method and its registry
    const CORE::CString& method = dispatch.method;
    if ( method == CMcpMethods::ServerDiscover )
        HandleDiscover( dispatch );
    else
    if ( method == CMcpMethods::ToolsList && m_settings.enableTools )
        HandleList( dispatch, params );
    else
    if ( method == CMcpMethods::ToolsCall && m_settings.enableTools )
        HandleToolCall( dispatch, params );
    else
    if ( ( method == CMcpMethods::ResourcesList || method == CMcpMethods::ResourcesTemplatesList ) && m_settings.enableResources )
        HandleList( dispatch, params );
    else
    if ( method == CMcpMethods::ResourcesRead && m_settings.enableResources )
        HandleResourceRead( dispatch, params );
    else
    if ( method == CMcpMethods::PromptsList && m_settings.enablePrompts )
        HandleList( dispatch, params );
    else
    if ( method == CMcpMethods::PromptsGet && m_settings.enablePrompts )
        HandlePromptGet( dispatch, params );
    else
    if ( method == CMcpMethods::SubscriptionsListen && ( m_settings.enableTools || m_settings.enableResources || m_settings.enablePrompts ) )
        HandleListen( dispatch, params );
    else
        SendError( dispatch, MCP_ERROR_METHOD_NOT_FOUND, "Method not found: " + method );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::DispatchLegacy( SDispatch& dispatch, const SJsonRpcRequest& request, const CORE::CDataNode* params )
{GUCEF_TRACE;

    const CORE::CString& method = dispatch.method;
    if ( method == CMcpMethods::Initialize )
        HandleInitialize( dispatch, params );
    else
    if ( method == CMcpMethods::Ping )
    {
        CORE::CDataNode result( GUCEF_DATATYPE_OBJECT );
        SendResult( dispatch, result, MCP_OUTCOME_COMPLETE );
    }
    else
    if ( method == CMcpMethods::ToolsList && m_settings.enableTools )
        HandleList( dispatch, params );
    else
    if ( method == CMcpMethods::ToolsCall && m_settings.enableTools )
        HandleToolCall( dispatch, params );
    else
    if ( ( method == CMcpMethods::ResourcesList || method == CMcpMethods::ResourcesTemplatesList ) && m_settings.enableResources )
        HandleList( dispatch, params );
    else
    if ( method == CMcpMethods::ResourcesRead && m_settings.enableResources )
        HandleResourceRead( dispatch, params );
    else
    if ( ( method == CMcpMethods::ResourcesSubscribe || method == CMcpMethods::ResourcesUnsubscribe ) && m_settings.enableResources &&
         CMcpChannel::CHANNEL_STREAM == dispatch.channel->GetKind() && GUCEF_NULL != dispatch.channel->GetLegacySession() )
        HandleLegacySubscribe( dispatch, params, method == CMcpMethods::ResourcesSubscribe );
    else
    if ( method == CMcpMethods::PromptsList && m_settings.enablePrompts )
        HandleList( dispatch, params );
    else
    if ( method == CMcpMethods::PromptsGet && m_settings.enablePrompts )
        HandlePromptGet( dispatch, params );
    else
        SendError( dispatch, MCP_ERROR_METHOD_NOT_FOUND, "Method not found: " + method );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::HandleDiscover( SDispatch& dispatch )
{GUCEF_TRACE;

    CORE::CDataNode result( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* versions = result.AddChild( "supportedVersions", GUCEF_DATATYPE_ARRAY );
    CORE::CString::StringVector modern;
    CMcpProtocolVersions::GetModernVersions( modern );
    for ( size_t i=0; i<modern.size(); ++i )
        versions->AddValueAsChild( CORE::CVariant( modern[ i ] ) );

    CORE::CDataNode* capabilities = result.AddChild( "capabilities", GUCEF_DATATYPE_OBJECT );
    if ( m_settings.enableTools )
        capabilities->AddChild( "tools", GUCEF_DATATYPE_OBJECT )->SetAttribute( "listChanged", true );
    if ( m_settings.enableResources )
    {
        CORE::CDataNode* resources = capabilities->AddChild( "resources", GUCEF_DATATYPE_OBJECT );
        resources->SetAttribute( "subscribe", true );
        resources->SetAttribute( "listChanged", true );
    }
    if ( m_settings.enablePrompts )
        capabilities->AddChild( "prompts", GUCEF_DATATYPE_OBJECT )->SetAttribute( "listChanged", true );

    {
        MT::CScopeMutex lock( m_configLock );
        if ( !m_settings.instructions.IsNULLOrEmpty() )
            result.SetAttribute( "instructions", CORE::CVariant( m_settings.instructions ) );
    }
    AddCacheHints( dispatch, result, m_settings.discoverTtlMs, MCP_CACHESCOPE_PUBLIC );
    AddResultMeta( dispatch, result );
    SendResult( dispatch, result, MCP_OUTCOME_COMPLETE );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::HandleList( SDispatch& dispatch, const CORE::CDataNode* params )
{GUCEF_TRACE;

    // 7. the params: an optional cursor, the base64url of the last key of the previous page
    CORE::CString afterKey;
    bool hasCursor = false;
    if ( GUCEF_NULL != params )
    {
        CORE::CString cursor;
        if ( !GetOptionalStringMember( *params, "cursor", cursor, hasCursor ) )
        {
            SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: cursor must be a string" );
            return;
        }
        if ( hasCursor )
        {
            CORE::CDynamicBuffer key;
            if ( !CORE::Base64UrlDecode( cursor, key ) )
            {
                SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid cursor" );
                return;
            }
            if ( key.GetDataSize() > 0 )
                afterKey = CORE::CString( static_cast< const char* >( key.GetConstBufferPtr() ), key.GetDataSize() );
        }
    }

    CIMcpAccessPolicyPtr holder;
    const CIMcpAccessPolicy& policy = GetPolicy( holder );
    const EMcpCacheScope scope = policy.IsVisibilityPrincipalDependent() ? MCP_CACHESCOPE_PRIVATE : MCP_CACHESCOPE_PUBLIC;
    const UInt32 pageSize = m_settings.pageSize > 0 ? m_settings.pageSize : 1;

    CORE::CDataNode result( GUCEF_DATATYPE_OBJECT );
    CORE::CString lastKey;
    bool more = false;
    UInt32 count = 0;

    if ( dispatch.method == CMcpMethods::ToolsList )
    {
        UInt64 generation = m_toolRegistry.GetGeneration();
        if ( generation != m_toolSnapshotGeneration || 0 == generation )
        {
            m_toolRegistry.GetEntries( m_toolSnapshot );
            m_toolSnapshotGeneration = generation;
        }
        CORE::CDataNode* items = result.AddChild( "tools", GUCEF_DATATYPE_ARRAY );
        for ( size_t i = hasCursor ? FirstAfter( m_toolSnapshot, afterKey ) : 0; i < m_toolSnapshot.size(); ++i )
        {
            const CMcpToolEntryPtr& entry = m_toolSnapshot[ i ];
            if ( !policy.IsToolVisible( dispatch.principal, *entry ) )
                continue;
            if ( count == pageSize )
            {
                more = true;
                break;
            }
            items->AddChild( entry->descriptorNode );
            lastKey = KeyOf( entry );
            ++count;
        }
    }
    else
    if ( dispatch.method == CMcpMethods::ResourcesList || dispatch.method == CMcpMethods::ResourcesTemplatesList )
    {
        UInt64 generation = m_resourceRegistry.GetGeneration();
        if ( generation != m_resourceSnapshotGeneration || 0 == generation )
        {
            m_resourceRegistry.GetResources( m_resourceSnapshot );
            m_resourceRegistry.GetTemplates( m_templateSnapshot );
            m_resourceSnapshotGeneration = generation;
        }
        if ( dispatch.method == CMcpMethods::ResourcesList )
        {
            CORE::CDataNode* items = result.AddChild( "resources", GUCEF_DATATYPE_ARRAY );
            for ( size_t i = hasCursor ? FirstAfter( m_resourceSnapshot, afterKey ) : 0; i < m_resourceSnapshot.size(); ++i )
            {
                const CMcpResourceEntryPtr& entry = m_resourceSnapshot[ i ];
                if ( !policy.IsResourceVisible( dispatch.principal, entry->descriptor.uri ) )
                    continue;
                if ( count == pageSize )
                {
                    more = true;
                    break;
                }
                items->AddChild( entry->descriptorNode );
                lastKey = KeyOf( entry );
                ++count;
            }
        }
        else
        {
            CORE::CDataNode* items = result.AddChild( "resourceTemplates", GUCEF_DATATYPE_ARRAY );
            for ( size_t i = hasCursor ? FirstAfter( m_templateSnapshot, afterKey ) : 0; i < m_templateSnapshot.size(); ++i )
            {
                const CMcpResourceTemplateEntryPtr& entry = m_templateSnapshot[ i ];
                if ( !policy.IsResourceVisible( dispatch.principal, entry->descriptor.uriTemplate ) )
                    continue;
                if ( count == pageSize )
                {
                    more = true;
                    break;
                }
                items->AddChild( entry->descriptorNode );
                lastKey = KeyOf( entry );
                ++count;
            }
        }
    }
    else
    {
        UInt64 generation = m_promptRegistry.GetGeneration();
        if ( generation != m_promptSnapshotGeneration || 0 == generation )
        {
            m_promptRegistry.GetEntries( m_promptSnapshot );
            m_promptSnapshotGeneration = generation;
        }
        CORE::CDataNode* items = result.AddChild( "prompts", GUCEF_DATATYPE_ARRAY );
        for ( size_t i = hasCursor ? FirstAfter( m_promptSnapshot, afterKey ) : 0; i < m_promptSnapshot.size(); ++i )
        {
            const CMcpPromptEntryPtr& entry = m_promptSnapshot[ i ];
            if ( !policy.IsPromptVisible( dispatch.principal, *entry ) )
                continue;
            if ( count == pageSize )
            {
                more = true;
                break;
            }
            items->AddChild( entry->descriptorNode );
            lastKey = KeyOf( entry );
            ++count;
        }
    }

    if ( more )
    {
        CORE::CString nextCursor;
        CORE::Base64UrlEncode( lastKey.C_String(), lastKey.ByteSize() > 0 ? lastKey.ByteSize() - 1 : 0, nextCursor );
        result.SetAttribute( "nextCursor", CORE::CVariant( nextCursor ) );
    }
    AddCacheHints( dispatch, result, m_settings.listTtlMs, scope );
    AddResultMeta( dispatch, result );
    SendResult( dispatch, result, MCP_OUTCOME_COMPLETE );
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::IsInFlightLimitReached( UInt64 channelId ) const
{GUCEF_TRACE;

    if ( m_inFlightTotal >= m_settings.maxInFlightTotal )
        return true;
    TChannelStateMap::const_iterator i = m_channels.find( channelId );
    return i != m_channels.end() && (*i).second->inFlight.size() >= m_settings.maxInFlightPerChannel;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::TakeToolCallToken( const CMcpPrincipal& principal )
{GUCEF_TRACE;

    if ( 0 == m_settings.toolCallsPerSecond )
        return true;

    const Float64 burst = (Float64) ( m_settings.toolCallBurst > 0 ? m_settings.toolCallBurst : 1 );
    const UInt64 now = NowMonotonicMs();
    MT::CScopeMutex lock( m_rateLock );
    TTokenBucketMap::iterator i = m_tokenBuckets.find( principal.subject );
    if ( i == m_tokenBuckets.end() )
    {
        STokenBucket bucket;
        bucket.tokens = burst;
        bucket.lastRefillInMs = now;
        i = m_tokenBuckets.insert( TTokenBucketMap::value_type( principal.subject, bucket ) ).first;
    }
    STokenBucket& bucket = (*i).second;
    if ( now > bucket.lastRefillInMs )
    {
        bucket.tokens += ( now - bucket.lastRefillInMs ) * (Float64) m_settings.toolCallsPerSecond / 1000.0;
        if ( bucket.tokens > burst )
            bucket.tokens = burst;
        bucket.lastRefillInMs = now;
    }
    if ( bucket.tokens < 1.0 )
        return false;
    bucket.tokens -= 1.0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::PrepareCall( SDispatch& dispatch, const CORE::CDataNode* params, const CORE::CDataNode* arguments, CMcpOperationTaskData& call )
{GUCEF_TRACE;

    dispatch.context = CMcpRequestContext::Create( dispatch.method, dispatch.target, dispatch.id, dispatch.era,
                                                   dispatch.principal, dispatch.meta, dispatch.channelId );
    call.context = dispatch.context;
    CMcpRequestStateSealer::DigestArguments( arguments, dispatch.argumentsDigest );

    // 10. the state of a multi round-trip retry; legacy requests know neither field
    if ( MCP_ERA_MODERN != dispatch.era || GUCEF_NULL == params )
        return true;

    const CORE::CDataNode* inputResponses = GUCEF_NULL;
    if ( !GetOptionalObjectMember( *params, "inputResponses", inputResponses ) )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: inputResponses must be an object" );
        return false;
    }
    if ( GUCEF_NULL != inputResponses )
        dispatch.context->SetInputResponses( *inputResponses );

    CORE::CString sealed;
    bool hasState = false;
    if ( !GetOptionalStringMember( *params, "requestState", sealed, hasState ) )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: requestState must be a string" );
        return false;
    }
    if ( hasState )
    {
        CMcpRequestStateSealer::SBinding binding;
        binding.subject = dispatch.principal.subject;
        binding.method = dispatch.method;
        binding.target = dispatch.target;
        ::memcpy( binding.argumentsDigest, dispatch.argumentsDigest, sizeof( binding.argumentsDigest ) );
        CORE::CDataNode state;
        CORE::CString nonce;
        CMcpRequestStateSealer::EUnsealResult unsealed = m_sealer.Unseal( sealed, binding, NowUnixMs(), state, nonce );
        if ( CMcpRequestStateSealer::UNSEAL_OK != unsealed )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: The requestState of a " << dispatch.method << " request was refused (" << (Int32) unsealed << ")" );
            SendError( dispatch, MCP_ERROR_INVALID_PARAMS, RequestStateRefusedText );
            return false;
        }
        dispatch.context->SetRequestState( state, nonce );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::HandleToolCall( SDispatch& dispatch, const CORE::CDataNode* params )
{GUCEF_TRACE;

    // 7. the params
    const CORE::CDataNode::TKeyValuePair* name = GUCEF_NULL != params ? params->GetAttribute( "name" ) : GUCEF_NULL;
    if ( GUCEF_NULL == name || !(*name).second.IsString() )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: name must be a string" );
        return;
    }
    dispatch.target = (*name).second.AsString();
    const CORE::CDataNode* arguments = GUCEF_NULL;
    if ( !GetOptionalObjectMember( *params, "arguments", arguments ) )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: arguments must be an object" );
        return;
    }

    // the audit trail has the digest and the preview of the arguments whatever happens to the request
    CaptureAuditArguments( dispatch, arguments, GUCEF_NULL );

    // 8. the limits
    if ( !dispatch.inProcess && !TakeToolCallToken( dispatch.principal ) )
    {
        SendError( dispatch, MCP_ERROR_GUCEF_RATE_LIMITED, "Rate limited: too many tool calls, try again later" );
        return;
    }
    if ( IsInFlightLimitReached( dispatch.channelId ) )
    {
        SendError( dispatch, MCP_ERROR_GUCEF_BUSY, "Busy: too many requests in progress" );
        return;
    }

    // 9. the lookup and the policy
    CIMcpAccessPolicyPtr holder;
    const CIMcpAccessPolicy& policy = GetPolicy( holder );
    const bool toolFound = m_toolRegistry.TryGet( dispatch.target, dispatch.tool );
    if ( !toolFound || !policy.IsToolVisible( dispatch.principal, *dispatch.tool ) )
    {
        NoteNotVisible( dispatch, toolFound );
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Unknown tool: " + dispatch.target );
        return;
    }
    if ( !dispatch.tool->descriptor.secretArguments.empty() )
        CaptureAuditArguments( dispatch, arguments, dispatch.tool.GetPointerAlways() );

    CMcpOperationTaskData call;
    call.operation = CMcpOperationTaskData::OPERATION_TOOL_CALL;
    call.toolEntry = dispatch.tool;
    if ( GUCEF_NULL != arguments )
    {
        call.arguments = *arguments;
        call.arguments.SetName( CORE::CString::Empty );
    }
    if ( !PrepareCall( dispatch, params, &call.arguments, call ) )
        return;

    CORE::CString reason;
    EMcpAccessDecision decision = policy.AuthorizeToolCall( *dispatch.context, *dispatch.tool, call.arguments, reason );
    NoteDecision( dispatch, decision, reason );
    if ( MCP_ACCESS_HIDE == decision )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Unknown tool: " + dispatch.target );
        return;
    }
    if ( MCP_ACCESS_DENY == decision )
    {
        CMcpToolResult denied;
        denied.SetError( "Access denied" + ( reason.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + reason ) );
        CORE::CDataNode node( GUCEF_DATATYPE_OBJECT );
        denied.ToDataNode( node );
        AddResultMeta( dispatch, node );
        SendResult( dispatch, node, MCP_OUTCOME_TOOL_ERROR );
        return;
    }

    // 11. the arguments against the input schema: a failure is a tool execution error the model can fix. arguments is
    // optional in CallToolRequestParams: absent arguments are {} and judged like them
    CORE::CJsonSchemaValidator::TErrorVector errors;
    if ( !CORE::CJsonSchemaValidator::Validate( dispatch.tool->inputSchema, call.arguments, errors, MaxArgumentErrors ) )
    {
        CMcpToolResult invalid;
        invalid.SetError( "Invalid arguments for tool " + dispatch.target + ":\n" + FormatValidationErrors( errors ) );
        CORE::CDataNode node( GUCEF_DATATYPE_OBJECT );
        invalid.ToDataNode( node );
        AddResultMeta( dispatch, node );
        SendResult( dispatch, node, MCP_OUTCOME_TOOL_ERROR );
        return;
    }

    // 12. dispatch by execution mode
    RunCall( dispatch, call, dispatch.tool->tool->GetExecutionMode(), dispatch.tool->descriptor.maxDurationMs );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::HandleResourceRead( SDispatch& dispatch, const CORE::CDataNode* params )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* uri = GUCEF_NULL != params ? params->GetAttribute( "uri" ) : GUCEF_NULL;
    if ( GUCEF_NULL == uri || !(*uri).second.IsString() )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: uri must be a string" );
        return;
    }

    // the one spelling of the URI is what is resolved, authorized, read and answered with: %31, ./1 and 1 are one URI
    CORE::CString uriProblem;
    if ( !CMcpResourceRegistry::NormalizeUri( (*uri).second.AsString(), dispatch.target, uriProblem ) )
    {
        dispatch.target = (*uri).second.AsString();
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: the uri is refused: " + uriProblem );
        return;
    }

    if ( IsInFlightLimitReached( dispatch.channelId ) )
    {
        SendError( dispatch, MCP_ERROR_GUCEF_BUSY, "Busy: too many requests in progress" );
        return;
    }

    CIMcpAccessPolicyPtr holder;
    const CIMcpAccessPolicy& policy = GetPolicy( holder );
    CMcpOperationTaskData call;
    CMcpResourceRegistry::EResolveResult resolved = m_resourceRegistry.Resolve( dispatch.target, dispatch.resource, dispatch.resourceTemplate, call.variables );
    bool found = CMcpResourceRegistry::RESOLVE_NONE != resolved && policy.IsResourceVisible( dispatch.principal, dispatch.target );
    if ( !found )
        NoteNotVisible( dispatch, CMcpResourceRegistry::RESOLVE_NONE != resolved );

    CORE::CDataNode notFoundData;
    SetUriData( notFoundData, dispatch.target );
    const Int32 notFoundCode = MCP_ERA_LEGACY == dispatch.era ? (Int32) MCP_ERROR_LEGACY_RESOURCE_NOT_FOUND : (Int32) MCP_ERROR_INVALID_PARAMS;
    if ( !found )
    {
        SendError( dispatch, notFoundCode, "Resource not found", &notFoundData );
        return;
    }

    call.operation = CMcpResourceRegistry::RESOLVE_RESOURCE == resolved ? CMcpOperationTaskData::OPERATION_RESOURCE_READ : CMcpOperationTaskData::OPERATION_TEMPLATE_READ;
    call.resourceEntry = dispatch.resource;
    call.templateEntry = dispatch.resourceTemplate;
    call.uri = dispatch.target;
    if ( !PrepareCall( dispatch, params, GUCEF_NULL, call ) )
        return;

    CORE::CString reason;
    EMcpAccessDecision decision = policy.AuthorizeResourceRead( *dispatch.context, dispatch.target, reason );
    NoteDecision( dispatch, decision, reason );
    if ( MCP_ACCESS_HIDE == decision )
    {
        SendError( dispatch, notFoundCode, "Resource not found", &notFoundData );
        return;
    }
    if ( MCP_ACCESS_DENY == decision )
    {
        SendError( dispatch, MCP_ERROR_GUCEF_ACCESS_DENIED, "Access denied" + ( reason.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + reason ) );
        return;
    }

    EMcpExecutionMode mode = CMcpResourceRegistry::RESOLVE_RESOURCE == resolved ? dispatch.resource->resource->GetExecutionMode() : dispatch.resourceTemplate->resourceTemplate->GetExecutionMode();
    RunCall( dispatch, call, mode, 0 );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::HandlePromptGet( SDispatch& dispatch, const CORE::CDataNode* params )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* name = GUCEF_NULL != params ? params->GetAttribute( "name" ) : GUCEF_NULL;
    if ( GUCEF_NULL == name || !(*name).second.IsString() )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: name must be a string" );
        return;
    }
    dispatch.target = (*name).second.AsString();
    const CORE::CDataNode* arguments = GUCEF_NULL;
    if ( !GetOptionalObjectMember( *params, "arguments", arguments ) || ( GUCEF_NULL != arguments && arguments->HasChildren() ) )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: arguments must be an object of strings" );
        return;
    }
    CaptureAuditArguments( dispatch, arguments, GUCEF_NULL );

    CMcpOperationTaskData call;
    if ( GUCEF_NULL != arguments )
    {
        for ( CORE::CDataNode::TAttributeMap::const_iterator i = arguments->AttributeBegin(); i != arguments->AttributeEnd(); ++i )
        {
            if ( !(*i).second.IsString() )
            {
                SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: the prompt argument \"" + (*i).first + "\" must be a string" );
                return;
            }
            call.promptArguments.Set( (*i).first, (*i).second );
        }
    }

    if ( IsInFlightLimitReached( dispatch.channelId ) )
    {
        SendError( dispatch, MCP_ERROR_GUCEF_BUSY, "Busy: too many requests in progress" );
        return;
    }

    CIMcpAccessPolicyPtr holder;
    const CIMcpAccessPolicy& policy = GetPolicy( holder );
    const bool promptFound = m_promptRegistry.TryGet( dispatch.target, dispatch.prompt );
    if ( !promptFound || !policy.IsPromptVisible( dispatch.principal, *dispatch.prompt ) )
    {
        NoteNotVisible( dispatch, promptFound );
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Unknown prompt: " + dispatch.target );
        return;
    }
    const CMcpPromptDescriptor::TArgumentVector& declared = dispatch.prompt->descriptor.arguments;
    for ( size_t i=0; i<declared.size(); ++i )
    {
        CORE::CVariant value;
        if ( declared[ i ].required && !call.promptArguments.TryGetValue( declared[ i ].name, value ) )
        {
            SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: the required argument \"" + declared[ i ].name + "\" of the prompt is missing" );
            return;
        }
    }

    call.operation = CMcpOperationTaskData::OPERATION_PROMPT_GET;
    call.promptEntry = dispatch.prompt;
    if ( GUCEF_NULL != arguments )
    {
        call.arguments = *arguments;
        call.arguments.SetName( CORE::CString::Empty );
    }
    if ( !PrepareCall( dispatch, params, &call.arguments, call ) )
        return;

    CORE::CString reason;
    EMcpAccessDecision decision = policy.AuthorizePromptGet( *dispatch.context, *dispatch.prompt, reason );
    NoteDecision( dispatch, decision, reason );
    if ( MCP_ACCESS_HIDE == decision )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Unknown prompt: " + dispatch.target );
        return;
    }
    if ( MCP_ACCESS_DENY == decision )
    {
        SendError( dispatch, MCP_ERROR_GUCEF_ACCESS_DENIED, "Access denied" + ( reason.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + reason ) );
        return;
    }

    RunCall( dispatch, call, dispatch.prompt->prompt->GetExecutionMode(), 0 );
}

/*-------------------------------------------------------------------------*/

CORE::ThreadPoolPtr
CMcpServer::GetThreadPool( void )
{GUCEF_TRACE;

    if ( !m_threadPool.IsNULL() )
        return m_threadPool;
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    CORE::ThreadPoolPtr pool;
    if ( !m_settings.threadPoolName.IsNULLOrEmpty() )
        pool = taskManager.GetOrCreateThreadPool( m_settings.threadPoolName, true );
    if ( pool.IsNULL() )
        pool = taskManager.GetThreadPool();
    return pool;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::RunCall( SDispatch& dispatch, CMcpOperationTaskData& call, EMcpExecutionMode mode, UInt32 maxDurationMs )
{GUCEF_TRACE;

    if ( MCP_EXECUTION_THREAD_POOL != mode )
    {
        // on the protocol thread, inside the dispatch
        dispatch.context->SetPulseGenerator( m_pulseGenerator );
        call.RunHandler();
        if ( dispatch.context->IsCompleted() )
        {
            FinishCall( dispatch );
            return;
        }
        // deferred: the answer comes through the mailbox
        RegisterInFlight( dispatch, maxDurationMs );
        return;
    }

    CORE::ThreadPoolPtr pool = GetThreadPool();
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
    if ( !pool.IsNULL() )
    {
        RegisterInFlight( dispatch, maxDurationMs );
        TInFlightMap& inFlightMap = GetChannelState( dispatch.channelId )->inFlight;
        TInFlightMap::iterator i = inFlightMap.find( dispatch.idKey );
        SInFlight* inFlight = i != inFlightMap.end() ? (*i).second : GUCEF_NULL;
        if ( GUCEF_NULL == inFlight )
            return;     // it was answered right away
        future = pool->QueueTask( CMcpOperationTaskConsumer::TaskType, &call );
        if ( future.HasAFuture() )
        {
            inFlight->future = future;
            inFlight->hasFuture = true;
            return;
        }
        SDispatch failed = inFlight->dispatch;
        EndInFlight( inFlight );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: A " << failed.method << " request could not be queued on the thread pool" );
        SendError( failed, MCP_ERROR_INTERNAL, "Internal error: the request could not be queued" );
        return;
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: There is no thread pool for a " << dispatch.method << " request" );
    SendError( dispatch, MCP_ERROR_INTERNAL, "Internal error: the request could not be queued" );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::RegisterInFlight( SDispatch& dispatch, UInt32 maxDurationMs )
{GUCEF_TRACE;

    SChannelState* state = GetChannelState( dispatch.channelId );
    if ( GUCEF_NULL == state )
        return;

    SInFlight* inFlight = GUCEF_NEW SInFlight();
    inFlight->dispatch = dispatch;
    const UInt32 duration = maxDurationMs > 0 ? maxDurationMs : m_settings.maxRequestDurationMs;
    inFlight->deadlineInMs = dispatch.startInMs + duration;
    state->inFlight[ dispatch.idKey ] = inFlight;
    m_inFlightByContext[ dispatch.context.GetPointerAlways() ] = inFlight;
    ++m_inFlightTotal;

    // attach, then look: a completion that came before the attach was told to no one
    dispatch.context->SetListener( this );
    if ( dispatch.context->IsCompleted() )
    {
        FinishCall( inFlight->dispatch );
        dispatch.finished = true;
        EndInFlight( inFlight );
        return;
    }
    UpdatePeriodicPulses();
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::EndInFlight( SInFlight* inFlight )
{GUCEF_TRACE;

    if ( GUCEF_NULL == inFlight )
        return;
    CMcpRequestContext* context = inFlight->dispatch.context.GetPointerAlways();
    context->SetListener( GUCEF_NULL );
    context->MarkEnded();
    m_inFlightByContext.erase( context );
    m_heldProgress.erase( inFlight );
    TChannelStateMap::iterator found = m_channels.find( inFlight->dispatch.channelId );
    if ( found != m_channels.end() )
    {
        SChannelState* state = (*found).second;
        state->inFlight.erase( inFlight->dispatch.idKey );
        if ( state->closed && state->inFlight.empty() )
        {
            GUCEF_DELETE state;
            m_channels.erase( found );
        }
    }
    if ( m_inFlightTotal > 0 )
        --m_inFlightTotal;
    GUCEF_DELETE inFlight;
    UpdatePeriodicPulses();
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::CancelInFlight( SInFlight* inFlight, EMcpRequestOutcome outcome )
{GUCEF_TRACE;

    // cooperative: the handler polls the flag; nothing more is sent for this request
    inFlight->dispatch.context->RequestCancellation();
    if ( inFlight->hasFuture )
        inFlight->future.RequestCancellation();
    if ( !inFlight->dispatch.finished )
    {
        inFlight->dispatch.finished = true;
        RaiseRequestFinished( inFlight->dispatch, outcome, 0 );
    }
    EndInFlight( inFlight );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::FinishCall( SDispatch& dispatch )
{GUCEF_TRACE;

    EMcpRequestOutcome outcome = MCP_OUTCOME_COMPLETE;
    Int32 errorCode = 0;
    SendCallOutcome( dispatch, outcome, errorCode );
    dispatch.context->MarkEnded();
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::BuildToolResultNode( SDispatch& dispatch, const CMcpToolResult& result, CORE::CDataNode& node, CORE::CString& error )
{GUCEF_TRACE;

    // 13. structured output: validated before it is sent, never sent unchecked
    const CMcpToolEntry& tool = *dispatch.tool;
    if ( !result.isError && m_settings.validateStructuredOutput && tool.descriptor.validateOutput && tool.hasOutputSchema )
    {
        if ( !result.hasStructuredContent )
        {
            error = "the tool \"" + dispatch.target + "\" has an output schema but gave no structuredContent";
            return false;
        }
        CORE::CJsonSchemaValidator::TErrorVector errors;
        if ( !CORE::CJsonSchemaValidator::Validate( tool.outputSchema, result.structuredContent, errors, MaxArgumentErrors ) )
        {
            error = "the structuredContent of the tool \"" + dispatch.target + "\" does not match its output schema: " + FormatValidationErrors( errors );
            return false;
        }
    }
    node = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
    if ( !result.ToDataNode( node ) )
    {
        error = "the result of the tool \"" + dispatch.target + "\" could not be written";
        return false;
    }
    AddResultMeta( dispatch, node );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::BuildInputRequiredNode( SDispatch& dispatch, const CMcpInputRequired& inputRequired, CORE::CDataNode& node,
                                    Int32& errorCode, CORE::CString& errorMessage, CORE::CDataNode& errorData )
{GUCEF_TRACE;

    errorCode = MCP_ERROR_INTERNAL;
    errorMessage = "Internal error";
    if ( !inputRequired.HasInputRequests() && !inputRequired.HasState() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: The handler of " << dispatch.method << " \"" << dispatch.target << "\" answered input_required without input requests and without state" );
        return false;
    }

    // never an input request the client did not declare it can fulfil
    CORE::CDataNode required;
    inputRequired.GetRequiredCapabilities( required );
    if ( !CMcpRequestMeta::HasAllCapabilities( dispatch.meta.GetClientCapabilities(), required ) )
    {
        errorCode = MCP_ERROR_MISSING_REQUIRED_CLIENT_CAPABILITY;
        errorMessage = "Missing required client capability";
        errorData = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* capabilities = errorData.AddChild( required );
        capabilities->SetName( "requiredCapabilities" );
        return false;
    }

    node = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "resultType", CORE::CVariant( CMcpResultType::InputRequired ) );
    if ( inputRequired.HasInputRequests() )
    {
        CORE::CDataNode* requests = node.AddChild( "inputRequests", GUCEF_DATATYPE_OBJECT );
        const CMcpInputRequired::TInputRequestMap& map = inputRequired.GetInputRequests();
        for ( CMcpInputRequired::TInputRequestMap::const_iterator i = map.begin(); i != map.end(); ++i )
        {
            CORE::CDataNode* request = requests->AddChild( (*i).first, GUCEF_DATATYPE_OBJECT );
            request->SetAttribute( "method", CORE::CVariant( (*i).second.method ) );
            if ( (*i).second.hasParams )
                CJsonRpcEndpoint::AddJsonMember( *request, "params", (*i).second.params );
        }
    }
    if ( inputRequired.HasState() )
    {
        CMcpRequestStateSealer::SBinding binding;
        binding.subject = dispatch.principal.subject;
        binding.method = dispatch.method;
        binding.target = dispatch.target;
        ::memcpy( binding.argumentsDigest, dispatch.argumentsDigest, sizeof( binding.argumentsDigest ) );
        CORE::CString sealed;
        CORE::CString error;
        if ( !m_sealer.Seal( binding, inputRequired.GetState(), inputRequired.GetWantsStateNonce(), NowUnixMs() + m_settings.requestStateTtlMs, sealed, error ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: The state of " << dispatch.method << " \"" << dispatch.target << "\" could not be sealed: " << error );
            return false;
        }
        node.SetAttribute( "requestState", CORE::CVariant( sealed ) );
    }
    AddResultMeta( dispatch, node );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::GetResourceCacheHints( SDispatch& dispatch, const CMcpResourceContents& contents, UInt32& ttlMs, EMcpCacheScope& scope ) const
{GUCEF_TRACE;

    ttlMs = 0;
    scope = MCP_CACHESCOPE_PRIVATE;
    if ( contents.hasCacheHints )
    {
        ttlMs = contents.ttlMs;
        scope = contents.cacheScope;
    }
    else
    if ( !dispatch.resource.IsNULL() )
        dispatch.resource->resource->GetCacheHints( ttlMs, scope );
    else
    if ( !dispatch.resourceTemplate.IsNULL() )
        dispatch.resourceTemplate->resourceTemplate->GetCacheHints( dispatch.target, ttlMs, scope );

    CIMcpAccessPolicyPtr holder;
    if ( GetPolicy( holder ).IsVisibilityPrincipalDependent() )
        scope = MCP_CACHESCOPE_PRIVATE;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::SendCallOutcome( SDispatch& dispatch, EMcpRequestOutcome& outcome, Int32& errorCode )
{GUCEF_TRACE;

    const CMcpRequestContext& context = *dispatch.context;
    switch ( context.GetCompletionKind() )
    {
        case CMcpRequestContext::COMPLETION_TOOL_RESULT:
        {
            CORE::CDataNode node;
            CORE::CString error;
            if ( !BuildToolResultNode( dispatch, context.GetToolResult(), node, error ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: " << error );
                SendError( dispatch, MCP_ERROR_INTERNAL, "Internal error" );
                outcome = MCP_OUTCOME_PROTOCOL_ERROR;
                errorCode = MCP_ERROR_INTERNAL;
                return;
            }
            outcome = context.GetToolResult().isError ? MCP_OUTCOME_TOOL_ERROR : MCP_OUTCOME_COMPLETE;
            SendResult( dispatch, node, outcome );
            return;
        }
        case CMcpRequestContext::COMPLETION_RESOURCE_CONTENTS:
        {
            const CMcpResourceContents& contents = context.GetResourceContents();
            CORE::CDataNode node( GUCEF_DATATYPE_OBJECT );
            contents.ToDataNode( node );
            UInt32 ttlMs = 0;
            EMcpCacheScope scope = MCP_CACHESCOPE_PRIVATE;
            GetResourceCacheHints( dispatch, contents, ttlMs, scope );
            AddCacheHints( dispatch, node, ttlMs, scope );
            AddResultMeta( dispatch, node );
            outcome = MCP_OUTCOME_COMPLETE;
            SendResult( dispatch, node, outcome );
            return;
        }
        case CMcpRequestContext::COMPLETION_PROMPT_RESULT:
        {
            CORE::CDataNode node( GUCEF_DATATYPE_OBJECT );
            context.GetPromptResult().ToDataNode( node );
            AddResultMeta( dispatch, node );
            outcome = MCP_OUTCOME_COMPLETE;
            SendResult( dispatch, node, outcome );
            return;
        }
        case CMcpRequestContext::COMPLETION_INPUT_REQUIRED:
        {
            if ( MCP_ERA_LEGACY == dispatch.era )
            {
                // 2025-11-25 has no multi round-trip requests
                if ( dispatch.method == CMcpMethods::ToolsCall )
                {
                    CMcpToolResult failed;
                    failed.SetError( LegacyInputRequiredText );
                    CORE::CDataNode node( GUCEF_DATATYPE_OBJECT );
                    failed.ToDataNode( node );
                    outcome = MCP_OUTCOME_TOOL_ERROR;
                    SendResult( dispatch, node, outcome );
                }
                else
                {
                    outcome = MCP_OUTCOME_PROTOCOL_ERROR;
                    errorCode = MCP_ERROR_INTERNAL;
                    SendError( dispatch, MCP_ERROR_INTERNAL, LegacyInputRequiredText );
                }
                return;
            }
            CORE::CDataNode node;
            CORE::CDataNode errorData;
            CORE::CString errorMessage;
            if ( !BuildInputRequiredNode( dispatch, context.GetInputRequired(), node, errorCode, errorMessage, errorData ) )
            {
                outcome = MCP_OUTCOME_PROTOCOL_ERROR;
                SendError( dispatch, errorCode, errorMessage, errorData.HasChildren() || errorData.GetAttCount() > 0 ? &errorData : GUCEF_NULL );
                return;
            }
            outcome = MCP_OUTCOME_INPUT_REQUIRED;
            SendResult( dispatch, node, outcome );
            return;
        }
        case CMcpRequestContext::COMPLETION_PROTOCOL_ERROR:
        default:
        {
            errorCode = context.GetProtocolErrorCode();
            outcome = MCP_OUTCOME_PROTOCOL_ERROR;
            if ( MCP_ERA_LEGACY == dispatch.era && MCP_ERROR_INVALID_PARAMS == errorCode && dispatch.method == CMcpMethods::ResourcesRead )
            {
                // the "does not exist" of a resource handler is -32002 in 2025-11-25
                CORE::CDataNode data;
                SetUriData( data, dispatch.target );
                errorCode = MCP_ERROR_LEGACY_RESOURCE_NOT_FOUND;
                SendError( dispatch, errorCode, "Resource not found", &data );
                return;
            }
            SendError( dispatch, errorCode, context.GetProtocolErrorMessage(), context.HasProtocolErrorData() ? &context.GetProtocolErrorData() : GUCEF_NULL );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::HandleListen( SDispatch& dispatch, const CORE::CDataNode* params )
{GUCEF_TRACE;

    const CORE::CDataNode* filter = GUCEF_NULL;
    if ( GUCEF_NULL == params || !GetOptionalObjectMember( *params, "notifications", filter ) || GUCEF_NULL == filter )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: notifications must be an object" );
        return;
    }
    bool toolsListChanged = false;
    bool promptsListChanged = false;
    bool resourcesListChanged = false;
    if ( !GetOptionalBoolMember( *filter, "toolsListChanged", toolsListChanged ) ||
         !GetOptionalBoolMember( *filter, "promptsListChanged", promptsListChanged ) ||
         !GetOptionalBoolMember( *filter, "resourcesListChanged", resourcesListChanged ) )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: the list change members of notifications must be booleans" );
        return;
    }
    const CORE::CDataNode* uris = filter->FindChild( "resourceSubscriptions" );
    if ( GUCEF_NULL != filter->GetAttribute( "resourceSubscriptions" ) || ( GUCEF_NULL != uris && GUCEF_DATATYPE_ARRAY != uris->GetNodeType() ) )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: resourceSubscriptions must be an array of strings" );
        return;
    }
    if ( GUCEF_NULL != uris )
    {
        for ( CORE::CDataNode::const_iterator i = uris->ConstBegin(); i != uris->ConstEnd(); ++i )
        {
            if ( (*i)->HasChildren() || (*i)->GetAttCount() > 0 || !(*i)->GetValue().IsString() )
            {
                SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: resourceSubscriptions must be an array of strings" );
                return;
            }
        }
    }

    SChannelState* state = GetChannelState( dispatch.channelId );
    if ( GUCEF_NULL == state )
        return;
    if ( state->subscriptions.size() >= m_settings.maxSubscriptionsPerChannel || m_subscriptionsTotal >= m_settings.maxSubscriptionsTotal )
    {
        SendError( dispatch, MCP_ERROR_GUCEF_BUSY, "Busy: too many subscriptions" );
        return;
    }

    dispatch.context = CMcpRequestContext::Create( dispatch.method, dispatch.target, dispatch.id, dispatch.era,
                                                   dispatch.principal, dispatch.meta, dispatch.channelId );
    CIMcpAccessPolicyPtr holder;
    const CIMcpAccessPolicy& policy = GetPolicy( holder );
    CORE::CString reason;
    const EMcpAccessDecision subscriptionDecision = policy.AuthorizeSubscription( *dispatch.context, reason );
    NoteDecision( dispatch, subscriptionDecision, reason );
    if ( MCP_ACCESS_ALLOW != subscriptionDecision )
    {
        SendError( dispatch, MCP_ERROR_GUCEF_ACCESS_DENIED, "Access denied" + ( reason.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + reason ) );
        return;
    }

    // what is honoured: the list changes of enabled registries and the URIs the principal may read, up to the limit
    CMcpSubscription subscription;
    subscription.id = dispatch.id;
    subscription.idKey = dispatch.idKey;
    subscription.channelId = dispatch.channelId;
    subscription.principal = dispatch.principal;
    subscription.toolsListChanged = toolsListChanged && m_settings.enableTools;
    subscription.promptsListChanged = promptsListChanged && m_settings.enablePrompts;
    subscription.resourcesListChanged = resourcesListChanged && m_settings.enableResources;
    if ( GUCEF_NULL != uris && m_settings.enableResources )
    {
        subscription.hadResourceSubscriptions = true;
        for ( CORE::CDataNode::const_iterator i = uris->ConstBegin(); i != uris->ConstEnd(); ++i )
        {
            if ( subscription.resourceSubscriptions.size() >= m_settings.maxResourceSubscriptionsPerListen )
                break;
            CORE::CString uri;
            CORE::CString uriProblem;
            if ( !CMcpResourceRegistry::NormalizeUri( (*i)->GetValue().AsString(), uri, uriProblem ) )
                continue;
            CMcpResourceEntryPtr resource;
            CMcpResourceTemplateEntryPtr resourceTemplate;
            CORE::CValueList variables;
            if ( CMcpResourceRegistry::RESOLVE_NONE != m_resourceRegistry.Resolve( uri, resource, resourceTemplate, variables ) &&
                 policy.IsResourceVisible( dispatch.principal, uri ) )
                subscription.resourceSubscriptions.insert( uri );
        }
    }
    TSubscriptionMap::iterator stored = state->subscriptions.insert( TSubscriptionMap::value_type( dispatch.idKey, subscription ) ).first;
    ++m_subscriptionsTotal;

    // the acknowledgment comes before any notification of the subscription
    CORE::CDataNode ack( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* meta = ack.AddChild( "_meta", GUCEF_DATATYPE_OBJECT );
    meta->SetAttribute( CMcpMetaKeys::SubscriptionId, dispatch.id );
    CORE::CDataNode honoured;
    (*stored).second.FilterToDataNode( honoured );
    CJsonRpcEndpoint::AddJsonMember( ack, "notifications", honoured );
    dispatch.channel->SendNotification( CMcpMethods::NotificationSubscriptionsAcknowledged, &ack );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::EndSubscription( SChannelState& state, CMcpSubscription& subscription, bool graceful )
{GUCEF_TRACE;

    if ( graceful && !state.channel.IsNULL() && !state.channel->IsClosed() )
    {
        // the graceful closure: the response to the listen request, carrying its id
        CORE::CDataNode result( GUCEF_DATATYPE_OBJECT );
        result.SetAttribute( "resultType", CORE::CVariant( CMcpResultType::Complete ) );
        CORE::CDataNode* meta = result.AddChild( "_meta", GUCEF_DATATYPE_OBJECT );
        meta->SetAttribute( CMcpMetaKeys::SubscriptionId, subscription.id );
        if ( m_settings.includeServerInfo )
        {
            CORE::CDataNode* serverInfo = meta->AddChild( m_serverInfoNode );
            serverInfo->SetName( CMcpMetaKeys::ServerInfo );
        }
        state.channel->SendResult( subscription.id, result );

        // and on a stream, where the subscription is no stream of its own, the notification the cancellation page asks for
        if ( CMcpChannel::CHANNEL_STREAM == state.channel->GetKind() )
        {
            CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
            params.SetAttribute( "requestId", subscription.id );
            params.SetAttribute( "reason", CORE::CVariant( "the server ended the subscription" ) );
            state.channel->SendNotification( CMcpMethods::NotificationCancelled, &params );
        }
    }

    SDispatch dispatch;
    dispatch.channel = state.channel;
    dispatch.channelId = subscription.channelId;
    dispatch.id = subscription.id;
    dispatch.idKey = subscription.idKey;
    dispatch.method = CMcpMethods::SubscriptionsListen;
    dispatch.principal = subscription.principal;
    dispatch.meta.SetForInProcess();
    RaiseRequestFinished( dispatch, graceful ? MCP_OUTCOME_COMPLETE : MCP_OUTCOME_CANCELLED, 0 );
    if ( m_subscriptionsTotal > 0 )
        --m_subscriptionsTotal;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::SendListChanged( const CORE::CString& method, int which )
{GUCEF_TRACE;

    for ( TChannelStateMap::iterator i = m_channels.begin(); i != m_channels.end(); ++i )
    {
        SChannelState& state = *(*i).second;
        if ( state.closed || state.channel.IsNULL() )
            continue;
        for ( TSubscriptionMap::iterator n = state.subscriptions.begin(); n != state.subscriptions.end(); ++n )
        {
            const CMcpSubscription& subscription = (*n).second;
            bool wanted = ( 0 == which && subscription.toolsListChanged ) ||
                          ( 1 == which && subscription.resourcesListChanged ) ||
                          ( 2 == which && subscription.promptsListChanged );
            if ( !wanted )
                continue;
            CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
            params.AddChild( "_meta", GUCEF_DATATYPE_OBJECT )->SetAttribute( CMcpMetaKeys::SubscriptionId, subscription.id );
            state.channel->SendNotification( method, &params );
        }

        // a legacy session on a stream gets the notification unasked once it is operational
        const CMcpLegacySession* session = state.channel->GetLegacySession();
        if ( m_settings.supportLegacy && GUCEF_NULL != session && session->isOperational && CMcpChannel::CHANNEL_STREAM == state.channel->GetKind() )
            state.channel->SendNotification( method, GUCEF_NULL );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::OnToolsListChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_settings.enableTools )
        SendListChanged( CMcpMethods::NotificationToolsListChanged, 0 );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::OnResourcesListChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_settings.enableResources )
        SendListChanged( CMcpMethods::NotificationResourcesListChanged, 1 );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::OnPromptsListChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_settings.enablePrompts )
        SendListChanged( CMcpMethods::NotificationPromptsListChanged, 2 );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::NotifyResourceUpdated( const CORE::CString& uri )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_updatesLock );
    m_pendingResourceUpdates.insert( uri );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::ProcessResourceUpdates( void )
{GUCEF_TRACE;

    CORE::CString::StringSet updates;
    {
        MT::CScopeMutex lock( m_updatesLock );
        if ( m_pendingResourceUpdates.empty() )
            return;
        updates.swap( m_pendingResourceUpdates );
    }

    for ( CORE::CString::StringSet::const_iterator u = updates.begin(); u != updates.end(); ++u )
    {
        const CORE::CString& uri = (*u);
        for ( TChannelStateMap::iterator i = m_channels.begin(); i != m_channels.end(); ++i )
        {
            SChannelState& state = *(*i).second;
            if ( state.closed || state.channel.IsNULL() )
                continue;
            for ( TSubscriptionMap::iterator n = state.subscriptions.begin(); n != state.subscriptions.end(); ++n )
            {
                if ( !(*n).second.IsSubscribedToResource( uri ) )
                    continue;
                CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
                params.AddChild( "_meta", GUCEF_DATATYPE_OBJECT )->SetAttribute( CMcpMetaKeys::SubscriptionId, (*n).second.id );
                params.SetAttribute( "uri", CORE::CVariant( uri ) );
                state.channel->SendNotification( CMcpMethods::NotificationResourcesUpdated, &params );
            }
            const CMcpLegacySession* session = state.channel->GetLegacySession();
            if ( m_settings.supportLegacy && GUCEF_NULL != session && session->resourceSubscriptions.end() != session->resourceSubscriptions.find( uri ) )
            {
                CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
                params.SetAttribute( "uri", CORE::CVariant( uri ) );
                state.channel->SendNotification( CMcpMethods::NotificationResourcesUpdated, &params );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::HandleInitialize( SDispatch& dispatch, const CORE::CDataNode* params )
{GUCEF_TRACE;

    CORE::CString requested;
    bool hasVersion = false;
    bool versionOk = GUCEF_NULL != params && GetOptionalStringMember( *params, "protocolVersion", requested, hasVersion ) && hasVersion;

    if ( !m_settings.supportLegacy )
    {
        // a modern only server names the versions it supports, the only diagnostic a legacy client can show
        CORE::CDataNode data( GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode* supported = data.AddChild( "supported", GUCEF_DATATYPE_ARRAY );
        CORE::CString::StringVector versions;
        CMcpProtocolVersions::GetModernVersions( versions );
        for ( size_t i=0; i<versions.size(); ++i )
            supported->AddValueAsChild( CORE::CVariant( versions[ i ] ) );
        data.SetAttribute( "requested", CORE::CVariant( requested ) );
        SendError( dispatch, MCP_ERROR_UNSUPPORTED_PROTOCOL_VERSION, "Unsupported protocol version: this server serves " + CMcpProtocolVersions::Version_2026_07_28 + " only", &data );
        return;
    }
    if ( !versionOk )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: protocolVersion must be a string" );
        return;
    }
    const CORE::CDataNode* capabilities = GUCEF_NULL;
    const CORE::CDataNode* clientInfo = GUCEF_NULL;
    if ( !GetOptionalObjectMember( *params, "capabilities", capabilities ) || !GetOptionalObjectMember( *params, "clientInfo", clientInfo ) )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: capabilities and clientInfo must be objects" );
        return;
    }

    // the requested version when it is served, else the newest legacy version, as the 2025-11-25 lifecycle prescribes
    const CORE::CString negotiated = CMcpProtocolVersions::IsLegacyVersion( requested ) ? requested : CMcpProtocolVersions::Version_2025_11_25;
    const bool isStream = CMcpChannel::CHANNEL_STREAM == dispatch.channel->GetKind();
    if ( isStream )
    {
        CMcpLegacySession& session = dispatch.channel->OpenLegacySession();
        session.protocolVersion = negotiated;
        if ( GUCEF_NULL != capabilities )
        {
            session.clientCapabilities = *capabilities;
            session.clientCapabilities.SetName( CORE::CString::Empty );
        }
        session.hasClientInfo = GUCEF_NULL != clientInfo && session.clientInfo.FromDataNode( *clientInfo );
    }

    CORE::CDataNode result( GUCEF_DATATYPE_OBJECT );
    result.SetAttribute( "protocolVersion", CORE::CVariant( negotiated ) );
    CORE::CDataNode* serverCapabilities = result.AddChild( "capabilities", GUCEF_DATATYPE_OBJECT );
    if ( m_settings.enableTools )
        serverCapabilities->AddChild( "tools", GUCEF_DATATYPE_OBJECT )->SetAttribute( "listChanged", isStream );
    if ( m_settings.enableResources )
    {
        CORE::CDataNode* resources = serverCapabilities->AddChild( "resources", GUCEF_DATATYPE_OBJECT );
        resources->SetAttribute( "subscribe", isStream );
        resources->SetAttribute( "listChanged", isStream );
    }
    if ( m_settings.enablePrompts )
        serverCapabilities->AddChild( "prompts", GUCEF_DATATYPE_OBJECT )->SetAttribute( "listChanged", isStream );
    {
        MT::CScopeMutex lock( m_configLock );
        CORE::CDataNode* serverInfo = result.AddChild( m_serverInfoNode );
        serverInfo->SetName( "serverInfo" );
        if ( !m_settings.instructions.IsNULLOrEmpty() )
            result.SetAttribute( "instructions", CORE::CVariant( m_settings.instructions ) );
    }
    dispatch.meta.SetForLegacy( negotiated, GUCEF_NULL != capabilities ? *capabilities : CORE::CDataNode( GUCEF_DATATYPE_OBJECT ), GUCEF_NULL );
    SendResult( dispatch, result, MCP_OUTCOME_COMPLETE );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::HandleLegacySubscribe( SDispatch& dispatch, const CORE::CDataNode* params, bool subscribe )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* uri = GUCEF_NULL != params ? params->GetAttribute( "uri" ) : GUCEF_NULL;
    if ( GUCEF_NULL == uri || !(*uri).second.IsString() )
    {
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: uri must be a string" );
        return;
    }
    CORE::CString uriProblem;
    if ( !CMcpResourceRegistry::NormalizeUri( (*uri).second.AsString(), dispatch.target, uriProblem ) )
    {
        dispatch.target = (*uri).second.AsString();
        SendError( dispatch, MCP_ERROR_INVALID_PARAMS, "Invalid params: the uri is refused: " + uriProblem );
        return;
    }
    CMcpLegacySession* session = dispatch.channel->GetLegacySession();
    if ( subscribe )
    {
        CIMcpAccessPolicyPtr holder;
        CMcpResourceEntryPtr resource;
        CMcpResourceTemplateEntryPtr resourceTemplate;
        CORE::CValueList variables;
        if ( CMcpResourceRegistry::RESOLVE_NONE == m_resourceRegistry.Resolve( dispatch.target, resource, resourceTemplate, variables ) ||
             !GetPolicy( holder ).IsResourceVisible( dispatch.principal, dispatch.target ) )
        {
            CORE::CDataNode data;
            SetUriData( data, dispatch.target );
            SendError( dispatch, MCP_ERROR_LEGACY_RESOURCE_NOT_FOUND, "Resource not found", &data );
            return;
        }
        session->resourceSubscriptions.insert( dispatch.target );
    }
    else
    {
        session->resourceSubscriptions.erase( dispatch.target );
    }
    CORE::CDataNode result( GUCEF_DATATYPE_OBJECT );
    SendResult( dispatch, result, MCP_OUTCOME_COMPLETE );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::OnChannelNotification( CMcpChannel& channel, const SJsonRpcNotification& notification )
{GUCEF_TRACE;

    SChannelState* state = GetChannelState( channel.GetChannelId() );
    if ( GUCEF_NULL == state || state->closed )
        return;

    if ( notification.method == CMcpMethods::NotificationInitialized )
    {
        CMcpLegacySession* session = channel.GetLegacySession();
        if ( GUCEF_NULL != session )
            session->isOperational = true;
        return;
    }
    if ( notification.method != CMcpMethods::NotificationCancelled )
        return;     // nothing else is expected from a client; unknown notifications are ignored

    // unknown, finished or malformed references are ignored
    if ( !notification.hasParams || GUCEF_DATATYPE_OBJECT != notification.params.GetNodeType() )
        return;
    const CORE::CDataNode::TKeyValuePair* requestId = notification.params.GetAttribute( "requestId" );
    if ( GUCEF_NULL == requestId || !IsValidRequestId( (*requestId).second ) )
        return;
    const CORE::CString key = IdKey( (*requestId).second );

    if ( CMcpChannel::CHANNEL_STREAM == channel.GetKind() )
    {
        TInFlightMap::iterator i = state->inFlight.find( key );
        if ( i != state->inFlight.end() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: The client cancelled request " << (*requestId).second.AsString() << " on channel " << channel.GetChannelId() );
            CancelInFlight( (*i).second, MCP_OUTCOME_CANCELLED );
            return;
        }
        TSubscriptionMap::iterator s = state->subscriptions.find( key );
        if ( s != state->subscriptions.end() )
        {
            EndSubscription( *state, (*s).second, false );
            state->subscriptions.erase( s );
        }
        return;
    }

    // HTTP: a modern client cancels by closing its stream; a legacy cancellation counts when exactly one legacy request
    // of the same principal has the id
    SInFlight* match = GUCEF_NULL;
    UInt32 matches = 0;
    for ( TChannelStateMap::iterator c = m_channels.begin(); c != m_channels.end(); ++c )
    {
        SChannelState& other = *(*c).second;
        if ( other.channel.IsNULL() || CMcpChannel::CHANNEL_EXCHANGE != other.channel->GetKind() )
            continue;
        TInFlightMap::iterator i = other.inFlight.find( key );
        if ( i != other.inFlight.end() && MCP_ERA_LEGACY == (*i).second->dispatch.era && (*i).second->dispatch.principal.subject == channel.GetPrincipal().subject )
        {
            match = (*i).second;
            ++matches;
        }
    }
    if ( 1 == matches )
    {
        // no response comes for a cancelled request, but its POST still has to end: the channel of the request is told
        CMcpChannelPtr requestChannel = match->dispatch.channel;
        const CORE::CVariant cancelledId = match->dispatch.id;
        CancelInFlight( match, MCP_OUTCOME_CANCELLED );
        if ( !requestChannel.IsNULL() )
            requestChannel->OnRequestCancelled( cancelledId );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::OnRequestContextNews( CMcpRequestContext& context )
{GUCEF_TRACE;

    // called with the lock of the context held, from any thread: only queue
    MT::CScopeMutex lock( m_mailboxLock );
    m_mailbox.push_back( &context );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::SendProgress( SInFlight& inFlight, const CMcpRequestContext::SProgress& progress, UInt64 now )
{GUCEF_TRACE;

    SDispatch& dispatch = inFlight.dispatch;
    if ( dispatch.channel.IsNULL() || dispatch.channel->IsClosed() || dispatch.context->IsCancellationRequested() )
        return;
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "progressToken", dispatch.meta.GetProgressToken() );
    params.SetAttribute( "progress", CORE::CVariant( progress.progress ) );
    if ( progress.hasTotal )
        params.SetAttribute( "total", CORE::CVariant( progress.total ) );
    if ( !progress.message.IsNULLOrEmpty() )
        params.SetAttribute( "message", CORE::CVariant( progress.message ) );
    dispatch.channel->SendNotification( CMcpMethods::NotificationProgress, &params );
    inFlight.hasSentProgress = true;
    inFlight.lastProgressInMs = now;
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::ProcessContextNews( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_mailboxLock );
        if ( m_mailbox.empty() )
            return;
        m_mailboxWork.swap( m_mailbox );
    }

    const UInt64 now = NowMonotonicMs();
    for ( size_t i=0; i<m_mailboxWork.size(); ++i )
    {
        TContextMap::iterator found = m_inFlightByContext.find( m_mailboxWork[ i ] );
        if ( found == m_inFlightByContext.end() )
            continue;       // ended already
        SInFlight* inFlight = (*found).second;
        CMcpRequestContext& context = *inFlight->dispatch.context;
        context.AcknowledgeNews();

        if ( context.IsCompleted() )
        {
            SChannelState* state = GetChannelState( inFlight->dispatch.channelId );
            if ( GUCEF_NULL == state || state->closed || inFlight->dispatch.channel->IsClosed() )
            {
                // the client went away while the request ran on
                inFlight->dispatch.finished = true;
                RaiseRequestFinished( inFlight->dispatch, MCP_ERA_LEGACY == inFlight->dispatch.era ? MCP_OUTCOME_CLIENT_GONE : MCP_OUTCOME_CANCELLED, 0 );
            }
            else
            {
                // nothing is sent after the final response: progress still waiting is dropped
                FinishCall( inFlight->dispatch );
            }
            EndInFlight( inFlight );
            continue;
        }

        CMcpRequestContext::SProgress progress;
        if ( context.TakePendingProgress( progress ) )
        {
            if ( !inFlight->hasSentProgress || now >= inFlight->lastProgressInMs + m_settings.progressMinIntervalMs )
            {
                inFlight->hasHeldProgress = false;
                m_heldProgress.erase( inFlight );
                SendProgress( *inFlight, progress, now );
            }
            else
            {
                // rate limited: the latest value is kept and sent when the interval ends
                inFlight->heldProgress = progress;
                inFlight->hasHeldProgress = true;
                m_heldProgress.insert( inFlight );
            }
        }
    }
    m_mailboxWork.clear();
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::ProcessHeldProgress( UInt64 now )
{GUCEF_TRACE;

    if ( m_heldProgress.empty() )
        return;
    GUCEF::vector< SInFlight* > due;
    for ( GUCEF::set< SInFlight* >::iterator i = m_heldProgress.begin(); i != m_heldProgress.end(); ++i )
        if ( now >= (*i)->lastProgressInMs + m_settings.progressMinIntervalMs )
            due.push_back( (*i) );
    for ( size_t i=0; i<due.size(); ++i )
    {
        m_heldProgress.erase( due[ i ] );
        if ( due[ i ]->hasHeldProgress )
        {
            due[ i ]->hasHeldProgress = false;
            SendProgress( *due[ i ], due[ i ]->heldProgress, now );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::ProcessTimeouts( UInt64 now )
{GUCEF_TRACE;

    if ( m_inFlightByContext.empty() )
        return;
    GUCEF::vector< SInFlight* > expired;
    for ( TContextMap::iterator i = m_inFlightByContext.begin(); i != m_inFlightByContext.end(); ++i )
        if ( now >= (*i).second->deadlineInMs )
            expired.push_back( (*i).second );
    for ( size_t i=0; i<expired.size(); ++i )
    {
        SInFlight* inFlight = expired[ i ];
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: A " << inFlight->dispatch.method << " request for \"" << inFlight->dispatch.target << "\" timed out" );
        if ( !inFlight->dispatch.channel.IsNULL() )
            inFlight->dispatch.channel->SendError( inFlight->dispatch.id, MCP_ERROR_INTERNAL, "Internal error: request timed out" );
        CancelInFlight( inFlight, MCP_OUTCOME_TIMED_OUT );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::UpdatePeriodicPulses( void )
{GUCEF_TRACE;

    bool wanted = m_started || !m_inFlightByContext.empty();
    if ( wanted && !m_requestingPulses )
    {
        m_requestingPulses = true;
        m_pulseGenerator->RequestPeriodicPulses( this, HousekeepingIntervalInMs );
    }
    else
    if ( !wanted && m_requestingPulses )
    {
        m_requestingPulses = false;
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    MT::SThreadIds ownerIds;
    MT::GetCurrentThreadIds( &ownerIds );
    m_protocolThreadId.Store( ownerIds.threadId );
    GUCEF_SYSTEM_THREAD_ID_CODE( m_protocolSystemThreadId.Store( ownerIds.systemThreadId ); )
    RaiseQueuedRequestEvents();
    ProcessContextNews();
    const UInt64 now = NowMonotonicMs();
    ProcessHeldProgress( now );
    ProcessTimeouts( now );
    ProcessResourceUpdates();
    FlushRefusalSummary( now, false );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::AuditTransportRefusal( const CMcpPrincipal& principal, const CORE::CString& reason, Int32 errorCode )
{GUCEF_TRACE;

    const UInt64 now = NowMonotonicMs();
    FlushRefusalSummary( now, false );
    if ( 0 == m_refusalWindowStartInMs || now - m_refusalWindowStartInMs >= 1000 )
    {
        m_refusalWindowStartInMs = now;
        m_refusalsInWindow = 0;
    }
    if ( m_refusalsInWindow >= MaxRefusalAuditsPerSecond )
    {
        ++m_refusalsSuppressed;
        m_suppressedRefusalTransport = principal.transport;
        m_suppressedRefusalReason = reason;
        return;
    }
    ++m_refusalsInWindow;

    CMcpAuditRecord record;
    record.timestampUtc = CORE::CDateTime::NowUTCDateTime();
    record.transport = principal.transport;
    record.subject = CORE::CRedactor::ForLogLine().RedactText( principal.subject );
    record.isAuthenticated = false;
    record.outcome = MCP_OUTCOME_REFUSED;
    record.errorCode = errorCode;
    record.refusalReason = reason;
    RaiseQueuedRequestEvents();
    TMcpAuditEventData auditData( record );
    NotifyObservers( AuditEvent, &auditData );
}

/*-------------------------------------------------------------------------*/

void
CMcpServer::FlushRefusalSummary( UInt64 now, bool force )
{GUCEF_TRACE;

    if ( 0 == m_refusalsSuppressed || ( !force && now - m_refusalWindowStartInMs < 1000 ) )
        return;

    CMcpAuditRecord record;
    record.timestampUtc = CORE::CDateTime::NowUTCDateTime();
    record.transport = m_suppressedRefusalTransport;
    record.isAuthenticated = false;
    record.outcome = MCP_OUTCOME_REFUSED;
    record.refusalReason = CORE::ToString( m_refusalsSuppressed ) + " more refusals within a second were not recorded one by one, the last: " + m_suppressedRefusalReason;
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: " << record.refusalReason );
    m_refusalsSuppressed = 0;
    m_suppressedRefusalReason.Clear();
    TMcpAuditEventData auditData( record );
    NotifyObservers( AuditEvent, &auditData );
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::PrepareInProcessContext( const CORE::CString& method, const CORE::CString& target, const CMcpPrincipal& principal, SDispatch& dispatch )
{GUCEF_TRACE;

    dispatch.inProcess = true;
    dispatch.method = method;
    dispatch.target = target;
    dispatch.principal = principal;
    dispatch.era = MCP_ERA_MODERN;
    dispatch.meta.SetForInProcess();
    dispatch.startInMs = NowMonotonicMs();
    RaiseRequestStarted( dispatch );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::RunInProcess( SDispatch& dispatch, CMcpOperationTaskData& call, UInt32 maxDurationMs, CORE::CString& error )
{GUCEF_TRACE;

    // a pulse generator of this thread: a deferred handler's event driven work runs here, and only here
    CORE::PulseGeneratorPtr localPulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
    localPulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
    dispatch.context->SetPulseGenerator( localPulseGenerator );

    call.RunHandler();
    const UInt64 deadline = dispatch.startInMs + ( maxDurationMs > 0 ? maxDurationMs : m_settings.maxRequestDurationMs );
    while ( !dispatch.context->IsCompleted() )
    {
        if ( NowMonotonicMs() >= deadline )
        {
            dispatch.context->RequestCancellation();
            dispatch.context->MarkEnded();
            error = "request timed out";
            RaiseRequestFinished( dispatch, MCP_OUTCOME_TIMED_OUT, 0 );
            return false;
        }
        localPulseGenerator->ForceDirectPulse();
        if ( !dispatch.context->IsCompleted() )
            MT::ThreadDelay( 1 );
    }
    dispatch.context->MarkEnded();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::CallTool( const CORE::CString& name             ,
                      const CORE::CDataNode& arguments      ,
                      const CMcpPrincipal& principal        ,
                      CMcpToolResult& result                ,
                      CORE::CString& error                  )
{GUCEF_TRACE;

    result.Clear();
    error.Clear();
    SDispatch dispatch;
    PrepareInProcessContext( CMcpMethods::ToolsCall, name, principal, dispatch );

    CIMcpAccessPolicyPtr holder;
    const CIMcpAccessPolicy& policy = GetPolicy( holder );
    if ( !m_settings.enableTools )
    {
        error = "Method not found: tools are not enabled";
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_METHOD_NOT_FOUND );
        return false;
    }
    const bool toolFound = m_toolRegistry.TryGet( name, dispatch.tool );
    if ( !toolFound || !policy.IsToolVisible( principal, *dispatch.tool ) )
    {
        NoteNotVisible( dispatch, toolFound );
        error = "Unknown tool: " + name;
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_INVALID_PARAMS );
        return false;
    }
    if ( GUCEF_DATATYPE_OBJECT != arguments.GetNodeType() || arguments.HasValue() )
    {
        error = "Invalid params: arguments must be an object";
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_INVALID_PARAMS );
        return false;
    }

    CMcpOperationTaskData call;
    call.operation = CMcpOperationTaskData::OPERATION_TOOL_CALL;
    call.toolEntry = dispatch.tool;
    call.arguments = arguments;
    call.arguments.SetName( CORE::CString::Empty );
    CaptureAuditArguments( dispatch, &call.arguments, dispatch.tool.GetPointerAlways() );
    PrepareCall( dispatch, GUCEF_NULL, &call.arguments, call );

    CORE::CString reason;
    EMcpAccessDecision decision = policy.AuthorizeToolCall( *dispatch.context, *dispatch.tool, call.arguments, reason );
    NoteDecision( dispatch, decision, reason );
    if ( MCP_ACCESS_HIDE == decision )
    {
        error = "Unknown tool: " + name;
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_INVALID_PARAMS );
        return false;
    }
    if ( MCP_ACCESS_DENY == decision )
    {
        result.SetError( "Access denied" + ( reason.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + reason ) );
        RaiseRequestFinished( dispatch, MCP_OUTCOME_TOOL_ERROR, 0 );
        return true;
    }

    CORE::CJsonSchemaValidator::TErrorVector errors;
    if ( !CORE::CJsonSchemaValidator::Validate( dispatch.tool->inputSchema, call.arguments, errors, MaxArgumentErrors ) )
    {
        result.SetError( "Invalid arguments for tool " + name + ":\n" + FormatValidationErrors( errors ) );
        RaiseRequestFinished( dispatch, MCP_OUTCOME_TOOL_ERROR, 0 );
        return true;
    }

    if ( !RunInProcess( dispatch, call, dispatch.tool->descriptor.maxDurationMs, error ) )
        return false;

    const CMcpRequestContext& context = *dispatch.context;
    switch ( context.GetCompletionKind() )
    {
        case CMcpRequestContext::COMPLETION_TOOL_RESULT:
        {
            CORE::CDataNode node;
            if ( !BuildToolResultNode( dispatch, context.GetToolResult(), node, error ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpServer: " << error );
                error = "Internal error: " + error;
                RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_INTERNAL );
                return false;
            }
            result = context.GetToolResult();
            RaiseRequestFinished( dispatch, result.isError ? MCP_OUTCOME_TOOL_ERROR : MCP_OUTCOME_COMPLETE, 0 );
            return true;
        }
        case CMcpRequestContext::COMPLETION_INPUT_REQUIRED:
        {
            CORE::CDataNode required;
            context.GetInputRequired().GetRequiredCapabilities( required );
            CORE::CString names;
            for ( CORE::CDataNode::const_iterator i = required.ConstBegin(); i != required.ConstEnd(); ++i )
                names += ( names.IsNULLOrEmpty() ? "" : ", " ) + (*i)->GetName();
            error = "Missing required client capability (-32021): the tool needs client input" +
                    ( names.IsNULLOrEmpty() ? CORE::CString( ", which an in-process call cannot give" ) : ": " + names );
            RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_MISSING_REQUIRED_CLIENT_CAPABILITY );
            return false;
        }
        case CMcpRequestContext::COMPLETION_PROTOCOL_ERROR:
        default:
        {
            error = context.GetProtocolErrorMessage();
            RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, context.GetProtocolErrorCode() );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::ReadResource( const CORE::CString& requestedUri, const CMcpPrincipal& principal, CMcpResourceContents& contents, CORE::CString& error )
{GUCEF_TRACE;

    contents.Clear();
    error.Clear();
    SDispatch dispatch;
    CORE::CString uri;
    CORE::CString uriProblem;
    const bool uriValid = CMcpResourceRegistry::NormalizeUri( requestedUri, uri, uriProblem );
    PrepareInProcessContext( CMcpMethods::ResourcesRead, uriValid ? uri : requestedUri, principal, dispatch );
    if ( !uriValid )
    {
        error = "Invalid params: the uri is refused: " + uriProblem;
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_INVALID_PARAMS );
        return false;
    }

    CIMcpAccessPolicyPtr holder;
    const CIMcpAccessPolicy& policy = GetPolicy( holder );
    CMcpOperationTaskData call;
    CMcpResourceRegistry::EResolveResult resolved = CMcpResourceRegistry::RESOLVE_NONE;
    if ( m_settings.enableResources )
        resolved = m_resourceRegistry.Resolve( uri, dispatch.resource, dispatch.resourceTemplate, call.variables );
    if ( CMcpResourceRegistry::RESOLVE_NONE == resolved || !policy.IsResourceVisible( principal, uri ) )
    {
        NoteNotVisible( dispatch, CMcpResourceRegistry::RESOLVE_NONE != resolved );
        error = "Resource not found: " + uri;
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_INVALID_PARAMS );
        return false;
    }
    call.operation = CMcpResourceRegistry::RESOLVE_RESOURCE == resolved ? CMcpOperationTaskData::OPERATION_RESOURCE_READ : CMcpOperationTaskData::OPERATION_TEMPLATE_READ;
    call.resourceEntry = dispatch.resource;
    call.templateEntry = dispatch.resourceTemplate;
    call.uri = uri;
    PrepareCall( dispatch, GUCEF_NULL, GUCEF_NULL, call );

    CORE::CString reason;
    EMcpAccessDecision decision = policy.AuthorizeResourceRead( *dispatch.context, uri, reason );
    NoteDecision( dispatch, decision, reason );
    if ( MCP_ACCESS_ALLOW != decision )
    {
        error = MCP_ACCESS_HIDE == decision ? "Resource not found: " + uri : "Access denied" + ( reason.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + reason );
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ACCESS_HIDE == decision ? (Int32) MCP_ERROR_INVALID_PARAMS : (Int32) MCP_ERROR_GUCEF_ACCESS_DENIED );
        return false;
    }
    if ( !RunInProcess( dispatch, call, 0, error ) )
        return false;

    const CMcpRequestContext& context = *dispatch.context;
    if ( CMcpRequestContext::COMPLETION_RESOURCE_CONTENTS == context.GetCompletionKind() )
    {
        contents = context.GetResourceContents();
        RaiseRequestFinished( dispatch, MCP_OUTCOME_COMPLETE, 0 );
        return true;
    }
    if ( CMcpRequestContext::COMPLETION_INPUT_REQUIRED == context.GetCompletionKind() )
    {
        error = "Missing required client capability (-32021): the resource needs client input";
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_MISSING_REQUIRED_CLIENT_CAPABILITY );
        return false;
    }
    error = context.GetProtocolErrorMessage();
    RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, context.GetProtocolErrorCode() );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::GetPrompt( const CORE::CString& name, const CORE::CValueList& arguments, const CMcpPrincipal& principal, CMcpPromptResult& result, CORE::CString& error )
{GUCEF_TRACE;

    result.Clear();
    error.Clear();
    SDispatch dispatch;
    PrepareInProcessContext( CMcpMethods::PromptsGet, name, principal, dispatch );

    CIMcpAccessPolicyPtr holder;
    const CIMcpAccessPolicy& policy = GetPolicy( holder );
    const bool promptFound = m_settings.enablePrompts && m_promptRegistry.TryGet( name, dispatch.prompt );
    if ( !promptFound || !policy.IsPromptVisible( principal, *dispatch.prompt ) )
    {
        NoteNotVisible( dispatch, promptFound );
        error = "Unknown prompt: " + name;
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_INVALID_PARAMS );
        return false;
    }
    CMcpOperationTaskData call;
    call.operation = CMcpOperationTaskData::OPERATION_PROMPT_GET;
    call.promptEntry = dispatch.prompt;
    call.promptArguments = arguments;
    call.arguments = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
    const CMcpPromptDescriptor::TArgumentVector& declared = dispatch.prompt->descriptor.arguments;
    for ( size_t i=0; i<declared.size(); ++i )
    {
        CORE::CVariant value;
        bool present = arguments.TryGetValue( declared[ i ].name, value );
        if ( declared[ i ].required && !present )
        {
            error = "Invalid params: the required argument \"" + declared[ i ].name + "\" of the prompt is missing";
            RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_INVALID_PARAMS );
            return false;
        }
        if ( present )
            call.arguments.SetAttribute( declared[ i ].name, CORE::CVariant( value.AsString() ) );
    }
    CaptureAuditArguments( dispatch, &call.arguments, GUCEF_NULL );
    PrepareCall( dispatch, GUCEF_NULL, &call.arguments, call );

    CORE::CString reason;
    EMcpAccessDecision decision = policy.AuthorizePromptGet( *dispatch.context, *dispatch.prompt, reason );
    NoteDecision( dispatch, decision, reason );
    if ( MCP_ACCESS_ALLOW != decision )
    {
        error = MCP_ACCESS_HIDE == decision ? "Unknown prompt: " + name : "Access denied" + ( reason.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + reason );
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ACCESS_HIDE == decision ? (Int32) MCP_ERROR_INVALID_PARAMS : (Int32) MCP_ERROR_GUCEF_ACCESS_DENIED );
        return false;
    }
    if ( !RunInProcess( dispatch, call, 0, error ) )
        return false;

    const CMcpRequestContext& context = *dispatch.context;
    if ( CMcpRequestContext::COMPLETION_PROMPT_RESULT == context.GetCompletionKind() )
    {
        result = context.GetPromptResult();
        RaiseRequestFinished( dispatch, MCP_OUTCOME_COMPLETE, 0 );
        return true;
    }
    if ( CMcpRequestContext::COMPLETION_INPUT_REQUIRED == context.GetCompletionKind() )
    {
        error = "Missing required client capability (-32021): the prompt needs client input";
        RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, MCP_ERROR_MISSING_REQUIRED_CLIENT_CAPABILITY );
        return false;
    }
    error = context.GetProtocolErrorMessage();
    RaiseRequestFinished( dispatch, MCP_OUTCOME_PROTOCOL_ERROR, context.GetProtocolErrorCode() );
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CMcpServer::QueueInProcess( CMcpOperationTaskData& data )
{GUCEF_TRACE;

    data.server = this;
    try
    {
        CORE::ThreadPoolPtr pool = GetThreadPool();
        if ( !pool.IsNULL() )
        {
            CORE::CFutureResult future = pool->QueueTask( CMcpOperationTaskConsumer::TaskType, &data );
            if ( future.HasAFuture() )
            {
                MT::CScopeMutex lock( m_inProcessLock );
                // forget the calls that ended, keep the ones the destructor has to wait for
                for ( TFutureVector::iterator i = m_inProcessFutures.begin(); i != m_inProcessFutures.end(); )
                {
                    if ( (*i).IsReady() )
                        i = m_inProcessFutures.erase( i );
                    else
                        ++i;
                }
                m_inProcessFutures.push_back( future );
                return future;
            }
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "McpServer: Failed to queue an in-process call: " << e.what() );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CMcpServer::CallToolAsync( const CORE::CString& name, const CORE::CDataNode& arguments, const CMcpPrincipal& principal )
{GUCEF_TRACE;

    CMcpOperationTaskData data;
    data.operation = CMcpOperationTaskData::OPERATION_IN_PROCESS_TOOL_CALL;
    data.name = name;
    data.arguments = arguments;
    data.principal = principal;
    return QueueInProcess( data );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CMcpServer::ReadResourceAsync( const CORE::CString& uri, const CMcpPrincipal& principal )
{GUCEF_TRACE;

    CMcpOperationTaskData data;
    data.operation = CMcpOperationTaskData::OPERATION_IN_PROCESS_RESOURCE_READ;
    data.uri = uri;
    data.principal = principal;
    return QueueInProcess( data );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CMcpServer::GetPromptAsync( const CORE::CString& name, const CORE::CValueList& arguments, const CMcpPrincipal& principal )
{GUCEF_TRACE;

    CMcpOperationTaskData data;
    data.operation = CMcpOperationTaskData::OPERATION_IN_PROCESS_PROMPT_GET;
    data.name = name;
    data.promptArguments = arguments;
    data.principal = principal;
    return QueueInProcess( data );
}

/*-------------------------------------------------------------------------*/

namespace {

const CMcpOperationTaskData*
GetEndedOperation( const CORE::CFutureResult& future, CMcpOperationTaskData::EOperation operation )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return GUCEF_NULL;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() )
        return GUCEF_NULL;
    const CMcpOperationTaskData* data = CORE::CICloneable::CloneableAs< CMcpOperationTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data || data->operation != operation )
        return GUCEF_NULL;
    return data;
}

}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::TryGetToolResult( const CORE::CFutureResult& future, CMcpToolResult& result, CORE::CString& error )
{GUCEF_TRACE;

    const CMcpOperationTaskData* data = GetEndedOperation( future, CMcpOperationTaskData::OPERATION_IN_PROCESS_TOOL_CALL );
    if ( GUCEF_NULL == data )
        return false;
    result = data->toolResult;
    error = data->error;
    return data->succeeded;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::TryGetResourceContents( const CORE::CFutureResult& future, CMcpResourceContents& contents, CORE::CString& error )
{GUCEF_TRACE;

    const CMcpOperationTaskData* data = GetEndedOperation( future, CMcpOperationTaskData::OPERATION_IN_PROCESS_RESOURCE_READ );
    if ( GUCEF_NULL == data )
        return false;
    contents = data->resourceContents;
    error = data->error;
    return data->succeeded;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServer::TryGetPromptResult( const CORE::CFutureResult& future, CMcpPromptResult& result, CORE::CString& error )
{GUCEF_TRACE;

    const CMcpOperationTaskData* data = GetEndedOperation( future, CMcpOperationTaskData::OPERATION_IN_PROCESS_PROMPT_GET );
    if ( GUCEF_NULL == data )
        return false;
    result = data->promptResult;
    error = data->error;
    return data->succeeded;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
