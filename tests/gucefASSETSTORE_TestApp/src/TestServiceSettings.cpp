/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#include <stdlib.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIGSETTINGS_H
#include "gucefASSETSTORE_CAssetStoreServiceConfigSettings.h"
#define GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIGSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIGSETTINGS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSOVERRIDESTORE_H
#include "gucefASSETSTORE_CAssetSettingsOverrideStore.h"
#define GUCEF_ASSETSTORE_CASSETSETTINGSOVERRIDESTORE_H
#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSOVERRIDESTORE_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#include "dbpluginPOSTGRESQL_CPostgreSqlClient.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#include "dbpluginPOSTGRESQL_CPostgreSqlConnection.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSql.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSettings.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H ? */

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */

#ifndef TESTASSETSTOREREMOTEWORLD_H
#include "TestAssetStoreRemoteWorld.h"
#define TESTASSETSTOREREMOTEWORLD_H
#endif /* TESTASSETSTOREREMOTEWORLD_H ? */

#include "TestServiceSettings.h"

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::ASSETSTORE;
using namespace ASSETSTORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The settings of the configuration of CAssetStoreServiceConfigSettings: 14 of the reprocessing, 8 of the downloads, 4 of the service */
const UInt32 RegisteredSettingCount = 26;

struct SSettingsFixture
{
    CString label;
    CString base;
    COMCORE::CDatabaseClientSettings database;
    AUTH::ICredentialStore* credentials;
    SSettingsFixture( void ) : label(), base(), database(), credentials( GUCEF_NULL ) {}
};

/** A service with a state folder; its site settings differ from the defaults in three values */
CAssetStoreServiceConfig
MakeSettingsConfig( const SSettingsFixture& fixture, const CString& stateDirectory, bool allowChanges )
{
    CAssetStoreServiceConfig config;
    config.primaryStore.locations.clear();
    CAssetStorageLocation main;
    main.locationId = "main";
    main.name = "Main";
    main.vfsRoot = fixture.base + "/main";
    config.primaryStore.locations.push_back( main );
    config.primaryStore.defaultLocationId = "main";
    config.primaryStore.analysisConcurrency = 1;
    config.primaryStore.database = fixture.database;
    config.credentialStore = fixture.credentials;
    if ( GUCEF_NULL != fixture.credentials )
    {
        config.primaryStore.credentialProvider = "gucefASSETSTORE.test";
        config.primaryStore.credentialAccount = "catalogue";
    }
    config.loadDownloadQueue = false;
    config.stateDirectory = stateDirectory;
    config.allowSettingsChanges = allowChanges;
    config.reprocess.maxConcurrentJobs = 2;
    config.downloads.maxAttempts = 3;
    config.downloadStagingKeepHours = 24;
    return config;
}

/** The SettingsChangedEvent of a service */
class CSettingsEventRecorder : public CObservingNotifier
{
    public:

    typedef CTEventHandlerFunctor< CSettingsEventRecorder > TEventCallback;

    GUCEF::vector< CAssetSettingsChangeResult > changes;

    explicit CSettingsEventRecorder( CIAssetStoreService& service )
        : CObservingNotifier(), changes()
    {
        TEventCallback callback( this, &CSettingsEventRecorder::OnChanged );
        SubscribeTo( &service, CIAssetStoreService::SettingsChangedEvent, callback );
    }

    virtual ~CSettingsEventRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    /** Pumps until the events are there; false when the time ran out */
    bool WaitFor( UInt32 count, UInt32 timeoutInMs = 10000 )
    {
        for ( UInt32 waited=0; waited<timeoutInMs && changes.size()<count; waited+=10 )
            PumpFor( 10 );
        return changes.size() >= count;
    }

    private:

    void OnChanged( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        const CAssetSettingsChangeResult* change = CICloneable::CloneableAs< CAssetSettingsChangeResult >( eventData );
        if ( GUCEF_NULL != change )
            changes.push_back( *change );
    }
};

CAssetSettingsRequest
RemoteCaller( const char* deviceId, const char* name, bool owner )
{
    CAssetSettingsRequest request;
    request.SetRemoteCaller( deviceId, name, owner, owner );
    return request;
}

CAssetSettingsRequest
WithValue( const CAssetSettingsRequest& caller, const char* key, const char* value )
{
    CAssetSettingsRequest request( caller );
    request.values[ key ] = value;
    return request;
}

CAssetSettingsRequest
WithKey( const CAssetSettingsRequest& caller, const char* key )
{
    CAssetSettingsRequest request( caller );
    request.keys.push_back( key );
    return request;
}

/** The value of the key in a description, "<none>" when it is not described */
CString
ValueOf( const CAssetSettingsDescription& description, const char* key )
{
    const CAssetSettingState* state = description.Find( key );
    return GUCEF_NULL != state ? state->value : CString( "<none>" );
}

CString
SourceOf( const CAssetSettingsDescription& description, const char* key )
{
    const CAssetSettingState* state = description.Find( key );
    return GUCEF_NULL != state ? state->source : CString( "<none>" );
}

/** The audit entries of the service with the event and the key (all when they are empty); the last one in 'last' */
UInt32
CountAudit( CIAssetStoreService& service, const CString& event, const CString& key, CDataNode* last = GUCEF_NULL )
{
    UInt32 count = 0;
    CString token;
    do
    {
        CAssetServiceDocument page;
        if ( ASSETSTORE_STATUS_OK != service.GetAuditRecords( token, 0, page ) )
            return count;
        const CDataNode* entries = page.document.FindChild( "entries" );
        if ( GUCEF_NULL != entries )
        {
            for ( CDataNode::const_iterator i = entries->ConstBegin(); i != entries->ConstEnd(); ++i )
            {
                if ( ( event.IsNULLOrEmpty() || event == (*i)->GetAttributeValueOrChildValueByName( "event" ).AsString( CString::Empty, true ) ) &&
                     ( key.IsNULLOrEmpty() || key == (*i)->GetAttributeValueOrChildValueByName( "key" ).AsString( CString::Empty, true ) ) )
                {
                    ++count;
                    if ( GUCEF_NULL != last )
                        *last = *(*i);
                }
            }
        }
        token = page.nextPageToken;
    }
    while ( !token.IsNULLOrEmpty() );
    return count;
}

CString
Attr( const CDataNode& node, const char* name )
{
    return node.GetAttributeValueOrChildValueByName( name ).AsString( CString::Empty, true );
}

CString
ReadDiskText( const CString& path )
{
    CDynamicBuffer buffer;
    if ( !buffer.LoadContentFromFile( path ) )
        return CString::Empty;
    return CString( buffer.AsConstTypePtr< char >(), buffer.GetDataSize() );
}

bool
WriteDiskText( const CString& path, const CString& text )
{
    // the whole file, also when one is there ("wb", not written over in place)
    CDynamicBuffer buffer;
    buffer.CopyFrom( text.Length(), text.C_String() );
    return buffer.WriteContentToFile( path, 0, true );
}

/*-------------------------------------------------------------------------*/

