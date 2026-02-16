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
// you can verify the correctness of the base64 encoded string below using an online tool like https://protobuf-decoder.netlify.app/
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

/*-------------------------------------------------------------------------*/

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

// the above as a json with test values
// can be used in a tool like https://www.protobufpal.com/ to generate the base64 encoded binary representations
// you can verify the correctness of the base64 encoded string below using an online tool like https://protobuf-decoder.netlify.app/
/*
{
  "id": 777,
  "inner_message": {
    "inner_id": 747,
    "inner_name": "this is the inner message"
  }
}
*/
static const CORE::CString PROTO_NESTED_MESSAGE_BASE64 = "CIkGEh4I6wUSGXRoaXMgaXMgdGhlIGlubmVyIG1lc3NhZ2U=";
static CORE::CDataDrivenDStoreCodecMetaPtr ddCodecMetaNestedMessage;
static CORE::CString ddCodecTypeNameForNestedMessage = "testCodec_protobuf_NestedMessage";

/*-------------------------------------------------------------------------*/

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
    "  oneof optional_data {\n"
    "    string optional_string = 4;\n"
    "    int32 optional_number = 5;\n"
    "    NestedOptionalMessage nested_optional_message = 6;\n"
    "  }\n"
    "  message NestedOptionalMessage {\n"
    "    string nested_optional_field_1 = 1;\n"
    "    double nested_optional_field_2 = 2;\n"
    "  }\n"
    "  message NestedMessage {\n"
    "    int32 inner_id = 1;\n"
    "    string inner_name = 2;\n"
    "    uint64 inner_timestamp = 3;\n"
    "  }\n"
    "  repeated NestedMessage nested_messages = 7;\n"
    "}";

/*
syntax = "proto3";

message ComplexMessage {
  repeated int32 ids = 1;
  map<string, int32> name_to_id = 2;
  enum Status {
    UNKNOWN = 0;
    ACTIVE = 1;
    INACTIVE = 2;
  }
  Status status = 3;
  oneof optional_data {
    string optional_string = 4;
    int32 optional_number = 5;
    NestedOptionalMessage nested_optional_message = 6;
  }
  message NestedOptionalMessage {
    string nested_optional_field_1 = 1;
    double nested_optional_field_2 = 2;
  }
  message NestedMessage {
    int32 inner_id = 1;
    string inner_name = 2;
    uint64 inner_timestamp = 3;
  }  
  repeated NestedMessage nested_messages = 7;
}
*/

