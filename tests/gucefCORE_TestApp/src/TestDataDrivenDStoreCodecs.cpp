/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#include <iostream>

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

#ifndef GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#include "CDStoreCodecPluginManager.h"
#define GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CGLOBALDATADRIVENDSTORECODECFACTORY_H
#include "gucefCORE_CGlobalDataDrivenDStoreCodecFactory.h"
#define GUCEF_CORE_CGLOBALDATADRIVENDSTORECODECFACTORY_H
#endif /* GUCEF_CORE_CGLOBALDATADRIVENDSTORECODECFACTORY_H ? */

#ifndef GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H
#include "gucefCORE_CDataUriResourceAccessor.h"
#define GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H
#endif /* GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#include "TestDataDrivenDStoreCodecs.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX || GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID
  #define DEBUGBREAK __builtin_trap()
#elif GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN
  #define DEBUGBREAK DebugBreak()
#else
  #define DEBUGBREAK
#endif

#define ERRORHERE { std::cout << "Test failed @ " << __FILE__ << "(" << __LINE__ << ")\n"; DEBUGBREAK; }
#define ASSERT_TRUE( test ) if ( !(test) ) { ERRORHERE; } 
#define ASSERT_FALSE( test ) if ( (test) ) { ERRORHERE; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::CString PROTO_DEFINITION_SIMPLE = 
    "syntax = \"proto3\";\n"
    "message SimpleMessage {\n"
    "  int32 id = 1;\n"
    "  string name = 2;\n"
    "  bool is_active = 3;\n"
    "}";

static const CORE::CString PROTO_DEFINITION_NESTED = 
    "syntax = \"proto3\";\n"
    "message NestedMessage {\n"
    "  message InnerMessage {\n"
    "    int32 inner_id = 1;\n"
    "    string inner_name = 2;\n"
    "  }\n"
    "  int32 id = 1;\n"
    "  InnerMessage inner_message = 2;\n"
    "}";

static const CORE::CString PROTO_DEFINITION_COMPLEX = 
    "syntax = \"proto3\";\n"
    "message ComplexMessage {\n"
    "  repeated int32 ids = 1;\n"
    "  map<string, int32> name_to_id = 2;\n"
    "  enum Status {\n"
    "    UNKNOWN = 0;\n"
    "    ACTIVE = 1;\n"
    "    INACTIVE = 2;\n"
    "  }\n"
    "  Status status = 3;\n"
    "}";

static const CORE::CString PROTO_DEFINITION_ALL_TYPES = 
    "syntax = \"proto3\";\n"
    "message AllTypesMessage {\n"
    "  int32 int_field = 1;\n"
    "  int64 long_field = 2;\n"
    "  uint32 uint_field = 3;\n"
    "  uint64 ulong_field = 4;\n"
    "  sint32 sint_field = 5;\n"
    "  sint64 slong_field = 6;\n"
    "  fixed32 fixed_field = 7;\n"
    "  fixed64 lfixed_field = 8;\n"
    "  sfixed32 sfixed_field = 9;\n"
    "  sfixed64 slfixed_field = 10;\n"
    "  float float_field = 11;\n"
    "  double double_field = 12;\n"
    "  bool bool_field = 13;\n"
    "  string string_field = 14;\n"
    "  bytes bytes_field = 15;\n"
    "}";

// the above as a json with test values
// can be used in a tool like https://www.protobufpal.com/ to generate the base64 encoded binary representations
/*
{
  "int_field": -44,
  "long_field": -3455,
  "uint_field": 3453577,
  "ulong_field": 3557878,
  "sint_field": 97676,
  "slong_field": 868680,
  "fixed_field": 5560,
  "lfixed_field": 7670,
  "sfixed_field": -670,
  "slfixed_field": -67077899,
  "float_field": 0.454535,
  "double_field": 0.3543534534,
  "bool_field": false,
  "string_field": "this is a string",
  "bytes_field": [ ]
}
*/
static const CORE::CString PROTO_ALL_TYPES_MESSAGE_BASE64 = "CNT//////////wEQgeX/////////ARiJ5dIBIPaT2QEomPYLMJCFaj24FQAAQfYdAAAAAAAATWL9//9R9XgA/P////9d0LjoPmFi+GQbuq3WP2gAchB0aGlzIGlzIGEgc3RyaW5negA=";
static CORE::CDataDrivenDStoreCodecMetaPtr ddCodecMetaAllTypesMessage;
static CORE::CString ddCodecTypeNameForAllTypesMessage = "testCodec_protobuf_AllTypesMessage";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------*/

