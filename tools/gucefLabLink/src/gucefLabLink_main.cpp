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

#include <stdio.h>
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <unistd.h>
#endif

#include "gucefLabLink_Cluster.h"
#include "gucefLabLink_Machine.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEFLabLink;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The command line, --name value or --name=value.
 *
 *  Commands (each does its work and ends):
 *    --init                 cluster: makes the lab secret when there is none and prints it
 *    --set-secret SECRET    machine: keeps the lab secret of the cluster
 *    --approve NAME|ID      cluster: approves a machine that asked for access
 *    --revoke NAME|ID       cluster: takes an approval back; the machine's token is refused from then on
 *    --list                 cluster: the machines that asked for access
 *
 *  Service (without a command):
 *    --role cluster|machine default: cluster on a k3s server node (/etc/systemd/system/k3s.service), else machine
 *    --state-dir DIR        default /var/lib/gucef-lablink
 *    --allow CIDR           an address range peers may be in, repeatable; none: every address
 *    --cluster-name NAME    cluster: default the host name
 *    --lab-port N, --access-port N   default 47810, 47811
 *    --no-mdns, --no-broadcast, --broadcast-port N (default 47812)
 *    --join-command PATH    cluster: agent-node/join-command.sh; without it machines cannot join
 *    --k3s-bin PATH         cluster: default /usr/local/bin/k3s
 *    --mcp-dir DIR          cluster: default /var/lib/gucef-mcp
 *    --join-script PATH     machine: agent-node/join.sh; with it the machine joins the cluster when it is no node yet
 *    --permanent            machine: join as a node that is always on, not an ephemeral one
 *    --node-marker PATH     machine: exists on a k3s node; default /etc/systemd/system/k3s-agent.service
 *    --claude-user NAME     machine: whose Claude Code gets the MCP services
 *    --claude-command PATH, --claude-home DIR   machine: default the claude and the home of that user
 *    --local-node NAME      machine: the node name of this machine, whose MCP service is registered locally
 *    --refresh SECONDS      machine: how often it visits the cluster again, default 600
 *    --retry SECONDS        machine: how often it tries again after a failure or while it waits for approval, default 30
 */
struct SOptions
{
    SSettings settings;
    CORE::CString::StringVector ranges;
    CORE::CString role;
    CORE::CString command;
    CORE::CString commandArgument;
    bool help;

    SOptions( void ) : settings(), ranges(), role(), command(), commandArgument(), help( false ) {}
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
        const bool flag = arg == "--init" || arg == "--list" || arg == "--no-mdns" || arg == "--no-broadcast" || arg == "--permanent" ||
                          arg == "--help" || arg == "--console";
        if ( !flag && value.IsNULLOrEmpty() )
        {
            if ( i+1 >= argc )
                return false;
            value = argv[ ++i ];
        }

