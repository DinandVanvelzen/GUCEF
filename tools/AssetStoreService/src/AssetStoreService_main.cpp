/*
 *  AssetStoreService: the asset store as a remote service
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

#include <sys/stat.h>
#include <stdio.h>

#include "gucef_essentials.h"
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
#include <termios.h>
#include <unistd.h>
#endif
#include "CDynamicBuffer.h"
#include "CFileAccess.h"
#include "CPluginControl.h"
#include "CConfigStore.h"
#include "CStdLogger.h"
#include "CObservingNotifier.h"
#include "CTimer.h"
#include "dvcppfileutils.h"
#include "dvcppstringutils.h"
#include "DVCPPOSWRAP.h"
#include "gucefCORE_CCoreGlobal.h"
#include "CGUCEFApplication.h"
#include "gucefCORE_CPlatformNativeConsoleLogger.h"
#include "gucefCORE_CStandardOutputWriter.h"
#include "gucefCORE_CServiceLifecycle.h"
#include "gucefCORE_CServiceStopSignals.h"
#include "gucefCORE_CServiceResourceWatch.h"
#include "gucefCORE_CServiceHealthRegistry.h"
#include "gucefCORE_CIServiceHealthCheck.h"
#include "CDStoreCodecRegistry.h"
#include "gucefCORE_CTEventHandlerFunctor.h"
#include "CLogManager.h"
#include "gucefVFS_CVfsGlobal.h"
#include "gucefVFS_CVFS.h"
#include "gucefCOM_CComGlobal.h"
#include "gucefCOM_CJsonRpcEndpoint.h"
#include "gucefCOM_CIServiceAdminHandler.h"
#include "gucefCOM_CServiceAdminEndpoint.h"
#include "gucefCOM_CServiceAdminClient.h"
#include "gucefWEB_CHTTPServer.h"
#include "gucefWEB_CServiceHealthHttpEndpoints.h"
#include "gucefAUTH_CFileCredentialStore.h"
#include "gucefAUTH_CMemoryCredentialStore.h"
#include "gucefAUTH_CAuthGlobal.h"
#include "gucefAUTH_CAuthManager.h"
#include "gucefAUTH_CKeyringSession.h"
#include "gucefAUTH_CKeyringSettings.h"
#include "gucefAUTH_CSecretString.h"
#include "gucefDRNUTILS_CRemoteServiceHost.h"
#include "gucefDRNUTILS_CRemoteServiceHostSettings.h"
#include "gucefDRNUTILS_CDevicePairingCrypto.h"
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#include "gucefASSETSTORE_CAssetStoreServiceConfig.h"
#include "gucefASSETSTORE_CAssetStoreServiceHost.h"
#include "gucefASSETSTORE_CRemoteAssetStoreServiceSettings.h"
#include "gucefASSETSTORE_CAssetLibrarySyncSpec.h"
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#include "gucefASSETSTORE_CAssetReprocessRequest.h"
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#include "AssetStoreConverterHook_CPipelineConverterHook.h"
#include "AssetStoreConverterHook_CConverterHookSettings.h"
#include "AssetStoreConverterHook_CPipelineOperationSettings.h"

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

namespace {

/** The exit codes of the program */
enum EExitCode
{
    EXIT_STOPPED = 0 ,              /**< stopped in order (after a drain), or the admin command succeeded */
    EXIT_BAD_OPTIONS = 2 ,          /**< the command line is wrong */
    EXIT_SETTINGS_REFUSED = 3 ,     /**< the bootstrap file is missing or a required plugin of it did not load, or the settings file cannot be read or is refused */
    EXIT_START_FAILED = 4 ,         /**< the service did not start: credentials, ports, the host, the admin endpoint */
    EXIT_ADMIN_FAILED = 5 ,         /**< admin mode: the service was not reached or the command failed */
    EXIT_SMOKE_FAILED = 6           /**< --smoke-test: the service did not become ready */
};

/** The options of the command line */
struct SOptions
{
    CORE::CString configFile;
    CORE::CString bootstrapFile;
    CORE::CString logFile;
    CORE::CString dataDir;
    bool help;
    bool smokeTest;
    bool admin;
    CORE::CString::StringVector adminWords;

    SOptions( void ) : configFile(), bootstrapFile(), logFile(), dataDir(), help( false ), smokeTest( false ), admin( false ), adminWords() {}
};

/** A file mode in the usual octal notation, for the log */
CORE::CString
OctalMode( unsigned int mode )
{GUCEF_TRACE;

    CORE::CString text;
    for ( int shift=12; shift>=0; shift-=3 )
        text += (char) ( '0' + ( ( mode >> shift ) & 7 ) );
    return text;
}

/*-------------------------------------------------------------------------*/

/**
 *  Makes sure a directory of the service exists and is for the service user alone: mode 0700. A directory that exists
 *  with a wider mode (a setgid directory of a volume, a mode of an older version) is corrected and the correction logged.
 */