// the above as a json with test values
// can be used in a tool like https://www.protobufpal.com/ to generate the base64 encoded binary representations
// you can verify the correctness of the base64 encoded string below using an online tool like https://protobuf-decoder.netlify.app/
/*
{
    "ids": [
        1,
        2,
        3,
        4,
        5
    ],
    "name_to_id": {
        "Alice": 1,
        "Bob": 2
    },
    "status": "ACTIVE",
    "nested_optional_message": {
        "nested_optional_field_1": "optional nested value",
        "nested_optional_field_2": 3.14159
    },
    "nested_messages": [
        {
            "inner_id": 10,
            "inner_name": "nested inner 1",
            "inner_timestamp": 345386786767
        },
        {
            "inner_id": 20,
            "inner_name": "nested inner 2",
            "inner_timestamp": 334435534534
        }
    ]
}
*/
static const CORE::CString PROTO_COMPLEX_MESSAGE_BASE64 = "CgUBAgMEBRIJCgVBbGljZRABEgcKA0JvYhACGAEyIAoVb3B0aW9uYWwgbmVzdGVkIHZhbHVlEW6GG/D5IQlAOhkIChIObmVzdGVkIGlubmVyIDEYz7+g1YYKOhkIFBIObmVzdGVkIGlubmVyIDIYxo2l790J";
static CORE::CDataDrivenDStoreCodecMetaPtr ddCodecMetaComplexMessage;
static CORE::CString ddCodecTypeNameForComplexMessage = "testCodec_protobuf_ComplexMessage";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

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

    ddCodecFactory.UnregisterAll();
    ddCodecFactory.UnregisterAllConcreteFactories();

    // prepare the "AllTypesMessage" codec

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

    // prepare the "NestedMessage" codec

    ddCodecMetaNestedMessage = CORE::CDataDrivenDStoreCodecMeta::CreateSharedObj();
    ASSERT_TRUE( !ddCodecMetaNestedMessage.IsNULL() );
    ddCodecMetaNestedMessage->SetBaseCodecTypeName( "protobuf" );
    ddCodecMetaNestedMessage->SetDataDrivenCodecTypeName( ddCodecTypeNameForNestedMessage );
    ddCodecMetaNestedMessage->SetIsShareable( true );
    
    // to keep things simple and self-contained we will use the protobuf definition as a data-uri vs pointing to something external
    uri.Clear();
    ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( uri, "application/test-protobuf-NestedMessage", PROTO_DEFINITION_NESTED, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    
    ddCodecMetaNestedMessage->GetDataMap()[ ddCodecTypeNameForNestedMessage ] = uri;

    ddCodecFactory.Register( ddCodecMetaNestedMessage->GetDataDrivenCodecTypeName(), ddCodecMetaNestedMessage );
    ddCodecPtr = ddCodecFactory.CreateCodec( ddCodecMetaNestedMessage->GetDataDrivenCodecTypeName(), false );    
    ASSERT_TRUE( !ddCodecPtr.IsNULL() );
    ASSERT_TRUE( !ddCodecPtr->GetDataDrivenDStoreCodecMeta().IsNULL() );
        
    ASSERT_TRUE( codecRegistry.TryRegister( ddCodecPtr->GetDataDrivenDStoreCodecMeta()->GetDataDrivenCodecTypeName(), ddCodecPtr.StaticCast< CORE::CDStoreCodec >() ) );

    // prepare the "ComplexMessage" codec

    ddCodecMetaComplexMessage = CORE::CDataDrivenDStoreCodecMeta::CreateSharedObj();
    ASSERT_TRUE( !ddCodecMetaComplexMessage.IsNULL() );
    ddCodecMetaComplexMessage->SetBaseCodecTypeName( "protobuf" );
    ddCodecMetaComplexMessage->SetDataDrivenCodecTypeName( ddCodecTypeNameForComplexMessage );
    ddCodecMetaComplexMessage->SetIsShareable( true );
    
    // to keep things simple and self-contained we will use the protobuf definition as a data-uri vs pointing to something external
    uri.Clear();
    ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( uri, "application/test-protobuf-ComplexMessage", PROTO_DEFINITION_COMPLEX, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    
    ddCodecMetaComplexMessage->GetDataMap()[ ddCodecTypeNameForComplexMessage ] = uri;

    ddCodecFactory.Register( ddCodecMetaComplexMessage->GetDataDrivenCodecTypeName(), ddCodecMetaComplexMessage );
    ddCodecPtr = ddCodecFactory.CreateCodec( ddCodecMetaComplexMessage->GetDataDrivenCodecTypeName(), false );    
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
    encodedData.CopyAndDecodeBase64From( PROTO_ALL_TYPES_MESSAGE_BASE64, 0 );
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
    ASSERT_TRUE( intFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT32 );
    ASSERT_TRUE( intFieldVar.AsInt32() == -44 );
    const CORE::CVariant& uintFieldVar = documentNode.GetAttributeValueOrChildValueByName( "uint_field" );
    ASSERT_TRUE( uintFieldVar.IsInitialized() );
    ASSERT_TRUE( uintFieldVar.IsInteger() );
    ASSERT_TRUE( !uintFieldVar.IsSignedInteger() );
    ASSERT_TRUE( uintFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_UINT32 );
    ASSERT_TRUE( uintFieldVar.AsInt32() == 3453577 );
    const CORE::CVariant& longFieldVar = documentNode.GetAttributeValueOrChildValueByName( "long_field" );
    ASSERT_TRUE( longFieldVar.IsInitialized() );
    ASSERT_TRUE( longFieldVar.IsInteger() );
    ASSERT_TRUE( longFieldVar.IsSignedInteger() );
    ASSERT_TRUE( longFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT64 );
    ASSERT_TRUE( longFieldVar.AsInt64() == -3455 );
    const CORE::CVariant& ulongFieldVar = documentNode.GetAttributeValueOrChildValueByName( "ulong_field" );
    ASSERT_TRUE( ulongFieldVar.IsInitialized() );
    ASSERT_TRUE( ulongFieldVar.IsInteger() );
    ASSERT_TRUE( !ulongFieldVar.IsSignedInteger() );
    ASSERT_TRUE( ulongFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_UINT64 );
    ASSERT_TRUE( ulongFieldVar.AsUInt64() == 3557878 );
    const CORE::CVariant& sintFieldVar = documentNode.GetAttributeValueOrChildValueByName( "sint_field" );
    ASSERT_TRUE( sintFieldVar.IsInitialized() );
    ASSERT_TRUE( sintFieldVar.IsInteger() );
    ASSERT_TRUE( sintFieldVar.IsSignedInteger() );
    ASSERT_TRUE( sintFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT32 );
    ASSERT_TRUE( sintFieldVar.AsInt32() == 97676 );
    const CORE::CVariant& slongFieldVar = documentNode.GetAttributeValueOrChildValueByName( "slong_field" );
    ASSERT_TRUE( slongFieldVar.IsInitialized() );
    ASSERT_TRUE( slongFieldVar.IsInteger() );
    ASSERT_TRUE( slongFieldVar.IsSignedInteger() );
    ASSERT_TRUE( slongFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT64 );
    ASSERT_TRUE( slongFieldVar.AsInt64() == 868680 );
    const CORE::CVariant& fixedFieldVar = documentNode.GetAttributeValueOrChildValueByName( "fixed_field" );
    ASSERT_TRUE( fixedFieldVar.IsInitialized() );
    ASSERT_TRUE( fixedFieldVar.IsInteger() );
    ASSERT_TRUE( !fixedFieldVar.IsSignedInteger() );
    ASSERT_TRUE( fixedFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_UINT32 );
    ASSERT_TRUE( fixedFieldVar.AsUInt32() == 5560 );
    const CORE::CVariant& lfixedFieldVar = documentNode.GetAttributeValueOrChildValueByName( "lfixed_field" );
    ASSERT_TRUE( lfixedFieldVar.IsInitialized() );
    ASSERT_TRUE( lfixedFieldVar.IsInteger() );
    ASSERT_TRUE( !lfixedFieldVar.IsSignedInteger() );
    ASSERT_TRUE( lfixedFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_UINT64 );
    ASSERT_TRUE( lfixedFieldVar.AsUInt64() == 7670 );
    const CORE::CVariant& sfixedFieldVar = documentNode.GetAttributeValueOrChildValueByName( "sfixed_field" );
    ASSERT_TRUE( sfixedFieldVar.IsInitialized() );
    ASSERT_TRUE( sfixedFieldVar.IsInteger() );
    ASSERT_TRUE( sfixedFieldVar.IsSignedInteger() );
    ASSERT_TRUE( sfixedFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT32 );
    ASSERT_TRUE( sfixedFieldVar.AsInt32() == -670 );
    const CORE::CVariant& slfixedFieldVar = documentNode.GetAttributeValueOrChildValueByName( "slfixed_field" );
    ASSERT_TRUE( slfixedFieldVar.IsInitialized() );
    ASSERT_TRUE( slfixedFieldVar.IsInteger() );
    ASSERT_TRUE( slfixedFieldVar.IsSignedInteger() );
    ASSERT_TRUE( slfixedFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT64 );
    ASSERT_TRUE( slfixedFieldVar.AsInt64() == -67077899 );
    const CORE::CVariant& floatFieldVar = documentNode.GetAttributeValueOrChildValueByName( "float_field" );
    ASSERT_TRUE( floatFieldVar.IsInitialized() );
    ASSERT_TRUE( !floatFieldVar.IsInteger() );
    ASSERT_TRUE( floatFieldVar.IsFloat() );
    ASSERT_TRUE( floatFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_FLOAT32 );
    CORE::CVariant compareFloat32( static_cast< Float32 >( 0.454535 ) );
    ASSERT_TRUE( floatFieldVar == compareFloat32 );
    const CORE::CVariant& doubleFieldVar = documentNode.GetAttributeValueOrChildValueByName( "double_field" );
    ASSERT_TRUE( doubleFieldVar.IsInitialized() );
    ASSERT_TRUE( !doubleFieldVar.IsInteger() );
    ASSERT_TRUE( doubleFieldVar.IsFloat() );
    ASSERT_TRUE( doubleFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_FLOAT64 );
    CORE::CVariant compareFloat64( static_cast< Float64 >( 0.3543534534 ) );
    ASSERT_TRUE( doubleFieldVar == compareFloat64 );
    const CORE::CVariant& boolFieldVar = documentNode.GetAttributeValueOrChildValueByName( "bool_field" );
    ASSERT_TRUE( boolFieldVar.IsInitialized() );
    ASSERT_TRUE( !boolFieldVar.IsInteger() );
    ASSERT_TRUE( !boolFieldVar.IsFloat() );
    ASSERT_TRUE( boolFieldVar.IsBoolean() );
    ASSERT_TRUE( boolFieldVar.GetTypeId() == GUCEF_DATATYPE_BOOLEAN_INT32 );
    ASSERT_TRUE( boolFieldVar == false );
    const CORE::CVariant& stringFieldVar = documentNode.GetAttributeValueOrChildValueByName( "string_field" );
    ASSERT_TRUE( stringFieldVar.IsInitialized() );
    ASSERT_TRUE( !stringFieldVar.IsInteger() );
    ASSERT_TRUE( !stringFieldVar.IsFloat() );
    ASSERT_TRUE( !stringFieldVar.IsBoolean() );
    ASSERT_TRUE( stringFieldVar.IsString() );
    ASSERT_TRUE( stringFieldVar.GetTypeId() == GUCEF_DATATYPE_UTF8_STRING );
    ASSERT_TRUE( stringFieldVar.AsString() == "this is a string" );
    const CORE::CVariant& bytesFieldVar = documentNode.GetAttributeValueOrChildValueByName( "bytes_field" );
    ASSERT_TRUE( bytesFieldVar.IsInitialized() );
    ASSERT_TRUE( !bytesFieldVar.IsInteger() );
    ASSERT_TRUE( !bytesFieldVar.IsFloat() );
    ASSERT_TRUE( !bytesFieldVar.IsBoolean() );
    ASSERT_TRUE( !bytesFieldVar.IsString() );
    ASSERT_TRUE( bytesFieldVar.IsBlob() );
    ASSERT_TRUE( bytesFieldVar.GetTypeId() == GUCEF_DATATYPE_BINARY_BLOB );
    ASSERT_TRUE( bytesFieldVar.ByteSize() == 0 );
   
}

/*-------------------------------------------------------------------------*/

void
PerformProtobufTest_NestedMessage( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();

    CORE::CDynamicBuffer encodedData;
    encodedData.CopyAndDecodeBase64From( PROTO_NESTED_MESSAGE_BASE64, 0 );
    CORE::CDynamicBufferAccess encodedDataAccess( encodedData );

    // First just fetch the DStoreCodec like any other regular codec
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr protobufCodec_NestedMessage;
    ASSERT_TRUE( codecRegistry.TryLookup( ddCodecTypeNameForNestedMessage, protobufCodec_NestedMessage, false ) );

    // Now we can use the codec to decode the test message

    CORE::CDataNode documentNode;
    ASSERT_TRUE( protobufCodec_NestedMessage->BuildDataTree( &documentNode, &encodedDataAccess ) );
    
    ASSERT_TRUE( documentNode.GetName() == "NestedMessage" );    

    const CORE::CVariant& intIDFieldVar = documentNode.GetAttributeValueOrChildValueByName( "id" );
    ASSERT_TRUE( intIDFieldVar.IsInitialized() );
    ASSERT_TRUE( intIDFieldVar.IsInteger() );
    ASSERT_TRUE( intIDFieldVar.IsSignedInteger() );
    ASSERT_TRUE( intIDFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT32 );
    ASSERT_TRUE( intIDFieldVar.AsInt32() == 777 );
    
    ASSERT_TRUE( documentNode.GetNrOfChildNodes() == 1 );    
    CORE::CDataNode* innerMsgNode = documentNode.FindChild( "inner_message", true );
    ASSERT_TRUE( innerMsgNode != GUCEF_NULL );
    ASSERT_TRUE( innerMsgNode->GetName() == "inner_message" );

    const CORE::CVariant& intInnerIDFieldVar = innerMsgNode->GetAttributeValueOrChildValueByName( "inner_id" );
    ASSERT_TRUE( intInnerIDFieldVar.IsInitialized() );
    ASSERT_TRUE( intInnerIDFieldVar.IsInteger() );
    ASSERT_TRUE( intInnerIDFieldVar.IsSignedInteger() );
    ASSERT_TRUE( intInnerIDFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT32 );
    ASSERT_TRUE( intInnerIDFieldVar.AsInt32() == 747 );    

    const CORE::CVariant& intInnerNameFieldVar = innerMsgNode->GetAttributeValueOrChildValueByName( "inner_name" );
    ASSERT_TRUE( intInnerNameFieldVar.IsInitialized() );
    ASSERT_TRUE( !intInnerNameFieldVar.IsInteger() );
    ASSERT_TRUE( intInnerNameFieldVar.IsString() );
    ASSERT_TRUE( intInnerNameFieldVar.GetTypeId() == GUCEF_DATATYPE_UTF8_STRING );
    ASSERT_TRUE( intInnerNameFieldVar.AsString() == "this is the inner message" );

}

/*-------------------------------------------------------------------------*/

void
PerformProtobufTest_ComplexMessage( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();

    CORE::CDynamicBuffer encodedData;
    encodedData.CopyAndDecodeBase64From( PROTO_COMPLEX_MESSAGE_BASE64, 0 );
    CORE::CDynamicBufferAccess encodedDataAccess( encodedData );

    // First just fetch the DStoreCodec like any other regular codec
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr protobufCodec_ComplexMessage;
    ASSERT_TRUE( codecRegistry.TryLookup( ddCodecTypeNameForComplexMessage, protobufCodec_ComplexMessage, false ) );

    // Now we can use the codec to decode the test message

    CORE::CDataNode documentNode;
    ASSERT_TRUE( protobufCodec_ComplexMessage->BuildDataTree( &documentNode, &encodedDataAccess ) );
    
    ASSERT_TRUE( documentNode.GetName() == "ComplexMessage" );    
    ASSERT_TRUE( documentNode.GetNrOfDirectChildNodes() == 5 );
    ASSERT_TRUE( documentNode.GetAttCount() == 0 );

    // test the packed array structure
    
    const CORE::CDataNode* idArrayNode = documentNode.FindChild( "ids" );
    ASSERT_TRUE( GUCEF_NULL != idArrayNode );
    ASSERT_TRUE( idArrayNode->GetNodeType() == GUCEF_DATATYPE_ARRAY );
    ASSERT_TRUE( idArrayNode->GetNrOfChildNodes() == 5 );
    ASSERT_TRUE( idArrayNode->GetNrOfDirectChildNodes() == 5 );    
    CORE::CDataNode::TVariantVector values = idArrayNode->GetChildrenValues();
    ASSERT_TRUE( !values.empty() );
    ASSERT_TRUE( values.size() == 5 );
    for ( int i=0; i<5; ++i )
    {    
        ASSERT_TRUE( values[ i ].IsInteger() );
        ASSERT_TRUE( values[ i ].IsSignedInteger() );
        ASSERT_TRUE( values[ i ] > 0 && values[ i ] < 6 );
    }

    // test the map structure

    const CORE::CDataNode* nameToIdMapNode = documentNode.FindChild( "name_to_id" );
    ASSERT_TRUE( GUCEF_NULL != nameToIdMapNode );
    ASSERT_TRUE( nameToIdMapNode->GetNodeType() == GUCEF_DATATYPE_MAP );
    ASSERT_TRUE( nameToIdMapNode->GetAttCount() == 2 );
    
    const CORE::CVariant& mapEntryAliceFieldVar = nameToIdMapNode->GetAttributeValueOrChildValueByName( "Alice" );
    ASSERT_TRUE( mapEntryAliceFieldVar.IsInitialized() );
    ASSERT_TRUE( mapEntryAliceFieldVar.IsInteger() );
    ASSERT_TRUE( mapEntryAliceFieldVar.IsSignedInteger() );
    ASSERT_TRUE( mapEntryAliceFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT32 );
    ASSERT_TRUE( mapEntryAliceFieldVar.AsInt32() == 1 );    
    
    const CORE::CVariant& mapEntryBobFieldVar = nameToIdMapNode->GetAttributeValueOrChildValueByName( "Bob" );
    ASSERT_TRUE( mapEntryBobFieldVar.IsInitialized() );
    ASSERT_TRUE( mapEntryBobFieldVar.IsInteger() );
    ASSERT_TRUE( mapEntryBobFieldVar.IsSignedInteger() );
    ASSERT_TRUE( mapEntryBobFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT32 );
    ASSERT_TRUE( mapEntryBobFieldVar.AsInt32() == 2 ); 

    // test enum field

    const CORE::CDataNode* statusEnumNode = documentNode.FindChild( "status" );
    ASSERT_TRUE( GUCEF_NULL != statusEnumNode );
    ASSERT_TRUE( statusEnumNode->GetNodeType() == GUCEF_DATATYPE_ENUM );
    ASSERT_TRUE( statusEnumNode->GetAttCount() == 0 );
    ASSERT_TRUE( statusEnumNode->GetNrOfChildNodes() == 0 );
    ASSERT_TRUE( statusEnumNode->GetName() == "status" );
    const CORE::CVariant& statusFieldVar = statusEnumNode->GetValue();
    ASSERT_TRUE( statusFieldVar.IsInitialized() );
    ASSERT_TRUE( statusFieldVar.IsInteger() );
    ASSERT_TRUE( statusFieldVar.IsSignedInteger() );
    ASSERT_TRUE( statusFieldVar.GetTypeId() == GUCEF_DATATYPE_LE_INT32 );
    ASSERT_TRUE( statusFieldVar.AsInt32() == 1 );
    const CORE::CVariant& sameStatusFieldVar = documentNode.GetAttributeValueOrChildValueByName( "status" );
    ASSERT_TRUE( statusFieldVar == sameStatusFieldVar );

    // test the nested optional message

    const CORE::CDataNode* nestedOptMsgNode = documentNode.FindChild( "nested_optional_message" );
    ASSERT_TRUE( GUCEF_NULL != nestedOptMsgNode );
    ASSERT_TRUE( nestedOptMsgNode->GetNodeType() == GUCEF_DATATYPE_OBJECT );
    ASSERT_TRUE( nestedOptMsgNode->GetNrOfChildNodes() == 0 );
    ASSERT_TRUE( nestedOptMsgNode->GetName() == "nested_optional_message" );
    ASSERT_TRUE( nestedOptMsgNode->GetAttCount() == 2 );
    const CORE::CVariant& nestedOptMsgField1Var = nestedOptMsgNode->GetAttributeValueOrChildValueByName( "nested_optional_field_1" );
    ASSERT_TRUE( nestedOptMsgField1Var.IsInitialized() );
    ASSERT_TRUE( !nestedOptMsgField1Var.IsInteger() );
    ASSERT_TRUE( nestedOptMsgField1Var.IsString() );
    ASSERT_TRUE( nestedOptMsgField1Var.GetTypeId() == GUCEF_DATATYPE_UTF8_STRING );
    ASSERT_TRUE( nestedOptMsgField1Var.AsString() == "optional nested value" );
    const CORE::CVariant& nestedOptMsgField2Var = nestedOptMsgNode->GetAttributeValueOrChildValueByName( "nested_optional_field_2" );
    ASSERT_TRUE( nestedOptMsgField2Var.IsInitialized() );
    ASSERT_TRUE( !nestedOptMsgField2Var.IsInteger() );
    ASSERT_TRUE( !nestedOptMsgField2Var.IsSignedInteger() );
    ASSERT_TRUE( nestedOptMsgField2Var.IsFloat() );
    ASSERT_TRUE( nestedOptMsgField2Var.GetTypeId() == GUCEF_DATATYPE_LE_FLOAT64 );
    CORE::CVariant compareFloat64( static_cast< Float64 >( 3.14159 ) );
    ASSERT_TRUE( nestedOptMsgField2Var == compareFloat64 );

    // test the repeated nested messages

    const CORE::CDataNode* nestedMsgArrayNode = documentNode.FindChild( "nested_messages" );
    ASSERT_TRUE( GUCEF_NULL != nestedMsgArrayNode );
    ASSERT_TRUE( nestedMsgArrayNode->GetNodeType() == GUCEF_DATATYPE_ARRAY );
    ASSERT_TRUE( nestedMsgArrayNode->GetNrOfChildNodes() == 2 );

}

/*-------------------------------------------------------------------------*/

void
PerformProtobufTest_SerializeAllTypesMessage( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr protobufCodec_AllTypesMessage;
    ASSERT_TRUE( codecRegistry.TryLookup( ddCodecTypeNameForAllTypesMessage, protobufCodec_AllTypesMessage, false ) );

    // Build a CDataNode tree with the same values used in the deserialization test

    CORE::CDataNode sourceNode( "AllTypesMessage", GUCEF_DATATYPE_OBJECT );
    sourceNode.SetAttribute( "int_field", CORE::CVariant( static_cast< Int32 >( -44 ) ) );
    sourceNode.SetAttribute( "long_field", CORE::CVariant( static_cast< Int64 >( -3455 ) ) );
    sourceNode.SetAttribute( "uint_field", CORE::CVariant( static_cast< UInt32 >( 3453577 ) ) );
    sourceNode.SetAttribute( "ulong_field", CORE::CVariant( static_cast< UInt64 >( 3557878 ) ) );
    sourceNode.SetAttribute( "sint_field", CORE::CVariant( static_cast< Int32 >( 97676 ) ) );
    sourceNode.SetAttribute( "slong_field", CORE::CVariant( static_cast< Int64 >( 868680 ) ) );
    sourceNode.SetAttribute( "fixed_field", CORE::CVariant( static_cast< UInt32 >( 5560 ) ) );
    sourceNode.SetAttribute( "lfixed_field", CORE::CVariant( static_cast< UInt64 >( 7670 ) ) );
    sourceNode.SetAttribute( "sfixed_field", CORE::CVariant( static_cast< Int32 >( -670 ) ) );
    sourceNode.SetAttribute( "slfixed_field", CORE::CVariant( static_cast< Int64 >( -67077899 ) ) );
    sourceNode.SetAttribute( "float_field", CORE::CVariant( static_cast< Float32 >( 0.454535f ) ) );
    sourceNode.SetAttribute( "double_field", CORE::CVariant( static_cast< Float64 >( 0.3543534534 ) ) );
    sourceNode.SetAttribute( "bool_field", CORE::CVariant( static_cast< Int32 >( 0 ) ) );
    sourceNode.SetAttribute( "string_field", CORE::CVariant( CORE::CString( "this is a string" ) ) );
    CORE::CDynamicBuffer emptyBlobBuffer;
    sourceNode.SetAttribute( "bytes_field", CORE::CVariant( emptyBlobBuffer ) );

    // Serialize to protobuf binary

    CORE::CDynamicBuffer serializedData;
    CORE::CDynamicBufferAccess serializedDataAccess( serializedData );
    ASSERT_TRUE( protobufCodec_AllTypesMessage->StoreDataTree( &sourceNode, &serializedDataAccess ) );
    ASSERT_TRUE( serializedData.GetDataSize() > 0 );

    // Deserialize the serialized data back into a new CDataNode tree to verify round-trip

    serializedDataAccess.Setpos( 0 );
    CORE::CDataNode roundTripNode;
    ASSERT_TRUE( protobufCodec_AllTypesMessage->BuildDataTree( &roundTripNode, &serializedDataAccess ) );

    ASSERT_TRUE( roundTripNode.GetName() == "AllTypesMessage" );

    const CORE::CVariant& rtIntField = roundTripNode.GetAttributeValueOrChildValueByName( "int_field" );
    ASSERT_TRUE( rtIntField.IsInitialized() );
    ASSERT_TRUE( rtIntField.AsInt32() == -44 );

    const CORE::CVariant& rtLongField = roundTripNode.GetAttributeValueOrChildValueByName( "long_field" );
    ASSERT_TRUE( rtLongField.IsInitialized() );
    ASSERT_TRUE( rtLongField.AsInt64() == -3455 );

    const CORE::CVariant& rtUintField = roundTripNode.GetAttributeValueOrChildValueByName( "uint_field" );
    ASSERT_TRUE( rtUintField.IsInitialized() );
    ASSERT_TRUE( rtUintField.AsUInt32() == 3453577 );

    const CORE::CVariant& rtUlongField = roundTripNode.GetAttributeValueOrChildValueByName( "ulong_field" );
    ASSERT_TRUE( rtUlongField.IsInitialized() );
    ASSERT_TRUE( rtUlongField.AsUInt64() == 3557878 );

    const CORE::CVariant& rtSintField = roundTripNode.GetAttributeValueOrChildValueByName( "sint_field" );
    ASSERT_TRUE( rtSintField.IsInitialized() );
    ASSERT_TRUE( rtSintField.AsInt32() == 97676 );

    const CORE::CVariant& rtSlongField = roundTripNode.GetAttributeValueOrChildValueByName( "slong_field" );
    ASSERT_TRUE( rtSlongField.IsInitialized() );
    ASSERT_TRUE( rtSlongField.AsInt64() == 868680 );

    const CORE::CVariant& rtFixedField = roundTripNode.GetAttributeValueOrChildValueByName( "fixed_field" );
    ASSERT_TRUE( rtFixedField.IsInitialized() );
    ASSERT_TRUE( rtFixedField.AsUInt32() == 5560 );

    const CORE::CVariant& rtLfixedField = roundTripNode.GetAttributeValueOrChildValueByName( "lfixed_field" );
    ASSERT_TRUE( rtLfixedField.IsInitialized() );
    ASSERT_TRUE( rtLfixedField.AsUInt64() == 7670 );

    const CORE::CVariant& rtSfixedField = roundTripNode.GetAttributeValueOrChildValueByName( "sfixed_field" );
    ASSERT_TRUE( rtSfixedField.IsInitialized() );
    ASSERT_TRUE( rtSfixedField.AsInt32() == -670 );

    const CORE::CVariant& rtSlfixedField = roundTripNode.GetAttributeValueOrChildValueByName( "slfixed_field" );
    ASSERT_TRUE( rtSlfixedField.IsInitialized() );
    ASSERT_TRUE( rtSlfixedField.AsInt64() == -67077899 );

    const CORE::CVariant& rtFloatField = roundTripNode.GetAttributeValueOrChildValueByName( "float_field" );
    ASSERT_TRUE( rtFloatField.IsInitialized() );
    ASSERT_TRUE( rtFloatField.IsFloat() );
    CORE::CVariant rtCompareFloat32( static_cast< Float32 >( 0.454535f ) );
    ASSERT_TRUE( rtFloatField == rtCompareFloat32 );

    const CORE::CVariant& rtDoubleField = roundTripNode.GetAttributeValueOrChildValueByName( "double_field" );
    ASSERT_TRUE( rtDoubleField.IsInitialized() );
    ASSERT_TRUE( rtDoubleField.IsFloat() );
    CORE::CVariant rtCompareFloat64( static_cast< Float64 >( 0.3543534534 ) );
    ASSERT_TRUE( rtDoubleField == rtCompareFloat64 );

    const CORE::CVariant& rtBoolField = roundTripNode.GetAttributeValueOrChildValueByName( "bool_field" );
    ASSERT_TRUE( rtBoolField.IsInitialized() );
    ASSERT_TRUE( rtBoolField == false );

    const CORE::CVariant& rtStringField = roundTripNode.GetAttributeValueOrChildValueByName( "string_field" );
    ASSERT_TRUE( rtStringField.IsInitialized() );
    ASSERT_TRUE( rtStringField.AsString() == "this is a string" );

    const CORE::CVariant& rtBytesField = roundTripNode.GetAttributeValueOrChildValueByName( "bytes_field" );
    ASSERT_TRUE( rtBytesField.IsInitialized() );
    ASSERT_TRUE( rtBytesField.IsBlob() );
    ASSERT_TRUE( rtBytesField.ByteSize() == 0 );
}

/*-------------------------------------------------------------------------*/

void
PerformProtobufTest_SerializeNestedMessage( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr protobufCodec_NestedMessage;
    ASSERT_TRUE( codecRegistry.TryLookup( ddCodecTypeNameForNestedMessage, protobufCodec_NestedMessage, false ) );

    // Build a CDataNode tree matching the nested message test values

    CORE::CDataNode sourceNode( "NestedMessage", GUCEF_DATATYPE_OBJECT );
    sourceNode.SetAttribute( "id", CORE::CVariant( static_cast< Int32 >( 777 ) ) );

    CORE::CDataNode* innerMsgNode = sourceNode.AddChild( "inner_message", GUCEF_DATATYPE_OBJECT );
    ASSERT_TRUE( GUCEF_NULL != innerMsgNode );
    innerMsgNode->SetAttribute( "inner_id", CORE::CVariant( static_cast< Int32 >( 747 ) ) );
    innerMsgNode->SetAttribute( "inner_name", CORE::CVariant( CORE::CString( "this is the inner message" ) ) );

    // Serialize to protobuf binary

    CORE::CDynamicBuffer serializedData;
    CORE::CDynamicBufferAccess serializedDataAccess( serializedData );
    ASSERT_TRUE( protobufCodec_NestedMessage->StoreDataTree( &sourceNode, &serializedDataAccess ) );
    ASSERT_TRUE( serializedData.GetDataSize() > 0 );

    // Deserialize the serialized data back into a new CDataNode tree to verify round-trip

    serializedDataAccess.Setpos( 0 );
    CORE::CDataNode roundTripNode;
    ASSERT_TRUE( protobufCodec_NestedMessage->BuildDataTree( &roundTripNode, &serializedDataAccess ) );

    ASSERT_TRUE( roundTripNode.GetName() == "NestedMessage" );

    const CORE::CVariant& rtIdField = roundTripNode.GetAttributeValueOrChildValueByName( "id" );
    ASSERT_TRUE( rtIdField.IsInitialized() );
    ASSERT_TRUE( rtIdField.AsInt32() == 777 );

    ASSERT_TRUE( roundTripNode.GetNrOfChildNodes() == 1 );
    CORE::CDataNode* rtInnerMsgNode = roundTripNode.FindChild( "inner_message", true );
    ASSERT_TRUE( rtInnerMsgNode != GUCEF_NULL );
    ASSERT_TRUE( rtInnerMsgNode->GetName() == "inner_message" );

    const CORE::CVariant& rtInnerIdField = rtInnerMsgNode->GetAttributeValueOrChildValueByName( "inner_id" );
    ASSERT_TRUE( rtInnerIdField.IsInitialized() );
    ASSERT_TRUE( rtInnerIdField.AsInt32() == 747 );

    const CORE::CVariant& rtInnerNameField = rtInnerMsgNode->GetAttributeValueOrChildValueByName( "inner_name" );
    ASSERT_TRUE( rtInnerNameField.IsInitialized() );
    ASSERT_TRUE( rtInnerNameField.IsString() );
    ASSERT_TRUE( rtInnerNameField.AsString() == "this is the inner message" );
}

/*-------------------------------------------------------------------------*/

void
PerformProtobufTest_SerializeComplexMessage( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr protobufCodec_ComplexMessage;
    ASSERT_TRUE( codecRegistry.TryLookup( ddCodecTypeNameForComplexMessage, protobufCodec_ComplexMessage, false ) );

    // Build a CDataNode tree matching the complex message test values

    CORE::CDataNode sourceNode( "ComplexMessage", GUCEF_DATATYPE_OBJECT );

    // repeated int32 ids = [1, 2, 3, 4, 5]
    CORE::CDataNode* idsArrayNode = sourceNode.AddChild( "ids", GUCEF_DATATYPE_ARRAY );
    ASSERT_TRUE( GUCEF_NULL != idsArrayNode );
    for ( Int32 i = 1; i <= 5; ++i )
    {
        idsArrayNode->AddValueAsChild( CORE::CVariant( i ) );
    }

    // map<string, int32> name_to_id = { "Alice": 1, "Bob": 2 }
    CORE::CDataNode* nameToIdMapNode = sourceNode.AddChild( "name_to_id", GUCEF_DATATYPE_MAP );
    ASSERT_TRUE( GUCEF_NULL != nameToIdMapNode );
    nameToIdMapNode->SetAttribute( "Alice", CORE::CVariant( static_cast< Int32 >( 1 ) ) );
    nameToIdMapNode->SetAttribute( "Bob", CORE::CVariant( static_cast< Int32 >( 2 ) ) );

    // enum Status status = ACTIVE (1)
    CORE::CDataNode* statusNode = sourceNode.AddChild( "status", GUCEF_DATATYPE_ENUM );
    ASSERT_TRUE( GUCEF_NULL != statusNode );
    statusNode->SetValue( CORE::CVariant( static_cast< Int32 >( 1 ) ) );

    // nested_optional_message (oneof)
    CORE::CDataNode* nestedOptMsgNode = sourceNode.AddChild( "nested_optional_message", GUCEF_DATATYPE_OBJECT );
    ASSERT_TRUE( GUCEF_NULL != nestedOptMsgNode );
    nestedOptMsgNode->SetAttribute( "nested_optional_field_1", CORE::CVariant( CORE::CString( "optional nested value" ) ) );
    nestedOptMsgNode->SetAttribute( "nested_optional_field_2", CORE::CVariant( static_cast< Float64 >( 3.14159 ) ) );

    // repeated NestedMessage nested_messages
    CORE::CDataNode* nestedMsgsArrayNode = sourceNode.AddChild( "nested_messages", GUCEF_DATATYPE_ARRAY );
    ASSERT_TRUE( GUCEF_NULL != nestedMsgsArrayNode );

    CORE::CDataNode* nestedMsg1 = nestedMsgsArrayNode->AddChild( "NestedMessage", GUCEF_DATATYPE_OBJECT );
    ASSERT_TRUE( GUCEF_NULL != nestedMsg1 );
    nestedMsg1->SetAttribute( "inner_id", CORE::CVariant( static_cast< Int32 >( 10 ) ) );
    nestedMsg1->SetAttribute( "inner_name", CORE::CVariant( CORE::CString( "nested inner 1" ) ) );
    nestedMsg1->SetAttribute( "inner_timestamp", CORE::CVariant( static_cast< UInt64 >( 345386786767ULL ) ) );

    CORE::CDataNode* nestedMsg2 = nestedMsgsArrayNode->AddChild( "NestedMessage", GUCEF_DATATYPE_OBJECT );
    ASSERT_TRUE( GUCEF_NULL != nestedMsg2 );
    nestedMsg2->SetAttribute( "inner_id", CORE::CVariant( static_cast< Int32 >( 20 ) ) );
    nestedMsg2->SetAttribute( "inner_name", CORE::CVariant( CORE::CString( "nested inner 2" ) ) );
    nestedMsg2->SetAttribute( "inner_timestamp", CORE::CVariant( static_cast< UInt64 >( 334435534534ULL ) ) );

    // Serialize to protobuf binary

    CORE::CDynamicBuffer serializedData;
    CORE::CDynamicBufferAccess serializedDataAccess( serializedData );
    ASSERT_TRUE( protobufCodec_ComplexMessage->StoreDataTree( &sourceNode, &serializedDataAccess ) );
    ASSERT_TRUE( serializedData.GetDataSize() > 0 );

    // Deserialize the serialized data back into a new CDataNode tree to verify round-trip

    serializedDataAccess.Setpos( 0 );
    CORE::CDataNode roundTripNode;
    ASSERT_TRUE( protobufCodec_ComplexMessage->BuildDataTree( &roundTripNode, &serializedDataAccess ) );

    ASSERT_TRUE( roundTripNode.GetName() == "ComplexMessage" );
    ASSERT_TRUE( roundTripNode.GetNrOfDirectChildNodes() == 5 );

    // Verify the packed array round-trip

    const CORE::CDataNode* rtIdsArrayNode = roundTripNode.FindChild( "ids" );
    ASSERT_TRUE( GUCEF_NULL != rtIdsArrayNode );
    ASSERT_TRUE( rtIdsArrayNode->GetNodeType() == GUCEF_DATATYPE_ARRAY );
    ASSERT_TRUE( rtIdsArrayNode->GetNrOfDirectChildNodes() == 5 );
    CORE::CDataNode::TVariantVector rtIdValues = rtIdsArrayNode->GetChildrenValues();
    ASSERT_TRUE( rtIdValues.size() == 5 );
    for ( int i = 0; i < 5; ++i )
    {
        ASSERT_TRUE( rtIdValues[ i ].IsInteger() );
        ASSERT_TRUE( rtIdValues[ i ] > 0 && rtIdValues[ i ] < 6 );
    }

    // Verify the map round-trip

    const CORE::CDataNode* rtNameToIdMapNode = roundTripNode.FindChild( "name_to_id" );
    ASSERT_TRUE( GUCEF_NULL != rtNameToIdMapNode );
    ASSERT_TRUE( rtNameToIdMapNode->GetNodeType() == GUCEF_DATATYPE_MAP );
    ASSERT_TRUE( rtNameToIdMapNode->GetAttCount() == 2 );

    const CORE::CVariant& rtAlice = rtNameToIdMapNode->GetAttributeValueOrChildValueByName( "Alice" );
    ASSERT_TRUE( rtAlice.IsInitialized() );
    ASSERT_TRUE( rtAlice.AsInt32() == 1 );

    const CORE::CVariant& rtBob = rtNameToIdMapNode->GetAttributeValueOrChildValueByName( "Bob" );
    ASSERT_TRUE( rtBob.IsInitialized() );
    ASSERT_TRUE( rtBob.AsInt32() == 2 );

    // Verify the enum round-trip

    const CORE::CDataNode* rtStatusNode = roundTripNode.FindChild( "status" );
    ASSERT_TRUE( GUCEF_NULL != rtStatusNode );
    ASSERT_TRUE( rtStatusNode->GetNodeType() == GUCEF_DATATYPE_ENUM );
    const CORE::CVariant& rtStatusValue = rtStatusNode->GetValue();
    ASSERT_TRUE( rtStatusValue.IsInitialized() );
    ASSERT_TRUE( rtStatusValue.AsInt32() == 1 );

    // Verify the nested optional message round-trip

    const CORE::CDataNode* rtNestedOptMsgNode = roundTripNode.FindChild( "nested_optional_message" );
    ASSERT_TRUE( GUCEF_NULL != rtNestedOptMsgNode );
    ASSERT_TRUE( rtNestedOptMsgNode->GetNodeType() == GUCEF_DATATYPE_OBJECT );
    ASSERT_TRUE( rtNestedOptMsgNode->GetAttCount() == 2 );

    const CORE::CVariant& rtOptField1 = rtNestedOptMsgNode->GetAttributeValueOrChildValueByName( "nested_optional_field_1" );
    ASSERT_TRUE( rtOptField1.IsInitialized() );
    ASSERT_TRUE( rtOptField1.AsString() == "optional nested value" );

    const CORE::CVariant& rtOptField2 = rtNestedOptMsgNode->GetAttributeValueOrChildValueByName( "nested_optional_field_2" );
    ASSERT_TRUE( rtOptField2.IsInitialized() );
    ASSERT_TRUE( rtOptField2.IsFloat() );
    CORE::CVariant rtCompareFloat64( static_cast< Float64 >( 3.14159 ) );
    ASSERT_TRUE( rtOptField2 == rtCompareFloat64 );

    // Verify the repeated nested messages round-trip

    const CORE::CDataNode* rtNestedMsgsArrayNode = roundTripNode.FindChild( "nested_messages" );
    ASSERT_TRUE( GUCEF_NULL != rtNestedMsgsArrayNode );
    ASSERT_TRUE( rtNestedMsgsArrayNode->GetNodeType() == GUCEF_DATATYPE_ARRAY );
    ASSERT_TRUE( rtNestedMsgsArrayNode->GetNrOfChildNodes() == 2 );
}

/*-------------------------------------------------------------------------*/

void
PerformProtobufTest_ConfigDrivenCodecSpecification( void )
{GUCEF_TRACE;

    try
    {
        CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
        CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
        CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();
        CORE::CDStoreCodecPluginManager& codecPluginManager = coreGlobal->GetDStoreCodecPluginManager();
        CORE::CDataDrivenDStoreCodecFactory& ddCodecFactory = coreGlobal->GetDataDrivenDStoreCodecFactory();

        ddCodecFactory.UnregisterAll();
        
        // We dont want to test the entire global config loading but we know that the data driven data storage 
        // codec factory is actually a global factory so we can just test that subset of the config loading

        CORE::CDataNode ddCodecFactoryConfig( "GlobalDataDrivenDStoreCodecFactory", GUCEF_DATATYPE_OBJECT );
        ddCodecFactoryConfig.SetAttribute( "autoInstantiateShareableCodecs", true );

        CORE::CUri allTypesUri;
        ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( allTypesUri, "application/test-protobuf-AllTypesMessage", PROTO_DEFINITION_ALL_TYPES, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    

        CORE::CDataNode* codecConfigMetaNode = ddCodecFactoryConfig.AddChild( "codecMeta", GUCEF_DATATYPE_ARRAY );
        ASSERT_TRUE( GUCEF_NULL != codecConfigMetaNode );

        CORE::CDataNode* allTypesCodecConfigNode = codecConfigMetaNode->AddChild( "codecMetaDefinition", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( GUCEF_NULL != allTypesCodecConfigNode );
        allTypesCodecConfigNode->SetAttribute( "dataDrivenCodecTypeName", ddCodecTypeNameForAllTypesMessage );
        allTypesCodecConfigNode->SetAttribute( "baseCodecTypeName", "protobuf" );
        allTypesCodecConfigNode->SetAttribute( "isShareable", "true" );
        CORE::CDataNode* allTypesCodecResourcesConfigNode = allTypesCodecConfigNode->AddChild( "resources", GUCEF_DATATYPE_ARRAY );
        ASSERT_TRUE( GUCEF_NULL != allTypesCodecResourcesConfigNode );
        ASSERT_TRUE( GUCEF_NULL != allTypesCodecResourcesConfigNode->AddValueAsChild( allTypesUri ) );

        CORE::CUri nestedMsgUri;
        ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( nestedMsgUri, "application/test-protobuf-NestedMessage", PROTO_DEFINITION_NESTED, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    

        CORE::CDataNode* nestedMsgConfigNode = codecConfigMetaNode->AddChild( "codecMetaDefinition", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( GUCEF_NULL != nestedMsgConfigNode );
        nestedMsgConfigNode->SetAttribute( "dataDrivenCodecTypeName", ddCodecTypeNameForNestedMessage );
        nestedMsgConfigNode->SetAttribute( "baseCodecTypeName", "protobuf" );
        nestedMsgConfigNode->SetAttribute( "isShareable", "true" );
        CORE::CDataNode* nestedMsgCodecResourcesConfigNode = nestedMsgConfigNode->AddChild( "resources", GUCEF_DATATYPE_ARRAY );
        ASSERT_TRUE( GUCEF_NULL != nestedMsgCodecResourcesConfigNode );
        ASSERT_TRUE( GUCEF_NULL != nestedMsgCodecResourcesConfigNode->AddValueAsChild( nestedMsgUri ) );

        CORE::CUri complexMsgUri;
        ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( complexMsgUri, "application/test-protobuf-ComplexMessage", PROTO_DEFINITION_COMPLEX, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    

        CORE::CDataNode* complexMsgCodecConfigNode = codecConfigMetaNode->AddChild( "codecMetaDefinition", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( GUCEF_NULL != complexMsgCodecConfigNode );
        complexMsgCodecConfigNode->SetAttribute( "dataDrivenCodecTypeName", ddCodecTypeNameForComplexMessage );
        complexMsgCodecConfigNode->SetAttribute( "baseCodecTypeName", "protobuf" );
        complexMsgCodecConfigNode->SetAttribute( "isShareable", "true" );
        CORE::CDataNode* complexMsgCodecResourcesConfigNode = complexMsgCodecConfigNode->AddChild( "resources", GUCEF_DATATYPE_ARRAY );
        ASSERT_TRUE( GUCEF_NULL != complexMsgCodecResourcesConfigNode );
        ASSERT_TRUE( GUCEF_NULL != complexMsgCodecResourcesConfigNode->AddValueAsChild( complexMsgUri ) );


        ASSERT_TRUE( ddCodecFactory.LoadConfig( ddCodecFactoryConfig ) );

    }
    catch( ... )
    {
        ERRORHERE;
    }
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

            // Then we can test the nested message, another fundamental feature of protobuf
            // slightly more complicated due to nesting vs just a flat structure
            PerformProtobufTest_NestedMessage();

            // Finally we can test a more complex message with repeated fields, maps and oneof
            PerformProtobufTest_ComplexMessage();

            // Now test serialization (writing) to protobuf and round-trip verification
            PerformProtobufTest_SerializeAllTypesMessage();
            PerformProtobufTest_SerializeNestedMessage();
            PerformProtobufTest_SerializeComplexMessage();

            // Also test the config driven codec specification instead of direct hardcoding
            PerformProtobufTest_ConfigDrivenCodecSpecification();

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
