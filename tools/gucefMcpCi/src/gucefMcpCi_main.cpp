/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_COREECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_COREECODECTYPES_H
#endif /* GUCEF_CORE_COREECODECTYPES_H ? */

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

#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */

#ifndef GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H
#include "gucefWEB_CMcpStaticBearerAuthenticator.h"
#define GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H
#endif /* GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H ? */

#ifndef GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#include "gucefWEB_CMcpStreamableHttpTransport.h"
#define GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#endif /* GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <unistd.h>
#endif

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiRegistry.h"
#include "gucefMcpCi_CSnapshotBuilder.h"
#include "gucefMcpCi_CForgejoBackend.h"
#include "gucefMcpCi_CDirectCiService.h"
#include "gucefMcpCi_CRemoteCiService.h"
#include "gucefMcpCi_CUploadStore.h"
#include "gucefMcpCi_CBundleReceiver.h"
#include "gucefMcpCi_CGatewayTokens.h"
#include "gucefMcpCi_CGatewayApi.h"
#include "gucefMcpCi_CCiGateway.h"
#include "gucefMcpCi_CKubectlDeployer.h"
#include "gucefMcpCi_Tools.h"

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
 *  The command line, in the form --name value or --name=value:
 *
 *    --mode MODE           auto (default): proxy when a gateway URL and a gateway token are there, else direct when a
 *                          CI registry and a CI token are there, else no CI tools. direct: this server works with the CI
 *                          itself. proxy: every call goes to a gateway, this machine holds no CI credentials. gateway:
 *                          serves the gateway API (HTTPS, bearer tokens) over the CI of this machine, for proxies
 *    --ci-config FILE      the CI registry (JSON, see CCiRegistry); default: ci.json of the lablink directory
 *    --forgejo-token-file FILE  the API token for Forgejo, readable by the owner only; default: forgejo.token of the
 *                          lablink directory. Never give a token on the command line
 *    --lablink-ci-dir DIR  where lablink puts ci.json, gateway.token, ca.crt and forgejo.token; default
 *                          /var/lib/gucef-mcp/ci on Linux and %ProgramData%\gucef-mcp\ci on Windows when it exists;
 *                          "none" for no such directory
 *    --gateway-url URL     proxy: the gateway, https://host:port; default: "gateway"."url" of ci.json
 *    --gateway-token-file FILE  proxy: the token of this machine for the gateway, owner only; default: gateway.token of
 *                          the lablink directory
 *    --ca-file FILE        PEM file with the authorities to trust: for the gateway in proxy mode (default:
 *                          "gateway"."caFile" of ci.json, else ca.crt of the lablink directory), for an https CI
 *                          otherwise; default: the system trust store
 *    --reports-owner NAME  the Forgejo user or organisation that owns the report packages (ci-reports) of CI ids whose
 *                          registry entry has no reportsOwner; default ci-bot
 *    --log-dir DIR         where the logs of jobs are kept and snapshots are prepared; default: gucefMcpCi in the
 *                          temporary directory
 *    --log-file FILE       also writes the log to a file; the log goes to standard error only otherwise
 *    --max-snapshot-mb N   the largest snapshot that is sent, or that a gateway takes; default 500
 *    --machine-name NAME   part of the names of the scratch branches; default: the host name
 *    --max-wait-seconds N  the longest wait of ci_status; default 600
 *    --poll-interval-ms N  how often a waiting ci_status asks the CI; default 5000
 *    --stdio               serves MCP over standard input and output; the default without --listen and --listen-tls
 *    --listen ADDR:PORT    serves MCP over HTTP (streamable HTTP, path /mcp) on a loopback address; gateway: the
 *                          gateway API over plain HTTP, on a loopback address only
 *    --listen-tls ADDR[:PORT]  serves MCP over HTTPS, with --tls-cert and --tls-key, for clients on other machines;
 *                          gateway: the gateway API over HTTPS, default port 8792
 *    --tls-cert PATH       PEM certificate of this machine, followed by its chain
 *    --tls-key PATH        PEM private key of the certificate
 *    --token-file PATH     the bearer token HTTP clients send; made for the owner only when it does not exist.
 *                          Required with --listen and --listen-tls. gateway: an optional static token (machine "admin")
 *    --allowed-host NAME   a name clients of --listen-tls use for this machine. Repeat for more
 *
 *  Deployments (direct and gateway):
 *    --kubeconfig FILE     the kubeconfig of the restricted ServiceAccount of the deployer; default
 *                          /var/lib/gucef-ci/kubeconfig. The environment variable KUBECONFIG is not used
 *    --kubectl PATH        the kubectl executable; default kubectl
 *    --allow-deploy        direct: the tools may deploy and roll back (scope deploy); without it scope build only.
 *                          A gateway takes the scopes from the tokens file
 *    --deploy-timeout-seconds N  the longest wait for a rollout; default 600
 *    --registry-token-file FILE  the token for the container registry (read:package), owner only; default: the CI token
 *    --work-dir DIR        also in direct mode: the bare repositories per CI id and the audit of deployments
 *
 *  Gateway only:
 *    --gateway-tokens-file FILE  the tokens of the machines: lines "<sha256 hex of the token><TAB><machine id>", owner
 *                          only, read again when it changes; the hub of lablink keeps it. Default
 *                          /var/lib/gucef-lablink/ci/gateway-tokens; "none" for no such file (then --token-file only)
 *    --work-dir DIR        the bare repositories per CI id and the uploads; default: gateway in --log-dir
 *    --upload-chunk-kb N   the chunk size of uploads, at most 8192 (default)
 *    --upload-idle-seconds N  an upload without a chunk for this long is removed; default 600
 *    --max-uploads N       uploads at the same time; default 4
 *    --max-pending-requests N  requests that wait or run; default 64
 *    --max-pending-polls N  statuses that wait for their run; default 32
 *    --gateway-threads N   threads of the gateway; default 6
 *    --max-requests-per-minute N  per machine; default 600, 0 for no limit
 *    --help
 */
