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

#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#include "gucefCORE_CPlatformNativeConsoleLogger.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#include "gucefCORE_CStandardOutputWriter.h"
#define GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#endif /* GUCEF_CORE_CSTANDARDOUTPUTWRITER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CSTDLOGGER_H
#include "CStdLogger.h"
#define GUCEF_CORE_CSTDLOGGER_H
#endif /* GUCEF_CORE_CSTDLOGGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#ifndef GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H
#include "gucefCOM_CMcpConfigurableAccessPolicy.h"
#define GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H
#endif /* GUCEF_COM_CMCPCONFIGURABLEACCESSPOLICY_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#include "gucefWEB_CMcpStreamableHttpTransport.h"
#define GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#endif /* GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <unistd.h>
#endif

#include "gucefMcpLocalLlm_Tools.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The command line, in the form --name value or --name=value. Options left out are taken from the environment.
 *
 *    --endpoint URL        an LLM server (llama.cpp llama-server). Repeat for more, in order of preference.
 *                          Environment: LOCAL_LLM_URLS (comma separated) or LOCAL_LLM_URL. Without any LLM server
 *                          only run_and_digest is offered, without explanations of unknown failures
 *    --ephemeral-endpoint URL  an LLM server on a machine that is not always on, such as a workstation that is an
 *                          ephemeral node of the cluster. Used after the others, except on that machine itself, where
 *                          it is used first. Repeat for more. Environment: LOCAL_LLM_EPHEMERAL_URLS (comma separated)
 *    --agent-script PATH   the coding agent client: local-agent.sh, or local-agent.ps1 on Windows.
 *                          Environment: LOCAL_AGENT_SCRIPT; default: next to this executable
 *    --model NAME          the model calls use when they name none, as the servers report it; when no reachable
 *                          server runs it, any model is used. Environment: LOCAL_LLM_MODEL; default: any model
 *    --timeout SECONDS     a task running longer is stopped. Environment: LOCAL_LLM_TIMEOUT; default 900
 *    --min-context N       servers with a smaller context per request are not used; default 100000, the smallest
 *                          window Claude Code works with
 *    --max-diff-lines N    a delegation result holds the diff when it has at most this many lines; default 150, 0: never
 *    --allow-command PAT   a command run_and_digest may run, * and ? as wildcards: "cmake --build *" for example.
 *                          Repeat for more. Without it run_and_digest is not offered
 *    --command-timeout S   run_and_digest stops a command after this many seconds; default 3600
 *    --log-dir PATH        where run_and_digest keeps the full output of commands; default: gucefMcpLocalLlm in the
 *                          temporary directory
 *    --endpoints-file PATH a file with more LLM servers, read again whenever it changes: one URL per line,
 *                          "ephemeral URL" for one on a machine that is not always on. Environment: LOCAL_LLM_ENDPOINTS_FILE
 *    --log-file PATH       also writes the log to a file; the log goes to standard error only otherwise
 *    --listen ADDR:PORT    serves MCP over HTTP (streamable HTTP, path /mcp) instead of stdio, for any number of
 *                          clients. Only a loopback address: plain HTTP is for this machine
 *    --listen-tls ADDR:PORT  serves MCP over HTTPS, with --tls-cert and --tls-key, for clients on other machines
 *    --tls-cert PATH       PEM certificate of this machine, followed by its chain; read again when it changes
 *    --tls-key PATH        PEM private key of the certificate
 *    --token-file PATH     the bearer token HTTP clients send (Authorization: Bearer). Made, for the owner only,
 *                          when it does not exist. Required with --listen and --listen-tls
 *    --allowed-host NAME   a name clients of --listen-tls use for this machine (the Host they send): its host
 *                          name, its address. Repeat for more; localhost and 127.0.0.1 always are
 *    --help
 *
 *  The API key of the servers, when they require one, is read from the environment: LOCAL_LLM_API_KEY.
 */
