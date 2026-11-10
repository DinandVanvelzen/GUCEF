/*
 *  gucefPRODMAN_TestApp: Application for testing gucefPRODMAN
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

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

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

#ifndef GUCEF_PRODMAN_CCODECBASEDPRODUCTINFOLISTPROVIDER_H
#include "gucefPRODMAN_CCodecBasedProductInfoListProvider.h"
#define GUCEF_PRODMAN_CCODECBASEDPRODUCTINFOLISTPROVIDER_H
#endif /* GUCEF_PRODMAN_CCODECBASEDPRODUCTINFOLISTPROVIDER_H ? */

#ifndef GUCEF_PRODMAN_CPRODUCTINFOLISTRETRIEVEDEVENTDATA_H
#include "gucefPRODMAN_CProductInfoListRetrievedEventData.h"
#define GUCEF_PRODMAN_CPRODUCTINFOLISTRETRIEVEDEVENTDATA_H
#endif /* GUCEF_PRODMAN_CPRODUCTINFOLISTRETRIEVEDEVENTDATA_H ? */

#ifndef GUCEF_PRODMAN_CDOWNLOADSMANAGER_H
#include "gucefPRODMAN_CDownloadsManager.h"
#define GUCEF_PRODMAN_CDOWNLOADSMANAGER_H
#endif /* GUCEF_PRODMAN_CDOWNLOADSMANAGER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestProductListRetrieval.h"

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

/** records what a product info list provider reports */
class CProviderRecorder : public CORE::CObservingNotifier
{
    public:

    UInt32 started;
    UInt32 retrieved;
    UInt32 failed;
    CORE::CString source;
    bool hadList;

    CProviderRecorder( PRODMAN::CProductInfoListProvider& provider )
        : CORE::CObservingNotifier(), started( 0 ), retrieved( 0 ), failed( 0 ), source(), hadList( false )
    {
        SubscribeTo( &provider );
    }

    bool IsFinished( void ) const { return retrieved + failed > 0; }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == PRODMAN::CProductInfoListProvider::ProductInfoListRetrievalStartedEvent )
            ++started;
        else
        if ( eventid == PRODMAN::CProductInfoListProvider::ProductInfoListRetrievedEvent )
        {
            ++retrieved;
            PRODMAN::CProductInfoListRetrievedEventData* data = static_cast< PRODMAN::CProductInfoListRetrievedEventData* >( eventdata );
            if ( GUCEF_NULL != data )
            {
                source = data->GetListSourceLocation();
                hadList = !data->GetProductInfoList().IsNULL();
            }
        }
        else
        if ( eventid == PRODMAN::CProductInfoListProvider::ProductInfoListRetrievalErrorEvent )
            ++failed;
    }
};

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
PerformProductListRetrievalTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING product list retrieval TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "ProductListRetrieval" );

    if ( !XmlCodecAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Product list retrieval TESTS skipped: the xml codec plugin is not available" );
        return;
    }

    CORE::CString workDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "prodmantemp" );
    CORE::CreateDirs( workDir );
    CORE::CString listPath = CORE::CombinePath( workDir, "products.xml" );
    CORE::WriteStringAsTextFile( listPath, "<ProductList></ProductList>" );

    // an absolute path in a file uri has no authority, so on posix the path follows "file://"
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    CORE::CString listUri = "file:///" + listPath.ReplaceChar( '\\', '/' );
    #else
    CORE::CString listUri = "file://" + listPath;
    #endif

    GUCEF_TESTFW_TESTCASE( "Test: A product list is retrieved from a file uri and from a data uri" )
        try
        {
            PRODMAN::CCodecBasedProductInfoListProvider provider;
            CProviderRecorder recorder( provider );

            ASSERT_TRUE( provider.RetrieveList( listUri ) );
            ASSERT_FALSE( provider.RetrieveList( listUri ) );       // busy
            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == recorder.started && 1 == recorder.retrieved && 0 == recorder.failed );
            ASSERT_TRUE( recorder.hadList );
            ASSERT_TRUE( recorder.source == listUri );

            // the same list, as a data uri
            CProviderRecorder dataRecorder( provider );
            ASSERT_TRUE( provider.RetrieveList( "data:text/xml;base64,PFByb2R1Y3RMaXN0PjwvUHJvZHVjdExpc3Q+" ) );
            ASSERT_TRUE( PumpUntil( [&dataRecorder]() { return dataRecorder.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == dataRecorder.retrieved && 0 == dataRecorder.failed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A product list that cannot be retrieved or decoded is reported as an error" )
        try
        {
            PRODMAN::CCodecBasedProductInfoListProvider provider;
            CProviderRecorder recorder( provider );

            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            CORE::CString missingUri = "file:///" + CORE::CombinePath( workDir, "missing.xml" ).ReplaceChar( '\\', '/' );
            #else
            CORE::CString missingUri = "file://" + CORE::CombinePath( workDir, "missing.xml" );
            #endif
            ASSERT_TRUE( provider.RetrieveList( missingUri ) );
            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.IsFinished(); }, 10000 ) );
            ASSERT_TRUE( 1 == recorder.failed && 0 == recorder.retrieved );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The list of available downloads is retrieved one at a time" )
        try
        {
            PRODMAN::CDownloadsManager manager;
            ASSERT_TRUE( manager.ObtainAvailableDownloadsList( listUri ) );
            ASSERT_FALSE( manager.ObtainAvailableDownloadsList( listUri ) );    // still busy

            // once the retrieval has ended a new one can be started
            bool restarted = false;
            ASSERT_TRUE( PumpUntil( [&manager, &restarted, &listUri]() { restarted = manager.ObtainAvailableDownloadsList( listUri ); return restarted; }, 10000 ) );
            ASSERT_TRUE( restarted );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::DeleteFile( listPath );
    CORE::Remove_Directory( workDir.C_String(), 1 );
}

/*-------------------------------------------------------------------------*/