bool
EnsureOwnerOnlyDirectory( const CORE::CString& directory, const char* purpose )
{GUCEF_TRACE;

    if ( !CORE::DirExists( directory ) && !CORE::CreateDirs( directory ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: The " << purpose << " directory " << directory << " cannot be created" );
        return false;
    }

    bool needsRestriction = true;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    struct stat info;
    if ( 0 == stat( directory.C_String(), &info ) )
    {
        const unsigned int mode = (unsigned int) ( info.st_mode & 07777 );
        needsRestriction = 0700 != mode;
        if ( needsRestriction && info.st_uid != geteuid() )
        {
            // A directory of another user (the root of a volume, its group is the one of the service) cannot be restricted
            // by the service, and 0700 would lock the service out: what the service makes below it is restricted instead
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreService: The " << purpose << " directory " << directory << " belongs to another user (uid "
                               << (UInt32) info.st_uid << ", mode " << OctalMode( mode ) << ") and is not restricted to the service user"
                               << ( 0 == strcmp( purpose, "data" ) ? ": its sub directories state and credentials are restricted" : "" ) );
            return true;
        }
        if ( needsRestriction )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreService: The " << purpose << " directory " << directory << " has mode " << OctalMode( mode ) << ", restricting it to the service user (0700)" );
        }
    }
    #endif

    if ( needsRestriction && !CORE::SetOwnerOnlyAccess( directory ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: The " << purpose << " directory " << directory << " cannot be restricted to the service user" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
ParseOptions( int argc, char** argv, SOptions& options )
{GUCEF_TRACE;

    for ( int i=1; i<argc; ++i )
    {
        const CORE::CString argument = argv[ i ];
        if ( options.admin )
            options.adminWords.push_back( argument );
        else
        if ( "--help" == argument || "-h" == argument )
            options.help = true;
        else
        if ( "--config" == argument && i + 1 < argc )
            options.configFile = argv[ ++i ];
        else
        if ( "--bootstrap" == argument && i + 1 < argc )
            options.bootstrapFile = argv[ ++i ];
        else
        if ( "--log-file" == argument && i + 1 < argc )
            options.logFile = argv[ ++i ];
        else
        if ( "--data-dir" == argument && i + 1 < argc )
            options.dataDir = argv[ ++i ];
        else
        if ( "--smoke-test" == argument )
            options.smokeTest = true;
        else
        if ( "--console" == argument )
            continue;
        else
        if ( "--admin" == argument )
            options.admin = true;
        else
            return false;
    }
    if ( options.configFile.IsNULLOrEmpty() )
        options.configFile = CORE::GetEnv( "GUCEF_ASSETSTORE_SERVICE_CONFIG" );
    if ( options.bootstrapFile.IsNULLOrEmpty() )
        options.bootstrapFile = CORE::GetEnv( "GUCEF_ASSETSTORE_SERVICE_BOOTSTRAP" );
    if ( options.admin && options.adminWords.empty() )
        return false;
    return options.help || !options.configFile.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

/** The bootstrap file of the program: the one given, else AssetStoreService_bootstrap.ini next to the program (in a debug
 *  build AssetStoreService_bootstrap_d.ini first). Empty when there is none */
CORE::CString
FindBootstrapFile( const CORE::CString& given )
{GUCEF_TRACE;

    if ( !given.IsNULLOrEmpty() )
    {
        const CORE::CString path = CORE::RelativePath( given );
        return CORE::FileExists( path ) ? path : CORE::CString::Empty;
    }
    #ifdef GUCEF_DEBUG_MODE
    static const char* names[] = { "AssetStoreService_bootstrap_d.ini", "AssetStoreService_bootstrap.ini" };
    #else
    static const char* names[] = { "AssetStoreService_bootstrap.ini" };
    #endif
    for ( size_t i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
    {
        const CORE::CString path = CORE::CombinePath( CORE::ModuleDir(), names[ i ] );
        if ( CORE::FileExists( path ) )
            return path;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

/** Loads the plugins of the bootstrap file through the config store, as every GUCEF application does: the groups in
 *  the order of the file (GroupPriority, PriorityInGroup), a plugin with "LoadFailAllowed"="false" that does not load
 *  fails the whole. The JSON codec of the settings comes from it */
bool
LoadBootstrap( const CORE::CString& given, CORE::CString& problem )
{GUCEF_TRACE;

    const CORE::CString bootstrapPath = FindBootstrapFile( given );
    if ( bootstrapPath.IsNULLOrEmpty() )
    {
        problem = given.IsNULLOrEmpty() ? CORE::CString( "there is no AssetStoreService_bootstrap.ini next to the program (" ) + CORE::ModuleDir() + "): no plugins can be loaded"
                                        : "the bootstrap file " + given + " does not exist";
        return false;
    }
    CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
    configStore.SetBootstrapConfigFile( bootstrapPath );
    if ( !configStore.LoadConfig() )
    {
        problem = "the bootstrap file " + bootstrapPath + " cannot be loaded, or a plugin it requires did not load (see the log)";
        return false;
    }
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
    {
        problem = "the bootstrap file " + bootstrapPath + " did not load a JSON codec (dstorepluginJSONPARSER): the settings cannot be read";
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreService: the plugins of " << bootstrapPath << " are loaded" );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Whether a plugin of this name (or its debug name) is in a group of the plugin control already */
bool
IsPluginKnown( const CORE::CString& pluginName )
{GUCEF_TRACE;

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    CORE::CString::StringSet groups;
    pluginControl.GetAvailablePluginGroups( groups );
    for ( CORE::CString::StringSet::const_iterator g = groups.begin(); g != groups.end(); ++g )
    {
        CORE::CString::StringSet modules;
        pluginControl.GetAvailablePluginsForGroup( (*g), modules );
        if ( modules.find( pluginName ) != modules.end() || modules.find( pluginName + "_d" ) != modules.end() )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** An extra plugin of the settings ("Plugins"): a path, or a name looked for in the plugins folder of the install layout
 *  (../plugins) and next to the program, the debug name first in a debug build */
bool
LoadExtraPlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginName.HasChar( '/' ) >= 0 || pluginName.HasChar( '\\' ) >= 0 )
        return pluginControl.AddPluginFromDir( pluginName, "AssetStoreService", true );
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString names[ 2 ] = { pluginName + "_d", pluginName };
    #else
    const CORE::CString names[ 2 ] = { pluginName, pluginName + "_d" };
    #endif
    static const char* folders[] = { "$MODULEDIR$/../plugins/", "$MODULEDIR$/" };
    for ( size_t f=0; f<sizeof( folders ) / sizeof( folders[ 0 ] ); ++f )
        for ( size_t n=0; n<2; ++n )
            if ( pluginControl.AddPluginFromDir( folders[ f ] + names[ n ], "AssetStoreService", true ) )
                return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** The settings file: a JSON object with the sections below, nothing else */
bool
ReadSettingsFile( const CORE::CString& path, CORE::CDataNode& settings, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    if ( !CORE::FileExists( path ) || !content.LoadContentFromFile( path, 0, 4194304 ) )
    {
        problem = "the settings file " + path + " cannot be read";
        return false;
    }
    CORE::CString text;
    text.Set( (const char*) content.GetConstBufferPtr(), content.GetDataSize() );
    if ( !COM::CJsonRpcEndpoint::ParseJsonText( text, settings ) || GUCEF_DATATYPE_OBJECT != settings.GetNodeType() )
    {
        problem = "the settings file " + path + " is no JSON object";
        return false;
    }
    static const char* known[] = { "Plugins", "Service", "RemoteHost", "AssetStoreHost", "Database", "Storage", "Credentials", "Stores",
                                   "AssetStoreService", "ConverterHook", "PipelineOperations" };
    for ( CORE::CDataNode::const_iterator i = settings.ConstBegin(); i != settings.ConstEnd(); ++i )
    {
        bool isKnown = false;
        for ( UInt32 k=0; k<sizeof( known ) / sizeof( known[ 0 ] ); ++k )
            isKnown = isKnown || (*i)->GetName() == known[ k ];
        if ( !isKnown )
        {
            problem = "the settings file has the unknown section \"" + (*i)->GetName() + "\"";
            return false;
        }
    }
    if ( settings.GetAttCount() > 0 )
    {
        problem = "the settings file has members outside its sections";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Refuses members of a section that are not settings of it */
bool
CheckKeys( const CORE::CDataNode* section, const char* sectionName, const char* const* keys, UInt32 keyCount, CORE::CString& problem )
{GUCEF_TRACE;

    if ( GUCEF_NULL == section )
        return true;
    for ( CORE::CDataNode::TAttributeMap::const_iterator a = section->AttributeBegin(); a != section->AttributeEnd(); ++a )
    {
        bool known = false;
        for ( UInt32 k=0; k<keyCount && !known; ++k )
            known = (*a).first == keys[ k ];
        if ( !known )
        {
            problem = "the setting \"" + (*a).first + "\" of the section " + sectionName + " is not known";
            return false;
        }
    }
    for ( CORE::CDataNode::const_iterator c = section->ConstBegin(); c != section->ConstEnd(); ++c )
    {
        bool known = false;
        for ( UInt32 k=0; k<keyCount && !known; ++k )
            known = (*c)->GetName() == keys[ k ];
        if ( !known )
        {
            problem = "the setting \"" + (*c)->GetName() + "\" of the section " + sectionName + " is not known";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The section Service: the process */
struct SServiceSection
{
    CORE::CString displayName;
    CORE::CString dataDir;
    UInt32 drainSeconds;
    UInt32 expectedBackSeconds;
    UInt32 resourceSampleMs;
    UInt32 resourceLogSeconds;
    UInt32 memorySoftLimitMB;
    UInt16 healthPort;
    CORE::CString healthBindAddress;
    UInt16 adminPort;
    UInt32 catalogueRetrySeconds;

    SServiceSection( void )
        : displayName( "Asset store" ), dataDir( "$CURWORKDIR$/assetstore-data" ), drainSeconds( 20 ), expectedBackSeconds( 30 ), resourceSampleMs( 5000 ),
          resourceLogSeconds( 300 ), memorySoftLimitMB( 0 ), healthPort( 47480 ), healthBindAddress(), adminPort( 47403 ), catalogueRetrySeconds( 30 ) {}

    bool Load( const CORE::CDataNode* section, CORE::CString& problem )
    {
        static const char* keys[] = { "displayName", "dataDir", "drainSeconds", "expectedBackSeconds", "resourceSampleMs", "resourceLogSeconds",
                                      "memorySoftLimitMB", "healthPort", "healthBindAddress", "adminPort", "catalogueRetrySeconds" };
        if ( !CheckKeys( section, "Service", keys, sizeof( keys ) / sizeof( keys[ 0 ] ), problem ) )
            return false;
        if ( GUCEF_NULL == section )
            return true;
        displayName = section->GetAttributeValueOrChildValueByName( "displayName" ).AsString( displayName, true );
        dataDir = section->GetAttributeValueOrChildValueByName( "dataDir" ).AsString( dataDir, true );
        drainSeconds = section->GetAttributeValueOrChildValueByName( "drainSeconds" ).AsUInt32( drainSeconds, true );
        expectedBackSeconds = section->GetAttributeValueOrChildValueByName( "expectedBackSeconds" ).AsUInt32( expectedBackSeconds, true );
        resourceSampleMs = section->GetAttributeValueOrChildValueByName( "resourceSampleMs" ).AsUInt32( resourceSampleMs, true );
        resourceLogSeconds = section->GetAttributeValueOrChildValueByName( "resourceLogSeconds" ).AsUInt32( resourceLogSeconds, true );
        memorySoftLimitMB = section->GetAttributeValueOrChildValueByName( "memorySoftLimitMB" ).AsUInt32( memorySoftLimitMB, true );
        healthPort = (UInt16) section->GetAttributeValueOrChildValueByName( "healthPort" ).AsUInt32( healthPort, true );
        healthBindAddress = section->GetAttributeValueOrChildValueByName( "healthBindAddress" ).AsString( healthBindAddress, true );
        adminPort = (UInt16) section->GetAttributeValueOrChildValueByName( "adminPort" ).AsUInt32( adminPort, true );
        catalogueRetrySeconds = section->GetAttributeValueOrChildValueByName( "catalogueRetrySeconds" ).AsUInt32( catalogueRetrySeconds, true );
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The section Database: the catalogue. The password is never in the settings: a file that holds it (passwordFile) or
 *  the name of an environment variable (passwordEnv); it goes to the primary store through a credential store in memory
 */
bool
ApplyDatabaseSection( const CORE::CDataNode* section, ASSETSTORE::CPrimaryAssetStoreSettings& store, AUTH::CMemoryCredentialStore& passwords, CORE::CString& problem )
{GUCEF_TRACE;

    static const char* keys[] = { "type", "host", "port", "dbname", "user", "sslMode", "schema", "createSchema", "connectTimeout", "applicationName",
                                  "maxRowsPerStatement", "passwordFile", "passwordEnv", "directory" };
    if ( GUCEF_NULL != section && ( GUCEF_NULL != section->GetAttribute( "password" ) || GUCEF_NULL != section->FindChild( "password" ) ) )
    {
        problem = "the database password is never written in the settings: name a file (passwordFile) or an environment variable (passwordEnv)";
        return false;
    }
    if ( !CheckKeys( section, "Database", keys, sizeof( keys ) / sizeof( keys[ 0 ] ), problem ) )
        return false;
    if ( GUCEF_NULL == section )
        return true;
    const CORE::CString type = section->GetAttributeValueOrChildValueByName( "type" ).AsString( "postgresql", true );
    COMCORE::CDatabaseClientSettings& database = store.database;
    database.backendOptions.SetAllowMultipleValues( false );
    if ( "vfstables" == type )
    {
        // tables in a folder of the file system (tests, a small library): mounted writeable at assetstore-catalogue
        const CORE::CString directory = CORE::RelativePath( section->GetAttributeValueOrChildValueByName( "directory" ).AsString() );
        if ( directory.IsNULLOrEmpty() )
        {
            problem = "a vfstables database needs its directory";
            return false;
        }
        CORE::CreateDirs( CORE::CombinePath( directory, "db" ) );
        VFS::CVfsGlobal::Instance()->GetVfs().AddRoot( directory, "AssetStoreServiceCatalogue", true, false, "assetstore-catalogue" );
        database.backendName = "vfstables";
        database.database = "assetstore-catalogue/db";
        return true;
    }
    if ( "postgresql" != type )
    {
        problem = "the database type \"" + type + "\" is not known (postgresql, vfstables)";
        return false;
    }
    database.backendName = "postgresql";
    database.database = section->GetAttributeValueOrChildValueByName( "dbname" ).AsString( "gucef_assets", true );
    static const char* options[] = { "host", "port", "user", "sslMode", "schema", "createSchema", "connectTimeout", "maxRowsPerStatement" };
    for ( UInt32 i=0; i<sizeof( options ) / sizeof( options[ 0 ] ); ++i )
    {
        const CORE::CString value = section->GetAttributeValueOrChildValueByName( options[ i ] ).AsString();
        if ( !value.IsNULLOrEmpty() )
            database.backendOptions.Set( options[ i ], CORE::CVariant( value ) );
    }
    database.backendOptions.Set( "applicationName", CORE::CVariant( section->GetAttributeValueOrChildValueByName( "applicationName" ).AsString( "AssetStoreService", true ) ) );
    const CORE::CString passwordFile = section->GetAttributeValueOrChildValueByName( "passwordFile" ).AsString();
    const CORE::CString passwordEnv = section->GetAttributeValueOrChildValueByName( "passwordEnv" ).AsString();
    if ( !passwordFile.IsNULLOrEmpty() )
    {
        store.databasePasswordFile = passwordFile;
    }
    else
    if ( !passwordEnv.IsNULLOrEmpty() )
    {
        AUTH::CSecretString password( CORE::GetEnv( passwordEnv ) );
        if ( password.IsEmpty() )
        {
            problem = "the environment variable " + passwordEnv + " named for the database password is not set";
            return false;
        }
        passwords.Store( "AssetStoreService", "catalogue", password );
        password.Clear();
        store.credentialProvider = "AssetStoreService";
        store.credentialAccount = "catalogue";
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The section Storage: the storage locations (a folder of the file system each, mounted at its VFS root by the store) */
bool
ApplyStorageSection( const CORE::CDataNode* section, ASSETSTORE::CPrimaryAssetStoreSettings& store, CORE::CString& problem )
{GUCEF_TRACE;

    static const char* keys[] = { "locations", "defaultLocationId", "scratchStagingPath" };
    if ( !CheckKeys( section, "Storage", keys, sizeof( keys ) / sizeof( keys[ 0 ] ), problem ) )
        return false;
    if ( GUCEF_NULL == section )
        return true;
    const CORE::CDataNode* locations = section->FindChild( "locations" );
    if ( GUCEF_NULL != locations )
    {
        store.locations.clear();
        for ( CORE::CDataNode::const_iterator i = locations->ConstBegin(); i != locations->ConstEnd(); ++i )
        {
            ASSETSTORE::CAssetStorageLocation location;
            location.locationId = (*i)->GetAttributeValueOrChildValueByName( "id" ).AsString();
            location.name = (*i)->GetAttributeValueOrChildValueByName( "name" ).AsString( location.locationId, true );
            location.fileSystemPath = (*i)->GetAttributeValueOrChildValueByName( "path" ).AsString();
            location.vfsRoot = (*i)->GetAttributeValueOrChildValueByName( "vfsRoot" ).AsString( "assetstore/" + location.locationId, true );
            location.writeable = (*i)->GetAttributeValueOrChildValueByName( "writeable" ).AsBool( true, true );
            if ( location.locationId.IsNULLOrEmpty() || location.fileSystemPath.IsNULLOrEmpty() )
            {
                problem = "a storage location needs an id and a path";
                return false;
            }
            CORE::CreateDirs( location.fileSystemPath );
            store.locations.push_back( location );
        }
    }
    store.scratchStagingPath = section->GetAttributeValueOrChildValueByName( "scratchStagingPath" ).AsString( store.scratchStagingPath, true );
    store.defaultLocationId = section->GetAttributeValueOrChildValueByName( "defaultLocationId" ).AsString( store.locations.empty() ? CORE::CString::Empty : store.locations[ 0 ].locationId, true );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Readiness: the catalogue of the service can be used */
class CCatalogueCheck : public CORE::CIServiceHealthCheck
{
    public:

    explicit CCatalogueCheck( ASSETSTORE::CIAssetStoreService& service ) : CORE::CIServiceHealthCheck(), m_service( service ) {}

    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "catalogue";
        return name;
    }

    virtual CORE::TServiceHealthState Check( CORE::CString& detail ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_service.IsCatalogueAvailable() )
            return CORE::HEALTH_PASS;
        detail = "the catalogue database is not available";
        return CORE::HEALTH_FAIL;
    }

    private:

    ASSETSTORE::CIAssetStoreService& m_service;
};

/*-------------------------------------------------------------------------*/

/** Readiness: every storage location can be written (a probe file, checked at most every 30 s) */
class CStorageCheck : public CORE::CIServiceHealthCheck
{
    public:

    explicit CStorageCheck( const ASSETSTORE::CPrimaryAssetStoreSettings::TLocationVector& locations )
        : CORE::CIServiceHealthCheck(), m_locations( locations ), m_lastCheckMs( 0 ), m_lastState( CORE::HEALTH_FAIL ), m_lastDetail() {}

    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "storage";
        return name;
    }

    virtual CORE::TServiceHealthState Check( CORE::CString& detail ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const UInt64 now = DRNUTILS::CRemoteServiceHost::NowMs();
        if ( 0 != m_lastCheckMs && now - m_lastCheckMs < 30000 )
        {
            detail = m_lastDetail;
            return m_lastState;
        }
        m_lastCheckMs = now;
        m_lastState = CORE::HEALTH_PASS;
        m_lastDetail.Clear();
        for ( size_t i=0; i<m_locations.size(); ++i )
        {
            if ( m_locations[ i ].fileSystemPath.IsNULLOrEmpty() || !m_locations[ i ].writeable )
                continue;
            const CORE::CString probe = CORE::CombinePath( CORE::RelativePath( m_locations[ i ].fileSystemPath ), ".assetstore-ready-probe" );
            CORE::CDynamicBuffer content;
            content.Append( "ready", 5 );
            if ( !content.WriteContentToFile( probe, 0, true ) || !CORE::DeleteFile( probe ) )
            {
                m_lastState = CORE::HEALTH_FAIL;
                m_lastDetail = "the storage location " + m_locations[ i ].locationId + " cannot be written";
                break;
            }
        }
        detail = m_lastDetail;
        return m_lastState;
    }

    private:

    ASSETSTORE::CPrimaryAssetStoreSettings::TLocationVector m_locations;
    UInt64 m_lastCheckMs;
    CORE::TServiceHealthState m_lastState;
    CORE::CString m_lastDetail;
};

/*-------------------------------------------------------------------------*/

/** Readiness: the service serves (READY or DEGRADED, not draining) and the session listener runs */
class CServingCheck : public CORE::CIServiceHealthCheck
{
    public:

    CServingCheck( CORE::CServiceLifecycle& lifecycle, DRNUTILS::CRemoteServiceHost& host ) : CORE::CIServiceHealthCheck(), m_lifecycle( lifecycle ), m_host( host ) {}

    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "serving";
        return name;
    }

    virtual CORE::TServiceHealthState Check( CORE::CString& detail ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_lifecycle.IsServing() && m_host.IsRunning() && !m_host.IsDraining() )
            return CORE::HEALTH_PASS;
        detail = CORE::CString( "the service is " ) + CORE::ServiceLifecycleStateToString( m_lifecycle.GetState() );
        return CORE::HEALTH_FAIL;
    }

    private:

    CORE::CServiceLifecycle& m_lifecycle;
    DRNUTILS::CRemoteServiceHost& m_host;
};

/*-------------------------------------------------------------------------*/

/** Liveness: the thread of the DRN node of the host pulsed within 30 s */
class CNodeLivenessCheck : public CORE::CIServiceHealthCheck
{
    public:

    explicit CNodeLivenessCheck( DRNUTILS::CRemoteServiceHost& host ) : CORE::CIServiceHealthCheck(), m_host( host ) {}

    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "drn-node";
        return name;
    }

    virtual CORE::TServiceHealthState Check( CORE::CString& detail ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !m_host.IsRunning() || m_host.GetNodeHeartbeatAgeMs() < 30000 )
            return CORE::HEALTH_PASS;
        detail = "the thread of the DRN node stopped pulsing";
        return CORE::HEALTH_FAIL;
    }

    private:

    DRNUTILS::CRemoteServiceHost& m_host;
};

/*-------------------------------------------------------------------------*/

/** Values for /info and the status command: no names of devices, no secrets */
void
FillInfo( CORE::CServiceLifecycle& lifecycle, DRNUTILS::CRemoteServiceHost& host, ASSETSTORE::CAssetStoreServiceHost& binding,
          ASSETSTORE::CIAssetStoreService& service, CORE::CServiceResourceWatch& watch, CORE::CValueList& info )
{GUCEF_TRACE;

    info.SetAllowMultipleValues( false );
    info.Set( "service", CORE::CVariant( "AssetStoreService" ) );
    info.Set( "state", CORE::CVariant( CORE::ServiceLifecycleStateToString( lifecycle.GetState() ) ) );
    info.Set( "uptimeMs", CORE::CVariant( lifecycle.GetUptimeInMs() ) );
    info.Set( "catalogueAvailable", CORE::CVariant( service.IsCatalogueAvailable() ) );
    const CORE::CString::StringMap conditions = lifecycle.GetConditions();
    CORE::CString conditionList;
    for ( CORE::CString::StringMap::const_iterator i = conditions.begin(); i != conditions.end(); ++i )
        conditionList += ( conditionList.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( "," ) ) + (*i).first;
    info.Set( "conditions", CORE::CVariant( conditionList ) );
    host.GetInfo( info );
    binding.GetInfo( info );
    watch.GetInfo( info );
}

/*-------------------------------------------------------------------------*/

void
ValueListToNode( const CORE::CValueList& values, CORE::CDataNode& node )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<values.GetKeyCount(); ++i )
        node.SetAttribute( values.GetKey( i ).AsString(), values.GetValue( i ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  The process: the stop signal starts the drain (SERVICE_STOPPING to the clients, the listeners refuse new work), the
 *  end of the drain stops the main loop; the state follows the catalogue (READY, DEGRADED while the database is away, with
 *  a reconnect every catalogueRetrySeconds); the resource line; /info
 */
class CServiceDriver : public CORE::CObservingNotifier
{
    public:

    CServiceDriver( const SServiceSection& settings, CORE::CServiceLifecycle& lifecycle, DRNUTILS::CRemoteServiceHost& host,
                    ASSETSTORE::CAssetStoreServiceHost& binding, ASSETSTORE::CIAssetStoreService& service, CORE::CServiceResourceWatch& watch,
                    CORE::CServiceHealthRegistry& registry, bool smokeTest )
        : CORE::CObservingNotifier(), m_settings( settings ), m_lifecycle( lifecycle ), m_host( host ), m_binding( binding ), m_service( service ),
          m_watch( watch ), m_registry( registry ), m_signals( CORE::CCoreGlobal::Instance()->GetPulseGenerator() ),
          m_timer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), 250 ), m_smokeTest( smokeTest ), m_draining( false ), m_reachedReady( false ),
          m_lastRetryMs( 0 ), m_lastInfoMs( 0 )
    {
        typedef CORE::CTEventHandlerFunctor< CServiceDriver > TEventCallback;
        TEventCallback onStop( this, &CServiceDriver::OnStopRequested );
        SubscribeTo( &m_signals, CORE::CServiceStopSignals::StopRequestedEvent, onStop );
        TEventCallback onTimer( this, &CServiceDriver::OnTimer );
        SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, onTimer );
        m_timer.SetEnabled( true );
    }

    virtual ~CServiceDriver()
    {
        m_timer.SetEnabled( false );
        UnsubscribeAllFromObserver();
    }

    /** Starts the drain: the main loop stops once it is complete or its time is over */
    void RequestStop( const CORE::CString& reason )
    {GUCEF_TRACE;

        if ( m_draining )
            return;
        m_draining = true;
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: draining (" << reason << "), at most " << m_settings.drainSeconds << " s" );
        m_lifecycle.BeginDrain( reason, m_settings.drainSeconds * 1000 );
        m_host.BeginDrain( reason, m_settings.expectedBackSeconds );
        m_registry.EvaluateChecks();
    }

    bool IsDraining( void ) const { return m_draining; }
    bool ReachedReady( void ) const { return m_reachedReady; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "AssetStoreService::CServiceDriver";
        return name;
    }

    private:

    void OnStopRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        RequestStop( "stop signal" );
    }

    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        const UInt64 now = DRNUTILS::CRemoteServiceHost::NowMs();
        m_watch.Tick( now );
        if ( m_draining )
        {
            if ( m_host.IsDrainComplete() || m_lifecycle.IsDrainExpired() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: the drain is " << ( m_host.IsDrainComplete() ? "complete" : "over its time" ) << ", stopping" );
                CORE::CCoreGlobal::Instance()->GetApplication().Stop();
            }
            return;
        }

        // the state follows the catalogue; a database that went away is tried again now and then
        if ( m_service.IsCatalogueAvailable() )
        {
            if ( m_lifecycle.ClearCondition( "DATABASE_UNAVAILABLE" ) || CORE::SERVICELIFECYCLE_DEGRADED == m_lifecycle.GetState() )
                m_lifecycle.Transition( CORE::SERVICELIFECYCLE_READY, "the catalogue is available" );
            if ( !m_reachedReady )
                m_reachedReady = CORE::SERVICELIFECYCLE_READY == m_lifecycle.GetState();
        }
        else
        {
            m_lifecycle.SetCondition( "DATABASE_UNAVAILABLE", m_service.GetStatusMessage() );
            if ( CORE::SERVICELIFECYCLE_DEGRADED != m_lifecycle.GetState() )
                m_lifecycle.Transition( CORE::SERVICELIFECYCLE_DEGRADED, "the catalogue is not available" );
            if ( 0 != m_settings.catalogueRetrySeconds && now - m_lastRetryMs >= (UInt64) m_settings.catalogueRetrySeconds * 1000 )
            {
                m_lastRetryMs = now;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreService: opening the catalogue again" );
                m_service.Close();
                m_service.Open();
            }
        }
        if ( now - m_lastInfoMs >= 5000 )
        {
            m_lastInfoMs = now;
            CORE::CValueList info;
            FillInfo( m_lifecycle, m_host, m_binding, m_service, m_watch, info );
            m_registry.SetInfo( info );
        }
        if ( m_smokeTest && m_reachedReady )
            RequestStop( "smoke test" );
    }

    const SServiceSection& m_settings;
    CORE::CServiceLifecycle& m_lifecycle;
    DRNUTILS::CRemoteServiceHost& m_host;
    ASSETSTORE::CAssetStoreServiceHost& m_binding;
    ASSETSTORE::CIAssetStoreService& m_service;
    CORE::CServiceResourceWatch& m_watch;
    CORE::CServiceHealthRegistry& m_registry;
    CORE::CServiceStopSignals m_signals;
    CORE::CTimer m_timer;
    bool m_smokeTest;
    bool m_draining;
    bool m_reachedReady;
    UInt64 m_lastRetryMs;
    UInt64 m_lastInfoMs;
};

/*-------------------------------------------------------------------------*/

/**
 *  The commands of the loopback admin endpoint (owner-only token file): status, pair-open, pair-close, pairings,
 *  pair-confirm, devices, sessions, revoke, keyring-status, keyring-setup, keyring-unlock, keyring-lock, drain. The code of the
 *  keyring commands comes from the admin client (standard input or a file) and is neither logged nor answered. The pairing code goes to the admin caller only, in the answer of
 *  pair-open; it is never logged
 */
class CAdminHandler : public COM::CIServiceAdminHandler
{
    public:

    CAdminHandler( CORE::CServiceLifecycle& lifecycle, DRNUTILS::CRemoteServiceHost& host, ASSETSTORE::CAssetStoreServiceHost& binding,
                   ASSETSTORE::CIAssetStoreService& service, CORE::CServiceResourceWatch& watch )
        : COM::CIServiceAdminHandler(), m_lifecycle( lifecycle ), m_host( host ), m_binding( binding ), m_service( service ), m_watch( watch ), m_driver( GUCEF_NULL ), m_keyring( GUCEF_NULL ),
          m_local( GUCEF_NULL ) {}

    /** The service of this process (not owned): reprocess-status adds the budget of the reprocessing; GUCEF_NULL for none */
    void SetLocalService( ASSETSTORE::CLocalAssetStoreService* local ) { m_local = local; }

    void SetDriver( CServiceDriver* driver ) { m_driver = driver; }

    /** The keyring of the saved store passwords, for the first code (not owned); GUCEF_NULL for none */
    void SetKeyring( AUTH::CKeyringSession* keyring ) { m_keyring = keyring; }

    virtual COM::TServiceAdminStatus HandleAdminCommand( const CORE::CString& command, const TArgumentMap& arguments, CORE::CDataNode& result,
                                                        CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        result.SetName( "result" );
        result.SetNodeType( GUCEF_DATATYPE_OBJECT );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreService: admin command \"" << command << "\"" );
        if ( "status" == command )
        {
            CORE::CValueList info;
            FillInfo( m_lifecycle, m_host, m_binding, m_service, m_watch, info );
            ValueListToNode( info, result );
            result.SetAttribute( "keyring", GetKeyringState() );
            return COM::SERVICEADMIN_OK;
        }
        if ( "keyring-status" == command )
        {
            result.SetAttribute( "keyring", GetKeyringState() );
            return COM::SERVICEADMIN_OK;
        }
        if ( "keyring-unlock" == command || "keyring-setup" == command )
        {
            // the code comes from the standard input or a file of the admin client, never from its arguments; it is not logged and not answered
            const CORE::CString code = Argument( arguments, "code", "" );
            if ( code.IsNULLOrEmpty() )
            {
                message = command + " needs the code (the admin client reads it from the standard input or --code-file)";
                return COM::SERVICEADMIN_INVALID_ARGUMENT;
            }
            ASSETSTORE::CAssetServiceResult outcome;
            ASSETSTORE::TAssetStoreStatus status = ASSETSTORE::ASSETSTORE_STATUS_OK;
            if ( "keyring-setup" == command )
            {
                if ( GUCEF_NULL == m_keyring )
                {
                    message = "this service has no keyring";
                    return COM::SERVICEADMIN_FAILED;
                }
                if ( m_keyring->IsSetUp() )
                {
                    message = "the keyring has a code already: unlock it with keyring-unlock";
                    return COM::SERVICEADMIN_FAILED;
                }
                const AUTH::TKeyringResult setUp = m_keyring->SetUpCode( AUTH::CSecretString( code ) );
                if ( AUTH::KEYRING_OK != setUp )
                {
                    message = "the code of the keyring could not be set up (empty, too long, or the keyring codecs are not there)";
                    return COM::SERVICEADMIN_FAILED;
                }
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreService: the keyring code was set up and the keyring is unlocked" );
            }
            else
                status = m_service.UnlockKeyring( CORE::CString::Empty, AUTH::CSecretString( code ), outcome );
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
            {
                message = outcome.message;
                return ASSETSTORE::ASSETSTORE_STATUS_INVALID_ARGUMENT == status ? COM::SERVICEADMIN_INVALID_ARGUMENT : COM::SERVICEADMIN_FAILED;
            }
            result.SetAttribute( "keyring", GetKeyringState() );
            return COM::SERVICEADMIN_OK;
        }
        if ( "keyring-lock" == command )
        {
            ASSETSTORE::CAssetServiceResult outcome;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.LockKeyring( CORE::CString::Empty, outcome ) )
            {
                message = outcome.message;
                return COM::SERVICEADMIN_FAILED;
            }
            result.SetAttribute( "keyring", GetKeyringState() );
            return COM::SERVICEADMIN_OK;
        }
        if ( "pair-open" == command )
        {
            DRNUTILS::CPairingWindowRequest request;
            request.deviceKind = Argument( arguments, "kind", "viewer" );
            const CORE::CString::StringVector roles = Argument( arguments, "roles", "owner" ).ParseElements( ',', false );
            for ( size_t i=0; i<roles.size(); ++i )
                request.roles.insert( roles[ i ].Trim( true ) );
            request.seconds = CORE::StringToUInt32( Argument( arguments, "seconds", "0" ) );
            request.count = CORE::StringToUInt32( Argument( arguments, "count", "1" ) );
            CORE::CString windowId;
            CORE::CString code;
            UInt64 expiresUtcMs = 0;
            if ( !m_host.OpenPairingWindow( request, DRNUTILS::CRemoteServiceHost::LocalAdmin, windowId, code, expiresUtcMs, message ) )
                return COM::SERVICEADMIN_FAILED;
            // the code is shown once, to the admin caller: it is in this answer and nowhere else
            result.SetAttribute( "windowId", windowId );
            result.SetAttribute( "code", code );
            code.Clear();
            result.SetAttribute( "expiresUtcMs", expiresUtcMs );
            result.SetAttribute( "serviceName", m_host.GetSettings().serviceName );
            result.SetAttribute( "drnSessionName", m_host.GetSettings().drnSessionName );
            result.SetAttribute( "keyFingerprint", DRNUTILS::CDevicePairingCrypto::Fingerprint( m_host.GetStaticPublicKey() ) );
            result.SetAttribute( "pairingEndpoints", JoinList( m_host.GetPairingEndpoints() ) );
            result.SetAttribute( "sessionEndpoints", JoinList( m_host.GetSessionEndpoints() ) );
            return COM::SERVICEADMIN_OK;
        }
        if ( "pair-close" == command )
        {
            result.SetAttribute( "closed", m_host.ClosePairingWindow( "closed by the admin" ) );
            return COM::SERVICEADMIN_OK;
        }
        if ( "pairings" == command )
        {
            DRNUTILS::CDeviceRegistry::TDeviceRecordVector pending;
            m_host.GetPendingPairings( pending );
            CORE::CDataNode* list = result.AddChild( "pending", GUCEF_DATATYPE_ARRAY );
            for ( size_t i=0; i<pending.size(); ++i )
                pending[ i ].ToDataNode( *list->AddChild( "device", GUCEF_DATATYPE_OBJECT ) );
            return COM::SERVICEADMIN_OK;
        }
        if ( "pair-confirm" == command )
        {
            const bool approve = "false" != Argument( arguments, "approve", "true" );
            if ( !m_host.ConfirmPairing( Argument( arguments, "device", "" ), Argument( arguments, "fingerprint", "" ), approve, DRNUTILS::CRemoteServiceHost::LocalAdmin, message ) )
                return COM::SERVICEADMIN_FAILED;
            return COM::SERVICEADMIN_OK;
        }
        if ( "devices" == command )
        {
            m_host.GetDevicesDocument( result );
            return COM::SERVICEADMIN_OK;
        }
        if ( "sessions" == command )
        {
            m_host.GetSessionsDocument( result );
            return COM::SERVICEADMIN_OK;
        }
        if ( "revoke" == command )
        {
            const CORE::CString device = Argument( arguments, "device", "" );
            if ( device.IsNULLOrEmpty() )
            {
                message = "revoke needs --device <deviceId>";
                return COM::SERVICEADMIN_INVALID_ARGUMENT;
            }
            if ( !m_host.RevokeDevice( device, Argument( arguments, "reason", "revoked by the admin" ), DRNUTILS::CRemoteServiceHost::LocalAdmin, message ) )
                return COM::SERVICEADMIN_FAILED;
            result.SetAttribute( "revoked", device );
            return COM::SERVICEADMIN_OK;
        }
        if ( "drain" == command )
        {
            if ( GUCEF_NULL == m_driver )
            {
                message = "the service is not running yet";
                return COM::SERVICEADMIN_FAILED;
            }
            m_driver->RequestStop( "admin drain" );
            result.SetAttribute( "draining", true );
            return COM::SERVICEADMIN_OK;
        }
        if ( 0 == command.HasSubstr( "sync-" ) )
            return HandleSyncCommand( command, arguments, result, message );
        if ( IsReprocessCommand( command ) )
            return HandleReprocessCommand( command, arguments, result, message );
        if ( IsSettingsCommand( command ) )
            return HandleSettingsCommand( command, arguments, result, message );
        message = "the command \"" + command + "\" is not known (status, pair-open, pair-close, pairings, pair-confirm, devices, sessions, revoke, keyring-status, keyring-setup, keyring-unlock, keyring-lock, drain, "
                  "sync-plan, sync-start, sync-status, sync-cancel, sync-retry, sync-pause, sync-resume, sync-marks, sync-ignore, sync-pin, sync-unmark, sync-profile, "
                  "reprocess, reprocess-plan, reprocess-status, reprocess-cancel, redownload, regen-intermediate, regen-target, reprocess-rollback, "
                  "settings-describe, settings-set, settings-reset)";
        return COM::SERVICEADMIN_INVALID_ARGUMENT;
    }

    /** Whether the command is one of the reprocessing of packs */
    static bool IsReprocessCommand( const CORE::CString& command )
    {
        return "reprocess" == command || "reprocess-plan" == command || "reprocess-status" == command || "reprocess-cancel" == command || "redownload" == command ||
               "regen-intermediate" == command || "regen-target" == command || "reprocess-rollback" == command;
    }

    /** Whether the command is one of the run-time settings */
    static bool IsSettingsCommand( const CORE::CString& command )
    {
        return "settings-describe" == command || "settings-set" == command || "settings-reset" == command;
    }

    private:

    /**
     *  The run-time settings of the service (section 18 of the remote service design): settings-describe answers every setting with its
     *  value, where it comes from and how a change applies; settings-set --<key> <value> ... (the words key=value on the command line)
     *  changes them, all or none; settings-reset --keys a,b puts them back to the site value. The caller is the site administrator: the
     *  changes are allowed whatever allowSettingsChanges says, and they are audited as "admin".
     */
    COM::TServiceAdminStatus HandleSettingsCommand( const CORE::CString& command, const TArgumentMap& arguments, CORE::CDataNode& result, CORE::CString& message )
    {GUCEF_TRACE;

        ASSETSTORE::CAssetSettingsRequest request;
        request.caller = "admin";
        request.callerName = "the admin command line";
        CORE::CDataNodeSerializableSettings serializerSettings;
        if ( "settings-describe" == command )
        {
            ASSETSTORE::CAssetSettingsDescription description;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.DescribeSettings( request, description ) )
            {
                message = description.message;
                return COM::SERVICEADMIN_FAILED;
            }
            description.Serialize( result, serializerSettings );
            return COM::SERVICEADMIN_OK;
        }
        ASSETSTORE::CAssetSettingsChangeResult outcome;
        ASSETSTORE::TAssetStoreStatus status = ASSETSTORE::ASSETSTORE_STATUS_OK;
        if ( "settings-set" == command )
        {
            for ( TArgumentMap::const_iterator a = arguments.begin(); a != arguments.end(); ++a )
                request.values[ (*a).first ] = (*a).second;
            if ( request.values.empty() )
            {
                message = "settings-set needs the settings: key=value ...";
                return COM::SERVICEADMIN_INVALID_ARGUMENT;
            }
            status = m_service.SetSettings( CORE::CString::Empty, request, outcome );
        }
        else
        {
            const CORE::CString::StringVector keys = Argument( arguments, "keys", "" ).ParseElements( ',', false );
            for ( size_t k=0; k<keys.size(); ++k )
                request.keys.push_back( keys[ k ].Trim( true ).Trim( false ) );
            if ( request.keys.empty() )
            {
                message = "settings-reset needs the keys: key ...";
                return COM::SERVICEADMIN_INVALID_ARGUMENT;
            }
            status = m_service.ResetSettings( CORE::CString::Empty, request, outcome );
        }
        outcome.Serialize( result, serializerSettings );
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK == status )
            return COM::SERVICEADMIN_OK;
        message = outcome.message;
        return ASSETSTORE::ASSETSTORE_STATUS_INVALID_ARGUMENT == status ? COM::SERVICEADMIN_INVALID_ARGUMENT : COM::SERVICEADMIN_FAILED;
    }

    /**
     *  The reprocessing of packs of the store (the pipeline jobs of CAssetPipelineOperations::IsReprocess(), section 17 of the
     *  remote service design): reprocess, redownload, regen-intermediate, regen-target and reprocess-rollback start a job per pack
     *  (--pack P, or --packs A,B,C for several; the jobs wait for a slot of the service), reprocess-plan answers the plan of each
     *  pack and starts nothing (as does --dry-run true on the others), reprocess-status and reprocess-cancel take --job J (or
     *  --jobs A,B). Options: --assets a,b (paths below the original folder, or globs: the asset form), --store-asset-ids a,b,
     *  --failed true, --statuses a,b, --kinds a,b, --targets ogre, --steps redownload,intermediate,target (reprocess),
     *  --no-cook true, --then-convert false, --no-repair true, --allow-large true, --accept-errors true. The caller is "admin" (the
     *  owner on the machine of the service): the limits of machine callers do not apply, the caps of the stores do.
     */
    COM::TServiceAdminStatus HandleReprocessCommand( const CORE::CString& command, const TArgumentMap& arguments, CORE::CDataNode& result, CORE::CString& message )
    {GUCEF_TRACE;

        typedef ASSETSTORE::CAssetPipelineOperations Op;
        CORE::CString::StringVector jobs = SplitArgument( arguments, "jobs" );
        const CORE::CString job = Argument( arguments, "job", "" );
        if ( !job.IsNULLOrEmpty() )
            jobs.push_back( job );
        if ( "reprocess-status" == command || "reprocess-cancel" == command )
        {
            if ( jobs.empty() )
            {
                message = command + " needs --job <jobId> or --jobs A,B";
                return COM::SERVICEADMIN_INVALID_ARGUMENT;
            }
            CORE::CDataNode* list = result.AddChild( "jobs", GUCEF_DATATYPE_ARRAY );
            bool allEnded = true;
            for ( size_t j=0; j<jobs.size(); ++j )
            {
                CORE::CDataNode* entry = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                entry->SetAttribute( "jobId", jobs[ j ] );
                if ( "reprocess-cancel" == command )
                {
                    ASSETSTORE::CAssetServiceResult cancelled;
                    entry->SetAttribute( "status", CORE::CString( ASSETSTORE::AssetStoreStatusToString( m_service.ControlJob( CORE::CString::Empty, jobs[ j ], ASSETSTORE::ASSETJOB_ACTION_CANCEL, cancelled ) ) ) );
                    if ( !cancelled.message.IsNULLOrEmpty() )
                        entry->SetAttribute( "message", cancelled.message );
                    continue;
                }
                ASSETSTORE::CAssetJobEntry state;
                if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.GetJob( jobs[ j ], state ) )
                {
                    entry->SetAttribute( "state", "unknown" );
                    continue;
                }
                entry->SetAttribute( "state", CORE::CString( ASSETSTORE::AssetJobStateToString( state.state ) ) );
                entry->SetAttribute( "stateText", state.stateText );
                entry->SetAttribute( "packId", state.packId );
                entry->SetAttribute( "operation", state.subKind );
                entry->SetAttribute( "message", state.message );
                allEnded = allEnded && state.IsEnded();
                if ( state.IsEnded() )
                {
                    ASSETSTORE::CAssetServiceDocument outcome;
                    if ( ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.GetJobResult( jobs[ j ], CORE::CString::Empty, CORE::CString::Empty, 0, outcome ) )
                    {
                        CORE::CDataNode* copied = entry->AddChild( outcome.document );
                        copied->SetName( "result" );
                    }
                }
            }
            result.SetAttribute( "allEnded", allEnded );
            // the jobs at a time, the threads per job, the memory budget and what the running and waiting jobs use of them
            if ( GUCEF_NULL != m_local && "reprocess-status" == command )
                m_local->DescribeReprocessing( *result.AddChild( "reprocessing", GUCEF_DATATYPE_OBJECT ) );
            return COM::SERVICEADMIN_OK;
        }

        CORE::CString operation = Op::PackReprocess;
        if ( "redownload" == command )
            operation = Op::PackRedownloadOriginal;
        else
        if ( "reprocess-rollback" == command )
            operation = Op::ReprocessRollback;
        else
        if ( "regen-intermediate" == command || "regen-target" == command )
        {
            const bool assets = !Argument( arguments, "assets", "" ).IsNULLOrEmpty() || !Argument( arguments, "store-asset-ids", "" ).IsNULLOrEmpty();
            if ( "regen-intermediate" == command )
                operation = assets ? Op::AssetRegenerateIntermediate : Op::PackRegenerateIntermediate;
            else
                operation = assets ? Op::AssetRegenerateTarget : Op::PackRegenerateTarget;
        }
        CORE::CString::StringVector packs = SplitArgument( arguments, "packs" );
        const CORE::CString pack = Argument( arguments, "pack", "" );
        if ( !pack.IsNULLOrEmpty() )
            packs.push_back( pack );
        if ( packs.empty() )
        {
            message = command + " needs --pack <packId> or --packs A,B";
            return COM::SERVICEADMIN_INVALID_ARGUMENT;
        }
        const bool plan = "reprocess-plan" == command || CORE::StringToBool( Argument( arguments, "dry-run", "false" ), false );
        CORE::CDataNode* list = result.AddChild( plan ? "plans" : "jobs", GUCEF_DATATYPE_ARRAY );
        CORE::CString::StringVector started;
        bool anyRefused = false;
        for ( size_t p=0; p<packs.size(); ++p )
        {
            ASSETSTORE::CAssetReprocessRequest spec;
            spec.operation = "reprocess-plan" == command ? CORE::CString( Op::PackReprocess ) : operation;
            spec.packId = packs[ p ];
            spec.assets = SplitArgument( arguments, "assets" );
            spec.storeAssetIds = SplitArgument( arguments, "store-asset-ids" );
            spec.steps = SplitArgument( arguments, "steps" );
            spec.targets = SplitArgument( arguments, "targets" );
            const CORE::CString::StringVector statuses = SplitArgument( arguments, "statuses" );
            spec.onlyStatuses.insert( statuses.begin(), statuses.end() );
            if ( CORE::StringToBool( Argument( arguments, "failed", "false" ), false ) )
            {
                spec.onlyStatuses.insert( "failed" );
                spec.onlyStatuses.insert( "skipped" );
            }
            const CORE::CString::StringVector kinds = SplitArgument( arguments, "kinds" );
            spec.kinds.insert( kinds.begin(), kinds.end() );
            spec.cook = !CORE::StringToBool( Argument( arguments, "no-cook", "false" ), false );
            spec.thenConvert = CORE::StringToBool( Argument( arguments, "then-convert", "true" ), true );
            spec.repairOriginals = !CORE::StringToBool( Argument( arguments, "no-repair", "false" ), false );
            spec.allowLarge = CORE::StringToBool( Argument( arguments, "allow-large", "false" ), false );
            spec.acceptErrors = CORE::StringToBool( Argument( arguments, "accept-errors", "false" ), false );
            spec.caller = "admin";
            spec.machine = false;
            CORE::CDataNode* entry = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            entry->SetAttribute( "packId", packs[ p ] );
            ASSETSTORE::CAssetPipelineRequest request;
            spec.ToRequest( request );
            if ( plan )
            {
                request.operation = Op::ReprocessPlan;
                request.arguments.SetAttribute( "operation", spec.operation );
                ASSETSTORE::CAssetServiceDocument planned;
                const ASSETSTORE::TAssetStoreStatus status = m_service.RunPipelineQuery( request, planned );
                entry->SetAttribute( "status", CORE::CString( ASSETSTORE::AssetStoreStatusToString( status ) ) );
                if ( ASSETSTORE::ASSETSTORE_STATUS_OK == status )
                    entry->AddChild( planned.document )->SetName( "plan" );
                else
                    entry->SetAttribute( "message", planned.message );
                continue;
            }
            ASSETSTORE::CAssetServiceResult outcome;
            const ASSETSTORE::TAssetStoreStatus status = m_service.StartPipelineJob( CORE::CString::Empty, request, outcome );
            entry->SetAttribute( "status", CORE::CString( ASSETSTORE::AssetStoreStatusToString( status ) ) );
            entry->SetAttribute( "operation", operation );
            entry->SetAttribute( "jobId", outcome.id );
            if ( !outcome.message.IsNULLOrEmpty() )
                entry->SetAttribute( "message", outcome.message );
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK == status )
                started.push_back( outcome.id );
            else
                anyRefused = true;
        }
        if ( !plan )
            result.SetAttribute( "jobIds", JoinList( started ) );
        if ( anyRefused && started.empty() )
        {
            message = "no job was started: see the status of each pack";
            result.SetAttribute( "message", message );
        }
        return COM::SERVICEADMIN_OK;
    }

    /** The values of a list argument ("a,b,c"), without blanks and empty ones */
    static CORE::CString::StringVector SplitArgument( const TArgumentMap& arguments, const char* name )
    {
        CORE::CString::StringVector values;
        const CORE::CString::StringVector parts = Argument( arguments, name, "" ).ParseElements( ',', false );
        for ( size_t i=0; i<parts.size(); ++i )
        {
            const CORE::CString value = parts[ i ].Trim( true ).Trim( false );
            if ( !value.IsNULLOrEmpty() )
                values.push_back( value );
        }
        return values;
    }

    /**
     *  The library sync (library sync design, section 7.5): sync-plan, sync-start, sync-status, sync-cancel, sync-retry, sync-pause,
     *  sync-resume with --store S (or --job J for the last four and sync-status); sync-start also takes --max-items N, --item-ids A,B,
     *  --retry-failed true and --replan true (list afresh instead of using the stored plan); sync-plan and sync-start take
     *  --refresh-details true (ask the details of the items again instead of taking them from the cache); sync-marks, sync-ignore, sync-pin, sync-unmark with
     *  --store S --item I [--reason R]; sync-profile --store S [--member value ...] shows the profile, or changes the members named.
     *  A plan and a run are the pipeline job "sync-library" with the profile the service keeps for the store; nothing here waits for a
     *  job (the admin client asks sync-status again for that)
     */
    COM::TServiceAdminStatus HandleSyncCommand( const CORE::CString& command, const TArgumentMap& arguments, CORE::CDataNode& result, CORE::CString& message )
    {GUCEF_TRACE;

        const CORE::CString storeId = Argument( arguments, "store", "" ).Lowercase();
        CORE::CString jobId = Argument( arguments, "job", "" );
        if ( "sync-plan" == command || "sync-start" == command )
        {
            if ( storeId.IsNULLOrEmpty() )
            {
                message = command + " needs --store <storeId>";
                return COM::SERVICEADMIN_INVALID_ARGUMENT;
            }
            ASSETSTORE::CAssetLibrarySyncSpec spec;
            spec.storeId = storeId;
            spec.mode = "sync-plan" == command ? ASSETSTORE::CAssetLibrarySyncSpec::ModePlan : ASSETSTORE::CAssetLibrarySyncSpec::ModeRun;
            spec.useStoreProfile = true;
            // a run of a few items first (--max-items 3), only some items (--item-ids a,b), the items that failed before now (--retry-failed true)
            spec.maxItems = CORE::StringToUInt32( Argument( arguments, "max-items", "0" ), 0 );
            const CORE::CString items = Argument( arguments, "item-ids", "" );
            if ( !items.IsNULLOrEmpty() )
                spec.itemIds = items.ParseElements( ',', false );
            spec.retryFailed = CORE::StringToBool( Argument( arguments, "retry-failed", "false" ), false );
            // a start lists afresh instead of using the stored plan of the store (--replan true); the details of the items are asked
            // of the store again instead of taken from the cache (--refresh-details true)
            spec.replan = CORE::StringToBool( Argument( arguments, "replan", "false" ), false );
            spec.refreshDetails = CORE::StringToBool( Argument( arguments, "refresh-details", "false" ), false );
            // the items named with --item-ids go through the size guard of the store plugin (--allow-large true)
            spec.allowLarge = CORE::StringToBool( Argument( arguments, "allow-large", "false" ), false );
            ASSETSTORE::CAssetPipelineRequest request;
            spec.ToRequest( request );
            ASSETSTORE::CAssetServiceResult started;
            const ASSETSTORE::TAssetStoreStatus status = m_service.StartPipelineJob( CORE::CString::Empty, request, started );
            result.SetAttribute( "storeId", storeId );
            result.SetAttribute( "jobId", started.id );
            result.SetAttribute( "status", CORE::CString( ASSETSTORE::AssetStoreStatusToString( status ) ) );
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status && ASSETSTORE::ASSETSTORE_STATUS_ALREADY_EXISTS != status )
            {
                message = started.message;
                return ASSETSTORE::ASSETSTORE_STATUS_NOT_FOUND == status || ASSETSTORE::ASSETSTORE_STATUS_INVALID_ARGUMENT == status ? COM::SERVICEADMIN_INVALID_ARGUMENT : COM::SERVICEADMIN_FAILED;
            }
            if ( !started.message.IsNULLOrEmpty() )
                result.SetAttribute( "message", started.message );
            return COM::SERVICEADMIN_OK;
        }
        if ( "sync-status" == command || "sync-cancel" == command || "sync-retry" == command || "sync-pause" == command || "sync-resume" == command )
        {
            if ( jobId.IsNULLOrEmpty() )
                jobId = FindLastSyncJob( storeId );
            if ( jobId.IsNULLOrEmpty() )
            {
                message = storeId.IsNULLOrEmpty() ? command + " needs --store <storeId> or --job <jobId>" : "there is no sync job of the store " + storeId;
                return storeId.IsNULLOrEmpty() ? COM::SERVICEADMIN_INVALID_ARGUMENT : COM::SERVICEADMIN_FAILED;
            }
            if ( "sync-status" != command )
            {
                ASSETSTORE::TAssetJobAction action = ASSETSTORE::ASSETJOB_ACTION_CANCEL;
                if ( "sync-retry" == command )
                    action = ASSETSTORE::ASSETJOB_ACTION_RETRY;
                else
                if ( "sync-pause" == command )
                    action = ASSETSTORE::ASSETJOB_ACTION_PAUSE;
                else
                if ( "sync-resume" == command )
                    action = ASSETSTORE::ASSETJOB_ACTION_RESUME;
                ASSETSTORE::CAssetServiceResult outcome;
                if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.ControlJob( CORE::CString::Empty, jobId, action, outcome ) )
                {
                    message = outcome.message.IsNULLOrEmpty() ? "the sync job " + jobId + " cannot be told to " + ASSETSTORE::AssetJobActionToString( action ) : outcome.message;
                    return COM::SERVICEADMIN_FAILED;
                }
                result.SetAttribute( "jobId", jobId );
                if ( !outcome.relatedId.IsNULLOrEmpty() )
                    result.SetAttribute( "newJobId", outcome.relatedId );
                return COM::SERVICEADMIN_OK;
            }
            // the job, the counts of its plan and of its run, and the first lines of the plan
            ASSETSTORE::CAssetJobEntry job;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.GetJob( jobId, job ) )
            {
                message = "there is no job " + jobId;
                return COM::SERVICEADMIN_FAILED;
            }
            CORE::CDataNodeSerializableSettings settings;
            job.Serialize( *result.AddChild( "job", GUCEF_DATATYPE_OBJECT ), settings );
            ASSETSTORE::CAssetServiceDocument plan;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.GetJobResult( jobId, "plan", CORE::CString::Empty, 0, plan ) )
            {
                for ( CORE::CDataNode::const_iterator c = plan.document.ConstBegin(); c != plan.document.ConstEnd(); ++c )
                    result.AddChild( *(*c) );
                result.SetAttribute( "summary", plan.document.GetAttributeValueOrChildValueByName( "summary" ).AsString() );
            }
            const UInt32 lines = CORE::StringToUInt32( Argument( arguments, "items", "20" ), 20 );
            ASSETSTORE::CAssetServiceDocument items;
            if ( lines > 0 && ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.GetJobResult( jobId, "items", CORE::CString::Empty, lines, items ) )
            {
                const CORE::CDataNode* itemNode = items.document.FindChild( "items" );
                if ( GUCEF_NULL != itemNode )
                    result.AddChild( *itemNode );
            }
            return COM::SERVICEADMIN_OK;
        }
        if ( "sync-marks" == command || "sync-ignore" == command || "sync-pin" == command || "sync-unmark" == command || "sync-unignore" == command )
        {
            if ( storeId.IsNULLOrEmpty() )
            {
                message = command + " needs --store <storeId>";
                return COM::SERVICEADMIN_INVALID_ARGUMENT;
            }
            if ( "sync-marks" != command )
            {
                const CORE::CString itemId = Argument( arguments, "item", "" );
                if ( itemId.IsNULLOrEmpty() )
                {
                    message = command + " needs --item <itemId>";
                    return COM::SERVICEADMIN_INVALID_ARGUMENT;
                }
                ASSETSTORE::CAssetSyncMarkPage marks;
                ASSETSTORE::CAssetSyncMark mark;
                mark.itemId = itemId;
                mark.mark = "sync-ignore" == command ? ASSETSTORE::CAssetSyncMark::Ignore : ( "sync-pin" == command ? ASSETSTORE::CAssetSyncMark::Pin : CORE::CString::Empty );
                mark.reason = Argument( arguments, "reason", "set by the admin" );
                marks.entries.push_back( mark );
                ASSETSTORE::CAssetServiceResult outcome;
                if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.SetSyncMarks( CORE::CString::Empty, storeId, marks, outcome ) )
                {
                    message = outcome.message;
                    return COM::SERVICEADMIN_FAILED;
                }
            }
            ASSETSTORE::CAssetSyncMarkPage page;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.GetSyncMarks( storeId, CORE::CString::Empty, ASSETSTORE::CIAssetStoreService::MaxPageEntries, page ) )
            {
                message = "the marks of the store " + storeId + " cannot be read";
                return COM::SERVICEADMIN_FAILED;
            }
            CORE::CDataNodeSerializableSettings settings;
            page.Serialize( result, settings );
            result.SetName( "result" );
            return COM::SERVICEADMIN_OK;
        }
        if ( "sync-profile" == command )
        {
            if ( storeId.IsNULLOrEmpty() )
            {
                message = command + " needs --store <storeId>";
                return COM::SERVICEADMIN_INVALID_ARGUMENT;
            }
            ASSETSTORE::CAssetLibrarySyncProfile profile;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.GetSyncProfile( storeId, profile ) )
            {
                message = "the sync profile of the store " + storeId + " cannot be read";
                return COM::SERVICEADMIN_FAILED;
            }
            // every other argument names a member of the profile and its new value
            CORE::CDataNodeSerializableSettings settings;
            CORE::CDataNode node( "profile", GUCEF_DATATYPE_OBJECT );
            profile.Serialize( node, settings );
            UInt32 changed = 0;
            for ( TArgumentMap::const_iterator a = arguments.begin(); a != arguments.end(); ++a )
            {
                if ( "store" == (*a).first )
                    continue;
                if ( !node.IsAttribute( (*a).first ) )
                {
                    message = "the sync profile has no member \"" + (*a).first + "\"";
                    return COM::SERVICEADMIN_INVALID_ARGUMENT;
                }
                node.SetAttribute( (*a).first, (*a).second );
                ++changed;
            }
            if ( changed > 0 )
            {
                profile.Deserialize( node, settings );
                ASSETSTORE::CAssetServiceResult outcome;
                if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.SetSyncProfile( CORE::CString::Empty, storeId, profile, outcome ) )
                {
                    message = outcome.message;
                    return COM::SERVICEADMIN_FAILED;
                }
            }
            profile.Serialize( result, settings );
            result.SetName( "result" );
            result.SetAttribute( "storeId", storeId );
            return COM::SERVICEADMIN_OK;
        }
        message = "the command \"" + command + "\" is not known (sync-plan, sync-start, sync-status, sync-cancel, sync-retry, sync-pause, sync-resume, "
                  "sync-marks, sync-ignore, sync-pin, sync-unmark, sync-profile)";
        return COM::SERVICEADMIN_INVALID_ARGUMENT;
    }

    /** The id of the sync job of the store that was made last: the one that runs, else the last that ended; empty when there is none */
    CORE::CString FindLastSyncJob( const CORE::CString& storeId )
    {GUCEF_TRACE;

        if ( storeId.IsNULLOrEmpty() )
            return CORE::CString::Empty;
        ASSETSTORE::CAssetJobPage page;
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.ListJobs( CORE::CString::Empty, ASSETSTORE::CIAssetStoreService::MaxPageEntries, page ) )
            return CORE::CString::Empty;
        CORE::CString found;
        UInt64 foundCreated = 0;
        bool foundRunning = false;
        for ( size_t i=0; i<page.entries.size(); ++i )
        {
            const ASSETSTORE::CAssetJobEntry& job = page.entries[ i ];
            if ( "sync" != job.subKind || storeId != job.providerId )
                continue;
            const bool running = !job.IsEnded();
            if ( found.IsNULLOrEmpty() || ( running && !foundRunning ) || ( running == foundRunning && job.createdAtMs >= foundCreated ) )
            {
                found = job.jobId;
                foundCreated = job.createdAtMs;
                foundRunning = running;
            }
        }
        return found;
    }

    CORE::CString GetKeyringState( void ) const
    {
        if ( GUCEF_NULL == m_keyring )
            return "none";
        if ( !m_keyring->IsSetUp() )
            return "notSetUp";
        return m_keyring->IsLocked() ? "locked" : "unlocked";
    }

    static CORE::CString JoinList( const CORE::CString::StringVector& values )
    {
        CORE::CString joined;
        for ( size_t i=0; i<values.size(); ++i )
            joined += ( 0 == i ? CORE::CString::Empty : CORE::CString( "," ) ) + values[ i ];
        return joined;
    }

    static CORE::CString Argument( const TArgumentMap& arguments, const char* name, const char* fallback )
    {
        TArgumentMap::const_iterator i = arguments.find( name );
        return i == arguments.end() ? CORE::CString( fallback ) : (*i).second;
    }

    CORE::CServiceLifecycle& m_lifecycle;
    DRNUTILS::CRemoteServiceHost& m_host;
    ASSETSTORE::CAssetStoreServiceHost& m_binding;
    ASSETSTORE::CIAssetStoreService& m_service;
    CORE::CServiceResourceWatch& m_watch;
    CServiceDriver* m_driver;
    AUTH::CKeyringSession* m_keyring;
    ASSETSTORE::CLocalAssetStoreService* m_local;
};

