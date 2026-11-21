/*
 *  gucefLabLink_TestApp: tests of gucefLabLink
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
#include <sys/stat.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#include "gucefLabLink_Cluster.h"
#include "gucefLabLink_HubStore.h"
#include "gucefLabLink_Machine.h"
#include "gucefLabLink_Registry.h"
#include "gucefLabLink_Reporter.h"
#include "TestLabLink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/* near and far are macros of the Windows headers, undefine them so they can be used as identifiers */
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

using namespace GUCEF;
using namespace GUCEFLabLink;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
LoadCryptography( void )
{
    COM::CComGlobal::Instance();
    DRN::CDRNGlobal::Instance();
    if ( DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsCipherAvailable( "xchacha20-poly1305" ) )
        return true;
    // as a codec plugin, as gucefLabLink loads it: no trial of the other plugin kinds
    CORE::CPluginControl& plugins = CORE::CCoreGlobal::Instance()->GetPluginControl();
    CORE::CPluginMetaData metaData;
    metaData.SetFullModulePath( CORE::RelativePath( "$MODULEDIR$" ) );
    metaData.SetPluginType( "GucefStandardCodecPlugin" );
    metaData.SetModuleFilename( "codecspluginMONOCYPHER" );
    if ( !plugins.AddPluginMetaData( metaData, "test", true ) )
    {
        metaData.SetModuleFilename( "codecspluginMONOCYPHER_d" );
        plugins.AddPluginMetaData( metaData, "test", true );
    }
    return DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsCipherAvailable( "xchacha20-poly1305" );
}

/*-------------------------------------------------------------------------*/

bool
WriteText( const CORE::CString& path, const CORE::CString& text, bool executable = false )
{
    FILE* file = fopen( path.C_String(), "wb" );
    if ( GUCEF_NULL == file )
        return false;
    fwrite( text.C_String(), 1, text.Length(), file );
    fclose( file );
    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    if ( executable )
        chmod( path.C_String(), 0755 );
    #endif
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
ReadText( const CORE::CString& path )
{
    CORE::CDynamicBuffer content;
    if ( !CORE::FileExists( path ) || !content.LoadContentFromFile( path ) )
        return CORE::CString::Empty;
    return ( 0 == content.GetDataSize() ? CORE::CString::Empty : CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) );
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part ) >= 0;
}

/*-------------------------------------------------------------------------*/

/**
 *  A lab in a directory: the state directories of a cluster and of machines, an MCP directory as the DaemonSet of
 *  cluster.sh fills it, and stand-ins for join-command.sh, join.sh and claude
 */
class CLab
{
    public:

    TESTFW::CScopedTestDir dir;
    CStore clusterStore;
    SSettings clusterSettings;
    CClusterRole* cluster;

    CLab( const char* name )
        : dir( name )
        , clusterStore( Path( "cluster" ) )
        , clusterSettings()
        , cluster( GUCEF_NULL )
    {
        CORE::CreateDirs( Path( "mcp/tls" ) );
        CORE::CreateDirs( Path( "bin" ) );
        WriteText( Path( "mcp/tls/ca.crt" ), "-----BEGIN CERTIFICATE-----\ntest authority\n-----END CERTIFICATE-----\n" );
        WriteText( Path( "mcp/lab-token" ), "the-lab-mcp-token\n" );
        WriteText( Path( "mcp/mcp-nodes" ), "# the nodes\nserver https://10.0.0.1:8791/mcp\nworkstation https://10.0.0.2:8791/mcp\n" );
        WriteText( Path( "bin/join-command.sh" ), "#!/bin/sh\necho \"$*\" > \"" + Path( "join-command.args" ) + "\"\n"
                   "echo server=https://10.0.0.1:6443\necho version=v1.37.2+k3s1\necho token=K10test::server:secret\n"
                   "case \" $* \" in *\" --ephemeral \"*) echo ephemeral=true ;; *) echo ephemeral=false ;; esac\n", true );
        WriteText( Path( "bin/join.sh" ), "#!/bin/sh\necho \"$* user=$SUDO_USER\" >> \"" + Path( "join.log" ) + "\"\n", true );
        WriteText( Path( "bin/no-k3s" ), "#!/bin/sh\nexit 1\n", true );     // k3s that gives no version
        WriteText( Path( "bin/claude" ), "#!/bin/sh\necho \"$*\" >> \"" + Path( "claude.log" ) + "\"\n", true );

        clusterSettings.stateDir = Path( "cluster" );
        clusterSettings.clusterName = "testlab";
        clusterSettings.labPort = TESTFW::CTlsTestServer::FindFreePort();
        clusterSettings.accessPort = TESTFW::CTlsTestServer::FindFreePort();
        while ( clusterSettings.accessPort == clusterSettings.labPort )
            clusterSettings.accessPort = TESTFW::CTlsTestServer::FindFreePort();
        clusterSettings.useMdns = false;
        clusterSettings.useBroadcast = false;
        clusterSettings.joinCommand = Path( "bin/join-command.sh" );
        clusterSettings.k3sBin = Path( "bin/no-k3s" );
        clusterSettings.mcpDir = Path( "mcp" );
    }

    ~CLab()
    {
        GUCEF_DELETE cluster;
    }

    CORE::CString Path( const char* name ) const { return CORE::CombinePath( dir.GetPath(), name ); }

