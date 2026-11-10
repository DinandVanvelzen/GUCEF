/*
 *  gucefPATCHERAPP_TestApp: Application for testing gucefPATCHERAPP
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PATCHER_CPATCHENGINE_H
#include "gucefPATCHER_CPatchEngine.h"
#define GUCEF_PATCHER_CPATCHENGINE_H
#endif /* GUCEF_PATCHER_CPATCHENGINE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPatchEngine.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** records the events of a patch engine that are of interest to the tests */
class CPatchEngineRecorder : public CORE::CObservingNotifier
{
    public:

    UInt32 started;
    UInt32 completed;
    UInt32 failed;
    UInt32 aborted;
    UInt32 listRetrievalFailed;
    UInt32 listDataReceived;
    UInt32 fileRetrievalStarted;
    UInt32 fileReplaced;

    CPatchEngineRecorder( PATCHER::CPatchEngine& engine )
        : CORE::CObservingNotifier()
        , started( 0 ), completed( 0 ), failed( 0 ), aborted( 0 ), listRetrievalFailed( 0 ), listDataReceived( 0 )
        , fileRetrievalStarted( 0 ), fileReplaced( 0 )
    {
        SubscribeTo( &engine );
    }

    bool IsFinished( void ) const { return completed + failed + aborted > 0; }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == PATCHER::CPatchEngine::PatchProcessStartedEvent ) ++started;
        else if ( eventid == PATCHER::CPatchEngine::PatchProcessCompletedEvent ) ++completed;
        else if ( eventid == PATCHER::CPatchEngine::PatchProcessFailedEvent ) ++failed;
        else if ( eventid == PATCHER::CPatchEngine::PatchProcessAbortedEvent ) ++aborted;
        else if ( eventid == PATCHER::CPatchEngine::PatchListRetrievalFailedEvent ) ++listRetrievalFailed;
        else if ( eventid == PATCHER::CPatchEngine::PatchListDataReceivedEvent ) ++listDataReceived;
        else if ( eventid == PATCHER::CPatchEngine::FileRetrievalStartedEvent ) ++fileRetrievalStarted;
        else if ( eventid == PATCHER::CPatchEngine::LocalFileReplacedEvent ) ++fileReplaced;
    }
};

/*-------------------------------------------------------------------------*/

/** accessor of the scheme "slowtest" which takes ages to deliver its data, in small chunks, until the destination refuses data */
class CSlowTestAccessor : public CORE::CUriResourceAccessor
{
    public:

    static const CORE::CString SchemeName;

    virtual bool CreateResource( const CORE::CUri& uri, CORE::CIOAccess& src ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool GetResource( const CORE::CUri& uri, CORE::CIOAccess& destination ) GUCEF_VIRTUAL_OVERRIDE
    {
        char chunk[ 100 ];
        memset( chunk, ' ', sizeof( chunk ) );
        for ( UInt32 i=0; i<100000; ++i )
        {
            if ( 1 != destination.Write( chunk, sizeof( chunk ), 1 ) )
                return false;
            MT::ThreadDelay( 2 );
        }
        return true;
    }

    virtual bool GetPartialResource( const CORE::CUri& uri, UInt64 byteOffset, Int64 bytesToGet, CORE::CIOAccess& destination ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool GetResourceAccess( const CORE::CUri& uri, CORE::IOAccessPtr& accessToResource, TURI_RESOURCEACCESS_MODE mode ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool GetResourceMetaData( const CORE::CUri& uri, CORE::CResourceMetaData& metaData ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool UpdateResource( const CORE::CUri& uri, CORE::CIOAccess& src ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool UpdatePartialResource( const CORE::CUri& uri, UInt64 byteOffset, Int64 bytesToWrite, CORE::CIOAccess& src ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool UpdatePartialResource( const CORE::CUri& uri, CORE::CIOAccess& src ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool GetSupportedOperations( const CORE::CUri& uri, CUriResourceAccessorOperations& ops ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool DeleteResource( const CORE::CUri& uri ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool IsACollectionResource( const CORE::CUri& uri ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool ResolveResourcesInCollection( const CORE::CUri& uri, UriVector& resources, bool recursive, bool addCollectionUris ) GUCEF_VIRTUAL_OVERRIDE { return false; }
};

const CORE::CString CSlowTestAccessor::SchemeName = "slowtest";

typedef CORE::CTFactory< CORE::CUriResourceAccessor, CSlowTestAccessor, MT::CMutex > TSlowTestAccessorFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
LoadPlugin( const CORE::CString& pluginName )
{
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
}

/*-------------------------------------------------------------------------*/

static bool
XmlCodecAvailable( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "xml", codec, false ) )
        LoadPlugin( "dstorepluginPARSIFALXML" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "xml", codec, false );
}

/*-------------------------------------------------------------------------*/

template< typename Condition >
static bool
PumpUntil( Condition condition, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; ++waited )
    {
        if ( condition() )
            return true;
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return condition();
}

/*-------------------------------------------------------------------------*/

void
PerformPatchEngineTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING patch engine TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "PatchEngine" );

    if ( !XmlCodecAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Patch engine TESTS skipped: the xml codec plugin is not available" );
        return;
    }

    TSlowTestAccessorFactory slowFactory;
    CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().RegisterConcreteFactory( CSlowTestAccessor::SchemeName, &slowFactory );

    // The "remote" side is a directory which is reached using file uris
    CORE::CString workDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "patcherapptemp" );
    CORE::CString remoteDir = CORE::CombinePath( workDir, "remote" );
    CORE::CString localDir = CORE::CombinePath( workDir, "local" );
    CORE::CString tempDir = CORE::CombinePath( workDir, "temp" );
    CORE::CreateDirs( remoteDir );
    CORE::CreateDirs( localDir );
    CORE::CreateDirs( tempDir );

    // a uri with empty authority holds the absolute path on posix, on windows the drive follows the slashes
    CORE::CString remoteUri = "file:///" + remoteDir;
    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    remoteUri = "file://" + remoteDir;
    #else
    remoteUri = remoteUri.ReplaceChar( '\\', '/' );
    #endif

    CORE::CString fileContent = "This file is patched into place using file uris. It is not big but it is not empty either.";

    GUCEF_TESTFW_TESTCASE( "Test: A patch list, patch set and file retrieved using file uris are patched into place" )
        try
        {
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( remoteDir, "fileA.txt" ), fileContent ) );

            CORE::CString patchSet =
                "<PatchSet>"
                    "<Dir Name=\"data\" TotalSize=\"" + CORE::ToString( fileContent.Length() ) + "\" Hash=\"none\">"
                        "<File Name=\"fileA.txt\" Size=\"" + CORE::ToString( fileContent.Length() ) + "\" Hash=\"none\">"
                            "<FileLocation URL=\"" + remoteUri + "/fileA.txt\" Codec=\"\" CodecParams=\"\"/>"
                        "</File>"
                    "</Dir>"
                "</PatchSet>";
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( remoteDir, "set1.xml" ), patchSet ) );

            CORE::CString patchList =
                "<PatchList>"
                    "<PatchSetEntry Name=\"set1\">"
                        "<Location URL=\"" + remoteUri + "/set1.xml\" Codec=\"xml\"/>"
                    "</PatchSetEntry>"
                "</PatchList>";
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( remoteDir, "list.xml" ), patchList ) );