/*-------------------------------------------------------------------------*/

/**
 *  Reads a keyring code for an admin command: from a file (the first line) or from the standard input. On a terminal the input is not
 *  echoed. The code never comes from the command line, where it would be in the process list and the history of the shell.
 */
bool
ReadKeyringCode( const CORE::CString& codeFile, CORE::CString& code, CORE::CString& problem )
{GUCEF_TRACE;

    code.Clear();
    static const UInt32 maxCodeBytes = 1024;
    if ( !codeFile.IsNULLOrEmpty() )
    {
        CORE::CDynamicBuffer content;
        if ( !CORE::FileExists( codeFile ) || !content.LoadContentFromFile( codeFile, 0, maxCodeBytes + 16 ) )
        {
            problem = "the code file " + codeFile + " cannot be read";
            return false;
        }
        code.Set( (const char*) content.GetConstBufferPtr(), content.GetDataSize() );
    }
    else
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
        struct termios saved;
        const bool terminal = 0 != isatty( 0 ) && 0 == tcgetattr( 0, &saved );
        if ( terminal )
        {
            struct termios hidden = saved;
            hidden.c_lflag &= ~( (tcflag_t) ECHO );
            tcsetattr( 0, TCSANOW, &hidden );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreService: enter the keyring code (the input is not shown)" );
        }
        #endif
        char line[ maxCodeBytes + 16 ];
        const bool read = GUCEF_NULL != fgets( line, (int) sizeof( line ), stdin );
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
        if ( terminal )
            tcsetattr( 0, TCSANOW, &saved );
        #endif
        if ( !read )
        {
            problem = "no keyring code was given on the standard input";
            return false;
        }
        code = line;
        // the line is wiped as far as this program can
        for ( size_t i=0; i<sizeof( line ); ++i )
            line[ i ] = '\0';
    }
    // the code is the first line, without its line break
    const Int32 lineBreak = code.HasChar( '\n' );
    if ( lineBreak >= 0 )
        code = code.SubstrToIndex( (UInt32) lineBreak, true );
    if ( !code.IsNULLOrEmpty() && '\r' == code[ code.Length() - 1 ] )
        code = code.CutChars( 1, false );
    if ( code.IsNULLOrEmpty() || code.Length() > maxCodeBytes )
    {
        code.Clear();
        problem = "the keyring code is empty or longer than " + CORE::ToString( maxCodeBytes ) + " bytes";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The words of a library sync command in the short form: "sync plan|start|status|cancel|retry|pause|resume <store>",
 *  "sync-<verb> <store>", "sync ignore|pin|unmark <store> <item> [reason]" become "sync-<verb> --store S [--item I --reason R]";
 *  other words are left as they are
 */
CORE::CString::StringVector
NormaliseSyncWords( const CORE::CString::StringVector& words )
{GUCEF_TRACE;

    if ( words.empty() )
        return words;
    CORE::CString::StringVector rest = words;
    CORE::CString command = rest[ 0 ];
    rest.erase( rest.begin() );
    if ( "settings" == command && !rest.empty() && !rest[ 0 ].StartsWith( "--" ) )
    {
        command = "settings-" + rest[ 0 ];
        rest.erase( rest.begin() );
    }
    if ( CAdminHandler::IsSettingsCommand( command ) )
    {
        // "settings-set key=value ..." is "settings-set --key value ...", "settings-reset key ..." is "settings-reset --keys a,b"
        CORE::CString::StringVector normal;
        normal.push_back( command );
        CORE::CString keys;
        for ( size_t i=0; i<rest.size(); ++i )
        {
            const Int32 equals = rest[ i ].HasChar( '=' );
            if ( "settings-set" == command && !rest[ i ].StartsWith( "--" ) && equals > 0 )
            {
                normal.push_back( "--" + rest[ i ].SubstrToIndex( (UInt32) equals, true ) );
                normal.push_back( rest[ i ].SubstrToIndex( (UInt32) equals + 1, false ) );
            }
            else
            if ( "settings-reset" == command && !rest[ i ].StartsWith( "--" ) )
                keys += ( keys.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( "," ) ) + rest[ i ];
            else
                normal.push_back( rest[ i ] );
        }
        if ( !keys.IsNULLOrEmpty() )
        {
            normal.push_back( "--keys" );
            normal.push_back( keys );
        }
        return normal;
    }
    if ( "sync" == command && !rest.empty() && !rest[ 0 ].StartsWith( "--" ) )
    {
        command = "sync-" + rest[ 0 ];
        rest.erase( rest.begin() );
    }
    if ( 0 != command.HasSubstr( "sync-" ) )
    {
        // the reprocessing: "<command> <pack> [<pack> ...]" is "<command> --packs P1,P2"
        if ( !CAdminHandler::IsReprocessCommand( command ) || rest.empty() || rest[ 0 ].StartsWith( "--" ) )
            return words;
        CORE::CString::StringVector normal;
        normal.push_back( command );
        CORE::CString packs;
        size_t i = 0;
        for ( ; i<rest.size() && !rest[ i ].StartsWith( "--" ); ++i )
            packs += ( packs.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( "," ) ) + rest[ i ];
        normal.push_back( "reprocess-status" == command || "reprocess-cancel" == command ? CORE::CString( "--jobs" ) : CORE::CString( "--packs" ) );
        normal.push_back( packs );
        for ( ; i<rest.size(); ++i )
            normal.push_back( rest[ i ] );
        return normal;
    }
    CORE::CString::StringVector normal;
    normal.push_back( command );
    static const char* const positional[] = { "--store", "--item", "--reason" };
    UInt32 position = 0;
    for ( size_t i=0; i<rest.size(); ++i )
    {
        if ( rest[ i ].StartsWith( "--" ) )
        {
            // the named arguments and their values follow as they are
            for ( ; i<rest.size(); ++i )
                normal.push_back( rest[ i ] );
            break;
        }
        if ( position < 3 )
        {
            normal.push_back( positional[ position++ ] );
            normal.push_back( rest[ i ] );
        }
    }
    return normal;
}

/*-------------------------------------------------------------------------*/

/** Admin mode: one command to the admin endpoint of the running service; the answer goes to standard output as JSON */
int
RunAdminCommand( const SServiceSection& service, const SOptions& options )
{GUCEF_TRACE;

    CORE::CString command;
    COM::CIServiceAdminHandler::TArgumentMap arguments;
    CORE::CString error;
    if ( !COM::CServiceAdminClient::ParseCommandLine( NormaliseSyncWords( options.adminWords ), command, arguments, error ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: the admin command is not valid: " << error );
        return EXIT_BAD_OPTIONS;
    }
    if ( "keyring-unlock" == command || "keyring-setup" == command )
    {
        // the code is read here, from the standard input or the file named with --code-file, and goes to the service over the loopback
        // connection of the admin endpoint (the token file of the data folder); a code on the command line is refused
        if ( arguments.find( "code" ) != arguments.end() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: the keyring code is not taken from the command line: give it on the standard input or with --code-file" );
            return EXIT_BAD_OPTIONS;
        }
        CORE::CString codeFile;
        COM::CIServiceAdminHandler::TArgumentMap::iterator file = arguments.find( "code-file" );
        if ( file != arguments.end() )
        {
            codeFile = (*file).second;
            arguments.erase( file );
        }
        CORE::CString code;
        CORE::CString codeProblem;
        if ( !ReadKeyringCode( codeFile, code, codeProblem ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: " << codeProblem );
            return EXIT_BAD_OPTIONS;
        }
        arguments[ "code" ] = code;
        code.Clear();
    }
    const CORE::CString stateDir = CORE::CombinePath( CORE::RelativePath( service.dataDir ), "state" );
    CORE::CDataNode result;
    COMCORE::CRpcStatus status;
    // --follow true on a reprocessing command: the admin client asks reprocess-status again until the jobs ended, and answers that
    bool follow = false;
    COM::CIServiceAdminHandler::TArgumentMap::iterator followArgument = arguments.find( "follow" );
    if ( followArgument != arguments.end() )
    {
        follow = CORE::StringToBool( (*followArgument).second, true );
        arguments.erase( followArgument );
    }
    // sync-plan waits for the plan unless --no-wait is given: the admin client asks sync-status again until the job ended
    bool waitForPlan = "sync-plan" == command;
    COM::CIServiceAdminHandler::TArgumentMap::iterator noWait = arguments.find( "no-wait" );
    if ( noWait != arguments.end() )
    {
        waitForPlan = false;
        arguments.erase( noWait );
    }
    if ( !COM::CServiceAdminClient::RunWithTokenFile( service.adminPort, stateDir, command, arguments, result, status ) || !status.IsOk() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: the admin command \"" << command << "\" failed: " << status.ToString() );
        return EXIT_ADMIN_FAILED;
    }
    if ( follow && CAdminHandler::IsReprocessCommand( command ) && !result.GetAttributeValueOrChildValueByName( "jobIds" ).AsString().IsNULLOrEmpty() )
    {
        COM::CIServiceAdminHandler::TArgumentMap statusArguments;
        statusArguments[ "jobs" ] = result.GetAttributeValueOrChildValueByName( "jobIds" ).AsString();
        const UInt32 waitMs = 2000;
        const UInt32 maxWaits = 12 * 3600 / 2;     // twelve hours: the conversion of a large pack takes its time
        for ( UInt32 w=0; w<maxWaits; ++w )
        {
            CORE::CDataNode followed;
            if ( !COM::CServiceAdminClient::RunWithTokenFile( service.adminPort, stateDir, "reprocess-status", statusArguments, followed, status ) || !status.IsOk() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: the state of the jobs " << statusArguments[ "jobs" ] << " cannot be read: " << status.ToString() );
                return EXIT_ADMIN_FAILED;
            }
            if ( followed.GetAttributeValueOrChildValueByName( "allEnded" ).AsBool( false, true ) )
            {
                result = followed;
                break;
            }
            MT::ThreadDelay( waitMs );
        }
    }
    if ( waitForPlan )
    {
        COM::CIServiceAdminHandler::TArgumentMap statusArguments;
        statusArguments[ "job" ] = result.GetAttributeValueOrChildValueByName( "jobId" ).AsString();
        const UInt32 waitMs = 500;
        const UInt32 maxWaits = 2 * 3600;     // an hour: a listing of a large library with its details is paced by the store
        for ( UInt32 w=0; w<maxWaits; ++w )
        {
            CORE::CDataNode planStatus;
            if ( !COM::CServiceAdminClient::RunWithTokenFile( service.adminPort, stateDir, "sync-status", statusArguments, planStatus, status ) || !status.IsOk() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: the state of the plan " << statusArguments[ "job" ] << " cannot be read: " << status.ToString() );
                return EXIT_ADMIN_FAILED;
            }
            const CORE::CDataNode* job = planStatus.FindChild( "job" );
            const CORE::CString state = GUCEF_NULL != job ? job->GetAttributeValueOrChildValueByName( "state" ).AsString() : CORE::CString::Empty;
            if ( "done" == state || "failed" == state || "cancelled" == state )
            {
                result = planStatus;
                break;
            }
            MT::ThreadDelay( waitMs );
        }
    }
    CORE::CString json;
    if ( !COM::CJsonRpcEndpoint::ToJsonText( result, json ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: the answer of the admin command \"" << command << "\" cannot be written as JSON" );
        return EXIT_ADMIN_FAILED;
    }
    CORE::CStandardOutputWriter output;
    const bool written = output.Start() && output.WriteLine( json );
    output.Stop( true );
    json.Clear();
    return written ? EXIT_STOPPED : EXIT_ADMIN_FAILED;
}

/*-------------------------------------------------------------------------*/

void
PrintUsage( void )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreService: the asset store as a remote service. Options: --config FILE (or "
                                             "GUCEF_ASSETSTORE_SERVICE_CONFIG), --bootstrap FILE (or GUCEF_ASSETSTORE_SERVICE_BOOTSTRAP; default "
                                             "AssetStoreService_bootstrap.ini next to the program: the plugins), --data-dir DIR, --log-file FILE, --console, --smoke-test (start, "
                                             "become ready, drain, exit), --admin <command> [--name value ...] (status, pair-open [--kind K --roles "
                                             "R --seconds S --count N], pair-close, pairings, pair-confirm --device D --fingerprint F, devices, "
                                             "sessions, revoke --device D, keyring-status, keyring-setup, keyring-unlock, keyring-lock [--code-file FILE] (the code comes from the standard input or "
                                             "the file, never from the command line), drain; the reprocessing of packs: reprocess|redownload|regen-intermediate|regen-target|reprocess-rollback|reprocess-plan <pack> [<pack> ...] "
                                             "[--assets a,b --failed true --statuses a,b --kinds a,b --targets ogre --steps redownload,intermediate,target --no-cook true --then-convert false "
                                             "--allow-large true --accept-errors true --dry-run true --follow true], reprocess-status|reprocess-cancel <job> [<job> ...]; "
                                             "the library sync: sync plan|start|status|cancel|retry|pause|resume <store> "
                                             "(sync plan waits for the plan unless --no-wait; start also --max-items N, --item-ids A,B, --retry-failed true, --replan true (list afresh, not the stored plan), --allow-large true (with --item-ids: past the size guard of the store); plan and start "
                                             "--refresh-details true (ask the details of the items again, not the cache); status --items N lines; cancel, retry and status also --job J), "
                                             "sync marks <store>, sync ignore|pin|unmark <store> <item> [reason], sync profile <store> [--member value ...]; the run-time settings: "
                                             "settings-describe, settings-set key=value ..., settings-reset key ... (also settings describe|set|reset)), --help. Exit codes: 0 stopped in order or the admin command "
                                             "succeeded, 2 bad options, 3 the bootstrap (plugins) or the settings refused, 4 the service did not start, 5 the "
                                             "admin command failed, 6 the smoke test did not become ready" );
}

/*-------------------------------------------------------------------------*/

/** SIGTERM and SIGINT: noted async-signal-safely; the drain starts at the next pulse */
void
NoteStopSignal( int signal )
{
    CORE::CServiceStopSignals::NoteSignal( signal );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSSERVICEMAIN_BEGIN( "AssetStoreService" )
{GUCEF_TRACE;

    SOptions options;
    const bool optionsValid = ParseOptions( argc, argv, options );
    GUCEF_OSMAIN_SIGNAL_HANDLER( NoteStopSignal );

    // the admin mode answers on standard output: the console logger goes to standard error then
    if ( options.admin )
        CORE::CStandardOutputWriter::ClaimExclusive();
    CORE::CPlatformNativeConsoleLogger console;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );
    CORE::CFileAccess* logFile = GUCEF_NULL;
    CORE::CStdLogger* fileLogger = GUCEF_NULL;
    if ( !options.logFile.IsNULLOrEmpty() )
    {
        logFile = GUCEF_NEW CORE::CFileAccess( options.logFile, "w" );
        fileLogger = GUCEF_NEW CORE::CStdLogger( *logFile );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( fileLogger );
    }
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    // the systems whose plugins the bootstrap file loads
    if ( optionsValid && !options.help )
    {
        COM::CComGlobal::Instance();
        VFS::CVfsGlobal::Instance();
        ASSETSTORE::CAssetStoreGlobal::Instance();
    }

    int exitCode = EXIT_STOPPED;
    CORE::CDataNode settings;
    CORE::CString problem;
    SServiceSection serviceSection;
    if ( !optionsValid || options.help )
    {
        PrintUsage();
        exitCode = options.help ? EXIT_STOPPED : EXIT_BAD_OPTIONS;
    }
    else
    if ( !LoadBootstrap( options.bootstrapFile, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: " << problem );
        exitCode = EXIT_SETTINGS_REFUSED;
    }
    else
    if ( !ReadSettingsFile( options.configFile, settings, problem ) || !serviceSection.Load( settings.FindChild( "Service" ), problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: " << problem );
        exitCode = EXIT_SETTINGS_REFUSED;
    }
    else
    if ( options.admin )
    {
        if ( !options.dataDir.IsNULLOrEmpty() )
            serviceSection.dataDir = options.dataDir;
        COM::CComGlobal::Instance();
        exitCode = RunAdminCommand( serviceSection, options );
    }
    else
    {
        if ( !options.dataDir.IsNULLOrEmpty() )
            serviceSection.dataDir = options.dataDir;
        COM::CComGlobal::Instance();
        ASSETSTORE::CAssetStoreGlobal::Instance();
        CORE::PulseGeneratorPtr pulse = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        CORE::CServiceLifecycle lifecycle( pulse );
        CORE::CDataNodeSerializableSettings serializerSettings;

        // extra plugins the settings name, besides those of the bootstrap file
        CORE::CString::StringVector plugins;
        settings.GetValuesOfChildByName( "Plugins", plugins );
        for ( size_t p=0; p<plugins.size(); ++p )
        {
            if ( IsPluginKnown( plugins[ p ] ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreService: The plugin " << plugins[ p ] << " of the settings is loaded by the bootstrap file already" );
            }
            else
            if ( !LoadExtraPlugin( plugins[ p ] ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: The plugin " << plugins[ p ] << " of the settings was not loaded" );
            }
        }

        // the data folder: state (devices, bans, admin token), staging, the command journal, the credentials
        const CORE::CString dataDir = CORE::RelativePath( serviceSection.dataDir );
        const CORE::CString stateDir = CORE::CombinePath( dataDir, "state" );
        EnsureOwnerOnlyDirectory( stateDir, "state" );
        EnsureOwnerOnlyDirectory( dataDir, "data" );

        // the settings of every part
        AUTH::CMemoryCredentialStore passwords;
        ASSETSTORE::CAssetStoreServiceConfig serviceConfig;
        const CORE::CDataNode* serviceNode = settings.FindChild( "AssetStoreService" );
        DRNUTILS::CRemoteServiceHostSettings hostSettings;
        hostSettings.serviceName = "gucef.assetstore";
        hostSettings.drnSessionName = "assetstore";
        hostSettings.sessionPort = 47400;
        hostSettings.pairingPort = 47401;
        hostSettings.adminRoles.clear();
        hostSettings.adminRoles.insert( "owner" );
        hostSettings.drainSeconds = serviceSection.drainSeconds;
        hostSettings.expectedBackSeconds = serviceSection.expectedBackSeconds;
        hostSettings.lanDiscoveryAdvertiser.serviceType = ASSETSTORE::CRemoteAssetStoreServiceSettings::DefaultServiceType;
        hostSettings.lanDiscoveryAdvertiser.displayName = serviceSection.displayName;
        ASSETSTORE::CAssetStoreServiceHostSettings bindingSettings;
        bindingSettings.stagingDirectory = CORE::CombinePath( dataDir, "staging" );
        bindingSettings.commandJournalDirectory = CORE::CombinePath( dataDir, "commands" );
        ASSETSTORECONVERTER::CConverterHookSettings hookSettings;
        ASSETSTORECONVERTER::CPipelineOperationSettings operationSettings;
        const CORE::CDataNode* credentialsNode = settings.FindChild( "Credentials" );
        static const char* credentialKeys[] = { "directory", "keyFile", "keyFileTrust", "keyringIdleLockMinutes" };
        bool settingsValid = true;
        if ( GUCEF_NULL != serviceNode && !serviceConfig.Deserialize( *serviceNode, serializerSettings ) )
        {
            problem = "the section AssetStoreService is refused";
            settingsValid = false;
        }
        if ( settingsValid && !ApplyDatabaseSection( settings.FindChild( "Database" ), serviceConfig.primaryStore, passwords, problem ) )
            settingsValid = false;
        if ( settingsValid && !ApplyStorageSection( settings.FindChild( "Storage" ), serviceConfig.primaryStore, problem ) )
            settingsValid = false;
        if ( settingsValid && GUCEF_NULL != settings.FindChild( "RemoteHost" ) && !hostSettings.LoadConfig( *settings.FindChild( "RemoteHost" ) ) )
        {
            problem = "the section RemoteHost is refused (see the log)";
            settingsValid = false;
        }
        if ( settingsValid && GUCEF_NULL != settings.FindChild( "AssetStoreHost" ) && !bindingSettings.LoadConfig( *settings.FindChild( "AssetStoreHost" ) ) )
        {
            problem = "the section AssetStoreHost is refused (see the log)";
            settingsValid = false;
        }
        if ( settingsValid && GUCEF_NULL != settings.FindChild( "ConverterHook" ) && !hookSettings.Deserialize( *settings.FindChild( "ConverterHook" ), serializerSettings ) )
        {
            problem = "the section ConverterHook is refused";
            settingsValid = false;
        }
        if ( settingsValid && GUCEF_NULL != settings.FindChild( "PipelineOperations" ) &&
             ( !operationSettings.Deserialize( *settings.FindChild( "PipelineOperations" ), serializerSettings ) || !operationSettings.IsValid( problem ) ) )
        {
            if ( problem.IsNULLOrEmpty() )
                problem = "the section PipelineOperations is refused";
            settingsValid = false;
        }
        if ( settingsValid && !CheckKeys( credentialsNode, "Credentials", credentialKeys, 4, problem ) )
            settingsValid = false;

        if ( !settingsValid )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: The settings are refused: " << problem );
            lifecycle.Fail( problem );
            exitCode = EXIT_SETTINGS_REFUSED;
        }
        else
        {
            lifecycle.Transition( CORE::SERVICELIFECYCLE_LOADING, "the settings are read" );

            // the credential store of the keys of the service (its identity: clients stay paired across restarts) and the journal key
            const CORE::CString credentialsDir = credentialsNode != GUCEF_NULL ? credentialsNode->GetAttributeValueOrChildValueByName( "directory" ).AsString( CORE::CombinePath( dataDir, "credentials" ), true )
                                                                                : CORE::CombinePath( dataDir, "credentials" );
            EnsureOwnerOnlyDirectory( credentialsDir, "credentials" );
            AUTH::CFileCredentialStore credentials( credentialsDir );
            if ( GUCEF_NULL != credentialsNode )
            {
                const CORE::CString keyFile = credentialsNode->GetAttributeValueOrChildValueByName( "keyFile" ).AsString();
                if ( !keyFile.IsNULLOrEmpty() )
                    credentials.SetKeyFile( keyFile );
                AUTH::CFileCredentialStore::EKeyFileTrust trust = AUTH::CFileCredentialStore::KEYFILETRUST_OWNER_ONLY;
                if ( AUTH::CFileCredentialStore::KeyFileTrustFromString( credentialsNode->GetAttributeValueOrChildValueByName( "keyFileTrust" ).AsString( "owner-only", true ), trust ) )
                    credentials.SetKeyFileTrust( trust );
            }

            // the sign in of the online stores on the server: the sessions stay in the credential store above (sealed with its key, under the
            // data folder, for the service user alone); the saved passwords of the stores that sign in on a web site are in the keyring code
            // mode of the same folder, unlocked by an admin device or the admin command and never kept; the flows of the stores that sign in with
            // steps of a person (an address to open, a code to enter) run on the manager of gucefAUTH, which keeps its tokens in the same store
            AUTH::CFileCredentialStore keyringStore( credentialsDir );
            keyringStore.SetKeyMode( AUTH::CFileCredentialStore::KEYMODE_KEYRING_CODE );
            AUTH::CKeyringSession keyringSession( &keyringStore );
            {
                AUTH::CKeyringSettings keyringSettings = keyringSession.GetSettings();
                const UInt32 idleMinutes = credentialsNode != GUCEF_NULL ? credentialsNode->GetAttributeValueOrChildValueByName( "keyringIdleLockMinutes" ).AsUInt32( 240, true ) : 240;
                keyringSettings.idleLockInSeconds = idleMinutes * 60;
                keyringSession.SetSettings( keyringSettings );
            }
            AUTH::CAuthManager& authManager = AUTH::CAuthGlobal::Instance()->GetManager();
            authManager.SetCredentialStore( &credentials );
            ASSETSTORE::CAssetStoreGlobal::Instance()->SetWebSignInStores( &credentials, &keyringStore );
            serviceConfig.signIn.authManager = &authManager;
            serviceConfig.signIn.sessionStore = &credentials;
            serviceConfig.signIn.credentialStore = &keyringStore;
            serviceConfig.signIn.keyringSession = &keyringSession;
            serviceConfig.signIn.browserAvailable = false;
            if ( GUCEF_NULL != settings.FindChild( "Stores" ) )
                serviceConfig.signIn.storeSettings.Copy( *settings.FindChild( "Stores" ) );
            serviceConfig.signIn.storeSettings.SetName( "storeSettings" );

            // the health endpoints answer from the start: startup and readiness fail until the service is ready
            CORE::CServiceHealthRegistry& registry = CORE::CCoreGlobal::Instance()->GetServiceHealthRegistry();
            registry.SetPulseGenerator( pulse );
            registry.SetStarted( false );
            WEB::CHttpServerSettings healthServerSettings;
            healthServerSettings.bindAddress = serviceSection.healthBindAddress;
            WEB::CHTTPServer healthServer( pulse, GUCEF_NULL, &healthServerSettings );
            WEB::CServiceHealthHttpEndpoints healthEndpoints( registry );
            bool healthUp = true;
            if ( 0 != serviceSection.healthPort )
                healthUp = healthEndpoints.RegisterWith( healthServer ) && healthServer.ListenOnPort( serviceSection.healthPort );
            if ( !healthUp )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: the health endpoints cannot listen on port " << serviceSection.healthPort );

            // the service of this process with the converter pipeline as its converter
            ASSETSTORECONVERTER::CPipelineConverterHook hook( hookSettings );
            hook.SetOperationSettings( operationSettings );
            serviceConfig.implementationId = ASSETSTORE::CAssetStoreServiceConfig::LocalImplementationId;
            serviceConfig.converterHook = &hook;
            serviceConfig.credentialStore = &passwords;
            serviceConfig.pulseGenerator = pulse;
            CORE::CString serviceError;
            ASSETSTORE::TAssetStoreServicePtr service = ASSETSTORE::CAssetStoreGlobal::Instance()->CreateService( serviceConfig, serviceError );
            if ( service.IsNULL() || !healthUp )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: The asset store service cannot be made: " << serviceError );
                lifecycle.Fail( "the service cannot be made" );
                exitCode = EXIT_START_FAILED;
            }
            else
            {
                // a database that is not there now makes the service DEGRADED, not stopped: it is tried again
                if ( ASSETSTORE::ASSETSTORE_STATUS_OK != service->Open() )
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: The catalogue is not available yet: " << service->GetStatusMessage() );

                DRNUTILS::CRemoteServiceHost host( pulse, hostSettings );
                host.SetStateDirectory( stateDir );
                host.SetCredentialStore( &credentials );
                ASSETSTORE::CAssetStoreServiceHost binding( host, service, bindingSettings );
                binding.SetCredentialStore( &credentials );
                CORE::CServiceResourceWatch watch;
                watch.Configure( serviceSection.resourceSampleMs, serviceSection.resourceLogSeconds, (UInt64) serviceSection.memorySoftLimitMB * 1024ULL * 1024ULL );
                CAdminHandler adminHandler( lifecycle, host, binding, *service, watch );
                adminHandler.SetKeyring( &keyringSession );
                // the service of this process is the local one (implementationId above)
                adminHandler.SetLocalService( static_cast< ASSETSTORE::CLocalAssetStoreService* >( service.GetPointerAlways() ) );
                COM::CServiceAdminEndpoint admin( adminHandler, pulse );
                CORE::CString startError;
                if ( !host.Start( startError ) || !binding.Attach( startError ) || ( 0 != serviceSection.adminPort && !admin.Open( stateDir, serviceSection.adminPort, startError ) ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: The service did not start: " << startError );
                    lifecycle.Fail( startError );
                    exitCode = EXIT_START_FAILED;
                }
                else
                {
                    watch.AddSource( &host );
                    watch.AddSource( &binding );
                    CCatalogueCheck catalogueCheck( *service );
                    CStorageCheck storageCheck( serviceConfig.primaryStore.locations );
                    CServingCheck servingCheck( lifecycle, host );
                    CNodeLivenessCheck livenessCheck( host );
                    registry.RegisterReadinessCheck( &servingCheck );
                    registry.RegisterReadinessCheck( &catalogueCheck );
                    registry.RegisterReadinessCheck( &storageCheck );
                    registry.RegisterLivenessCheck( &livenessCheck );

                    lifecycle.Transition( service->IsCatalogueAvailable() ? CORE::SERVICELIFECYCLE_READY : CORE::SERVICELIFECYCLE_DEGRADED, "listening" );
                    CServiceDriver driver( serviceSection, lifecycle, host, binding, *service, watch, registry, options.smokeTest );
                    adminHandler.SetDriver( &driver );
                    registry.SetStarted( true );
                    registry.EvaluateChecks();
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: " << CORE::ServiceLifecycleStateToString( lifecycle.GetState() ) << ", sessions on port "
                               << host.GetSessionPort() << ", health on port " << healthServer.GetBoundPort() << ", admin on 127.0.0.1:" << admin.GetBoundPort()
                               << ( host.IsAdvertisingOnLan() ? ", announced on the local network as " + hostSettings.lanDiscoveryAdvertiser.serviceType : CORE::CString( ", not announced on the local network" ) ) );

                    CORE::CGUCEFApplication& app = CORE::CCoreGlobal::Instance()->GetApplication();
                    app.GetPulseGenerator()->RequestPulseInterval( 10 );
                    app.main( argc, argv, true );

                    if ( options.smokeTest && !driver.ReachedReady() )
                        exitCode = EXIT_SMOKE_FAILED;
                    adminHandler.SetDriver( GUCEF_NULL );
                    registry.SetStarted( false );
                    registry.UnregisterLivenessCheck( &livenessCheck );
                    registry.UnregisterReadinessCheck( &storageCheck );
                    registry.UnregisterReadinessCheck( &catalogueCheck );
                    registry.UnregisterReadinessCheck( &servingCheck );
                    watch.RemoveSource( &binding );
                    watch.RemoveSource( &host );
                }
                admin.Close();
                binding.Detach();
                host.Stop();
                service->Close();
                if ( EXIT_STOPPED == exitCode )
                    lifecycle.Transition( CORE::SERVICELIFECYCLE_STOPPED, "stopped in order" );
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreService: " << CORE::ServiceLifecycleStateToString( lifecycle.GetState() ) << ", exit code " << exitCode );
            }
            service.Unlink();
            // the stores of the sign in go out of scope below: nothing may keep their addresses
            ASSETSTORE::CAssetStoreGlobal::Instance()->SetWebSignInStores( GUCEF_NULL, GUCEF_NULL );
            authManager.SetCredentialStore( GUCEF_NULL );
            healthEndpoints.Unregister();
            healthServer.Close();
        }
    }

    // The plugins go here, in order, while everything they use is alive: the service has unloaded them when its application
    // stopped, but the other modes (an admin command, help, a refused setting) never ran the application. Left to the
    // exit handlers they unload after the function local statics they made (a provider identifier, a lock) are destroyed:
    // an unload that needs one aborts, or leaves its provider in the registry to be released after its code is gone.
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadAll();

    CORE::CLogStreamScope::FlushLogs();
    if ( GUCEF_NULL != fileLogger )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( fileLogger );
        GUCEF_DELETE fileLogger;
        GUCEF_DELETE logFile;
    }
    if ( options.admin )
        CORE::CStandardOutputWriter::ReleaseClaim();
    return exitCode;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
