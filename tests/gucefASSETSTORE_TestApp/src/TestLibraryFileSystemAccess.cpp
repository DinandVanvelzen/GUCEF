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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */
#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */
#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */
#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */
#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H
#include "gucefASSETSTORE_CAssetStoreLibraryFileSystemAccess.h"
#define GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H ? */

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */
#ifndef TESTFAKESTORESERVER_H
#include "TestFakeStoreServer.h"
#define TESTFAKESTORESERVER_H
#endif /* TESTFAKESTORESERVER_H ? */
#ifndef TESTFAKESTOREPROVIDER_H
#include "TestFakeStoreProvider.h"
#define TESTFAKESTOREPROVIDER_H
#endif /* TESTFAKESTOREPROVIDER_H ? */
#include "TestLibraryFileSystemAccess.h"

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::ASSETSTORE;
using namespace ASSETSTORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Counts the events of a library file system and notes on which thread they came */
class CLibraryObserver : public CObserver
{
    public:

    UInt32 changed;
    UInt32 failed;
    CString lastMessage;
    bool allOnThisThread;

    CLibraryObserver( void )
        : CObserver()
        , changed( 0 )
        , failed( 0 )
        , lastMessage()
        , allOnThisThread( true )
        , m_thread( MT::GetCurrentThreadID() )
    {
    }

    virtual ~CLibraryObserver()
    {
        UnsubscribeAllFromObserver();
    }

    void Observe( CAssetStoreLibraryFileSystemAccess& access )
    {
        TEventCallback onChanged( this, &CLibraryObserver::OnChanged );
        TEventCallback onFailed( this, &CLibraryObserver::OnFailed );
        SubscribeTo( &access, CAssetStoreLibraryFileSystemAccess::LibraryChangedEvent, onChanged );
        SubscribeTo( &access, CAssetStoreLibraryFileSystemAccess::RefreshFailedEvent, onFailed );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString classTypeName = "ASSETSTORETEST::CLibraryObserver";
        return classTypeName;
    }

    private:

    typedef CTEventHandlerFunctor< CLibraryObserver > TEventCallback;

    void OnChanged( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        ++changed;
        allOnThisThread = allOnThisThread && m_thread == MT::GetCurrentThreadID();
    }

    void OnFailed( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
    {
        ++failed;
        allOnThisThread = allOnThisThread && m_thread == MT::GetCurrentThreadID();
        const TCloneableString* message = static_cast< const TCloneableString* >( eventData );
        if ( GUCEF_NULL != message )
            lastMessage = message->GetData();
    }

    private:

    TThreadId m_thread;
};

/** A store that is never signed in */
class CNotSignedInProvider : public CIAssetStoreProvider
{
    public:

    CNotSignedInProvider( void ) : CIAssetStoreProvider(), m_providerId( "nosignin" ) {}
    virtual ~CNotSignedInProvider() { WaitForAsyncOperations(); SignalUpcomingDestruction(); }

    virtual const CString& GetProviderId( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_providerId; }
    virtual CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Store without sign in"; }
    virtual bool IsSignedIn( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual TAssetStoreStatus QueryLibrary( const CAssetStoreQuery& query, CAssetStoreLibraryPage& page ) GUCEF_VIRTUAL_OVERRIDE { return ASSETSTORE_STATUS_NOT_SIGNED_IN; }
    virtual TAssetStoreStatus GetItem( const CString& itemId, CAssetStoreItem& item ) GUCEF_VIRTUAL_OVERRIDE { return ASSETSTORE_STATUS_NOT_SIGNED_IN; }
    virtual TAssetStoreStatus PlanDownload( const CString& itemId, const CString& formatId, const CString::StringVector& fileIds, CAssetStoreDownloadPlan& plan ) GUCEF_VIRTUAL_OVERRIDE { return ASSETSTORE_STATUS_NOT_SIGNED_IN; }

    private:

    CString m_providerId;
};

typedef CTFactory< CIAssetStoreProvider, CNotSignedInProvider, MT::CMutex > TNotSignedInProviderFactory;

/** Lists folders and looks up files on a thread of its own, while the main thread refreshes the library */
class CListingHammer : public MT::CActiveObject
{
    public:

    MT::CTAtomic< UInt32 > cycles;
    MT::CTAtomic< UInt32 > inconsistencies;

