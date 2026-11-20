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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <unistd.h>
#endif

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

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#include "gucefLabLink_Cluster.h"
#include "gucefLabLink_CiFiles.h"
#include "gucefLabLink_CiGatewayTokens.h"
#include "gucefLabLink_CiHub.h"
#include "gucefLabLink_CiRegistryFile.h"
#include "gucefLabLink_Machine.h"
#include "TestLabLinkCi.h"

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

/** A registry in the form of gucefMcpCi, with a gateway object of its own that the hub replaces */
const char* Registry = "{\n"
                       "  \"gateway\": { \"url\": \"https://elsewhere.invalid:1\" },\n"
                       "  \"ci\": [\n"
                       "    { \"id\": \"GUCEF\", \"backend\": \"forgejo\", \"url\": \"http://forgejo.invalid:32001\", \"owner\": \"o\", \"repo\": \"GUCEF\",\n"
                       "      \"scratchOwner\": \"ci\", \"scratchRepo\": \"GUCEF\", \"workflow\": \"gucef-ci.yml\", \"retentionDays\": 14,\n"
                       "      \"remotes\": [ \"https://github.com/o/GUCEF.git\" ] }\n"
                       "  ],\n"
                       "  \"note\": \"kept as it is\"\n"
                       "}\n";

/*-------------------------------------------------------------------------*/

