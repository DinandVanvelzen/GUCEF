/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestXmlDStoreCodec.h"

#include <string.h>

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

// the group the plugin is loaded in by these tests, its own so unloading it touches nothing else
static const char* XmlPluginGroup = "xmldstore_tests";
static bool g_loadedXmlPluginHere = false;

static bool
GetXmlCodec( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& xmlCodec )
{GUCEF_TRACE;

    // Normally this would be managed via config however to keep the test self contained we do it programmatically
    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    if ( coreGlobal->GetDStoreCodecRegistry().TryLookup( "xml", xmlCodec, false ) )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginPARSIFALXML_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/dstorepluginPARSIFALXML";
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginPARSIFALXML";
    CORE::CString secondPluginPath = "$MODULEDIR$/dstorepluginPARSIFALXML_d";
    #endif

    CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, XmlPluginGroup, true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, XmlPluginGroup, true );

    g_loadedXmlPluginHere = coreGlobal->GetDStoreCodecRegistry().TryLookup( "xml", xmlCodec, false );
    return g_loadedXmlPluginHere;
}

/*-------------------------------------------------------------------------*/

static bool
BufferContains( const CORE::CDynamicBuffer& buffer, const char* needle )
{GUCEF_TRACE;

    // Byte based search on purpose: the test is about the exact bytes being written
    const char* data = buffer.AsConstTypePtr< char >();
    UInt32 dataSize = buffer.GetDataSize();
    UInt32 needleSize = (UInt32) strlen( needle );
    if ( GUCEF_NULL == data || needleSize > dataSize )
        return false;
    for ( UInt32 i=0; i<=dataSize-needleSize; ++i )
    {
        if ( 0 == memcmp( data+i, needle, needleSize ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
PerformXmlDStoreCodecTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING XML DStore Codec TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "XmlDStoreCodec" );

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr xmlCodec;
    if ( !GetXmlCodec( xmlCodec ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "XML DStore Codec TESTS skipped: the xml codec plugin is not available" );
        return;
    }

    // Regression: the writer escaped the raw byte 0xBB as "&#187;" which corrupted UTF-8 multi-byte
    // sequences such as "û" (C3 BB) and it declared the document as ISO-8859-1 instead of UTF-8
    GUCEF_TESTFW_TESTCASE( "Test: UTF-8 text is written as-is and round trips" )
        try
        {
            // "Café û »" in UTF-8
            const CORE::CString utf8Value( "Caf\xC3\xA9 \xC3\xBB \xC2\xBB" );

            CORE::CDataNode sourceNode( "TestNode" );
            ASSERT_TRUE( sourceNode.SetAttribute( "text", utf8Value ) );

            CORE::CDynamicBuffer xmlData;
            CORE::CDynamicBufferAccess xmlDataAccess( xmlData );
            ASSERT_TRUE( xmlCodec->StoreDataTree( &sourceNode, &xmlDataAccess ) );

            ASSERT_TRUE( BufferContains( xmlData, "encoding=\"UTF-8\"" ) );
            ASSERT_TRUE( BufferContains( xmlData, "Caf\xC3\xA9 \xC3\xBB \xC2\xBB" ) );
            ASSERT_FALSE( BufferContains( xmlData, "&#187;" ) );

            xmlDataAccess.Setpos( 0 );
            CORE::CDataNode roundTripRoot;
            ASSERT_TRUE( xmlCodec->BuildDataTree( &roundTripRoot, &xmlDataAccess ) );
            const CORE::CDataNode* roundTripNode = roundTripRoot.Find( "TestNode" );
            ASSERT_TRUE( GUCEF_NULL != roundTripNode );
            if ( GUCEF_NULL != roundTripNode )
                ASSERT_TRUE( roundTripNode->GetAttributeValue( "text" ).AsString() == utf8Value );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: XML special characters are escaped and round trip" )
        try
        {
            const CORE::CString specialValue( "a<b>c&d'e\"f" );

            CORE::CDataNode sourceNode( "TestNode" );
            ASSERT_TRUE( sourceNode.SetAttribute( "text", specialValue ) );

            CORE::CDynamicBuffer xmlData;
            CORE::CDynamicBufferAccess xmlDataAccess( xmlData );
            ASSERT_TRUE( xmlCodec->StoreDataTree( &sourceNode, &xmlDataAccess ) );

            ASSERT_TRUE( BufferContains( xmlData, "a&lt;b&gt;c&amp;d&apos;e&quot;f" ) );

            xmlDataAccess.Setpos( 0 );
            CORE::CDataNode roundTripRoot;
            ASSERT_TRUE( xmlCodec->BuildDataTree( &roundTripRoot, &xmlDataAccess ) );
            const CORE::CDataNode* roundTripNode = roundTripRoot.Find( "TestNode" );
            ASSERT_TRUE( GUCEF_NULL != roundTripNode );
            if ( GUCEF_NULL != roundTripNode )
                ASSERT_TRUE( roundTripNode->GetAttributeValue( "text" ).AsString() == specialValue );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // An XML reader turns line breaks and tabs in attribute values into spaces unless they are character references
    GUCEF_TESTFW_TESTCASE( "Test: Line breaks and tabs in attribute values are written as character references and round trip" )
        try
        {
            const CORE::CString multiLine( "first\nsecond\r\nthird\tcolumn" );

            CORE::CDataNode sourceNode( "TestNode" );
            ASSERT_TRUE( sourceNode.SetAttribute( "text", multiLine ) );

            CORE::CDynamicBuffer xmlData;
            CORE::CDynamicBufferAccess xmlDataAccess( xmlData );
            ASSERT_TRUE( xmlCodec->StoreDataTree( &sourceNode, &xmlDataAccess ) );

            ASSERT_TRUE( BufferContains( xmlData, "first&#10;second&#13;&#10;third&#9;column" ) );

            xmlDataAccess.Setpos( 0 );
            CORE::CDataNode roundTripRoot;
            ASSERT_TRUE( xmlCodec->BuildDataTree( &roundTripRoot, &xmlDataAccess ) );
            const CORE::CDataNode* roundTripNode = roundTripRoot.Find( "TestNode" );
            ASSERT_TRUE( GUCEF_NULL != roundTripNode );
            if ( GUCEF_NULL != roundTripNode )
                ASSERT_TRUE( roundTripNode->GetAttributeValue( "text" ).AsString() == multiLine );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // A C plugin found by trial and error (no plugin type given) must still be unloaded through its plugin manager,
    // which takes its codec out of the registry, and it must load again afterwards
    if ( g_loadedXmlPluginHere )
    {
        GUCEF_TESTFW_TESTCASE( "Test: The xml plugin unloads cleanly and loads again" )
            try
            {
                CORE::CDStoreCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();
                CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
                xmlCodec.Unlink();

                ASSERT_TRUE( pluginControl.UnloadPluginGroup( XmlPluginGroup ) );
                CORE::CDStoreCodecRegistry::TDStoreCodecPtr gone;
                ASSERT_FALSE( registry.TryLookup( "xml", gone, false ) );

                ASSERT_TRUE( pluginControl.LoadPluginGroup( XmlPluginGroup ) );
                CORE::CDStoreCodecRegistry::TDStoreCodecPtr reloaded;
                ASSERT_TRUE( registry.TryLookup( "xml", reloaded, false ) );
                if ( !reloaded.IsNULL() )
                {
                    CORE::CDataNode sourceNode( "Again" );
                    sourceNode.SetAttribute( "value", "1" );
                    CORE::CDynamicBuffer xmlData;
                    CORE::CDynamicBufferAccess xmlDataAccess( xmlData );
                    ASSERT_TRUE( reloaded->StoreDataTree( &sourceNode, &xmlDataAccess ) );
                    ASSERT_TRUE( BufferContains( xmlData, "Again" ) );
                }
            }
            catch( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL XML DStore Codec TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