        SSettings& s = options.settings;
        if ( arg == "--init" || arg == "--list" ) options.command = arg;
        else if ( arg == "--set-secret" || arg == "--approve" || arg == "--revoke" ) { options.command = arg; options.commandArgument = value; }
        else if ( arg == "--role" ) options.role = value;
        else if ( arg == "--state-dir" ) s.stateDir = value;
        else if ( arg == "--allow" ) options.ranges.push_back( value );
        else if ( arg == "--cluster-name" ) s.clusterName = value;
        else if ( arg == "--lab-port" ) s.labPort = (UInt16) CORE::StringToUInt32( value );
        else if ( arg == "--access-port" ) s.accessPort = (UInt16) CORE::StringToUInt32( value );
        else if ( arg == "--no-mdns" ) s.useMdns = false;
        else if ( arg == "--no-broadcast" ) s.useBroadcast = false;
        else if ( arg == "--broadcast-port" ) s.broadcastPort = (UInt16) CORE::StringToUInt32( value );
        else if ( arg == "--join-command" ) s.joinCommand = value;
        else if ( arg == "--k3s-bin" ) s.k3sBin = value;
        else if ( arg == "--mcp-dir" ) s.mcpDir = value;
        else if ( arg == "--join-script" ) { s.joinScript = value; s.join = true; }
        else if ( arg == "--permanent" ) s.ephemeral = false;
        else if ( arg == "--node-marker" ) s.nodeMarkerFile = value;
        else if ( arg == "--claude-user" ) s.claudeUser = value;
        else if ( arg == "--claude-command" ) s.claudeCommand = value;
        else if ( arg == "--claude-home" ) s.claudeHome = value;
        else if ( arg == "--local-node" ) s.localMcpNode = value;
        else if ( arg == "--refresh" ) s.refreshIntervalInSecs = CORE::StringToUInt32( value, 600 );
        else if ( arg == "--retry" ) s.retryIntervalInSecs = CORE::StringToUInt32( value, 30 );
        else if ( arg == "--help" ) options.help = true;
        else if ( arg == "--console" ) continue;     // read by the entry point of the framework
        else return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void
PrintUsage( void )
{
    fprintf( stdout, "gucefLabLink: links the machines of a home network to its k3s cluster.\n"
                     "Commands: --init, --set-secret SECRET, --approve NAME|ID, --revoke NAME|ID, --list.\n"
                     "Service: --role cluster|machine, --state-dir DIR, --allow CIDR (repeatable), --cluster-name NAME, --lab-port N,\n"
                     "  --access-port N, --no-mdns, --no-broadcast, --broadcast-port N, --join-command PATH, --k3s-bin PATH, --mcp-dir DIR,\n"
                     "  --join-script PATH, --permanent, --node-marker PATH, --claude-user NAME, --claude-command PATH, --claude-home DIR,\n"
                     "  --local-node NAME, --refresh SECONDS, --retry SECONDS\n" );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
HostName( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    char name[ 256 ];
    if ( 0 == ::gethostname( name, sizeof( name ) - 1 ) )
    {
        name[ sizeof( name ) - 1 ] = '\0';
        return CORE::CString( name ).Lowercase();
    }
    #endif
    return CORE::GetEnv( "COMPUTERNAME" ).Lowercase();
}

/*-------------------------------------------------------------------------*/

/** codecspluginMONOCYPHER: the key agreement and ciphers of DRN */
static bool
LoadCryptography( void )
{
    if ( DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsCipherAvailable( "xchacha20-poly1305" ) )
        return true;
    CORE::CPluginControl& plugins = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !plugins.AddPluginFromDir( "$MODULEDIR$/codecspluginMONOCYPHER", "gucefLabLink", true ) )
        plugins.AddPluginFromDir( "$MODULEDIR$/codecspluginMONOCYPHER_d", "gucefLabLink", true );
    return DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsCipherAvailable( "xchacha20-poly1305" );
}

/*-------------------------------------------------------------------------*/

/** The approvals command line: --list, --approve, --revoke. @return the exit code */
static int
ChangeApprovals( CStore& store, const CORE::CString& command, const CORE::CString& who )
{
    CORE::CString problem;
    GUCEF::vector< SApproval > approvals;
    if ( !store.LoadApprovals( approvals, problem ) )
    {
        fprintf( stderr, "gucefLabLink: %s\n", problem.C_String() );
        return 1;
    }
    if ( command == "--list" )
    {
        if ( approvals.empty() )
            fprintf( stdout, "No machine asked for access.\n" );
        for ( size_t i=0; i<approvals.size(); ++i )
            fprintf( stdout, "%-9s %-24s %-16s %-8s %s\n", approvals[ i ].state.C_String(), approvals[ i ].name.C_String(),
                     approvals[ i ].address.C_String(), approvals[ i ].os.C_String(), approvals[ i ].machineId.C_String() );
        return 0;
    }

    UInt32 matches = 0;
    for ( size_t i=0; i<approvals.size(); ++i )
    {
        if ( approvals[ i ].name != who && approvals[ i ].machineId != who )
            continue;
        ++matches;
        if ( command == "--approve" )
            approvals[ i ].state = AccessApproved;
        else
        {
            approvals[ i ].state = AccessRefused;
            approvals[ i ].token.Clear();
        }
    }
    if ( 0 == matches )
    {
        fprintf( stderr, "gucefLabLink: no machine \"%s\" asked for access (see --list)\n", who.C_String() );
        return 1;
    }
    if ( matches > 1 && command == "--approve" )
    {
        fprintf( stderr, "gucefLabLink: %u machines are called \"%s\": approve one by its id (see --list)\n", matches, who.C_String() );
        return 1;
    }
    if ( !store.SaveApprovals( approvals, problem ) )
    {
        fprintf( stderr, "gucefLabLink: %s\n", problem.C_String() );
        return 1;
    }
    fprintf( stdout, "%s %s. It is told on its next request.\n", who.C_String(), command == "--approve" ? "is approved" : "is refused from now on" );
    return 0;
}

/*-------------------------------------------------------------------------*/

/** SIGTERM and SIGINT: only the async-signal-safe stop request */
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

