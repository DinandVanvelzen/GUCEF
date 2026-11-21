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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADER_H
#include "gucefASSETSTORE_CAssetStorePluginLoader.h"
#define GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADER_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H
#include "gucefASSETSTORE_CIAssetStoreProbe.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H ? */

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */

#ifndef TESTFAKESTOREPROVIDER_H
#include "TestFakeStoreProvider.h"
#define TESTFAKESTOREPROVIDER_H
#endif /* TESTFAKESTOREPROVIDER_H ? */

#include "TestAssetStorePluginLoader.h"

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

/** A plugin host of the tests: what a library is follows from its file name, and loading a store plugin registers a fake store */
class CFakePluginHost : public CIAssetStorePluginHost
{
    public:

    CFakePluginHost( void ) : loadCount( 0 ), factory( "fakeplugstore" ), registered( false ) {}

    virtual ~CFakePluginHost()
    {
        if ( registered )
        {
            CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "fakeplugstore" );
            CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "fakeplugstore" );
        }
    }

    virtual TAssetStorePluginKind InspectModule( const CString& modulePath, CString& reason ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CString name = ExtractFilename( modulePath );
        if ( name.HasSubstr( "broken" ) >= 0 )
        {
            reason = "the library cannot be loaded: undefined symbol";
            return ASSETSTORE_PLUGINKIND_BROKEN;
        }
        if ( name.HasSubstr( "helper" ) >= 0 )
            return ASSETSTORE_PLUGINKIND_NOT_A_PLUGIN;
        if ( name.HasSubstr( "other" ) >= 0 )
            return ASSETSTORE_PLUGINKIND_OTHER_PLUGIN;
        return ASSETSTORE_PLUGINKIND_STORE_PLUGIN;
    }

    virtual bool LoadPlugin( const CString& directory, const CString& moduleFileName, CString& reason ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++loadCount;
        loaded.push_back( moduleFileName );
        if ( moduleFileName.HasSubstr( "storeb" ) >= 0 )
        {
            reason = "the plugin did not start";
            return false;
        }
        if ( !registered )
        {
            CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "fakeplugstore", &factory );
            CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();
            registered = true;
        }
        return true;
    }

    UInt32 loadCount;
    CString::StringVector loaded;
    CFakeStoreProviderFactory factory;
    bool registered;
};

/** A probe of the tests: what the checks say is in the result, and only that */
class CFakeProbe : public CIAssetStoreProbe
{
    public:

    CFakeProbe( void ) : runCount( 0 ) {}

    virtual CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE { return "checks the fake store"; }

    virtual bool Run( CAssetStoreProbeResult& result ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++runCount;
        result.AddCheck( "sign in", "the session is accepted", true );
        result.AddCheck( "library listing", "an array of items with an id, a title and formats", true );
        result.AddCheck( "item details", "the field 'files' is missing", false );
        result.summary = "2 of 3 checks ok";
        return true;
    }

    UInt32 runCount;
};

/** A store with a probe */
class CProbeProvider : public CFakeStoreProvider
{
    public:

    virtual CIAssetStoreProbe* GetProbe( void ) GUCEF_VIRTUAL_OVERRIDE { return &probe; }

    CFakeProbe probe;
};

