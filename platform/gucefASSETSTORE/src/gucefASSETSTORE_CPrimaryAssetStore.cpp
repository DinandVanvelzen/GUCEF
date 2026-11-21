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
#include "gucefASSETSTORE_CAssetPackIndexLoader.h"
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
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H
#include "gucefASSETSTORE_CAssetLibrarySyncAdmission.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCADMISSION_H ? */

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
const CORE::CString CPrimaryAssetStore::ScratchStagingVfsRoot = "assetstore-scratch-staging";
const CORE::CEvent CPrimaryAssetStore::IngestProgressEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::IngestProgressEvent";
const CORE::CEvent CPrimaryAssetStore::IngestFinishedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::IngestFinishedEvent";
const CORE::CEvent CPrimaryAssetStore::IngestFailedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::IngestFailedEvent";
const CORE::CEvent CPrimaryAssetStore::ConversionProgressEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::ConversionProgressEvent";
const CORE::CEvent CPrimaryAssetStore::ConversionFinishedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::ConversionFinishedEvent";
const CORE::CEvent CPrimaryAssetStore::ConversionFailedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::ConversionFailedEvent";
const CORE::CEvent CPrimaryAssetStore::LocationStateChangedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::LocationStateChangedEvent";
const CORE::CEvent CPrimaryAssetStore::PackIndexUpdatedEvent = "GUCEF::ASSETSTORE::CPrimaryAssetStore::PackIndexUpdatedEvent";

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

const char* const ScratchStagingMountName = "assetstore-scratch-staging";

/** The room a scratch staging area must have beyond the download: a download whose size is not known counts as this much */
const UInt64 ScratchStagingMarginBytes = 64ULL * 1024ULL * 1024ULL;
const UInt64 ScratchStagingUnknownSizeBytes = 1024ULL * 1024ULL * 1024ULL;

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
    PackIndexUpdatedEvent.Initialize();
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
    , m_syncStore( m_catalogue )
    , m_locationOnline()
    , m_mountedLocations()
    , m_pendingOperations( 0 )
    , m_lastError()
    , m_scratchMounted( false )
    , m_scratchWarned( false )
    , m_lock()
    , m_indexLock()
    , m_folderLock()
    , m_reservedFolders()
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

