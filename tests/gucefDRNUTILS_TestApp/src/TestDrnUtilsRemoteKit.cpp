/*
 *  gucefDRNUTILS_TestApp: GUCEF test application
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CSYSTEMUTCCLOCK_H
#include "gucefCORE_CSystemUtcClock.h"
#define GUCEF_CORE_CSYSTEMUTCCLOCK_H
#endif /* GUCEF_CORE_CSYSTEMUTCCLOCK_H ? */

#ifndef GUCEF_CORE_CMONTHLYJSONLINESFILE_H
#include "gucefCORE_CMonthlyJsonLinesFile.h"
#define GUCEF_CORE_CMONTHLYJSONLINESFILE_H
#endif /* GUCEF_CORE_CMONTHLYJSONLINESFILE_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COMCORE_CRPCPRINCIPAL_H
#include "gucefCOMCORE_CRpcPrincipal.h"
#define GUCEF_COMCORE_CRPCPRINCIPAL_H
#endif /* GUCEF_COMCORE_CRPCPRINCIPAL_H ? */

#ifndef GUCEF_COMCORE_CRPCMETADATA_H
#include "gucefCOMCORE_CRpcMetadata.h"
#define GUCEF_COMCORE_CRPCMETADATA_H
#endif /* GUCEF_COMCORE_CRPCMETADATA_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#include "gucefCOMCORE_CRpcRequestId.h"
#define GUCEF_COMCORE_CRPCREQUESTID_H
#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#include "gucefDRN_CIDRNAuthenticator.h"
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H
#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */

#ifndef GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#include "gucefDRNUTILS_RemoteServiceTypes.h"
#define GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#endif /* GUCEF_DRNUTILS_REMOTESERVICETYPES_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICERECORD_H
#include "gucefDRNUTILS_CDeviceRecord.h"
#define GUCEF_DRNUTILS_CDEVICERECORD_H
#endif /* GUCEF_DRNUTILS_CDEVICERECORD_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEREGISTRY_H
#include "gucefDRNUTILS_CDeviceRegistry.h"
#define GUCEF_DRNUTILS_CDEVICEREGISTRY_H
#endif /* GUCEF_DRNUTILS_CDEVICEREGISTRY_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEACCESSRULES_H
#include "gucefDRNUTILS_CDeviceAccessRules.h"
#define GUCEF_DRNUTILS_CDEVICEACCESSRULES_H
#endif /* GUCEF_DRNUTILS_CDEVICEACCESSRULES_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEDRNACCESSPOLICY_H
#include "gucefDRNUTILS_CDeviceDrnAccessPolicy.h"
#define GUCEF_DRNUTILS_CDEVICEDRNACCESSPOLICY_H
#endif /* GUCEF_DRNUTILS_CDEVICEDRNACCESSPOLICY_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#include "gucefDRNUTILS_CDevicePairingCrypto.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#include "gucefDRNUTILS_CDeviceCredentials.h"
#define GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#endif /* GUCEF_DRNUTILS_CDEVICECREDENTIALS_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONEVENTRING_H
#include "gucefDRNUTILS_CRemoteSessionEventRing.h"
#define GUCEF_DRNUTILS_CREMOTESESSIONEVENTRING_H
#endif /* GUCEF_DRNUTILS_CREMOTESESSIONEVENTRING_H ? */

#ifndef GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H
#include "gucefDRNUTILS_CCommandOutcomeCache.h"
#define GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H
#endif /* GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H ? */

#ifndef GUCEF_DRNUTILS_CCOMMANDJOURNAL_H
#include "gucefDRNUTILS_CCommandJournal.h"
#define GUCEF_DRNUTILS_CCOMMANDJOURNAL_H
#endif /* GUCEF_DRNUTILS_CCOMMANDJOURNAL_H ? */

#ifndef GUCEF_DRNUTILS_CUPLOADRANGESET_H
#include "gucefDRNUTILS_CUploadRangeSet.h"
#define GUCEF_DRNUTILS_CUPLOADRANGESET_H
#endif /* GUCEF_DRNUTILS_CUPLOADRANGESET_H ? */

#ifndef GUCEF_DRNUTILS_CUPLOADSTAGING_H
#include "gucefDRNUTILS_CUploadStaging.h"
#define GUCEF_DRNUTILS_CUPLOADSTAGING_H
#endif /* GUCEF_DRNUTILS_CUPLOADSTAGING_H ? */

#ifndef GUCEF_DRNUTILS_CRECONNECTBACKOFF_H
#include "gucefDRNUTILS_CReconnectBackoff.h"
#define GUCEF_DRNUTILS_CRECONNECTBACKOFF_H
#endif /* GUCEF_DRNUTILS_CRECONNECTBACKOFF_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H
#include "TestRpcSupport.h"
#define GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H
#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H
#include "TestDrnUtilsDrnGroup.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
#include <sys/stat.h>
#define DRNUTILSTEST_HAS_STAT
#endif

#include "TestDrnUtilsRemoteKit.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define ERRORHERE       GUCEF_TESTFW_ERRORHERE