bool
LoadCryptography( void )
{
    COM::CComGlobal::Instance();
    DRN::CDRNGlobal::Instance();
    if ( DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsCipherAvailable( "xchacha20-poly1305" ) )
        return true;
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
WriteText( const CORE::CString& path, const CORE::CString& text )
{
    CORE::CreatePathDirectories( path );
    FILE* file = fopen( path.C_String(), "wb" );
    if ( GUCEF_NULL == file )
        return false;
    fwrite( text.C_String(), 1, text.Length(), file );
    fclose( file );
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

UInt32
Count( const CORE::CString& text, const CORE::CString& part )
{
    UInt32 count = 0;
    Int32 at = text.HasSubstr( part, (Int32) 0, true );
    while ( at >= 0 )
    {
        ++count;
        at = text.HasSubstr( part, (Int32) ( at + (Int32) part.Length() ), true );
    }
    return count;
}

/*-------------------------------------------------------------------------*/

bool
IsHex64( const CORE::CString& text )
{
    if ( 64 != text.Length() )
        return false;
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        const UInt32 c = text[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The mode bits, owner and inode of a path, without following a link; mode 0 when it is not there */
struct SPathInfo
{
    UInt32 mode;
    UInt32 uid;
    UInt64 inode;

    SPathInfo( void ) : mode( 0 ), uid( 0 ), inode( 0 ) {}
};

SPathInfo
InfoOf( const CORE::CString& path )
{
    SPathInfo info;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    struct stat st;
    if ( 0 == ::lstat( path.C_String(), &st ) )
    {
        info.mode = (UInt32) ( st.st_mode & 07777 );
        info.uid = (UInt32) st.st_uid;
        info.inode = (UInt64) st.st_ino;
    }
    #endif
    return info;
}

/*-------------------------------------------------------------------------*/

/** An approval in the store of a hub */
bool
AddApproval( CStore& store, const CORE::CString& machineId, const char* name, const char* state )
{
    CORE::CString problem;
    GUCEF::vector< SApproval > approvals;
    if ( !store.LoadApprovals( approvals, problem ) )
        return false;
    SApproval approval;
    approval.state = state;
    approval.machineId = machineId;
    approval.name = name;
    approval.address = "127.0.0.1";
    approval.os = "linux";
    approvals.push_back( approval );
    return store.SaveApprovals( approvals, problem );
}

/*-------------------------------------------------------------------------*/

/** A hub with a CI registry and machines with their own state and MCP directories, over loopback */
class CCiLab
{
    public:

    TESTFW::CScopedTestDir dir;
    CStore clusterStore;
    SSettings clusterSettings;
    CClusterRole* cluster;

    CCiLab( const char* name )
        : dir( name )
        , clusterStore( Path( "cluster" ) )
        , clusterSettings()
        , cluster( GUCEF_NULL )
    {
        CORE::CreateDirs( Path( "mcp/tls" ) );
        CORE::CreateDirs( Path( "cluster" ) );
        CORE::CreateDirs( Path( "bin" ) );
        WriteText( Path( "mcp/tls/ca.crt" ), "-----BEGIN CERTIFICATE-----\nci test authority\n-----END CERTIFICATE-----\n" );
        WriteText( Path( "mcp/lab-token" ), "the-lab-mcp-token\n" );
        WriteText( Path( "mcp/mcp-nodes" ), "" );
        WriteText( Path( "bin/no-k3s" ), "#!/bin/sh\nexit 1\n" );
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
        ::chmod( Path( "bin/no-k3s" ).C_String(), 0755 );
        #endif

        clusterSettings.stateDir = Path( "cluster" );
        clusterSettings.clusterName = "citestlab";
        clusterSettings.labPort = TESTFW::CTlsTestServer::FindFreePort();
        clusterSettings.accessPort = TESTFW::CTlsTestServer::FindFreePort();
        while ( clusterSettings.accessPort == clusterSettings.labPort )
            clusterSettings.accessPort = TESTFW::CTlsTestServer::FindFreePort();
        clusterSettings.useMdns = false;
        clusterSettings.useBroadcast = false;
        clusterSettings.k3sBin = Path( "bin/no-k3s" );
        clusterSettings.mcpDir = Path( "mcp" );
        clusterSettings.dbType = "none";
        clusterSettings.ciGatewayUrl = "https://hub.test:8792";
    }

    ~CCiLab()
    {
        GUCEF_DELETE cluster;
    }

    CORE::CString Path( const char* name ) const { return CORE::CombinePath( dir.GetPath(), name ); }

    CORE::CString RegistryPath( void ) const { return Path( "cluster/ci.json" ); }

    bool StartCluster( void )
    {
        CORE::CString problem;
        CORE::CString::StringVector ranges;
        ranges.push_back( "127.0.0.0/8" );
        clusterSettings.allowedRanges.Set( ranges, problem );
        CORE::CString secret;
        if ( !clusterStore.ReadSecret( secret, problem ) && !clusterStore.WriteSecret( MakeLabSecret(), problem ) )
            return false;
        cluster = GUCEF_NEW CClusterRole( clusterSettings, clusterStore );
        if ( cluster->Start( problem ) )
            return true;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "LabLink CI test: the cluster does not start: " << problem );
        return false;
    }

    CORE::CString Secret( void ) const
    {
        CORE::CString secret, problem;
        clusterStore.ReadSecret( secret, problem );
        return secret;
    }

    SSettings MachineSettings( const char* name ) const
    {
        SSettings settings;
        settings.stateDir = Path( name );
        settings.machineName = CORE::CString( "citest-" ) + name;
        settings.useMdns = false;
        settings.useBroadcast = false;
        settings.report = false;
        settings.mcpDir = Path( ( CORE::CString( name ) + "-mcp" ).C_String() );
        settings.claudeUser = CORE::GetEnv( "USER" );       // the files are given to the user that runs the test: itself
        CORE::CString problem;
        CORE::CString::StringVector ranges;
        ranges.push_back( "127.0.0.0/8" );
        settings.allowedRanges.Set( ranges, problem );
        return settings;
    }

    CORE::CString CiDir( const char* name ) const { return CORE::CombinePath( Path( ( CORE::CString( name ) + "-mcp" ).C_String() ), "ci" ); }

    SClusterSighting Sighting( void ) const
    {
        SClusterSighting sighting;
        sighting.name = "citestlab";
        sighting.address = "127.0.0.1";
        sighting.key = cluster->GetLabKey();
        sighting.labPort = clusterSettings.labPort;
        sighting.accessPort = clusterSettings.accessPort;
        return sighting;
    }

    bool SetApproval( const CORE::CString& machineName, const char* state )
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

    /** The machine asks for access, is approved and links */
    bool Approve( CMachineRole& machine, const SSettings& settings )
    {
        machine.Visit( Sighting() );
        return SetApproval( settings.machineName, AccessApproved ) && machine.Visit( Sighting() );
    }

    CORE::CString MachineIdOf( const SSettings& settings ) const
    {
        CStore store( settings.stateDir );
        SIdentity identity;
        CORE::CString problem;
        store.LoadIdentity( identity, problem );
        return identity.machineId;
    }

    CORE::CString HashFile( void ) const { return ReadText( Path( "cluster/ci/gateway-tokens" ) ); }
};

/*-------------------------------------------------------------------------*/

/** A role set of one approval token */
CORE::CString::StringSet
Roles( const char* role )
{
    CORE::CString::StringSet roles;
    roles.insert( role );
    return roles;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformLabLinkCiTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "LabLinkCi" );

    GUCEF_TESTFW_TESTCASE( "CI contract: gucef.lab 1.2 adds CiConfig, its answer survives the message, the methods of minor 1 and 0 are unchanged" )
        try
        {
            CORE::CString error;
            COMCORE::CRpcContractPtr contract = BuildLabContract( error );
            ASSERT_FALSE( contract.IsNULL() );
            ASSERT_TRUE( 1 == contract->GetMajor() && 2 == contract->GetMinor() && 2 == LabContractMinor );
            const COMCORE::CRpcMethodDescriptor* method = contract->FindMethod( CiConfigPath );
            ASSERT_TRUE( GUCEF_NULL != method && 2 == method->GetSince() );
            ASSERT_TRUE( 1 == contract->FindMethod( ReportPath )->GetSince() && 0 == contract->FindMethod( HelloPath )->GetSince() );
            ASSERT_TRUE( 0 == contract->FindMethod( JoinPath )->GetSince() && 0 == contract->FindMethod( ClientConfigPath )->GetSince() );

            COMCORE::CRpcMessage response( contract, contract->FindType( "CiConfigResponse" ) );
            response.SetString( 1, Registry );
            response.SetString( 2, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" );
            response.SetString( 3, "-----BEGIN CERTIFICATE-----\nx\n-----END CERTIFICATE-----\n" );
            response.SetString( 4, "https://hub.test:8792" );
            ASSERT_TRUE( response.GetString( 1 ) == Registry && response.GetString( 4 ) == "https://hub.test:8792" );
            ASSERT_TRUE( IsHex64( response.GetString( 2 ) ) && Contains( response.GetString( 3 ), "BEGIN CERTIFICATE" ) );
            COMCORE::CRpcMessage request( contract, contract->FindType( "CiConfigRequest" ) );
            ASSERT_TRUE( 0 == request.GetCount( 1 ) );

            // the contract of a hub of minor 1 is that of before: without CiConfig
            COMCORE::CRpcContractPtr older = BuildLabContractOfMinor( 1, error );
            ASSERT_FALSE( older.IsNULL() );
            ASSERT_TRUE( 1 == older->GetMinor() && GUCEF_NULL == older->FindMethod( CiConfigPath ) && GUCEF_NULL != older->FindMethod( ReportPath ) );
            ASSERT_TRUE( GUCEF_NULL == older->FindType( "CiConfigResponse" ) );
            ASSERT_TRUE( BuildLabContractOfMinor( 3, error ).IsNULL() && !error.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CI registry: missing, too big, not JSON, not an object, without a list; the gateway object of the hub; read again only when the file changed" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !CCiRegistryFile::IsJsonCodecAvailable(), "dstorepluginJSONPARSER is not available" );
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_CiRegistry" );
            const CORE::CString path = CORE::CombinePath( dir.GetPath(), "ci.json" );
            CCiRegistryFile registry( path, "https://hub.test:8792" );
            CORE::CString merged, problem;

            ASSERT_TRUE( CIREGISTRY_MISSING == registry.Get( merged, problem ) && merged.IsNULLOrEmpty() );

            // the gateway object of the hub replaces that of the file; the other members are passed on as they are
            ASSERT_TRUE( WriteText( path, Registry ) );
            ASSERT_TRUE( CIREGISTRY_OK == registry.Get( merged, problem ) );
            ASSERT_TRUE( 1 == Count( merged, "\"gateway\"" ) && !Contains( merged, "elsewhere.invalid" ) );
            ASSERT_TRUE( Contains( merged, "\"gateway\": { \"url\": \"https://hub.test:8792\", \"caFile\": \"ca.crt\" }" ) );
            ASSERT_TRUE( Contains( merged, "\"note\": \"kept as it is\"" ) && Contains( merged, "\"remotes\": [ \"https://github.com/o/GUCEF.git\" ]" ) );
            CORE::CString again, problemAgain;
            ASSERT_TRUE( CIREGISTRY_OK == CCiRegistryFile::Merge( merged, "https://hub.test:8792", again, problemAgain ) );
            ASSERT_TRUE( again == merged );       // the merged text is valid JSON and merging it again changes nothing

            // a registry without a gateway object of its own gets one
            ASSERT_TRUE( CIREGISTRY_OK == CCiRegistryFile::Merge( "{\"ci\":[]}", "https://h:1", again, problem ) );
            ASSERT_TRUE( Contains( again, "\"url\": \"https://h:1\"" ) && Contains( again, "\"ci\":[]" ) );

            // what is refused
            const char* wrong[] = { "{ \"ci\": [ ", "[ { \"id\": \"GUCEF\" } ]", "{ \"other\": [] }", "{ \"ci\": { \"id\": \"GUCEF\" } }", "", "{ \"ci\": [] } trailing" };
            for ( size_t i=0; i<sizeof( wrong ) / sizeof( wrong[ 0 ] ); ++i )
            {
                ASSERT_TRUE( CIREGISTRY_INVALID == CCiRegistryFile::Merge( wrong[ i ], "https://h:1", again, problem ) );
                ASSERT_FALSE( problem.IsNULLOrEmpty() );
            }
            CORE::CString big = "{ \"ci\": [], \"pad\": \"";
            while ( big.Length() <= CCiRegistryFile::MaxSizeInBytes )
                big += "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
            big += "\" }";
            ASSERT_TRUE( CIREGISTRY_INVALID == CCiRegistryFile::Merge( big, "https://h:1", again, problem ) && Contains( problem, "64 KiB" ) );
            ASSERT_TRUE( WriteText( path, big ) );
            ASSERT_TRUE( CIREGISTRY_INVALID == registry.Get( merged, problem ) && merged.IsNULLOrEmpty() && Contains( problem, "64 KiB" ) );
            ASSERT_TRUE( WriteText( path, "{ not json" ) );
            ASSERT_TRUE( CIREGISTRY_INVALID == registry.Get( merged, problem ) );

            // read again when it changes, not on every call: the same size and time is the same file
            ASSERT_TRUE( WriteText( path, Registry ) );
            ASSERT_TRUE( CIREGISTRY_OK == registry.Get( merged, problem ) );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            struct stat before;
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &before ) );
            CORE::CString sameSize = Registry;
            sameSize = sameSize.ReplaceSubstr( "kept as it is", "KEPT AS IT IS" );
            ASSERT_TRUE( WriteText( path, sameSize ) );
            struct timeval times[ 2 ];
            times[ 0 ].tv_sec = before.st_atim.tv_sec; times[ 0 ].tv_usec = before.st_atim.tv_nsec / 1000;
            times[ 1 ].tv_sec = before.st_mtim.tv_sec; times[ 1 ].tv_usec = before.st_mtim.tv_nsec / 1000;
            ASSERT_TRUE( 0 == ::utimes( path.C_String(), times ) );
            struct stat touched;
            ASSERT_TRUE( 0 == ::stat( path.C_String(), &touched ) );
            if ( touched.st_mtim.tv_nsec / 1000000 == before.st_mtim.tv_nsec / 1000000 )
            {
                ASSERT_TRUE( CIREGISTRY_OK == registry.Get( merged, problem ) && Contains( merged, "kept as it is" ) );
            }
            ::utimes( path.C_String(), GUCEF_NULL );
            #endif
            ASSERT_TRUE( WriteText( path, CORE::CString( Registry ).ReplaceSubstr( "kept as it is", "changed" ) ) );
            ASSERT_TRUE( CIREGISTRY_OK == registry.Get( merged, problem ) && Contains( merged, "\"note\": \"changed\"" ) );
            ASSERT_TRUE( CORE::DeleteFile( path ) );
            ASSERT_TRUE( CIREGISTRY_MISSING == registry.Get( merged, problem ) );
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

    GUCEF_TESTFW_TESTCASE( "CI gateway tokens: one per machine and the same again, distinct per machine, bounded, the hash file, removal and approvals" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_CiTokens" );
            CStore store( dir.GetPath() );
            CCiGatewayTokens tokens( store );
            const CORE::CString a = "aaaaaaaaaaaaaaaa0000000000000001";
            const CORE::CString b = "bbbbbbbbbbbbbbbb0000000000000002";
            CORE::CString tokenA, tokenB, again, problem;
            bool made = false, full = false;

            // nothing made, no hash file: a hub without tokens says nothing
            UInt32 removed = 0;
            ASSERT_TRUE( tokens.Reconcile( GUCEF::vector< SApproval >(), removed, problem ) && 0 == removed );
            ASSERT_FALSE( CORE::FileExists( tokens.GetHashFilePath() ) );

            ASSERT_TRUE( tokens.GetOrMake( a, tokenA, made, full, problem ) && made && IsHex64( tokenA ) );
            ASSERT_TRUE( tokens.GetOrMake( a, again, made, full, problem ) && !made && again == tokenA );
            ASSERT_TRUE( tokens.GetOrMake( b, tokenB, made, full, problem ) && made && IsHex64( tokenB ) && tokenB != tokenA );
            ASSERT_FALSE( tokens.GetOrMake( "short", again, made, full, problem ) );

            // the token store and the hash file are their owner's only; the hash file holds hashes and machine ids
            const CORE::CString hashes = ReadText( tokens.GetHashFilePath() );
            ASSERT_TRUE( Contains( hashes, Sha256Hex( tokenA ) + "\t" + a + "\n" ) && Contains( hashes, Sha256Hex( tokenB ) + "\t" + b + "\n" ) );
            ASSERT_FALSE( Contains( hashes, tokenA ) || Contains( hashes, tokenB ) );
            ASSERT_TRUE( hashes.StartsWith( "#" ) && hashes == CCiGatewayTokens::HashFileContent( CCiGatewayTokens::TTokenMap() ) +
                         Sha256Hex( tokenA ) + "\t" + a + "\n" + Sha256Hex( tokenB ) + "\t" + b + "\n" );
            CORE::CString::StringVector lines = hashes.ParseElements( '\n', false );
            UInt32 entries = 0;
            for ( size_t i=0; i<lines.size(); ++i )
            {
                if ( lines[ i ].IsNULLOrEmpty() || '#' == lines[ i ][ 0 ] )
                    continue;
                CORE::CString::StringVector fields = lines[ i ].ParseElements( '\t', true );
                ASSERT_TRUE( 2 == fields.size() && IsHex64( fields[ 0 ] ) && fields[ 0 ] == fields[ 0 ].Lowercase() );
                ++entries;
            }
            ASSERT_TRUE( 2 == entries );
            ASSERT_TRUE( Sha256Hex( "abc" ) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0600 == InfoOf( tokens.GetHashFilePath() ).mode && 0600 == InfoOf( store.GetPath( "ci-gateway-tokens" ) ).mode );
            #endif

            // removing takes the token and its line away; a hash file that went missing is made again
            CORE::CString::StringVector ids;
            ids.push_back( a );
            ASSERT_TRUE( tokens.Remove( ids, removed, problem ) && 1 == removed );
            ASSERT_FALSE( Contains( ReadText( tokens.GetHashFilePath() ), a ) );
            ASSERT_TRUE( tokens.GetOrMake( a, again, made, full, problem ) && made && again != tokenA );
            ASSERT_TRUE( CORE::DeleteFile( tokens.GetHashFilePath() ) );
            ASSERT_TRUE( tokens.GetOrMake( b, again, made, full, problem ) && !made && again == tokenB );
            ASSERT_TRUE( Contains( ReadText( tokens.GetHashFilePath() ), b ) );

            // only approved machines keep theirs
            GUCEF::vector< SApproval > approvals;
            SApproval approval;
            approval.machineId = b;
            approval.state = AccessApproved;
            approvals.push_back( approval );
            approval.machineId = a;
            approval.state = AccessRefused;
            approvals.push_back( approval );
            ASSERT_TRUE( tokens.Reconcile( approvals, removed, problem ) && 1 == removed );
            CCiGatewayTokens::TTokenMap loaded;
            ASSERT_TRUE( tokens.Load( loaded, problem ) && 1 == loaded.size() && loaded[ b ] == tokenB );
            ASSERT_TRUE( tokens.Reconcile( approvals, removed, problem ) && 0 == removed );

            // bounded
            for ( UInt32 i=0; i<CCiGatewayTokens::MaxMachines + 2; ++i )
            {
                CORE::CString id = "cccccccccccccccc" + CORE::ToString( 1000 + i );
                if ( !tokens.GetOrMake( id, again, made, full, problem ) )
                    break;
            }
            ASSERT_TRUE( full && Contains( problem, "256" ) );
            ASSERT_TRUE( tokens.Load( loaded, problem ) && CCiGatewayTokens::MaxMachines == loaded.size() );
            ASSERT_TRUE( tokens.GetOrMake( b, again, made, full, problem ) && again == tokenB );   // one that has a token still gets it
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

    GUCEF_TESTFW_TESTCASE( "CI gateway tokens file: mode 0600 after it is made, rewritten and revoked, given to the gateway user, a path of its own, accepted by ReadOwnerOnlyFile" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_CiTokenFile" );
            const CORE::CString stateDir = CORE::CombinePath( dir.GetPath(), "state" );
            CORE::CreateDirs( stateDir );
            CStore store( stateDir );
            const CORE::CString a = "aaaaaaaaaaaaaaaa0000000000000011";
            const CORE::CString b = "bbbbbbbbbbbbbbbb0000000000000022";
            CORE::CString tokenA, tokenB, problem;
            bool made = false, full = false;
            UInt32 removed = 0;

            // the default place, given to the gateway user: here the user that runs the test, the one user an
            // unprivileged process can give a file to
            const CORE::CString user = CORE::GetEnv( "USER" );
            CCiGatewayTokens tokens( store, CORE::CString::Empty, user );
            ASSERT_TRUE( tokens.GetHashFilePath() == CORE::CombinePath( CORE::CombinePath( stateDir, "ci" ), "gateway-tokens" ) );
            ASSERT_TRUE( tokens.GetOrMake( a, tokenA, made, full, problem ) && made );
            const CORE::CString path = tokens.GetHashFilePath();
            CORE::CDynamicBuffer content;
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0600 == InfoOf( path ).mode && (UInt32) ::geteuid() == InfoOf( path ).uid );
            ASSERT_TRUE( 0755 == InfoOf( CORE::CombinePath( stateDir, "ci" ) ).mode );
            ASSERT_TRUE( CORE::ReadOwnerOnlyFile( path, content, problem ) );
            const UInt64 inode = InfoOf( path ).inode;
            #endif
            ASSERT_TRUE( CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) ==
                         CCiGatewayTokens::HashFileContent( CCiGatewayTokens::TTokenMap() ) + Sha256Hex( tokenA ) + "\t" + a + "\n" );

            // rewritten for a new token, the same again for a request that changes nothing, after a revoke
            ASSERT_TRUE( tokens.GetOrMake( b, tokenB, made, full, problem ) && made );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0600 == InfoOf( path ).mode && inode != InfoOf( path ).inode && CORE::ReadOwnerOnlyFile( path, content, problem ) );
            const UInt64 rewritten = InfoOf( path ).inode;
            ::chmod( path.C_String(), 0644 );          // widened by hand: narrowed again, without a rewrite
            ASSERT_TRUE( tokens.GetOrMake( b, tokenB, made, full, problem ) && !made );
            ASSERT_TRUE( 0600 == InfoOf( path ).mode && rewritten == InfoOf( path ).inode );
            #endif
            CORE::CString::StringVector ids;
            ids.push_back( a );
            ASSERT_TRUE( tokens.Remove( ids, removed, problem ) && 1 == removed );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0600 == InfoOf( path ).mode && CORE::ReadOwnerOnlyFile( path, content, problem ) );
            #endif
            ASSERT_TRUE( ReadText( path ) == CCiGatewayTokens::HashFileContent( CCiGatewayTokens::TTokenMap() ) + Sha256Hex( tokenB ) + "\t" + b + "\n" );

            // a gateway user that does not exist: the file stays with the account of the hub, said once in the log
            {
                TESTFW::CLogTextCapture capture;
                CCiGatewayTokens strange( store, CORE::CString::Empty, "no-such-user-gucef-lablink" );
                ASSERT_TRUE( strange.Remove( ids, removed, problem ) );
                ASSERT_TRUE( strange.GetOrMake( a, tokenA, made, full, problem ) && made );
                ASSERT_TRUE( capture.Contains( "stay with the account of the hub" ) && 1 == Count( capture.GetText(), "no-such-user-gucef-lablink does not exist" ) );
                #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
                ASSERT_TRUE( 0600 == InfoOf( path ).mode && (UInt32) ::geteuid() == InfoOf( path ).uid );
                #endif
            }

            // a path of its own: used, in a directory that exists; a missing directory is not made, and said once
            const CORE::CString gatewayDir = CORE::CombinePath( dir.GetPath(), "gateway" );
            CORE::CreateDirs( gatewayDir );
            const CORE::CString custom = CORE::CombinePath( gatewayDir, "tokens.sha256" );
            CCiGatewayTokens elsewhere( store, custom, user );
            ASSERT_TRUE( elsewhere.GetHashFilePath() == custom );
            ASSERT_TRUE( elsewhere.Remove( ids, removed, problem ) && 1 == removed );
            ASSERT_TRUE( elsewhere.GetOrMake( a, tokenA, made, full, problem ) && made );
            ASSERT_TRUE( Contains( ReadText( custom ), Sha256Hex( tokenA ) + "\t" + a + "\n" ) && Contains( ReadText( custom ), Sha256Hex( tokenB ) + "\t" + b + "\n" ) );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0600 == InfoOf( custom ).mode && CORE::ReadOwnerOnlyFile( custom, content, problem ) );
            #endif
            const CORE::CString missingDir = CORE::CombinePath( CORE::CombinePath( dir.GetPath(), "not-made" ), "tokens" );
            {
                TESTFW::CLogTextCapture capture;
                CCiGatewayTokens nowhere( store, missingDir, CORE::CString::Empty );
                ASSERT_FALSE( nowhere.GetOrMake( b, tokenB, made, full, problem ) );
                ASSERT_TRUE( Contains( problem, "does not exist" ) );
                ASSERT_FALSE( nowhere.Remove( ids, removed, problem ) );
                ASSERT_TRUE( 1 == Count( capture.GetText(), "of the CI gateway tokens does not exist" ) );
                ASSERT_FALSE( CORE::DirExists( CORE::CombinePath( dir.GetPath(), "not-made" ) ) );
            }

            // the hub takes both settings
            SSettings settings;
            settings.stateDir = stateDir;
            settings.ciGatewayTokensFile = custom;
            settings.ciGatewayUser = user;
            CCiHub hub( settings, store );
            ASSERT_TRUE( hub.GetTokens().GetHashFilePath() == custom );
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

    GUCEF_TESTFW_TESTCASE( "CI hub: an approval token gets the registry, its own token and the authority; a lab secret token and a machine that is not approved are refused and logged; no registry, no CI" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            GUCEF_TESTFW_SKIP_IF( !CCiRegistryFile::IsJsonCodecAvailable(), "dstorepluginJSONPARSER is not available" );
            TESTFW::CLogTextCapture capture;
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_CiHub" );
            SSettings settings;
            settings.stateDir = CORE::CombinePath( dir.GetPath(), "state" );
            settings.mcpDir = CORE::CombinePath( dir.GetPath(), "mcp" );
            settings.clusterName = "hub";
            ASSERT_TRUE( WriteText( CORE::CombinePath( settings.mcpDir, "tls/ca.crt" ), "the authority\n" ) );
            CStore store( settings.stateDir );
            CORE::CreateDirs( settings.stateDir );
            const CORE::CString approved = "1111111111111111aaaaaaaaaaaaaaaa";
            const CORE::CString other = "2222222222222222bbbbbbbbbbbbbbbb";
            const CORE::CString pending = "3333333333333333cccccccccccccccc";
            ASSERT_TRUE( AddApproval( store, approved, "approved-pc", AccessApproved ) && AddApproval( store, other, "other-pc", AccessApproved ) );
            ASSERT_TRUE( AddApproval( store, pending, "pending-pc", AccessPending ) );

            CCiHub hub( settings, store );
            hub.Start();
            ASSERT_TRUE( CCiHub::GatewayUrlOf( settings ) == "https://hub:8792" );
            ASSERT_TRUE( CCiHub::RegistryFileOf( settings ) == CORE::CombinePath( settings.stateDir, "ci.json" ) );
            SCiConfigAnswer answer;

            // without a registry: no CI, and no token is made
            COMCORE::CRpcStatus status = hub.Answer( "drn:" + approved, Roles( RoleApproved ), "10.0.0.7", answer );
            ASSERT_TRUE( COMCORE::RPC_STATUS_NOT_FOUND == status.code && status.message == CCiHub::NoCiConfigured );
            CCiGatewayTokens::TTokenMap tokens;
            CORE::CString problem;
            ASSERT_TRUE( hub.GetTokens().Load( tokens, problem ) && tokens.empty() );

            ASSERT_TRUE( WriteText( CCiHub::RegistryFileOf( settings ), Registry ) );
            status = hub.Answer( "drn:" + approved, Roles( RoleApproved ), "10.0.0.7", answer );
            ASSERT_TRUE( status.IsOk() );
            ASSERT_TRUE( Contains( answer.registry, "\"url\": \"https://hub:8792\"" ) && Contains( answer.registry, "\"id\": \"GUCEF\"" ) );
            ASSERT_TRUE( IsHex64( answer.gatewayToken ) && answer.caCertificate == "the authority\n" && answer.gatewayUrl == "https://hub:8792" );
            const CORE::CString tokenApproved = answer.gatewayToken;

            // the same machine gets the same token; another machine its own
            ASSERT_TRUE( hub.Answer( "drn:" + approved, Roles( RoleApproved ), "10.0.0.7", answer ).IsOk() && answer.gatewayToken == tokenApproved );
            ASSERT_TRUE( hub.Answer( "drn:" + other, Roles( RoleApproved ), "10.0.0.8", answer ).IsOk() && answer.gatewayToken != tokenApproved );
            const CORE::CString tokenOther = answer.gatewayToken;

            // a token of the lab secret, of whatever machine id, gets nothing; neither does a machine that waits
            status = hub.Answer( "drn:" + approved, Roles( RoleLab ), "10.0.0.9", answer );
            ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == status.code && answer.gatewayToken.IsNULLOrEmpty() && answer.registry.IsNULLOrEmpty() );
            CORE::CString::StringSet both = Roles( RoleApproved );
            both.insert( RoleLab );
            ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == hub.Answer( "drn:" + approved, both, "10.0.0.9", answer ).code );
            ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == hub.Answer( "drn:" + pending, Roles( RoleApproved ), "10.0.0.9", answer ).code );
            ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == hub.Answer( "tcp:10.0.0.9", Roles( RoleApproved ), "10.0.0.9", answer ).code );
            ASSERT_TRUE( capture.Contains( "CiConfig refused to drn:" + approved + " at 10.0.0.9" ) );
            ASSERT_TRUE( capture.Contains( "CiConfig refused to drn:" + pending ) );
            ASSERT_TRUE( hub.GetTokens().Load( tokens, problem ) && 2 == tokens.size() );

            // an invalid registry: the hub says why in its log, the machine is told it cannot have it
            ASSERT_TRUE( WriteText( CCiHub::RegistryFileOf( settings ), "{ \"ci\": [ " ) );
            status = hub.Answer( "drn:" + approved, Roles( RoleApproved ), "10.0.0.7", answer );
            ASSERT_TRUE( COMCORE::RPC_STATUS_FAILED_PRECONDITION == status.code );
            ASSERT_TRUE( capture.Contains( "is not handed out: it is not valid JSON" ) );

            // a gateway that is not https is not handed out
            ASSERT_TRUE( WriteText( CCiHub::RegistryFileOf( settings ), Registry ) );
            SSettings plain = settings;
            plain.ciGatewayUrl = "http://hub:8792";
            CCiHub plainHub( plain, store );
            ASSERT_TRUE( COMCORE::RPC_STATUS_FAILED_PRECONDITION == plainHub.Answer( "drn:" + approved, Roles( RoleApproved ), "10.0.0.7", answer ).code );

            // an approval taken back takes the token with it, at the next check of the hub
            GUCEF::vector< SApproval > approvals;
            ASSERT_TRUE( store.LoadApprovals( approvals, problem ) );
            for ( size_t i=0; i<approvals.size(); ++i )
                if ( approvals[ i ].machineId == other )
                    approvals[ i ].state = AccessRefused;
            ASSERT_TRUE( store.SaveApprovals( approvals, problem ) );
            hub.Update( NowInMs() + 61 * 1000 );
            ASSERT_TRUE( hub.GetTokens().Load( tokens, problem ) && 1 == tokens.size() && tokens[ approved ] == tokenApproved );
            ASSERT_FALSE( Contains( ReadText( hub.GetTokens().GetHashFilePath() ), other ) );

            // no token or secret in the log
            ASSERT_FALSE( capture.Contains( tokenApproved ) || capture.Contains( tokenOther ) );
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

    GUCEF_TESTFW_TESTCASE( "CI files: written with owner-only access, only when they changed, removed on request; a link in the place of the directory is not followed" )
        try
        {
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_CiFiles" );
            const CORE::CString ciDir = CORE::CombinePath( dir.GetPath(), "mcp/ci" );
            CCiFiles files( ciDir, CORE::GetEnv( "USER" ) );
            const CORE::CString token = "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff";
            bool changed = false, removed = false;
            CORE::CString problem;
            ASSERT_TRUE( files.Write( Registry, token, "authority\n", changed, problem ) && changed );
            ASSERT_TRUE( ReadText( CORE::CombinePath( ciDir, "ci.json" ) ) == Registry );
            ASSERT_TRUE( ReadText( CORE::CombinePath( ciDir, "gateway.token" ) ) == token + "\n" );
            ASSERT_TRUE( ReadText( CORE::CombinePath( ciDir, "ca.crt" ) ) == "authority\n" );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0700 == InfoOf( ciDir ).mode && (UInt32) ::geteuid() == InfoOf( ciDir ).uid );
            const char* names[] = { "ci.json", "gateway.token", "ca.crt" };
            for ( int i=0; i<3; ++i )
                ASSERT_TRUE( 0600 == InfoOf( CORE::CombinePath( ciDir, names[ i ] ) ).mode && (UInt32) ::geteuid() == InfoOf( CORE::CombinePath( ciDir, names[ i ] ) ).uid );

            // what gucefMcpCi does: the files pass the check of ReadOwnerOnlyFile
            CORE::CDynamicBuffer content;
            ASSERT_TRUE( CORE::ReadOwnerOnlyFile( CORE::CombinePath( ciDir, "gateway.token" ), content, problem ) );

            // the same content: nothing is written; a mode that was widened is narrowed again
            const UInt64 inode = InfoOf( CORE::CombinePath( ciDir, "ci.json" ) ).inode;
            ::chmod( CORE::CombinePath( ciDir, "ca.crt" ).C_String(), 0644 );
            ASSERT_TRUE( files.Write( Registry, token, "authority\n", changed, problem ) && !changed );
            ASSERT_TRUE( inode == InfoOf( CORE::CombinePath( ciDir, "ci.json" ) ).inode && 0600 == InfoOf( CORE::CombinePath( ciDir, "ca.crt" ) ).mode );
            ASSERT_TRUE( files.Write( "{ \"ci\": [] }\n", token, "authority\n", changed, problem ) && changed );
            ASSERT_TRUE( inode != InfoOf( CORE::CombinePath( ciDir, "ci.json" ) ).inode && ReadText( CORE::CombinePath( ciDir, "ci.json" ) ) == "{ \"ci\": [] }\n" );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( ciDir, ".ci.json.tmp" ) ) );
            #endif

            ASSERT_TRUE( files.Remove( removed, problem ) && removed );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( ciDir, "ci.json" ) ) || CORE::FileExists( CORE::CombinePath( ciDir, "gateway.token" ) ) );
            ASSERT_TRUE( files.Remove( removed, problem ) && !removed );
            CCiFiles none( CORE::CombinePath( dir.GetPath(), "no-such/ci" ), CORE::CString::Empty );
            ASSERT_TRUE( none.Remove( removed, problem ) && !removed );

            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            // a link where the directory should be is refused, and nothing is written where it points
            const CORE::CString target = CORE::CombinePath( dir.GetPath(), "elsewhere" );
            CORE::CreateDirs( target );
            CORE::CreateDirs( CORE::CombinePath( dir.GetPath(), "linked" ) );
            ASSERT_TRUE( 0 == ::symlink( target.C_String(), CORE::CombinePath( dir.GetPath(), "linked/ci" ).C_String() ) );
            CCiFiles linked( CORE::CombinePath( dir.GetPath(), "linked/ci" ), CORE::CString::Empty );
            ASSERT_FALSE( linked.Write( Registry, token, "authority\n", changed, problem ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( target, "gateway.token" ) ) );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CI machine: an approved machine writes the CI files, again only when they change, removes them when the hub has no CI; a lab secret machine and --no-ci do not ask" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            GUCEF_TESTFW_SKIP_IF( !CCiRegistryFile::IsJsonCodecAvailable(), "dstorepluginJSONPARSER is not available" );
            TESTFW::CLogTextCapture capture;
            CCiLab lab( "gucefLabLink_TestApp_CiMachine" );
            ASSERT_TRUE( WriteText( lab.RegistryPath(), Registry ) );
            ASSERT_TRUE( lab.StartCluster() );

            SSettings settings = lab.MachineSettings( "machine" );
            CStore store( settings.stateDir );
            CMachineRole machine( settings, store );
            ASSERT_TRUE( lab.Approve( machine, settings ) );
            ASSERT_TRUE( machine.GetCiOutcome() == "written" );
            const CORE::CString ciDir = lab.CiDir( "machine" );
            const CORE::CString registry = ReadText( CORE::CombinePath( ciDir, "ci.json" ) );
            ASSERT_TRUE( Contains( registry, "\"gateway\": { \"url\": \"https://hub.test:8792\", \"caFile\": \"ca.crt\" }" ) && Contains( registry, "\"id\": \"GUCEF\"" ) );
            ASSERT_TRUE( ReadText( CORE::CombinePath( ciDir, "ca.crt" ) ) == ReadText( lab.Path( "mcp/tls/ca.crt" ) ) );
            const CORE::CString token = StripText( ReadText( CORE::CombinePath( ciDir, "gateway.token" ) ) );
            ASSERT_TRUE( IsHex64( token ) );
            const CORE::CString machineId = lab.MachineIdOf( settings );
            ASSERT_TRUE( Contains( lab.HashFile(), Sha256Hex( token ) + "\t" + machineId ) );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0700 == InfoOf( ciDir ).mode && 0600 == InfoOf( CORE::CombinePath( ciDir, "gateway.token" ) ).mode );
            ASSERT_TRUE( (UInt32) ::geteuid() == InfoOf( CORE::CombinePath( ciDir, "gateway.token" ) ).uid );
            const UInt64 inode = InfoOf( CORE::CombinePath( ciDir, "ci.json" ) ).inode;
            #endif

            // a visit that brings the same: nothing is written; a changed registry is written, the token stays
            ASSERT_TRUE( machine.Visit( lab.Sighting() ) && machine.GetCiOutcome() == "unchanged" );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( inode == InfoOf( CORE::CombinePath( ciDir, "ci.json" ) ).inode );
            #endif
            ASSERT_TRUE( WriteText( lab.RegistryPath(), CORE::CString( Registry ).ReplaceSubstr( "kept as it is", "a new registry" ) ) );
            ASSERT_TRUE( machine.Visit( lab.Sighting() ) && machine.GetCiOutcome() == "written" );
            ASSERT_TRUE( Contains( ReadText( CORE::CombinePath( ciDir, "ci.json" ) ), "a new registry" ) );
            ASSERT_TRUE( StripText( ReadText( CORE::CombinePath( ciDir, "gateway.token" ) ) ) == token );

            // a second approved machine has a token of its own
            SSettings secondSettings = lab.MachineSettings( "second" );
            CStore secondStore( secondSettings.stateDir );
            CMachineRole second( secondSettings, secondStore );
            ASSERT_TRUE( lab.Approve( second, secondSettings ) && second.GetCiOutcome() == "written" );
            const CORE::CString secondToken = StripText( ReadText( CORE::CombinePath( lab.CiDir( "second" ), "gateway.token" ) ) );
            ASSERT_TRUE( IsHex64( secondToken ) && secondToken != token );

            // with the lab secret the machine links, and does not ask
            SSettings secretSettings = lab.MachineSettings( "secret" );
            CStore secretStore( secretSettings.stateDir );
            CORE::CString problem;
            ASSERT_TRUE( secretStore.WriteSecret( lab.Secret(), problem ) );
            CMachineRole secretMachine( secretSettings, secretStore );
            ASSERT_TRUE( secretMachine.Visit( lab.Sighting() ) );
            ASSERT_TRUE( Contains( secretMachine.GetCiOutcome(), "not asked" ) && !CORE::DirExists( lab.CiDir( "secret" ) ) );

            // switched off: an approved machine that does not ask
            SSettings offSettings = lab.MachineSettings( "off" );
            offSettings.ci = false;
            CStore offStore( offSettings.stateDir );
            CMachineRole off( offSettings, offStore );
            ASSERT_TRUE( lab.Approve( off, offSettings ) && off.GetCiOutcome() == "switched off" && !CORE::DirExists( lab.CiDir( "off" ) ) );
            CCiGatewayTokens::TTokenMap tokens;
            ASSERT_TRUE( lab.cluster->GetCiHub()->GetTokens().Load( tokens, problem ) && 2 == tokens.size() );

            // the hub has no registry any more: the files go; it has one again: they come back, with the same token
            ASSERT_TRUE( CORE::DeleteFile( lab.RegistryPath() ) );
            ASSERT_TRUE( machine.Visit( lab.Sighting() ) && Contains( machine.GetCiOutcome(), "removed" ) && Contains( machine.GetCiOutcome(), CCiHub::NoCiConfigured ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( ciDir, "ci.json" ) ) || CORE::FileExists( CORE::CombinePath( ciDir, "gateway.token" ) ) ||
                          CORE::FileExists( CORE::CombinePath( ciDir, "ca.crt" ) ) );
            ASSERT_TRUE( WriteText( lab.RegistryPath(), Registry ) );
            ASSERT_TRUE( machine.Visit( lab.Sighting() ) && machine.GetCiOutcome() == "written" );
            ASSERT_TRUE( StripText( ReadText( CORE::CombinePath( ciDir, "gateway.token" ) ) ) == token );

            // revoked: the machine cannot link any more, and the hub takes its token and its line of the hash file away
            ASSERT_TRUE( lab.SetApproval( settings.machineName, AccessRefused ) );
            ASSERT_FALSE( machine.Visit( lab.Sighting() ) );
            lab.cluster->GetCiHub()->Update( NowInMs() + 61 * 1000 );
            ASSERT_TRUE( lab.cluster->GetCiHub()->GetTokens().Load( tokens, problem ) && 1 == tokens.size() && tokens.find( machineId ) == tokens.end() );
            ASSERT_FALSE( Contains( lab.HashFile(), machineId ) || Contains( lab.HashFile(), Sha256Hex( token ) ) );
            ASSERT_TRUE( Contains( lab.HashFile(), Sha256Hex( secondToken ) ) );

            // no gateway token, approval token or lab secret in any line of the log
            SIdentity identity;
            ASSERT_TRUE( store.LoadIdentity( identity, problem ) );
            ASSERT_FALSE( capture.Contains( token ) || capture.Contains( secondToken ) || capture.Contains( lab.Secret() ) );
            ASSERT_FALSE( identity.token.IsNULLOrEmpty() || capture.Contains( identity.token ) );
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

    GUCEF_TESTFW_TESTCASE( "CI machine and an older hub: a hub of gucef.lab 1.1 links as before, the machine keeps its files and asks again only after an hour" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            CCiLab lab( "gucefLabLink_TestApp_CiOldHub" );
            lab.clusterSettings.offeredLabMinor = 1;
            ASSERT_TRUE( WriteText( lab.RegistryPath(), Registry ) );
            ASSERT_TRUE( lab.StartCluster() );

            SSettings settings = lab.MachineSettings( "machine" );
            const CORE::CString ciDir = lab.CiDir( "machine" );
            ASSERT_TRUE( WriteText( CORE::CombinePath( ciDir, "ci.json" ), "{ \"ci\": [] }\n" ) );     // what a manual setup put there
            CStore store( settings.stateDir );
            CMachineRole machine( settings, store );
            ASSERT_TRUE( lab.Approve( machine, settings ) );
            ASSERT_TRUE( machine.GetLastAnswers().ciAsked && COMCORE::RPC_STATUS_UNIMPLEMENTED == machine.GetLastAnswers().ciStatus );
            ASSERT_TRUE( Contains( machine.GetCiOutcome(), "does not hand out" ) && Contains( machine.GetCiOutcome(), "kept" ) );
            ASSERT_TRUE( machine.GetLastAnswers().mcpToken == "the-lab-mcp-token" );
            ASSERT_TRUE( ReadText( CORE::CombinePath( ciDir, "ci.json" ) ) == "{ \"ci\": [] }\n" );

            ASSERT_TRUE( machine.Visit( lab.Sighting() ) );
            ASSERT_FALSE( machine.GetLastAnswers().ciAsked );
            ASSERT_TRUE( Contains( machine.GetCiOutcome(), "not asked" ) );
            CCiGatewayTokens::TTokenMap tokens;
            CORE::CString problem;
            ASSERT_TRUE( lab.cluster->GetCiHub()->GetTokens().Load( tokens, problem ) && tokens.empty() );
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

    GUCEF_TESTFW_TESTCASE( "Program: --list marks the machines with a CI gateway token, --revoke takes the token and its line of the hash file away" )
        try
        {
            const CORE::CString program = CORE::RelativePath( "$MODULEDIR$/gucefLabLink" );
            GUCEF_TESTFW_SKIP_IF( !CORE::FileExists( program ), "gucefLabLink is not next to the test application" );
            GUCEF_TESTFW_SKIP_IF( !LoadCryptography(), "codecspluginMONOCYPHER is not available" );
            TESTFW::CScopedTestDir dir( "gucefLabLink_TestApp_CiProgram" );
            const CORE::CString stateDir = CORE::CombinePath( dir.GetPath(), "state" );
            CORE::CreateDirs( stateDir );
            CStore store( stateDir );
            const CORE::CString withToken = "4444444444444444dddddddddddddddd";
            const CORE::CString withoutToken = "5555555555555555eeeeeeeeeeeeeeee";
            ASSERT_TRUE( AddApproval( store, withToken, "ci-pc", AccessApproved ) && AddApproval( store, withoutToken, "plain-pc", AccessApproved ) );
            CCiGatewayTokens tokens( store );
            CORE::CString token, problem;
            bool made = false, full = false;
            ASSERT_TRUE( tokens.GetOrMake( withToken, token, made, full, problem ) && made );

            CORE::CProcess::TStringMap environment;
            environment[ "GUCEF_CONSOLE_LOGGER" ] = "none";
            CORE::CProcess::TStringVector args;
            args.push_back( "--state-dir" );
            args.push_back( stateDir );
            args.push_back( "--list" );
            std::string output;
            Int32 exitCode = -1;
            ASSERT_TRUE( RunProgram( program, args, environment, 60000, output, exitCode ) );
            GUCEF_TESTFW_SKIP_IF( 3 == exitCode, "codecspluginMONOCYPHER is not next to gucefLabLink" );
            ASSERT_TRUE( 0 == exitCode );
            const CORE::CString list = output.c_str();
            ASSERT_TRUE( Contains( list, withToken + " ci-token\n" ) && Contains( list, withoutToken + "\n" ) && !Contains( list, withoutToken + " ci-token" ) );
            ASSERT_FALSE( Contains( list, token ) );

            args.clear();
            args.push_back( "--state-dir" );
            args.push_back( stateDir );
            args.push_back( "--revoke" );
            args.push_back( "ci-pc" );
            ASSERT_TRUE( RunProgram( program, args, environment, 60000, output, exitCode ) && 0 == exitCode );
            ASSERT_TRUE( Contains( output.c_str(), "Its CI gateway token is removed." ) );
            CCiGatewayTokens::TTokenMap loaded;
            ASSERT_TRUE( tokens.Load( loaded, problem ) && loaded.empty() );
            ASSERT_FALSE( Contains( ReadText( tokens.GetHashFilePath() ), withToken ) );
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
