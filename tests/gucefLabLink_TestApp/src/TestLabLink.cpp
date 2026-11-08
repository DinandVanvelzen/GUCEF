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
#include "gucefLabLink_Machine.h"
#include "TestLabLink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

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
    if ( NULL == file )
        return false;
    fwrite( text.C_String(), 1, text.Length(), file );
    fclose( file );
    if ( executable )
        chmod( path.C_String(), 0755 );
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
}

/*-------------------------------------------------------------------------*/