namespace {

typedef DRNUTILS::CRemoteServiceDetailCodes TCodes;

/** A fresh empty directory below the work directory for one test case */
CORE::CString
MakeTestDirectory( const char* name )
{GUCEF_TRACE;

    CORE::CString dir = CORE::RelativePath( "$CURWORKDIR$" );
    dir = CORE::CombinePath( dir, "RemoteKitTestData" );
    dir = CORE::CombinePath( dir, name );
    if ( CORE::DirExists( dir ) )
        CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

bool
StartsWith( const CORE::CString& text, const char* prefix )
{GUCEF_TRACE;

    UInt32 length = (UInt32) strlen( prefix );
    return text.Length() >= length && text.SubstrToIndex( length, true ) == prefix;
}

/*-------------------------------------------------------------------------*/

CORE::CString
HexKey( UInt8 fill )
{GUCEF_TRACE;

    UInt8 key[ 32 ];
    memset( key, fill, sizeof( key ) );
    return DRN::CDRNCrypto::ToHex( key, sizeof( key ) ).Lowercase();
}

/*-------------------------------------------------------------------------*/

DRNUTILS::CDeviceRecord
MakeDevice( const char* id, const char* kind, const char* rolesCsv, DRNUTILS::TDeviceStatus status, UInt8 keyFill )
{GUCEF_TRACE;

    DRNUTILS::CDeviceRecord record;
    record.deviceId = id;
    record.userId = "owner";
    record.name = CORE::CString( "device " ) + id;
    record.kind = kind;
    record.status = status;
    record.publicKey = HexKey( keyFill );
    record.fingerprint = DRNUTILS::CDevicePairingCrypto::FingerprintOfHex( record.publicKey );
    record.pairedUtcMs = 1700000000000ULL;
    record.pairedBy = "local-admin";
    CORE::CString::StringVector roles = CORE::CString( rolesCsv ).ParseElements( ',', false );
    for ( UInt32 i=0; i<roles.size(); ++i )
        record.roles.insert( roles[ i ] );
    return record;
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcPrincipal
DevicePrincipal( const char* deviceId )
{GUCEF_TRACE;

    return COMCORE::CRpcPrincipal( "drn", CORE::CString( "drn:" ) + deviceId, true );
}

/*-------------------------------------------------------------------------*/

/** A credential store in memory, for the credentials tests */
class CMemoryCredentialStore : public AUTH::ICredentialStore
{
    public:

    CORE::CString::StringMap slots;

    CMemoryCredentialStore( void ) : AUTH::ICredentialStore(), slots() {}
    virtual ~CMemoryCredentialStore() {}

    virtual bool Store( const CORE::CString& providerName, const CORE::CString& accountKey, const AUTH::CSecretString& secret ) GUCEF_VIRTUAL_OVERRIDE
    {
        slots[ providerName + "/" + accountKey ] = secret.Reveal();
        return true;
    }
    virtual bool Load( const CORE::CString& providerName, const CORE::CString& accountKey, AUTH::CSecretString& secret ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CString::StringMap::iterator i = slots.find( providerName + "/" + accountKey );
        if ( i == slots.end() )
            return false;
        secret.Set( (*i).second );
        return true;
    }
    virtual bool Remove( const CORE::CString& providerName, const CORE::CString& accountKey ) GUCEF_VIRTUAL_OVERRIDE
    {
        slots.erase( providerName + "/" + accountKey );
        return true;
    }
    virtual bool ListAccounts( const CORE::CString& providerName, CORE::CString::StringVector& accountKeys ) GUCEF_VIRTUAL_OVERRIDE
    {
        accountKeys.clear();
        return true;
    }
    virtual bool RemoveProvider( const CORE::CString& providerName ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }
    virtual bool Wipe( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        slots.clear();
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** A contract with a command message and a response, for the outcome cache and the journal */
COMCORE::CRpcContractPtr
GetCommandTestContract( void )
{GUCEF_TRACE;

    static COMCORE::CRpcContractPtr contract;
    if ( !contract.IsNULL() )
        return contract;
    COMCORE::CRpcContractBuilder builder( "gucef.test.remotekit", 1, 0 );
    builder.AddType( "Command" );
    builder.AddField( "Command", 1, "commandId", "string" );
    builder.AddField( "Command", 2, "target", "string" );
    builder.AddField( "Command", 3, "count", "uint32" );
    builder.AddType( "Outcome" );
    builder.AddField( "Outcome", 1, "replayed", "bool" );
    builder.AddField( "Outcome", 2, "revision", "uint64" );
    builder.AddService( "Commands", 1 );
    builder.AddMethod( "Commands", 1, "Do", COMCORE::RPC_METHODKIND_UNARY, "Command", "Outcome" );
    CORE::CString error;
    contract = builder.Build( error );
    if ( contract.IsNULL() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteKit: the test contract does not build: " << error );
    return contract;
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcMessage
NewMessage( const char* typeName )
{GUCEF_TRACE;

    COMCORE::CRpcContractPtr contract = GetCommandTestContract();
    if ( contract.IsNULL() )
        return COMCORE::CRpcMessage();
    return COMCORE::CRpcMessage( contract, contract->FindType( typeName ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
Sha256Hex( const void* data, UInt32 size )
{GUCEF_TRACE;

    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( data, size, digest );
    return DRN::CDRNCrypto::ToHex( digest, sizeof( digest ) ).Lowercase();
}

/*-------------------------------------------------------------------------*/

bool
WriteTextFile( const CORE::CString& path, const char* text )
{GUCEF_TRACE;

    CORE::CFileAccess file;
    if ( !file.Open( path, "wb" ) )
        return false;
    UInt32 length = (UInt32) strlen( text );
    bool ok = 0 == length || 1 == file.Write( text, length, 1 );
    file.Close();
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A clock the journal does not take: the journal uses the system clock, its entries are placed in time by atUtcMs */
UInt64
NowUtcMs( void )
{GUCEF_TRACE;

    CORE::CSystemUtcClock clock;
    return (UInt64) clock.GetUtcTimeInMs();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformDeviceRecordAndRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: device record round trip and malformed records refused" )
    {
        try
        {
            DRNUTILS::CDeviceRecord record = MakeDevice( "d-RECORD", "viewer", "viewer,curator", DRNUTILS::DEVICESTATUS_ACTIVE, 0x11 );
            record.tokenExpiresUtcMs = 1800000000000ULL;
            record.lastAddress = "192.0.2.4";
            CORE::CDataNode node;
            record.ToDataNode( node );
            DRNUTILS::CDeviceRecord back;
            CORE::CString error;
            ASSERT_TRUE( back.FromDataNode( node, error ) );
            ASSERT_TRUE( back.deviceId == record.deviceId );
            ASSERT_TRUE( back.roles == record.roles );
            ASSERT_TRUE( back.status == DRNUTILS::DEVICESTATUS_ACTIVE );
            ASSERT_TRUE( back.publicKey == record.publicKey );
            ASSERT_TRUE( back.tokenExpiresUtcMs == record.tokenExpiresUtcMs );
            ASSERT_TRUE( back.lastAddress == record.lastAddress );
            ASSERT_TRUE( back.GetRolesText() == "curator,viewer" );
            ASSERT_TRUE( back.HasRole( "curator" ) && !back.HasRole( "admin" ) );

            CORE::CDataNode noId( node );
            noId.SetAttribute( "deviceId", CORE::CString::Empty );
            ASSERT_FALSE( back.FromDataNode( noId, error ) );
            CORE::CDataNode badStatus( node );
            badStatus.SetAttribute( "status", CORE::CString( "SLEEPING" ) );
            ASSERT_FALSE( back.FromDataNode( badStatus, error ) );
            CORE::CDataNode badKey( node );
            badKey.SetAttribute( "publicKey", CORE::CString( "abcd" ) );
            ASSERT_FALSE( back.FromDataNode( badKey, error ) );
            // a refused read leaves the record as it was
            ASSERT_TRUE( back.deviceId == record.deviceId );

            DRNUTILS::TDeviceStatus status;
            ASSERT_TRUE( DRNUTILS::DeviceStatusFromString( "revoked", status ) && DRNUTILS::DEVICESTATUS_REVOKED == status );
            ASSERT_TRUE( CORE::CString( "PENDING_CONFIRMATION" ) == DRNUTILS::DeviceStatusToString( DRNUTILS::DEVICESTATUS_PENDING_CONFIRMATION ) );
            ASSERT_TRUE( CORE::CString( "SUSPENDED" ) == DRNUTILS::RemoteSessionStateToString( DRNUTILS::REMOTESESSION_SUSPENDED ) );
            ASSERT_TRUE( CORE::CString( "RECONNECTING" ) == DRNUTILS::RemoteConnectionStateToString( DRNUTILS::REMOTECONNECTION_RECONNECTING ) );

            CORE::CDataNode details;
            TCodes::SetDetails( details, TCodes::RoleRequired );
            ASSERT_TRUE( TCodes::GetCode( details ) == "ROLE_REQUIRED" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: device registry saves, reloads, falls back to the previous copy, is owner only" )
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "registry" );
            {
                DRNUTILS::CDeviceRegistry registry;
                CORE::CString error;
                ASSERT_TRUE( registry.Load( dir, error ) );
                ASSERT_TRUE( 0 == registry.GetCount() );
                registry.Put( MakeDevice( "d-ONE", "viewer", "viewer", DRNUTILS::DEVICESTATUS_ACTIVE, 0x21 ) );
                registry.Put( MakeDevice( "d-TWO", "worker", "renderer", DRNUTILS::DEVICESTATUS_REVOKED, 0x22 ) );
                ASSERT_TRUE( registry.IsDirty() );
                ASSERT_TRUE( registry.Save( error ) );
                ASSERT_FALSE( registry.IsDirty() );
                // a second save makes the previous copy
                registry.NoteSeen( "d-ONE", 1700000001000ULL, "192.0.2.9" );
                ASSERT_TRUE( registry.Save( error ) );
                ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( dir, "devices.json.prev" ) ) );
            }
            CORE::CString path = CORE::CombinePath( dir, "devices.json" );
            #ifdef DRNUTILSTEST_HAS_STAT
            struct stat info;
            ASSERT_TRUE( 0 == stat( path.C_String(), &info ) );
            ASSERT_TRUE( 0600 == ( info.st_mode & 0777 ) );
            #endif
            {
                DRNUTILS::CDeviceRegistry registry;
                CORE::CString error;
                ASSERT_TRUE( registry.Load( dir, error ) );
                ASSERT_TRUE( 2 == registry.GetCount() );
                ASSERT_TRUE( 1 == registry.GetActiveCount() );
                DRNUTILS::CDeviceRecord one;
                ASSERT_TRUE( registry.Find( "d-ONE", one ) );
                ASSERT_TRUE( one.lastAddress == "192.0.2.9" && 1700000001000ULL == one.lastSeenUtcMs );
                DRNUTILS::CDeviceRecord byKey;
                ASSERT_TRUE( registry.FindByPublicKey( HexKey( 0x22 ).Uppercase(), byKey ) );
                ASSERT_TRUE( byKey.deviceId == "d-TWO" );
                ASSERT_FALSE( registry.FindByPublicKey( HexKey( 0x33 ), byKey ) );
                ASSERT_TRUE( registry.Remove( "d-TWO" ) );
                ASSERT_FALSE( registry.Remove( "d-TWO" ) );
            }

            // a torn devices.json: the previous copy is used
            ASSERT_TRUE( WriteTextFile( path, "{ \"format\": \"gucef.drnutils.DeviceRe" ) );
            {
                DRNUTILS::CDeviceRegistry registry;
                CORE::CString error;
                ASSERT_TRUE( registry.Load( dir, error ) );
                ASSERT_TRUE( 2 == registry.GetCount() );
            }
            // both damaged: refused with a reason
            ASSERT_TRUE( WriteTextFile( CORE::CombinePath( dir, "devices.json.prev" ), "not json" ) );
            {
                DRNUTILS::CDeviceRegistry registry;
                CORE::CString error;
                ASSERT_FALSE( registry.Load( dir, error ) );
                ASSERT_FALSE( error.IsNULLOrEmpty() );
            }
            // in memory: saves nothing and succeeds
            {
                DRNUTILS::CDeviceRegistry registry;
                registry.SetInMemory();
                registry.Put( MakeDevice( "d-MEM", "viewer", "viewer", DRNUTILS::DEVICESTATUS_ACTIVE, 0x24 ) );
                CORE::CString error;
                ASSERT_TRUE( registry.Save( error ) );
                ASSERT_FALSE( registry.IsDirty() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformAccessTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: device access rules: exact over pattern, longest pattern, roles, kinds, device states" )
    {
        try
        {
            DRNUTILS::CDeviceRegistry registry;
            registry.SetInMemory();
            registry.Put( MakeDevice( "d-VIEW", "viewer", "viewer", DRNUTILS::DEVICESTATUS_ACTIVE, 0x31 ) );
            registry.Put( MakeDevice( "d-CURA", "viewer", "viewer,curator", DRNUTILS::DEVICESTATUS_ACTIVE, 0x32 ) );
            registry.Put( MakeDevice( "d-WORK", "worker", "renderer", DRNUTILS::DEVICESTATUS_ACTIVE, 0x33 ) );
            registry.Put( MakeDevice( "d-GONE", "viewer", "viewer", DRNUTILS::DEVICESTATUS_REVOKED, 0x34 ) );
            registry.Put( MakeDevice( "d-WAIT", "viewer", "viewer", DRNUTILS::DEVICESTATUS_PENDING_CONFIRMATION, 0x35 ) );
            registry.Put( MakeDevice( "d-NONE", "viewer", "", DRNUTILS::DEVICESTATUS_ACTIVE, 0x36 ) );

            DRNUTILS::CDeviceAccessRules rules( registry );
            rules.AddRule( "gucef.test.Catalogue/*", "viewer,curator" );
            rules.AddRule( "gucef.test.Catalogue/Remove", "curator" );
            rules.AddRule( "gucef.test.*", "admin" );
            rules.AddRule( "gucef.test.Thumbnails/Put", "renderer,curator", "worker" );
            rules.AddRule( "gucef.session.Session/*", "" );
            ASSERT_TRUE( 5 == rules.GetRuleCount() );

            struct SCase
            {
                const char* device;
                const char* path;
                bool allowed;
                const char* code;
            };
            const SCase cases[] =
            {
                { "d-VIEW", "gucef.test.Catalogue/Search", true, "" },
                { "d-VIEW", "gucef.test.Catalogue/Remove", false, "ROLE_REQUIRED" },          // the exact rule wins over the pattern
                { "d-CURA", "gucef.test.Catalogue/Remove", true, "" },
                { "d-VIEW", "gucef.test.Admin/Stop", false, "ROLE_REQUIRED" },                // only the short pattern matches
                { "d-WORK", "gucef.test.Thumbnails/Put", true, "" },
                { "d-CURA", "gucef.test.Thumbnails/Put", false, "DEVICE_KIND" },              // the role is there, the kind is not
                { "d-GONE", "gucef.test.Catalogue/Search", false, "DEVICE_NOT_ACTIVE" },
                { "d-WAIT", "gucef.test.Catalogue/Search", false, "DEVICE_NOT_ACTIVE" },
                { "d-NOBODY", "gucef.test.Catalogue/Search", false, "DEVICE_UNKNOWN" },
                { "d-VIEW", "gucef.other.Service/Call", false, "NO_RULE" },
                { "d-VIEW", "gucef.session.Session/Hello", true, "" },                        // a rule without roles: every active device with a role
                { "d-NONE", "gucef.session.Session/Hello", false, "ROLE_REQUIRED" }           // ... but not one without any role
            };
            for ( UInt32 i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
            {
                CORE::CString reason;
                DRNUTILS::CDeviceRecord device;
                bool allowed = rules.Authorize( DevicePrincipal( cases[ i ].device ), cases[ i ].path, reason, &device );
                if ( allowed != cases[ i ].allowed || ( !allowed && !StartsWith( reason, cases[ i ].code ) ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteKit: access case " << i << " (" << cases[ i ].device << " " << cases[ i ].path << ") gave " << ( allowed ? "allowed" : "refused" ) << ": " << reason );
                ASSERT_TRUE( allowed == cases[ i ].allowed );
                if ( !cases[ i ].allowed )
                    ASSERT_TRUE( StartsWith( reason, cases[ i ].code ) );
            }

            // an unauthenticated principal of the device's subject is not the device
            CORE::CString reason;
            COMCORE::CRpcPrincipal anonymous( "drn", "drn:d-VIEW", false );
            ASSERT_FALSE( rules.Authorize( anonymous, "gucef.test.Catalogue/Search", reason ) );
            ASSERT_TRUE( StartsWith( reason, "DEVICE_UNKNOWN" ) );
            // another subject form
            ASSERT_TRUE( DRNUTILS::CDeviceAccessRules::DeviceIdOfPrincipal( COMCORE::CRpcPrincipal( "tcp", "tcp:127.0.0.1:9", false ) ).IsNULLOrEmpty() );
            ASSERT_TRUE( DRNUTILS::CDeviceAccessRules::DeviceIdOfPrincipal( DevicePrincipal( "d-VIEW" ) ) == "d-VIEW" );

            // trusted transports pass every call
            COMCORE::CRpcPrincipal local( "loopback", "local:in-process", false );
            ASSERT_FALSE( rules.Authorize( local, "gucef.other.Service/Call", reason ) );
            CORE::CString::StringSet trusted;
            trusted.insert( "loopback" );
            rules.SetTrustedTransports( trusted );
            ASSERT_TRUE( rules.Authorize( local, "gucef.other.Service/Call", reason ) );

            ASSERT_TRUE( rules.GetRefusedCount() > 0 );

            // a role change in the registry applies at the next call
            DRNUTILS::CDeviceRecord view;
            registry.Find( "d-VIEW", view );
            view.roles.insert( "curator" );
            registry.Put( view );
            ASSERT_TRUE( rules.Authorize( DevicePrincipal( "d-VIEW" ), "gucef.test.Catalogue/Remove", reason ) );
            view.status = DRNUTILS::DEVICESTATUS_REVOKED;
            registry.Put( view );
            ASSERT_FALSE( rules.Authorize( DevicePrincipal( "d-VIEW" ), "gucef.test.Catalogue/Search", reason ) );

            // a rule replaced and removed
            rules.AddRule( "gucef.test.Catalogue/Remove", "viewer" );
            ASSERT_TRUE( 5 == rules.GetRuleCount() );
            ASSERT_TRUE( rules.RemoveRule( "gucef.test.Catalogue/Remove" ) );
            ASSERT_FALSE( rules.RemoveRule( "gucef.test.Catalogue/Remove" ) );

            // DURABLE ids: their own subject only
            COMCORE::CRpcRequestId id;
            ASSERT_TRUE( rules.MayQueryRequest( DevicePrincipal( "d-CURA" ), "drn:d-CURA", id ) );
            ASSERT_FALSE( rules.MayQueryRequest( DevicePrincipal( "d-CURA" ), "drn:d-WORK", id ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: device access rules: contracts are bindable when a rule could allow one of their methods" )
    {
        try
        {
            COMCORE::CRpcContractPtr contract = GetCommandTestContract();
            ASSERT_TRUE( !contract.IsNULL() );
            if ( !contract.IsNULL() )
            {
                DRNUTILS::CDeviceRegistry registry;
                registry.SetInMemory();
                registry.Put( MakeDevice( "d-VIEW", "viewer", "viewer", DRNUTILS::DEVICESTATUS_ACTIVE, 0x41 ) );
                registry.Put( MakeDevice( "d-ADMN", "viewer", "admin", DRNUTILS::DEVICESTATUS_ACTIVE, 0x42 ) );
                DRNUTILS::CDeviceAccessRules rules( registry );
                rules.AddRule( "gucef.test.remotekit.Commands/*", "admin" );
                ASSERT_TRUE( rules.MayBindContract( DevicePrincipal( "d-ADMN" ), *contract ) );
                ASSERT_FALSE( rules.MayBindContract( DevicePrincipal( "d-VIEW" ), *contract ) );
                ASSERT_FALSE( rules.MayBindContract( DevicePrincipal( "d-UNKNOWN" ), *contract ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: DRN access policy: joins of registered active devices with their own key only; subscriptions by role; no writes" )
    {
        try
        {
            DRNUTILS::CDeviceRegistry registry;
            registry.SetInMemory();
            registry.Put( MakeDevice( "d-VIEW", "viewer", "viewer", DRNUTILS::DEVICESTATUS_ACTIVE, 0x51 ) );
            registry.Put( MakeDevice( "d-GONE", "viewer", "viewer", DRNUTILS::DEVICESTATUS_REVOKED, 0x52 ) );
            registry.Put( MakeDevice( "d-NONE", "viewer", "", DRNUTILS::DEVICESTATUS_ACTIVE, 0x53 ) );
            registry.Put( MakeDevice( "d-RELY", "relay", "mcp", DRNUTILS::DEVICESTATUS_ACTIVE, 0x54 ) );

            DRNUTILS::CDeviceDrnAccessPolicy policy( registry );
            UInt8 key[ 32 ];

            DRN::CDRNAuthResult auth;
            auth.userId = "d-VIEW";
            memset( key, 0x51, sizeof( key ) );
            auth.publicKey.CopyFrom( sizeof( key ), key );
            CORE::CString reason;
            ASSERT_TRUE( policy.MayJoinAs( auth, reason ) );

            DRN::CDRNAuthResult otherKey( auth );
            memset( key, 0x5F, sizeof( key ) );
            otherKey.publicKey.CopyFrom( sizeof( key ), key );
            ASSERT_FALSE( policy.MayJoinAs( otherKey, reason ) );

            DRN::CDRNAuthResult noKey( auth );
            noKey.publicKey.Clear( false );
            ASSERT_FALSE( policy.MayJoinAs( noKey, reason ) );

            DRN::CDRNAuthResult revoked;
            revoked.userId = "d-GONE";
            memset( key, 0x52, sizeof( key ) );
            revoked.publicKey.CopyFrom( sizeof( key ), key );
            ASSERT_FALSE( policy.MayJoinAs( revoked, reason ) );

            DRN::CDRNAuthResult noRoles;
            noRoles.userId = "d-NONE";
            memset( key, 0x53, sizeof( key ) );
            noRoles.publicKey.CopyFrom( sizeof( key ), key );
            ASSERT_FALSE( policy.MayJoinAs( noRoles, reason ) );

            DRN::CDRNAuthResult unknown;
            unknown.userId = "d-UNKNOWN";
            unknown.publicKey.CopyFrom( sizeof( key ), key );
            ASSERT_FALSE( policy.MayJoinAs( unknown, reason ) );

            CORE::CString::StringSet viewers;
            viewers.insert( "viewer" );
            policy.AddSubscribeRule( "svc/*", viewers );
            policy.AddSubscribeRule( "svc/private", CORE::CString::StringSet() );
            ASSERT_TRUE( policy.MaySubscribeAs( 2, auth, "svc/jobs" ) );
            DRN::CDRNAuthResult relay;
            relay.userId = "d-RELY";
            ASSERT_FALSE( policy.MaySubscribeAs( 3, relay, "svc/jobs" ) );          // the relay has no viewer role
            ASSERT_TRUE( policy.MaySubscribeAs( 3, relay, "svc/private" ) );        // a rule without roles: any role
            ASSERT_FALSE( policy.MaySubscribeAs( 2, auth, "other/item" ) );         // no rule
            ASSERT_FALSE( policy.MaySubscribeAs( 2, revoked, "svc/jobs" ) );
            ASSERT_FALSE( policy.MaySubscribe( 2, "svc/jobs" ) );
            ASSERT_FALSE( policy.MayWrite( 2, "svc/jobs", 0 ) );
            ASSERT_FALSE( policy.MayWriteAs( 2, auth, "svc/jobs", 0 ) );
            policy.ClearSubscribeRules();
            ASSERT_FALSE( policy.MaySubscribeAs( 2, auth, "svc/jobs" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformCryptoAndCredentialTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: pairing codes, keys from codes, server proof, ids and the Crockford encoding" )
    {
        try
        {
            // a known vector: 0x00..0x09 as 16 characters (5 bits each, most significant first)
            UInt8 vector[ 10 ] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::ToCrockfordBase32( vector, 10, 16 ) == "000G40R40M30E209" );
            UInt8 ones[ 5 ] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::ToCrockfordBase32( ones, 5, 8 ) == "ZZZZZZZZ" );

            CORE::CString code;
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::GenerateCode( code ) );
            ASSERT_TRUE( 16 == code.Length() );
            CORE::CString other;
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::GenerateCode( other ) );
            ASSERT_TRUE( code != other );
            CORE::CString formatted = DRNUTILS::CDevicePairingCrypto::FormatCode( code );
            ASSERT_TRUE( 19 == formatted.Length() && '-' == formatted.C_String()[ 4 ] && '-' == formatted.C_String()[ 14 ] );
            CORE::CString normalised;
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::NormaliseCode( formatted.Lowercase(), normalised ) && normalised == code );
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::NormaliseCode( "k7qm-3xrd 9htv-w2pa", normalised ) && normalised == "K7QM3XRD9HTVW2PA" );
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::NormaliseCode( "IL0O-0000-0000-0000", normalised ) && normalised == "1100000000000000" );
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::NormaliseCode( "K7QM-3XRD-9HTV-W2P", normalised ) );        // too short
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::NormaliseCode( "K7QM-3XRD-9HTV-W2PAX", normalised ) );      // too long
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::NormaliseCode( "K7QM-3XRD-9HTV-W2PU", normalised ) );       // U is not Crockford
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::NormaliseCode( "K7QM_3XRD_9HTV_W2PA", normalised ) );

            CORE::CString secretA, secretB, secretC;
            UInt8 proofKeyA[ 32 ], proofKeyB[ 32 ], proofKeyC[ 32 ];
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::DeriveKeys( "K7QM3XRD9HTVW2PA", "gucef.assetstore", secretA, proofKeyA ) );
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::DeriveKeys( "K7QM3XRD9HTVW2PA", "gucef.assetstore", secretB, proofKeyB ) );
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::DeriveKeys( "K7QM3XRD9HTVW2PA", "gucef.other", secretC, proofKeyC ) );
            ASSERT_TRUE( 64 == secretA.Length() && secretA == secretB && secretA != secretC );
            ASSERT_TRUE( 0 == memcmp( proofKeyA, proofKeyB, 32 ) && 0 != memcmp( proofKeyA, proofKeyC, 32 ) );
            ASSERT_TRUE( secretA != DRN::CDRNCrypto::ToHex( proofKeyA, 32 ).Lowercase() );
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::DeriveKeys( "SHORT", "gucef.assetstore", secretA, proofKeyA ) );

            UInt8 staticKey[ 32 ];
            UInt8 nonce[ 32 ];
            memset( staticKey, 0x61, sizeof( staticKey ) );
            memset( nonce, 0x62, sizeof( nonce ) );
            UInt8 proof[ 32 ];
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::ComputeServerProof( proofKeyA, staticKey, nonce, 32, proof ) );
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::VerifyServerProof( proofKeyB, staticKey, nonce, 32, proof, 32 ) );
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::VerifyServerProof( proofKeyC, staticKey, nonce, 32, proof, 32 ) );     // another code or service
            UInt8 otherStatic[ 32 ];
            memcpy( otherStatic, staticKey, 32 );
            otherStatic[ 7 ] ^= 1;
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::VerifyServerProof( proofKeyA, otherStatic, nonce, 32, proof, 32 ) );  // another key
            proof[ 0 ] ^= 1;
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::VerifyServerProof( proofKeyA, staticKey, nonce, 32, proof, 32 ) );
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::VerifyServerProof( proofKeyA, staticKey, nonce, 32, proof, 31 ) );

            CORE::CString deviceId, sessionId, windowId;
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::GenerateDeviceId( deviceId ) );
            ASSERT_TRUE( 28 == deviceId.Length() && StartsWith( deviceId, "d-" ) );
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::GenerateSessionId( sessionId ) );
            ASSERT_TRUE( 26 == sessionId.Length() );
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::GenerateWindowId( windowId ) );
            ASSERT_TRUE( 18 == windowId.Length() && StartsWith( windowId, "w-" ) );

            CORE::CString fingerprint = DRNUTILS::CDevicePairingCrypto::Fingerprint( staticKey );
            ASSERT_TRUE( 24 == fingerprint.Length() );             // 20 characters in 5 groups of 4
            ASSERT_TRUE( '-' == fingerprint.C_String()[ 4 ] && '-' == fingerprint.C_String()[ 19 ] );
            ASSERT_TRUE( fingerprint == DRNUTILS::CDevicePairingCrypto::FingerprintOfHex( DRN::CDRNCrypto::ToHex( staticKey, 32 ) ) );
            ASSERT_TRUE( fingerprint != DRNUTILS::CDevicePairingCrypto::Fingerprint( otherStatic ) );
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::FingerprintOfHex( "xyz" ).IsNULLOrEmpty() );

            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::Equals( staticKey, staticKey, 32 ) );
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::Equals( staticKey, otherStatic, 32 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: device key proof at pairing: signed by the device key, refused for another key, static key or nonce" )
    {
        GUCEF_TESTFW_SKIP_IF( !DRNUTILSTEST::EnsureCryptoCodecPluginLoaded(), "the signature codec (codecspluginMONOCYPHER) is not available" );
        try
        {
            UInt8 seed[ 32 ], publicKey[ 32 ];
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::GenerateDeviceKey( seed, publicKey ) );
            UInt8 otherSeed[ 32 ], otherPublic[ 32 ];
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::GenerateDeviceKey( otherSeed, otherPublic ) );
            UInt8 staticKey[ 32 ], nonce[ 32 ];
            memset( staticKey, 0x71, sizeof( staticKey ) );
            memset( nonce, 0x72, sizeof( nonce ) );
            UInt8 signature[ 64 ];
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::SignDeviceProof( seed, staticKey, nonce, 32, signature ) );
            ASSERT_TRUE( DRNUTILS::CDevicePairingCrypto::VerifyDeviceProof( publicKey, staticKey, nonce, 32, signature, 64 ) );
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::VerifyDeviceProof( otherPublic, staticKey, nonce, 32, signature, 64 ) );
            UInt8 otherNonce[ 32 ];
            memcpy( otherNonce, nonce, 32 );
            otherNonce[ 31 ] ^= 0x80;
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::VerifyDeviceProof( publicKey, staticKey, otherNonce, 32, signature, 64 ) );
            UInt8 otherStatic[ 32 ];
            memcpy( otherStatic, staticKey, 32 );
            otherStatic[ 0 ] ^= 2;
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::VerifyDeviceProof( publicKey, otherStatic, nonce, 32, signature, 64 ) );
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::VerifyDeviceProof( publicKey, staticKey, nonce, 32, signature, 63 ) );
            signature[ 10 ] ^= 1;
            ASSERT_FALSE( DRNUTILS::CDevicePairingCrypto::VerifyDeviceProof( publicKey, staticKey, nonce, 32, signature, 64 ) );
            memset( seed, 0, sizeof( seed ) );
            memset( otherSeed, 0, sizeof( otherSeed ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: device credentials: secrets only on request, the key pinned in the endpoints, kept in a credential store" )
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        try
        {
            DRNUTILS::CDeviceCredentials credentials;
            ASSERT_FALSE( credentials.IsComplete() );
            credentials.serviceName = "gucef.assetstore";
            credentials.serviceId = "s-TEST";
            credentials.drnSessionName = "assetstore";
            credentials.deviceId = "d-CREDENTIALS";
            credentials.deviceKind = "viewer";
            credentials.roles.insert( "viewer" );
            credentials.deviceSeedHex.Set( HexKey( 0x81 ) );
            credentials.devicePublicKeyHex = HexKey( 0x82 );
            credentials.token.Set( "VG9rZW5WYWx1ZUZvclRlc3RpbmdPbmx5" );
            credentials.tokenExpiresUtcMs = 1800000000000ULL;
            credentials.serverStaticKeyHex = HexKey( 0x83 );
            credentials.issuerPublicKeyHex = HexKey( 0x84 );
            credentials.sessionEndpoints.push_back( "tcp://assets.lan:47400" );
            credentials.sessionEndpoints.push_back( "192.0.2.10:47400" );
            credentials.securityPolicy = CORE::CDataNode( "DRNSecurityPolicy", GUCEF_DATATYPE_OBJECT );
            credentials.securityPolicy.SetAttribute( "requireEncryption", true );
            ASSERT_TRUE( credentials.IsComplete() );

            CORE::CDataNode publicForm;
            credentials.ToDataNode( publicForm, false );
            ASSERT_TRUE( publicForm.GetAttributeValueOrChildValueByName( "token" ).AsString().IsNULLOrEmpty() );
            ASSERT_TRUE( publicForm.GetAttributeValueOrChildValueByName( "deviceSeed" ).AsString().IsNULLOrEmpty() );
            ASSERT_TRUE( publicForm.GetAttributeValueOrChildValueByName( "deviceId" ).AsString() == "d-CREDENTIALS" );

            CORE::CDataNode secretForm;
            credentials.ToDataNode( secretForm, true );
            DRNUTILS::CDeviceCredentials back;
            CORE::CString error;
            ASSERT_TRUE( back.FromDataNode( secretForm, error ) );
            ASSERT_TRUE( back.token.Reveal() == credentials.token.Reveal() );
            ASSERT_TRUE( back.deviceSeedHex.Reveal() == credentials.deviceSeedHex.Reveal() );
            ASSERT_TRUE( back.sessionEndpoints == credentials.sessionEndpoints );
            ASSERT_TRUE( back.securityPolicy.GetAttributeValueOrChildValueByName( "requireEncryption" ).AsBool( false ) );
            ASSERT_TRUE( back.IsComplete() );

            DRN::CDRNEndpointList endpoints;
            ASSERT_TRUE( credentials.GetEndpointList( endpoints ) );
            ASSERT_TRUE( 2 == endpoints.GetCount() );
            ASSERT_TRUE( endpoints.Get( 0 ).GetParams().GetValueAlways( "key" ).AsString() == HexKey( 0x83 ) );
            ASSERT_TRUE( endpoints.Get( 1 ).GetPort() == 47400 );
            ASSERT_TRUE( credentials.GetEndpointList( endpoints, "127.0.0.1:47999" ) );
            ASSERT_TRUE( 3 == endpoints.GetCount() && 47999 == endpoints.Get( 0 ).GetPort() );
            ASSERT_TRUE( endpoints.Get( 0 ).GetParams().GetValueAlways( "key" ).AsString() == HexKey( 0x83 ) );

            CORE::CString::StringVector same = credentials.sessionEndpoints;
            ASSERT_FALSE( credentials.UpdateSessionEndpoints( same ) );
            CORE::CString::StringVector moved;
            moved.push_back( "tcp://assets.lan:47410" );
            ASSERT_TRUE( credentials.UpdateSessionEndpoints( moved ) );
            ASSERT_FALSE( credentials.UpdateSessionEndpoints( CORE::CString::StringVector() ) );

            CMemoryCredentialStore store;
            ASSERT_TRUE( credentials.SaveToStore( store, "service-1", error ) );
            ASSERT_TRUE( 1 == store.slots.size() );
            DRNUTILS::CDeviceCredentials loaded;
            ASSERT_TRUE( loaded.LoadFromStore( store, "service-1", error ) );
            ASSERT_TRUE( loaded.token.Reveal() == credentials.token.Reveal() && loaded.sessionEndpoints == moved );
            ASSERT_TRUE( loaded.serverStaticKeyHex == credentials.serverStaticKeyHex );
            ASSERT_FALSE( loaded.LoadFromStore( store, "service-2", error ) );
            ASSERT_TRUE( DRNUTILS::CDeviceCredentials::RemoveFromStore( store, "service-1" ) );
            ASSERT_TRUE( store.slots.empty() );
            loaded.Wipe();
            ASSERT_TRUE( loaded.token.Reveal().IsNULLOrEmpty() && loaded.deviceSeedHex.Reveal().IsNULLOrEmpty() );
            ASSERT_FALSE( loaded.IsComplete() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformRingAndCommandTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: event ring: sequences, count and byte limits, Covers and Find after drops" )
    {
        try
        {
            COMCORE::CRpcContractBuilder builder( "gucef.test.events", 1, 0 );
            builder.AddType( "Event" );
            builder.AddField( "Event", 1, "eventSeq", "uint64" );
            builder.AddField( "Event", 2, "kind", "string" );
            builder.AddField( "Event", 3, "atUtcMs", "uint64" );
            builder.AddField( "Event", 6, "text", "string" );
            CORE::CString error;
            COMCORE::CRpcContractPtr contract = builder.Build( error );
            ASSERT_TRUE( !contract.IsNULL() );
            if ( !contract.IsNULL() )
            {
                DRNUTILS::CRemoteSessionEventRing ring;
                ASSERT_TRUE( 1 == ring.GetNextSeq() && 1 == ring.GetRetainedFrom() && 0 == ring.GetRetainedCount() );
                ASSERT_TRUE( ring.Covers( 0 ) && ring.Covers( 1 ) );
                ring.SetLimits( 4, 0 );
                for ( UInt32 i=0; i<10; ++i )
                {
                    COMCORE::CRpcMessage event( contract, contract->FindType( "Event" ) );
                    event.SetString( 2, "NOTICE" );
                    UInt64 seq = ring.Append( event, 1000 + i );
                    ASSERT_TRUE( seq == i + 1 );
                    ASSERT_TRUE( event.GetUInt64( 1 ) == seq && event.GetUInt64( 3 ) == 1000 + i );
                }
                ASSERT_TRUE( 4 == ring.GetRetainedCount() );
                ASSERT_TRUE( 7 == ring.GetRetainedFrom() && 11 == ring.GetNextSeq() );
                ASSERT_TRUE( ring.Covers( 7 ) && ring.Covers( 10 ) && ring.Covers( 11 ) && ring.Covers( 50 ) );
                ASSERT_FALSE( ring.Covers( 6 ) || ring.Covers( 1 ) );
                ASSERT_TRUE( GUCEF_NULL == ring.Find( 6 ) && GUCEF_NULL == ring.Find( 11 ) );
                const DRNUTILS::CRemoteSessionEventRing::SEvent* found = ring.Find( 9 );
                ASSERT_TRUE( GUCEF_NULL != found && 9 == found->eventSeq && 9 == found->message.GetUInt64( 1 ) );

                // the byte limit: large events push the old ones out, the newest always stays
                DRNUTILS::CRemoteSessionEventRing bytes;
                bytes.SetLimits( 0, 300 );
                CORE::CString text;
                for ( UInt32 c=0; c<100; ++c )
                    text += 'x';
                for ( UInt32 i=0; i<10; ++i )
                {
                    COMCORE::CRpcMessage event( contract, contract->FindType( "Event" ) );
                    event.SetString( 6, text );
                    bytes.Append( event, 5 );
                }
                ASSERT_TRUE( bytes.GetRetainedBytes() <= 300 );
                ASSERT_TRUE( bytes.GetRetainedCount() >= 1 && bytes.GetRetainedCount() <= 3 );
                ASSERT_TRUE( GUCEF_NULL != bytes.Find( 10 ) );
                bytes.SetLimits( 0, 10 );
                COMCORE::CRpcMessage big( contract, contract->FindType( "Event" ) );
                big.SetString( 6, text );
                bytes.Append( big, 6 );
                ASSERT_TRUE( 1 == bytes.GetRetainedCount() && 11 == bytes.GetRetainedFrom() );

                // many appends keep the numbering and the bounds
                for ( UInt32 i=0; i<1000; ++i )
                {
                    COMCORE::CRpcMessage event( contract, contract->FindType( "Event" ) );
                    ring.Append( event, 7 );
                }
                ASSERT_TRUE( 4 == ring.GetRetainedCount() && 1011 == ring.GetNextSeq() && 1007 == ring.GetRetainedFrom() );
                ASSERT_TRUE( GUCEF_NULL != ring.Find( 1008 ) && 1008 == ring.Find( 1008 )->eventSeq );
                ring.Clear();
                ASSERT_TRUE( 0 == ring.GetRetainedCount() && 1011 == ring.GetNextSeq() && 1011 == ring.GetRetainedFrom() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: command outcome cache: new, replay, reused; the hash ignores the command id; command ids checked" )
    {
        try
        {
            ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::IsValidCommandId( "cmd-1.A_b" ) );
            ASSERT_FALSE( DRNUTILS::CCommandOutcomeCache::IsValidCommandId( "" ) );
            ASSERT_FALSE( DRNUTILS::CCommandOutcomeCache::IsValidCommandId( "has space" ) );
            ASSERT_FALSE( DRNUTILS::CCommandOutcomeCache::IsValidCommandId( "slash/no" ) );
            CORE::CString longId;
            for ( UInt32 i=0; i<65; ++i )
                longId += 'a';
            ASSERT_FALSE( DRNUTILS::CCommandOutcomeCache::IsValidCommandId( longId ) );
            ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::IsValidCommandId( longId.CutChars( 1, false ) ) );

            COMCORE::CRpcMessage request = NewMessage( "Command" );
            ASSERT_TRUE( request.IsBound() );
            request.SetString( 1, "c-1" );
            request.SetString( 2, "pack-7" );
            request.SetUInt32( 3, 3 );

            DRNUTILS::CCommandOutcomeCache cache;
            CORE::CString hash = cache.HashRequest( request, 1 );
            ASSERT_TRUE( 64 == hash.Length() );
            COMCORE::CRpcMessage sameOtherId( request );
            sameOtherId.SetString( 1, "c-2" );
            ASSERT_TRUE( cache.HashRequest( sameOtherId, 1 ) == hash );           // the id is not part of the request
            ASSERT_TRUE( cache.HashRequest( sameOtherId, 0 ) != hash );           // unless asked
            COMCORE::CRpcMessage otherTarget( request );
            otherTarget.SetString( 2, "pack-8" );
            CORE::CString otherHash = cache.HashRequest( otherTarget, 1 );
            ASSERT_TRUE( otherHash != hash );
            DRNUTILS::CCommandOutcomeCache otherCache;
            ASSERT_TRUE( otherCache.HashRequest( request, 1 ) != hash );          // a key per cache

            DRNUTILS::CCommandOutcomeCache::SOutcome outcome;
            ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_NEW == cache.Find( "drn:d-1", "c-1", hash, 1000, outcome ) );
            COMCORE::CRpcMessage response = NewMessage( "Outcome" );
            response.SetUInt64( 2, 42 );
            DRNUTILS::CCommandOutcomeCache::SOutcome done;
            DRNUTILS::CCommandOutcomeCache::MakeOutcome( COMCORE::CRpcStatus::Ok, &response, done );
            ASSERT_TRUE( done.hasResponse && done.response.GetDataSize() > 0 );
            cache.Remember( "drn:d-1", "c-1", hash, 1000, done );
            ASSERT_TRUE( 1 == cache.GetCount() );

            DRNUTILS::CCommandOutcomeCache::SOutcome kept;
            ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_REPLAY == cache.Find( "drn:d-1", "c-1", hash, 2000, kept ) );
            ASSERT_TRUE( kept.status.IsOk() && kept.hasResponse );
            COMCORE::CRpcMessage replayed = NewMessage( "Outcome" );
            COMCORE::CRpcStatus decodeStatus;
            ASSERT_TRUE( replayed.Decode( kept.response.GetConstBufferPtr(), kept.response.GetDataSize(), decodeStatus ) );
            ASSERT_TRUE( 42 == replayed.GetUInt64( 2 ) );
            ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_REUSED == cache.Find( "drn:d-1", "c-1", otherHash, 2000, kept ) );
            ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_NEW == cache.Find( "drn:d-2", "c-1", hash, 2000, kept ) );   // per subject

            // a failure is kept as the failure
            DRNUTILS::CCommandOutcomeCache::SOutcome failed;
            DRNUTILS::CCommandOutcomeCache::MakeOutcome( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_FAILED_PRECONDITION, "JOB_STATE: ended" ), &response, failed );
            ASSERT_FALSE( failed.hasResponse );
            cache.Remember( "drn:d-1", "c-9", otherHash, 3000, failed );
            ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_REPLAY == cache.Find( "drn:d-1", "c-9", otherHash, 3000, kept ) );
            ASSERT_TRUE( COMCORE::RPC_STATUS_FAILED_PRECONDITION == kept.status.code && !kept.hasResponse );

            // bounds: the time to live and the per subject maximum
            cache.SetBounds( 5000, 2, 0 );
            ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_NEW == cache.Find( "drn:d-1", "c-1", hash, 1000 + 6000, kept ) );
            cache.Remember( "drn:d-3", "a", hash, 10000, done );
            cache.Remember( "drn:d-3", "b", hash, 10001, done );
            cache.Remember( "drn:d-3", "c", hash, 10002, done );
            ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_NEW == cache.Find( "drn:d-3", "a", hash, 10003, kept ) );
            ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_REPLAY == cache.Find( "drn:d-3", "c", hash, 10003, kept ) );
            cache.Clear();
            ASSERT_TRUE( 0 == cache.GetCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: command journal: record, reopen, replay and reuse with the same key, another key, a torn line, the day cut" )
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "journal" );
            UInt8 key[ 32 ];
            memset( key, 0x91, sizeof( key ) );