    bool StartCluster( const char* allowedRange = "127.0.0.0/8" )
    {
        CORE::CString problem;
        CORE::CString::StringVector ranges;
        ranges.push_back( allowedRange );
        clusterSettings.allowedRanges.Set( ranges, problem );
        CORE::CString secret;
        if ( !clusterStore.ReadSecret( secret, problem ) && !clusterStore.WriteSecret( MakeLabSecret(), problem ) )
            return false;
        cluster = GUCEF_NEW CClusterRole( clusterSettings, clusterStore );
        if ( cluster->Start( problem ) )
            return true;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "LabLink test: the cluster does not start: " << problem );
        return false;
    }

    CORE::CString Secret( void ) const
    {
        CORE::CString secret, problem;
        clusterStore.ReadSecret( secret, problem );
        return secret;
    }

    /** The settings of a machine with a state directory of its own */
    SSettings MachineSettings( const char* name ) const
    {
        SSettings settings;
        settings.stateDir = Path( name );
        settings.machineName = CORE::CString( "test-" ) + name;    // not the host name: the log shows a test machine
        settings.useMdns = false;
        settings.useBroadcast = false;
        settings.claudeUser = CORE::GetEnv( "USER" ).IsNULLOrEmpty() ? CORE::CString( "test" ) : CORE::GetEnv( "USER" );
        settings.claudeCommand = Path( "bin/claude" );
        settings.claudeHome = Path( ( CORE::CString( name ) + "-home" ).C_String() );
        settings.mcpDir = Path( ( CORE::CString( name ) + "-mcp" ).C_String() );   // the CI files of the machine, not those of this computer
        settings.localMcpNode = "server";
        CORE::CString problem;
        CORE::CString::StringVector ranges;
        ranges.push_back( "127.0.0.0/8" );
        settings.allowedRanges.Set( ranges, problem );
        return settings;
    }

    /** The cluster as its discovery entry would describe it */
    SClusterSighting Sighting( void ) const
    {
        SClusterSighting sighting;
        sighting.name = "testlab";
        sighting.address = "127.0.0.1";
        sighting.key = cluster->GetLabKey();
        sighting.labPort = clusterSettings.labPort;
        sighting.accessPort = clusterSettings.accessPort;
        return sighting;
    }

    bool SetApproval( const char* machineName, const char* state )
    {
        CORE::CString problem;
        GUCEF::vector< SApproval > approvals;
        if ( !clusterStore.LoadApprovals( approvals, problem ) )
            return false;
        bool found = false;
        for ( size_t i=0; i<approvals.size(); ++i )
        {
            if ( approvals[ i ].name != machineName )
                continue;
            approvals[ i ].state = state;
            if ( CORE::CString( state ) != AccessApproved )
                approvals[ i ].token.Clear();
            found = true;
        }
        return found && clusterStore.SaveApprovals( approvals, problem );
    }
};

/** Runs the application and the roles until the machine's reporter has had count reports accepted, or the time is out */
bool
WaitForReports( CLab& lab, CMachineRole& machine, UInt32 count, UInt32 timeoutInMs )
{
    const UInt64 deadline = NowInMs() + timeoutInMs;
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    while ( NowInMs() < deadline )
    {
        pulseGenerator->ForceDirectPulse();
        machine.Update();
        lab.cluster->Update();
        const CNodeReporter* reporter = machine.GetReporter();
        if ( GUCEF_NULL != reporter && reporter->GetReportsAccepted() >= count )
            return true;
        MT::ThreadDelay( 2 );
    }
    return false;
}

