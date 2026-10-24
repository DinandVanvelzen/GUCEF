/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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

#ifndef GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H
#include "gucefMULTIPLAY_CPassiveURLServerListProvider.h"
#define GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPassiveUrlServerListProvider.h"

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

/** records what a server list provider reports */
class CServerListRecorder : public CORE::CObservingNotifier
{
    public:

    GUCEF::vector< MULTIPLAY::CServerListProvider::TServerInfo > servers;
    UInt32 finished;
    UInt32 failed;
    UInt32 aborted;

    CServerListRecorder( MULTIPLAY::CServerListProvider& provider )
        : CORE::CObservingNotifier(), servers(), finished( 0 ), failed( 0 ), aborted( 0 )
    {
        SubscribeTo( &provider );
    }

    bool IsDone( void ) const { return finished + failed + aborted > 0; }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == MULTIPLAY::CServerListProvider::ServerInfoEvent )
        {
            MULTIPLAY::CServerListProvider::ServerInfoEventData* data = static_cast< MULTIPLAY::CServerListProvider::ServerInfoEventData* >( eventdata );
            if ( GUCEF_NULL != data )
                servers.push_back( data->GetData() );
        }
        else
        if ( eventid == MULTIPLAY::CServerListProvider::ServerListProviderFinisedEvent ) ++finished;
        else
        if ( eventid == MULTIPLAY::CServerListProvider::ServerListProviderErrorEvent ) ++failed;
        else
        if ( eventid == MULTIPLAY::CServerListProvider::ServerListProviderAbortEvent ) ++aborted;
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

/** Pumps the pulse until the provider reported its end, or the time is up */
static bool
PumpUntilDone( const CServerListRecorder& recorder, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; ++waited )
    {
        if ( recorder.IsDone() )
            return true;
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return recorder.IsDone();
}

/*-------------------------------------------------------------------------*/

void
PerformPassiveUrlServerListProviderTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING passive url server list provider TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "PassiveUrlServerListProvider" );

    if ( !XmlCodecAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Passive url server list provider TESTS skipped: the xml codec plugin is not available" );
        return;
    }

    TSlowTestAccessorFactory slowFactory;
    CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().RegisterConcreteFactory( CSlowTestAccessor::SchemeName, &slowFactory );

    CORE::CString workDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "multiplaytemp" );
    CORE::CreateDirs( workDir );
    CORE::CString listPath = CORE::CombinePath( workDir, "servers.xml" );
    ASSERT_TRUE( CORE::WriteStringAsTextFile( listPath,
        "<ServerList>"
            "<ServerEntry Name=\"Alpha\" Address=\"10.1.2.4\" Port=\"4000\" Map=\"desert\"/>"
            "<ServerEntry Name=\"Beta\" Address=\"10.1.2.3\" Port=\"4001\"/>"
            "<ServerEntry Name=\"NoPortSoNotValid\" Address=\"gamma.example.com\"/>"
        "</ServerList>" ) );

    // an absolute path in a file uri has no authority, so on posix the path follows "file://"
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    CORE::CString listUri = "file:///" + listPath.ReplaceChar( '\\', '/' );
    #else
    CORE::CString listUri = "file://" + listPath;
    #endif

    GUCEF_TESTFW_TESTCASE( "Test: A server list is retrieved from a file uri and its valid entries are reported" )
        try
        {
            MULTIPLAY::CPassiveURLServerListProvider provider;
            CServerListRecorder recorder( provider );

            CORE::CValueList params;
            params.Set( "URL", listUri );
            ASSERT_TRUE( provider.Start( params ) );
            ASSERT_TRUE( provider.IsBusy() );
            ASSERT_FALSE( provider.Start( params ) );     // one at a time

            ASSERT_TRUE( PumpUntilDone( recorder, 10000 ) );
            ASSERT_TRUE( 1 == recorder.finished && 0 == recorder.failed );
            ASSERT_FALSE( provider.IsBusy() );
            ASSERT_TRUE( 2 == recorder.servers.size() );
            if ( 2 == recorder.servers.size() )
            {
                ASSERT_TRUE( "Alpha" == recorder.servers[ 0 ].name );
                ASSERT_TRUE( "10.1.2.4" == recorder.servers[ 0 ].hostAddress.GetHostname() );
                ASSERT_TRUE( 4000 == recorder.servers[ 0 ].hostAddress.GetPortInHostByteOrder() );
                ASSERT_TRUE( "Beta" == recorder.servers[ 1 ].name );
                ASSERT_TRUE( 4001 == recorder.servers[ 1 ].hostAddress.GetPortInHostByteOrder() );
                // the attributes beyond name, address and port come along as extras, per entry
                ASSERT_TRUE( "desert" == recorder.servers[ 0 ].extra.GetValueAlways( "Map" ).AsString() );
                ASSERT_FALSE( recorder.servers[ 1 ].extra.HasKey( "Map" ) );
            }

            // and it can be used again, which also works with a url that was set up front
            CServerListRecorder secondRecorder( provider );
            ASSERT_TRUE( provider.SetURL( listUri ) );
            ASSERT_TRUE( listUri == provider.GetURL() );
            ASSERT_TRUE( provider.Start( CORE::CValueList() ) );
            ASSERT_TRUE( PumpUntilDone( secondRecorder, 10000 ) );
            ASSERT_TRUE( 1 == secondRecorder.finished && 2 == secondRecorder.servers.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A server list that cannot be retrieved is reported as an error" )
        try
        {
            MULTIPLAY::CPassiveURLServerListProvider provider;
            CServerListRecorder recorder( provider );

            CORE::CValueList params;
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            params.Set( "URL", "file:///" + CORE::CombinePath( workDir, "missing.xml" ).ReplaceChar( '\\', '/' ) );
            #else
            params.Set( "URL", "file://" + CORE::CombinePath( workDir, "missing.xml" ) );
            #endif
            ASSERT_TRUE( provider.Start( params ) );
            ASSERT_TRUE( PumpUntilDone( recorder, 10000 ) );
            ASSERT_TRUE( 1 == recorder.failed && 0 == recorder.finished );
            ASSERT_FALSE( provider.IsBusy() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Stopping the provider while it retrieves the list aborts it" )
        try
        {
            MULTIPLAY::CPassiveURLServerListProvider provider;
            CServerListRecorder recorder( provider );

            CORE::CValueList params;
            params.Set( "URL", "slowtest://servers" );
            ASSERT_TRUE( provider.Start( params ) );
            ASSERT_TRUE( provider.IsBusy() );

            provider.Stop();
            ASSERT_FALSE( provider.IsBusy() );
            ASSERT_TRUE( PumpUntilDone( recorder, 10000 ) );
            ASSERT_TRUE( 1 == recorder.aborted && 0 == recorder.failed && 0 == recorder.finished );

            // nothing else is reported after the abort
            MT::ThreadDelay( 20 );
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            ASSERT_TRUE( 1 == recorder.aborted && 0 == recorder.failed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().UnregisterConcreteFactory( CSlowTestAccessor::SchemeName );
    CORE::DeleteFile( listPath );
    CORE::Remove_Directory( workDir.C_String(), 1 );
}

/*-------------------------------------------------------------------------*/