            PATCHER::CPatchEngine engine;
            CPatchEngineRecorder recorder( engine );
            engine.GetConfig().SetPatchListURL( remoteUri + "/list.xml" );
            engine.GetConfig().SetPatchListCodec( "xml" );
            engine.GetConfig().SetLocalRootDir( localDir );
            engine.GetConfig().SetLocalTempStorageRootDir( tempDir );

            ASSERT_TRUE( engine.Start() );
            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.IsFinished(); }, 20000 ) );
            ASSERT_TRUE( 1 == recorder.started );
            ASSERT_TRUE( 1 == recorder.completed && 0 == recorder.failed && 0 == recorder.aborted );
            ASSERT_TRUE( 0 == recorder.listRetrievalFailed );
            ASSERT_TRUE( recorder.listDataReceived > 0 );
            ASSERT_TRUE( recorder.fileRetrievalStarted > 0 );
            ASSERT_FALSE( engine.IsActive() );

            CORE::CString patched;
            // the top level directory of a patch set is the root, which is the local root dir
            ASSERT_TRUE( CORE::LoadTextFileAsString( CORE::CombinePath( localDir, "fileA.txt" ), patched, false ) );
            ASSERT_TRUE( fileContent == patched );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A patch list which cannot be retrieved fails the patch process" )
        try
        {
            PATCHER::CPatchEngine engine;
            CPatchEngineRecorder recorder( engine );
            engine.GetConfig().SetPatchListURL( remoteUri + "/nosuchlist.xml" );
            engine.GetConfig().SetPatchListCodec( "xml" );
            engine.GetConfig().SetLocalRootDir( localDir );
            engine.GetConfig().SetLocalTempStorageRootDir( tempDir );

            ASSERT_TRUE( engine.Start() );
            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.IsFinished(); }, 20000 ) );
            ASSERT_TRUE( 1 == recorder.listRetrievalFailed );
            ASSERT_TRUE( 1 == recorder.failed && 0 == recorder.completed );
            ASSERT_FALSE( engine.IsActive() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Stopping the engine while it retrieves the patch list aborts the patch process" )
        try
        {
            PATCHER::CPatchEngine engine;
            CPatchEngineRecorder recorder( engine );
            engine.GetConfig().SetPatchListURL( "slowtest://list" );
            engine.GetConfig().SetPatchListCodec( "xml" );
            engine.GetConfig().SetLocalRootDir( localDir );
            engine.GetConfig().SetLocalTempStorageRootDir( tempDir );

            ASSERT_TRUE( engine.Start() );
            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.listDataReceived > 0; }, 20000 ) );
            ASSERT_TRUE( engine.IsActive() );

            engine.Stop();
            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.IsFinished(); }, 20000 ) );
            ASSERT_TRUE( 1 == recorder.aborted && 0 == recorder.completed && 0 == recorder.failed );
            ASSERT_FALSE( engine.IsActive() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().UnregisterConcreteFactory( CSlowTestAccessor::SchemeName );
    CORE::Remove_Directory( workDir.C_String(), 1 );
}

/*-------------------------------------------------------------------------*/
