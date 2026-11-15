/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */
#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */
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
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */
#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#include "gucefASSETSTORE_CAssetNameHints.h"
#define GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#endif /* GUCEF_ASSETSTORE_CASSETNAMEHINTS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTEVENTDATA_H
#include "gucefASSETSTORE_CAssetIngestEventData.h"
#define GUCEF_ASSETSTORE_CASSETINGESTEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTEVENTDATA_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONEVENTDATA_H
#include "gucefASSETSTORE_CAssetConversionEventData.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONEVENTDATA_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H
#include "gucefASSETSTORE_CAssetPackConversion.h"
#define GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#include "gucefASSETSTORE_CIAssetConverterHook.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTPIPELINE_H
#include "gucefASSETSTORE_CAssetIngestPipeline.h"
#define GUCEF_ASSETSTORE_CASSETINGESTPIPELINE_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTPIPELINE_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTOREOPERATIONTASKCONSUMER_H
#include "gucefASSETSTORE_CPrimaryAssetStoreOperationTaskConsumer.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTOREOPERATIONTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTOREOPERATIONTASKCONSUMER_H ? */
#include "gucefASSETSTORE_CPrimaryAssetStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CPrimaryAssetStore::ProviderId = "primary";
const CORE::CEvent CPrimaryAssetStore::IngestProgressEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::IngestProgressEvent";
const CORE::CEvent CPrimaryAssetStore::IngestFinishedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::IngestFinishedEvent";
const CORE::CEvent CPrimaryAssetStore::IngestFailedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::IngestFailedEvent";
const CORE::CEvent CPrimaryAssetStore::ConversionProgressEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::ConversionProgressEvent";
const CORE::CEvent CPrimaryAssetStore::ConversionFinishedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::ConversionFinishedEvent";
const CORE::CEvent CPrimaryAssetStore::ConversionFailedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::ConversionFailedEvent";
const CORE::CEvent CPrimaryAssetStore::LocationStateChangedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::LocationStateChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The bytes of the text without the terminating zero, for every string format of CString */
inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

CORE::CString
NormalizeVfsPath( const CORE::CString& path )
{
    CORE::CString normalized = path.ReplaceChar( '\\', '/' );
    while ( ByteLengthOf( normalized ) > 0 && '/' == normalized.C_String()[ 0 ] )
        normalized = normalized.CutChars( 1, true );
    while ( ByteLengthOf( normalized ) > 0 && '/' == normalized.C_String()[ ByteLengthOf( normalized )-1 ] )
        normalized = normalized.CutChars( 1, false );
    return normalized;
}

CORE::CString
JoinVfs( const CORE::CString& a, const CORE::CString& b )
{
    const CORE::CString left = NormalizeVfsPath( a );
    const CORE::CString right = NormalizeVfsPath( b );
    if ( left.IsNULLOrEmpty() )
        return right;
    if ( right.IsNULLOrEmpty() )
        return left;
    return left + "/" + right;
}