void
PerformProtobufTestsIfFeasible_PluginBootstrap( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& protobufCodec )
{GUCEF_TRACE;

    // Normally this would be managed via config however to keep the test self contained we do it programmatically
    
    protobufCodec.Unlink();
    
    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
    CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();
    CORE::CDStoreCodecPluginManager& codecPluginManager = coreGlobal->GetDStoreCodecPluginManager();
    CORE::CDataDrivenDStoreCodecFactory& ddCodecFactory = coreGlobal->GetDataDrivenDStoreCodecFactory();
        
    // Check if the protobuf codec is available
    if ( !codecRegistry.TryLookup( "protobuf", protobufCodec, false ) )
    {
        #ifdef GUCEF_DEBUG_MODE
        CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginPROTOBUF_d";
        CORE::CString secondPluginpath = "$MODULEDIR$/dstorepluginPROTOBUF";
        #else
        CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginPROTOBUF";
        CORE::CString secondPluginpath = "$MODULEDIR$/dstorepluginPROTOBUF_d";
        #endif
            
        // try to load it manually in a hardcoded manner
        if ( !pluginControl.AddPluginFromDir( firstPluginPath ,
                                                "test"          ,
                                                true            ) )
        {   
            if ( !pluginControl.AddPluginFromDir( secondPluginpath ,
                                                    "test"           ,
                                                    true             ) )
            { 
                std::cout << "protobuf codec not available\n";
                return;
            }
        }
    }

    codecRegistry.TryLookup( "protobuf", protobufCodec, false );
}

/*-------------------------------------------------------------------------*/