CORE::CString
CPrimaryAssetStore::ChoosePackFolder( const CORE::CString& locationId, const CORE::CString& sourceName, const CORE::CString& packId )
{GUCEF_TRACE;

    const CORE::CString clean = CPackStorageLayout::MakePackFolderName( sourceName );
    MT::CScopeMutex lock( m_folderLock );

    // the folders of the other packs of the location: in the catalogue and reserved by ingests that did not write their row yet
    CORE::CString::StringSet taken;
    CORE::CDataTableQuery query;
    query.WhereEquals( "location_id", CORE::CVariant( locationId ) );
    CAssetCatalogue::TRowVector rows;
    if ( ASSETSTORE_STATUS_OK == m_catalogue.Select( "packs", query, rows ) )
    {
        for ( size_t r=0; r<rows.size(); ++r )
        {
            const CPackStorageLayout layout( CAssetCatalogue::GetText( rows[ r ], "root_path" ) );
            if ( !layout.IsAreaLayout() )
                continue;
            if ( CAssetCatalogue::GetText( rows[ r ], "pack_id" ) == packId )
                return layout.GetPackFolder();      // placed before (an ingest that is resumed): the same folder
            taken.insert( layout.GetPackFolder().Lowercase() );
        }
    }
    const CORE::CString prefix = locationId + "\n";
    for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator i=m_reservedFolders.begin(); i!=m_reservedFolders.end(); ++i )
    {
        if ( (*i).first != packId && 0 == (*i).second.HasSubstr( prefix, true ) )
            taken.insert( (*i).second.CutChars( prefix.Length(), true ) );
    }
    const CORE::CString folder = CPackStorageLayout::ChoosePackFolderName( clean, packId, taken );
    m_reservedFolders[ packId ] = prefix + folder.Lowercase();
    return folder;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::ReleasePackFolder( const CORE::CString& packId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_folderLock );
    m_reservedFolders.erase( packId );
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
        RepairInterruptedIndexes();
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Opened, catalogue schema version " << m_catalogue.GetSchemaVersion() );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::Close( void )
{GUCEF_TRACE;

    // the operations that run (ingests, conversions, migrations) end before the catalogue and the locations go: an ingest that
    // ran on after the close wrote into a closed catalogue and kept files and tables of the location busy for the next owner
    while ( m_pendingOperations.Load() > 0 )
        MT::ThreadDelay( 2 );

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
    bool scratchMounted = false;
    {
        MT::CScopeMutex lock( m_lock );
        scratchMounted = m_scratchMounted;
        m_scratchMounted = false;
        m_scratchWarned = false;
    }
    if ( scratchMounted )
        vfs.UnmountArchiveByName( ScratchStagingMountName );
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
        const CPackStorageLayout layout( CAssetCatalogue::GetText( packs[ p ], "root_path" ) );
        CAssetCatalogue::TRowVector files;
        CORE::CDataTableQuery fileQuery;
        fileQuery.WhereEquals( "pack_id", CORE::CVariant( CAssetCatalogue::GetText( packs[ p ], "pack_id" ) ) );
        if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.Select( "asset_files", fileQuery, files ) ) )
            return report.status = status;
        for ( size_t f=0; f<files.size(); ++f )
        {
            const CORE::CString path = layout.Resolve( root, CAssetCatalogue::GetText( files[ f ], "path" ) );
            const CORE::CString lower = path.Lowercase();
            if ( lower.HasSubstr( ".zip/", true ) >= 0 || lower.HasSubstr( ".7z/", true ) >= 0 || lower.HasSubstr( ".rar/", true ) >= 0 || lower.HasSubstr( ".unitypackage/", true ) >= 0 || lower.HasSubstr( ".mhpkg/", true ) >= 0 )
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

TAssetStoreStatus
CPrimaryAssetStore::MigrateStorageLayout( const CORE::CString& locationId, bool dryRun, CAssetLayoutMigrationReport& report )
{GUCEF_TRACE;

    report = CAssetLayoutMigrationReport();
    report.locationId = locationId;
    report.dryRun = dryRun;
    if ( !IsOpen() )
        return report.status = Fail( ASSETSTORE_STATUS_INVALID_ARGUMENT, "The primary store is not open" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    MT::CScopeMutex indexLock( m_indexLock );
    CORE::CDataTableQuery query;
    if ( !locationId.IsNULLOrEmpty() )
        query.WhereEquals( "location_id", CORE::CVariant( locationId ) );
    CAssetCatalogue::TRowVector packs;
    TAssetStoreStatus status = m_catalogue.Select( "packs", query, packs );
    if ( ASSETSTORE_STATUS_OK != status )
        return report.status = status;

    CORE::CString::StringVector reserved;
    for ( size_t p=0; p<packs.size(); ++p )
    {
        const CORE::CString packId = CAssetCatalogue::GetText( packs[ p ], "pack_id" );
        const CORE::CString rootPath = CAssetCatalogue::GetText( packs[ p ], "root_path" );
        const CPackStorageLayout layout( rootPath );
        if ( layout.IsAreaLayout() || 0 != layout.GetLegacyRoot().HasSubstr( "packs/", true ) )
            continue;       // in the new layout already, or referenced in place: not the store's to move
        ++report.packsInOldLayout;

        const CORE::CString packLocation = CAssetCatalogue::GetText( packs[ p ], "location_id" );
        const CORE::CString root = GetLocationRoot( packLocation );
        if ( root.IsNULLOrEmpty() || !IsLocationOnline( packLocation ) )
        {
            ++report.packsRefused;
            report.problems.push_back( packId + ": the storage location " + packLocation + " is not reachable" );
            continue;
        }
        const CORE::CString oldRoot = CPackStorageLayout::Join( root, layout.GetLegacyRoot() );
        const CORE::CString oldOriginal = CPackStorageLayout::Join( oldRoot, "original" );
        const CORE::CString oldConverted = CPackStorageLayout::Join( oldRoot, "converted" );
        const CORE::CString leaf = layout.GetLegacyRoot().ReplaceChar( '\\', '/' );
        const CORE::CString::StringVector parts = leaf.ParseElements( '/', false );
        const CORE::CString folder = ChoosePackFolder( packLocation, parts.empty() ? packId : parts.back(), packId );
        reserved.push_back( packId );
        const CPackStorageLayout target( CPackStorageLayout::MakeRootPath( folder ) );
        const CORE::CString newOriginal = target.GetOriginalFolder( root );
        const CORE::CString newIntermediate = target.GetIntermediateFolder( root, CORE::CString::Empty );
        const CORE::CString newConverted = CPackStorageLayout::Join( newIntermediate, "converted" );

        const bool originalToMove = vfs.DirExists( oldOriginal );
        const bool convertedToMove = vfs.DirExists( oldConverted );
        if ( ( originalToMove && vfs.DirExists( newOriginal ) ) || ( convertedToMove && vfs.DirExists( newConverted ) ) )
        {
            ++report.packsRefused;
            report.problems.push_back( packId + ": a target folder exists already (" + folder + "), nothing was moved" );
            continue;
        }
        report.actions.push_back( packId + ": " + layout.GetLegacyRoot() + " -> " + target.GetStoredRootPath() );
        if ( dryRun )
        {
            ++report.packsMigrated;
            continue;
        }
        bool moved = true;
        if ( originalToMove )
        {
            vfs.CreateDir( CPackStorageLayout::Join( root, CPackStorageLayout::OriginalArea ) );
            moved = vfs.MoveDir( oldOriginal, newOriginal );
        }
        if ( moved && convertedToMove )
        {
            vfs.CreateDir( newIntermediate );
            moved = vfs.MoveDir( oldConverted, newConverted );
        }
        if ( !moved )
        {
            ++report.packsRefused;
            report.problems.push_back( packId + ": the folders could not be moved" );
            continue;
        }
        CAssetCatalogue::TRowVector rows( 1, packs[ p ] );
        rows[ 0 ][ "root_path" ] = CORE::CVariant( target.GetStoredRootPath() );
        if ( ASSETSTORE_STATUS_OK != ( status = m_catalogue.Write( "packs", rows ) ) )
        {
            report.problems.push_back( packId + ": the row of the pack cannot be written, run the migration again" );
            ++report.packsRefused;
            continue;
        }

        // the old folder goes when nothing is left in it
        VFS::CVFS::TStringVector left;
        vfs.GetFileList( left, oldRoot, true, true, CORE::CString::Empty, 1 );
        if ( left.empty() )
            vfs.DeleteDir( oldRoot, true );
        ++report.packsMigrated;
    }
    for ( size_t r=0; r<reserved.size(); ++r )
        ReleasePackFolder( reserved[ r ] );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Storage layout migration" << ( dryRun ? " (dry run)" : "" ) << ": " << report.packsInOldLayout <<
                      " pack(s) in the old layout, " << report.packsMigrated << " moved, " << report.packsRefused << " refused" );
    return report.status = ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::MigrateStorageLayoutAsync( const CORE::CString& locationId, bool dryRun )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = GUCEF_NEW CPrimaryAssetStoreOperationData();
    data->operation = CPrimaryAssetStoreOperationData::OPERATION_MIGRATE_LAYOUT;
    data->locationId = locationId;
    data->dryRun = dryRun;
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

CORE::CString
CPrimaryAssetStore::ChooseDownloadStagingPath( const CORE::CString& locationId, UInt64 expectedBytes )
{GUCEF_TRACE;

    const CORE::CString scratch = GetSettings().scratchStagingPath;
    if ( scratch.IsNULLOrEmpty() )
        return MakeDownloadStagingPath( locationId );

    // the scratch area is a preference: whatever stands in its way sends the download to the staging of its location
    CORE::CString why;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString folder = CORE::RelativePath( CORE::ResolveVars( scratch ) );
    bool mounted = false;
    {
        MT::CScopeMutex lock( m_lock );
        mounted = m_scratchMounted;
    }
    if ( !mounted )
    {
        if ( !CORE::DirExists( folder ) )
            CORE::CreateDirs( folder );
        if ( !CORE::DirExists( folder ) )
            why = "the folder " + folder + " cannot be made";
        else
        if ( !vfs.IsMountedArchive( ScratchStagingMountName ) && !vfs.AddRoot( folder, ScratchStagingMountName, true, false, ScratchStagingVfsRoot ) )
            why = "the folder " + folder + " cannot be mounted";
        else
        {
            MT::CScopeMutex lock( m_lock );
            m_scratchMounted = true;
        }
    }
    UInt64 freeBytes = 0;
    UInt64 totalBytes = 0;
    const UInt64 needed = ( 0 == expectedBytes ? ScratchStagingUnknownSizeBytes : expectedBytes ) + ScratchStagingMarginBytes;
    if ( why.IsNULLOrEmpty() && !CAssetLibrarySyncAdmission::ReadVolume( folder, freeBytes, totalBytes ) )
        why = "the free space of " + folder + " cannot be read";
    else
    if ( why.IsNULLOrEmpty() && freeBytes < needed )
        why = "it has " + CORE::ToString( freeBytes ) + " bytes free, the download needs " + CORE::ToString( needed );
    CORE::CString path;
    if ( why.IsNULLOrEmpty() )
    {
        const UInt64 now = CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
        path = JoinVfs( ScratchStagingVfsRoot, "dl" + CORE::ToString( now ) + "-" + CORE::ToString( CORE::CRandom::GetRandomUInt32( 0, 999999 ) ) );
        // writable: the folder of the download can be made there
        if ( !vfs.CreateDir( path ) || !vfs.DirExists( path ) )
        {
            why = "a folder cannot be written in " + folder;
            path.Clear();
        }
    }
    if ( !why.IsNULLOrEmpty() )
    {
        bool warn = false;
        {
            MT::CScopeMutex lock( m_lock );
            warn = !m_scratchWarned;
            m_scratchWarned = true;
        }
        if ( warn )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: The scratch staging area is not used for a download (" << why << "): it is staged in its storage location; this is said once" );
        return MakeDownloadStagingPath( locationId );
    }
    return path;
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

CAssetSyncMarkStore&
CPrimaryAssetStore::GetSyncStore( void )
{GUCEF_TRACE;

    return m_syncStore;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::ConfirmSourceRevision( const CORE::CString& packId, const CORE::CString& revision, Int64 changedMs )
{GUCEF_TRACE;

    if ( !IsOpen() )
        return Fail( ASSETSTORE_STATUS_INVALID_ARGUMENT, "The primary store is not open" );
    CAssetPackSourceRecord record;
    TAssetStoreStatus status = m_syncStore.GetPackSource( packId, record );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    record.revision = revision;
    if ( changedMs > record.changedMs )
        record.changedMs = changedMs;
    record.syncedMs = (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
    status = m_syncStore.WritePackSource( record );
    if ( ASSETSTORE_STATUS_OK == status )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Confirmed the revision " << revision << " of pack " << packId );
    }
    return status;
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
    if ( m_catalogue.GetSchemaVersion() >= 4 )
    {
        // the rows of the pack index, a derived copy of its document
        CORE::CString::StringVector indexTables;
        CAssetCatalogue::GetPackIndexTables( indexTables );
        for ( size_t t=0; t<indexTables.size(); ++t )
        {
            const TAssetStoreStatus status = m_catalogue.DeleteWhereEquals( indexTables[ t ], "pack_id", CORE::CVariant( packId ) );
            if ( ASSETSTORE_STATUS_OK != status )
                result = status;
        }
    }
    if ( m_catalogue.GetSchemaVersion() >= 5 )
    {
        // the store facts of the pack (the library sync)
        CORE::CString::StringVector sourceTables;
        CAssetCatalogue::GetPackSourceTables( sourceTables );
        for ( size_t t=0; t<sourceTables.size(); ++t )
        {
            const TAssetStoreStatus status = m_catalogue.DeleteWhereEquals( sourceTables[ t ], "pack_id", CORE::CVariant( packId ) );
            if ( ASSETSTORE_STATUS_OK != status )
                result = status;
        }
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
    if ( deleteFiles )
    {
        // only folders that the store made: the folders of a pack that is referenced in place are not its own
        const CORE::CString root = GetLocationRoot( pack.locationId );
        CORE::CString::StringVector folders;
        CPackStorageLayout( pack.rootPath ).GetOwnedFolders( root, folders );
        for ( size_t f=0; f<folders.size() && !root.IsNULLOrEmpty(); ++f )
            VFS::CVfsGlobal::Instance()->GetVfs().DeleteDir( folders[ f ], true );
    }
    MT::CScopeMutex indexLock( m_indexLock );
    return RemovePackRows( packId );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE PACK INDEX                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The name of the last folder of a path of a folder listing, which may end with a separator */
CORE::CString
LastFolderName( const CORE::CString& path )
{
    CORE::CString text = path.ReplaceChar( '\\', '/' );
    while ( text.Length() > 0 && '/' == text.C_String()[ text.Length() - 1 ] )
        text = text.SubstrToIndex( text.Length() - 1, true );
    return text.SubstrToChar( '/', false );
}

/** The observer of the indexing of a pack: the cancel request of the job's future, and the observer of the caller */
class CIndexPackObserver : public CIAssetConversionObserver
{
    public:

    CIndexPackObserver( const CORE::CFutureResultSource* cancel, CIAssetConversionObserver* inner )
        : CIAssetConversionObserver()
        , m_cancel( cancel )
        , m_inner( inner )
    {

    }

    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return ( GUCEF_NULL != m_cancel && m_cancel->IsCancellationRequested() ) || ( GUCEF_NULL != m_inner && m_inner->IsCancelRequested() );
    }

    virtual void OnConversionProgress( UInt32 done, UInt32 total, const CORE::CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_inner )
            m_inner->OnConversionProgress( done, total, assetPath );
    }

    private:

    const CORE::CFutureResultSource* m_cancel;
    CIAssetConversionObserver* m_inner;
};

}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::NotifyPackIndexUpdated( const CORE::CString& packId )
{GUCEF_TRACE;

    CORE::TCloneableString eventData( packId );
    NotifyObserversFromThread( PackIndexUpdatedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::LoadPackIndexDocument( const CORE::CString& packId, const CORE::CString& manifestPath, bool force, CAssetPackIndexOutcome& outcome )
{GUCEF_TRACE;

    MT::CScopeMutex indexLock( m_indexLock );
    CAssetPackIndexLoader loader( m_catalogue, &m_vocabulary.GetKeywordNormalizer() );
    return loader.Load( packId, manifestPath, force, outcome );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::FindPackIndexPath( const CAssetPackRecord& pack, CORE::CString& indexPath ) const
{GUCEF_TRACE;

    indexPath.Clear();
    const CORE::CString root = GetLocationRoot( pack.locationId );
    if ( root.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_UNAVAILABLE;
    const CPackStorageLayout layout( pack.rootPath );
    CIAssetConverterHook* hook = GetConverterHook();
    const CORE::CString profile = GUCEF_NULL != hook ? hook->GetProfileName() : CORE::CString::Empty;
    indexPath = layout.GetPackIndexPath( root, profile );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !layout.IsAreaLayout() && !vfs.FileExists( indexPath ) )
    {
        // a pack of the earlier layout: the index is in the folder of the profile that converted it
        const CORE::CString converted = CPackStorageLayout::Join( CPackStorageLayout::Join( root, layout.GetLegacyRoot() ), "converted" );
        VFS::CVFS::TStringVector folders;
        vfs.GetDirList( folders, converted, false, false, CORE::CString::Empty );
        for ( size_t f=0; f<folders.size(); ++f )
        {
            const CORE::CString candidate = CPackStorageLayout::Join( CPackStorageLayout::Join( converted, LastFolderName( folders[ f ] ) ), "pack-index.json" );
            if ( vfs.FileExists( candidate ) )
            {
                indexPath = candidate;
                break;
            }
        }
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::IndexPackRecord( const CAssetPackRecord& pack, const CAssetPackIndexOptions& options, CAssetPackIndexOutcome& outcome, const CORE::CFutureResultSource* cancel )
{GUCEF_TRACE;

    const CORE::CString packId = pack.packId;
    outcome = CAssetPackIndexOutcome();
    outcome.packId = packId;
    const UInt64 startedMs = (UInt64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
    if ( !m_catalogue.IsOpen() || m_catalogue.GetSchemaVersion() < 4 )
    {
        outcome.message = "the catalogue has no pack index tables (schema version 4)";
        return outcome.status = ASSETSTORE_STATUS_UNSUPPORTED;
    }
    if ( !AssetPackStateIsIngested( pack.state ) )
    {
        outcome.message = "the pack is not ingested (its state is " + CORE::CString( AssetPackStateToString( pack.state ) ) + ")";
        return outcome.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    if ( !IsLocationOnline( pack.locationId ) )
    {
        outcome.message = "the storage location " + pack.locationId + " that holds the pack is not reachable";
        return outcome.status = ASSETSTORE_STATUS_UNAVAILABLE;
    }
    CORE::CString indexPath;
    TAssetStoreStatus status = FindPackIndexPath( pack, indexPath );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        outcome.message = "the folder of the pack index is not known";
        return outcome.status = status;
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const bool redo = options.reindex || options.reanalyse || ( options.verify && GUCEF_NULL != GetConverterHook() );
    const bool exists = vfs.FileExists( indexPath );

    // work once: the document that is there and fresh is loaded, nothing is built
    CORE::CString loadProblem;
    if ( exists && !redo )
    {
        CAssetPackIndexOutcome loaded;
        status = LoadPackIndexDocument( packId, indexPath, options.force, loaded );
        if ( ASSETSTORE_STATUS_OK == status )
        {
            CAssetPackIndexInfo info;
            GetPackIndexInfo( packId, false, info );
            if ( !info.stale || GUCEF_NULL == GetConverterHook() || !options.allowBuild )
            {
                outcome = loaded;
                outcome.reused = true;
                outcome.durationMs = (UInt32) ( (UInt64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() - startedMs );
                if ( outcome.loaded && options.notify )
                    NotifyPackIndexUpdated( packId );
                return outcome.status;
            }
            loadProblem = info.stateReason;
        }
        else
            loadProblem = loaded.message;
    }

    // the converter hook builds it (a document that is stale, damaged or not there)
    if ( !options.allowBuild )
    {
        outcome.message = "there is no pack index document of the pack" + ( loadProblem.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + loadProblem + ")" );
        return outcome.status = ASSETSTORE_STATUS_UNSUPPORTED;
    }
    CIAssetConverterHook* hook = GetConverterHook();
    if ( GUCEF_NULL == hook )
    {
        outcome.message = "there is no pack index of the pack and no converter is attached to the store to build one" + ( loadProblem.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + loadProblem + ")" );
        return outcome.status = ASSETSTORE_STATUS_UNSUPPORTED;
    }
    const CPackStorageLayout layout( pack.rootPath );
    const CORE::CString root = GetLocationRoot( pack.locationId );
    const CORE::CString profile = hook->GetProfileName();
    CAssetPackIndexRequest request;
    request.pack = pack;
    request.sourceVfsPath = layout.GetOriginalFolder( root );
    request.sourcePrefix = layout.GetOriginalPrefix() + "/";
    request.intermediateVfsFolder = layout.GetIntermediateFolder( root, profile );
    if ( layout.IsAreaLayout() )
        request.cookedVfsFolder = layout.GetCookedFolder( root, profile );
    request.reindex = options.reindex;
    request.reanalyse = options.reanalyse;
    request.analyse = options.analyse;
    request.derive = options.derive || !vfs.DirExists( request.sourceVfsPath );
    if ( request.derive )
    {
        status = GetAssets( packId, request.assets );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            outcome.message = "the assets of the pack cannot be read: " + GetLastErrorMessage();
            return outcome.status = status;
        }
        // the relations the catalogue has, so a derived document keeps them (pack index design 2.12)
        CAssetCatalogue::TRowVector relationRows;
        CORE::CDataTableQuery relationQuery;
        relationQuery.WhereEquals( "pack_id", CORE::CVariant( packId ) );
        if ( ASSETSTORE_STATUS_OK == m_catalogue.Select( "relations", relationQuery, relationRows ) )
        {
            for ( size_t r=0; r<relationRows.size(); ++r )
            {
                CAssetRelationRecord relation;
                relation.fromAssetId = CAssetCatalogue::GetText( relationRows[ r ], "from_asset" );
                relation.kind = CAssetCatalogue::GetText( relationRows[ r ], "kind" );
                relation.toAssetId = CAssetCatalogue::GetText( relationRows[ r ], "to_asset" );
                request.relations.push_back( relation );
            }
        }
    }
    CIndexPackObserver observer( cancel, options.observer );
    request.observer = &observer;
    CAssetPackIndexOutcome built;
    TAssetStoreStatus hookStatus = ASSETSTORE_STATUS_FAILED;
    try
    {
        hookStatus = hook->BuildPackIndex( request, built );
    }
    catch ( ... )
    {
        hookStatus = ASSETSTORE_STATUS_FAILED;
        built.message = "an exception ended the indexing";
    }
    if ( ASSETSTORE_STATUS_OK != hookStatus )
    {
        outcome = built;
        outcome.packId = packId;
        outcome.status = hookStatus;
        if ( outcome.message.IsNULLOrEmpty() )
            outcome.message = "the converter could not index the pack";
        return outcome.status;
    }
    const CORE::CString documentPath = built.documentPath.IsNULLOrEmpty() ? indexPath : built.documentPath;
    CAssetPackIndexOutcome loaded;
    status = LoadPackIndexDocument( packId, documentPath, options.force || built.built, loaded );
    outcome = loaded;
    outcome.packId = packId;
    outcome.reused = built.reused;
    outcome.built = built.built;
    outcome.derived = built.derived;
    outcome.locked = built.locked;
    outcome.durationMs = (UInt32) ( (UInt64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() - startedMs );
    if ( ASSETSTORE_STATUS_OK == status && outcome.loaded && options.notify )
        NotifyPackIndexUpdated( packId );
    return outcome.status = status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::IndexPack( const CORE::CString& packId, const CAssetPackIndexOptions& options, CAssetPackIndexOutcome& outcome )
{GUCEF_TRACE;

    return RunIndexPack( packId, options, outcome, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RunIndexPack( const CORE::CString& packId, const CAssetPackIndexOptions& options, CAssetPackIndexOutcome& outcome, const CORE::CFutureResultSource* cancel )
{GUCEF_TRACE;

    outcome = CAssetPackIndexOutcome();
    outcome.packId = packId;
    CAssetPackRecord pack;
    const TAssetStoreStatus status = GetPack( packId, pack );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        outcome.message = "there is no pack " + packId + " in the catalogue";
        return outcome.status = status;
    }
    return IndexPackRecord( pack, options, outcome, cancel );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::IndexPackAsync( const CORE::CString& packId, const CAssetPackIndexOptions& options )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = GUCEF_NEW CPrimaryAssetStoreOperationData();
    data->operation = CPrimaryAssetStoreOperationData::OPERATION_INDEX_PACK;
    data->packId = packId;
    data->indexOptions = options;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

namespace {

/** Counts what an outcome of the indexing of one pack says into the report of a run over many */
void
CountOutcome( const CAssetPackIndexOutcome& outcome, const CORE::CString& packId, CAssetPackIndexReport& report )
{GUCEF_TRACE;

    ++report.packsSeen;
    if ( ASSETSTORE_STATUS_OK == outcome.status )
    {
        if ( outcome.derived )
            ++report.packsDerived;
        else
        if ( outcome.built )
            ++report.packsBuilt;
        else
        if ( outcome.loaded )
            ++report.packsLoaded;
        else
            ++report.packsUnchanged;
        return;
    }
    if ( ASSETSTORE_STATUS_UNSUPPORTED == outcome.status || ASSETSTORE_STATUS_UNAVAILABLE == outcome.status )
        ++report.packsSkipped;
    else
        ++report.packsFailed;
    if ( report.problems.size() < 50 )
        report.problems.push_back( packId + ": " + outcome.message );
}

}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::IndexPacksWithoutIndex( const CAssetPackIndexOptions& options, CAssetPackIndexReport& report )
{GUCEF_TRACE;

    return RunIndexPacksWithoutIndex( options, report, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RunIndexPacksWithoutIndex( const CAssetPackIndexOptions& options, CAssetPackIndexReport& report, const CORE::CFutureResultSource* cancel )
{GUCEF_TRACE;

    report = CAssetPackIndexReport();
    if ( !m_catalogue.IsOpen() || m_catalogue.GetSchemaVersion() < 4 )
    {
        report.message = "the catalogue has no pack index tables (schema version 4)";
        return report.status = ASSETSTORE_STATUS_UNSUPPORTED;
    }
    CORE::CString::StringVector packIds;
    {
        MT::CScopeMutex indexLock( m_indexLock );
        CAssetPackIndexLoader loader( m_catalogue, &m_vocabulary.GetKeywordNormalizer() );
        const TAssetStoreStatus status = loader.GetPacksWithoutIndex( packIds );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            report.message = "the packs without an index cannot be listed: " + m_catalogue.GetLastErrorMessage();
            return report.status = status;
        }
    }
    for ( size_t p=0; p<packIds.size(); ++p )
    {
        if ( GUCEF_NULL != cancel && cancel->IsCancellationRequested() )
        {
            report.message = "cancelled";
            return report.status = ASSETSTORE_STATUS_CANCELLED;
        }
        if ( GUCEF_NULL != options.observer )
            options.observer->OnConversionProgress( (UInt32) p, (UInt32) packIds.size(), packIds[ p ] );
        CAssetPackIndexOutcome outcome;
        RunIndexPack( packIds[ p ], options, outcome, cancel );
        if ( ASSETSTORE_STATUS_CANCELLED == outcome.status )
        {
            report.message = "cancelled";
            return report.status = ASSETSTORE_STATUS_CANCELLED;
        }
        CountOutcome( outcome, packIds[ p ], report );
    }
    report.message = report.ToSummaryText();
    return report.status = ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::IndexPacksWithoutIndexAsync( const CAssetPackIndexOptions& options )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = GUCEF_NEW CPrimaryAssetStoreOperationData();
    data->operation = CPrimaryAssetStoreOperationData::OPERATION_INDEX_MISSING;
    data->indexOptions = options;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::ReindexFromStorage( bool buildMissing, const CAssetPackIndexOptions& options, CAssetPackIndexReport& report )
{GUCEF_TRACE;

    return RunReindexFromStorage( buildMissing, options, report, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::RunReindexFromStorage( bool buildMissing, const CAssetPackIndexOptions& options, CAssetPackIndexReport& report, const CORE::CFutureResultSource* cancel )
{GUCEF_TRACE;

    report = CAssetPackIndexReport();
    if ( !m_catalogue.IsOpen() || m_catalogue.GetSchemaVersion() < 4 )
    {
        report.message = "the catalogue has no pack index tables (schema version 4)";
        return report.status = ASSETSTORE_STATUS_UNSUPPORTED;
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDataTableQuery query;
    query.Select( "pack_id" );
    CAssetCatalogue::TRowVector rows;
    TAssetStoreStatus status = m_catalogue.Select( "packs", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        report.message = "the packs cannot be listed: " + m_catalogue.GetLastErrorMessage();
        return report.status = status;
    }
    CORE::CString::StringSet ownedFolders;      // per location, as "location id and a line feed and the folder"
    for ( size_t r=0; r<rows.size(); ++r )
    {
        if ( GUCEF_NULL != cancel && cancel->IsCancellationRequested() )
        {
            report.message = "cancelled";
            return report.status = ASSETSTORE_STATUS_CANCELLED;
        }
        const CORE::CString packId = CAssetCatalogue::GetText( rows[ r ], "pack_id" );
        CAssetPackRecord pack;
        if ( ASSETSTORE_STATUS_OK != GetPack( packId, pack ) )
            continue;
        const CPackStorageLayout layout( pack.rootPath );
        if ( layout.IsAreaLayout() )
            ownedFolders.insert( pack.locationId + "\n" + layout.GetPackFolder().Lowercase() );
        if ( GUCEF_NULL != options.observer )
            options.observer->OnConversionProgress( (UInt32) r, (UInt32) rows.size(), packId );
        CAssetPackIndexOutcome outcome;
        CORE::CString indexPath;
        if ( AssetPackStateIsIngested( pack.state ) && IsLocationOnline( pack.locationId ) && ASSETSTORE_STATUS_OK == FindPackIndexPath( pack, indexPath ) && vfs.FileExists( indexPath ) )
        {
            // the document is the primary store: its rows are written from it again
            outcome.packId = packId;
            status = LoadPackIndexDocument( packId, indexPath, options.force, outcome );
            outcome.status = status;
            if ( ASSETSTORE_STATUS_OK == status && outcome.loaded && options.notify )
                NotifyPackIndexUpdated( packId );
        }
        else
        if ( buildMissing && AssetPackStateIsIngested( pack.state ) )
        {
            RunIndexPack( packId, options, outcome, cancel );
            if ( ASSETSTORE_STATUS_CANCELLED == outcome.status )
            {
                report.message = "cancelled";
                return report.status = ASSETSTORE_STATUS_CANCELLED;
            }
        }
        else
        {
            outcome.packId = packId;
            outcome.status = ASSETSTORE_STATUS_UNSUPPORTED;
            outcome.message = "the pack has no pack index document in the storage";
        }
        CountOutcome( outcome, packId, report );
    }

    // the documents that no pack of the catalogue owns: reported, not loaded
    const CPrimaryAssetStoreSettings settings = GetSettings();
    for ( size_t l=0; l<settings.locations.size(); ++l )
    {
        const CORE::CString root = GetLocationRoot( settings.locations[ l ].locationId );
        if ( root.IsNULLOrEmpty() || !IsLocationOnline( settings.locations[ l ].locationId ) )
            continue;
        VFS::CVFS::TStringVector folders;
        vfs.GetDirList( folders, CPackStorageLayout::Join( root, CPackStorageLayout::IntermediateArea ), false, false, CORE::CString::Empty );
        for ( size_t f=0; f<folders.size(); ++f )
        {
            const CORE::CString name = LastFolderName( folders[ f ] );
            if ( ownedFolders.end() != ownedFolders.find( settings.locations[ l ].locationId + "\n" + name.Lowercase() ) )
                continue;
            if ( vfs.FileExists( CPackStorageLayout::Join( CPackStorageLayout::Join( CPackStorageLayout::Join( root, CPackStorageLayout::IntermediateArea ), name ), "pack-index.json" ) ) )
            {
                ++report.documentsWithoutPack;
                if ( report.problems.size() < 50 )
                    report.problems.push_back( "the folder " + name + " of the location " + settings.locations[ l ].locationId + " has a pack index and no pack of the catalogue owns it" );
            }
        }
    }
    report.message = report.ToSummaryText();
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Reindex from the storage: " << report.message );
    return report.status = ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPrimaryAssetStore::ReindexFromStorageAsync( bool buildMissing, const CAssetPackIndexOptions& options )
{GUCEF_TRACE;

    CPrimaryAssetStoreOperationData* data = GUCEF_NEW CPrimaryAssetStoreOperationData();
    data->operation = CPrimaryAssetStoreOperationData::OPERATION_REINDEX_FROM_STORAGE;
    data->indexOptions = options;
    data->buildMissing = buildMissing;
    return QueueOperation( data );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryAssetStore::RepairInterruptedIndexes( void )
{GUCEF_TRACE;

    // a load of a pack index that was interrupted (a backend without transactions) is made again from the document
    if ( m_catalogue.GetSchemaVersion() < 4 )
        return;
    CORE::CString::StringVector packIds;
    {
        CAssetPackIndexLoader loader( m_catalogue, &m_vocabulary.GetKeywordNormalizer() );
        if ( ASSETSTORE_STATUS_OK != loader.GetPacksInState( CAssetPackIndexInfo::StateLoading, packIds ) )
            return;
    }
    for ( size_t p=0; p<packIds.size(); ++p )
    {
        CAssetPackIndexInfo info;
        GetPackIndexInfo( packIds[ p ], false, info );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PrimaryAssetStore: Repairing the pack index of pack " << packIds[ p ] << ": its load was interrupted" );
        CAssetPackIndexOutcome outcome;
        const TAssetStoreStatus status = LoadPackIndexDocument( packIds[ p ], info.documentPath, true, outcome );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            MT::CScopeMutex indexLock( m_indexLock );
            CAssetPackIndexLoader loader( m_catalogue, &m_vocabulary.GetKeywordNormalizer() );
            loader.Remove( packIds[ p ] );
        }
    }
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetPackIndexInfo( const CORE::CString& packId, bool withManifest, CAssetPackIndexInfo& info )
{GUCEF_TRACE;

    CAssetPackIndexLoader loader( m_catalogue, &m_vocabulary.GetKeywordNormalizer() );
    const TAssetStoreStatus status = loader.GetInfo( packId, false, info );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;

    // where the document is now: the path in the row is where it was when it was loaded, and a location that was relocated or a pack that was
    // moved to another layout has it elsewhere
    CAssetPackRecord pack;
    CORE::CString currentPath;
    if ( ASSETSTORE_STATUS_OK == GetPack( packId, pack ) && ASSETSTORE_STATUS_OK == FindPackIndexPath( pack, currentPath ) && VFS::CVfsGlobal::Instance()->GetVfs().FileExists( currentPath ) )
        info.documentPath = currentPath;
    if ( withManifest && !info.documentPath.IsNULLOrEmpty() )
        loader.ReadManifest( info.documentPath, info );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetPackIndexGroups( const CORE::CString& packId, TAssetPackGroupVector& groups )
{GUCEF_TRACE;

    CAssetPackIndexLoader loader( m_catalogue, &m_vocabulary.GetKeywordNormalizer() );
    return loader.GetGroups( packId, groups );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetPackIndexResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup, TAssetPackResourceVector& resources )
{GUCEF_TRACE;

    CAssetPackIndexLoader loader( m_catalogue, &m_vocabulary.GetKeywordNormalizer() );
    return loader.GetResources( packId, groupId, subGroup, resources );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetPackIndexResource( const CORE::CString& resourceId, CAssetPackResource& resource )
{GUCEF_TRACE;

    CAssetPackIndexLoader loader( m_catalogue, &m_vocabulary.GetKeywordNormalizer() );
    return loader.GetResource( resourceId, resource );
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
    request.formatId = job.formatId;
    request.expectedRevision = job.expectedRevision;
    request.syncJobId = job.syncJobId;
    return request;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::EnqueueDownload( CAssetStoreDownloadManager& downloadManager, const CAssetDownloadRequest& request, CORE::CString& jobId )
{GUCEF_TRACE;

    const CAssetSourcePolicy policy = GetSettings().GetPolicy( request.providerId );
    const CORE::CString destination = ChooseDownloadStagingPath( policy.locationId, request.expectedBytes );
    if ( destination.IsNULLOrEmpty() )
        return Fail( ASSETSTORE_STATUS_NOT_FOUND, "There is no storage location for downloads of " + request.providerId );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( destination ) && !vfs.CreateDir( destination ) )
        return Fail( ASSETSTORE_STATUS_IO_ERROR, "The staging folder " + destination + " cannot be made" );
    CAssetStoreDownloadJob job;
    job.providerId = request.providerId;
    job.itemId = request.itemId;
    job.itemTitle = request.itemTitle;
    job.formatId = request.formatId;
    job.fileIds = request.fileIds;
    job.destination = destination;
    job.forIngest = true;
    job.expectedRevision = request.expectedRevision;
    job.syncJobId = request.syncJobId;
    return downloadManager.Enqueue( job, jobId );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::EnqueueDownload( CAssetStoreDownloadManager& downloadManager, const CORE::CString& providerId, const CORE::CString& itemId,
                                     const CORE::CString& formatId, const CORE::CString::StringVector& fileIds, CORE::CString& jobId )
{GUCEF_TRACE;

    const CAssetSourcePolicy policy = GetSettings().GetPolicy( providerId );
    const CORE::CString destination = ChooseDownloadStagingPath( policy.locationId, 0 );
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
    vfsPath = CPackStorageLayout( pack.rootPath ).Resolve( GetLocationRoot( pack.locationId ), CAssetCatalogue::GetText( rows[ 0 ], "original_path" ) );
    if ( !IsLocationOnline( pack.locationId ) )
    {
        message = "the storage location " + pack.locationId + " that holds the pack is not reachable";
        return ASSETSTORE_STATUS_UNAVAILABLE;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

const UInt32 CPrimaryAssetStore::MaxCompanionDepth = 8;
const UInt32 CPrimaryAssetStore::MaxCompanionFiles = 4096;

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStore::IsCompanionRelationKind( const CORE::CString& kind )
{GUCEF_TRACE;

    return "usesMaterial" == kind || "usesTexture" == kind || "usesMesh" == kind || "usesSkeleton" == kind || "usesAnimation" == kind ||
           "instancesPrefab" == kind || "dependsOn" == kind || "usesFile" == kind;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CPrimaryAssetStore::GetCompanionFiles( const CORE::CString& assetId, TAssetFileRecordVector& files, UInt32& relationCount, bool& truncated ) const
{GUCEF_TRACE;

    files.clear();
    relationCount = 0;
    truncated = false;
    if ( assetId.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;

    // the records of the packs met, read once: a pack has many companions of one asset
    typedef GUCEF::map< CORE::CString, const CAssetRecord* > TAssetIndex;
    GUCEF::map< CORE::CString, TAssetRecordVector > packAssets;
    TAssetIndex assetIndex;
    GUCEF::map< CORE::CString, CAssetPackRecord > packs;

    CORE::CString::StringSet listed;
    CORE::CString::StringSet visited;
    visited.insert( assetId );
    GUCEF::vector< std::pair< CORE::CString, UInt32 > > pending;
    pending.push_back( std::make_pair( assetId, (UInt32) 0 ) );
    for ( size_t next=0; next<pending.size(); ++next )
    {
        const CORE::CString from = pending[ next ].first;
        const UInt32 depth = pending[ next ].second;
        CORE::CDataTableQuery query;
        query.WhereEquals( "from_asset", CORE::CVariant( from ) );
        CAssetCatalogue::TRowVector rows;
        const TAssetStoreStatus status = m_catalogue.Select( "relations", query, rows );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
        GUCEF::vector< std::pair< CORE::CString, CORE::CString > > edges;      // the canonical order: kind, then target
        for ( size_t r=0; r<rows.size(); ++r )
            edges.push_back( std::make_pair( CAssetCatalogue::GetText( rows[ r ], "kind" ), CAssetCatalogue::GetText( rows[ r ], "to_asset" ) ) );
        std::sort( edges.begin(), edges.end() );
        for ( size_t r=0; r<edges.size(); ++r )
        {
            const CORE::CString& toAsset = edges[ r ].second;
            if ( !IsCompanionRelationKind( edges[ r ].first ) || toAsset.IsNULLOrEmpty() )
                continue;
            ++relationCount;
            if ( visited.find( toAsset ) != visited.end() )
                continue;
            if ( depth + 1 > MaxCompanionDepth || files.size() >= MaxCompanionFiles )
            {
                truncated = true;
                continue;
            }
            visited.insert( toAsset );

            // the record of the asset, through the assets of its pack
            TAssetIndex::const_iterator known = assetIndex.find( toAsset );
            if ( known == assetIndex.end() )
            {
                CORE::CDataTableQuery assetQuery;
                assetQuery.WhereEquals( "asset_id", CORE::CVariant( toAsset ) );
                CAssetCatalogue::TRowVector assetRows;
                if ( ASSETSTORE_STATUS_OK != m_catalogue.Select( "assets", assetQuery, assetRows ) || assetRows.empty() )
                    continue;
                const CORE::CString packId = CAssetCatalogue::GetText( assetRows[ 0 ], "pack_id" );
                if ( packAssets.find( packId ) != packAssets.end() )
                    continue;                                       // read before, the asset is not in it
                TAssetRecordVector& assets = packAssets[ packId ];
                CAssetPackRecord pack;
                if ( ASSETSTORE_STATUS_OK != GetPack( packId, pack ) || ASSETSTORE_STATUS_OK != GetAssets( packId, assets ) )
                    continue;
                packs[ packId ] = pack;
                for ( size_t a=0; a<assets.size(); ++a )
                    assetIndex[ assets[ a ].assetId ] = &assets[ a ];
                known = assetIndex.find( toAsset );
                if ( known == assetIndex.end() )
                    continue;
            }
            const CAssetRecord& used = *(*known).second;
            const CAssetPackRecord& usedPack = packs[ used.packId ];
            const CPackStorageLayout usedLayout( usedPack.rootPath );
            const CORE::CString usedLocationRoot = GetLocationRoot( usedPack.locationId );
            for ( size_t f=0; f<used.files.size() && files.size() < MaxCompanionFiles; ++f )
            {
                if ( "main" != used.files[ f ].role )
                    continue;
                CAssetFileRecord file = used.files[ f ];
                file.path = usedLayout.Resolve( usedLocationRoot, file.path );
                if ( !listed.insert( file.path ).second )
                    continue;
                file.role = "referenced";
                files.push_back( file );
            }
            pending.push_back( std::make_pair( toAsset, depth + 1 ) );
        }
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
    vfsPath = CPackStorageLayout( pack.rootPath ).Resolve( GetLocationRoot( pack.locationId ), convertedPath );
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

CORE::CString
CPrimaryAssetStore::GetPackFolderOfVfsPath( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    // layout version 2: "<root of the location>/<area>/<folder of the pack>/..." with the areas original, intermediate and the cooked area
    const CORE::CString locationId = FindLocationOfVfsPath( vfsPath );
    if ( locationId.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    const CORE::CString root = GetLocationRoot( locationId );
    const CORE::CString path = NormalizeVfsPath( vfsPath );
    if ( path.Length() <= root.Length() + 1 )
        return CORE::CString::Empty;
    const CORE::CString relative = path.CutChars( root.Length() + 1, true );
    const Int32 firstSlash = relative.HasChar( '/', true );
    if ( firstSlash <= 0 || !CPackStorageLayout::IsAreaName( relative.SubstrFromRange( 0, (UInt32) firstSlash ) ) )
        return CORE::CString::Empty;
    CORE::CString folder = relative.CutChars( (UInt32) firstSlash + 1, true );
    const Int32 secondSlash = folder.HasChar( '/', true );
    if ( secondSlash > 0 )
        folder = folder.SubstrFromRange( 0, (UInt32) secondSlash );
    return folder.IsNULLOrEmpty() ? CORE::CString::Empty : root + "/" + folder;
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryAssetStore::GetRestrictionsOfVfsPath( const CORE::CString& vfsPath, CORE::CString::StringSet& flags ) const
{GUCEF_TRACE;

    flags.clear();
    const CORE::CString locationId = FindLocationOfVfsPath( vfsPath );
    if ( locationId.IsNULLOrEmpty() )
        return false;
    const CORE::CString root = GetLocationRoot( locationId );
    const CORE::CString path = NormalizeVfsPath( vfsPath );
    CORE::CString packId;
    const CORE::CString areaKey = GetPackFolderOfVfsPath( vfsPath );
    CORE::CDataTableQuery packQuery;
    packQuery.WhereEquals( "location_id", CORE::CVariant( locationId ) );
    if ( !areaKey.IsNULLOrEmpty() )
        packQuery.WhereEquals( "root_path", CORE::CVariant( CPackStorageLayout::MakeRootPath( areaKey.CutChars( root.Length() + 1, true ) ) ) );
    CAssetCatalogue::TRowVector packs;
    if ( ASSETSTORE_STATUS_OK != m_catalogue.Select( "packs", packQuery, packs ) )
        return false;
    for ( size_t p=0; p<packs.size() && packId.IsNULLOrEmpty(); ++p )
    {
        if ( !areaKey.IsNULLOrEmpty() )
        {
            packId = CAssetCatalogue::GetText( packs[ p ], "pack_id" );
            continue;
        }
        // a pack of the legacy layout has its folder in the root of the location
        const CPackStorageLayout layout( CAssetCatalogue::GetText( packs[ p ], "root_path" ) );
        if ( layout.IsAreaLayout() )
            continue;
        const CORE::CString folder = NormalizeVfsPath( layout.GetBrowseFolder( root ) );
        if ( !folder.IsNULLOrEmpty() && ( path == folder || 0 == path.HasSubstr( folder + "/", true ) ) )
            packId = CAssetCatalogue::GetText( packs[ p ], "pack_id" );
    }
    if ( packId.IsNULLOrEmpty() )
        return false;
    CORE::CDataTableQuery facetQuery;
    facetQuery.WhereEquals( "facet", CORE::CVariant( CORE::CString( "licenceRestriction" ) ) );
    facetQuery.WhereEquals( "object_id", CORE::CVariant( packId ) );
    CAssetCatalogue::TRowVector rows;
    if ( ASSETSTORE_STATUS_OK == m_catalogue.Select( "facets", facetQuery, rows ) )
        for ( size_t r=0; r<rows.size(); ++r )
            flags.insert( CAssetCatalogue::GetText( rows[ r ], "value" ) );
    return true;
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
                                       TAssetIngestStep step, TAssetStoreStatus status, const CORE::CString& message,
                                       const CORE::CString& downloadJobId )
{GUCEF_TRACE;

    CAssetIngestEventData eventData;
    eventData.ingestId = ingestId;
    eventData.packId = packId;
    eventData.step = AssetIngestStepToString( step );
    eventData.status = AssetStoreStatusToString( status );
    eventData.message = message;
    eventData.downloadJobId = downloadJobId;
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