/** Runs the application and the cluster for a while, for the nodes that are silent */
void
RunLab( CLab& lab, UInt32 timeInMs )
{
    const UInt64 deadline = NowInMs() + timeInMs;
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    while ( NowInMs() < deadline )
    {
        pulseGenerator->ForceDirectPulse();
        lab.cluster->Update();
        MT::ThreadDelay( 5 );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformLabLinkTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "LabLink" );

    GUCEF_TESTFW_TESTCASE( "Protocol: address ranges, the token key and the proof of the cluster" )
        try
        {
            CAddressRanges ranges;
            CORE::CString problem;
            CORE::CString::StringVector list;
            list.push_back( "172.16.0.0/24" );
            list.push_back( " 172.16.1.0/24 " );
            ASSERT_TRUE( ranges.Set( list, problem ) );
            ASSERT_TRUE( ranges.Allows( "172.16.0.9" ) && ranges.Allows( "172.16.1.200" ) );
            ASSERT_FALSE( ranges.Allows( "172.16.2.1" ) || ranges.Allows( "10.0.0.1" ) || ranges.Allows( "not an address" ) );
            list.push_back( "172.16.3.0" );
            ASSERT_FALSE( ranges.Set( list, problem ) );
            ASSERT_TRUE( Contains( problem, "172.16.3.0" ) );
            CAddressRanges none;
            ASSERT_TRUE( none.IsEmpty() && none.Allows( "8.8.8.8" ) );

            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            const CORE::CString secret = MakeLabSecret();
            ASSERT_TRUE( 64 == secret.Length() && secret != MakeLabSecret() );
            ASSERT_TRUE( DeriveTokenKey( secret ) == DeriveTokenKey( secret ) && DeriveTokenKey( secret ) != DeriveTokenKey( MakeLabSecret() ) );
            ASSERT_TRUE( DeriveTokenKey( secret ) != secret );
            ASSERT_TRUE( MakeClusterProof( secret, "key-a" ) != MakeClusterProof( secret, "key-b" ) );
            ASSERT_TRUE( EqualInConstantTime( "abc", "abc" ) && !EqualInConstantTime( "abc", "abd" ) && !EqualInConstantTime( "abc", "ab" ) );
            ASSERT_TRUE( ValueOfLine( "server=https://x:6443\r\ntoken=t\n", "token" ) == "t" && ValueOfLine( "a=1", "b" ).IsNULLOrEmpty() );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lab secret: a machine with it links, the cluster proves itself, the machine joins and Claude Code gets the MCP services" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CLab lab( "gucefLabLink_TestApp_Secret" );
            ASSERT_TRUE( lab.StartCluster() );

            SSettings settings = lab.MachineSettings( "machine" );
            settings.join = true;
            settings.joinScript = lab.Path( "bin/join.sh" );
            settings.nodeMarkerFile = lab.Path( "no-such-file" );
            CStore store( settings.stateDir );
            CORE::CString problem;
            ASSERT_TRUE( store.WriteSecret( lab.Secret(), problem ) );
            CMachineRole machine( settings, store );
            ASSERT_TRUE( machine.Visit( lab.Sighting() ) );

            const SClusterAnswers& answers = machine.GetLastAnswers();
            ASSERT_TRUE( answers.cluster == "testlab" );
            ASSERT_TRUE( Contains( answers.caCertificate, "test authority" ) );
            ASSERT_TRUE( answers.mcpToken == "the-lab-mcp-token" );
            ASSERT_TRUE( 2 == answers.nodes.size() );

            // the join: a token for one hour, ephemeral, join.sh with what the cluster gave
            ASSERT_TRUE( Contains( ReadText( lab.Path( "join-command.args" ) ), "--values --ttl 1h --ephemeral --node test-machine" ) );
            const CORE::CString join = ReadText( lab.Path( "join.log" ) );
            ASSERT_TRUE( Contains( join, "--server https://10.0.0.1:6443 --version v1.37.2+k3s1 --token K10test::server:secret --ephemeral" ) );
            ASSERT_TRUE( Contains( join, "user=" + settings.claudeUser ) );

            // Claude Code: the own node locally, the other by HTTPS, with the lab token; the authority in the profile
            const CORE::CString claude = ReadText( lab.Path( "claude.log" ) );
            ASSERT_TRUE( Contains( claude, "mcp add --transport http local-llm http://127.0.0.1:8790/mcp --scope user --header Authorization: Bearer the-lab-mcp-token" ) );
            ASSERT_TRUE( Contains( claude, "mcp add --transport http workstation-llm https://10.0.0.2:8791/mcp --scope user" ) );
            ASSERT_FALSE( Contains( claude, "server-llm" ) );
            ASSERT_TRUE( Contains( ReadText( CORE::CombinePath( settings.claudeHome, ".config/gucef-lab/ca.crt" ) ), "test authority" ) );
            ASSERT_TRUE( Contains( ReadText( CORE::CombinePath( settings.claudeHome, ".profile" ) ), "export NODE_EXTRA_CA_CERTS=" ) );

            // a second visit changes nothing that is the same; a node that is gone is removed
            WriteText( lab.Path( "claude.log" ), "" );
            ASSERT_TRUE( machine.Visit( lab.Sighting() ) );
            ASSERT_FALSE( Contains( ReadText( lab.Path( "claude.log" ) ), "mcp add" ) );
            WriteText( lab.Path( "mcp/mcp-nodes" ), "server https://10.0.0.1:8791/mcp\n" );
            ASSERT_TRUE( machine.Visit( lab.Sighting() ) );
            ASSERT_TRUE( Contains( ReadText( lab.Path( "claude.log" ) ), "mcp remove workstation-llm --scope user" ) );
            CORE::CString profile = ReadText( CORE::CombinePath( settings.claudeHome, ".profile" ) );
            ASSERT_TRUE( profile.HasSubstr( "NODE_EXTRA_CA_CERTS" ) == profile.HasSubstr( "NODE_EXTRA_CA_CERTS", false ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Trust: another secret, another key or an address outside the ranges gets nowhere" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CLab lab( "gucefLabLink_TestApp_Trust" );
            ASSERT_TRUE( lab.StartCluster() );
            CORE::CString problem;

            // a secret that is not the cluster's makes tokens the cluster refuses
            SSettings settings = lab.MachineSettings( "wrong-secret" );
            CStore store( settings.stateDir );
            ASSERT_TRUE( store.WriteSecret( MakeLabSecret(), problem ) );
            CMachineRole wrong( settings, store );
            ASSERT_FALSE( wrong.Visit( lab.Sighting() ) );
            ASSERT_TRUE( Contains( wrong.GetLastOutcome(), "did not take the link" ) );

            // the right secret, but the discovery entry names another key: the link to that key does not come up
            SSettings rightSettings = lab.MachineSettings( "right-secret" );
            CStore rightStore( rightSettings.stateDir );
            ASSERT_TRUE( rightStore.WriteSecret( lab.Secret(), problem ) );
            CMachineRole right( rightSettings, rightStore );
            SClusterSighting impostor = lab.Sighting();
            impostor.key = "00000000000000000000000000000000000000000000000000000000000000aa";
            ASSERT_FALSE( right.Visit( impostor ) );
            ASSERT_TRUE( right.Visit( lab.Sighting() ) );

            // a machine only uses clusters in its ranges
            SSettings outside = lab.MachineSettings( "outside" );
            CORE::CString::StringVector ranges;
            ranges.push_back( "10.0.0.0/8" );
            outside.allowedRanges.Set( ranges, problem );
            CStore outsideStore( outside.stateDir );
            ASSERT_TRUE( outsideStore.WriteSecret( lab.Secret(), problem ) );
            CMachineRole far( outside, outsideStore );
            ASSERT_FALSE( far.Visit( lab.Sighting() ) );
            ASSERT_TRUE( Contains( far.GetLastOutcome(), "outside the allowed ranges" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Trust: a cluster only answers machines in its ranges" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CLab lab( "gucefLabLink_TestApp_Ranges" );
            ASSERT_TRUE( lab.StartCluster( "10.0.0.0/8" ) );
            SSettings settings = lab.MachineSettings( "machine" );
            CStore store( settings.stateDir );
            CORE::CString problem;
            ASSERT_TRUE( store.WriteSecret( lab.Secret(), problem ) );
            CMachineRole machine( settings, store );
            ASSERT_FALSE( machine.Visit( lab.Sighting() ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Approval: a machine without the secret waits, links once approved, and is refused once revoked" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CLab lab( "gucefLabLink_TestApp_Approval" );
            ASSERT_TRUE( lab.StartCluster() );
            SSettings settings = lab.MachineSettings( "machine" );
            CStore store( settings.stateDir );
            CMachineRole machine( settings, store );

            ASSERT_FALSE( machine.Visit( lab.Sighting() ) );
            ASSERT_TRUE( Contains( machine.GetLastOutcome(), "waiting for approval" ) );
            CORE::CString problem;
            GUCEF::vector< SApproval > approvals;
            ASSERT_TRUE( lab.clusterStore.LoadApprovals( approvals, problem ) && 1 == approvals.size() );
            ASSERT_TRUE( approvals[ 0 ].state == AccessPending && approvals[ 0 ].name == "test-machine" && approvals[ 0 ].address == "127.0.0.1" );

            // asking again adds no second request
            ASSERT_FALSE( machine.Visit( lab.Sighting() ) );
            ASSERT_TRUE( lab.clusterStore.LoadApprovals( approvals, problem ) && 1 == approvals.size() );

            ASSERT_TRUE( lab.SetApproval( "test-machine", AccessApproved ) );
            ASSERT_TRUE( machine.Visit( lab.Sighting() ) );
            SIdentity identity;
            ASSERT_TRUE( store.LoadIdentity( identity, problem ) );
            ASSERT_FALSE( identity.token.IsNULLOrEmpty() );
            ASSERT_TRUE( identity.clusterKey == lab.cluster->GetLabKey() && identity.cluster == "testlab" );
            ASSERT_TRUE( Contains( ReadText( lab.Path( "claude.log" ) ), "workstation-llm" ) );

            // the token is kept: the next visit needs no access request
            ASSERT_TRUE( machine.Visit( lab.Sighting() ) );

            ASSERT_TRUE( lab.SetApproval( "test-machine", AccessRefused ) );
            ASSERT_FALSE( machine.Visit( lab.Sighting() ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Discovery: a machine finds a cluster that runs as a program of its own by UDP broadcast, and links" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            const CORE::CString program = CORE::RelativePath( "$MODULEDIR$/gucefLabLink" );
            GUCEF_TESTFW_SKIP_IF( !CORE::FileExists( program ), "gucefLabLink is not next to the test application" );
            CLab lab( "gucefLabLink_TestApp_Discovery" );
            CORE::CString problem;
            ASSERT_TRUE( lab.clusterStore.WriteSecret( MakeLabSecret(), problem ) );

            // the cluster as the service runs it; discovery in one process does not report what that process advertises
            const UInt16 broadcastPort = TESTFW::CTlsTestServer::FindFreePort();
            CORE::CProcess::TStringVector args;
            args.push_back( "--role" ); args.push_back( "cluster" );
            args.push_back( "--state-dir" ); args.push_back( lab.Path( "cluster" ) );
            args.push_back( "--cluster-name" ); args.push_back( "testlab" );
            args.push_back( "--lab-port" ); args.push_back( CORE::ToString( lab.clusterSettings.labPort ) );
            args.push_back( "--access-port" ); args.push_back( CORE::ToString( lab.clusterSettings.accessPort ) );
            args.push_back( "--no-mdns" );
            args.push_back( "--broadcast-port" ); args.push_back( CORE::ToString( broadcastPort ) );
            args.push_back( "--join-command" ); args.push_back( lab.Path( "bin/join-command.sh" ) );
            args.push_back( "--k3s-bin" ); args.push_back( lab.Path( "bin/no-k3s" ) );
            args.push_back( "--mcp-dir" ); args.push_back( lab.Path( "mcp" ) );
            CORE::CProcess::TStringMap environment;
            environment[ "GUCEF_CONSOLE_LOGGER" ] = "stderr";
            CORE::CProcess cluster;
            cluster.SetIsolateFromParent( true );
            ASSERT_TRUE( cluster.Launch( program, args, CORE::CString::Empty, environment ) );

            SSettings settings = lab.MachineSettings( "machine" );
            settings.useBroadcast = true;
            settings.broadcastPort = broadcastPort;
            CORE::CString::StringVector everywhere;
            settings.allowedRanges.Set( everywhere, problem );
            CStore store( settings.stateDir );
            ASSERT_TRUE( store.WriteSecret( lab.Secret(), problem ) );
            CMachineRole machine( settings, store );
            ASSERT_TRUE( machine.Start( problem ) );
            for ( int i=0; i<6000 && machine.GetLastAnswers().cluster.IsNULLOrEmpty(); ++i )
            {
                CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
                machine.Update();
                MT::ThreadDelay( 5 );
            }
            ASSERT_TRUE( machine.GetLastAnswers().cluster == "testlab" );
            machine.Stop();

            // the service stops on SIGTERM
            ASSERT_TRUE( cluster.Terminate( true, 15000 ) );
            ASSERT_FALSE( cluster.IsRunning() );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Registry: online and offline, unknown after a restart, a second instance, unverified claims, bounds and storage" )
        try
        {
            SRegistryLimits limits;
            limits.offlineAfterInSecs = 60;
            limits.maxVerifiedNodes = 2;
            limits.maxUnverifiedNodes = 1;
            limits.flushIntervalInSecs = 300;
            limits.forgetUnverifiedAfterInSecs = 1000;
            CNodeRegistry registry( limits );
            vector< SNodeEvent > events;

            SNodeReport a;
            a.machineId = "a";
            a.instanceId = "run1";
            a.startedInUnixSecs = 100;
            a.name = "alpha";
            ASSERT_TRUE( REPORT_ACCEPTED == registry.OnReport( a, true, "10.0.0.5", 1000, 10000 ) );
            SNodeEntry entry;
            ASSERT_TRUE( registry.Find( "a", entry ) );
            ASSERT_TRUE( NODELIVENESS_ONLINE == entry.liveness && entry.verified && 10000 == entry.firstSeenInUnixSecs && entry.address == "10.0.0.5" );
            registry.TakeEvents( events );
            ASSERT_TRUE( 1 == events.size() && events[ 0 ].kind == "online" );

            // silent for less than the window it is online, after it offline; the wall clock plays no part
            registry.Evaluate( 1000 + 59000, 10059 );
            ASSERT_TRUE( registry.Find( "a", entry ) && NODELIVENESS_ONLINE == entry.liveness );
            registry.Evaluate( 1000 + 61000, 5 );
            ASSERT_TRUE( registry.Find( "a", entry ) && NODELIVENESS_OFFLINE == entry.liveness );
            registry.TakeEvents( events );
            ASSERT_TRUE( 1 == events.size() && events[ 0 ].kind == "offline" );
            ASSERT_TRUE( REPORT_ACCEPTED == registry.OnReport( a, true, "10.0.0.5", 70000, 10070 ) );
            registry.TakeEvents( events );
            ASSERT_TRUE( 1 == events.size() && events[ 0 ].kind == "online" && events[ 0 ].detail == "back" );

            // a second run of the agent: the older one is refused while the newer is heard, the newer takes over at once
            SNodeReport other = a;
            other.instanceId = "run2";
            other.startedInUnixSecs = 50;
            ASSERT_TRUE( REPORT_DUPLICATE_INSTANCE == registry.OnReport( other, true, "10.0.0.6", 71000, 10071 ) );
            ASSERT_TRUE( registry.Find( "a", entry ) && 1 == entry.duplicateInstanceRefusals && entry.address == "10.0.0.5" && entry.report.instanceId == "run1" );
            other.startedInUnixSecs = 200;
            ASSERT_TRUE( REPORT_ACCEPTED == registry.OnReport( other, true, "10.0.0.6", 72000, 10072 ) );
            ASSERT_TRUE( registry.Find( "a", entry ) && entry.report.instanceId == "run2" && entry.address == "10.0.0.6" );
            registry.TakeEvents( events );
            ASSERT_TRUE( 1 == events.size() && events[ 0 ].kind == "instance" );
            // when the first run was silent for the window, even an older start takes over: its machine is gone
            SNodeReport third = a;
            third.instanceId = "run3";
            third.startedInUnixSecs = 10;
            ASSERT_TRUE( REPORT_ACCEPTED == registry.OnReport( third, true, "10.0.0.7", 72000 + 61000, 10133 ) );

            // a token made with the lab secret claims any machine id: it cannot speak for a node that was approved
            SNodeReport claim = a;
            claim.instanceId = "run3";
            ASSERT_TRUE( REPORT_UNVERIFIED_CLAIM == registry.OnReport( claim, false, "10.0.0.9", 135000, 10135 ) );
            ASSERT_TRUE( registry.Find( "a", entry ) && entry.verified );

            // the two tables have bounds of their own; a verified report takes the place of an unverified node
            SNodeReport u1;
            u1.machineId = "u1";
            SNodeReport u2;
            u2.machineId = "u2";
            ASSERT_TRUE( REPORT_ACCEPTED == registry.OnReport( u1, false, "10.0.0.10", 136000, 10136 ) );
            ASSERT_TRUE( REPORT_TABLE_FULL == registry.OnReport( u2, false, "10.0.0.11", 136000, 10136 ) );
            ASSERT_TRUE( REPORT_ACCEPTED == registry.OnReport( u1, true, "10.0.0.10", 137000, 10137 ) );
            ASSERT_TRUE( registry.Find( "u1", entry ) && entry.verified );
            ASSERT_TRUE( REPORT_ACCEPTED == registry.OnReport( u2, false, "10.0.0.11", 137000, 10137 ) );
            SNodeReport v;
            v.machineId = "v";
            ASSERT_TRUE( REPORT_TABLE_FULL == registry.OnReport( v, true, "10.0.0.12", 137000, 10137 ) );
            SNodeReport none;
            ASSERT_TRUE( REPORT_INVALID == registry.OnReport( none, true, "10.0.0.13", 137000, 10137 ) );
            ASSERT_TRUE( 3 == registry.GetCount() );

            // storage: what changed is handed out once, an online node again when the flush interval is over
            vector< SNodeEntry > persist;
            registry.CollectForPersist( 10140, persist );
            ASSERT_TRUE( 3 == persist.size() );
            registry.CollectForPersist( 10141, persist );
            ASSERT_TRUE( persist.empty() );
            registry.OnReport( third, true, "10.0.0.7", 200000, 10300 );
            registry.CollectForPersist( 10300, persist );
            ASSERT_TRUE( persist.empty() );                         // nothing changed and the interval is not over
            registry.OnReport( third, true, "10.0.0.7", 399000, 10500 );
            registry.Evaluate( 400000, 10500 );                     // the two others are silent: offline, which is a change
            registry.CollectForPersist( 10500, persist );
            ASSERT_TRUE( 3 == persist.size() );                     // "a" is due for its flush as well
            registry.CollectForPersist( 10501, persist );
            ASSERT_TRUE( persist.empty() );
            registry.OnReport( third, true, "10.0.0.7", 430000, 10530 );
            registry.CollectForPersist( 10530, persist );
            ASSERT_TRUE( persist.empty() );
            registry.OnReport( third, true, "10.0.0.7", 440000, 10810 );
            registry.CollectForPersist( 10810, persist );
            ASSERT_TRUE( 1 == persist.size() && persist[ 0 ].report.machineId == "a" );

            // a node not heard from for very long is forgotten, and storage is told
            registry.Evaluate( 441000, 12000 );
            CORE::CString::StringVector forgotten;
            registry.TakeForgotten( forgotten );
            ASSERT_TRUE( 1 == forgotten.size() && forgotten[ 0 ] == "u2" );
            ASSERT_TRUE( registry.Find( "a", entry ) && registry.Find( "u1", entry ) && !registry.Find( "u2", entry ) );

            // after a restart of the hub the nodes of storage are unknown, then offline when the window passed with no report
            vector< SNodeEntry > stored;
            registry.GetAll( stored );
            CNodeRegistry restarted( limits );
            restarted.Load( stored, 1000 );
            ASSERT_TRUE( restarted.Find( "a", entry ) && NODELIVENESS_UNKNOWN == entry.liveness && !entry.dirty );
            restarted.Evaluate( 1000 + 30000, 20000 );
            ASSERT_TRUE( restarted.Find( "a", entry ) && NODELIVENESS_UNKNOWN == entry.liveness );
            restarted.Evaluate( 1000 + 61000, 20061 );
            ASSERT_TRUE( restarted.Find( "a", entry ) && NODELIVENESS_OFFLINE == entry.liveness );
            restarted.TakeEvents( events );
            ASSERT_TRUE( events.empty() );
            ASSERT_TRUE( REPORT_ACCEPTED == restarted.OnReport( a, true, "10.0.0.5", 70000, 20070 ) );
            ASSERT_TRUE( restarted.Find( "a", entry ) && NODELIVENESS_ONLINE == entry.liveness && entry.firstSeenInUnixSecs == 10000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Report: a report survives the message, text and lists are cut to their limits" )
        try
        {
            CORE::CString error;
            COMCORE::CRpcContractPtr contract = BuildLabContract( error );
            ASSERT_FALSE( contract.IsNULL() );
            ASSERT_TRUE( 1 == contract->GetMajor() && LabContractMinor == contract->GetMinor() );
            const COMCORE::CRpcMethodDescriptor* method = contract->FindMethod( ReportPath );
            ASSERT_TRUE( GUCEF_NULL != method && 1 == method->GetSince() );
            // the methods of minor 0 are still minor 0
            ASSERT_TRUE( GUCEF_NULL != contract->FindMethod( HelloPath ) && 0 == contract->FindMethod( HelloPath )->GetSince() );
            ASSERT_TRUE( 0 == contract->FindMethod( JoinPath )->GetSince() && 0 == contract->FindMethod( ClientConfigPath )->GetSince() );

            SNodeReport report;
            report.machineId = "0123456789abcdef";
            report.instanceId = "run";
            report.startedInUnixSecs = 1234;
            report.bootId = "boot";
            report.name = "kid-pc\n1";
            report.os = "windows";
            report.arch = "x86_64";
            report.agentVersion = AgentVersion;
            report.uptimeInSecs = 99;
            report.ephemeral = true;
            report.nodeState = NodeStateBusy;
            report.presence = "fullscreen";
            report.idleInSecs = 7;
            report.cpuCores = 12;
            report.cpuLoadPercent = 250;
            report.memTotalInMiB = 32768;
            report.memAvailableInMiB = 20000;
            report.diskFreeInMiB = 500000;
            SGpuInfo gpu;
            gpu.name = "GPU";
            gpu.vendor = "nvidia";
            gpu.vramTotalInMiB = 12288;
            gpu.vramFreeInMiB = 11000;
            gpu.utilizationPercent = 3;
            report.gpus.push_back( gpu );
            SHostInfo host;
            host.name = "wsl2";
            host.state = "ready";
            report.hosts.push_back( host );
            for ( UInt32 i=0; i<MaxReportWorkloads + 10; ++i )
            {
                SWorkloadInfo workload;
                workload.id = "w" + CORE::ToString( i );
                workload.kind = "llm";
                workload.idleSlots = 2;
                report.workloads.push_back( workload );
            }
            report.capabilities.push_back( "mcp" );
            report.capabilities.push_back( "llm" );

            COMCORE::CRpcMessage message( contract, contract->FindType( "ReportRequest" ) );
            ASSERT_TRUE( NodeReportToMessage( report, message ) );
            SNodeReport back;
            ASSERT_TRUE( NodeReportFromMessage( message, back ) );
            ASSERT_TRUE( back.machineId == report.machineId && back.instanceId == "run" && 1234 == back.startedInUnixSecs && back.bootId == "boot" );
            ASSERT_TRUE( back.name == "kid-pc 1" );                       // control characters become spaces
            ASSERT_TRUE( back.os == "windows" && back.arch == "x86_64" && back.agentVersion == AgentVersion && 99 == back.uptimeInSecs && back.ephemeral );
            ASSERT_TRUE( back.nodeState == NodeStateBusy && back.presence == "fullscreen" && 7 == back.idleInSecs );
            ASSERT_TRUE( 12 == back.cpuCores && 100 == back.cpuLoadPercent );   // a percentage is at most 100
            ASSERT_TRUE( 32768 == back.memTotalInMiB && 20000 == back.memAvailableInMiB && 500000 == back.diskFreeInMiB );
            ASSERT_TRUE( 1 == back.gpus.size() && back.gpus[ 0 ].vendor == "nvidia" && 12288 == back.gpus[ 0 ].vramTotalInMiB && 11000 == back.gpus[ 0 ].vramFreeInMiB );
            ASSERT_TRUE( 1 == back.hosts.size() && back.hosts[ 0 ].name == "wsl2" );
            ASSERT_TRUE( MaxReportWorkloads == back.workloads.size() && back.workloads[ 0 ].id == "w0" && 2 == back.workloads[ 0 ].idleSlots );
            ASSERT_TRUE( 2 == back.capabilities.size() && back.capabilities[ 1 ] == "llm" );

            // text over the limit is cut; a report without a machine id is no report; a node state defaults to available
            CORE::CString longName;
            for ( UInt32 i=0; i<MaxReportTextLength + 50; ++i )
                longName += "x";
            report.name = longName;
            COMCORE::CRpcMessage longMessage( contract, contract->FindType( "ReportRequest" ) );
            report.nodeState.Clear();
            ASSERT_TRUE( NodeReportToMessage( report, longMessage ) );
            ASSERT_TRUE( NodeReportFromMessage( longMessage, back ) );
            ASSERT_TRUE( MaxReportTextLength == back.name.Length() && back.nodeState == NodeStateAvailable );
            COMCORE::CRpcMessage empty( contract, contract->FindType( "ReportRequest" ) );
            ASSERT_FALSE( NodeReportFromMessage( empty, back ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Report: nodes report to the hub, the lab secret makes them unverified, an approval verifies, a claim on an approved machine is refused, a silent node goes offline" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CLab lab( "gucefLabLink_TestApp_Report" );
            lab.clusterSettings.reportIntervalInSecs = 2;           // a node is offline after 6 seconds of silence
            ASSERT_TRUE( lab.StartCluster() );
            CORE::CString problem;

            // a machine with the lab secret: its token is its own work, so it is on the hub as unverified
            SSettings secretSettings = lab.MachineSettings( "secret-machine" );
            CStore secretStore( secretSettings.stateDir );
            ASSERT_TRUE( secretStore.WriteSecret( lab.Secret(), problem ) );
            CMachineRole* secretMachine = GUCEF_NEW CMachineRole( secretSettings, secretStore );
            ASSERT_TRUE( secretMachine->Visit( lab.Sighting() ) );
            ASSERT_TRUE( WaitForReports( lab, *secretMachine, 1, 20000 ) );
            SIdentity secretIdentity;
            ASSERT_TRUE( secretStore.LoadIdentity( secretIdentity, problem ) );
            SNodeEntry entry;
            ASSERT_TRUE( lab.cluster->GetRegistry().Find( secretIdentity.machineId, entry ) );
            ASSERT_TRUE( NODELIVENESS_ONLINE == entry.liveness && !entry.verified && entry.address == "127.0.0.1" );
            ASSERT_TRUE( entry.report.name == "test-secret-machine" && entry.report.os == OsName() && entry.report.agentVersion == AgentVersion );
            ASSERT_TRUE( entry.report.memTotalInMiB > 0 && entry.report.cpuCores > 0 && entry.report.diskFreeInMiB > 0 );
            ASSERT_FALSE( entry.report.instanceId.IsNULLOrEmpty() );
            bool hasMcp = false;
            for ( size_t i=0; i<entry.report.capabilities.size(); ++i )
                hasMcp = hasMcp || entry.report.capabilities[ i ] == "mcp";
            ASSERT_TRUE( hasMcp );
            ASSERT_TRUE( !entry.report.hosts.empty() && entry.report.hosts[ 0 ].name == "native" );

            // a machine that was approved: verified, and told the interval of the hub
            SSettings approvedSettings = lab.MachineSettings( "approved-machine" );
            CStore approvedStore( approvedSettings.stateDir );
            CMachineRole approvedMachine( approvedSettings, approvedStore );
            ASSERT_FALSE( approvedMachine.Visit( lab.Sighting() ) );
            ASSERT_TRUE( lab.SetApproval( "test-approved-machine", AccessApproved ) );
            ASSERT_TRUE( approvedMachine.Visit( lab.Sighting() ) );
            ASSERT_TRUE( WaitForReports( lab, approvedMachine, 1, 20000 ) );
            SIdentity approvedIdentity;
            ASSERT_TRUE( approvedStore.LoadIdentity( approvedIdentity, problem ) );
            ASSERT_TRUE( lab.cluster->GetRegistry().Find( approvedIdentity.machineId, entry ) && entry.verified && NODELIVENESS_ONLINE == entry.liveness );

            // a machine with the lab secret that uses the machine id of the approved one: refused, the node stays as it is
            SSettings claimSettings = lab.MachineSettings( "claiming-machine" );
            CStore claimStore( claimSettings.stateDir );
            ASSERT_TRUE( claimStore.WriteSecret( lab.Secret(), problem ) );
            SIdentity claimIdentity;
            claimIdentity.machineId = approvedIdentity.machineId;
            ASSERT_TRUE( claimStore.SaveIdentity( claimIdentity, problem ) );
            CMachineRole claimMachine( claimSettings, claimStore );
            ASSERT_TRUE( claimMachine.Visit( lab.Sighting() ) );
            ASSERT_FALSE( WaitForReports( lab, claimMachine, 1, 4000 ) );
            ASSERT_TRUE( GUCEF_NULL != claimMachine.GetReporter() && 0 == claimMachine.GetReporter()->GetReportsAccepted() );
            ASSERT_TRUE( Contains( claimMachine.GetReporter()->GetLastOutcome(), "Report" ) );
            ASSERT_TRUE( lab.cluster->GetRegistry().Find( approvedIdentity.machineId, entry ) && entry.verified && entry.report.name == "test-approved-machine" );

            // a node that stops reporting is offline when the window has passed; the others stay online
            GUCEF_DELETE secretMachine;
            RunLab( lab, 9000 );
            ASSERT_TRUE( lab.cluster->GetRegistry().Find( secretIdentity.machineId, entry ) && NODELIVENESS_OFFLINE == entry.liveness );
            ASSERT_TRUE( entry.lastSeenInUnixSecs > 0 );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Storage: nodes and their history are kept in the database, trimmed by age and count, and found again after it is opened anew" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_Storage" );
            SSettings settings;
            settings.stateDir = dir.GetPath();
            CORE::CString problem;
            {
                CHubStore store( settings );
                GUCEF_TESTFW_SKIP_IF( !store.Open( problem ), "the database plugins are not available" );
                ASSERT_TRUE( store.IsOpen() );

                vector< SNodeEntry > nodes( 2 );
                nodes[ 0 ].report.machineId = "node-a";
                nodes[ 0 ].report.name = "alpha";
                nodes[ 0 ].report.os = "windows";
                nodes[ 0 ].report.arch = "x86_64";
                nodes[ 0 ].report.agentVersion = AgentVersion;
                nodes[ 0 ].report.instanceId = "run1";
                nodes[ 0 ].report.startedInUnixSecs = 4242;
                nodes[ 0 ].report.nodeState = NodeStateBusy;
                nodes[ 0 ].report.presence = "fullscreen";
                nodes[ 0 ].report.ephemeral = true;
                nodes[ 0 ].report.cpuCores = 8;
                nodes[ 0 ].report.cpuLoadPercent = 40;
                nodes[ 0 ].report.memTotalInMiB = 16384;
                nodes[ 0 ].report.memAvailableInMiB = 9000;
                nodes[ 0 ].report.diskFreeInMiB = 123456;
                SGpuInfo gpu;
                gpu.name = "GPU one";
                gpu.vendor = "nvidia";
                gpu.vramTotalInMiB = 12288;
                gpu.vramFreeInMiB = 1000;
                gpu.utilizationPercent = 97;
                nodes[ 0 ].report.gpus.push_back( gpu );
                SHostInfo host;
                host.name = "wsl2";
                host.state = "ready";
                host.detail = "Ubuntu";
                nodes[ 0 ].report.hosts.push_back( host );
                SWorkloadInfo workload;
                workload.id = "llm1";
                workload.kind = "llm";
                workload.state = "running";
                workload.endpoint = "http://10.0.0.5:8080";
                workload.model = "qwen";
                workload.idleSlots = 3;
                nodes[ 0 ].report.workloads.push_back( workload );
                nodes[ 0 ].report.capabilities.push_back( "mcp" );
                nodes[ 0 ].report.capabilities.push_back( "llm" );
                nodes[ 0 ].verified = true;
                nodes[ 0 ].address = "10.0.0.5";
                nodes[ 0 ].liveness = NODELIVENESS_ONLINE;
                nodes[ 0 ].firstSeenInUnixSecs = 1000;
                nodes[ 0 ].lastSeenInUnixSecs = 2000;
                nodes[ 1 ].report.machineId = "node-b";
                nodes[ 1 ].report.name = "beta";
                nodes[ 1 ].liveness = NODELIVENESS_OFFLINE;
                nodes[ 1 ].lastSeenInUnixSecs = 1500;
                ASSERT_TRUE( store.SaveNodes( nodes, problem ) );

                // writing a node again replaces it
                nodes[ 1 ].report.name = "beta two";
                vector< SNodeEntry > again( 1, nodes[ 1 ] );
                ASSERT_TRUE( store.SaveNodes( again, problem ) );

                vector< SNodeEvent > events;
                for ( UInt32 i=0; i<6; ++i )
                {
                    SNodeEvent event;
                    event.unixSecs = 1000 + i * 100;
                    event.machineId = i < 4 ? "node-a" : "node-b";
                    event.kind = i % 2 ? "offline" : "online";
                    event.detail = "event " + CORE::ToString( i );
                    events.push_back( event );
                }
                ASSERT_TRUE( store.AppendEvents( events, problem ) );
                store.Close();
                ASSERT_FALSE( store.IsOpen() );
            }

            // a new object on the same directory finds all of it
            {
                CHubStore store( settings );
                ASSERT_TRUE( store.Open( problem ) );
                vector< SNodeEntry > nodes;
                ASSERT_TRUE( store.LoadNodes( nodes, problem ) && 2 == nodes.size() );
                const SNodeEntry* a = nodes[ 0 ].report.machineId == "node-a" ? &nodes[ 0 ] : &nodes[ 1 ];
                const SNodeEntry* b = nodes[ 0 ].report.machineId == "node-b" ? &nodes[ 0 ] : &nodes[ 1 ];
                ASSERT_TRUE( a->report.name == "alpha" && a->report.os == "windows" && a->report.arch == "x86_64" && a->report.agentVersion == AgentVersion );
                ASSERT_TRUE( a->verified && a->address == "10.0.0.5" && NODELIVENESS_ONLINE == a->liveness && 1000 == a->firstSeenInUnixSecs && 2000 == a->lastSeenInUnixSecs );
                ASSERT_TRUE( a->report.instanceId == "run1" && 4242 == a->report.startedInUnixSecs && a->report.nodeState == NodeStateBusy && a->report.presence == "fullscreen" && a->report.ephemeral );
                ASSERT_TRUE( 8 == a->report.cpuCores && 40 == a->report.cpuLoadPercent && 16384 == a->report.memTotalInMiB && 9000 == a->report.memAvailableInMiB && 123456 == a->report.diskFreeInMiB );
                ASSERT_TRUE( 1 == a->report.gpus.size() && a->report.gpus[ 0 ].name == "GPU one" && 12288 == a->report.gpus[ 0 ].vramTotalInMiB && 97 == a->report.gpus[ 0 ].utilizationPercent );
                ASSERT_TRUE( 1 == a->report.hosts.size() && a->report.hosts[ 0 ].detail == "Ubuntu" );
                ASSERT_TRUE( 1 == a->report.workloads.size() && a->report.workloads[ 0 ].endpoint == "http://10.0.0.5:8080" && 3 == a->report.workloads[ 0 ].idleSlots );
                ASSERT_TRUE( 2 == a->report.capabilities.size() && a->report.capabilities[ 1 ] == "llm" );
                ASSERT_TRUE( b->report.name == "beta two" && NODELIVENESS_OFFLINE == b->liveness && 1500 == b->lastSeenInUnixSecs && !b->verified );

                // the history of a node, oldest first
                vector< SNodeEvent > events;
                ASSERT_TRUE( store.LoadEvents( "node-a", events, problem ) && 4 == events.size() );
                ASSERT_TRUE( events[ 0 ].detail == "event 0" && events[ 3 ].detail == "event 3" && 1000 == events[ 0 ].unixSecs );
                ASSERT_TRUE( store.LoadEvents( "node-b", events, problem ) && 2 == events.size() );

                // trimmed by age: three days of events and a retention of two days at 1000 + 3 days
                ASSERT_TRUE( store.PurgeEvents( 1300 + 2 * 86400, 2, 0, problem ) );       // only those before 1300 are older than two days
                ASSERT_TRUE( store.LoadEvents( "node-a", events, problem ) && 1 == events.size() && events[ 0 ].detail == "event 3" );
                ASSERT_TRUE( store.LoadEvents( "node-b", events, problem ) && 2 == events.size() );
                // and by count: of the three that are left one stays
                ASSERT_TRUE( store.PurgeEvents( 1300 + 2 * 86400, 2, 1, problem ) );
                vector< SNodeEvent > left;
                ASSERT_TRUE( store.LoadEvents( "node-a", events, problem ) && store.LoadEvents( "node-b", left, problem ) );
                ASSERT_TRUE( events.size() + left.size() == 1 && ( left.size() == 1 && left[ 0 ].detail == "event 5" ) );

                // nodes can be removed
                CORE::CString::StringVector ids;
                ids.push_back( "node-a" );
                ASSERT_TRUE( store.DeleteNodes( ids, problem ) );
                ASSERT_TRUE( store.LoadNodes( nodes, problem ) && 1 == nodes.size() && nodes[ 0 ].report.machineId == "node-b" );
            }

            // a database that is switched off, or of a type that is not known, does not open
            SSettings off = settings;
            off.dbType = "none";
            CHubStore none( off );
            ASSERT_FALSE( none.Open( problem ) );
            off.dbType = "nonsense";
            CHubStore nonsense( off );
            ASSERT_FALSE( nonsense.Open( problem ) );
            ASSERT_TRUE( Contains( problem, "not known" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Storage: a hub that restarts knows its nodes again, as unknown until they report, with their last seen; and runs on without the database" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CLab lab( "gucefLabLink_TestApp_Restart" );
            lab.clusterSettings.reportIntervalInSecs = 2;
            ASSERT_TRUE( lab.StartCluster() );
            CORE::CString problem;
            SSettings settings = lab.MachineSettings( "machine" );
            CStore store( settings.stateDir );
            ASSERT_TRUE( store.WriteSecret( lab.Secret(), problem ) );
            CMachineRole machine( settings, store );
            ASSERT_TRUE( machine.Visit( lab.Sighting() ) );
            ASSERT_TRUE( WaitForReports( lab, machine, 1, 20000 ) );
            SIdentity identity;
            ASSERT_TRUE( store.LoadIdentity( identity, problem ) );
            SNodeEntry before;
            ASSERT_TRUE( lab.cluster->GetRegistry().Find( identity.machineId, before ) && NODELIVENESS_ONLINE == before.liveness );

            // the hub stops: the node it knew is written
            GUCEF_DELETE lab.cluster;
            lab.cluster = GUCEF_NULL;
            ASSERT_TRUE( CORE::DirExists( lab.Path( "cluster/db/db" ) ) );

            // and a hub on the same state directory knows the node again, as unknown, with its identity and last seen
            ASSERT_TRUE( lab.StartCluster() );
            SNodeEntry after;
            ASSERT_TRUE( lab.cluster->GetRegistry().Find( identity.machineId, after ) );
            ASSERT_TRUE( NODELIVENESS_UNKNOWN == after.liveness && after.report.name == "test-machine" && after.firstSeenInUnixSecs == before.firstSeenInUnixSecs );
            ASSERT_TRUE( after.lastSeenInUnixSecs >= before.lastSeenInUnixSecs - 1 && after.report.instanceId == before.report.instanceId );
            ASSERT_TRUE( after.report.memTotalInMiB > 0 && !after.report.capabilities.empty() );

            // the node reports again, to the new hub: online, and the history holds the events of both
            ASSERT_TRUE( WaitForReports( lab, machine, 2, 20000 ) );
            ASSERT_TRUE( lab.cluster->GetRegistry().Find( identity.machineId, after ) && NODELIVENESS_ONLINE == after.liveness );
            GUCEF_DELETE lab.cluster;
            lab.cluster = GUCEF_NULL;
            CHubStore history( lab.clusterSettings );
            ASSERT_TRUE( history.Open( problem ) );
            vector< SNodeEvent > events;
            ASSERT_TRUE( history.LoadEvents( identity.machineId, events, problem ) && events.size() >= 2 );
            ASSERT_TRUE( events[ 0 ].kind == "online" && events[ 0 ].detail == "first report" );
            history.Close();

            // without a database the hub still works: the nodes are in memory only
            lab.clusterSettings.dbType = "none";
            ASSERT_TRUE( lab.StartCluster() );
            SNodeEntry none;
            ASSERT_FALSE( lab.cluster->GetRegistry().Find( identity.machineId, none ) );
            ASSERT_TRUE( WaitForReports( lab, machine, 3, 20000 ) );
            ASSERT_TRUE( lab.cluster->GetRegistry().Find( identity.machineId, none ) && NODELIVENESS_ONLINE == none.liveness );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