void
PerformProtobufTestsIfFeasible_DataBootstrap( void )
{GUCEF_TRACE;

    // Normally this would be managed via config however to keep the test self contained we do it programmatically
    // this also ensures a consistent state for the test
    
    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDataDrivenDStoreCodecFactory& ddCodecFactory = coreGlobal->GetDataDrivenDStoreCodecFactory();
    CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();

    ddCodecMetaAllTypesMessage = CORE::CDataDrivenDStoreCodecMeta::CreateSharedObj();
    ASSERT_TRUE( !ddCodecMetaAllTypesMessage.IsNULL() );
    ddCodecMetaAllTypesMessage->SetBaseCodecTypeName( "protobuf" );
    ddCodecMetaAllTypesMessage->SetDataDrivenCodecTypeName( ddCodecTypeNameForAllTypesMessage );
    ddCodecMetaAllTypesMessage->SetIsShareable( true );
    
    // to keep things simple and self-contained we will use the protobuf definition as a data-uri vs pointing to something external
    CORE::CUri uri;
    ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( uri, "application/test-protobuf-AllTypesMessage", PROTO_DEFINITION_ALL_TYPES, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    
    ddCodecMetaAllTypesMessage->GetDataMap()[ ddCodecTypeNameForAllTypesMessage ] = uri;

    ddCodecFactory.Register( ddCodecMetaAllTypesMessage->GetDataDrivenCodecTypeName(), ddCodecMetaAllTypesMessage );
    CORE::CDataDrivenDStoreCodecPtr ddCodecPtr = ddCodecFactory.CreateCodec( ddCodecMetaAllTypesMessage->GetDataDrivenCodecTypeName(), false );    
    ASSERT_TRUE( !ddCodecPtr.IsNULL() );
    ASSERT_TRUE( !ddCodecPtr->GetDataDrivenDStoreCodecMeta().IsNULL() );
        
    ASSERT_TRUE( codecRegistry.TryRegister( ddCodecPtr->GetDataDrivenDStoreCodecMeta()->GetDataDrivenCodecTypeName(), ddCodecPtr.StaticCast< CORE::CDStoreCodec >() ) );

}

/*-------------------------------------------------------------------------*/

void
PerformProtobufTest_FieldTypesMapping( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();

    CORE::CDynamicBuffer encodedData;
    encodedData.LinkTo( PROTO_ALL_TYPES_MESSAGE_BASE64 );
    CORE::CDynamicBufferAccess encodedDataAccess( encodedData );

    // First just fetch the DStoreCodec like any other regular codec
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr protobufCodec_AllTypesMessage;
    ASSERT_TRUE( codecRegistry.TryLookup( ddCodecTypeNameForAllTypesMessage, protobufCodec_AllTypesMessage, false ) );

    // Now we can use the codec to decode the test message

    CORE::CDataNode documentNode;
    ASSERT_TRUE( protobufCodec_AllTypesMessage->BuildDataTree( &documentNode, &encodedDataAccess ) );
    
    ASSERT_TRUE( documentNode.GetName() == "AllTypesMessage" );
    const CORE::CVariant& intFieldVar = documentNode.GetAttributeValueOrChildValueByName( "int_field" );
    ASSERT_TRUE( intFieldVar.IsInitialized() );
    ASSERT_TRUE( intFieldVar.IsInteger() );
    ASSERT_TRUE( intFieldVar.IsSignedInteger() );
    ASSERT_TRUE( intFieldVar.GetTypeId() == GUCEF_DATATYPE_INT32 );
    ASSERT_TRUE( intFieldVar.AsInt32() == -44 );
    const CORE::CVariant& longFieldVar = documentNode.GetAttributeValueOrChildValueByName( "long_field" );
    ASSERT_TRUE( longFieldVar.IsInitialized() );
    ASSERT_TRUE( longFieldVar.IsInteger() );
    ASSERT_TRUE( longFieldVar.IsSignedInteger() );
    ASSERT_TRUE( longFieldVar.GetTypeId() == GUCEF_DATATYPE_INT32 );
    ASSERT_TRUE( longFieldVar.AsInt32() == -3455 );
    const CORE::CVariant& uintFieldVar = documentNode.GetAttributeValueOrChildValueByName( "uint_field" );
    ASSERT_TRUE( uintFieldVar.IsInitialized() );
    ASSERT_TRUE( uintFieldVar.IsInteger() );
    ASSERT_TRUE( !uintFieldVar.IsSignedInteger() );
    ASSERT_TRUE( uintFieldVar.GetTypeId() == GUCEF_DATATYPE_UINT32 );
    ASSERT_TRUE( uintFieldVar.AsInt32() == 3453577 );

  //
  //"ulong_field": 3557878,
  //"sint_field": 97676,
  //"slong_field": 868680,
  //"fixed_field": 5560,
  //"lfixed_field": 7670,
  //"sfixed_field": -670,
  //"slfixed_field": -67077899,
  //"float_field": 0.454535,
  //"double_field": 0.3543534534,
  //"bool_field": false,
  //"string_field": "this is a string",

    
}

/*-------------------------------------------------------------------------*/

void
PerformProtobufTestsIfFeasible( void )
{GUCEF_TRACE;

    std::cout << "\n\n**** COMMENCING Data Driven Data Storage Codec: protobuf TESTS ****\n";
    
    try
    {
        CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
        CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
        CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();
        CORE::CDStoreCodecPluginManager& codecPluginManager = coreGlobal->GetDStoreCodecPluginManager();
        CORE::CDataDrivenDStoreCodecFactory& ddCodecFactory = coreGlobal->GetDataDrivenDStoreCodecFactory();
        
        // Check if the base protobuf codec is available
        CORE::CDStoreCodecRegistry::TDStoreCodecPtr protobufCodec;
        PerformProtobufTestsIfFeasible_PluginBootstrap( protobufCodec );        
        if ( !protobufCodec.IsNULL() )
        {
            PerformProtobufTestsIfFeasible_DataBootstrap();

            // Considering we have the base codec we can perform the relevant tests            
            // We start with a simple field type mapping test
            PerformProtobufTest_FieldTypesMapping();

        }
    }
    catch( ... )
    {
        ERRORHERE;
    }

    std::cout << "\n\n**** FINISHED Data Driven Data Storage Codec: protobuf TESTS ****\n";
}

/*-------------------------------------------------------------------------*/

void
PerformDataDrivenDStoreCodecsTests( void )
{
    std::cout << "\n\n**** COMMENCING Data Driven Data Storage Codec TESTS ****\n";
    
    try
    {
        PerformProtobufTestsIfFeasible();
        
    }
    catch( ... )
    {
        ERRORHERE;
    }

    std::cout << "\n\n**** FINISHED Data Driven Data Storage Codec parser TESTS ****\n";
}

/*-------------------------------------------------------------------------*/