    explicit CListingHammer( CAssetStoreLibraryFileSystemAccess* access )
        : MT::CActiveObject()
        , cycles( 0 )
        , inconsistencies( 0 )
        , m_access( access )
    {
    }

    virtual ~CListingHammer() {}

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        TFileSystemIteratorPtr iterator;
        if ( FILESYSTEMACCESS_SUCCESS == m_access->TryGetFileSystemIterator( "/3d-model/Acme Studio", iterator ) )
        {
            CFileSystemEntryListIterator::TEntryVector entries;
            CFileSystemEntryListIterator::TryReadAll( *iterator, "/3d-model/Acme Studio", entries );
            for ( size_t e=0; e<entries.size(); ++e )
            {
                // an item that is found by the path of an entry is the item of that entry, whichever cache answers
                CAssetStoreItem item;
                if ( m_access->TryGetItem( entries[ e ].path, item ) && entries[ e ].name.HasSubstr( "[" + item.itemId + "]" ) < 0 )
                    ++inconsistencies;
            }
        }
        m_access->GetItemCount();
        m_access->IsLoading();
        ++cycles;
        MT::ThreadDelay( 1 );
        return false;
    }

    private:

    CAssetStoreLibraryFileSystemAccess* m_access;
};

/** The fake store with its items */
struct SLibraryWorld
{
    CFakeStoreServer server;
    CFakeStoreProviderFactory factory;

    SLibraryWorld( void ) : server(), factory( "libfake" ) {}
};

CAssetStoreItem
MakeLibraryItem( const SLibraryWorld& world, const CString& itemId, const CString& title, const CString& seller, const CString& listingType, UInt32 firstFormatSize )
{
    CAssetStoreItem item;
    item.itemId = itemId;
    item.title = title;
    item.seller = seller;
    item.listingType = listingType;
    item.thumbnailUrl = world.server.GetBaseUrl() + "/files/thumb-" + itemId;
    if ( firstFormatSize > 0 )
    {
        CAssetStoreFormat first;
        first.formatId = "fbx";
        CAssetStoreFile file;
        file.fileId = "file-" + itemId + "-a";
        file.name = "a.fbx";
        file.size = firstFormatSize;
        first.files.push_back( file );
        file.fileId = "file-" + itemId + "-b";
        file.name = "b.fbx";
        file.size = firstFormatSize / 2;
        first.files.push_back( file );
        item.formats.push_back( first );
        CAssetStoreFormat second;
        second.formatId = "obj";
        CAssetStoreFile other;
        other.fileId = "file-" + itemId + "-c";
        other.name = "c.obj";
        other.size = 7;
        second.files.push_back( other );
        item.formats.push_back( second );
    }
    return item;
}

void
SetUpLibrary( SLibraryWorld& world )
{
    world.server.AddItem( MakeLibraryItem( world, "item-1", "Sword/Pack: Deluxe?", "Acme Studio", "3d-model", 3000 ) );
    world.server.AddItem( MakeLibraryItem( world, "item-2", "Sci-fi Crate", "Acme Studio", "3d-model", 0 ) );
    world.server.AddItem( MakeLibraryItem( world, "item-3", "Rock Material", "Rock Co", "material", 1000 ) );
    world.server.AddItem( MakeLibraryItem( world, "item-4", "Mystery", CString::Empty, CString::Empty, 10 ) );
    for ( UInt32 b=0; b<30; ++b )
    {
        const CString number = b < 10 ? "0" + ToString( b ) : ToString( b );
        world.server.AddItem( MakeLibraryItem( world, "bulk-" + number, "Bulk " + number, "Bulk Co", "3d-model", 100 ) );
    }
}

/** The names in a folder, folders with a closing separator; false when the folder cannot be listed */
bool
ListFolder( IFileSystemAccess& fileSystem, const CString& path, CString::StringVector& names )
{
    names.clear();
    TFileSystemIteratorPtr iterator;
    if ( FILESYSTEMACCESS_SUCCESS != fileSystem.TryGetFileSystemIterator( path, iterator ) )
        return false;
    CFileSystemEntryListIterator::TEntryVector entries;
    if ( !CFileSystemEntryListIterator::TryReadAll( *iterator, path, entries ) )
        return true;
    for ( size_t e=0; e<entries.size(); ++e )
        names.push_back( entries[ e ].isDirectory ? entries[ e ].name + "/" : entries[ e ].name );
    return true;
}

bool
Contains( const CString::StringVector& names, const CString& name )
{
    for ( size_t n=0; n<names.size(); ++n )
        if ( names[ n ] == name )
            return true;
    return false;
}

/** Pulses until the refresh ended and its event had its turn; true when it ended in time */
bool
WaitForRefresh( CAssetStoreLibraryFileSystemAccess& access, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; waited+=2 )
    {
        CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        if ( !access.IsLoading() )
        {
            PumpFor( 30 );
            return true;
        }
        MT::ThreadDelay( 2 );
    }
    return false;
}