CORE::CString
LocationMountName( const CORE::CString& locationId )
{
    return "assetstore-location-" + locationId;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::RegisterEvents( void )
{GUCEF_TRACE;

    IngestProgressEvent.Initialize();
    IngestFinishedEvent.Initialize();
    IngestFailedEvent.Initialize();
    ConversionProgressEvent.Initialize();
    ConversionFinishedEvent.Initialize();
    ConversionFailedEvent.Initialize();
    LocationStateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPrimaryAssetStore::CPrimaryAssetStore( void )
    : CIAssetStoreProvider()
    , CORE::CIConfigurable()
    , m_settings()
    , m_credentialStore( GUCEF_NULL )
    , m_converterHook( GUCEF_NULL )
    , m_downloadManager( GUCEF_NULL )
    , m_catalogue()
    , m_vocabulary()
    , m_searchEngine( m_catalogue, m_vocabulary )
    , m_locationOnline()
    , m_mountedLocations()
    , m_pendingOperations( 0 )
    , m_lastError()
    , m_lock()
    , m_indexLock()
{GUCEF_TRACE;

    CAssetStoreGlobal::Instance();
    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPrimaryAssetStore::~CPrimaryAssetStore()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    AttachDownloadManager( GUCEF_NULL );
    while ( m_pendingOperations.Load() > 0 )
        MT::ThreadDelay( 2 );
    WaitForAsyncOperations();
    Close();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::Fail( TAssetStoreStatus status, const CORE::CString& message ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_lastError = message;
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: " << message );
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPrimaryAssetStore::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastError;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::SetSettings( const CPrimaryAssetStoreSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

CPrimaryAssetStoreSettings
CPrimaryAssetStore::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::SetCredentialStore( AUTH::ICredentialStore* credentialStore )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_credentialStore = credentialStore;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::SetConverterHook( CIAssetConverterHook* converterHook )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_converterHook = converterHook;
}

/*-------------------------------------------------------------------------*/

CIAssetConverterHook*
CPrimaryAssetStore::GetConverterHook( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_converterHook;
}

/*-------------------------------------------------------------------------*/

MT::CMutex&
CPrimaryAssetStore::GetIndexLock( void )
{GUCEF_TRACE;

    return m_indexLock;
}

/*-------------------------------------------------------------------------*/

CAssetCatalogue&
CPrimaryAssetStore::GetCatalogue( void )
{GUCEF_TRACE;

    return m_catalogue;
}

/*-------------------------------------------------------------------------*/

CAssetVocabulary&
CPrimaryAssetStore::GetVocabulary( void )
{GUCEF_TRACE;

    return m_vocabulary;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::Open( UInt32 targetSchemaVersion )
{GUCEF_TRACE;

    Close();
    const CPrimaryAssetStoreSettings settings = GetSettings();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    // the locations that name a folder are mounted at their roots
    for ( size_t l=0; l<settings.locations.size(); ++l )
    {
        const CAssetStorageLocation& location = settings.locations[ l ];
        if ( location.fileSystemPath.IsNULLOrEmpty() || vfs.IsMountedArchive( LocationMountName( location.locationId ) ) )
            continue;
        const CORE::CString folder = CORE::RelativePath( CORE::ResolveVars( location.fileSystemPath ) );
        if ( !CORE::DirExists( folder ) && location.writeable )
            CORE::CreateDirs( folder );
        if ( !CORE::DirExists( folder ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: The folder of storage location " << location.locationId << " is not there: " << folder );
            continue;
        }
        if ( vfs.AddRoot( folder, LocationMountName( location.locationId ), location.writeable, false, location.vfsRoot ) )
        {
            MT::CScopeMutex lock( m_lock );
            m_mountedLocations.insert( location.locationId );
        }
    }

    // the seed data of the module
    if ( !vfs.FileExists( JoinVfs( settings.seedDataPath, "vocabulary.json" ) ) && settings.seedDataPath == "gucefASSETSTORE" )
        vfs.MountModuleData( "gucefASSETSTORE" );
    CORE::CString seedError;
    const bool hasSeed = m_vocabulary.LoadSeed( settings.seedDataPath, seedError );
    if ( !hasSeed )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: No seed data: " << seedError );

    // the database
    COMCORE::CDatabaseClientSettings databaseSettings;
    CORE::CString error;
    AUTH::ICredentialStore* credentialStore = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        credentialStore = m_credentialStore;
    }
    if ( !settings.GetDatabaseSettingsWithCredentials( credentialStore, databaseSettings, error ) )
        return Fail( ASSETSTORE_STATUS_NOT_SIGNED_IN, "The catalogue cannot be opened: " + error );
    if ( databaseSettings.backendName.Equals( "vfstables", false ) && !vfs.DirExists( databaseSettings.database ) )
        vfs.CreateDir( databaseSettings.database );
    COMCORE::CIDatabaseClientPtr client = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( databaseSettings.backendName );
    if ( client.IsNULL() )
        return Fail( ASSETSTORE_STATUS_UNSUPPORTED, "There is no database backend \"" + databaseSettings.backendName + "\" (is its plugin loaded?)" );
    if ( !client->Open( databaseSettings ) )
        return Fail( ASSETSTORE_STATUS_DATABASE_ERROR, "The catalogue database cannot be opened: " + client->GetLastErrorMessage() );

    TAssetStoreStatus status = m_catalogue.Open( client, hasSeed ? &m_vocabulary : GUCEF_NULL, targetSchemaVersion );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        client->Close();
        return Fail( status, "The catalogue cannot be opened: " + m_catalogue.GetLastErrorMessage() );
    }
    if ( m_catalogue.GetSchemaVersion() >= 2 )
    {
        status = m_vocabulary.LoadFromCatalogue( m_catalogue );
        if ( ASSETSTORE_STATUS_OK != status )
            return Fail( status, "The vocabulary cannot be read: " + m_catalogue.GetLastErrorMessage() );
    }
    if ( m_catalogue.GetSchemaVersion() >= 1 )
    {
        SyncLocationAndSourceRows();
        RefreshLocationStates();
        RepairInterruptedPacks();
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Opened, catalogue schema version " << m_catalogue.GetSchemaVersion() );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::Close( void )
{GUCEF_TRACE;

    COMCORE::CIDatabaseClientPtr client = m_catalogue.GetClient();
    m_catalogue.Close();
    if ( !client.IsNULL() )
        client->Close();

    CORE::CString::StringSet mounted;
    {
        MT::CScopeMutex lock( m_lock );
        mounted = m_mountedLocations;
        m_mountedLocations.clear();
        m_locationOnline.clear();
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( CORE::CString::StringSet::const_iterator m=mounted.begin(); m!=mounted.end(); ++m )
        vfs.UnmountArchiveByName( LocationMountName( (*m) ) );
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStore::IsOpen( void ) const
{GUCEF_TRACE;

    return m_catalogue.IsOpen();
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::SyncLocationAndSourceRows( void )
{GUCEF_TRACE;

    const CPrimaryAssetStoreSettings settings = GetSettings();
    CAssetCatalogue::TRowVector locations;
    for ( size_t l=0; l<settings.locations.size(); ++l )
    {
        CAssetCatalogue::TRow row;
        row[ "location_id" ] = CORE::CVariant( settings.locations[ l ].locationId );
        row[ "name" ] = CORE::CVariant( settings.locations[ l ].name );
        row[ "vfs_root" ] = CORE::CVariant( settings.locations[ l ].vfsRoot );
        row[ "is_default" ] = CORE::CVariant( settings.locations[ l ].locationId == settings.defaultLocationId );
        row[ "online" ] = CORE::CVariant( IsLocationOnline( settings.locations[ l ].locationId ) );
        row[ "checked_ms" ] = CORE::CVariant( (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() );
        locations.push_back( row );
    }
    m_catalogue.Write( "store_locations", locations );

    CAssetCatalogue::TRowVector sources;
    for ( size_t s=0; s<settings.sources.size(); ++s )
    {
        CAssetCatalogue::TRow row;
        row[ "source_id" ] = CORE::CVariant( settings.sources[ s ].sourceId );
        row[ "ingest_mode" ] = CORE::CVariant( CORE::CString( AssetIngestModeToString( settings.sources[ s ].mode ) ) );
        row[ "keep" ] = CORE::CVariant( CORE::CString( AssetKeepPolicyToString( settings.sources[ s ].keep ) ) );
        row[ "location_id" ] = CORE::CVariant( settings.sources[ s ].locationId );
        sources.push_back( row );
    }
    m_catalogue.Write( "sources", sources );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::RefreshLocationStates( void )
{GUCEF_TRACE;

    const CPrimaryAssetStoreSettings settings = GetSettings();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CString::StringVector changed;
    for ( size_t l=0; l<settings.locations.size(); ++l )
    {
        const CAssetStorageLocation& location = settings.locations[ l ];
        const bool online = !location.vfsRoot.IsNULLOrEmpty() && vfs.DirExists( location.vfsRoot );
        MT::CScopeMutex lock( m_lock );
        GUCEF::map< CORE::CString, bool >::iterator known = m_locationOnline.find( location.locationId );
        if ( known == m_locationOnline.end() || (*known).second != online )
            changed.push_back( location.locationId );
        m_locationOnline[ location.locationId ] = online;
    }
    if ( changed.empty() )
        return;
    if ( m_catalogue.IsOpen() )
        SyncLocationAndSourceRows();
    for ( size_t c=0; c<changed.size(); ++c )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Storage location " << changed[ c ] << " is " << ( IsLocationOnline( changed[ c ] ) ? "reachable" : "not reachable" ) );
        CORE::TCloneableString eventData( changed[ c ] );
        NotifyObserversFromThread( LocationStateChangedEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStore::IsLocationOnline( const CORE::CString& locationId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    GUCEF::map< CORE::CString, bool >::const_iterator i = m_locationOnline.find( locationId );
    return i != m_locationOnline.end() && (*i).second;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPrimaryAssetStore::GetLocationRoot( const CORE::CString& locationId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    const CAssetStorageLocation* location = m_settings.FindLocation( locationId );
    return GUCEF_NULL == location ? CORE::CString::Empty : NormalizeVfsPath( location->vfsRoot );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPrimaryAssetStore::FindLocationOfVfsPath( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    const CORE::CString path = NormalizeVfsPath( vfsPath );
    MT::CScopeMutex lock( m_lock );
    CORE::CString best;
    UInt32 bestLength = 0;
    for ( size_t l=0; l<m_settings.locations.size(); ++l )
    {
        const CORE::CString root = NormalizeVfsPath( m_settings.locations[ l ].vfsRoot );
        if ( root.IsNULLOrEmpty() )
            continue;
        if ( ( path == root || 0 == path.HasSubstr( root + "/", true ) ) && root.Length() > bestLength )
        {
            best = m_settings.locations[ l ].locationId;
            bestLength = root.Length();
        }
    }
    return best;
}

/*-------------------------------------------------------------------------*/

CAssetSearchEngine::TLocationStateMap
CPrimaryAssetStore::GetLocationStates( void ) const
{GUCEF_TRACE;

    CAssetSearchEngine::TLocationStateMap states;
    MT::CScopeMutex lock( m_lock );
    for ( size_t l=0; l<m_settings.locations.size(); ++l )
    {
        CAssetSearchEngine::SLocationState state;
        state.vfsRoot = NormalizeVfsPath( m_settings.locations[ l ].vfsRoot );
        GUCEF::map< CORE::CString, bool >::const_iterator online = m_locationOnline.find( m_settings.locations[ l ].locationId );
        state.online = online != m_locationOnline.end() && (*online).second;
        states[ m_settings.locations[ l ].locationId ] = state;
    }
    return states;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RelocateLocation( const CORE::CString& locationId, const CORE::CString& newVfsRoot, const CORE::CString& newFileSystemPath )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CAssetStorageLocation location;
    {
        MT::CScopeMutex lock( m_lock );
        CAssetStorageLocation* found = GUCEF_NULL;
        for ( size_t l=0; l<m_settings.locations.size(); ++l )
            if ( m_settings.locations[ l ].locationId == locationId )
                found = &m_settings.locations[ l ];
        if ( GUCEF_NULL == found )
            return Fail( ASSETSTORE_STATUS_NOT_FOUND, "There is no storage location " + locationId );
        found->vfsRoot = newVfsRoot;
        if ( !newFileSystemPath.IsNULLOrEmpty() )
            found->fileSystemPath = newFileSystemPath;
        location = *found;
    }
    if ( !newFileSystemPath.IsNULLOrEmpty() )
    {
        vfs.UnmountArchiveByName( LocationMountName( locationId ) );
        const CORE::CString folder = CORE::RelativePath( CORE::ResolveVars( newFileSystemPath ) );
        if ( CORE::DirExists( folder ) && vfs.AddRoot( folder, LocationMountName( locationId ), location.writeable, false, newVfsRoot ) )
        {
            MT::CScopeMutex lock( m_lock );
            m_mountedLocations.insert( locationId );
        }
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Storage location " << locationId << " moved to " << newVfsRoot );
    {
        MT::CScopeMutex lock( m_lock );
        m_locationOnline.erase( locationId );       // checked again below, the change is reported
    }
    RefreshLocationStates();
    if ( m_catalogue.IsOpen() )
        SyncLocationAndSourceRows();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::VerifyLocation( const CORE::CString& locationId, CAssetVerificationReport& report )
{GUCEF_TRACE;

    report = CAssetVerificationReport();
    report.locationId = locationId;
    const CORE::CString root = GetLocationRoot( locationId );
    if ( root.IsNULLOrEmpty() )
        return report.status = Fail( ASSETSTORE_STATUS_NOT_FOUND, "There is no storage location " + locationId );
    RefreshLocationStates();
    if ( !IsLocationOnline( locationId ) )
        return report.status = Fail( ASSETSTORE_STATUS_UNAVAILABLE, "The storage location " + locationId + " is not reachable" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDataTableQuery query;
    query.WhereEquals( "location_id", CORE::CVariant( locationId ) );
    CAssetCatalogue::TRowVector packs;
    TAssetStoreStatus status = m_catalogue.Select( "packs", query, packs );
    if ( ASSETSTORE_STATUS_OK != status )
        return report.status = status;
    for ( size_t p=0; p<packs.size(); ++p )
    {
        const CORE::CString state = CAssetCatalogue::GetText( packs[ p ], "state" );
        if ( !AssetPackStateIsIngested( state ) )
            continue;
        ++report.packsChecked;
        const CORE::CString packRoot = JoinVfs( root, CAssetCatalogue::GetText( packs[ p ], "root_path" ) );
        CAssetCatalogue::TRowVector files;
        CORE::CDataTableQuery fileQuery;
        fileQuery.WhereEquals( "pack_id", CORE::CVariant( CAssetCatalogue::GetText( packs[ p ], "pack_id" ) ) );
        if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.Select( "asset_files", fileQuery, files ) ) )
            return report.status = status;
        for ( size_t f=0; f<files.size(); ++f )
        {
            const CORE::CString path = JoinVfs( packRoot, CAssetCatalogue::GetText( files[ f ], "path" ) );
            const CORE::CString lower = path.Lowercase();
            if ( lower.HasSubstr( ".zip/", true ) >= 0 || lower.HasSubstr( ".7z/", true ) >= 0 || lower.HasSubstr( ".rar/", true ) >= 0 || lower.HasSubstr( ".unitypackage/", true ) >= 0 )
                continue;       // inside an archive: the archive itself is checked
            ++report.filesChecked;
            if ( !vfs.FileExists( path ) )
            {
                report.missingFiles.push_back( path );
                continue;
            }
            CORE::CString digest;
            if ( !CAssetDigest::ComputeVfsFileDigest( path, CAssetDigest::Sha256, digest ) || !CAssetDigest::AreEqual( digest, CAssetCatalogue::GetText( files[ f ], "digest" ) ) )
                report.mismatchedFiles.push_back( path );
        }
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Verified storage location " << locationId << ": " << report.filesChecked << " file(s), " <<
                      report.missingFiles.size() << " missing, " << report.mismatchedFiles.size() << " changed" );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::VerifyLocationAsync( const CORE::CString& locationId )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = GUCEF_NEW CPrimaryAssetStoreOperationData();
    data->operation = CPrimaryAssetStoreOperationData::OPERATION_VERIFY;
    data->locationId = locationId;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPrimaryAssetStore::MakeDownloadStagingPath( const CORE::CString& locationId ) const
{GUCEF_TRACE;

    const CORE::CString id = locationId.IsNULLOrEmpty() ? GetSettings().defaultLocationId : locationId;
    const CORE::CString root = GetLocationRoot( id );
    if ( root.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    const UInt64 now = CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
    return JoinVfs( root, "staging/dl" + CORE::ToString( now ) + "-" + CORE::ToString( CORE::CRandom::GetRandomUInt32( 0, 999999 ) ) );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::Ingest( const CAssetIngestRequest& request, CAssetIngestResult& result )
{GUCEF_TRACE;

    if ( !IsOpen() )
    {
        result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        result.message = "the primary store is not open";
        return result.status;
    }
    return RunIngest( request, result, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RunIngest( const CAssetIngestRequest& request, CAssetIngestResult& result, const CORE::CFutureResultSource* cancel )
{GUCEF_TRACE;

    if ( !IsOpen() )
    {
        result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        result.message = "the primary store is not open";
        return result.status;
    }
    CAssetIngestPipeline pipeline( *this, cancel );
    return pipeline.Start( request, result );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::IngestAsync( const CAssetIngestRequest& request )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = GUCEF_NEW CPrimaryAssetStoreOperationData();
    data->operation = CPrimaryAssetStoreOperationData::OPERATION_INGEST;
    data->request = request;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::ResumeIngest( const CORE::CString& ingestId, CAssetIngestResult& result )
{GUCEF_TRACE;

    if ( !IsOpen() )
    {
        result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        result.message = "the primary store is not open";
        return result.status;
    }
    return RunResumeIngest( ingestId, result, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RunResumeIngest( const CORE::CString& ingestId, CAssetIngestResult& result, const CORE::CFutureResultSource* cancel )
{GUCEF_TRACE;

    if ( !IsOpen() )
    {
        result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        result.message = "the primary store is not open";
        return result.status;
    }
    CAssetIngestPipeline pipeline( *this, cancel );
    return pipeline.Resume( ingestId, result );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::ResumeIngestAsync( const CORE::CString& ingestId )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = GUCEF_NEW CPrimaryAssetStoreOperationData();
    data->operation = CPrimaryAssetStoreOperationData::OPERATION_RESUME_INGEST;
    data->ingestId = ingestId;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::ConvertPack( const CORE::CString& packId, CAssetConversionOutcome& outcome )
{GUCEF_TRACE;

    return RunConvertPack( packId, outcome, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RunConvertPack( const CORE::CString& packId, CAssetConversionOutcome& outcome, const CORE::CFutureResultSource* cancel )
{GUCEF_TRACE;

    CAssetPackConversion conversion( *this, cancel );
    return conversion.Convert( packId, outcome );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::ConvertPackAsync( const CORE::CString& packId )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = GUCEF_NEW CPrimaryAssetStoreOperationData();
    data->operation = CPrimaryAssetStoreOperationData::OPERATION_CONVERT_PACK;
    data->packId = packId;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::ConvertPendingPacks( CAssetConversionOutcome& outcome )
{GUCEF_TRACE;

    return RunConvertPending( outcome, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RunConvertPending( CAssetConversionOutcome& outcome, const CORE::CFutureResultSource* cancel )
{GUCEF_TRACE;

    CAssetPackConversion conversion( *this, cancel );
    return conversion.ConvertPending( outcome );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::ConvertPendingPacksAsync( void )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = GUCEF_NEW CPrimaryAssetStoreOperationData();
    data->operation = CPrimaryAssetStoreOperationData::OPERATION_CONVERT_PENDING;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::GetPacksPendingConversion( CORE::CString::StringVector& packIds ) const
{GUCEF_TRACE;

    packIds.clear();
    if ( !IsOpen() )
        return;
    CORE::CDataTableQuery query;
    query.WhereEquals( "state", CORE::CVariant( CORE::CString( AssetPackStateToString( ASSETPACK_STATE_CONVERSION_PENDING ) ) ) );
    query.Select( "pack_id" );
    CAssetCatalogue::TRowVector rows;
    if ( ASSETSTORE_STATUS_OK != m_catalogue.Select( "packs", query, rows ) )
        return;
    for ( size_t r=0; r<rows.size(); ++r )
        packIds.push_back( CAssetCatalogue::GetText( rows[ r ], "pack_id" ) );
    std::sort( packIds.begin(), packIds.end() );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::IsConversionStale( const CORE::CString& packId, bool& stale, CORE::CString& reason ) const
{GUCEF_TRACE;

    stale = false;
    reason.Clear();
    CAssetPackRecord pack;
    const TAssetStoreStatus status = GetPack( packId, pack );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    CIAssetConverterHook* hook = GetConverterHook();
    if ( GUCEF_NULL == hook )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    if ( CAssetConversionRecord::OutcomeConverted != pack.conversion.outcome )
        return ASSETSTORE_STATUS_OK;
    stale = hook->IsConversionStale( pack.conversion, reason );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetStaleConversions( CORE::CString::StringVector& packIds ) const
{GUCEF_TRACE;

    packIds.clear();
    if ( !IsOpen() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    CIAssetConverterHook* hook = GetConverterHook();
    if ( GUCEF_NULL == hook )
        return ASSETSTORE_STATUS_UNSUPPORTED;
    CORE::CDataTableQuery query;
    query.WhereEquals( "outcome", CORE::CVariant( CAssetConversionRecord::OutcomeConverted ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "pack_conversions", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t r=0; r<rows.size(); ++r )
    {
        CAssetConversionRecord record;
        CAssetCatalogue::ConversionFromRow( rows[ r ], record );
        CORE::CString reason;
        if ( hook->IsConversionStale( record, reason ) )
            packIds.push_back( CAssetCatalogue::GetText( rows[ r ], "pack_id" ) );
    }
    std::sort( packIds.begin(), packIds.end() );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::GetPendingIngests( CORE::CString::StringVector& ingestIds ) const
{GUCEF_TRACE;

    ingestIds.clear();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CPrimaryAssetStoreSettings settings = GetSettings();
    for ( size_t l=0; l<settings.locations.size(); ++l )
    {
        const CORE::CString staging = JoinVfs( settings.locations[ l ].vfsRoot, "staging" );
        if ( !vfs.DirExists( staging ) )
            continue;
        VFS::CVFS::TStringVector folders;
        vfs.GetDirList( folders, staging, false, false, CORE::CString::Empty );
        for ( size_t f=0; f<folders.size(); ++f )
        {
            CORE::CString name = NormalizeVfsPath( folders[ f ] );
            const Int32 slash = name.HasChar( '/', false );
            if ( slash >= 0 )
                name = name.CutChars( (UInt32) slash + 1, true );
            if ( vfs.FileExists( JoinVfs( JoinVfs( staging, name ), CAssetIngestJob::ManifestFileName ) ) )
                ingestIds.push_back( name );
        }
    }
    std::sort( ingestIds.begin(), ingestIds.end() );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RemovePackRows( const CORE::CString& packId )
{GUCEF_TRACE;

    CORE::CString::StringVector tables;
    CAssetCatalogue::GetPackChildTables( tables );
    TAssetStoreStatus result = ASSETSTORE_STATUS_OK;
    for ( size_t t=tables.size(); t>0; --t )
    {
        const TAssetStoreStatus status = m_catalogue.DeleteWhereEquals( tables[ t-1 ], "pack_id", CORE::CVariant( packId ) );
        if ( ASSETSTORE_STATUS_OK != status )
            result = status;
    }
    const TAssetStoreStatus status = m_catalogue.DeleteWhereEquals( "packs", "pack_id", CORE::CVariant( packId ) );
    return ASSETSTORE_STATUS_OK != status ? status : result;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RepairInterruptedPacks( void )
{GUCEF_TRACE;

    CORE::CDataTableQuery query;
    query.WhereEquals( "state", CORE::CVariant( CORE::CString( AssetPackStateToString( ASSETPACK_STATE_INDEXING ) ) ) );
    CAssetCatalogue::TRowVector packs;
    TAssetStoreStatus status = m_catalogue.Select( "packs", query, packs );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t p=0; p<packs.size(); ++p )
    {
        const CORE::CString packId = CAssetCatalogue::GetText( packs[ p ], "pack_id" );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Repairing pack " << packId << ": its indexing was interrupted" );
        RemovePackRows( packId );
        CAssetCatalogue::TRowVector rows( 1, packs[ p ] );
        rows[ 0 ][ "state" ] = CORE::CVariant( CORE::CString( AssetPackStateToString( ASSETPACK_STATE_FAILED ) ) );
        rows[ 0 ][ "state_reason" ] = CORE::CVariant( CORE::CString( "the indexing was interrupted; resume the ingest" ) );
        status = m_catalogue.Write( "packs", rows );
    }
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RemovePack( const CORE::CString& packId, bool deleteFiles )
{GUCEF_TRACE;

    CAssetPackRecord pack;
    TAssetStoreStatus status = GetPack( packId, pack );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( deleteFiles && 0 == pack.rootPath.HasSubstr( "packs/", true ) )
    {
        const CORE::CString root = GetLocationRoot( pack.locationId );
        if ( !root.IsNULLOrEmpty() )
            VFS::CVfsGlobal::Instance()->GetVfs().DeleteDir( JoinVfs( root, pack.rootPath ), true );
    }
    MT::CScopeMutex indexLock( m_indexLock );
    return RemovePackRows( packId );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::AttachDownloadManager( CAssetStoreDownloadManager* downloadManager )
{GUCEF_TRACE;

    CAssetStoreDownloadManager* previous = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        previous = m_downloadManager;
        m_downloadManager = downloadManager;
    }
    if ( GUCEF_NULL != previous )
        UnsubscribeFrom( previous, CAssetStoreDownloadManager::JobFinishedEvent );
    if ( GUCEF_NULL != downloadManager )
    {
        TEventCallback callback( this, &CPrimaryAssetStore::OnDownloadJobFinished );
        SubscribeTo( downloadManager, CAssetStoreDownloadManager::JobFinishedEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::OnDownloadJobFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAssetStoreDownloadJobEventData* data = CORE::CICloneable::CloneableAs< CAssetStoreDownloadJobEventData >( eventData );
    if ( GUCEF_NULL == data || !data->job.forIngest )
        return;
    const CAssetSourcePolicy policy = GetSettings().GetPolicy( data->job.providerId );
    if ( ASSETINGEST_MODE_AUTOMATIC != policy.mode )
        return;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Ingesting download " << data->job.jobId << " of store " << data->job.providerId << " (automatic policy)" );
    IngestAsync( MakeIngestRequest( data->job ) );
}

/*-------------------------------------------------------------------------*/

CAssetIngestRequest
CPrimaryAssetStore::MakeIngestRequest( const CAssetStoreDownloadJob& job )
{GUCEF_TRACE;

    CAssetIngestRequest request;
    request.sourceId = job.providerId;
    request.itemId = job.itemId;
    request.contentPath = job.destination;
    request.downloadJobId = job.jobId;
    return request;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::EnqueueDownload( CAssetStoreDownloadManager& downloadManager, const CORE::CString& providerId, const CORE::CString& itemId,
                                     const CORE::CString& formatId, const CORE::CString::StringVector& fileIds, CORE::CString& jobId )
{GUCEF_TRACE;

    const CAssetSourcePolicy policy = GetSettings().GetPolicy( providerId );
    const CORE::CString destination = MakeDownloadStagingPath( policy.locationId );
    if ( destination.IsNULLOrEmpty() )
        return Fail( ASSETSTORE_STATUS_NOT_FOUND, "There is no storage location for downloads of " + providerId );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( destination ) && !vfs.CreateDir( destination ) )
        return Fail( ASSETSTORE_STATUS_IO_ERROR, "The staging folder " + destination + " cannot be made" );
    return downloadManager.Enqueue( providerId, itemId, formatId, fileIds, destination, true, jobId );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::Search( const CAssetSearchQuery& query, CAssetSearchResultPage& page ) const
{GUCEF_TRACE;

    if ( !IsOpen() )
        return Fail( ASSETSTORE_STATUS_INVALID_ARGUMENT, "The primary store is not open" );
    const TAssetStoreStatus status = m_searchEngine.Search( query, GetLocationStates(), page );
    if ( ASSETSTORE_STATUS_OK != status )
        return Fail( status, "The search failed: " + m_catalogue.GetLastErrorMessage() );
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::SearchAsync( const CAssetSearchQuery& query )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = GUCEF_NEW CPrimaryAssetStoreOperationData();
    data->operation = CPrimaryAssetStoreOperationData::OPERATION_SEARCH;
    data->query = query;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::Suggest( const CORE::CString& prefix, UInt32 maxCount, CORE::CString::StringVector& keywords ) const
{GUCEF_TRACE;

    if ( !IsOpen() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    return m_searchEngine.Suggest( prefix, maxCount, keywords );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetSavedSearches( TAssetSavedSearchVector& searches ) const
{GUCEF_TRACE;

    searches.clear();
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.SelectAll( "saved_searches", rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    GUCEF::vector< std::pair< std::pair< Int64, CORE::CString >, size_t > > order;
    for ( size_t r=0; r<rows.size(); ++r )
        order.push_back( std::pair< std::pair< Int64, CORE::CString >, size_t >( std::pair< Int64, CORE::CString >( CAssetCatalogue::GetInt( rows[ r ], "sort_order" ), CAssetCatalogue::GetText( rows[ r ], "search_id" ) ), r ) );
    std::sort( order.begin(), order.end() );
    CORE::CDataNodeSerializableSettings serializerSettings;
    for ( size_t o=0; o<order.size(); ++o )
    {
        const CAssetCatalogue::TRow& row = rows[ order[ o ].second ];
        CAssetSavedSearch search;
        search.searchId = CAssetCatalogue::GetText( row, "search_id" );
        search.name = CAssetCatalogue::GetText( row, "name" );
        search.precanned = CAssetCatalogue::GetBool( row, "precanned" );
        search.sortOrder = CAssetCatalogue::GetInt( row, "sort_order" );
        CORE::CDataNode queryNode;
        if ( CAssetStoreJson::FromText( CAssetCatalogue::GetText( row, "query_json" ), queryNode ) )
            search.query.Deserialize( queryNode, serializerSettings );
        searches.push_back( search );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::SaveSearch( const CAssetSavedSearch& search )
{GUCEF_TRACE;

    if ( search.searchId.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    CORE::CDataNode queryNode( "query", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNodeSerializableSettings serializerSettings;
    CORE::CString queryJson;
    if ( !search.query.Serialize( queryNode, serializerSettings ) || !CAssetStoreJson::ToText( queryNode, queryJson ) )
        return Fail( ASSETSTORE_STATUS_FAILED, "The query of the saved search cannot be written as JSON" );
    CAssetCatalogue::TRowVector rows( 1 );
    rows[ 0 ][ "search_id" ] = CORE::CVariant( search.searchId );
    rows[ 0 ][ "name" ] = CORE::CVariant( search.name );
    rows[ 0 ][ "query_json" ] = CORE::CVariant( queryJson );
    rows[ 0 ][ "precanned" ] = CORE::CVariant( search.precanned );
    rows[ 0 ][ "sort_order" ] = CORE::CVariant( search.sortOrder );
    return m_catalogue.Write( "saved_searches", rows );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::DeleteSavedSearch( const CORE::CString& searchId )
{GUCEF_TRACE;

    return m_catalogue.DeleteWhereEquals( "saved_searches", "search_id", CORE::CVariant( searchId ) );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RunSavedSearch( const CORE::CString& searchId, UInt32 pageIndex, CAssetSearchResultPage& page ) const
{GUCEF_TRACE;

    TAssetSavedSearchVector searches;
    const TAssetStoreStatus status = GetSavedSearches( searches );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t s=0; s<searches.size(); ++s )
    {
        if ( searches[ s ].searchId != searchId )
            continue;
        CAssetSearchQuery query( searches[ s ].query );
        query.pageIndex = pageIndex;
        return Search( query, page );
    }
    return ASSETSTORE_STATUS_NOT_FOUND;
}

/*-------------------------------------------------------------------------*/

CAssetPackRecord
CPrimaryAssetStore::PackFromRow( const CAssetCatalogue::TRow& row )
{GUCEF_TRACE;

    CAssetPackRecord pack;
    pack.packId = CAssetCatalogue::GetText( row, "pack_id" );
    pack.sourceId = CAssetCatalogue::GetText( row, "source_id" );
    pack.sourceItemId = CAssetCatalogue::GetText( row, "source_item_id" );
    pack.sourceVersion = CAssetCatalogue::GetText( row, "source_version" );
    pack.sourceUrl = CAssetCatalogue::GetText( row, "source_url" );
    pack.title = CAssetCatalogue::GetText( row, "title" );
    pack.seller = CAssetCatalogue::GetText( row, "seller" );
    pack.description = CAssetCatalogue::GetText( row, "description" );
    pack.sourceCategories = CAssetCatalogue::SplitLines( CAssetCatalogue::GetText( row, "source_categories" ) );
    pack.sourceTags = CAssetCatalogue::SplitLines( CAssetCatalogue::GetText( row, "source_tags" ) );
    pack.categories = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "categories" ) );
    pack.unmappedCategories = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "unmapped_categories" ) );
    pack.kinds = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "kinds" ) );
    pack.themes = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "themes" ) );
    pack.artStyles = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "art_styles" ) );
    pack.pbrRoles = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "pbr_roles" ) );
    pack.licence.licenceId = CAssetCatalogue::GetText( row, "licence_id" );
    pack.licence.licenceClass = CAssetCatalogue::GetText( row, "licence_class" );
    pack.licence.restrictions = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "licence_restrictions" ) );
    pack.formatsOriginal = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "formats_original" ) );
    pack.formatsConverted = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "formats_converted" ) );
    pack.keep = AssetKeepPolicyFromString( CAssetCatalogue::GetText( row, "keep" ) );
    pack.locationId = CAssetCatalogue::GetText( row, "location_id" );
    pack.rootPath = CAssetCatalogue::GetText( row, "root_path" );
    pack.sizeBytes = CAssetCatalogue::GetInt( row, "size_bytes" );
    pack.fileCount = CAssetCatalogue::GetInt( row, "file_count" );
    pack.assetCount = CAssetCatalogue::GetInt( row, "asset_count" );
    pack.ingestedAtMs = CAssetCatalogue::GetInt( row, "ingested_ms" );
    pack.normalizer = CAssetCatalogue::GetText( row, "normalizer" );
    pack.analyzers = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "analyzers" ) );
    pack.state = AssetPackStateFromString( CAssetCatalogue::GetText( row, "state" ) );
    pack.stateReason = CAssetCatalogue::GetText( row, "state_reason" );
    return pack;
}

/*-------------------------------------------------------------------------*/

CAssetRecord
CPrimaryAssetStore::AssetFromRow( const CAssetCatalogue::TRow& row )
{GUCEF_TRACE;

    CAssetRecord asset;
    asset.assetId = CAssetCatalogue::GetText( row, "asset_id" );
    asset.packId = CAssetCatalogue::GetText( row, "pack_id" );
    asset.kind = CAssetCatalogue::GetText( row, "kind" );
    asset.name = CAssetCatalogue::GetText( row, "name" );
    asset.originalPath = CAssetCatalogue::GetText( row, "original_path" );
    asset.convertedPath = CAssetCatalogue::GetText( row, "converted_path" );
    asset.sourceFormat = CAssetCatalogue::GetText( row, "source_format" );
    asset.convertedFormat = CAssetCatalogue::GetText( row, "converted_format" );
    asset.sizeBytes = CAssetCatalogue::GetInt( row, "size_bytes" );
    asset.digest = CAssetCatalogue::GetText( row, "digest" );
    asset.categoryId = CAssetCatalogue::GetText( row, "category_id" );
    return asset;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetPack( const CORE::CString& packId, CAssetPackRecord& pack ) const
{GUCEF_TRACE;

    CORE::CDataTableQuery query;
    query.WhereEquals( "pack_id", CORE::CVariant( packId ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "packs", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    pack = PackFromRow( rows[ 0 ] );

    // the licence record and the keywords of the pack
    CORE::CDataTableQuery licenceQuery;
    licenceQuery.WhereEquals( "licence_id", CORE::CVariant( pack.licence.licenceId ) );
    if ( ASSETSTORE_STATUS_OK == m_catalogue.Select( "licences", licenceQuery, rows ) && !rows.empty() )
    {
        pack.licence.name = CAssetCatalogue::GetText( rows[ 0 ], "name" );
        pack.licence.url = CAssetCatalogue::GetText( rows[ 0 ], "url" );
        pack.licence.text = CAssetCatalogue::GetText( rows[ 0 ], "text" );
    }
    CORE::CDataTableQuery keywordQuery;
    keywordQuery.WhereEquals( "pack_id", CORE::CVariant( packId ) );
    keywordQuery.WhereEquals( "object_type", CORE::CVariant( CORE::CString( "pack" ) ) );
    if ( ASSETSTORE_STATUS_OK == m_catalogue.Select( "keywords", keywordQuery, rows ) )
    {
        for ( size_t r=0; r<rows.size(); ++r )
        {
            CAssetKeyword keyword;
            keyword.keyword = CAssetCatalogue::GetText( rows[ r ], "keyword" );
            keyword.original = CAssetCatalogue::GetText( rows[ r ], "original" );
            keyword.field = CAssetCatalogue::GetText( rows[ r ], "field" );
            keyword.weight = CAssetCatalogue::GetInt( rows[ r ], "weight" );
            pack.keywords.push_back( keyword );
        }
    }

    // what the last conversion did
    if ( m_catalogue.GetSchemaVersion() >= 3 )
    {
        CORE::CDataTableQuery conversionQuery;
        conversionQuery.WhereEquals( "pack_id", CORE::CVariant( packId ) );
        if ( ASSETSTORE_STATUS_OK == m_catalogue.Select( "pack_conversions", conversionQuery, rows ) && !rows.empty() )
            CAssetCatalogue::ConversionFromRow( rows[ 0 ], pack.conversion );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetAssets( const CORE::CString& packId, TAssetRecordVector& assets ) const
{GUCEF_TRACE;

    assets.clear();
    CORE::CDataTableQuery query;
    query.WhereEquals( "pack_id", CORE::CVariant( packId ) );
    CAssetCatalogue::TRowVector rows;
    TAssetStoreStatus status = m_catalogue.Select( "assets", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    GUCEF::map< CORE::CString, size_t > byId;
    for ( size_t r=0; r<rows.size(); ++r )
    {
        byId[ CAssetCatalogue::GetText( rows[ r ], "asset_id" ) ] = assets.size();
        assets.push_back( AssetFromRow( rows[ r ] ) );
    }

    // the typed properties
    if ( ASSETSTORE_STATUS_OK == m_catalogue.Select( "asset_model_props", query, rows ) )
    {
        for ( size_t r=0; r<rows.size(); ++r )
        {
            GUCEF::map< CORE::CString, size_t >::const_iterator i = byId.find( CAssetCatalogue::GetText( rows[ r ], "asset_id" ) );
            if ( i == byId.end() )
                continue;
            CAssetModelProps& model = assets[ (*i).second ].model;
            model.isSet = true;
            model.triangles = CAssetCatalogue::GetInt( rows[ r ], "triangles" );
            model.vertices = CAssetCatalogue::GetInt( rows[ r ], "vertices" );
            model.subMeshes = CAssetCatalogue::GetInt( rows[ r ], "sub_meshes" );
            model.bones = CAssetCatalogue::GetInt( rows[ r ], "bones" );
            model.animations = CAssetCatalogue::GetInt( rows[ r ], "animations" );
            model.lods = CAssetCatalogue::GetInt( rows[ r ], "lods", 1 );
            model.boundsX = CAssetCatalogue::GetFloat( rows[ r ], "bounds_x" );
            model.boundsY = CAssetCatalogue::GetFloat( rows[ r ], "bounds_y" );
            model.boundsZ = CAssetCatalogue::GetFloat( rows[ r ], "bounds_z" );
            model.upAxis = CAssetCatalogue::GetText( rows[ r ], "up_axis" );
            model.unit = CAssetCatalogue::GetText( rows[ r ], "unit" );
            model.rigged = CAssetCatalogue::GetBool( rows[ r ], "rigged" );
            model.animated = CAssetCatalogue::GetBool( rows[ r ], "animated" );
            model.hasCollision = CAssetCatalogue::GetBool( rows[ r ], "has_collision" );
        }
    }
    if ( ASSETSTORE_STATUS_OK == m_catalogue.Select( "asset_texture_props", query, rows ) )
    {
        for ( size_t r=0; r<rows.size(); ++r )
        {
            GUCEF::map< CORE::CString, size_t >::const_iterator i = byId.find( CAssetCatalogue::GetText( rows[ r ], "asset_id" ) );
            if ( i == byId.end() )
                continue;
            CAssetTextureProps& texture = assets[ (*i).second ].texture;
            texture.isSet = true;
            texture.width = CAssetCatalogue::GetInt( rows[ r ], "width" );
            texture.height = CAssetCatalogue::GetInt( rows[ r ], "height" );
            texture.channels = CAssetCatalogue::GetInt( rows[ r ], "channels" );
            texture.bitDepth = CAssetCatalogue::GetInt( rows[ r ], "bit_depth" );
            texture.colourSpace = CAssetCatalogue::GetText( rows[ r ], "colour_space" );
            texture.compression = CAssetCatalogue::GetText( rows[ r ], "compression" );
            texture.role = CAssetCatalogue::GetText( rows[ r ], "role" );
        }
    }
    if ( ASSETSTORE_STATUS_OK == m_catalogue.Select( "asset_files", query, rows ) )
    {
        for ( size_t r=0; r<rows.size(); ++r )
        {
            GUCEF::map< CORE::CString, size_t >::const_iterator i = byId.find( CAssetCatalogue::GetText( rows[ r ], "asset_id" ) );
            if ( i == byId.end() )
                continue;
            CAssetFileRecord file;
            file.path = CAssetCatalogue::GetText( rows[ r ], "path" );
            file.sizeBytes = CAssetCatalogue::GetInt( rows[ r ], "size_bytes" );
            file.digest = CAssetCatalogue::GetText( rows[ r ], "digest" );
            file.role = CAssetCatalogue::GetText( rows[ r ], "role" );
            assets[ (*i).second ].files.push_back( file );
        }
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetIngestHistory( GUCEF::vector< CAssetIngestHistoryEntry >& entries, const CORE::CString& packId ) const
{GUCEF_TRACE;

    entries.clear();
    CORE::CDataTableQuery query;
    if ( !packId.IsNULLOrEmpty() )
        query.WhereEquals( "pack_id", CORE::CVariant( packId ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_catalogue.Select( "ingest_history", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    GUCEF::vector< std::pair< std::pair< Int64, CORE::CString >, size_t > > order;
    for ( size_t r=0; r<rows.size(); ++r )
        order.push_back( std::pair< std::pair< Int64, CORE::CString >, size_t >( std::pair< Int64, CORE::CString >( CAssetCatalogue::GetInt( rows[ r ], "started_ms" ), CAssetCatalogue::GetText( rows[ r ], "entry_id" ) ), r ) );
    std::sort( order.begin(), order.end() );
    for ( size_t o=0; o<order.size(); ++o )
    {
        const CAssetCatalogue::TRow& row = rows[ order[ o ].second ];
        CAssetIngestHistoryEntry entry;
        entry.entryId = CAssetCatalogue::GetText( row, "entry_id" );
        entry.ingestId = CAssetCatalogue::GetText( row, "ingest_id" );
        entry.packId = CAssetCatalogue::GetText( row, "pack_id" );
        entry.sourceId = CAssetCatalogue::GetText( row, "source_id" );
        entry.origin = CAssetCatalogue::GetText( row, "origin" );
        entry.policy = CAssetCatalogue::GetText( row, "policy" );
        entry.startedAtMs = CAssetCatalogue::GetInt( row, "started_ms" );
        entry.durationMs = CAssetCatalogue::GetInt( row, "duration_ms" );
        entry.outcome = CAssetCatalogue::GetText( row, "outcome" );
        entry.step = CAssetCatalogue::GetText( row, "step" );
        entry.problems = CAssetCatalogue::SplitLines( CAssetCatalogue::GetText( row, "problems" ) );
        entries.push_back( entry );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetAssetVfsPath( const CORE::CString& assetId, CORE::CString& vfsPath, CORE::CString& message ) const
{GUCEF_TRACE;

    CORE::CDataTableQuery query;
    query.WhereEquals( "asset_id", CORE::CVariant( assetId ) );
    CAssetCatalogue::TRowVector rows;
    TAssetStoreStatus status = m_catalogue.Select( "assets", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    CAssetPackRecord pack;
    status = GetPack( CAssetCatalogue::GetText( rows[ 0 ], "pack_id" ), pack );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    vfsPath = JoinVfs( JoinVfs( GetLocationRoot( pack.locationId ), pack.rootPath ), CAssetCatalogue::GetText( rows[ 0 ], "original_path" ) );
    if ( !IsLocationOnline( pack.locationId ) )
    {
        message = "the storage location " + pack.locationId + " that holds the pack is not reachable";
        return ASSETSTORE_STATUS_UNAVAILABLE;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetAssetConvertedVfsPath( const CORE::CString& assetId, CORE::CString& vfsPath, CORE::CString& message ) const
{GUCEF_TRACE;

    CORE::CDataTableQuery query;
    query.WhereEquals( "asset_id", CORE::CVariant( assetId ) );
    CAssetCatalogue::TRowVector rows;
    TAssetStoreStatus status = m_catalogue.Select( "assets", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    const CORE::CString convertedPath = CAssetCatalogue::GetText( rows[ 0 ], "converted_path" );
    if ( convertedPath.IsNULLOrEmpty() )
    {
        message = "the asset was not converted";
        return ASSETSTORE_STATUS_NOT_FOUND;
    }
    CAssetPackRecord pack;
    status = GetPack( CAssetCatalogue::GetText( rows[ 0 ], "pack_id" ), pack );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    vfsPath = JoinVfs( JoinVfs( GetLocationRoot( pack.locationId ), pack.rootPath ), convertedPath );
    if ( !IsLocationOnline( pack.locationId ) )
    {
        message = "the storage location " + pack.locationId + " that holds the pack is not reachable";
        return ASSETSTORE_STATUS_UNAVAILABLE;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetPacksAllowingUse( const CORE::CString& restrictionFlag, CORE::CString::StringSet& packIds ) const
{GUCEF_TRACE;

    packIds.clear();
    CORE::CDataTableQuery packQuery;
    CORE::CVariant::VariantVector states;
    states.push_back( CORE::CVariant( CORE::CString( "ingested" ) ) );
    states.push_back( CORE::CVariant( CORE::CString( "conversionPending" ) ) );
    states.push_back( CORE::CVariant( CORE::CString( "conversionRefused" ) ) );
    packQuery.WhereIn( "state", states );
    packQuery.Select( "pack_id" );
    CAssetCatalogue::TRowVector rows;
    TAssetStoreStatus status = m_catalogue.Select( "packs", packQuery, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    CORE::CDataTableQuery facetQuery;
    facetQuery.WhereEquals( "facet", CORE::CVariant( CORE::CString( "licenceRestriction" ) ) );
    facetQuery.WhereEquals( "value", CORE::CVariant( restrictionFlag ) );
    CAssetCatalogue::TRowVector restricted;
    if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.Select( "facets", facetQuery, restricted ) ) )
        return status;
    CORE::CString::StringSet excluded;
    for ( size_t r=0; r<restricted.size(); ++r )
        excluded.insert( CAssetCatalogue::GetText( restricted[ r ], "object_id" ) );
    for ( size_t r=0; r<rows.size(); ++r )
    {
        const CORE::CString packId = CAssetCatalogue::GetText( rows[ r ], "pack_id" );
        if ( excluded.end() == excluded.find( packId ) )
            packIds.insert( packId );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::IsUseAllowed( const CORE::CString& packId, const CORE::CString& restrictionFlag, bool& allowed ) const
{GUCEF_TRACE;

    allowed = false;
    CAssetPackRecord pack;
    const TAssetStoreStatus status = GetPack( packId, pack );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    allowed = !pack.licence.HasRestriction( restrictionFlag );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::ExcludeRestricted( CAssetSearchQuery& query, const CORE::CString& restrictionFlag )
{GUCEF_TRACE;

    query.excludeLicenceRestrictions.insert( restrictionFlag );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPrimaryAssetStore::GetProviderId( void ) const
{GUCEF_TRACE;

    return ProviderId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPrimaryAssetStore::GetDisplayName( void ) const
{GUCEF_TRACE;

    return "Primary store";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPrimaryAssetStore::GetIconName( void ) const
{GUCEF_TRACE;

    return "archive";
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStore::IsSignedIn( void ) const
{GUCEF_TRACE;

    return IsOpen();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::QueryLibrary( const CAssetStoreQuery& query, CAssetStoreLibraryPage& page )
{GUCEF_TRACE;

    page.Clear();
    CAssetSearchQuery search;
    search.text = query.text;
    search.prefixLastWord = true;
    search.resultKind = "packs";
    search.pageSize = query.pageSize > 0 ? query.pageSize : CAssetStoreQuery::DefaultPageSize;
    search.pageIndex = query.cursor.IsNULLOrEmpty() ? 0 : CORE::StringToInt64( query.cursor );
    search.withFacets = false;
    if ( !query.listingType.IsNULLOrEmpty() )
        search.kinds.insert( query.listingType );
    if ( !query.formatGroup.IsNULLOrEmpty() )
        search.formats.insert( query.formatGroup );
    if ( !query.sort.IsNULLOrEmpty() )
        search.sort = query.sort;
    CAssetSearchResultPage result;
    const TAssetStoreStatus status = Search( search, result );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t h=0; h<result.hits.size(); ++h )
    {
        CAssetStoreItem item;
        item.providerId = ProviderId;
        item.itemId = result.hits[ h ].packId;
        item.title = result.hits[ h ].name;
        item.seller = result.hits[ h ].seller;
        item.listingType = "pack";
        if ( !result.hits[ h ].categoryId.IsNULLOrEmpty() )
            item.categories.push_back( result.hits[ h ].categoryId );
        page.items.push_back( item );
    }
    page.hasTotal = true;
    page.total = result.total;
    if ( ( (UInt64) search.pageIndex + 1 ) * (UInt64) search.pageSize < result.total )
        page.nextCursor = CORE::ToString( (UInt64) search.pageIndex + 1 );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetItem( const CORE::CString& itemId, CAssetStoreItem& item )
{GUCEF_TRACE;

    CAssetPackRecord pack;
    TAssetStoreStatus status = GetPack( itemId, pack );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    TAssetRecordVector assets;
    if ( ASSETSTORE_STATUS_OK != ( status = GetAssets( itemId, assets ) ) )
        return status;
    item = CAssetStoreItem();
    item.providerId = ProviderId;
    item.itemId = pack.packId;
    item.version = pack.sourceVersion;
    item.title = pack.title;
    item.seller = pack.seller;
    item.listingType = "pack";
    item.url = pack.sourceUrl;
    item.description = pack.description;
    item.licenceName = pack.licence.name;
    item.licenceUrl = pack.licence.url;
    item.licenceText = pack.licence.text;
    item.licenceRestrictions = pack.licence.restrictions;
    item.categories.assign( pack.categories.begin(), pack.categories.end() );
    for ( size_t k=0; k<pack.keywords.size(); ++k )
        if ( pack.keywords[ k ].field == "tag" )
            item.tags.push_back( pack.keywords[ k ].original );
    CAssetStoreFormat original;
    original.formatId = "original";
    original.name = "Original";
    original.group = "original";
    CAssetStoreFormat converted;
    converted.formatId = "converted";
    converted.name = "Converted";
    converted.group = "converted";
    for ( size_t a=0; a<assets.size(); ++a )
    {
        // an original that went (keep: converted) is not a file of the pack any more
        const bool originalGone = assets[ a ].files.empty() && !assets[ a ].convertedPath.IsNULLOrEmpty();
        if ( !originalGone )
        {
            CAssetStoreFile file;
            file.fileId = assets[ a ].assetId;
            file.name = assets[ a ].originalPath;
            file.size = (UInt64) assets[ a ].sizeBytes;
            file.digest = assets[ a ].digest;
            file.digestAlgorithm = CAssetDigest::Sha256;
            file.isArchive = CAssetNameHints::IsArchiveExtension( CAssetNameHints::GetExtension( file.name ) );
            original.files.push_back( file );
        }
        if ( !assets[ a ].convertedPath.IsNULLOrEmpty() )
        {
            CAssetStoreFile convertedFile;
            convertedFile.fileId = assets[ a ].assetId + ":converted";
            convertedFile.name = assets[ a ].convertedPath;
            converted.files.push_back( convertedFile );
        }
    }
    item.formats.push_back( original );
    if ( !converted.files.empty() )
        item.formats.push_back( converted );
    item.storeAttributes.SetAttribute( "state", CORE::CString( AssetPackStateToString( pack.state ) ) );
    item.storeAttributes.SetAttribute( "locationId", pack.locationId );
    item.storeAttributes.SetAttribute( "rootPath", pack.rootPath );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::PlanDownload( const CORE::CString& itemId, const CORE::CString& formatId, const CORE::CString::StringVector& fileIds, CAssetStoreDownloadPlan& plan )
{GUCEF_TRACE;

    // the files of the primary store are opened through their VFS paths, not downloaded
    return ASSETSTORE_STATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::QueueOperation( CORE::CICloneable* operationData )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = static_cast< CPrimaryAssetStoreOperationData* >( operationData );
    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( GetSettings().threadPoolName );
    if ( threadPool.IsNULL() )
    {
        GUCEF_DELETE data;
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
    }
    CORE::CFutureResultSource source;
    data->store = this;
    data->source = source;
    ++m_pendingOperations;
    CORE::CFutureResult queued = threadPool->QueueTask( CPrimaryAssetStoreOperationTaskConsumer::TaskType, data, GUCEF_NULL, true );
    if ( queued.HasNoFuture() )
    {
        --m_pendingOperations;
        GUCEF_DELETE data;
        source.Fail( CORE::TASKSTATUS_QUEUEING_FAILED, "The operation could not be queued on the thread pool" );
    }
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::OperationEnded( void )
{GUCEF_TRACE;

    --m_pendingOperations;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::NotifyIngestEvent( const CORE::CEvent& eventId, const CORE::CString& ingestId, const CORE::CString& packId,
                                       TAssetIngestStep step, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    CAssetIngestEventData eventData;
    eventData.ingestId = ingestId;
    eventData.packId = packId;
    eventData.step = AssetIngestStepToString( step );
    eventData.status = AssetStoreStatusToString( status );
    eventData.message = message;
    NotifyObserversFromThread( eventId, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::NotifyConversionEvent( const CORE::CEvent& eventId, const CORE::CString& jobId, const CORE::CString& packId, bool forIngest,
                                           UInt32 done, UInt32 total, const CORE::CString& assetPath, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    CAssetConversionEventData eventData;
    eventData.jobId = jobId;
    eventData.packId = packId;
    eventData.forIngest = forIngest;
    eventData.done = done;
    eventData.total = total;
    eventData.assetPath = assetPath;
    eventData.status = AssetStoreStatusToString( status );
    eventData.message = message;
    NotifyObserversFromThread( eventId, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStore::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    CORE::CDataNode* node = config.FindOrAddChild( "primaryStore" );
    if ( GUCEF_NULL == node )
        return false;
    CORE::CDataNodeSerializableSettings serializerSettings;
    return GetSettings().Serialize( *node, serializerSettings );
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStore::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CORE::CDataNode* node = config.Find( "primaryStore" );
    if ( GUCEF_NULL == node )
        node = &config;
    CPrimaryAssetStoreSettings settings;
    CORE::CDataNodeSerializableSettings serializerSettings;
    if ( !settings.Deserialize( *node, serializerSettings ) )
        return false;
    SetSettings( settings );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPrimaryAssetStore::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CPrimaryAssetStore";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