struct SOptions
{
    CORE::CString::StringVector endpoints;
    CORE::CString::StringVector ephemeralEndpoints;
    CORE::CString agentScript;
    CORE::CString model;
    UInt32 timeoutInSecs;
    UInt32 minContextSize;
    UInt32 maxDiffLines;
    CORE::CString::StringVector allowedCommands;
    UInt32 commandTimeoutInSecs;
    CORE::CString logDir;
    CORE::CString logFile;
    CORE::CString endpointsFile;
    CORE::CString listen;
    CORE::CString listenTls;
    CORE::CString tlsCert;
    CORE::CString tlsKey;
    CORE::CString tokenFile;
    CORE::CString::StringVector allowedHosts;
    bool help;

    SOptions( void ) : endpoints(), ephemeralEndpoints(), agentScript(), model(), timeoutInSecs( 0 ), minContextSize( 100000 ), maxDiffLines( 150 ), allowedCommands(),
                       commandTimeoutInSecs( 3600 ), logDir(), logFile(), endpointsFile(), listen(), listenTls(), tlsCert(), tlsKey(), tokenFile(), allowedHosts(),
                       help( false ) {}
};

/*-------------------------------------------------------------------------*/

static bool
ParseOptions( int argc, char* argv[], SOptions& options )
{
    for ( int i=1; i<argc; ++i )
    {
        CORE::CString arg = argv[ i ];
        CORE::CString value;
        const char* equals = ::strchr( argv[ i ], '=' );
        if ( arg.Length() > 2 && 0 == ::strncmp( argv[ i ], "--", 2 ) && GUCEF_NULL != equals )
        {
            value = equals + 1;
            arg = CORE::CString( argv[ i ], (UInt32) ( equals - argv[ i ] ) );
        }
        const bool needsValue = arg == "--endpoint" || arg == "--ephemeral-endpoint" || arg == "--agent-script" || arg == "--model" || arg == "--timeout" ||
                                arg == "--min-context" || arg == "--log-file" || arg == "--max-diff-lines" || arg == "--allow-command" ||
                                arg == "--command-timeout" || arg == "--log-dir" || arg == "--endpoints-file" || arg == "--listen" ||
                                arg == "--listen-tls" || arg == "--tls-cert" || arg == "--tls-key" || arg == "--token-file" || arg == "--allowed-host";
        if ( needsValue && value.IsNULLOrEmpty() )
        {
            if ( i+1 >= argc )
                return false;
            value = argv[ ++i ];
        }

        if ( arg == "--endpoint" )
            options.endpoints.push_back( value );
        else
        if ( arg == "--ephemeral-endpoint" )
            options.ephemeralEndpoints.push_back( value );
        else
        if ( arg == "--agent-script" )
            options.agentScript = value;
        else
        if ( arg == "--model" )
            options.model = value;
        else
        if ( arg == "--timeout" )
            options.timeoutInSecs = CORE::StringToUInt32( value );
        else
        if ( arg == "--min-context" )
            options.minContextSize = CORE::StringToUInt32( value );
        else
        if ( arg == "--log-file" )
            options.logFile = value;
        else
        if ( arg == "--max-diff-lines" )
            options.maxDiffLines = CORE::StringToUInt32( value );
        else
        if ( arg == "--allow-command" )
            options.allowedCommands.push_back( value );
        else
        if ( arg == "--command-timeout" )
            options.commandTimeoutInSecs = CORE::StringToUInt32( value, 3600 );
        else
        if ( arg == "--log-dir" )
            options.logDir = value;
        else
        if ( arg == "--endpoints-file" )
            options.endpointsFile = value;
        else
        if ( arg == "--listen" )
            options.listen = value;
        else
        if ( arg == "--listen-tls" )
            options.listenTls = value;
        else
        if ( arg == "--tls-cert" )
            options.tlsCert = value;
        else
        if ( arg == "--tls-key" )
            options.tlsKey = value;
        else
        if ( arg == "--token-file" )
            options.tokenFile = value;
        else
        if ( arg == "--allowed-host" )
            options.allowedHosts.push_back( value );
        else
        if ( arg == "--help" )
            options.help = true;
        else
        if ( arg == "--console" )
            continue;                           // read by the entry point of the framework
        else
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Fills in what the command line left out from the environment and the defaults */
static void
ApplyEnvironmentAndDefaults( SOptions& options )
{
    if ( options.endpoints.empty() )
    {
        const CORE::CString urls = CORE::GetEnv( "LOCAL_LLM_URLS" );
        if ( !urls.IsNULLOrEmpty() )
            options.endpoints = urls.ParseElements( ',', false );
    }
    if ( options.endpoints.empty() )
    {
        const CORE::CString url = CORE::GetEnv( "LOCAL_LLM_URL" );
        if ( !url.IsNULLOrEmpty() )
            options.endpoints.push_back( url );
    }
    if ( options.ephemeralEndpoints.empty() )
    {
        const CORE::CString urls = CORE::GetEnv( "LOCAL_LLM_EPHEMERAL_URLS" );
        if ( !urls.IsNULLOrEmpty() )
            options.ephemeralEndpoints = urls.ParseElements( ',', false );
    }

    CORE::CString::StringVector* lists[] = { &options.endpoints, &options.ephemeralEndpoints };
    for ( size_t l=0; l<2; ++l )
    {
        CORE::CString::StringVector& list = *lists[ l ];
        for ( size_t i=0; i<list.size(); ++i )
        {
            CORE::CString url = list[ i ].Trim( true ).Trim( false );
            while ( url.EndsWith( "/" ) )
                url = url.CutChars( 1, false );
            list[ i ] = url;
        }
    }
    // all endpoints, the ephemeral ones after the others
    for ( size_t i=0; i<options.ephemeralEndpoints.size(); ++i )
    {
        bool listed = false;
        for ( size_t e=0; e<options.endpoints.size(); ++e )
            listed = listed || options.endpoints[ e ] == options.ephemeralEndpoints[ i ];
        if ( !listed )
            options.endpoints.push_back( options.ephemeralEndpoints[ i ] );
    }

    if ( options.endpointsFile.IsNULLOrEmpty() )
        options.endpointsFile = CORE::GetEnv( "LOCAL_LLM_ENDPOINTS_FILE" );

    if ( options.agentScript.IsNULLOrEmpty() )
        options.agentScript = CORE::GetEnv( "LOCAL_AGENT_SCRIPT" );
    if ( options.agentScript.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        options.agentScript = CORE::RelativePath( "$MODULEDIR$/local-agent.ps1" );
        #else
        options.agentScript = CORE::RelativePath( "$MODULEDIR$/local-agent.sh" );
        #endif
    }

    if ( options.model.IsNULLOrEmpty() )
        options.model = CORE::GetEnv( "LOCAL_LLM_MODEL" );
    if ( 0 == options.timeoutInSecs )
        options.timeoutInSecs = CORE::StringToUInt32( CORE::GetEnv( "LOCAL_LLM_TIMEOUT" ), 900 );
    if ( 0 == options.timeoutInSecs )
        options.timeoutInSecs = 900;
    if ( 0 == options.commandTimeoutInSecs )
        options.commandTimeoutInSecs = 3600;

    if ( options.logDir.IsNULLOrEmpty() )
    {
        CORE::CString tempDir = CORE::GetEnv( "TMPDIR" );
        if ( tempDir.IsNULLOrEmpty() )
            tempDir = CORE::GetEnv( "TEMP" );
        if ( tempDir.IsNULLOrEmpty() )
            tempDir = CORE::GetEnv( "TMP" );
        if ( tempDir.IsNULLOrEmpty() )
            tempDir = "/tmp";
        options.logDir = CORE::CombinePath( tempDir, "gucefMcpLocalLlm" );
    }
}

/*-------------------------------------------------------------------------*/

/** Whether the options are usable together, problem says why not */
static bool
CheckServingOptions( const SOptions& options, CORE::CString& problem )
{
    if ( ( !options.listen.IsNULLOrEmpty() || !options.listenTls.IsNULLOrEmpty() ) && options.tokenFile.IsNULLOrEmpty() )
    {
        problem = "--listen and --listen-tls need --token-file: HTTP clients are only served with the token";
        return false;
    }
    if ( !options.listenTls.IsNULLOrEmpty() && ( options.tlsCert.IsNULLOrEmpty() || options.tlsKey.IsNULLOrEmpty() ) )
    {
        problem = "--listen-tls needs --tls-cert and --tls-key";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The name of this machine, empty when unknown */
static CORE::CString
GetMachineName( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    char name[ 256 ];
    if ( 0 == ::gethostname( name, sizeof( name ) - 1 ) )
    {
        name[ sizeof( name ) - 1 ] = '\0';
        return name;
    }
    return CORE::CString::Empty;
    #else
    return CORE::GetEnv( "COMPUTERNAME" );
    #endif
}

/*-------------------------------------------------------------------------*/

/**
 *  The streamable HTTP transport on ADDR:PORT, with the bearer token of the token file; with TLS for clients on
 *  other machines, which name this machine by one of the allowed hosts
 */
static COM::CIMcpTransportPtr
MakeHttpTransport( const CORE::CString& listen, bool tls, const SOptions& options, CORE::CString& problem )
{
    const Int32 colon = listen.HasChar( ':', false );
    const CORE::CString address = colon > 0 ? listen.SubstrToIndex( (UInt32) colon, true ) : CORE::CString::Empty;
    const UInt32 port = colon > 0 ? CORE::StringToUInt32( listen.CutChars( (UInt32) colon + 1, true ) ) : 0;
    if ( address.IsNULLOrEmpty() || 0 == port || port > 65535 )
    {
        problem = "\"" + listen + "\" is not ADDRESS:PORT";
        return COM::CIMcpTransportPtr();
    }

    WEB::CMcpHttpTransportSettings settings;
    settings.bindAddress = address;
    settings.port = (UInt16) port;
    if ( tls )
    {
        settings.tls.SetCertificateChainPath( options.tlsCert );
        settings.tls.SetPrivateKeyPath( options.tlsKey );
        settings.tls.AddAlpnProtocol( "http/1.1" );
        settings.allowedHosts.push_back( "localhost" );
        settings.allowedHosts.push_back( "127.0.0.1" );
        for ( size_t i=0; i<options.allowedHosts.size(); ++i )
            settings.allowedHosts.push_back( options.allowedHosts[ i ] );
    }

    COM::CIMcpTransportPtr transport = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( COM::CIMcpTransport::StreamableHttpTransportName );
    WEB::CIMcpHttpAuthenticatorPtr authenticator = WEB::CWebGlobal::Instance()->GetMcpHttpAuthenticatorFactory().Create( WEB::CIMcpHttpAuthenticator::StaticBearerAuthenticatorName );
    if ( transport.IsNULL() || authenticator.IsNULL() )
    {
        problem = "the streamable HTTP transport or its bearer authenticator is not available";
        return COM::CIMcpTransportPtr();
    }
    CORE::CDataNode authenticatorConfig( GUCEF_DATATYPE_OBJECT );
    authenticatorConfig.SetAttribute( "bearerTokenFile", CORE::CVariant( options.tokenFile ) );
    authenticatorConfig.SetAttribute( "tokenLabel", CORE::CVariant( tls ? "remote" : "local" ) );
    authenticator->LoadConfig( authenticatorConfig );

    WEB::CMcpStreamableHttpTransport* http = static_cast< WEB::CMcpStreamableHttpTransport* >( transport.GetPointerAlways() );
    http->SetSettings( settings );
    http->SetAuthenticator( authenticator );
    return transport;
}

/*-------------------------------------------------------------------------*/

static bool
EnsureJsonCodec( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return true;
    const CORE::CString pluginName = "dstorepluginJSONPARSER";
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "gucefMcpLocalLlm", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "gucefMcpLocalLlm", true );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/*-------------------------------------------------------------------------*/

/** Ends the main loop of the application when the server asks the application to stop */
class CShutdownListener : public CORE::CObserver
{
    public:

    explicit CShutdownListener( COM::CMcpServer& server )
    {
        typedef CORE::CTEventHandlerFunctor< CShutdownListener > TEventCallback;
        TEventCallback callback( this, &CShutdownListener::OnShutdownRequested );
        SubscribeTo( &server, COM::CMcpServer::ShutdownRequestedEvent, callback );
    }

    virtual ~CShutdownListener() GUCEF_VIRTUAL_OVERRIDE
    {
        UnsubscribeAllFromObserver();
    }

    void OnShutdownRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpLocalLlm: Stopping" );
        CORE::CCoreGlobal::Instance()->GetApplication().Stop();
    }
};

/*-------------------------------------------------------------------------*/

static void
PrintUsage( void )
{
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpLocalLlm: An MCP server (stdio) that delegates coding tasks to local LLMs. Options: "
                                             "--endpoint URL (repeatable), --ephemeral-endpoint URL (repeatable), --agent-script PATH, --model NAME, --timeout SECONDS, "
                                             "--min-context N, --max-diff-lines N, --allow-command PATTERN (repeatable), --command-timeout SECONDS, "
                                             "--log-dir PATH, --log-file PATH, --endpoints-file PATH, --listen ADDR:PORT, --listen-tls ADDR:PORT, --tls-cert PATH, "
                                             "--tls-key PATH, --token-file PATH, --allowed-host NAME (repeatable). Without --listen and --listen-tls it serves stdio. "
                                             "Environment: LOCAL_LLM_URLS, LOCAL_LLM_URL, LOCAL_LLM_EPHEMERAL_URLS, LOCAL_LLM_ENDPOINTS_FILE, LOCAL_LLM_API_KEY, "
                                             "LOCAL_LLM_MODEL, LOCAL_LLM_TIMEOUT, LOCAL_AGENT_SCRIPT" );
}

/*-------------------------------------------------------------------------*/

/** SIGTERM and SIGINT, the way a service manager stops the server: only the async-signal-safe stop request */
static void
StopSignalHandler( int signal )
{
    CORE::CGUCEFApplication::RequestStopFromSignal( signal );
}

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    GUCEF_OSMAIN_SIGNAL_HANDLER( StopSignalHandler );

    SOptions options;
    const bool optionsValid = ParseOptions( argc, argv, options );
    ApplyEnvironmentAndDefaults( options );

    // Standard output belongs to the protocol. The claim makes the console logger choose standard error, and never
    // a window of its own, even when the process has no terminal. The transport claims it again when it starts
    CORE::CStandardOutputWriter::ClaimExclusive();
    CORE::CPlatformNativeConsoleLogger console;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );
    CORE::CStandardOutputWriter::ReleaseClaim();

    CORE::CFileAccess* logFile = GUCEF_NULL;
    CORE::CStdLogger* fileLogger = GUCEF_NULL;
    if ( !options.logFile.IsNULLOrEmpty() )
    {
        logFile = GUCEF_NEW CORE::CFileAccess( options.logFile, "w" );
        fileLogger = GUCEF_NEW CORE::CStdLogger( *logFile );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( fileLogger );
    }
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    int exitCode = 0;
    CORE::CString optionsProblem;
    if ( !optionsValid || options.help )
    {
        PrintUsage();
        exitCode = options.help ? 0 : 2;
    }
    else
    if ( !CheckServingOptions( options, optionsProblem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpLocalLlm: " + optionsProblem );
        exitCode = 2;
    }
    else
    if ( !EnsureJsonCodec() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpLocalLlm: The JSON codec plugin dstorepluginJSONPARSER is not available" );
        exitCode = 3;
    }
    else
    {
        COM::CComGlobal::Instance();
        WEB::CWebGlobal::Instance();            // the HTTP client used to probe the LLM servers

        COM::CMcpServer server;
        COM::CMcpImplementationInfo info;
        info.name = "gucefMcpLocalLlm";
        info.title = "GUCEF local LLM delegation";
        info.version = "1.0.0";
        server.SetServerInfo( info );

        // the tools exist to be used: all are allowed, including those that edit files or run commands; what
        // run_and_digest may run is limited by --allow-command
        COM::CMcpConfigurableAccessPolicy* policy = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
        policy->GetDefaultRules().AddAllowTool( "delegate_to_local_llm" );
        policy->GetDefaultRules().AddAllowTool( "delegate_batch" );
        policy->GetDefaultRules().AddAllowTool( "run_and_digest" );
        policy->GetDefaultRules().AddAllowTool( "ask_local_llm" );
        policy->GetDefaultRules().AddAllowTool( "local_llm_status" );
        server.SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );

        GUCEFMcpLocalLlm::CLlmEndpointPool pool( options.endpoints, options.ephemeralEndpoints, CORE::GetEnv( "LOCAL_LLM_API_KEY" ), options.minContextSize );
        if ( !options.endpointsFile.IsNULLOrEmpty() )
            pool.SetEndpointsFile( options.endpointsFile );
        const bool servesHttp = !options.listen.IsNULLOrEmpty() || !options.listenTls.IsNULLOrEmpty();
        // without the coding agent the tools that delegate edits are not offered; the others work without it
        if ( !CORE::FileExists( options.agentScript ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpLocalLlm: The coding agent script \"" + options.agentScript +
                               "\" does not exist, so delegate_to_local_llm and delegate_batch are not offered. Pass --agent-script or set LOCAL_AGENT_SCRIPT" );
            options.agentScript = CORE::CString::Empty;
        }

        GUCEFMcpLocalLlm::SToolSettings toolSettings;
        toolSettings.agentScript = options.agentScript;
        toolSettings.model = options.model;
        toolSettings.agentTimeoutInSecs = options.timeoutInSecs;
        toolSettings.maxDiffLines = options.maxDiffLines;
        toolSettings.allowedCommands = options.allowedCommands;
        toolSettings.commandTimeoutInSecs = options.commandTimeoutInSecs;
        toolSettings.logDir = options.logDir;
        GUCEFMcpLocalLlm::RegisterTools( server, pool, toolSettings );

        // The instructions are always in the context of the client, unlike the tool definitions, which a client
        // may load only when it looks for them: they name the tools and say when each replaces the client's own work
        CORE::CString instructions = "Tools that do work on local LLMs and save your tokens; each call replaces several of your own steps. "
                                     "Prefer them over doing the same yourself:\n";
        const CORE::CString machineName = GetMachineName();
        if ( servesHttp && !machineName.IsNULLOrEmpty() )
            instructions += "- They run on the machine " + machineName + ": paths and commands are those of that machine.\n";
        if ( !toolSettings.allowedCommands.empty() )
        {
            CORE::CString patterns;
            for ( size_t i=0; i<toolSettings.allowedCommands.size(); ++i )
                patterns += ( i > 0 ? ", " : "" ) + toolSettings.allowedCommands[ i ];
            instructions += "- Builds and tests (" + patterns + "): run_and_digest, not your shell. It returns only the errors "
                            "and failed tests; grep its log file for more.\n";
        }
        if ( !toolSettings.agentScript.IsNULLOrEmpty() && !pool.IsEmpty() )
            instructions += "- Mechanical edits across many lines or files: delegate_to_local_llm, several independent ones at once: "
                            "delegate_batch. Review the change they return.\n";
        if ( !pool.IsEmpty() )
        {
            instructions += "- Questions that need many or long files read: ask_local_llm (wildcards like src/**/*.cpp). Check the cited lines.\n";

            // the models differ per server, each GPU holds one; which are up changes, local_llm_status tells
            std::vector< GUCEFMcpLocalLlm::SEndpointState > states;
            pool.ProbeAll( states );
            const CORE::CString models = pool.DescribeModels( states );
            instructions += "- Models: the servers may run different models. Pass model to route a call to one; without it any "
                            "server is used" + ( toolSettings.model.IsNULLOrEmpty() ? CORE::CString::Empty : ", preferring " + toolSettings.model ) +
                            ". Reachable at start: " + ( models.IsNULLOrEmpty() ? CORE::CString( "none" ) : models ) + "; local_llm_status lists them now.";
        }
        else
        {
            instructions += "- No LLM server is configured: run_and_digest works without one.";
        }
        server.SetInstructions( instructions );

        // tool calls run on the thread pool of the server: enough threads for a task per request slot of every
        // endpoint at the same time, plus status calls
        UInt32 threads = (UInt32) options.endpoints.size() * 2 + 2;
        if ( ( servesHttp || !options.endpointsFile.IsNULLOrEmpty() ) && threads < 8 )
            threads = 8;                        // several clients, and servers that come with the endpoints file
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( server.GetSettings().threadPoolName, true );
        if ( !threadPool.IsNULL() )
        {
            threadPool->SetDesiredMinNrOfWorkerThreads( threads );
            threadPool->SetDesiredMaxTotalNrOfThreads( (Int32) threads );
        }

        CORE::CString endpointList;
        for ( size_t i=0; i<options.endpoints.size(); ++i )
            endpointList += ( i > 0 ? ", " : "" ) + options.endpoints[ i ];
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpLocalLlm: LLM servers: " + endpointList + "; agent: " +
                          ( options.agentScript.IsNULLOrEmpty() ? CORE::CString( "none" ) : options.agentScript ) );

        // stdio for the one client that started this process; HTTP for any number of clients, each with the token: the
        // tools edit files and run commands on this machine. Plain HTTP on loopback only, HTTPS for other machines
        std::vector< COM::CIMcpTransportPtr > transports;
        CORE::CString transportProblem;
        if ( !servesHttp )
        {
            CORE::CDataNode transportConfig( GUCEF_DATATYPE_OBJECT );
            transportConfig.SetAttribute( "type", CORE::CVariant( "stdio" ) );
            COM::CIMcpTransportPtr transport = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "stdio" );
            if ( transport.IsNULL() || !transport->LoadConfig( transportConfig ) )
                transportProblem = "The stdio transport cannot be made";
            else
                transports.push_back( transport );
        }
        if ( transportProblem.IsNULLOrEmpty() && !options.listen.IsNULLOrEmpty() )
        {
            COM::CIMcpTransportPtr transport = MakeHttpTransport( options.listen, false, options, transportProblem );
            if ( !transport.IsNULL() )
                transports.push_back( transport );
        }
        if ( transportProblem.IsNULLOrEmpty() && !options.listenTls.IsNULLOrEmpty() )
        {
            COM::CIMcpTransportPtr transport = MakeHttpTransport( options.listenTls, true, options, transportProblem );
            if ( !transport.IsNULL() )
                transports.push_back( transport );
        }
        if ( !transportProblem.IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpLocalLlm: " + transportProblem );
            exitCode = 4;
        }
        else
        {
            for ( size_t i=0; i<transports.size(); ++i )
                server.AddTransport( transports[ i ] );
            CShutdownListener shutdownListener( server );
            if ( server.Start() )
            {
                exitCode = CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, true );
                server.Stop( true );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpLocalLlm: A transport did not start, see the log above" );
                exitCode = 4;
            }
        }
    }

    CORE::CLogStreamScope::FlushLogs();
    if ( GUCEF_NULL != fileLogger )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( fileLogger );
        GUCEF_DELETE fileLogger;
        GUCEF_DELETE logFile;
    }
    return exitCode;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