/** Makes the file of a library (the content is of no consequence to the fake host) */
bool
MakeLibraryFile( const CString& directory, const CString& name )
{
    CFileAccess file( directory + "/" + name, "wb" );
    return file.IsValid() && file.Write( "x", 1, 1 ) == 1;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetStorePluginLoaderTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "AssetStorePluginLoader" );

    GUCEF_TESTFW_TESTCASE( "Plugin loader: no folder set, a folder that does not exist and an empty folder are reported and load nothing" )
    try
    {
        CFakePluginHost host;
        CAssetStorePluginLoader loader( &host );
        CAssetStorePluginLoadReport report;

        ASSERT_FALSE( loader.LoadFromDirectory( CString::Empty, report ) );
        ASSERT_TRUE( !report.directoryGiven && !report.directoryFound && report.loaded.empty() );
        CString::StringVector lines;
        report.GetLines( lines );
        ASSERT_TRUE( 1 == lines.size() && lines[ 0 ].HasSubstr( "No folder" ) >= 0 );

        ASSERT_FALSE( loader.LoadFromDirectory( TestDirectory( "plugins_missing_folder" ), report ) );
        ASSERT_TRUE( report.directoryGiven && !report.directoryFound && 0 == host.loadCount );
        lines.clear();
        report.GetLines( lines );
        ASSERT_TRUE( 1 == lines.size() && lines[ 0 ].HasSubstr( "does not exist" ) >= 0 );

        const CString empty = MountFreshDirectory( "plugins_empty", "astest/plugins_empty" );
        ASSERT_TRUE( loader.LoadFromDirectory( empty, report ) );
        ASSERT_TRUE( report.directoryFound && 0 == report.libraryCount && report.loaded.empty() && report.refused.empty() && 0 == host.loadCount );
        UnmountDirectory( "plugins_empty" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Plugin loader: only what declares itself an asset store plugin is loaded; broken libraries and failing plugins are refused with their reason, the rest is left alone" )
    try
    {
        const CString directory = MountFreshDirectory( "plugins_mixed", "astest/plugins_mixed" );
        ASSERT_TRUE( MakeLibraryFile( directory, "libstorea.so" ) && MakeLibraryFile( directory, "libstoreb.so" ) && MakeLibraryFile( directory, "libbroken.so" ) &&
                     MakeLibraryFile( directory, "libhelper.so" ) && MakeLibraryFile( directory, "libother.so" ) && MakeLibraryFile( directory, "notes.txt" ) );
        CFakePluginHost host;
        CAssetStorePluginLoader loader( &host );
        CAssetStorePluginLoadReport report;

        ASSERT_FALSE( loader.LoadFromDirectory( directory, report ) );
        ASSERT_TRUE( 5 == report.libraryCount );
        ASSERT_TRUE( 1 == report.loaded.size() && "libstorea.so" == report.loaded[ 0 ] );
        ASSERT_TRUE( 2 == report.refused.size() );
        ASSERT_TRUE( report.refused[ 0 ].HasSubstr( "libbroken.so" ) >= 0 && report.refused[ 0 ].HasSubstr( "undefined symbol" ) >= 0 );
        ASSERT_TRUE( report.refused[ 1 ].HasSubstr( "libstoreb.so" ) >= 0 && report.refused[ 1 ].HasSubstr( "did not start" ) >= 0 );
        ASSERT_TRUE( 2 == report.ignored.size() );
        // the helper library and the other plugin were never loaded, only the two store plugins were tried
        ASSERT_TRUE( 2 == host.loadCount && "libstorea.so" == host.loaded[ 0 ] && "libstoreb.so" == host.loaded[ 1 ] );
        // the store the plugin registered is in the report
        ASSERT_TRUE( 1 == report.stores.size() && "fakeplugstore" == report.stores[ 0 ] );
        ASSERT_TRUE( CAssetStoreGlobal::Instance()->GetRegistry().HasProvider( "fakeplugstore" ) );
        CString::StringVector lines;
        report.GetLines( lines );
        ASSERT_TRUE( lines.size() >= 6 && lines[ 0 ].HasSubstr( "1 loaded" ) >= 0 );
        UnmountDirectory( "plugins_mixed" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Plugin loader: with the system host a file that is no library is refused with the reason of the system and nothing is loaded" )
    try
    {
        const CString directory = MountFreshDirectory( "plugins_system", "astest/plugins_system" );
        {
            CFileAccess file( directory + "/libnotreallyalibrary.so", "wb" );
            ASSERT_TRUE( file.IsValid() );
            const char text[] = "this is text and not a shared library";
            ASSERT_TRUE( file.Write( text, sizeof( text ), 1 ) == 1 );
        }
        CAssetStorePluginLoader loader;
        CAssetStorePluginLoadReport report;
        ASSERT_FALSE( loader.LoadFromDirectory( directory, report ) );
        ASSERT_TRUE( 1 == report.libraryCount && report.loaded.empty() && 1 == report.refused.size() && report.stores.empty() );
        ASSERT_TRUE( report.refused[ 0 ].HasSubstr( "libnotreallyalibrary.so: " ) == 0 && report.refused[ 0 ].Length() > 30 );
        UnmountDirectory( "plugins_system" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Probe: a provider that offers one runs it as a task and gives the checks as lines; a provider without one ends in unsupported" )
    try
    {
        CProbeProvider provider;
        ASSERT_TRUE( GUCEF_NULL != provider.GetProbe() && provider.GetProbe()->GetDescription().HasSubstr( "fake store" ) >= 0 );
        CFutureResult future = provider.RunProbeAsync();
        CAssetStoreProbeResult result;
        ASSERT_TRUE( GetFutureResult( future, result ) );
        ASSERT_TRUE( 3 == result.checks.size() && 2 == result.GetPassedCount() && !result.IsAllPassed() );
        CString::StringVector lines;
        result.GetLines( lines );
        ASSERT_TRUE( 4 == lines.size() && "ok: sign in: the session is accepted" == lines[ 0 ] && lines[ 2 ].HasSubstr( "failed: item details" ) == 0 && "2 of 3 checks ok" == lines[ 3 ] );
        ASSERT_TRUE( 1 == provider.probe.runCount );

        CFakeStoreProvider plain;
        ASSERT_TRUE( GUCEF_NULL == plain.GetProbe() );
        CFutureResult unsupported = plain.RunProbeAsync();
        ASSERT_TRUE( WaitForFuture( unsupported, 10000 ) );
        ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == CIAssetStoreProvider::GetStatusOfFuture( unsupported ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