/** Starts a refresh and waits for it */
bool
RefreshAndWait( CAssetStoreLibraryFileSystemAccess& access, UInt32 timeoutInMs )
{
    if ( !access.Refresh() )
    {
        PumpFor( 30 );
        return true;
    }
    return WaitForRefresh( access, timeoutInMs );
}

TAssetStoreProviderPtr
LibraryProvider( void )
{
    TAssetStoreProviderPtr provider;
    CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( "libfake", provider );
    return provider;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformLibraryFileSystemAccessTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "LibraryFileSystemAccess" );

    SLibraryWorld world;
    const bool serverStarted = world.server.Start();
    SetUpLibrary( world );
    CFakeStoreProvider::SetServer( &world.server );
    CAssetStoreRegistry& registry = CAssetStoreGlobal::Instance()->GetRegistry();
    CAssetStoreProviderFactory& factory = CAssetStoreGlobal::Instance()->GetProviderFactory();
    factory.RegisterConcreteFactory( "libfake", &world.factory );
    registry.RefreshProviders();

    {
        CAssetStoreLibraryFileSystemAccess* library = GUCEF_NEW CAssetStoreLibraryFileSystemAccess( LibraryProvider() );
        TFileSystemAccessPtr holder( library );
        CLibraryObserver observer;
        observer.Observe( *library );
        library->SetPageSize( 8 );

        GUCEF_TESTFW_TESTCASE( "Library listing: the cache answers, a refresh over several pages fills it and raises one event" )
            try
            {
                ASSERT_TRUE( serverStarted && !LibraryProvider().IsNULL() );
                ASSERT_TRUE( "libfake" == library->GetProviderId() && "/" == library->GetRootPath() && !library->IsWriteable( "/" ) );
                ASSERT_TRUE( 0 == library->GetItemCount() && !library->IsLoading() && ASSETSTORE_STATUS_OK == library->GetLastStatus() );

                // nothing is listed before the first refresh, and no query was made
                CString::StringVector names;
                ASSERT_TRUE( ListFolder( *library, "/", names ) && names.empty() );
                ASSERT_TRUE( 0 == world.server.GetStats().libraryRequests );

                world.server.ResetStats();
                ASSERT_TRUE( library->Refresh() && library->IsLoading() );
                ASSERT_TRUE( WaitForRefresh( *library, 20000 ) );
                ASSERT_TRUE( 34 == library->GetItemCount() && !library->IsLoading() && !library->WasTruncated() );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == library->GetLastStatus() && library->GetLastMessage().IsNULLOrEmpty() );
                ASSERT_TRUE( 5 == world.server.GetStats().libraryRequests );
                ASSERT_TRUE( 1 == observer.changed && 0 == observer.failed && observer.allOnThisThread );

                ASSERT_TRUE( ListFolder( *library, "/", names ) && 3 == names.size() );
                ASSERT_TRUE( Contains( names, "3d-model/" ) && Contains( names, "material/" ) && Contains( names, "other/" ) );
                ASSERT_TRUE( ListFolder( *library, "/3d-model", names ) && 2 == names.size() && Contains( names, "Acme Studio/" ) && Contains( names, "Bulk Co/" ) );
                ASSERT_TRUE( ListFolder( *library, "/other", names ) && 1 == names.size() && Contains( names, "unknown seller/" ) );
                ASSERT_TRUE( ListFolder( *library, "/material/Rock Co", names ) && 1 == names.size() && Contains( names, "Rock Material [item-3]" ) );
                ASSERT_TRUE( ListFolder( *library, "/3d-model/Bulk Co", names ) && 30 == names.size() && "Bulk 00 [bulk-00]" == names[ 0 ] );
                ASSERT_TRUE( ListFolder( *library, "/other/unknown seller", names ) && 1 == names.size() && "Mystery [item-4]" == names[ 0 ] );
                ASSERT_TRUE( ListFolder( *library, "/3d-model/Acme Studio/", names ) && 2 == names.size() );
                ASSERT_FALSE( ListFolder( *library, "/nothing", names ) );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( "Library names: titles are made safe, the id between the brackets round trips, sizes are those of the first format" )
            try
            {
                CAssetStoreItem item;
                ASSERT_TRUE( "Sword_Pack_ Deluxe_ [item-1]" == CAssetStoreLibraryFileSystemAccess::MakeItemFileName( MakeLibraryItem( world, "item-1", "Sword/Pack: Deluxe?", "A", "B", 1 ) ) );
                const CString path = "/3d-model/Acme Studio/Sword_Pack_ Deluxe_ [item-1]";
                ASSERT_TRUE( library->TryGetItem( path, item ) && "item-1" == item.itemId && "Sword/Pack: Deluxe?" == item.title );
                ASSERT_TRUE( "item-1" == library->GetItemIdOfPath( path ) && path == library->GetPathOfItem( "item-1" ) );
                ASSERT_TRUE( "item-1" == library->GetItemIdOfPath( path + "/" ) && library->TryGetItem( path + "/", item ) );
                ASSERT_TRUE( "item-1" == library->GetItemIdOfPath( "\\3d-model\\Acme Studio\\Sword_Pack_ Deluxe_ [item-1]" ) );
                ASSERT_TRUE( "3d-model/Acme Studio/Sword_Pack_ Deluxe_ [item-1]" == path.CutChars( 1, true ) && "item-1" == library->GetItemIdOfPath( path.CutChars( 1, true ) ) );
                ASSERT_TRUE( world.server.GetBaseUrl() + "/files/thumb-item-1" == library->GetThumbnailUrlOfPath( path ) );

                // folders and unknown paths give no item
                ASSERT_FALSE( library->TryGetItem( "/3d-model/Acme Studio", item ) );
                ASSERT_TRUE( library->GetItemIdOfPath( "/3d-model" ).IsNULLOrEmpty() && library->GetItemIdOfPath( "/" ).IsNULLOrEmpty() );
                ASSERT_TRUE( library->GetItemIdOfPath( "/3d-model/Acme Studio/nothing [x]" ).IsNULLOrEmpty() && library->GetPathOfItem( "unknown" ).IsNULLOrEmpty() );
                ASSERT_TRUE( library->GetThumbnailUrlOfPath( "/3d-model" ).IsNULLOrEmpty() );

                // every listed file gives its own item back
                CString::StringVector names;
                ASSERT_TRUE( ListFolder( *library, "/3d-model/Bulk Co", names ) );
                for ( size_t n=0; n<names.size(); ++n )
                {
                    const CString filePath = "/3d-model/Bulk Co/" + names[ n ];
                    const CString itemId = library->GetItemIdOfPath( filePath );
                    ASSERT_TRUE( !itemId.IsNULLOrEmpty() && filePath == library->GetPathOfItem( itemId ) && names[ n ].HasSubstr( "[" + itemId + "]" ) > 0 );
                }

                // sizes: the first format only, 0 without formats, the entries of the listing carry it too
                CResourceMetaData metaData;
                ASSERT_TRUE( FILESYSTEMACCESS_SUCCESS == library->TryGetMetaData( path, metaData ) && metaData.hasResourceSizeInBytes && 4500 == metaData.resourceSizeInBytes );
                ASSERT_TRUE( "Sword_Pack_ Deluxe_ [item-1]" == metaData.name && metaData.hasIsReadOnly && metaData.isReadOnly && !metaData.hasModifiedDateTime );
                ASSERT_TRUE( FILESYSTEMACCESS_SUCCESS == library->TryGetMetaData( "/3d-model/Acme Studio/Sci-fi Crate [item-2]", metaData ) && metaData.hasResourceSizeInBytes && 0 == metaData.resourceSizeInBytes );
                metaData.Clear();
                ASSERT_TRUE( FILESYSTEMACCESS_SUCCESS == library->TryGetMetaData( "/3d-model", metaData ) && "3d-model" == metaData.name && !metaData.hasResourceSizeInBytes );
                metaData.Clear();
                ASSERT_TRUE( FILESYSTEMACCESS_SUCCESS == library->TryGetMetaData( "/", metaData ) && library->GetDisplayName() == metaData.name );
                ASSERT_TRUE( FILESYSTEMACCESS_NOT_FOUND == library->TryGetMetaData( "/3d-model/missing", metaData ) );
                TFileSystemIteratorPtr iterator;
                CFileSystemEntryListIterator::TEntryVector entries;
                ASSERT_TRUE( FILESYSTEMACCESS_SUCCESS == library->TryGetFileSystemIterator( "/3d-model/Acme Studio", iterator ) && CFileSystemEntryListIterator::TryReadAll( *iterator, "/3d-model/Acme Studio", entries ) );
                ASSERT_TRUE( 2 == entries.size() && entries[ 0 ].isFile && entries[ 0 ].hasMetaData && entries[ 0 ].metaData.hasResourceSizeInBytes );
                ASSERT_TRUE( path == entries[ 1 ].path || path == entries[ 0 ].path );

                TFileSystemEntryType type = FILESYSTEMENTRYTYPE_NONE;
                ASSERT_TRUE( FILESYSTEMACCESS_SUCCESS == library->TryGetEntryType( "/", type ) && FILESYSTEMENTRYTYPE_DIRECTORY == type );
                ASSERT_TRUE( FILESYSTEMACCESS_SUCCESS == library->TryGetEntryType( path, type ) && FILESYSTEMENTRYTYPE_FILE == type );
                ASSERT_TRUE( FILESYSTEMACCESS_SUCCESS == library->TryGetEntryType( "/3d-model/Acme Studio/", type ) && FILESYSTEMENTRYTYPE_DIRECTORY == type );
                ASSERT_TRUE( FILESYSTEMACCESS_SUCCESS == library->TryGetEntryType( "/missing", type ) && FILESYSTEMENTRYTYPE_NONE == type );
                ASSERT_TRUE( FILESYSTEMACCESS_NOT_A_DIRECTORY == library->TryGetFileSystemIterator( path, iterator ) );
                TSpecialDirSet specialDirs;
                CString special;
                ASSERT_TRUE( FILESYSTEMACCESS_NOT_SUPPORTED == library->ListAvailableSpecialDirs( specialDirs ) && specialDirs.empty() );
                ASSERT_TRUE( FILESYSTEMACCESS_NOT_SUPPORTED == library->TryResolveSpecialDir( SPECIALDIR_HOME_DIR, special ) );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( "Library read only: nothing can be written, created, moved, renamed or deleted" )
            try
            {
                const CString path = "/3d-model/Acme Studio/Sci-fi Crate [item-2]";
                ASSERT_FALSE( library->IsWriteable( "/" ) || library->IsWriteable( path ) || library->IsWriteable( "/3d-model" ) );
                ASSERT_TRUE( FILESYSTEMACCESS_READ_ONLY == library->TryCreateDirectory( "/new", false ) );
                ASSERT_TRUE( FILESYSTEMACCESS_READ_ONLY == library->TryCreateDirectory( "/a/b/c", true ) );
                ASSERT_TRUE( FILESYSTEMACCESS_READ_ONLY == library->TryDeleteDirectory( "/3d-model", true ) );
                ASSERT_TRUE( FILESYSTEMACCESS_READ_ONLY == library->TryDeleteDirectory( "/3d-model", false ) );
                ASSERT_TRUE( FILESYSTEMACCESS_READ_ONLY == library->TryMoveDirectory( "/3d-model", "/moved" ) );
                ASSERT_TRUE( FILESYSTEMACCESS_READ_ONLY == library->TryMoveFile( path, "/other/moved", true ) );
                CString::StringVector files;
                files.push_back( path );
                files.push_back( "/missing" );
                IFileSystemAccess::TFileSystemAccessResultVector perFile;
                ASSERT_TRUE( FILESYSTEMACCESS_READ_ONLY == library->TryMoveFiles( files, "/other", false, &perFile ) );
                ASSERT_TRUE( 2 == perFile.size() && FILESYSTEMACCESS_READ_ONLY == perFile[ 0 ] && FILESYSTEMACCESS_READ_ONLY == perFile[ 1 ] );
                ASSERT_TRUE( FILESYSTEMACCESS_READ_ONLY == library->TryMoveFiles( files, "/other", false, GUCEF_NULL ) );
                ASSERT_TRUE( FILESYSTEMACCESS_READ_ONLY == library->TryRenameFile( path, "other" ) );
                ASSERT_TRUE( FILESYSTEMACCESS_READ_ONLY == library->TryDeleteFile( path ) );

                // the library is as it was
                CAssetStoreItem item;
                ASSERT_TRUE( 34 == library->GetItemCount() && library->TryGetItem( path, item ) );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( "Library refresh: the query text and the listing type reach the store, a second refresh replaces the cache" )
            try
            {
                const UInt32 changedBefore = observer.changed;
                CAssetStoreItem item;
                ASSERT_TRUE( !library->TryGetItem( "/3d-model/Acme Studio/Fresh Item [item-new]", item ) );

                library->SetQueryText( "crate" );
                ASSERT_TRUE( "crate" == library->GetQueryText() );
                ASSERT_TRUE( RefreshAndWait( *library, 20000 ) );
                ASSERT_TRUE( 1 == library->GetItemCount() && observer.changed == changedBefore + 1 );
                ASSERT_TRUE( "/3d-model/Acme Studio/Sci-fi Crate [item-2]" == library->GetPathOfItem( "item-2" ) && library->GetPathOfItem( "item-1" ).IsNULLOrEmpty() );
                CString::StringVector names;
                ASSERT_TRUE( ListFolder( *library, "/", names ) && 1 == names.size() && "3d-model/" == names[ 0 ] );

                library->SetQueryText( CString::Empty );
                library->SetListingTypeFilter( "material" );
                ASSERT_TRUE( "material" == library->GetListingTypeFilter() );
                ASSERT_TRUE( RefreshAndWait( *library, 20000 ) );
                ASSERT_TRUE( 1 == library->GetItemCount() && "item-3" == library->GetItemIdOfPath( "/material/Rock Co/Rock Material [item-3]" ) );
                ASSERT_TRUE( library->GetPathOfItem( "item-2" ).IsNULLOrEmpty() );

                // the store gets an item, the next refresh shows it and keeps the rest
                library->SetListingTypeFilter( CString::Empty );
                world.server.AddItem( MakeLibraryItem( world, "item-new", "Fresh Item", "Acme Studio", "3d-model", 20 ) );
                ASSERT_TRUE( RefreshAndWait( *library, 20000 ) );
                ASSERT_TRUE( 35 == library->GetItemCount() && observer.changed == changedBefore + 3 );
                ASSERT_TRUE( library->TryGetItem( "/3d-model/Acme Studio/Fresh Item [item-new]", item ) && "item-new" == item.itemId );
                ASSERT_TRUE( ListFolder( *library, "/3d-model/Acme Studio", names ) && 3 == names.size() );
                ASSERT_TRUE( 0 == observer.failed );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( "Library refresh: a refresh that starts while another runs replaces it, one event for the last" )
            try
            {
                const UInt32 changedBefore = observer.changed;
                library->SetQueryText( "bulk" );
                ASSERT_TRUE( library->Refresh() );
                library->SetQueryText( "mystery" );
                ASSERT_TRUE( library->Refresh() && library->IsLoading() );
                for ( UInt32 waited=0; waited<20000 && library->IsLoading(); waited+=2 )
                {
                    PumpFor( 2 );
                }
                PumpFor( 100 );
                ASSERT_TRUE( !library->IsLoading() && observer.changed == changedBefore + 1 );
                ASSERT_TRUE( 1 == library->GetItemCount() && "item-4" == library->GetItemIdOfPath( "/other/unknown seller/Mystery [item-4]" ) );
                library->SetQueryText( CString::Empty );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( "Library refresh: the bound on the items leaves the rest out and says so" )
            try
            {
                library->SetMaxItems( 10 );
                ASSERT_TRUE( 10 == library->GetMaxItems() );
                ASSERT_TRUE( RefreshAndWait( *library, 20000 ) );
                ASSERT_TRUE( 10 == library->GetItemCount() && library->WasTruncated() && ASSETSTORE_STATUS_OK == library->GetLastStatus() );
                library->SetMaxItems( 0 );
                ASSERT_TRUE( CAssetStoreLibraryFileSystemAccess::DefaultMaxItems == library->GetMaxItems() && 5000 == CAssetStoreLibraryFileSystemAccess::DefaultMaxItems );
                ASSERT_TRUE( RefreshAndWait( *library, 20000 ) );
                ASSERT_TRUE( 35 == library->GetItemCount() && !library->WasTruncated() );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( "Library threads: listing and lookups while refreshes complete see a whole cache every time" )
            try
            {
                CListingHammer hammer( library );
                ASSERT_TRUE( hammer.Activate( GUCEF_NULL, 1, 1 ) );
                for ( UInt32 r=0; r<6; ++r )
                {
                    library->SetQueryText( 0 == r % 2 ? CString( "sword" ) : CString::Empty );
                    ASSERT_TRUE( RefreshAndWait( *library, 20000 ) );
                }
                hammer.Deactivate( false, true );
                ASSERT_TRUE( hammer.cycles.Load() > 3 && 0 == hammer.inconsistencies.Load() );
                ASSERT_TRUE( 35 == library->GetItemCount() );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( "Library failure: a store that is out of reach fails the refresh, keeps the cache and says why" )
            try
            {
                const UInt32 failedBefore = observer.failed;
                const UInt32 changedBefore = observer.changed;
                ASSERT_TRUE( 35 == library->GetItemCount() );
                world.server.Stop();
                ASSERT_TRUE( RefreshAndWait( *library, 30000 ) );
                ASSERT_TRUE( !library->IsLoading() && ASSETSTORE_STATUS_NETWORK_ERROR == library->GetLastStatus() && !library->GetLastMessage().IsNULLOrEmpty() );
                ASSERT_TRUE( observer.failed == failedBefore + 1 && observer.changed == changedBefore && observer.lastMessage == library->GetLastMessage() );
                ASSERT_TRUE( 35 == library->GetItemCount() && "item-1" == library->GetItemIdOfPath( "/3d-model/Acme Studio/Sword_Pack_ Deluxe_ [item-1]" ) );
                ASSERT_TRUE( observer.allOnThisThread );

                // the store is back: the next refresh clears the failure
                ASSERT_TRUE( world.server.Start() );
                ASSERT_TRUE( RefreshAndWait( *library, 20000 ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == library->GetLastStatus() && library->GetLastMessage().IsNULLOrEmpty() && 35 == library->GetItemCount() );
                ASSERT_TRUE( observer.changed == changedBefore + 1 );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

        GUCEF_TESTFW_TESTCASE( "Library failure: a store without sign in fails the refresh with its status and an empty library" )
            try
            {
                TNotSignedInProviderFactory closedFactory;
                CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "nosignin", &closedFactory );
                CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();
                TAssetStoreProviderPtr provider;
                ASSERT_TRUE( CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( "nosignin", provider ) );
                CAssetStoreLibraryFileSystemAccess* closed = GUCEF_NEW CAssetStoreLibraryFileSystemAccess( provider );
                TFileSystemAccessPtr closedHolder( closed );
                CLibraryObserver closedObserver;
                closedObserver.Observe( *closed );
                ASSERT_TRUE( "nosignin" == closed->GetProviderId() && "Store without sign in" == closed->GetDisplayName() );
                ASSERT_TRUE( RefreshAndWait( *closed, 20000 ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_NOT_SIGNED_IN == closed->GetLastStatus() && 0 == closed->GetItemCount() );
                ASSERT_TRUE( 0 == closedObserver.changed && 1 == closedObserver.failed && !closedObserver.lastMessage.IsNULLOrEmpty() );
                CString::StringVector names;
                ASSERT_TRUE( ListFolder( *closed, "/", names ) && names.empty() );
                ASSERT_TRUE( closed->GetLastMessage() == closedObserver.lastMessage );
                closedHolder.Unlink();
                provider.Unlink();
                CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "nosignin" );
                CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "nosignin" );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END;

    }

    // the providers go before their factories
    registry.RemoveProvider( "libfake" );
    factory.UnregisterConcreteFactory( "libfake" );
    CFakeStoreProvider::SetServer( GUCEF_NULL );
    world.server.Stop();
}

/*-------------------------------------------------------------------------*/
