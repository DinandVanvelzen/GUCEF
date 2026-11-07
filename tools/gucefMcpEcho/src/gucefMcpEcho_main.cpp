/*
 *  gucefMcpEcho: a small reference MCP server built on gucefCOM
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

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

#ifndef GUCEF_COM_CMCPJSONLINESAUDITLOG_H
#include "gucefCOM_CMcpJsonLinesAuditLog.h"
#define GUCEF_COM_CMCPJSONLINESAUDITLOG_H
#endif /* GUCEF_COM_CMCPJSONLINESAUDITLOG_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#include "gucefMcpEcho_Objects.h"

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
 *  The command line, in the form --name value or --name=value
 *
 *    --transport stdio|tcp|http  how clients connect, stdio by default; http is the Streamable HTTP transport
 *    --port N                tcp, http: the port, 0 (the default) picks a free one and logs it
 *    --token-file PATH       tcp, http: where the authentication token (http: the bearer token) is read from, or made
 *                            when the file is not there
 *    --no-token              tcp, http: do not require a token
 *    --allow-origin ORIGIN   http: an origin whose browser requests are allowed, http://localhost:6274 for example. Repeat for more
 *    --allow-tool PATTERN    names a tool that the access policy allows, add_note for example. Repeat for more
 *    --audit-log PATH        appends a JSON line per request to a rolling file
 *    --log-file PATH         also writes the log to a file; the log goes to standard error only otherwise
 *    --ignore-sigpipe        stdio: ignore SIGPIPE process wide
 *    --rate-limit N          tool calls per second and principal, 10 by default; 0: no limit (for benchmarks)
 *    --rate-burst N          the burst of the tool call rate limit, 20 by default
 *    --cycle-ms N            the shortest time between two pulses of the main loop, 1 by default (the framework's
 *                            default is 25); the server and its sockets ask for a pulse every 10 ms
 *    --help
 */
struct SOptions
{
    CORE::CString transport;
    UInt16 port;
    CORE::CString tokenFile;
    bool requireToken;
    CORE::CString::StringVector allowTools;
    CORE::CString::StringVector allowOrigins;
    CORE::CString auditLog;
    CORE::CString logFile;
    bool ignoreSigPipe;
    bool help;
    bool hasRateLimit;
    UInt32 rateLimit;
    bool hasRateBurst;
    UInt32 rateBurst;
    UInt32 cycleMs;

    SOptions( void ) : transport( "stdio" ), port( 0 ), tokenFile(), requireToken( true ), allowTools(), allowOrigins(), auditLog(), logFile(), ignoreSigPipe( false ), help( false ),
                       hasRateLimit( false ), rateLimit( 0 ), hasRateBurst( false ), rateBurst( 0 ), cycleMs( 1 ) {}
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
        const bool needsValue = arg == "--transport" || arg == "--port" || arg == "--token-file" || arg == "--allow-tool" || arg == "--allow-origin" || arg == "--audit-log" || arg == "--log-file" ||
                                arg == "--rate-limit" || arg == "--rate-burst" || arg == "--cycle-ms";
        if ( needsValue && value.IsNULLOrEmpty() )
        {
            if ( i+1 >= argc )
                return false;
            value = argv[ ++i ];
        }

        if ( arg == "--transport" )
            options.transport = value;
        else
        if ( arg == "--port" )
            options.port = CORE::StringToUInt16( value );
        else
        if ( arg == "--token-file" )
            options.tokenFile = value;
        else
        if ( arg == "--no-token" )
            options.requireToken = false;
        else
        if ( arg == "--allow-tool" )
            options.allowTools.push_back( value );
        else
        if ( arg == "--allow-origin" )
            options.allowOrigins.push_back( value );
        else
        if ( arg == "--audit-log" )
            options.auditLog = value;
        else
        if ( arg == "--log-file" )
            options.logFile = value;
        else
        if ( arg == "--ignore-sigpipe" )
            options.ignoreSigPipe = true;
        else
        if ( arg == "--rate-limit" )
        {
            options.hasRateLimit = true;
            options.rateLimit = CORE::StringToUInt32( value );
        }
        else
        if ( arg == "--rate-burst" )
        {
            options.hasRateBurst = true;
            options.rateBurst = CORE::StringToUInt32( value );
        }
        else
        if ( arg == "--cycle-ms" )
            options.cycleMs = CORE::StringToUInt32( value );
        else
        if ( arg == "--help" )
            options.help = true;
        else
        if ( arg == "--console" )
            continue;                           // read by the entry point of the framework
        else
            return false;
    }
    return options.transport == "stdio" || options.transport == "tcp" || options.transport == "http";
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
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "gucefMcpEcho", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "gucefMcpEcho", true );
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

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpEcho: Stopping" );
        CORE::CCoreGlobal::Instance()->GetApplication().Stop();
    }
};

/*-------------------------------------------------------------------------*/