struct SOptions
{
    CORE::CString mode;
    CORE::CString ciConfig;
    CORE::CString forgejoTokenFile;
    CORE::CString lablinkCiDir;
    CORE::CString gatewayUrl;
    CORE::CString gatewayTokenFile;
    CORE::CString caFile;
    CORE::CString reportsOwner;
    CORE::CString logDir;
    CORE::CString logFile;
    UInt32 maxSnapshotMb;
    CORE::CString machineName;
    UInt32 maxWaitSeconds;
    UInt32 pollIntervalInMs;
    bool stdio;
    CORE::CString listen;
    CORE::CString listenTls;
    CORE::CString tlsCert;
    CORE::CString tlsKey;
    CORE::CString tokenFile;
    CORE::CString::StringVector allowedHosts;
    CORE::CString gatewayTokensFile;
    CORE::CString workDir;
    UInt32 uploadChunkKb;
    UInt32 uploadIdleSeconds;
    UInt32 maxUploads;
    UInt32 maxPendingRequests;
    UInt32 maxPendingPolls;
    UInt32 gatewayThreads;
    UInt32 maxRequestsPerMinute;
    CORE::CString kubeconfig;
    CORE::CString kubectl;
    bool allowDeploy;
    UInt32 deployTimeoutSeconds;
    CORE::CString registryTokenFile;
    bool help;

    SOptions( void ) : mode( "auto" ), ciConfig(), forgejoTokenFile(), lablinkCiDir(), gatewayUrl(), gatewayTokenFile(), caFile(), reportsOwner(), logDir(), logFile(),
                       maxSnapshotMb( 500 ), machineName(), maxWaitSeconds( 600 ), pollIntervalInMs( 5000 ), stdio( false ), listen(), listenTls(), tlsCert(), tlsKey(),
                       tokenFile(), allowedHosts(), gatewayTokensFile(), workDir(), uploadChunkKb( 8192 ), uploadIdleSeconds( 600 ), maxUploads( 4 ),
                       maxPendingRequests( 64 ), maxPendingPolls( 32 ), gatewayThreads( 6 ), maxRequestsPerMinute( 600 ),
                       kubeconfig(), kubectl(), allowDeploy( false ), deployTimeoutSeconds( 600 ), registryTokenFile(), help( false ) {}
};

/*-------------------------------------------------------------------------*/