            COMCORE::CRpcMessage request = NewMessage( "Command" );
            request.SetString( 1, "c-1" );
            request.SetString( 2, "pack-1" );
            COMCORE::CRpcMessage other( request );
            other.SetString( 2, "pack-2" );

            CORE::CString hash;
            {
                DRNUTILS::CCommandJournal journal;
                journal.SetKey( key, sizeof( key ) );
                CORE::CString error;
                ASSERT_TRUE( journal.Open( dir, error ) );
                ASSERT_TRUE( journal.IsOpen() && 0 == journal.GetCount() );
                hash = journal.HashRequest( request, 1 );
                ASSERT_TRUE( 64 == hash.Length() );

                DRNUTILS::CCommandJournal::SEntry entry;
                entry.subject = "drn:d-1";
                entry.commandId = "c-1";
                entry.requestHash = hash;
                entry.method = "gucef.test.remotekit.Commands/Do";
                entry.code = COMCORE::RPC_STATUS_OK;
                entry.outcome = CORE::CDataNode( "outcome", GUCEF_DATATYPE_OBJECT );
                entry.outcome.SetAttribute( "revision", (UInt64) 77 );
                entry.outcome.SetAttribute( "note", CORE::CString( "a line\nwith a break" ) );
                ASSERT_TRUE( journal.Record( entry, error ) );

                DRNUTILS::CCommandJournal::SEntry old;
                old.atUtcMs = NowUtcMs() - (UInt64) 30 * 24 * 60 * 60 * 1000;     // beyond the seven days
                old.subject = "drn:d-1";
                old.commandId = "c-old";
                old.requestHash = hash;
                old.method = entry.method;
                ASSERT_TRUE( journal.Record( old, error ) );
                ASSERT_TRUE( 2 == journal.GetCount() );
            }
            // a torn last line, as a process killed while it wrote leaves it
            CORE::CString::StringVector before;
            {
                CORE::CMonthlyJsonLinesFile file;
                ASSERT_TRUE( file.SetDirectory( dir ) );
                ASSERT_TRUE( file.ReadLines( 0, before ) );
                ASSERT_TRUE( 2 == before.size() );
                CORE::CFileAccess raw;
                ASSERT_TRUE( raw.Open( file.GetCurrentFilePath(), "ab" ) );
                const char torn[] = "{\"atUtcMs\":1,\"subject\":\"drn:d-1\",\"comm";
                raw.Write( torn, (UInt32) sizeof( torn ) - 1, 1 );
                raw.Close();
            }
            {
                DRNUTILS::CCommandJournal journal;
                journal.SetKey( key, sizeof( key ) );
                CORE::CString error;
                ASSERT_TRUE( journal.Open( dir, error ) );
                ASSERT_TRUE( 1 == journal.GetCount() );                 // the old entry is beyond the days kept, the fragment is no entry
                ASSERT_TRUE( journal.HashRequest( request, 1 ) == hash );
                DRNUTILS::CCommandJournal::SEntry found;
                ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_REPLAY == journal.Lookup( "drn:d-1", "c-1", hash, found ) );
                ASSERT_TRUE( 77 == found.outcome.GetAttributeValueOrChildValueByName( "revision" ).AsUInt64( 0 ) );
                ASSERT_TRUE( found.outcome.GetAttributeValueOrChildValueByName( "note" ).AsString() == "a line\nwith a break" );
                ASSERT_TRUE( found.method == "gucef.test.remotekit.Commands/Do" && COMCORE::RPC_STATUS_OK == found.code );
                ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_REUSED == journal.Lookup( "drn:d-1", "c-1", journal.HashRequest( other, 1 ), found ) );
                ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_NEW == journal.Lookup( "drn:d-2", "c-1", hash, found ) );
                ASSERT_TRUE( DRNUTILS::CCommandOutcomeCache::COMMAND_NEW == journal.Lookup( "drn:d-1", "c-old", hash, found ) );
            }
            {
                // another key gives other hashes: a repeat is then not recognised as the same request
                DRNUTILS::CCommandJournal journal;
                UInt8 otherKey[ 32 ];
                memset( otherKey, 0x92, sizeof( otherKey ) );
                journal.SetKey( otherKey, sizeof( otherKey ) );
                ASSERT_TRUE( journal.HashRequest( request, 1 ) != hash );
            }
            {
                // the key kept in a credential store is the same at the next start
                CMemoryCredentialStore store;
                DRNUTILS::CCommandJournal first;
                CORE::CString problem;
                ASSERT_TRUE( first.UseKeyOfStore( store, "gucef.test", problem ) );
                ASSERT_TRUE( 1 == store.slots.size() );
                DRNUTILS::CCommandJournal second;
                ASSERT_TRUE( second.UseKeyOfStore( store, "gucef.test", problem ) );
                ASSERT_TRUE( first.HashRequest( request, 1 ) == second.HashRequest( request, 1 ) );
            }
            {
                DRNUTILS::CCommandJournal closed;
                DRNUTILS::CCommandJournal::SEntry entry;
                entry.subject = "drn:d-1";
                entry.commandId = "c-x";
                CORE::CString error;
                ASSERT_FALSE( closed.Record( entry, error ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformUploadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: upload range sets: merging, Contains, missing ranges and the text form" )
    {
        try
        {
            DRNUTILS::CUploadRangeSet set;
            ASSERT_TRUE( set.IsComplete( 0 ) && !set.IsComplete( 1 ) );
            set.Add( 100, 50 );
            set.Add( 0, 10 );
            set.Add( 200, 10 );
            ASSERT_TRUE( 3 == set.GetRanges().size() );
            set.Add( 150, 50 );                                     // touches both neighbours
            ASSERT_TRUE( 2 == set.GetRanges().size() );
            ASSERT_TRUE( 100 == set.GetRanges()[ 1 ].offset && 110 == set.GetRanges()[ 1 ].length );
            set.Add( 5, 100 );                                      // overlaps
            ASSERT_TRUE( 1 == set.GetRanges().size() && 0 == set.GetRanges()[ 0 ].offset && 210 == set.GetRanges()[ 0 ].length );
            set.Add( 0, 0 );
            ASSERT_TRUE( 210 == set.GetTotalBytes() );
            ASSERT_TRUE( set.Contains( 0, 210 ) && set.Contains( 50, 10 ) && !set.Contains( 200, 11 ) );
            ASSERT_TRUE( set.IsComplete( 210 ) && !set.IsComplete( 211 ) );

            DRNUTILS::CUploadRangeSet holes;
            holes.Add( 10, 10 );
            holes.Add( 40, 10 );
            DRNUTILS::CUploadRangeSet::TRangeVector missing;
            holes.GetMissing( 60, missing );
            ASSERT_TRUE( 3 == missing.size() );
            ASSERT_TRUE( 0 == missing[ 0 ].offset && 10 == missing[ 0 ].length );
            ASSERT_TRUE( 20 == missing[ 1 ].offset && 20 == missing[ 1 ].length );
            ASSERT_TRUE( 50 == missing[ 2 ].offset && 10 == missing[ 2 ].length );
            holes.GetMissing( 45, missing );
            ASSERT_TRUE( 2 == missing.size() );

            CORE::CString text = holes.ToString();
            ASSERT_TRUE( text == "10-19,40-49" );
            DRNUTILS::CUploadRangeSet parsed;
            ASSERT_TRUE( parsed.FromString( text ) && parsed.ToString() == text );
            ASSERT_TRUE( parsed.FromString( "" ) && parsed.GetRanges().empty() );
            ASSERT_FALSE( parsed.FromString( "5-2" ) );
            ASSERT_FALSE( parsed.FromString( "a-b" ) );
            ASSERT_FALSE( parsed.FromString( "-5" ) );
            ASSERT_TRUE( parsed.FromString( "0-9,10-19" ) && 1 == parsed.GetRanges().size() );
            parsed.Clear();
            ASSERT_TRUE( 0 == parsed.GetTotalBytes() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: upload staging: unsafe relative paths are refused" )
    {
        try
        {
            const char* bad[] =
            {
                "", "/abs", "/", "a//b", "a/", "./a", "a/./b", "../a", "a/../b", "..", ".", "a\\b", "C:/x", "c:x", "a:b",
                "con", "CON", "Con.txt", "nul", "aux.log", "prn", "COM1", "com9.dat", "LPT1", "lpt9", "a/CON/b",
                "trailingdot.", "trailing space ", "a/b./c", "tab\there", "new\nline", "bell\x07", "del\x7f"
            };
            for ( UInt32 i=0; i<sizeof( bad ) / sizeof( bad[ 0 ] ); ++i )
            {
                bool safe = DRNUTILS::CUploadStaging::IsSafeRelativePath( bad[ i ] );
                if ( safe )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteKit: the path \"" << bad[ i ] << "\" was taken as safe" );
                ASSERT_FALSE( safe );
            }
            CORE::CString longSegment;
            for ( UInt32 i=0; i<256; ++i )
                longSegment += 'a';
            ASSERT_FALSE( DRNUTILS::CUploadStaging::IsSafeRelativePath( longSegment ) );
            const char* good[] = { "a", "pack/Textures/rock.png", "COM10", "console", "con-fig.txt", "a.b/c.d", "COM0", ".hidden", "x y/z" };
            for ( UInt32 i=0; i<sizeof( good ) / sizeof( good[ 0 ] ); ++i )
            {
                bool safe = DRNUTILS::CUploadStaging::IsSafeRelativePath( good[ i ] );
                if ( !safe )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteKit: the path \"" << good[ i ] << "\" was refused" );
                ASSERT_TRUE( safe );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: upload staging: quotas, blocks, ranges, commit with digests, reopen, abort, expiry, subjects" )
    {
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        try
        {
            CORE::CString dir = MakeTestDirectory( "staging" );
            DRNUTILS::CUploadStaging::SSettings settings;
            settings.directory = dir;
            settings.maxBytesPerSubject = 1000000;
            settings.maxTotalBytes = 1500000;
            settings.maxUploadsPerSubject = 1;
            settings.maxFilesPerUpload = 4;
            settings.maxFileBytes = 900000;
            settings.expiryMs = 60000;
            settings.blockSize = 1024;
            const UInt64 t0 = NowUtcMs();

            // the content: a file of three and a half blocks and an empty file
            static UInt8 bytes[ 3584 ];
            for ( UInt32 i=0; i<3584; ++i )
                bytes[ i ] = (UInt8) ( i * 7 + 3 );

            DRNUTILS::CUploadStaging::TFileSpecVector files;
            DRNUTILS::CUploadStaging::SFileSpec first;
            first.relativePath = "pack/data.bin";
            first.size = 3584;
            first.sha256Hex = Sha256Hex( bytes, 3584 );
            files.push_back( first );
            DRNUTILS::CUploadStaging::SFileSpec empty;
            empty.relativePath = "pack/empty.txt";
            empty.size = 0;
            empty.sha256Hex = Sha256Hex( "", 0 );
            files.push_back( empty );

            CORE::CString uploadId, code, message;
            {
                DRNUTILS::CUploadStaging staging;
                CORE::CString error;
                ASSERT_TRUE( staging.Open( settings, error ) );

                // refusals at Begin()
                DRNUTILS::CUploadStaging::TFileSpecVector unsafe = files;
                unsafe[ 0 ].relativePath = "../escape";
                ASSERT_FALSE( staging.Begin( "drn:d-1", "x", unsafe, t0 + 1000, uploadId, code, message ) );
                ASSERT_TRUE( code == TCodes::UnsafePath );
                DRNUTILS::CUploadStaging::TFileSpecVector twice = files;
                twice[ 1 ].relativePath = "PACK/data.bin";
                ASSERT_FALSE( staging.Begin( "drn:d-1", "x", twice, t0 + 1000, uploadId, code, message ) );
                ASSERT_TRUE( code == TCodes::UnsafePath );
                DRNUTILS::CUploadStaging::TFileSpecVector badDigest = files;
                badDigest[ 0 ].sha256Hex = "ABC";
                ASSERT_FALSE( staging.Begin( "drn:d-1", "x", badDigest, t0 + 1000, uploadId, code, message ) );
                ASSERT_TRUE( code == TCodes::OutOfRange );
                DRNUTILS::CUploadStaging::TFileSpecVector tooLarge = files;
                tooLarge[ 0 ].size = 900001;
                ASSERT_FALSE( staging.Begin( "drn:d-1", "x", tooLarge, t0 + 1000, uploadId, code, message ) );
                ASSERT_TRUE( code == TCodes::QuotaExceeded );
                DRNUTILS::CUploadStaging::TFileSpecVector tooMany = files;
                for ( UInt32 i=0; i<3; ++i )
                {
                    DRNUTILS::CUploadStaging::SFileSpec more = empty;
                    more.relativePath = "more/" + CORE::ToString( i );
                    tooMany.push_back( more );
                }
                ASSERT_FALSE( staging.Begin( "drn:d-1", "x", tooMany, t0 + 1000, uploadId, code, message ) );
                ASSERT_TRUE( code == TCodes::QuotaExceeded );
                ASSERT_FALSE( staging.Begin( "drn:d-1", "x", DRNUTILS::CUploadStaging::TFileSpecVector(), t0 + 1000, uploadId, code, message ) );

                ASSERT_TRUE( staging.Begin( "drn:d-1", "the pack", files, t0 + 1000, uploadId, code, message ) );
                ASSERT_TRUE( StartsWith( uploadId, "u-" ) );
                ASSERT_TRUE( 3584 == staging.GetUsedBytes( "drn:d-1" ) && 1 == staging.GetOpenCount( "drn:d-1" ) );

                // one upload at a time per subject; the byte quota of another subject
                CORE::CString secondId;
                ASSERT_FALSE( staging.Begin( "drn:d-1", "second", files, t0 + 1000, secondId, code, message ) );
                ASSERT_TRUE( code == TCodes::UploadLimit );
                DRNUTILS::CUploadStaging::TFileSpecVector big = files;
                big[ 0 ].size = 900000;
                ASSERT_TRUE( staging.Begin( "drn:d-2", "big", big, t0 + 1000, secondId, code, message ) );
                DRNUTILS::CUploadStaging::TFileSpecVector third = files;
                third[ 0 ].size = 800000;
                CORE::CString thirdId;
                ASSERT_FALSE( staging.Begin( "drn:d-3", "third", third, t0 + 1000, thirdId, code, message ) );
                ASSERT_TRUE( code == TCodes::StagingFull );
                ASSERT_TRUE( staging.Abort( "drn:d-2", secondId ) );
                ASSERT_FALSE( staging.Abort( "drn:d-2", secondId ) );

                // blocks out of range or of another subject
                ASSERT_FALSE( staging.WriteBlock( "drn:d-1", uploadId, 2, 0, bytes, 10, t0 + 1100, code, message ) );
                ASSERT_TRUE( code == TCodes::OutOfRange );
                ASSERT_FALSE( staging.WriteBlock( "drn:d-1", uploadId, 0, 3000, bytes, 1000, t0 + 1100, code, message ) );     // beyond the end
                ASSERT_FALSE( staging.WriteBlock( "drn:d-1", uploadId, 0, 0, bytes, 2048, t0 + 1100, code, message ) );        // larger than a block
                ASSERT_FALSE( staging.WriteBlock( "drn:d-1", uploadId, 0, 0, bytes, 0, t0 + 1100, code, message ) );
                ASSERT_FALSE( staging.WriteBlock( "drn:d-9", uploadId, 0, 0, bytes, 1024, t0 + 1100, code, message ) );
                ASSERT_TRUE( code == TCodes::UploadUnknown );

                // two of the four blocks, out of order
                ASSERT_TRUE( staging.WriteBlock( "drn:d-1", uploadId, 0, 2048, bytes + 2048, 1024, t0 + 1200, code, message ) );
                ASSERT_TRUE( staging.WriteBlock( "drn:d-1", uploadId, 0, 0, bytes, 1024, t0 + 1300, code, message ) );
                DRNUTILS::CUploadStaging::SUploadInfo info;
                ASSERT_TRUE( staging.Query( "drn:d-1", uploadId, info ) );
                ASSERT_TRUE( info.received[ 0 ].ToString() == "0-1023,2048-3071" );
                ASSERT_TRUE( 2048 == info.receivedBytes && 3584 == info.totalBytes && info.name == "the pack" );
                ASSERT_FALSE( staging.Query( "drn:d-2", uploadId, info ) );
                CORE::CString staged;
                ASSERT_FALSE( staging.Commit( "drn:d-1", uploadId, staged, code, message ) );
                ASSERT_TRUE( code == TCodes::UploadIncomplete );
            }

            // a restart of the service: the upload is still there with its ranges
            {
                DRNUTILS::CUploadStaging staging;
                CORE::CString error;
                ASSERT_TRUE( staging.Open( settings, error ) );
                DRNUTILS::CUploadStaging::SUploadInfo info;
                bool kept = staging.Query( "drn:d-1", uploadId, info );
                ASSERT_TRUE( kept );
                if ( !kept )
                    return;     // the failure is recorded; what follows needs the upload
                ASSERT_TRUE( info.received[ 0 ].ToString() == "0-1023,2048-3071" );
                DRNUTILS::CUploadRangeSet::TRangeVector missing;
                info.received[ 0 ].GetMissing( info.files[ 0 ].size, missing );
                ASSERT_TRUE( 2 == missing.size() );

                // the rest, with a damaged block first: the digest does not match
                CORE::CDynamicBuffer damaged( 1024, true );
                memcpy( damaged.GetBufferPtr(), bytes + 1024, 1024 );
                ( (UInt8*) damaged.GetBufferPtr() )[ 5 ] ^= 0xFF;
                ASSERT_TRUE( staging.WriteBlock( "drn:d-1", uploadId, 0, 1024, damaged.GetConstBufferPtr(), 1024, t0 + 2000, code, message ) );
                ASSERT_TRUE( staging.WriteBlock( "drn:d-1", uploadId, 0, 3072, bytes + 3072, 512, t0 + 2000, code, message ) );
                CORE::CString staged;
                ASSERT_FALSE( staging.Commit( "drn:d-1", uploadId, staged, code, message ) );
                ASSERT_TRUE( code == TCodes::UploadDigestMismatch );
                ASSERT_TRUE( staging.Query( "drn:d-1", uploadId, info ) );
                ASSERT_TRUE( 0 == info.received[ 0 ].GetTotalBytes() );     // the file is sent again

                for ( UInt32 offset=0; offset<3584; offset+=1024 )
                {
                    UInt32 size = 3584 - offset < 1024 ? 3584 - offset : 1024;
                    ASSERT_TRUE( staging.WriteBlock( "drn:d-1", uploadId, 0, offset, bytes + offset, size, t0 + 3000, code, message ) );
                }
                ASSERT_FALSE( staging.Commit( "drn:d-9", uploadId, staged, code, message ) );
                ASSERT_TRUE( staging.Commit( "drn:d-1", uploadId, staged, code, message ) );
                ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( CORE::CombinePath( staged, "pack" ), "data.bin" ) ) );
                ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( CORE::CombinePath( staged, "pack" ), "empty.txt" ) ) );
                ASSERT_TRUE( 3584 == CORE::FileSize( CORE::CombinePath( CORE::CombinePath( staged, "pack" ), "data.bin" ) ) );
                ASSERT_TRUE( staging.Query( "drn:d-1", uploadId, info ) && DRNUTILS::CUploadStaging::UPLOADSTATE_COMMITTED == info.state );
                CORE::CString again;
                ASSERT_TRUE( staging.Commit( "drn:d-1", uploadId, again, code, message ) && again == staged );
                ASSERT_FALSE( staging.WriteBlock( "drn:d-1", uploadId, 0, 0, bytes, 1024, t0 + 3100, code, message ) );

                // a committed upload is not expired; the service releases it when it took the folder
                ASSERT_TRUE( 0 == staging.ExpireOld( t0 + 10000000 ) );
                ASSERT_TRUE( staging.Release( uploadId ) );
                ASSERT_FALSE( CORE::DirExists( CORE::CombinePath( dir, uploadId ) ) );
                ASSERT_TRUE( 0 == staging.GetOpenCount() && 0 == staging.GetTotalUsedBytes() );

                // an upload that nobody continues expires
                CORE::CString idle;
                ASSERT_TRUE( staging.Begin( "drn:d-1", "idle", files, t0 + 5000, idle, code, message ) );
                ASSERT_TRUE( 0 == staging.ExpireOld( t0 + 5000 + 59000 ) );
                ASSERT_TRUE( 1 == staging.ExpireOld( t0 + 5000 + 60000 ) );
                ASSERT_FALSE( staging.Query( "drn:d-1", idle, info ) );
                ASSERT_FALSE( CORE::DirExists( CORE::CombinePath( dir, idle ) ) );
            }

            // an expired upload of an earlier run and a folder without a manifest are removed at Open()
            {
                DRNUTILS::CUploadStaging staging;
                CORE::CString error;
                ASSERT_TRUE( staging.Open( settings, error ) );
                CORE::CString old;
                ASSERT_TRUE( staging.Begin( "drn:d-1", "old", files, 1000, old, code, message ) );        // expires at 61000, long ago
                staging.Close();
                CORE::CreateDirs( CORE::CombinePath( dir, "u-STRAY" ) );
                ASSERT_TRUE( staging.Open( settings, error ) );
                ASSERT_TRUE( 0 == staging.GetOpenCount() );
                ASSERT_FALSE( CORE::DirExists( CORE::CombinePath( dir, old ) ) );
                ASSERT_FALSE( CORE::DirExists( CORE::CombinePath( dir, "u-STRAY" ) ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

static void
PerformBackoffTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "RemoteKit: reconnect backoff: first attempt, doubling bounds to the cap, hints as lower bounds, reset after a stable period" )
    {
        try
        {
            DRNUTILS::CReconnectBackoff backoff;
            backoff.SetSeed( 12345 );
            ASSERT_TRUE( 250 == backoff.GetBoundMs( 0 ) && 500 == backoff.GetBoundMs( 1 ) && 1000 == backoff.GetBoundMs( 2 ) );
            ASSERT_TRUE( 30000 == backoff.GetBoundMs( 7 ) && 30000 == backoff.GetBoundMs( 40 ) );

            UInt32 maxSeen[ 10 ];
            memset( maxSeen, 0, sizeof( maxSeen ) );
            for ( UInt32 round=0; round<200; ++round )
            {
                backoff.Reset();
                for ( UInt32 attempt=0; attempt<10; ++attempt )
                {
                    UInt32 bound = backoff.GetBoundMs( attempt );
                    UInt32 delay = backoff.NextDelayMs();
                    ASSERT_TRUE( delay <= bound );
                    if ( delay > maxSeen[ attempt ] )
                        maxSeen[ attempt ] = delay;
                }
            }
            // full jitter spreads over the whole window
            ASSERT_TRUE( maxSeen[ 0 ] > 125 && maxSeen[ 9 ] > 15000 );
            ASSERT_TRUE( 10 == backoff.GetAttempt() );

            // a hint of the service is a lower bound with a spread of at least the minimum
            for ( UInt32 i=0; i<100; ++i )
            {
                UInt32 delay = backoff.NextDelayMs( 30000 );
                ASSERT_TRUE( delay >= 30000 && delay <= 45000 );
                UInt32 small = backoff.NextDelayMs( 200 );
                ASSERT_TRUE( small >= 200 && small <= 1200 );
            }

            // the same seed gives the same delays
            DRNUTILS::CReconnectBackoff a;
            DRNUTILS::CReconnectBackoff b;
            a.SetSeed( 99 );
            b.SetSeed( 99 );
            for ( UInt32 i=0; i<20; ++i )
                ASSERT_TRUE( a.NextDelayMs() == b.NextDelayMs() );

            // the exponent resets only after the connection stayed up for the stable period
            DRNUTILS::CReconnectBackoff stable;
            stable.SetPolicy( 250, 500, 30000, 60000, 1000 );
            stable.NextDelayMs();
            stable.NextDelayMs();
            stable.NextDelayMs();
            ASSERT_TRUE( 3 == stable.GetAttempt() );
            stable.OnConnected( 1000 );
            stable.OnStillConnected( 30000 );
            ASSERT_TRUE( 3 == stable.GetAttempt() );
            stable.NextDelayMs();                                   // it dropped again before the period was over
            ASSERT_TRUE( 4 == stable.GetAttempt() );
            stable.OnConnected( 100000 );
            stable.OnStillConnected( 159999 );
            ASSERT_TRUE( 4 == stable.GetAttempt() );
            stable.OnStillConnected( 160000 );
            ASSERT_TRUE( 0 == stable.GetAttempt() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

void
PerformDrnUtilsRemoteKitTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsRemoteKit" );

    PerformDeviceRecordAndRegistryTests();
    PerformAccessTests();
    PerformCryptoAndCredentialTests();
    PerformRingAndCommandTests();
    PerformUploadTests();
    PerformBackoffTests();
}

/*-------------------------------------------------------------------------*/