    // a service writes its log to standard error, which systemd keeps in the journal
    if ( CORE::GetEnv( "GUCEF_CONSOLE_LOGGER" ).IsNULLOrEmpty() )
        CORE::GUCEFSetEnv( "GUCEF_CONSOLE_LOGGER", "stderr" );
    CORE::CPlatformNativeConsoleLogger console;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    if ( !optionsValid || options.help )
    {
        PrintUsage();
        return options.help ? 0 : 2;
    }
    CORE::CString problem;
    if ( !options.settings.allowedRanges.Set( options.ranges, problem ) )
    {
        fprintf( stderr, "gucefLabLink: %s\n", problem.C_String() );
        return 2;
    }
    if ( options.settings.clusterName.IsNULLOrEmpty() )
        options.settings.clusterName = HostName();

    CStore store( options.settings.stateDir );
    COM::CComGlobal::Instance();
    DRN::CDRNGlobal::Instance();
    if ( !LoadCryptography() )
    {
        fprintf( stderr, "gucefLabLink: the cryptography of DRN is not available: codecspluginMONOCYPHER is missing\n" );
        return 3;
    }

    // the commands
    if ( options.command == "--init" )
    {
        CORE::CString secret;
        if ( !store.ReadSecret( secret, problem ) )
        {
            secret = MakeLabSecret();
            if ( secret.IsNULLOrEmpty() || !store.WriteSecret( secret, problem ) )
            {
                fprintf( stderr, "gucefLabLink: cannot make the lab secret: %s\n", problem.C_String() );
                return 1;
            }
        }
        fprintf( stdout, "The lab secret, for the machines that link without approval:\n\n  %s\n\n"
                         "On such a machine: sudo gucefLabLink --set-secret <secret>\n", secret.C_String() );
        return 0;
    }
    if ( options.command == "--set-secret" )
    {
        if ( StripText( options.commandArgument ).Length() < 32 || !store.WriteSecret( options.commandArgument, problem ) )
        {
            fprintf( stderr, "gucefLabLink: %s\n", problem.IsNULLOrEmpty() ? "that is not a lab secret" : problem.C_String() );
            return 1;
        }
        fprintf( stdout, "The lab secret is kept in %s.\n", store.GetPath( "secret" ).C_String() );
        return 0;
    }
    if ( options.command == "--approve" || options.command == "--revoke" || options.command == "--list" )
        return ChangeApprovals( store, options.command, options.commandArgument );

    // the service
    CORE::CString role = options.role;
    if ( role.IsNULLOrEmpty() )
        role = CORE::FileExists( "/etc/systemd/system/k3s.service" ) ? "cluster" : "machine";
    int exitCode = 0;
    CClusterRole* cluster = GUCEF_NULL;
    CMachineRole* machine = GUCEF_NULL;
    if ( role == "cluster" )
    {
        cluster = GUCEF_NEW CClusterRole( options.settings, store );
        if ( !cluster->Start( problem ) )
            exitCode = 4;
    }
    else
    if ( role == "machine" )
    {
        machine = GUCEF_NEW CMachineRole( options.settings, store );
        if ( !machine->Start( problem ) )
            exitCode = 4;
    }
    else
    {
        problem = "the role is cluster or machine, not " + role;
        exitCode = 2;
    }

    if ( 0 == exitCode )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        while ( !CORE::CGUCEFApplication::IsStopRequestedFromSignal() )
        {
            pulseGenerator->ForceDirectPulse();
            if ( GUCEF_NULL != machine )
                machine->Update();
            MT::ThreadDelay( 10 );
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Stopping" );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefLabLink: " << problem );
    }

    GUCEF_DELETE cluster;
    GUCEF_DELETE machine;
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
    CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( console.GetLogger() );
    return exitCode;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