static bool
ParseOptions( int argc, char* argv[], SOptions& options )
{
    static const char* valueOptions[] = { "--mode", "--ci-config", "--forgejo-token-file", "--lablink-ci-dir", "--gateway-url", "--gateway-token-file",
                                          "--ca-file", "--reports-owner", "--log-dir", "--log-file", "--max-snapshot-mb", "--machine-name", "--max-wait-seconds",
                                          "--poll-interval-ms", "--listen", "--listen-tls", "--tls-cert", "--tls-key", "--token-file", "--allowed-host",
                                          "--gateway-tokens-file", "--work-dir", "--upload-chunk-kb", "--upload-idle-seconds", "--max-uploads",
                                          "--max-pending-requests", "--max-pending-polls", "--gateway-threads", "--max-requests-per-minute",
                                          "--kubeconfig", "--kubectl", "--deploy-timeout-seconds", "--registry-token-file" };
    for ( int i=1; i<argc; ++i )
    {
        CORE::CString arg = argv[ i ];
        CORE::CString value;
        bool hasValue = false;
        const char* equals = ::strchr( argv[ i ], '=' );
        if ( arg.Length() > 2 && 0 == ::strncmp( argv[ i ], "--", 2 ) && GUCEF_NULL != equals )
        {
            value = equals + 1;
            arg = CORE::CString( argv[ i ], (UInt32) ( equals - argv[ i ] ) );
            hasValue = true;
        }
        bool needsValue = false;
        for ( size_t v=0; v<sizeof( valueOptions ) / sizeof( valueOptions[ 0 ] ) && !needsValue; ++v )
            needsValue = arg == valueOptions[ v ];
        if ( needsValue && !hasValue )
        {
            if ( i+1 >= argc )
                return false;
            value = argv[ ++i ];
        }

        if ( arg == "--mode" )
            options.mode = value.Lowercase();
        else
        if ( arg == "--ci-config" )
            options.ciConfig = value;
        else
        if ( arg == "--forgejo-token-file" )
            options.forgejoTokenFile = value;
        else
        if ( arg == "--lablink-ci-dir" )
            options.lablinkCiDir = value;
        else
        if ( arg == "--gateway-url" )
            options.gatewayUrl = value;
        else
        if ( arg == "--gateway-token-file" )
            options.gatewayTokenFile = value;
        else
        if ( arg == "--ca-file" )
            options.caFile = value;
        else
        if ( arg == "--reports-owner" )
            options.reportsOwner = value;
        else
        if ( arg == "--log-dir" )
            options.logDir = value;
        else
        if ( arg == "--log-file" )
            options.logFile = value;
        else
        if ( arg == "--max-snapshot-mb" )
            options.maxSnapshotMb = CORE::StringToUInt32( value, 500 );
        else
        if ( arg == "--machine-name" )
            options.machineName = value;
        else
        if ( arg == "--max-wait-seconds" )
            options.maxWaitSeconds = CORE::StringToUInt32( value, 600 );
        else
        if ( arg == "--poll-interval-ms" )
            options.pollIntervalInMs = CORE::StringToUInt32( value, 5000 );
        else
        if ( arg == "--stdio" )
            options.stdio = true;
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
        if ( arg == "--gateway-tokens-file" )
            options.gatewayTokensFile = value;
        else
        if ( arg == "--work-dir" )
            options.workDir = value;
        else
        if ( arg == "--upload-chunk-kb" )
            options.uploadChunkKb = CORE::StringToUInt32( value, 8192 );
        else
        if ( arg == "--upload-idle-seconds" )
            options.uploadIdleSeconds = CORE::StringToUInt32( value, 600 );
        else
        if ( arg == "--max-uploads" )
            options.maxUploads = CORE::StringToUInt32( value, 4 );
        else
        if ( arg == "--max-pending-requests" )
            options.maxPendingRequests = CORE::StringToUInt32( value, 64 );
        else
        if ( arg == "--max-pending-polls" )
            options.maxPendingPolls = CORE::StringToUInt32( value, 32 );
        else
        if ( arg == "--gateway-threads" )
            options.gatewayThreads = CORE::StringToUInt32( value, 6 );
        else
        if ( arg == "--max-requests-per-minute" )
            options.maxRequestsPerMinute = CORE::StringToUInt32( value, 600 );
        else
        if ( arg == "--kubeconfig" )
            options.kubeconfig = value;
        else
        if ( arg == "--kubectl" )
            options.kubectl = value;
        else
        if ( arg == "--allow-deploy" )
            options.allowDeploy = true;
        else
        if ( arg == "--deploy-timeout-seconds" )
            options.deployTimeoutSeconds = CORE::StringToUInt32( value, 600 );
        else
        if ( arg == "--registry-token-file" )
            options.registryTokenFile = value;
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

/** Fills in what the command line left out */
static void
ApplyDefaults( SOptions& options )
{
    if ( options.logDir.IsNULLOrEmpty() )
        options.logDir = CORE::CombinePath( CORE::TempDir(), "gucefMcpCi" );
    options.logDir = GUCEFMcpCi::CCiUtils::MakeAbsolute( options.logDir );
    if ( options.machineName.IsNULLOrEmpty() )
        options.machineName = GetMachineName();
    if ( 0 == options.maxSnapshotMb )
        options.maxSnapshotMb = 500;
    if ( 0 == options.maxWaitSeconds )
        options.maxWaitSeconds = 600;
    if ( options.lablinkCiDir.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        const CORE::CString programData = CORE::GetEnv( "ProgramData" );
        const CORE::CString standard = programData.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CombinePath( programData, "gucef-mcp\\ci" );
        #else
        const CORE::CString standard = "/var/lib/gucef-mcp/ci";
        #endif
        if ( !standard.IsNULLOrEmpty() && CORE::DirExists( standard ) )
            options.lablinkCiDir = standard;
    }
    if ( options.lablinkCiDir == "none" )
        options.lablinkCiDir = CORE::CString::Empty;
    if ( options.mode == "gateway" && options.gatewayTokensFile.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        const CORE::CString programData = CORE::GetEnv( "ProgramData" );
        if ( !programData.IsNULLOrEmpty() )
            options.gatewayTokensFile = CORE::CombinePath( programData, "gucef-lablink\\ci\\gateway-tokens" );
        #else
        options.gatewayTokensFile = "/var/lib/gucef-lablink/ci/gateway-tokens";
        #endif
    }
    if ( options.gatewayTokensFile == "none" )
        options.gatewayTokensFile = CORE::CString::Empty;
    if ( options.workDir.IsNULLOrEmpty() )
        options.workDir = CORE::CombinePath( options.logDir, "gateway" );
    options.workDir = GUCEFMcpCi::CCiUtils::MakeAbsolute( options.workDir );
    if ( 0 == options.uploadChunkKb || options.uploadChunkKb > 8192 )
        options.uploadChunkKb = 8192;
    if ( options.kubeconfig.IsNULLOrEmpty() )
        options.kubeconfig = GUCEFMcpCi::CKubectlDeployer::DefaultKubeconfigPath;
    if ( options.kubectl.IsNULLOrEmpty() )
        options.kubectl = "kubectl";
    if ( 0 == options.deployTimeoutSeconds || options.deployTimeoutSeconds > 3600 )
        options.deployTimeoutSeconds = 600;
    // the gateway listens on its default port when none is given
    if ( options.mode == "gateway" && !options.listenTls.IsNULLOrEmpty() && options.listenTls.HasChar( ':', false ) < 0 )
        options.listenTls += ":8792";
}

/*-------------------------------------------------------------------------*/

/** ADDRESS:PORT as its parts. @return false when it is not that */
static bool
SplitListen( const CORE::CString& listen, CORE::CString& address, UInt16& port )
{
    const Int32 colon = listen.HasChar( ':', false );
    address = colon > 0 ? listen.SubstrToIndex( (UInt32) colon, true ) : CORE::CString::Empty;
    const UInt32 number = colon > 0 ? CORE::StringToUInt32( listen.CutChars( (UInt32) colon + 1, true ) ) : 0;
    port = (UInt16) number;
    return !address.IsNULLOrEmpty() && number > 0 && number <= 65535;
}

/*-------------------------------------------------------------------------*/

/** Whether the options are usable together, problem says why not */
static bool
CheckServingOptions( const SOptions& options, CORE::CString& problem )
{
    if ( options.mode != "auto" && options.mode != "direct" && options.mode != "proxy" && options.mode != "gateway" )
    {
        problem = "--mode " + options.mode + " is not known: auto, direct, proxy or gateway";
        return false;
    }
    if ( options.mode == "gateway" )
    {
        if ( options.listen.IsNULLOrEmpty() == options.listenTls.IsNULLOrEmpty() )
        {
            problem = "--mode gateway serves on one address: --listen-tls ADDR:PORT (with --tls-cert and --tls-key), or --listen 127.0.0.1:PORT for this machine only";
            return false;
        }
        CORE::CString address;
        UInt16 port = 0;
        if ( !SplitListen( options.listen.IsNULLOrEmpty() ? options.listenTls : options.listen, address, port ) )
        {
            problem = "\"" + ( options.listen.IsNULLOrEmpty() ? options.listenTls : options.listen ) + "\" is not ADDRESS:PORT";
            return false;
        }
        if ( !options.listen.IsNULLOrEmpty() && address != "127.0.0.1" )
        {
            problem = "--listen of the gateway is plain HTTP and only for 127.0.0.1: use --listen-tls for other machines";
            return false;
        }
        if ( !options.listenTls.IsNULLOrEmpty() && ( options.tlsCert.IsNULLOrEmpty() || options.tlsKey.IsNULLOrEmpty() ) )
        {
            problem = "--listen-tls needs --tls-cert and --tls-key";
            return false;
        }
        if ( options.gatewayTokensFile.IsNULLOrEmpty() && options.tokenFile.IsNULLOrEmpty() )
        {
            problem = "--mode gateway needs --gateway-tokens-file (the tokens of the machines) or --token-file: nobody would be accepted";
            return false;
        }
        return true;
    }
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

/** Reads a token from a file that only its owner may read. @return false with the reason */
static bool
ReadToken( const CORE::CString& path, CORE::CString& token, CORE::CString& problem )
{
    CORE::CDynamicBuffer content;
    CORE::CString readProblem;
    if ( !CORE::ReadOwnerOnlyFile( path, content, readProblem ) )
    {
        problem = "the token file \"" + path + "\" cannot be used: " + readProblem;
        return false;
    }
    token = GUCEFMcpCi::CCiUtils::Trimmed( CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) );
    if ( token.IsNULLOrEmpty() )
    {
        problem = "the token file \"" + path + "\" is empty";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** What the configuration gives: the registry, the credentials, and what each mode would miss */
struct SConfiguration
{
    GUCEFMcpCi::CCiRegistry registry;
    CORE::CString forgejoToken;
    CORE::CString gatewayUrl;
    CORE::CString gatewayToken;
    CORE::CString gatewayCaFile;
    CORE::CString registryToken;
    CORE::CString missingDirect;        /**< what direct mode lacks, empty when nothing */
    CORE::CString missingProxy;         /**< what proxy mode lacks, empty when nothing */

    SConfiguration( void ) : registry(), forgejoToken(), gatewayUrl(), gatewayToken(), gatewayCaFile(), registryToken(), missingDirect(), missingProxy() {}
};

/*-------------------------------------------------------------------------*/

/**
 *  The CI registry, the tokens and the gateway: the explicit options and files first, then those lablink provides.
 *  @return false when a file is there but cannot be used, which always stops the server
 */
static bool
LoadConfiguration( const SOptions& options, SConfiguration& config, CORE::CString& fatal )
{
    const CORE::CString lablink = options.lablinkCiDir;
    if ( !options.ciConfig.IsNULLOrEmpty() )
    {
        if ( !config.registry.LoadFromFile( options.ciConfig, false, fatal ) )
            return false;
    }
    else
    if ( !lablink.IsNULLOrEmpty() && CORE::FileExists( CORE::CombinePath( lablink, "ci.json" ) ) )
    {
        if ( !config.registry.LoadFromFile( CORE::CombinePath( lablink, "ci.json" ), true, fatal ) )
            return false;
    }

    if ( !options.forgejoTokenFile.IsNULLOrEmpty() )
    {
        if ( !ReadToken( options.forgejoTokenFile, config.forgejoToken, fatal ) )
            return false;
    }
    else
    if ( !lablink.IsNULLOrEmpty() && CORE::FileExists( CORE::CombinePath( lablink, "forgejo.token" ) ) )
    {
        if ( !ReadToken( CORE::CombinePath( lablink, "forgejo.token" ), config.forgejoToken, fatal ) )
            return false;
    }

    // the container registry: its own token, or the CI token, which has read:package
    if ( !options.registryTokenFile.IsNULLOrEmpty() )
    {
        if ( !ReadToken( options.registryTokenFile, config.registryToken, fatal ) )
            return false;
    }
    else
        config.registryToken = config.forgejoToken;

    // the gateway of proxy mode
    config.gatewayUrl = GUCEFMcpCi::CCiUtils::Trimmed( options.gatewayUrl );
    config.gatewayCaFile = config.registry.GetGateway().caFile;
    if ( config.gatewayUrl.IsNULLOrEmpty() )
        config.gatewayUrl = config.registry.GetGateway().url;
    if ( !config.gatewayUrl.IsNULLOrEmpty() )
    {
        CORE::CString urlProblem;
        if ( !GUCEFMcpCi::CCiGatewayEndpoint::IsAcceptableUrl( config.gatewayUrl, urlProblem ) )
        {
            fatal = "the gateway URL cannot be used: " + urlProblem;
            return false;
        }
    }
    if ( !options.gatewayTokenFile.IsNULLOrEmpty() )
    {
        if ( !ReadToken( options.gatewayTokenFile, config.gatewayToken, fatal ) )
            return false;
    }
    else
    if ( !lablink.IsNULLOrEmpty() && CORE::FileExists( CORE::CombinePath( lablink, "gateway.token" ) ) )
    {
        if ( !ReadToken( CORE::CombinePath( lablink, "gateway.token" ), config.gatewayToken, fatal ) )
            return false;
    }
    if ( !options.caFile.IsNULLOrEmpty() )
        config.gatewayCaFile = options.caFile;
    else
    if ( config.gatewayCaFile.IsNULLOrEmpty() && !lablink.IsNULLOrEmpty() && CORE::FileExists( CORE::CombinePath( lablink, "ca.crt" ) ) )
        config.gatewayCaFile = CORE::CombinePath( lablink, "ca.crt" );

    const CORE::CString where = lablink.IsNULLOrEmpty() ? CORE::CString::Empty : " in " + lablink;
    if ( config.registry.IsEmpty() )
        config.missingDirect = "no CI registry: pass --ci-config, or let lablink provide ci.json" + where;
    else
    if ( config.forgejoToken.IsNULLOrEmpty() )
        config.missingDirect = "no CI token: pass --forgejo-token-file, or let lablink provide forgejo.token" + where;
    if ( config.gatewayUrl.IsNULLOrEmpty() )
        config.missingProxy = "no CI gateway: pass --gateway-url, or let lablink provide ci.json with a gateway" + where;
    else
    if ( config.gatewayToken.IsNULLOrEmpty() )
        config.missingProxy = "no token for the CI gateway: pass --gateway-token-file, or let lablink provide gateway.token" + where;
    return true;
}

/*-------------------------------------------------------------------------*/

/** The streamable HTTP transport on ADDR:PORT, with the bearer token of the token file; with TLS for other machines */
static COM::CIMcpTransportPtr
MakeHttpTransport( const CORE::CString& listen, bool tls, const SOptions& options, CORE::CString& problem )
{
    CORE::CString address;
    UInt16 port = 0;
    if ( !SplitListen( listen, address, port ) )
    {
        problem = "\"" + listen + "\" is not ADDRESS:PORT";
        return COM::CIMcpTransportPtr();
    }

    WEB::CMcpHttpTransportSettings settings;
    settings.bindAddress = address;
    settings.port = port;
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

/** Loads a codec plugin from next to the executable, the debug build first in a debug build */
static void
LoadPlugin( const CORE::CString& pluginName )
{
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    const CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    const CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    const CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "gucefMcpCi", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "gucefMcpCi", true );
}

/*-------------------------------------------------------------------------*/

static bool
EnsureJsonCodec( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return true;
    LoadPlugin( "dstorepluginJSONPARSER" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/*-------------------------------------------------------------------------*/

/** The gzip codec for the compressed logs of jobs; without it the uncompressed logs are fetched */
static bool
EnsureGzipCodec( void )
{
    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    if ( !registry.TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "gzip" ).IsNULL() )
        return true;
    LoadPlugin( "codecspluginZLIB" );
    return !registry.TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "gzip" ).IsNULL();
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

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Stopping" );
        CORE::CCoreGlobal::Instance()->GetApplication().Stop();
    }
};

/*-------------------------------------------------------------------------*/

static void
PrintUsage( void )
{
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: An MCP server that hands the build matrix of a repository to the lab CI. Options: "
                                             "--mode auto|direct|proxy|gateway, --ci-config FILE, --forgejo-token-file FILE, --lablink-ci-dir DIR|none, "
                                             "--gateway-url URL, --gateway-token-file FILE, --ca-file FILE, --reports-owner NAME, "
                                             "--log-dir DIR, --log-file FILE, --max-snapshot-mb N, --machine-name NAME, --max-wait-seconds N, "
                                             "--poll-interval-ms N, --stdio, --listen ADDR:PORT, --listen-tls ADDR[:PORT], --tls-cert PATH, --tls-key PATH, "
                                             "--token-file PATH, --allowed-host NAME (repeatable). Gateway: --gateway-tokens-file FILE, --work-dir DIR, "
                                             "--upload-chunk-kb N, --upload-idle-seconds N, --max-uploads N, --max-pending-requests N, --max-pending-polls N, "
                                             "--gateway-threads N, --max-requests-per-minute N. Deployments: --kubeconfig FILE, --kubectl PATH, --allow-deploy, "
                                             "--deploy-timeout-seconds N, --registry-token-file FILE. Without --listen and --listen-tls it serves stdio." );
}

/*-------------------------------------------------------------------------*/

/** SIGTERM and SIGINT, the way a service manager stops the server: only the async-signal-safe stop request */
static void
StopSignalHandler( int signal )
{
    CORE::CGUCEFApplication::RequestStopFromSignal( signal );
}

/*-------------------------------------------------------------------------*/

/** The Forgejo backend of a service that works with the CI itself */
static void
AddForgejoBackend( GUCEFMcpCi::CDirectCiService& service, const GUCEFMcpCi::CSnapshotBuilder& snapshotBuilder, const SOptions& options, const CORE::CString& token )
{
    GUCEFMcpCi::CForgejoBackend* forgejo = GUCEF_NEW GUCEFMcpCi::CForgejoBackend();
    forgejo->SetToken( token );
    forgejo->SetCaBundlePath( options.caFile );
    forgejo->SetDefaultReportsOwner( options.reportsOwner );
    forgejo->SetSnapshotPusher( &snapshotBuilder );
    service.AddBackend( forgejo );
}

/*-------------------------------------------------------------------------*/

/** The deployments of a service that works with the CI itself: kubectl, the registry, the audit, the bare repositories */
static void
SetUpDeployments( GUCEFMcpCi::CDirectCiService& service, const SOptions& options, const SConfiguration& config )
{
    service.SetRepositoryDirectory( CORE::CombinePath( options.workDir, "repos" ) );
    GUCEFMcpCi::CKubectlDeployer* deployer = GUCEF_NEW GUCEFMcpCi::CKubectlDeployer();
    deployer->SetKubectl( options.kubectl );
    deployer->SetKubeconfig( options.kubeconfig );
    deployer->AddSecret( config.forgejoToken );
    deployer->AddSecret( config.registryToken );
    deployer->AddSecret( config.gatewayToken );
    GUCEFMcpCi::CCiDeployManager& deployments = service.GetDeployManager();
    deployments.SetDeployer( deployer );
    deployments.SetRolloutTimeout( options.deployTimeoutSeconds );
    deployments.GetAudit().SetDirectory( CORE::CombinePath( options.workDir, "audit" ) );
    deployments.GetAudit().AddSecret( config.forgejoToken );
    deployments.GetAudit().AddSecret( config.registryToken );
    deployments.GetAudit().AddSecret( config.gatewayToken );
    deployments.GetRegistryClient().SetToken( config.registryToken );
    deployments.GetRegistryClient().SetCaBundlePath( options.caFile );
    if ( !config.registryToken.IsNULLOrEmpty() )
        WEB::CHttpLogRedaction::AddSecret( config.registryToken );

    bool anyDeployable = false;
    const GUCEFMcpCi::CCiRegistry::TDefinitionVector& definitions = config.registry.GetDefinitions();
    for ( size_t i=0; i<definitions.size(); ++i )
        anyDeployable = anyDeployable || !definitions[ i ].deployImages.empty();
    if ( anyDeployable && !CORE::FileExists( options.kubeconfig ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: The registry has deploy blocks but there is no kubeconfig at " << options.kubeconfig <<
                           " (--kubeconfig): deployments are refused until the setup of the deployer has written it" );
}

/*-------------------------------------------------------------------------*/

/** Mode gateway: the gateway API over the CI of this machine, until the application is asked to stop */
static int
RunGateway( const SOptions& options, const SConfiguration& config, int argc, char* argv[] )
{
    if ( !EnsureGzipCodec() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: The gzip codec (codecspluginZLIB) is not available: the uncompressed logs of jobs are fetched instead" );
    COM::CComGlobal::Instance();
    WEB::CWebGlobal::Instance();
    WEB::CHttpLogRedaction::AddSecret( config.forgejoToken );

    GUCEFMcpCi::CSnapshotBuilder snapshotBuilder;
    snapshotBuilder.SetMaxBytes( (UInt64) options.maxSnapshotMb * 1024 * 1024 );
    snapshotBuilder.SetWorkDir( CORE::CombinePath( options.workDir, "snapshots" ) );
    GUCEFMcpCi::CDirectCiService service( config.registry, snapshotBuilder );
    service.SetMachineName( options.machineName );
    service.SetPollInterval( options.pollIntervalInMs );
    AddForgejoBackend( service, snapshotBuilder, options, config.forgejoToken );
    SetUpDeployments( service, options, config );

    CORE::CString problem;
    GUCEFMcpCi::CUploadStore uploads;
    uploads.SetDirectory( CORE::CombinePath( options.workDir, "uploads" ) );
    uploads.SetLimits( options.maxUploads, (UInt64) options.maxSnapshotMb * 1024 * 1024, options.uploadChunkKb * 1024, options.uploadIdleSeconds );
    if ( !uploads.Start( problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: " << problem );
        return 4;
    }
    GUCEFMcpCi::CBundleReceiver receiver( service );
    receiver.SetDirectory( CORE::CombinePath( options.workDir, "repos" ) );
    CORE::CreateDirs( CORE::CombinePath( options.workDir, "repos" ) );
    GUCEFMcpCi::CGatewayApi api( service, uploads, receiver );
    api.SetMaxWaitSeconds( options.maxWaitSeconds );

    // the tokens of the machines, read again when lablink changes them; and the static token of the administrator
    GUCEFMcpCi::CGatewayTokens tokens;
    if ( !options.gatewayTokensFile.IsNULLOrEmpty() && !tokens.SetTokensFile( options.gatewayTokensFile, problem ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: Gateway: " << problem << "; it is read again when it changes" );
    if ( !options.tokenFile.IsNULLOrEmpty() )
    {
        WEB::CMcpStaticBearerAuthenticator tokenReader;
        tokenReader.SetTokenFile( options.tokenFile );
        if ( !tokenReader.Prepare( problem ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: The static token of the gateway: " << problem );
            return 2;
        }
        const CORE::CString token = tokenReader.GetToken();
        WEB::CHttpLogRedaction::AddSecret( token );
        tokens.SetStaticToken( token );
    }

    GUCEFMcpCi::CCiGateway::CSettings settings;
    const bool tls = !options.listenTls.IsNULLOrEmpty();
    SplitListen( tls ? options.listenTls : options.listen, settings.listenAddress, settings.port );
    if ( tls )
    {
        settings.certificateFile = options.tlsCert;
        settings.privateKeyFile = options.tlsKey;
    }
    settings.allowedHosts = options.allowedHosts;
    settings.threads = options.gatewayThreads;
    settings.maxPending = options.maxPendingRequests;
    settings.maxPendingPolls = options.maxPendingPolls;
    settings.maxPendingChunks = options.maxUploads * 2;
    settings.requestsPerMinute = options.maxRequestsPerMinute;
    settings.requestBurst = options.maxRequestsPerMinute > 0 && options.maxRequestsPerMinute < 120 ? options.maxRequestsPerMinute : 120;
    settings.pollIntervalInMs = options.pollIntervalInMs;
    const UInt64 halfIdle = (UInt64) options.uploadIdleSeconds * 500;
    settings.housekeepingIntervalInMs = (UInt32) ( halfIdle < 15000 ? halfIdle : 15000 );

    int exitCode = 0;
    GUCEFMcpCi::CCiGateway gateway( api, tokens, uploads, CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
    if ( !gateway.Start( settings, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: " << problem );
        exitCode = 4;
    }
    else
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway for the CI ids " << config.registry.ListIds() << "; work directory " << options.workDir );
        exitCode = CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, true );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Stopping the gateway" );
    }
    gateway.Stop();
    uploads.Stop();
    return exitCode;
}

/*-------------------------------------------------------------------------*/

/** Modes direct, proxy and none: the MCP server with the CI tools, until the client or the application stops it */
static int
RunMcpServer( const SOptions& options, const SConfiguration& config, const CORE::CString& mode, int argc, char* argv[] )
{
    if ( mode == "direct" && !EnsureGzipCodec() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: The gzip codec (codecspluginZLIB) is not available: the uncompressed logs of jobs are fetched instead" );

    COM::CComGlobal::Instance();
    WEB::CWebGlobal::Instance();            // the HTTP client used to reach the CI or the gateway
    if ( !config.forgejoToken.IsNULLOrEmpty() )
        WEB::CHttpLogRedaction::AddSecret( config.forgejoToken );
    if ( !config.gatewayToken.IsNULLOrEmpty() )
        WEB::CHttpLogRedaction::AddSecret( config.gatewayToken );

    COM::CMcpServer server;
    COM::CMcpImplementationInfo info;
    info.name = "gucefMcpCi";
    info.title = "GUCEF lab CI";
    info.version = "1.2.0";
    server.SetServerInfo( info );

    const CORE::CString::StringVector toolNames = GUCEFMcpCi::GetToolNames();
    COM::CMcpConfigurableAccessPolicy* policy = GUCEF_NEW COM::CMcpConfigurableAccessPolicy();
    for ( size_t i=0; i<toolNames.size(); ++i )
        policy->GetDefaultRules().AddAllowTool( toolNames[ i ] );
    server.SetAccessPolicy( COM::CIMcpAccessPolicyPtr( policy ) );

    GUCEFMcpCi::CSnapshotBuilder snapshotBuilder;
    snapshotBuilder.SetMaxBytes( (UInt64) options.maxSnapshotMb * 1024 * 1024 );
    snapshotBuilder.SetWorkDir( CORE::CombinePath( options.logDir, "snapshots" ) );

    GUCEFMcpCi::CDirectCiService directService( config.registry, snapshotBuilder );
    directService.SetMachineName( options.machineName );
    directService.SetPollInterval( options.pollIntervalInMs );
    if ( mode == "direct" )
    {
        AddForgejoBackend( directService, snapshotBuilder, options, config.forgejoToken );
        SetUpDeployments( directService, options, config );
    }

    GUCEFMcpCi::CRemoteCiService remoteService( snapshotBuilder );
    remoteService.SetGateway( config.gatewayUrl, config.gatewayToken, config.gatewayCaFile );
    remoteService.SetMachineName( options.machineName );
    remoteService.SetWorkDir( options.logDir );

    GUCEFMcpCi::ICiService& service = mode == "proxy" ? static_cast< GUCEFMcpCi::ICiService& >( remoteService ) : static_cast< GUCEFMcpCi::ICiService& >( directService );
    const bool servesHttp = !options.listen.IsNULLOrEmpty() || !options.listenTls.IsNULLOrEmpty();
    const bool offersTools = mode == "direct" || mode == "proxy";
    CORE::CString instructions;
    if ( offersTools )
    {
        GUCEFMcpCi::SToolSettings toolSettings;
        toolSettings.logDir = CORE::CombinePath( options.logDir, "logs" );
        toolSettings.maxWaitSeconds = options.maxWaitSeconds;
        toolSettings.redactor.AddSecret( config.forgejoToken );
        toolSettings.redactor.AddSecret( config.gatewayToken );
        toolSettings.redactor.AddSecret( config.registryToken );
        // direct: deploy only when the owner of this machine allows it; proxy: the gateway decides by the token
        toolSettings.caller = GUCEFMcpCi::CCiCaller( options.machineName, options.allowDeploy && mode == "direct" ? "build,deploy" : "build" );
        GUCEFMcpCi::RegisterTools( server, service, toolSettings );

        // proxy: the state of the gateway as of now, so that a gateway that is down is no surprise
        CORE::CString ciIds = config.registry.ListIds();
        CORE::CString gatewayState;
        if ( mode == "proxy" )
        {
            CORE::CString gatewayIds;
            CORE::CString probeProblem;
            if ( remoteService.Probe( gatewayIds, probeProblem ) )
            {
                ciIds = gatewayIds;
                gatewayState = " The CI is reached through the gateway " + config.gatewayUrl + ".";
            }
            else
            {
                gatewayState = " The CI gateway at " + config.gatewayUrl + " cannot be reached at the moment (" + probeProblem +
                               "); the CI tools answer with an error until it can.";
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: " << probeProblem );
            }
        }
        instructions = "Lab CI tools: hand the whole build matrix of a repository (all toolchains, C++ standards and products) to the CI "
                       "instead of building or testing variants yourself. Flow: ci_resolve_repo gives the CI id of a repository; ci_submit "
                       "ONCE sends the unpublished work as a snapshot and starts a run; ci_status with wait_seconds until it is finished; "
                       "ci_report; ci_job_details only for failed jobs. Deployments (ci_images, ci_release_status, ci_deploy, ci_rollback) "
                       "change what runs: deploy or roll back only when the user explicitly asks for it, and show ci_release_status first. CI ids: " +
                       ( ciIds.IsNULLOrEmpty() ? CORE::CString( "as ci_resolve_repo finds them" ) : ciIds ) + "." + gatewayState;
        if ( servesHttp && !options.machineName.IsNULLOrEmpty() )
            instructions += " The tools run on the machine " + options.machineName + ": repo_dir paths are those of that machine.";
    }
    else
    {
        const CORE::CString missing = config.missingProxy + "; " + config.missingDirect;
        instructions = "No CI is set up for this machine (" + missing + "), so no CI tools are offered.";
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: No CI tools: " << missing );
    }
    server.SetInstructions( instructions );

    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( server.GetSettings().threadPoolName, true );
    if ( !threadPool.IsNULL() )
    {
        threadPool->SetDesiredMinNrOfWorkerThreads( 8 );
        threadPool->SetDesiredMaxTotalNrOfThreads( 8 );
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Mode " << mode << "; CI ids: " << ( service.ListCiIds().IsNULLOrEmpty() ? CORE::CString( "none" ) : service.ListCiIds() ) <<
                      "; logs in " << options.logDir );

    // stdio for the one client that started this process; HTTP for any number of clients, each with the token
    int exitCode = 0;
    GUCEF::vector< COM::CIMcpTransportPtr > transports;
    CORE::CString transportProblem;
    if ( !servesHttp || options.stdio )
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
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: " << transportProblem );
        return 4;
    }
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
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: A transport did not start, see the log above" );
        exitCode = 4;
    }
    return exitCode;
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
    ApplyDefaults( options );

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
    CORE::CString problem;
    SConfiguration config;
    if ( !optionsValid || options.help )
    {
        PrintUsage();
        exitCode = options.help ? 0 : 2;
    }
    else
    if ( !CheckServingOptions( options, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: " << problem );
        exitCode = 2;
    }
    else
    if ( !EnsureJsonCodec() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: The JSON codec plugin dstorepluginJSONPARSER is not available" );
        exitCode = 3;
    }
    else
    if ( !LoadConfiguration( options, config, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: " << problem );
        exitCode = 2;
    }
    else
    {
        // auto: a gateway first, then the CI of this machine, else no tools
        CORE::CString mode = options.mode;
        if ( mode == "auto" )
            mode = config.missingProxy.IsNULLOrEmpty() ? "proxy" : ( config.missingDirect.IsNULLOrEmpty() ? "direct" : "none" );
        if ( ( mode == "direct" || mode == "gateway" ) && !config.missingDirect.IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: --mode " << mode << " needs a CI registry and a token: " << config.missingDirect );
            exitCode = 2;
        }
        else
        if ( mode == "proxy" && !config.missingProxy.IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: --mode proxy needs a gateway and a token for it: " << config.missingProxy );
            exitCode = 2;
        }
        else
        if ( mode == "gateway" )
            exitCode = RunGateway( options, config, argc, argv );
        else
            exitCode = RunMcpServer( options, config, mode, argc, argv );
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