static void
PrintUsage( void )
{
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpEcho: A reference MCP server of GUCEF. Options: --transport stdio|tcp|http, --port N, --token-file PATH, --no-token, "
                                             "--allow-tool PATTERN, --allow-origin ORIGIN, --audit-log PATH, --log-file PATH, --ignore-sigpipe, --rate-limit N, --rate-burst N, --cycle-ms N" );
}

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    SOptions options;
    const bool optionsValid = ParseOptions( argc, argv, options );

    // Standard output belongs to the protocol in stdio mode. The claim makes the console logger choose standard
    // error, and never a window of its own, even when the process has no terminal. The transport claims it again when it starts
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
    if ( !optionsValid || options.help )
    {
        PrintUsage();
        exitCode = options.help ? 0 : 2;
    }
    else
    if ( !EnsureJsonCodec() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpEcho: The JSON codec plugin dstorepluginJSONPARSER is not available" );
        exitCode = 3;
    }
    else
    {
        COM::CComGlobal::Instance();

        // gucefWEB registers the Streamable HTTP transport in the transport factory of gucefCOM
        if ( options.transport == "http" )
            WEB::CWebGlobal::Instance();

        COM::CMcpServer server;
        COM::CMcpImplementationInfo info;
        info.name = "gucefMcpEcho";
        info.title = "GUCEF reference MCP server";
        info.version = "1.0.0";
        server.SetServerInfo( info );
        server.SetInstructions( "A reference server with demo tools, a resource and a prompt, for conformance runs." );
        if ( options.hasRateLimit || options.hasRateBurst )
        {
            COM::CMcpServerSettings settings = server.GetSettings();
            if ( options.hasRateLimit )
                settings.toolCallsPerSecond = options.rateLimit;
            if ( options.hasRateBurst )
                settings.toolCallBurst = options.rateBurst;
            server.SetSettings( settings );
        }

        // a request is taken in on a pulse of the main loop: the shorter the cycle, the lower the latency (section 7)
        CORE::CCoreGlobal::Instance()->GetApplication().SetForcedMinimalCycleDeltaInMilliSecs( options.cycleMs );

        // the policy: read-only tools are allowed and the rest is hidden until it is named
        COM::CMcpConfigurableAccessPolicy* configurablePolicy = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
        for ( size_t i=0; i<options.allowTools.size(); ++i )
            configurablePolicy->GetDefaultRules().AddAllowTool( options.allowTools[ i ] );
        server.SetAccessPolicy( COM::CIMcpAccessPolicyPtr( configurablePolicy ) );

        GUCEFMcpEcho::CNoteStore notes;
        GUCEFMcpEcho::RegisterDemoObjects( server, notes );

        COM::CMcpJsonLinesAuditLog auditLog;
        if ( !options.auditLog.IsNULLOrEmpty() )
        {
            if ( auditLog.OpenRollingFile( options.auditLog, 1048576, 5 ) )
                auditLog.Attach( server );
            else
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpEcho: The audit log could not be opened" );
        }

        // the transport comes from the factory of the module and is set up the way a configuration file does it
        const CORE::CString transportType = options.transport == "http" ? COM::CIMcpTransport::StreamableHttpTransportName : options.transport;
        CORE::CDataNode transportConfig( GUCEF_DATATYPE_OBJECT );
        transportConfig.SetAttribute( "type", CORE::CVariant( transportType ) );
        if ( options.transport == "http" )
        {
            transportConfig.SetAttribute( "port", (UInt32) options.port );
            transportConfig.SetAttribute( "authenticator", CORE::CVariant( options.requireToken ? "static-bearer" : "none" ) );
            transportConfig.SetAttribute( "bearerTokenFile", CORE::CVariant( options.tokenFile ) );
            CORE::CString origins;
            for ( size_t i=0; i<options.allowOrigins.size(); ++i )
            {
                if ( i > 0 )
                    origins += ',';
                origins += options.allowOrigins[ i ];
            }
            transportConfig.SetAttribute( "allowedOrigins", CORE::CVariant( origins ) );
        }
        else
        if ( options.transport == "tcp" )
        {
            transportConfig.SetAttribute( "port", (UInt32) options.port );
            transportConfig.SetAttribute( "requireToken", options.requireToken );
            transportConfig.SetAttribute( "tokenFile", CORE::CVariant( options.tokenFile ) );
        }
        else
        {
            transportConfig.SetAttribute( "ignoreSigPipe", options.ignoreSigPipe );
        }
        COM::CIMcpTransportPtr transport = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( transportType );
        if ( transport.IsNULL() || !transport->LoadConfig( transportConfig ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpEcho: The \"" << options.transport << "\" transport cannot be made" );
            return 4;
        }
        server.AddTransport( transport );

        CShutdownListener shutdownListener( server );
        if ( server.Start() )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpEcho: Serving on " << transport->GetEndpointDescription() );
            exitCode = CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, true );
            server.Stop( true );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpEcho: The transport did not start" );
            exitCode = 4;
        }
        auditLog.Detach();
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
