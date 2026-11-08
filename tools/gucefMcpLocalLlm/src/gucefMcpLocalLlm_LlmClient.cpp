/*
 *  gucefMcpLocalLlm: MCP server that delegates coding tasks to local LLMs
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "gucefMcpLocalLlm_LlmClient.h"
#include "gucefMcpLocalLlm_ProcessRun.h"

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpLocalLlm {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 ProbeTimeoutInMs = 3000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
GetJsonCodec( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& codec )
{
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/*-------------------------------------------------------------------------*/

static bool
ExecuteForJson( WEB::CHttpRequestData& request ,
                const CORE::CString& apiKey    ,
                UInt32 timeoutInMs             ,
                CORE::CDataNode& root          ,
                CORE::CString& error           ,
                bool* connectionFailed         )
{
    if ( GUCEF_NULL != connectionFailed )
        *connectionFailed = false;
    if ( !apiKey.IsNULLOrEmpty() )
        request.otherHeaders[ "Authorization" ] = "Bearer " + apiKey;

    WEB::CHttpClientRequestOptions options;
    options.connectTimeoutInMs = ProbeTimeoutInMs;
    options.totalTimeoutInMs = timeoutInMs;

    WEB::CHTTPClient client;
    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo transferInfo;
    if ( !client.Execute( request, options, response, &transferInfo ) )
    {
        if ( GUCEF_NULL != connectionFailed && 0 == response.statusCode )
            *connectionFailed = true;
        error = transferInfo.errorDescription.IsNULLOrEmpty() ? CORE::CString( "no response" ) : transferInfo.errorDescription;
        return false;
    }
    if ( 200 != response.statusCode )
    {
        error = "HTTP status " + CORE::Int32ToString( response.statusCode );
        return false;
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    CORE::CDynamicBufferAccess access( response.content );
    if ( !GetJsonCodec( codec ) || !codec->BuildDataTree( &root, &access ) )
    {
        error = "the response is not JSON";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
HttpGetJson( const CORE::CString& url    ,
             const CORE::CString& apiKey ,
             UInt32 timeoutInMs          ,
             CORE::CDataNode& root       ,
             CORE::CString& error        )
{GUCEF_TRACE;

    WEB::CHttpRequestData request;
    request.requestType = "GET";
    if ( !request.SetUrl( url ) )
    {
        error = "invalid URL " + url;
        return false;
    }
    return ExecuteForJson( request, apiKey, timeoutInMs, root, error, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
HttpPostJson( const CORE::CString& url     ,
              const CORE::CString& apiKey  ,
              const CORE::CDataNode& body  ,
              UInt32 timeoutInMs           ,
              CORE::CDataNode& root        ,
              CORE::CString& error         ,
              bool* connectionFailed       )
{GUCEF_TRACE;

    if ( GUCEF_NULL != connectionFailed )
        *connectionFailed = false;
    WEB::CHttpRequestData request;
    request.requestType = "POST";
    if ( !request.SetUrl( url ) )
    {
        error = "invalid URL " + url;
        return false;
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    CORE::CDynamicBufferAccess bodyAccess( request.content );
    if ( !GetJsonCodec( codec ) || !codec->StoreDataTree( &body, &bodyAccess ) )
    {
        error = "the request could not be written as JSON";
        return false;
    }
    request.contentRepresentation = "application/json";
    return ExecuteForJson( request, apiKey, timeoutInMs, root, error, connectionFailed );
}

/*-------------------------------------------------------------------------*/

SChatRequest::SChatRequest( void )
    : model()
    , system()
    , user()
    , maxTokens( 2048 )
    , thinking( false )
    , timeoutInMs( 600000 )
{
}

/*-------------------------------------------------------------------------*/

bool
ChatCompletion( const CORE::CString& baseUrl ,
                const CORE::CString& apiKey  ,
                const SChatRequest& request  ,
                CORE::CString& answer        ,
                CORE::CString& error         ,
                bool* connectionFailed       )
{GUCEF_TRACE;

    CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
    if ( !request.model.IsNULLOrEmpty() )
        body.SetAttribute( "model", request.model );
    body.SetAttribute( "max_tokens", request.maxTokens );
    body.SetAttribute( "temperature", CORE::CVariant( 0.2 ) );
    CORE::CDataNode* messages = body.AddChild( "messages", GUCEF_DATATYPE_ARRAY );
    if ( !request.system.IsNULLOrEmpty() )
    {
        CORE::CDataNode* message = messages->AddChild( "", GUCEF_DATATYPE_OBJECT );
        message->SetAttribute( "role", "system" );
        message->SetAttribute( "content", request.system );
    }
    CORE::CDataNode* message = messages->AddChild( "", GUCEF_DATATYPE_OBJECT );
    message->SetAttribute( "role", "user" );
    message->SetAttribute( "content", request.user );
    CORE::CDataNode* templateArgs = body.AddChild( "chat_template_kwargs", GUCEF_DATATYPE_OBJECT );
    templateArgs->SetAttribute( "enable_thinking", CORE::CVariant( request.thinking ) );

    CORE::CDataNode response;
    if ( !HttpPostJson( baseUrl + "/v1/chat/completions", apiKey, body, request.timeoutInMs, response, error, connectionFailed ) )
        return false;

    const CORE::CDataNode* choices = response.FindChild( "choices" );
    const CORE::CDataNode* choice = GUCEF_NULL != choices ? choices->GetFirstChild() : GUCEF_NULL;
    const CORE::CDataNode* reply = GUCEF_NULL != choice ? choice->FindChild( "message" ) : GUCEF_NULL;
    if ( GUCEF_NULL == reply )
    {
        error = "the response has no message";
        return false;
    }
    answer = reply->GetAttributeValue( "content" ).AsString();
    if ( answer.IsNULLOrEmpty() )
    {
        error = "the answer is empty";
        if ( GUCEF_NULL != choice && choice->GetAttributeValue( "finish_reason" ).AsString() == "length" )
            error += " (the model used up max_tokens, probably while thinking)";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
HostOfUrl( const CORE::CString& url )
{GUCEF_TRACE;

    std::string text = url.STL_String();
    const size_t scheme = text.find( "://" );
    if ( std::string::npos != scheme )
        text = text.substr( scheme + 3 );
    const size_t pathStart = text.find( '/' );
    if ( std::string::npos != pathStart )
        text = text.substr( 0, pathStart );
    const size_t at = text.rfind( '@' );
    if ( std::string::npos != at )
        text = text.substr( at + 1 );
    if ( !text.empty() && '[' == text[ 0 ] )
    {
        // [IPv6]:port
        const size_t close = text.find( ']' );
        return CORE::CString( text.substr( 1, std::string::npos != close ? close - 1 : std::string::npos ) );
    }
    const size_t portStart = text.find( ':' );
    if ( std::string::npos != portStart )
        text = text.substr( 0, portStart );
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

bool
IsLocalHost( const CORE::CString& host, bool* resolved )
{GUCEF_TRACE;

    if ( GUCEF_NULL != resolved )
        *resolved = true;
    const CORE::CString lowerHost = host.Lowercase();
    if ( lowerHost == "localhost" || lowerHost == "::1" || lowerHost.StartsWith( "127." ) )
        return true;

    COMCORE::CHostAddress address;
    if ( !address.SetHostname( host ) )
    {
        if ( GUCEF_NULL != resolved )
            *resolved = false;
        return false;
    }
    const COMCORE::CIPv4Address::TIPv4AddressVector& hostIps = address.GetIPv4Addresses();

    COMCORE::CINetworkInterface::TIPv4InfoVector localIps;
    COMCORE::CComCoreGlobal::Instance()->GetCom().GetAllNetworkInterfaceIPInfo( localIps );
    for ( size_t h=0; h<hostIps.size(); ++h )
    {
        if ( ( hostIps[ h ].GetAddressInHostByteOrder() >> 24 ) == 127 )
            return true;
        for ( size_t l=0; l<localIps.size(); ++l )
            if ( hostIps[ h ] == localIps[ l ].ip )
                return true;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

SEndpointState::SEndpointState( void )
    : url()
    , reachable( false )
    , error()
    , model()
    , contextSize( 0 )
    , slots( 0 )
    , idleSlots( 0 )
    , inFlight( 0 )
    , isLocal( false )
    , isEphemeral( false )
    , downForMs( 0 )
{
}

/*-------------------------------------------------------------------------*/

CLlmEndpointPool::CLlmEndpointPool( const CORE::CString::StringVector& urls          ,
                                    const CORE::CString::StringVector& ephemeralUrls ,
                                    const CORE::CString& apiKey                      ,
                                    UInt32 minContextSize                            )
    : m_endpoints()
    , m_givenEndpoints()
    , m_endpointsFile()
    , m_endpointsFileStamp()
    , m_apiKey( apiKey )
    , m_minContextSize( minContextSize )
    , m_lock()
    , m_inFlight()
{
    for ( size_t i=0; i<urls.size(); ++i )
    {
        SEndpoint endpoint;
        endpoint.url = urls[ i ];
        for ( size_t e=0; e<ephemeralUrls.size(); ++e )
            endpoint.ephemeral = endpoint.ephemeral || ephemeralUrls[ e ] == urls[ i ];
        m_endpoints.push_back( endpoint );
    }
    m_givenEndpoints = m_endpoints;
}

/*-------------------------------------------------------------------------*/

void
CLlmEndpointPool::SetEndpointsFile( const CORE::CString& path )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_endpointsFile = path;
        m_endpointsFileStamp.Clear();
    }
    RefreshFromFile();
}

/*-------------------------------------------------------------------------*/

void
CLlmEndpointPool::RefreshFromFile( void )
{GUCEF_TRACE;

    CORE::CString path;
    CORE::CString previousStamp;
    {
        MT::CScopeMutex lock( m_lock );
        path = m_endpointsFile;
        previousStamp = m_endpointsFileStamp;
    }
    if ( path.IsNULLOrEmpty() )
        return;

    const bool exists = CORE::FileExists( path );
    const CORE::CString stamp = exists ? CORE::GetFileModificationTime( path ).ToIso8601DateTimeString( false, true ) + "/" + CORE::ToString( CORE::FileSize( path ) ) : CORE::CString( "none" );
    if ( stamp == previousStamp )
        return;

    // read outside the lock; a file that cannot be read now is read at a later call
    TEndpointVector listed;
    if ( exists )
    {
        CORE::CDynamicBuffer content;
        if ( !content.LoadContentFromFile( path ) )
            return;
        const CORE::CString text = 0 == content.GetDataSize() ? CORE::CString::Empty : CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
        const CORE::CString::StringVector lines = text.ParseElements( '\n', false );
        for ( size_t i=0; i<lines.size(); ++i )
        {
            CORE::CString line = lines[ i ];
            const Int32 comment = line.HasChar( '#' );
            if ( comment >= 0 )
                line = line.SubstrToIndex( (UInt32) comment, true );
            line = line.Trim( true ).Trim( false );
            if ( line.IsNULLOrEmpty() )
                continue;
            SEndpoint endpoint;
            if ( line.StartsWith( "ephemeral " ) || line.StartsWith( "ephemeral\t" ) )
            {
                endpoint.ephemeral = true;
                line = line.CutChars( 10, true ).Trim( true );
            }
            while ( line.EndsWith( "/" ) )
                line = line.CutChars( 1, false );
            endpoint.url = line;
            listed.push_back( endpoint );
        }
    }

    MT::CScopeMutex lock( m_lock );
    m_endpointsFileStamp = stamp;

    // the given endpoints first, then those of the file; an endpoint that stays keeps what is known about it
    TEndpointVector endpoints = m_givenEndpoints;
    for ( size_t i=0; i<listed.size(); ++i )
    {
        bool known = false;
        for ( size_t e=0; e<endpoints.size(); ++e )
            known = known || endpoints[ e ].url == listed[ i ].url;
        if ( !known )
            endpoints.push_back( listed[ i ] );
    }
    for ( size_t i=0; i<endpoints.size(); ++i )
    {
        for ( size_t e=0; e<m_endpoints.size(); ++e )
        {
            if ( m_endpoints[ e ].url != endpoints[ i ].url )
                continue;
            const bool ephemeral = endpoints[ i ].ephemeral;
            endpoints[ i ] = m_endpoints[ e ];
            endpoints[ i ].ephemeral = ephemeral;
        }
    }
    m_endpoints = endpoints;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpLocalLlm: " << (UInt32) m_endpoints.size() << " LLM server(s) after reading " << path );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLlmEndpointPool::GetApiKey( void ) const
{
    return m_apiKey;
}

/*-------------------------------------------------------------------------*/

UInt32
CLlmEndpointPool::GetMinContextSize( void ) const
{
    return m_minContextSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CLlmEndpointPool::InFlight( const CORE::CString& url ) const
{
    MT::CScopeMutex lock( m_lock );
    TInFlightMap::const_iterator found = m_inFlight.find( url );
    return found != m_inFlight.end() ? (*found).second : 0;
}

/*-------------------------------------------------------------------------*/

void
CLlmEndpointPool::Probe( const CORE::CString& url, SEndpointState& state ) const
{GUCEF_TRACE;

    state = SEndpointState();
    state.url = url;

    CORE::CDataNode health;
    if ( !HttpGetJson( url + "/health", m_apiKey, ProbeTimeoutInMs, health, state.error ) )
        return;
    state.reachable = true;

    CORE::CDataNode props;
    CORE::CString error;
    if ( HttpGetJson( url + "/props", m_apiKey, ProbeTimeoutInMs, props, error ) )
    {
        state.model = props.GetAttributeValue( "model_alias" ).AsString();
        state.slots = props.GetAttributeValue( "total_slots" ).AsUInt32();
        const CORE::CDataNode* settings = props.FindChild( "default_generation_settings" );
        if ( GUCEF_NULL != settings )
            state.contextSize = settings->GetAttributeValue( "n_ctx" ).AsUInt32();
    }

    CORE::CDataNode slotList;
    if ( HttpGetJson( url + "/slots", m_apiKey, ProbeTimeoutInMs, slotList, error ) )
    {
        UInt32 slotCount = 0;
        for ( CORE::CDataNode::const_iterator i = slotList.ConstBegin(); i != slotList.ConstEnd(); ++i )
        {
            ++slotCount;
            if ( !(*i)->GetAttributeValue( "is_processing" ).AsBool( true ) )
                ++state.idleSlots;
        }
        if ( 0 == state.slots )
            state.slots = slotCount;
    }
    else
    {
        // a server started with --no-slots: its slots count as idle unless this server uses them
        if ( 0 == state.slots )
            state.slots = 1;
        state.idleSlots = state.slots;
    }
    state.inFlight = InFlight( url );
}

/*-------------------------------------------------------------------------*/

UInt32
CLlmEndpointPool::PreferenceClass( SEndpoint& endpoint )
{GUCEF_TRACE;

    // resolved once; a name that cannot be resolved yet is tried again when the endpoint is probed again
    const bool inRetryDelay = 0 != endpoint.downSinceInMs && MilliSecsSince( endpoint.downSinceInMs ) < RetryDelayInMs;
    if ( !endpoint.localityKnown && !inRetryDelay )
        endpoint.isLocal = IsLocalHost( HostOfUrl( endpoint.url ), &endpoint.localityKnown );
    if ( endpoint.isLocal )
        return 0;
    return endpoint.ephemeral ? 2 : 1;
}

/*-------------------------------------------------------------------------*/

void
CLlmEndpointPool::ProbeAll( std::vector< SEndpointState >& states, bool recheck )
{GUCEF_TRACE;

    RefreshFromFile();

    // a copy to work on: name resolution and probes run without the lock
    std::vector< SEndpoint > endpoints;
    {
        MT::CScopeMutex lock( m_lock );
        endpoints = m_endpoints;
    }
    std::vector< UInt32 > classes;
    for ( size_t i=0; i<endpoints.size(); ++i )
        classes.push_back( PreferenceClass( endpoints[ i ] ) );
    {
        MT::CScopeMutex lock( m_lock );
        // by URL: the list may have changed meanwhile
        for ( size_t i=0; i<endpoints.size(); ++i )
        {
            if ( !endpoints[ i ].localityKnown )
                continue;
            for ( size_t e=0; e<m_endpoints.size(); ++e )
            {
                if ( m_endpoints[ e ].url != endpoints[ i ].url || m_endpoints[ e ].localityKnown )
                    continue;
                m_endpoints[ e ].localityKnown = true;
                m_endpoints[ e ].isLocal = endpoints[ i ].isLocal;
            }
        }
    }

    states.clear();
    const UInt64 now = MT::PrecisionTickCount();
    for ( UInt32 preference=0; preference<3; ++preference )
    {
        for ( size_t i=0; i<endpoints.size(); ++i )
        {
            if ( classes[ i ] != preference )
                continue;
            const SEndpoint& endpoint = endpoints[ i ];
            SEndpointState state;
            const UInt64 downForMs = 0 != endpoint.downSinceInMs ? MilliSecsSince( endpoint.downSinceInMs ) : 0;
            if ( !recheck && 0 != endpoint.downSinceInMs && downForMs < RetryDelayInMs )
            {
                state.url = endpoint.url;
                state.error = endpoint.downError;
                state.downForMs = (UInt32) downForMs;
                state.inFlight = InFlight( endpoint.url );
            }
            else
            {
                Probe( endpoint.url, state );
            }
            state.isLocal = endpoint.isLocal;
            state.isEphemeral = endpoint.ephemeral && !endpoint.isLocal;

            if ( 0 == state.downForMs )
            {
                MT::CScopeMutex lock( m_lock );
                for ( size_t e=0; e<m_endpoints.size(); ++e )
                {
                    if ( m_endpoints[ e ].url != endpoint.url )
                        continue;
                    m_endpoints[ e ].downSinceInMs = state.reachable ? 0 : now;
                    m_endpoints[ e ].downError = state.error;
                }
            }
            states.push_back( state );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CLlmEndpointPool::Recheck( const CORE::CString& url )
{GUCEF_TRACE;

    SEndpointState state;
    Probe( url, state );
    MT::CScopeMutex lock( m_lock );
    for ( size_t e=0; e<m_endpoints.size(); ++e )
    {
        if ( m_endpoints[ e ].url != url )
            continue;
        m_endpoints[ e ].downSinceInMs = state.reachable ? 0 : MT::PrecisionTickCount();
        m_endpoints[ e ].downError = state.error;
    }
    return state.reachable;
}

/*-------------------------------------------------------------------------*/

bool
CLlmEndpointPool::IsUsable( const SEndpointState& state ) const
{
    // an unknown context size (0) is accepted; the agent reports a context that is too small itself
    return state.reachable && ( 0 == state.contextSize || state.contextSize >= m_minContextSize );
}

/*-------------------------------------------------------------------------*/

bool
CLlmEndpointPool::RunsModel( const SEndpointState& state, const CORE::CString& model )
{
    return model.IsNULLOrEmpty() || state.model.Lowercase() == model.Lowercase();
}

/*-------------------------------------------------------------------------*/

bool
CLlmEndpointPool::IsEmpty( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_endpoints.empty() && m_endpointsFile.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

UInt32
CLlmEndpointPool::CountFreeSlots( const std::vector< SEndpointState >& states, const CORE::CString& model ) const
{GUCEF_TRACE;

    UInt32 free = 0;
    for ( size_t i=0; i<states.size(); ++i )
    {
        if ( !IsUsable( states[ i ] ) || !RunsModel( states[ i ], model ) )
            continue;
        const UInt32 used = InFlight( states[ i ].url );
        if ( states[ i ].idleSlots > used )
            free += states[ i ].idleSlots - used;
    }
    return free;
}

/*-------------------------------------------------------------------------*/

bool
CLlmEndpointPool::AcquireFrom( const std::vector< SEndpointState >& states, SEndpointState& chosen, const CORE::CString& model )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    const SEndpointState* pick = GUCEF_NULL;
    const SEndpointState* firstUsable = GUCEF_NULL;
    for ( size_t i=0; i<states.size(); ++i )
    {
        const SEndpointState& state = states[ i ];
        if ( !IsUsable( state ) || !RunsModel( state, model ) )
            continue;
        if ( GUCEF_NULL == firstUsable )
            firstUsable = &state;
        TInFlightMap::const_iterator found = m_inFlight.find( state.url );
        const UInt32 inFlight = found != m_inFlight.end() ? (*found).second : 0;
        if ( state.idleSlots > inFlight )
        {
            pick = &state;
            break;
        }
    }
    if ( GUCEF_NULL == pick )
        pick = firstUsable;
    if ( GUCEF_NULL == pick )
        return false;

    chosen = *pick;
    ++m_inFlight[ pick->url ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLlmEndpointPool::Acquire( SEndpointState& chosen, CORE::CString& report, const CORE::CString& model )
{GUCEF_TRACE;

    std::vector< SEndpointState > states;
    ProbeAll( states );
    report = Describe( states );
    return AcquireFrom( states, chosen, model );
}

/*-------------------------------------------------------------------------*/

void
CLlmEndpointPool::Release( const CORE::CString& url )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TInFlightMap::iterator found = m_inFlight.find( url );
    if ( found != m_inFlight.end() && (*found).second > 0 )
        --(*found).second;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLlmEndpointPool::Describe( const std::vector< SEndpointState >& states ) const
{GUCEF_TRACE;

    if ( m_endpoints.empty() )
        return "No LLM server is configured (--endpoint, --ephemeral-endpoint).\n";
    CORE::CString text;
    for ( size_t i=0; i<states.size(); ++i )
    {
        const SEndpointState& state = states[ i ];
        text += CORE::UInt32ToString( (UInt32) i+1 ) + ". " + state.url;
        if ( state.isLocal )
            text += " (this machine)";
        else if ( state.isEphemeral )
            text += " (ephemeral: not always on)";
        text += ": ";
        if ( !state.reachable )
        {
            text += "not reachable (" + state.error + ")";
            if ( state.downForMs > 0 )
                text += ", checked " + CORE::UInt32ToString( state.downForMs / 1000 ) + "s ago";
            text += "\n";
            continue;
        }
        text += "reachable, model " + ( state.model.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : state.model );
        text += ", context " + ( 0 == state.contextSize ? CORE::CString( "unknown" ) : CORE::UInt32ToString( state.contextSize ) + " tokens" );
        text += ", " + CORE::UInt32ToString( state.idleSlots ) + " of " + CORE::UInt32ToString( state.slots ) + " slots idle";
        if ( state.inFlight > 0 )
            text += ", " + CORE::UInt32ToString( state.inFlight ) + " task(s) of this server running";
        if ( 0 != state.contextSize && state.contextSize < m_minContextSize )
            text += "; not used: context below the minimum of " + CORE::UInt32ToString( m_minContextSize ) + " tokens";
        text += "\n";
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLlmEndpointPool::DescribeModels( const std::vector< SEndpointState >& states ) const
{GUCEF_TRACE;

    // in the order of preference of their first endpoint
    std::vector< CORE::CString > names;
    std::map< CORE::CString, UInt32 > servers;
    std::map< CORE::CString, UInt32 > idle;
    for ( size_t i=0; i<states.size(); ++i )
    {
        const SEndpointState& state = states[ i ];
        if ( !IsUsable( state ) || state.model.IsNULLOrEmpty() )
            continue;
        const CORE::CString key = state.model.Lowercase();
        if ( servers.find( key ) == servers.end() )
            names.push_back( state.model );
        ++servers[ key ];
        const UInt32 used = InFlight( state.url );
        idle[ key ] += state.idleSlots > used ? state.idleSlots - used : 0;
    }
    CORE::CString text;
    for ( size_t i=0; i<names.size(); ++i )
    {
        const CORE::CString key = names[ i ].Lowercase();
        text += ( i > 0 ? ", " : "" ) + names[ i ] + " (" + CORE::UInt32ToString( servers[ key ] ) +
                ( 1 == servers[ key ] ? " server, " : " servers, " ) + CORE::UInt32ToString( idle[ key ] ) +
                ( 1 == idle[ key ] ? " idle slot)" : " idle slots)" );
    }
    return text;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpLocalLlm */

/*-------------------------------------------------------------------------*/