void
RunServiceSettingsUnitTests( void )
{
    typedef CAssetSettingDescriptor D;

    GUCEF_TESTFW_TESTCASE( "Settings: a descriptor checks a value of its type, range, choices and length and gives the canonical text" )
        try
        {
            CString value;
            CString problem;
            const D flag = D::MakeBool( "a.flag", "G", true, "a flag" );
            ASSERT_TRUE( flag.Normalize( " TRUE ", value, problem ) && "true" == value );
            ASSERT_TRUE( flag.Normalize( "0", value, problem ) && "false" == value );
            ASSERT_TRUE( !flag.Normalize( "maybe", value, problem ) && problem.HasSubstr( "true or false" ) >= 0 );
            const D count = D::MakeUInt( "a.count", "G", 0, 256, 4, "jobs", "a count" );
            ASSERT_TRUE( count.Normalize( " 007 ", value, problem ) && "7" == value );
            ASSERT_TRUE( !count.Normalize( "-1", value, problem ) && !count.Normalize( "257", value, problem ) && !count.Normalize( "1.5", value, problem ) );
            ASSERT_TRUE( !count.Normalize( "", value, problem ) && !count.Normalize( "18446744073709551616", value, problem ) && problem.HasSubstr( "0 to 256" ) >= 0 );
            const D big = D::MakeUInt( "a.big", "G", 0, GUCEF_UINT64MAX, 0, "", "a big one" );
            ASSERT_TRUE( big.Normalize( "18446744073709551615", value, problem ) && "18446744073709551615" == value );
            const D offset = D::MakeInt( "a.offset", "G", -10, 10, 0, "", "an offset" );
            ASSERT_TRUE( offset.Normalize( "-5", value, problem ) && "-5" == value && offset.Normalize( "+3", value, problem ) && "3" == value );
            ASSERT_TRUE( !offset.Normalize( "-11", value, problem ) && !offset.Normalize( "--1", value, problem ) );
            const D factor = D::MakeFloat( "a.factor", "G", 0.5, 1000.0, 1.0, "x", "a factor" );
            ASSERT_TRUE( factor.Normalize( "1.5e2", value, problem ) && ToString( 150.0 ) == value );
            ASSERT_TRUE( !factor.Normalize( "abc", value, problem ) && !factor.Normalize( "1.2.3", value, problem ) && !factor.Normalize( "0.1", value, problem ) && !factor.Normalize( "1e", value, problem ) );
            const D name = D::MakeString( "a.name", "G", 4, "ab", "a name" );
            ASSERT_TRUE( name.Normalize( "abcd", value, problem ) && "abcd" == value );
            ASSERT_TRUE( !name.Normalize( "abcde", value, problem ) && !name.Normalize( "a\tb", value, problem ) );
            CString::StringVector orders;
            orders.push_back( "newest" );
            orders.push_back( "oldest" );
            const D order = D::MakeEnum( "a.order", "G", orders, "newest", "an order" );
            ASSERT_TRUE( order.Normalize( "Oldest", value, problem ) && "oldest" == value );
            ASSERT_TRUE( !order.Normalize( "sideways", value, problem ) && problem.HasSubstr( "one of newest, oldest" ) >= 0 );
            ASSERT_TRUE( D::ApplyRestart == count.AppliedAtRestart().applyMode && !count.OwnerOnlyRead().IsPublic() && count.IsPublic() && count.AppliesLive() );

            // what a descriptor that cannot be used says
            ASSERT_TRUE( count.IsValid( problem ) && flag.IsValid( problem ) && order.IsValid( problem ) );
            ASSERT_TRUE( !D::MakeUInt( "a..b", "G", 0, 1, 0, "", "" ).IsValid( problem ) && !D::MakeUInt( ".a", "G", 0, 1, 0, "", "" ).IsValid( problem ) );
            ASSERT_TRUE( !D::MakeUInt( "", "G", 0, 1, 0, "", "" ).IsValid( problem ) && !D::MakeUInt( "a b", "G", 0, 1, 0, "", "" ).IsValid( problem ) );
            ASSERT_TRUE( !D::MakeUInt( "a.c", "G", 0, 1, 5, "", "" ).IsValid( problem ) && problem.HasSubstr( "default" ) >= 0 );
            D unknown = count;
            unknown.type = "colour";
            ASSERT_TRUE( !unknown.IsValid( problem ) );
            ASSERT_TRUE( !D::MakeEnum( "a.e", "G", CString::StringVector(), "", "" ).IsValid( problem ) );

            // the descriptor as data, as it travels
            CDataNode node( "descriptor", GUCEF_DATATYPE_OBJECT );
            CDataNodeSerializableSettings settings;
            ASSERT_TRUE( order.OwnerOnlyRead().AppliedAtRestart().Serialize( node, settings ) );
            D copy;
            ASSERT_TRUE( copy.Deserialize( node, settings ) && "a.order" == copy.key && 2 == copy.allowedValues.size() && !copy.IsPublic() && !copy.AppliesLive() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Settings: the registry keeps one descriptor per key and refuses a key twice; the binding to the configuration registers 26 settings, reads and writes them, and leaves paths, pools and allowSettingsChanges out" )
        try
        {
            CAssetSettingsRegistry registry;
            CString reason;
            ASSERT_TRUE( registry.Register( D::MakeUInt( "x.y", "G", 0, 9, 1, "", "" ), reason ) );
            ASSERT_TRUE( !registry.Register( D::MakeUInt( "x.y", "G", 0, 9, 1, "", "" ), reason ) && reason.HasSubstr( "registered already" ) >= 0 );
            CString value;
            CString problem;
            ASSERT_TRUE( !registry.Normalize( "x.z", "1", value, problem ) && "not a setting of this service" == problem );
            ASSERT_TRUE( registry.Normalize( "x.y", "9", value, problem ) && 1 == registry.GetCount() );

            CAssetSettingsRegistry service;
            ASSERT_TRUE( CAssetStoreServiceConfigSettings::RegisterDescriptors( service, reason ) && RegisteredSettingCount == service.GetCount() );
            static const char* const absent[] = { "allowSettingsChanges", "stateDirectory", "thumbnailsDirectory", "imageCacheFolder", "threadPoolName", "downloads.queueFilePath",
                                                  "downloads.threadPoolName", "downloads.autoSaveQueue", "implementationId", "loadDownloadQueue", "startDownloads" };
            for ( UInt32 a=0; a<(UInt32) ( sizeof( absent ) / sizeof( absent[ 0 ] ) ); ++a )
                ASSERT_TRUE( GUCEF_NULL == service.Find( absent[ a ] ) );
            ASSERT_TRUE( GUCEF_NULL != service.Find( "signInPromptSeconds" ) && !service.Find( "signInPromptSeconds" )->IsPublic() );
            ASSERT_TRUE( !service.Find( "resumeIngestsOnStart" )->AppliesLive() && !service.Find( "ingestResumeAttempts" )->AppliesLive() );
            ASSERT_TRUE( service.Find( "reprocess.maxConcurrentJobs" )->AppliesLive() && "Reprocessing" == service.Find( "reprocess.maxConcurrentJobs" )->group );

            // the values of the defaults are the defaults of the descriptors
            CAssetStoreServiceConfig config;
            CString::StringMap values;
            CAssetStoreServiceConfigSettings::ReadValues( service, config, values );
            ASSERT_TRUE( RegisteredSettingCount == values.size() );
            for ( UInt32 d=0; d<service.GetCount(); ++d )
                ASSERT_TRUE( values[ service.GetDescriptors()[ d ].key ] == service.GetDescriptors()[ d ].defaultValue );

            // written and read back
            ASSERT_TRUE( CAssetStoreServiceConfigSettings::WriteValue( config, "reprocess.maxConcurrentJobs", "7" ) && 7 == config.reprocess.maxConcurrentJobs );
            ASSERT_TRUE( CAssetStoreServiceConfigSettings::WriteValue( config, "reprocess.limitOwner", "true" ) && config.reprocess.limitOwner );
            ASSERT_TRUE( CAssetStoreServiceConfigSettings::WriteValue( config, "downloads.maxBytesPerSecond", "1048576" ) && 1048576 == config.downloads.maxBytesPerSecond );
            ASSERT_TRUE( CAssetStoreServiceConfigSettings::WriteValue( config, "downloads.maxAttempts", "9" ) && 9 == config.downloads.maxAttempts );
            ASSERT_TRUE( CAssetStoreServiceConfigSettings::WriteValue( config, "resumeIngestsOnStart", "false" ) && !config.resumeIngestsOnStart );
            ASSERT_TRUE( CAssetStoreServiceConfigSettings::WriteValue( config, "signInPromptSeconds", "120" ) && 120 == config.signInPromptSeconds );
            ASSERT_TRUE( !CAssetStoreServiceConfigSettings::WriteValue( config, "stateDirectory", "/tmp" ) && config.stateDirectory.IsNULLOrEmpty() );
            CAssetStoreServiceConfigSettings::ReadValues( service, config, values );
            ASSERT_TRUE( "7" == values[ "reprocess.maxConcurrentJobs" ] && "true" == values[ "reprocess.limitOwner" ] && "1048576" == values[ "downloads.maxBytesPerSecond" ] );
            ASSERT_TRUE( "false" == values[ "resumeIngestsOnStart" ] && "120" == values[ "signInPromptSeconds" ] );
            // allowSettingsChanges is a member of the site settings file only
            CDataNode site( "AssetStoreService", GUCEF_DATATYPE_OBJECT );
            CDataNodeSerializableSettings settings;
            config.allowSettingsChanges = false;
            ASSERT_TRUE( config.Serialize( site, settings ) );
            CAssetStoreServiceConfig read;
            ASSERT_TRUE( read.allowSettingsChanges && read.Deserialize( site, settings ) && !read.allowSettingsChanges );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Settings: the records of the operations survive their data node form: a request with dotted keys, a change result with errors and changes, a description" )
        try
        {
            CDataNodeSerializableSettings settings;
            CAssetSettingsRequest request = RemoteCaller( "dev-1", "device one", false );
            request.values[ "reprocess.maxConcurrentJobs" ] = "3";
            request.keys.push_back( "downloads.maxAttempts" );
            CDataNode node( "request", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( request.Serialize( node, settings ) );
            CAssetSettingsRequest requestCopy;
            ASSERT_TRUE( requestCopy.fullRead && !requestCopy.remoteClient && requestCopy.Deserialize( node, settings ) );
            ASSERT_TRUE( "3" == requestCopy.values[ "reprocess.maxConcurrentJobs" ] && 1 == requestCopy.keys.size() && "device:dev-1" == requestCopy.caller );
            ASSERT_TRUE( requestCopy.remoteClient && !requestCopy.fullRead && !requestCopy.mayChange && "device one" == requestCopy.callerName );

            CAssetSettingsChangeResult result;
            result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
            result.errors[ "a.b" ] = "not a setting of this service";
            CAssetSettingChange change;
            change.key = "c.d";
            change.oldValue = "1";
            change.newValue = "2";
            change.applied = CAssetSettingChange::AppliedRestart;
            result.changes.push_back( change );
            result.revision = 5;
            CDataNode resultNode( "result", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( result.Serialize( resultNode, settings ) );
            CAssetSettingsChangeResult resultCopy;
            ASSERT_TRUE( resultCopy.Deserialize( resultNode, settings ) && ASSETSTORE_STATUS_INVALID_ARGUMENT == resultCopy.status && 1 == resultCopy.errors.size() );
            ASSERT_TRUE( GUCEF_NULL != resultCopy.Find( "c.d" ) && "2" == resultCopy.Find( "c.d" )->newValue && CAssetSettingChange::AppliedRestart == resultCopy.Find( "c.d" )->applied && 5 == resultCopy.revision );
            change.HideValues();
            ASSERT_TRUE( !change.valuesShown && change.oldValue.IsNULLOrEmpty() && change.newValue.IsNULLOrEmpty() );

            CAssetSettingsDescription description;
            CAssetSettingState state;
            state.descriptor = CAssetSettingDescriptor::MakeUInt( "e.f", "G", 0, 9, 1, "", "" );
            state.value = "4";
            state.hasPendingValue = true;
            state.pendingValue = "5";
            state.source = CAssetSettingState::SourceOverridden;
            description.entries.push_back( state );
            description.changeRefusal = "no";
            CDataNode descriptionNode( "description", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( description.Serialize( descriptionNode, settings ) );
            CAssetSettingsDescription descriptionCopy;
            ASSERT_TRUE( descriptionCopy.Deserialize( descriptionNode, settings ) && 1 == descriptionCopy.entries.size() && "no" == descriptionCopy.changeRefusal );
            ASSERT_TRUE( "4" == ValueOf( descriptionCopy, "e.f" ) && descriptionCopy.Find( "e.f" )->hasPendingValue && "5" == descriptionCopy.Find( "e.f" )->pendingValue );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Settings: the store of the overrides writes its files through a temporary file and a rename, reads them back, keeps the newest 500 audit entries, and says when a file cannot be read" )
        try
        {
            const CString disk = MountFreshDirectory( "settings_unit_store", "astest/settings_unit_store" );
            CAssetSettingsOverrideStore store;
            ASSERT_TRUE( !store.IsPersistent() );
            CString problem;
            CString::StringMap overrides;
            overrides[ "reprocess.maxConcurrentJobs" ] = "3";
            overrides[ "downloadStagingKeepHours" ] = "12";
            ASSERT_TRUE( store.SetOverrides( overrides, problem ) );
            store.SetFolder( "astest/settings_unit_store" );
            ASSERT_TRUE( store.IsPersistent() && store.GetOverrides().empty() );
            ASSERT_TRUE( store.SetOverrides( overrides, problem ) );
            ASSERT_TRUE( FileExists( CombinePath( disk, CAssetSettingsOverrideStore::OverridesFileName ) ) );
            ASSERT_TRUE( !FileExists( CombinePath( disk, CAssetSettingsOverrideStore::OverridesFileName + ".tmp" ) ) );
            for ( UInt32 e=0; e<CAssetSettingsOverrideStore::MaxAuditEntries + 5; ++e )
            {
                CDataNode entry( "entry", GUCEF_DATATYPE_OBJECT );
                entry.SetAttribute( "index", e );
                ASSERT_TRUE( store.AddAuditEntry( entry ) );
            }
            ASSERT_TRUE( CAssetSettingsOverrideStore::MaxAuditEntries == store.GetAuditCount() );

            CAssetSettingsOverrideStore again;
            again.SetFolder( "astest/settings_unit_store" );
            ASSERT_TRUE( again.Load( problem ) );
            CString value;
            ASSERT_TRUE( 2 == again.GetOverrides().size() && again.GetOverride( "downloadStagingKeepHours", value ) && "12" == value );
            ASSERT_TRUE( CAssetSettingsOverrideStore::MaxAuditEntries == again.GetAuditCount() );
            GUCEF::vector< CDataNode > entries;
            again.GetAuditEntries( 0, 2, entries );
            ASSERT_TRUE( 2 == entries.size() && "5" == Attr( entries[ 0 ], "index" ) && "6" == Attr( entries[ 1 ], "index" ) );
            again.GetAuditEntries( CAssetSettingsOverrideStore::MaxAuditEntries - 1, 10, entries );
            ASSERT_TRUE( 1 == entries.size() && ToString( CAssetSettingsOverrideStore::MaxAuditEntries + 4 ) == Attr( entries[ 0 ], "index" ) );

            // a file that is not JSON: no overrides, and the reason
            ASSERT_TRUE( WriteDiskText( CombinePath( disk, CAssetSettingsOverrideStore::OverridesFileName ), "{ not json" ) );
            CAssetSettingsOverrideStore broken;
            broken.SetFolder( "astest/settings_unit_store" );
            ASSERT_TRUE( !broken.Load( problem ) && problem.HasSubstr( CAssetSettingsOverrideStore::OverridesFileName ) >= 0 && broken.GetOverrides().empty() );
            UnmountDirectory( "settings_unit_store" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/

/** In process: the operations, the checks, the restart, the reset, the audit, the event and allowSettingsChanges */
void
RunServiceSettingsSuite( SSettingsFixture& fixture )
{
    MountFreshDirectory( fixture.label + "_main", fixture.base + "/main" );
    const CString stateDirectory = TestDirectory( fixture.label + "_state" );
    if ( DirExists( stateDirectory ) )
        DeleteDir( stateDirectory, true );
    const CAssetStoreServiceConfig config = MakeSettingsConfig( fixture, stateDirectory, true );
    CString error;
    TAssetStoreServicePtr servicePtr = CAssetStoreGlobal::Instance()->CreateService( config, error );
    bool opened = !servicePtr.IsNULL() && ASSETSTORE_STATUS_OK == servicePtr->Open();
    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings: the service opens with a state folder" ).C_String() )
        ASSERT_TRUE( opened );
    GUCEF_TESTFW_TESTCASE_END;
    if ( !opened )
        return;
    CLocalAssetStoreService* local = static_cast< CLocalAssetStoreService* >( servicePtr.GetPointerAlways() );
    const CAssetSettingsRequest admin;
    const CAssetSettingsRequest owner = RemoteCaller( "dev-owner", "owner device", true );
    const CAssetSettingsRequest viewer = RemoteCaller( "dev-viewer", "viewer device", false );
    UInt32 setAudits = 0;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings: Describe gives every setting with its value, site value, default and source to the site administrator; a caller without the role gets no owner-only-read setting and may not change; paths, pools and allowSettingsChanges are not described" ).C_String() )
        try
        {
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( admin, description ) );
            ASSERT_TRUE( RegisteredSettingCount == description.entries.size() && description.persistent && description.canChange && description.changesAllowed && description.callerMayChange );
            ASSERT_TRUE( "2" == ValueOf( description, "reprocess.maxConcurrentJobs" ) && CAssetSettingState::SourceSite == SourceOf( description, "reprocess.maxConcurrentJobs" ) );
            ASSERT_TRUE( "0" == description.Find( "reprocess.maxConcurrentJobs" )->descriptor.defaultValue && "2" == description.Find( "reprocess.maxConcurrentJobs" )->siteValue );
            ASSERT_TRUE( "64" == ValueOf( description, "reprocess.maxQueuedJobs" ) && CAssetSettingState::SourceDefault == SourceOf( description, "reprocess.maxQueuedJobs" ) );
            ASSERT_TRUE( "3" == ValueOf( description, "downloads.maxAttempts" ) && "24" == ValueOf( description, "downloadStagingKeepHours" ) && "600" == ValueOf( description, "signInPromptSeconds" ) );
            ASSERT_TRUE( "<none>" == ValueOf( description, "allowSettingsChanges" ) && "<none>" == ValueOf( description, "stateDirectory" ) && "<none>" == ValueOf( description, "downloads.queueFilePath" ) );
            for ( size_t e=0; e<description.entries.size(); ++e )
                ASSERT_TRUE( description.entries[ e ].descriptor.key.HasSubstr( "Directory" ) < 0 && description.entries[ e ].descriptor.key.HasSubstr( "PoolName" ) < 0 );

            CAssetSettingsDescription seen;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( viewer, seen ) );
            ASSERT_TRUE( RegisteredSettingCount - 1 == seen.entries.size() && GUCEF_NULL == seen.Find( "signInPromptSeconds" ) );
            ASSERT_TRUE( !seen.canChange && !seen.callerMayChange && seen.changesAllowed && seen.changeRefusal.HasSubstr( "owner role" ) >= 0 );
            CAssetSettingsDescription ownerView;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( owner, ownerView ) && ownerView.canChange && RegisteredSettingCount == ownerView.entries.size() );
            CAssetServiceInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->GetServiceInfo( info ) && 1 == info.capabilities.count( "settings" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings: Set applies live settings at once (the reprocessing, the download manager, the staging sweep), keeps them, announces them with SettingsChangedEvent, audits them with the device and the old and new value; a repeat with its command id is answered from the first" ).C_String() )
        try
        {
            CSettingsEventRecorder recorder( *local );
            CAssetSettingsRequest request( owner );
            request.values[ "reprocess.maxQueuedJobs" ] = "10";
            request.values[ "downloads.maxConcurrentJobs" ] = " 7 ";
            request.values[ "downloadStagingKeepHours" ] = "12";
            CAssetSettingsChangeResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->SetSettings( "settings-set-1", request, result ) );
            ASSERT_TRUE( 3 == result.changes.size() && !result.noChange && !result.replayed && 1 == result.revision && "device:dev-owner" == result.caller );
            ASSERT_TRUE( CAssetSettingChange::AppliedLive == result.Find( "reprocess.maxQueuedJobs" )->applied && "64" == result.Find( "reprocess.maxQueuedJobs" )->oldValue );
            ASSERT_TRUE( "7" == result.Find( "downloads.maxConcurrentJobs" )->newValue && result.message.HasSubstr( "applies at once" ) >= 0 );
            ASSERT_TRUE( 10 == local->GetReprocessSettings().maxQueuedJobs && 7 == local->GetDownloadManager()->GetSettings().maxConcurrentJobs );
            CString value;
            ASSERT_TRUE( local->GetSettingValue( "downloadStagingKeepHours", value ) && "12" == value );
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( admin, description ) && 1 == description.revision );
            ASSERT_TRUE( "10" == ValueOf( description, "reprocess.maxQueuedJobs" ) && CAssetSettingState::SourceOverridden == SourceOf( description, "reprocess.maxQueuedJobs" ) );
            ASSERT_TRUE( "64" == description.Find( "reprocess.maxQueuedJobs" )->siteValue && !description.Find( "reprocess.maxQueuedJobs" )->hasPendingValue );
            ASSERT_TRUE( FileExists( CombinePath( stateDirectory, CAssetSettingsOverrideStore::OverridesFileName ) ) );
            ASSERT_TRUE( !FileExists( CombinePath( stateDirectory, CAssetSettingsOverrideStore::OverridesFileName + ".tmp" ) ) );
            ASSERT_TRUE( ReadDiskText( CombinePath( stateDirectory, CAssetSettingsOverrideStore::OverridesFileName ) ).HasSubstr( "reprocess.maxQueuedJobs" ) >= 0 );

            // the event, once
            ASSERT_TRUE( recorder.WaitFor( 1 ) && 3 == recorder.changes[ 0 ].changes.size() && "device:dev-owner" == recorder.changes[ 0 ].caller && 1 == recorder.changes[ 0 ].revision );
            // the audit: the device, its name, the key, the old and the new value
            CDataNode last;
            ASSERT_TRUE( 1 == CountAudit( *local, "settings-set", "reprocess.maxQueuedJobs", &last ) );
            ASSERT_TRUE( "device:dev-owner" == Attr( last, "caller" ) && "owner device" == Attr( last, "callerName" ) && "64" == Attr( last, "oldValue" ) && "10" == Attr( last, "newValue" ) );
            ASSERT_TRUE( "remote client" == Attr( last, "origin" ) );
            setAudits = CountAudit( *local, "settings-set", CString::Empty );
            ASSERT_TRUE( 3 == setAudits );

            // the same command id: the first outcome, nothing again
            CAssetSettingsChangeResult replay;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->SetSettings( "settings-set-1", request, replay ) && replay.replayed && 1 == replay.revision );
            CAssetSettingsChangeResult other;
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == local->ResetSettings( "settings-set-1", WithKey( owner, "reprocess.maxQueuedJobs" ), other ) );
            PumpFor( 100 );
            ASSERT_TRUE( 1 == recorder.changes.size() && setAudits == CountAudit( *local, "settings-set", CString::Empty ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings: a set to the values in effect changes nothing and says so (no event, no audit); a set with an unknown key, a value out of range or of the wrong type changes nothing at all and names every key that is not valid" ).C_String() )
        try
        {
            CSettingsEventRecorder recorder( *local );
            CAssetSettingsChangeResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->SetSettings( CString::Empty, WithValue( owner, "reprocess.maxQueuedJobs", "10" ), result ) );
            ASSERT_TRUE( result.noChange && 1 == result.changes.size() && CAssetSettingChange::AppliedUnchanged == result.changes[ 0 ].applied && result.message.HasSubstr( "Nothing changed" ) >= 0 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->SetSettings( CString::Empty, WithValue( owner, "reprocess.maxConcurrentJobs", "2" ), result ) && result.noChange );

            CAssetSettingsRequest request( owner );
            request.values[ "reprocess.maxConcurrentJobs" ] = "9999";
            request.values[ "no.such.key" ] = "1";
            request.values[ "reprocess.limitOwner" ] = "maybe";
            request.values[ "downloadStagingKeepHours" ] = "5";
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == local->SetSettings( CString::Empty, request, result ) );
            ASSERT_TRUE( 3 == result.errors.size() && 1 == result.errors.count( "reprocess.maxConcurrentJobs" ) && 1 == result.errors.count( "no.such.key" ) && 1 == result.errors.count( "reprocess.limitOwner" ) );
            ASSERT_TRUE( "not a setting of this service" == result.errors[ "no.such.key" ] && result.message.HasSubstr( "Nothing was changed" ) == 0 && result.changes.empty() );
            CString value;
            ASSERT_TRUE( local->GetSettingValue( "downloadStagingKeepHours", value ) && "12" == value && 2 == local->GetReprocessSettings().maxConcurrentJobs );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == local->SetSettings( CString::Empty, owner, result ) );
            PumpFor( 100 );
            ASSERT_TRUE( recorder.changes.empty() && setAudits == CountAudit( *local, "settings-set", CString::Empty ) );
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( admin, description ) && 1 == description.revision );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings: a setting that applies at the next start is kept and reported as pending; the value of an owner-only-read setting is left out of the event and the audit but given to the caller that may read it" ).C_String() )
        try
        {
            CSettingsEventRecorder recorder( *local );
            CAssetSettingsRequest request( owner );
            request.values[ "resumeIngestsOnStart" ] = "false";
            request.values[ "ingestResumeAttempts" ] = "5";
            CAssetSettingsChangeResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->SetSettings( CString::Empty, request, result ) );
            ASSERT_TRUE( CAssetSettingChange::AppliedRestart == result.Find( "resumeIngestsOnStart" )->applied && result.message.HasSubstr( "applies after a restart" ) >= 0 );
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( admin, description ) );
            const CAssetSettingState* resume = description.Find( "resumeIngestsOnStart" );
            ASSERT_TRUE( GUCEF_NULL != resume && "true" == resume->value && resume->hasPendingValue && "false" == resume->pendingValue && CAssetSettingState::SourceOverridden == resume->source );
            ASSERT_TRUE( "3" == ValueOf( description, "ingestResumeAttempts" ) && "5" == description.Find( "ingestResumeAttempts" )->pendingValue );

            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->SetSettings( CString::Empty, WithValue( owner, "signInPromptSeconds", "900" ), result ) );
            ASSERT_TRUE( result.Find( "signInPromptSeconds" )->valuesShown && "900" == result.Find( "signInPromptSeconds" )->newValue );
            ASSERT_TRUE( recorder.WaitFor( 2 ) );
            const CAssetSettingChange* announced = recorder.changes[ 1 ].Find( "signInPromptSeconds" );
            ASSERT_TRUE( GUCEF_NULL != announced && !announced->valuesShown && announced->newValue.IsNULLOrEmpty() && announced->oldValue.IsNULLOrEmpty() );
            CDataNode last;
            ASSERT_TRUE( 1 == CountAudit( *local, "settings-set", "signInPromptSeconds", &last ) && Attr( last, "newValue" ).IsNULLOrEmpty() && Attr( last, "oldValue" ).IsNULLOrEmpty() );
            CString value;
            ASSERT_TRUE( local->GetSettingValue( "signInPromptSeconds", value ) && "900" == value );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings: the overrides are applied over the site settings at the next start: the live ones and the ones that waited for the restart; the audit is kept too" ).C_String() )
        try
        {
            const UInt32 audits = CountAudit( *local, CString::Empty, CString::Empty );
            servicePtr->Close();
            local = GUCEF_NULL;
            servicePtr = CAssetStoreGlobal::Instance()->CreateService( config, error );
            opened = !servicePtr.IsNULL() && ASSETSTORE_STATUS_OK == servicePtr->Open();
            ASSERT_TRUE( opened );
            local = static_cast< CLocalAssetStoreService* >( servicePtr.GetPointerAlways() );
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( admin, description ) && 0 == description.revision );
            ASSERT_TRUE( "10" == ValueOf( description, "reprocess.maxQueuedJobs" ) && CAssetSettingState::SourceOverridden == SourceOf( description, "reprocess.maxQueuedJobs" ) );
            ASSERT_TRUE( "false" == ValueOf( description, "resumeIngestsOnStart" ) && !description.Find( "resumeIngestsOnStart" )->hasPendingValue );
            ASSERT_TRUE( "5" == ValueOf( description, "ingestResumeAttempts" ) && "900" == ValueOf( description, "signInPromptSeconds" ) && "12" == ValueOf( description, "downloadStagingKeepHours" ) );
            ASSERT_TRUE( 10 == local->GetReprocessSettings().maxQueuedJobs && 7 == local->GetDownloadManager()->GetSettings().maxConcurrentJobs );
            ASSERT_TRUE( "2" == ValueOf( description, "reprocess.maxConcurrentJobs" ) && CAssetSettingState::SourceSite == SourceOf( description, "reprocess.maxConcurrentJobs" ) );
            ASSERT_TRUE( audits > 0 && audits == CountAudit( *local, CString::Empty, CString::Empty ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
    if ( !opened )
        return;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings: Reset puts settings back to the site value at once and drops their overrides; a reset of a setting without an override changes nothing; an unknown key is refused" ).C_String() )
        try
        {
            CSettingsEventRecorder recorder( *local );
            CAssetSettingsRequest request( owner );
            request.keys.push_back( "reprocess.maxQueuedJobs" );
            request.keys.push_back( "downloads.maxConcurrentJobs" );
            CAssetSettingsChangeResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->ResetSettings( "settings-reset-1", request, result ) && 2 == result.changes.size() && "reset" == result.operation );
            ASSERT_TRUE( "64" == result.Find( "reprocess.maxQueuedJobs" )->newValue && "10" == result.Find( "reprocess.maxQueuedJobs" )->oldValue && "4" == result.Find( "downloads.maxConcurrentJobs" )->newValue );
            ASSERT_TRUE( 64 == local->GetReprocessSettings().maxQueuedJobs && 4 == local->GetDownloadManager()->GetSettings().maxConcurrentJobs );
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( admin, description ) && CAssetSettingState::SourceDefault == SourceOf( description, "reprocess.maxQueuedJobs" ) );
            ASSERT_TRUE( ReadDiskText( CombinePath( stateDirectory, CAssetSettingsOverrideStore::OverridesFileName ) ).HasSubstr( "reprocess.maxQueuedJobs" ) < 0 );
            ASSERT_TRUE( recorder.WaitFor( 1 ) && 2 == CountAudit( *local, "settings-reset", CString::Empty ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->ResetSettings( CString::Empty, WithKey( owner, "reprocess.maxQueuedJobs" ), result ) && result.noChange );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == local->ResetSettings( CString::Empty, WithKey( owner, "no.such.key" ), result ) && 1 == result.errors.count( "no.such.key" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == local->ResetSettings( CString::Empty, owner, result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings: an override of the state folder that is no longer a setting or no longer valid is left out at the start; the others apply" ).C_String() )
        try
        {
            servicePtr->Close();
            local = GUCEF_NULL;
            const CString file = CombinePath( stateDirectory, CAssetSettingsOverrideStore::OverridesFileName );
            ASSERT_TRUE( WriteDiskText( file, "{ \"version\": 1, \"overrides\": [ { \"key\": \"reprocess.maxQueuedJobs\", \"value\": \"11\" }, "
                                              "{ \"key\": \"gone.key\", \"value\": \"1\" }, { \"key\": \"downloads.maxAttempts\", \"value\": \"500\" } ] }" ) );
            servicePtr = CAssetStoreGlobal::Instance()->CreateService( config, error );
            opened = !servicePtr.IsNULL() && ASSETSTORE_STATUS_OK == servicePtr->Open();
            ASSERT_TRUE( opened );
            local = static_cast< CLocalAssetStoreService* >( servicePtr.GetPointerAlways() );
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( admin, description ) );
            ASSERT_TRUE( "11" == ValueOf( description, "reprocess.maxQueuedJobs" ) && "3" == ValueOf( description, "downloads.maxAttempts" ) );
            ASSERT_TRUE( CAssetSettingState::SourceSite == SourceOf( description, "downloads.maxAttempts" ) && 11 == local->GetReprocessSettings().maxQueuedJobs );
            const CString kept = ReadDiskText( file );
            ASSERT_TRUE( kept.HasSubstr( "gone.key" ) < 0 && kept.HasSubstr( "reprocess.maxQueuedJobs" ) >= 0 );
            CAssetSettingsChangeResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->ResetSettings( CString::Empty, WithKey( admin, "reprocess.maxQueuedJobs" ), result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
    if ( !opened )
        return;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings: with allowSettingsChanges false a change by a client is refused with the reason and audited, the description still works, and the site administrator may change them" ).C_String() )
        try
        {
            servicePtr->Close();
            local = GUCEF_NULL;
            const CAssetStoreServiceConfig closed = MakeSettingsConfig( fixture, stateDirectory, false );
            servicePtr = CAssetStoreGlobal::Instance()->CreateService( closed, error );
            opened = !servicePtr.IsNULL() && ASSETSTORE_STATUS_OK == servicePtr->Open();
            ASSERT_TRUE( opened );
            local = static_cast< CLocalAssetStoreService* >( servicePtr.GetPointerAlways() );
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( owner, description ) && RegisteredSettingCount == description.entries.size() );
            ASSERT_TRUE( !description.changesAllowed && description.callerMayChange && !description.canChange );
            ASSERT_TRUE( "settings changes by clients are switched off on this server" == description.changeRefusal );
            CSettingsEventRecorder recorder( *local );
            const UInt32 refusals = CountAudit( *local, "settings-refused", CString::Empty );
            CAssetSettingsChangeResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == local->SetSettings( CString::Empty, WithValue( owner, "reprocess.maxQueuedJobs", "20" ), result ) );
            ASSERT_TRUE( "settings changes by clients are switched off on this server" == result.message && result.changes.empty() && 64 == local->GetReprocessSettings().maxQueuedJobs );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == local->ResetSettings( CString::Empty, WithKey( owner, "downloadStagingKeepHours" ), result ) );
            CDataNode last;
            ASSERT_TRUE( refusals + 2 == CountAudit( *local, "settings-refused", CString::Empty, &last ) && "device:dev-owner" == Attr( last, "caller" ) );
            ASSERT_TRUE( Attr( last, "reason" ).HasSubstr( "switched off" ) >= 0 );

            // the site administrator: allowed whatever the flag says, and audited
            CAssetSettingsRequest siteAdmin;
            siteAdmin.caller = "admin";
            siteAdmin.callerName = "the admin command line";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->DescribeSettings( siteAdmin, description ) && description.canChange && !description.changesAllowed );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->SetSettings( CString::Empty, WithValue( siteAdmin, "reprocess.maxQueuedJobs", "20" ), result ) && 20 == local->GetReprocessSettings().maxQueuedJobs );
            ASSERT_TRUE( 2 == CountAudit( *local, "settings-set", "reprocess.maxQueuedJobs", &last ) && "admin" == Attr( last, "caller" ) && "site administrator" == Attr( last, "origin" ) );
            ASSERT_TRUE( recorder.WaitFor( 1 ) && 1 == recorder.changes.size() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == local->ResetSettings( CString::Empty, WithKey( siteAdmin, "reprocess.maxQueuedJobs" ), result ) );
            // the host refuses a device without the role before the service; the service keeps that refusal in its audit
            local->AuditSettingsRefusal( WithValue( viewer, "reprocess.maxQueuedJobs", "30" ), CAssetServiceOperations::SetSettings, "the settings of the service are changed by the devices with the owner role" );
            ASSERT_TRUE( 1 <= CountAudit( *local, "settings-refused", "reprocess.maxQueuedJobs", &last ) && "device:dev-viewer" == Attr( last, "caller" ) && "30" == Attr( last, "newValue" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    if ( !servicePtr.IsNULL() )
        servicePtr->Close();
    DeleteDir( stateDirectory, true );
    UnmountDirectory( fixture.label + "_main" );
}

/*-------------------------------------------------------------------------*/

/** Over loopback DRN with a device of the owner role, one of the machine role and a viewer */
void
RunServiceSettingsRemoteSuite( SSettingsFixture& fixture )
{
    MountFreshDirectory( fixture.label + "_main", fixture.base + "/main" );
    CRemoteAssetStoreWorld world( fixture.label );
    const CString stateDirectory = CombinePath( world.directory, "service_state" );
    world.bindingSettings.roles.insert( "viewer" );
    world.bindingSettings.roles.insert( "mcp" );
    CString error;
    const bool serverStarted = world.StartServer( MakeSettingsConfig( fixture, stateDirectory, true ), error );
    TAssetStoreServicePtr owner = serverStarted ? world.MakeClient( "owner", error ) : TAssetStoreServicePtr();
    CString::StringSet viewerRoles;
    viewerRoles.insert( "viewer" );
    TAssetStoreServicePtr viewer = serverStarted ? world.MakeClient( "viewer", error, &viewerRoles, false ) : TAssetStoreServicePtr();
    CString::StringSet machineRoles;
    machineRoles.insert( "mcp" );
    TAssetStoreServicePtr machine = serverStarted ? world.MakeClient( "machine", error, &machineRoles, false ) : TAssetStoreServicePtr();
    bool clientsOpen = !owner.IsNULL() && !viewer.IsNULL() && !machine.IsNULL() &&
                       ASSETSTORE_STATUS_OK == owner->Open() && ASSETSTORE_STATUS_OK == viewer->Open() && ASSETSTORE_STATUS_OK == machine->Open();
    if ( !clientsOpen )
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ServiceSettingsRemote: the world did not start: " << error );
    CLocalAssetStoreService* local = serverStarted ? static_cast< CLocalAssetStoreService* >( world.serverService.GetPointerAlways() ) : GUCEF_NULL;
    const CAssetSettingsRequest asked;      // what a client sends: the host sets who asks

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings remote: Describe for every role; the host says who asks, whatever the client sent: the owner device reads all and may change, the viewer and the machine device read the public settings only and may not change" ).C_String() )
        try
        {
            ASSERT_TRUE( clientsOpen && GUCEF_NULL != local );
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->DescribeSettings( asked, description ) && RegisteredSettingCount == description.entries.size() && description.canChange );
            ASSERT_TRUE( "2" == ValueOf( description, "reprocess.maxConcurrentJobs" ) && "600" == ValueOf( description, "signInPromptSeconds" ) && description.persistent );
            // a client that says it is the site administrator is still the device it is
            CAssetSettingsRequest pretending;
            pretending.remoteClient = false;
            pretending.fullRead = true;
            pretending.mayChange = true;
            pretending.caller = "admin";
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == viewer->DescribeSettings( pretending, description ) && RegisteredSettingCount - 1 == description.entries.size() );
            ASSERT_TRUE( GUCEF_NULL == description.Find( "signInPromptSeconds" ) && !description.canChange && !description.callerMayChange );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == machine->DescribeSettings( asked, description ) && RegisteredSettingCount - 1 == description.entries.size() && !description.canChange );
            ASSERT_TRUE( description.changeRefusal.HasSubstr( "owner role" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings remote: the owner device sets a live setting: applied on the server at once, the other client sees SettingsChangedEvent (without the values of an owner-only-read setting), a repeat with the command id is answered from the first, an invalid set names every key over the wire" ).C_String() )
        try
        {
            ASSERT_TRUE( clientsOpen && GUCEF_NULL != local );
            CSettingsEventRecorder seen( *viewer );
            CAssetSettingsChangeResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->SetSettings( "remote-settings-1", WithValue( asked, "reprocess.maxConcurrentJobs", "5" ), result ) && !result.replayed );
            ASSERT_TRUE( CAssetSettingChange::AppliedLive == result.Find( "reprocess.maxConcurrentJobs" )->applied && 5 == local->GetReprocessSettings().maxConcurrentJobs );
            ASSERT_TRUE( 5 == local->GetReprocessGuard().GetBudget().concurrentJobs );
            ASSERT_TRUE( seen.WaitFor( 1 ) && GUCEF_NULL != seen.changes[ 0 ].Find( "reprocess.maxConcurrentJobs" ) && "5" == seen.changes[ 0 ].Find( "reprocess.maxConcurrentJobs" )->newValue );
            ASSERT_TRUE( seen.changes[ 0 ].caller.HasSubstr( "device:" ) == 0 );
            CAssetSettingsChangeResult again;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->SetSettings( "remote-settings-1", WithValue( asked, "reprocess.maxConcurrentJobs", "5" ), again ) && again.replayed );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->SetSettings( CString::Empty, WithValue( asked, "signInPromptSeconds", "700" ), result ) && "700" == result.Find( "signInPromptSeconds" )->newValue );
            ASSERT_TRUE( seen.WaitFor( 2 ) && !seen.changes[ 1 ].Find( "signInPromptSeconds" )->valuesShown && seen.changes[ 1 ].Find( "signInPromptSeconds" )->newValue.IsNULLOrEmpty() );

            CAssetSettingsRequest invalid( asked );
            invalid.values[ "reprocess.maxConcurrentJobs" ] = "300";
            invalid.values[ "no.such.key" ] = "x";
            invalid.values[ "downloadStagingKeepHours" ] = "6";
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == owner->SetSettings( CString::Empty, invalid, result ) );
            ASSERT_TRUE( 2 == result.errors.size() && 1 == result.errors.count( "no.such.key" ) && 5 == local->GetReprocessSettings().maxConcurrentJobs );
            CString value;
            ASSERT_TRUE( local->GetSettingValue( "downloadStagingKeepHours", value ) && "24" == value );

            // a setting that applies at the next start
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->SetSettings( CString::Empty, WithValue( asked, "ingestResumeAttempts", "7" ), result ) );
            ASSERT_TRUE( CAssetSettingChange::AppliedRestart == result.Find( "ingestResumeAttempts" )->applied );
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->DescribeSettings( asked, description ) && "3" == ValueOf( description, "ingestResumeAttempts" ) );
            ASSERT_TRUE( description.Find( "ingestResumeAttempts" )->hasPendingValue && "7" == description.Find( "ingestResumeAttempts" )->pendingValue );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings remote: the viewer and the machine device are refused a set and a reset (PERMISSION_DENIED at the host), nothing changes, the refusals are audited with the device; the audit records are for the owner device" ).C_String() )
        try
        {
            ASSERT_TRUE( clientsOpen && GUCEF_NULL != local );
            CAssetSettingsChangeResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == viewer->SetSettings( CString::Empty, WithValue( asked, "reprocess.maxConcurrentJobs", "1" ), result ) );
            ASSERT_TRUE( result.message.HasSubstr( "owner role" ) >= 0 );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == machine->SetSettings( CString::Empty, WithValue( asked, "reprocess.maxConcurrentJobs", "1" ), result ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == machine->ResetSettings( CString::Empty, WithKey( asked, "reprocess.maxConcurrentJobs" ), result ) );
            ASSERT_TRUE( 5 == local->GetReprocessSettings().maxConcurrentJobs );
            CDataNode last;
            ASSERT_TRUE( 3 == CountAudit( *local, "settings-refused", "reprocess.maxConcurrentJobs", &last ) );
            ASSERT_TRUE( 0 == Attr( last, "caller" ).HasSubstr( "device:" ) && "test device machine" == Attr( last, "callerName" ) );
            CAssetServiceDocument audit;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == owner->GetAuditRecords( CString::Empty, 0, audit ) && 0 < audit.document.GetAttributeValueOrChildValueByName( "total" ).AsUInt32( 0, true ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == viewer->GetAuditRecords( CString::Empty, 0, audit ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == machine->GetAuditRecords( CString::Empty, 0, audit ) );
            CDataNode ownerEntry;
            ASSERT_TRUE( 1 <= CountAudit( *local, "settings-set", "reprocess.maxConcurrentJobs", &ownerEntry ) && "test device owner" == Attr( ownerEntry, "callerName" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( fixture.label + ": settings remote: after a restart of the server the overrides apply (also the one that waited for the restart); with allowSettingsChanges false a set of the owner device is refused with the reason and the settings are still read; open again, the owner device resets" ).C_String() )
        try
        {
            ASSERT_TRUE( clientsOpen );
            owner->Close();
            viewer->Close();
            machine->Close();
            owner.Unlink();
            viewer.Unlink();
            machine.Unlink();
            local = GUCEF_NULL;
            world.StopServer();
            ASSERT_TRUE( world.StartServer( MakeSettingsConfig( fixture, stateDirectory, false ), error ) );
            TAssetStoreServicePtr reopened = world.MakeClient( "owner", error );
            ASSERT_TRUE( !reopened.IsNULL() && ASSETSTORE_STATUS_OK == reopened->Open() );
            local = static_cast< CLocalAssetStoreService* >( world.serverService.GetPointerAlways() );
            CAssetSettingsDescription description;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == reopened->DescribeSettings( asked, description ) );
            ASSERT_TRUE( "5" == ValueOf( description, "reprocess.maxConcurrentJobs" ) && "7" == ValueOf( description, "ingestResumeAttempts" ) && "700" == ValueOf( description, "signInPromptSeconds" ) );
            ASSERT_TRUE( !description.changesAllowed && !description.canChange && "settings changes by clients are switched off on this server" == description.changeRefusal );
            CAssetSettingsChangeResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_REFUSED == reopened->SetSettings( CString::Empty, WithValue( asked, "reprocess.maxConcurrentJobs", "1" ), result ) );
            ASSERT_TRUE( "settings changes by clients are switched off on this server" == result.message && 5 == local->GetReprocessSettings().maxConcurrentJobs );
            reopened->Close();
            reopened.Unlink();
            local = GUCEF_NULL;
            world.StopServer();

            ASSERT_TRUE( world.StartServer( MakeSettingsConfig( fixture, stateDirectory, true ), error ) );
            reopened = world.MakeClient( "owner", error );
            ASSERT_TRUE( !reopened.IsNULL() && ASSETSTORE_STATUS_OK == reopened->Open() );
            local = static_cast< CLocalAssetStoreService* >( world.serverService.GetPointerAlways() );
            CAssetSettingsRequest reset( asked );
            reset.keys.push_back( "reprocess.maxConcurrentJobs" );
            reset.keys.push_back( "ingestResumeAttempts" );
            reset.keys.push_back( "signInPromptSeconds" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == reopened->ResetSettings( CString::Empty, reset, result ) && 3 == result.changes.size() );
            ASSERT_TRUE( 2 == local->GetReprocessSettings().maxConcurrentJobs && CAssetSettingChange::AppliedRestart == result.Find( "ingestResumeAttempts" )->applied );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == reopened->DescribeSettings( asked, description ) && CAssetSettingState::SourceSite == SourceOf( description, "reprocess.maxConcurrentJobs" ) );
            ASSERT_TRUE( "7" == ValueOf( description, "ingestResumeAttempts" ) && "3" == description.Find( "ingestResumeAttempts" )->pendingValue );
            reopened->Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    if ( !owner.IsNULL() )
        owner->Close();
    if ( !viewer.IsNULL() )
        viewer->Close();
    if ( !machine.IsNULL() )
        machine->Close();
    world.StopServer();
    UnmountDirectory( fixture.label + "_main" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformServiceSettingsUnitTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ServiceSettingsUnit" );
    RunServiceSettingsUnitTests();
}

/*-------------------------------------------------------------------------*/

void
PerformServiceSettingsVfsTablesTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ServiceSettingsVfsTables" );

    SSettingsFixture fixture;
    fixture.label = "st_vfstables";
    fixture.base = "astest/st_vfstables";
    MountFreshDirectory( "st_vfstables_catalogue", "astest/st_vfstables/catalogue" );
    fixture.database.backendName = "vfstables";
    fixture.database.database = "astest/st_vfstables/catalogue/db";
    fixture.database.backendOptions.SetAllowMultipleValues( false );
    fixture.database.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    RunServiceSettingsSuite( fixture );
    UnmountDirectory( "st_vfstables_catalogue" );

    SSettingsFixture remote( fixture );
    remote.label = "st_vfstables_remote";
    remote.base = "astest/st_vfstables_remote";
    MountFreshDirectory( "st_vfstables_remote_catalogue", "astest/st_vfstables_remote/catalogue" );
    remote.database.database = "astest/st_vfstables_remote/catalogue/db";
    RunServiceSettingsRemoteSuite( remote );
    UnmountDirectory( "st_vfstables_remote_catalogue" );
}

/*-------------------------------------------------------------------------*/

void
PerformServiceSettingsPostgreSqlTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ServiceSettingsPostgreSql" );

    CString connectionString;
    if ( !GetPostgreSqlConnectionString( connectionString ) )
    {
        GUCEF_LOG( LOGLEVEL_IMPORTANT, "ServiceSettingsPostgreSql: GUCEF_TEST_POSTGRESQL is not set, the PostgreSQL tests are skipped" );
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the run-time settings against a PostgreSQL test database" )
            GUCEF_TESTFW_SKIP( "GUCEF_TEST_POSTGRESQL is not set" );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }
    CString reachReason;
    if ( !IsPostgreSqlReachable( connectionString, reachReason ) )
    {
        GUCEF_TESTFW_TESTCASE( "PostgreSQL: the test database can be reached" )
            ASSERT_TRUE( reachReason.IsNULLOrEmpty() );
        GUCEF_TESTFW_TESTCASE_END;
        return;
    }
    AUTH::CMemoryCredentialStore credentials;
    const char* password = getenv( "PGPASSWORD" );
    const char* const labels[] = { "st_postgresql", "st_postgresql_remote" };
    for ( UInt32 run=0; run<2; ++run )
    {
        SSettingsFixture fixture;
        fixture.label = labels[ run ];
        fixture.base = CString( "astest/" ) + labels[ run ];
        const CString schema = MakeTestSchemaName();
        fixture.database.backendName = "postgresql";
        CValueList parsed;
        CString error;
        if ( DBPLUGIN::POSTGRESQL::CPostgreSqlSettings::ParseConnectionString( connectionString, parsed, error ) )
            fixture.database.database = parsed.GetValueAlways( CString( "dbname" ) ).AsString();
        fixture.database.backendOptions.SetAllowMultipleValues( false );
        fixture.database.backendOptions.Set( "connectionString", CVariant( connectionString ) );
        fixture.database.backendOptions.Set( "schema", CVariant( schema ) );
        fixture.database.backendOptions.Set( "createSchema", CVariant( "true" ) );
        fixture.database.backendOptions.Set( "applicationName", CVariant( "gucefASSETSTORE_TestApp" ) );
        fixture.database.backendOptions.Set( "connectTimeout", CVariant( "10" ) );
        if ( GUCEF_NULL != password && '\0' != password[ 0 ] )
        {
            AUTH::CSecretString secret( ( CString( password ) ) );
            credentials.Store( "gucefASSETSTORE.test", "catalogue", secret );
            fixture.credentials = &credentials;
        }
        if ( 0 == run )
            RunServiceSettingsSuite( fixture );
        else
            RunServiceSettingsRemoteSuite( fixture );

        // the schema of the run goes
        CPrimaryAssetStoreSettings settings;
        settings.database = fixture.database;
        if ( GUCEF_NULL != fixture.credentials )
        {
            settings.credentialProvider = "gucefASSETSTORE.test";
            settings.credentialAccount = "catalogue";
        }
        COMCORE::CDatabaseClientSettings databaseSettings;
        if ( settings.GetDatabaseSettingsWithCredentials( fixture.credentials, databaseSettings, error ) )
        {
            COMCORE::CIDatabaseClientPtr client = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( "postgresql" );
            if ( !client.IsNULL() && client->Open( databaseSettings ) )
            {
                DBPLUGIN::POSTGRESQL::CPostgreSqlConnectionPtr connection = static_cast< DBPLUGIN::POSTGRESQL::CPostgreSqlClient* >( client.GetPointerAlways() )->GetConnection();
                if ( !connection.IsNULL() )
                {
                    MT::CScopeMutex lock( connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
                    CString dropError;
                    connection->Execute( "DROP SCHEMA " + DBPLUGIN::POSTGRESQL::CPostgreSqlSql::QuoteIdentifier( schema ) + " CASCADE", dropError );
                }
                client->Close();
            }
        }
    }
}

/*-------------------------------------------------------------------------*/
