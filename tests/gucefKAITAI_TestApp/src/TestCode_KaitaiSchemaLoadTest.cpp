/*
 *  gucefKAITAI: Platform module supporting the Kaitai binary description format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#include <vector>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"          /* GUCEF CORE library API */
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_KAITAI_H
#include "gucefKAITAI.h"
#define GUCEF_KAITAI_H
#endif /* GUCEF_KAITAI_H ? */

#ifdef GUCEF_MSWIN_BUILD
  #include <windows.h>
#else
  #include <assert.h>
#endif

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
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString kaitaiSchema_core_dynamic =
"meta:\n"
"  id: gucef_core_dynamic\n"
"  endian: le\n"
"seq:\n"
"  - id: len_data\n"
"    type: u4\n"
"  - id: data\n"
"    size: len_data\n";

const CORE::CString kaitaiSchema_core_variant =
"meta:\n"
"  id: gucef_core_variant\n"
"  endian: le\n"
"  imports:\n"
"  - gucef_core_dynamic\n"
"seq:\n"
"  - id: type_id\n"
"    type: u1\n"
"    enum: gucef_datatypes\n"
"  - id: payload\n"
"    type:\n"
"      switch-on: type_id\n"
"      cases:\n"
"        'gucef_datatypes::uint8': gucef_uint8\n"
"        'gucef_datatypes::int8': gucef_int8\n"
"        'gucef_datatypes::le_uint16': gucef_le_uint16\n"
"        'gucef_datatypes::be_uint16': gucef_be_uint16\n"
"        'gucef_datatypes::le_int16': gucef_le_int16\n"
"        'gucef_datatypes::be_int16': gucef_be_int16\n"
"        'gucef_datatypes::le_uint32': gucef_le_uint32\n"
"        'gucef_datatypes::be_uint32': gucef_be_uint32\n"
"        'gucef_datatypes::le_int32': gucef_le_int32\n"
"        'gucef_datatypes::be_int32': gucef_be_int32\n"
"        'gucef_datatypes::le_uint64': gucef_le_uint64\n"
"        'gucef_datatypes::be_uint64': gucef_be_uint64\n"
"        'gucef_datatypes::le_int64': gucef_le_int64\n"
"        'gucef_datatypes::be_int64': gucef_be_int64\n"
"        'gucef_datatypes::le_float32': gucef_le_float32\n"
"        'gucef_datatypes::be_float32': gucef_be_float32\n"
"        'gucef_datatypes::le_float64': gucef_le_float64\n"
"        'gucef_datatypes::be_float64': gucef_be_float64\n"
"        'gucef_datatypes::le_boolean_int32': gucef_le_boolean_int32\n"
"        'gucef_datatypes::be_boolean_int32': gucef_be_boolean_int32\n"
"        'gucef_datatypes::binary_bsob': gucef_bsob\n"
"        'gucef_datatypes::binary_blob': gucef_core_dynamic\n"
"        'gucef_datatypes::le_timestamp_in_secs_since_unix_epoch': gucef_le_timestamp_in_secs_since_unix_epoch\n"
"        'gucef_datatypes::be_timestamp_in_secs_since_unix_epoch': gucef_be_timestamp_in_secs_since_unix_epoch\n"
"        'gucef_datatypes::le_timestamp_in_ms_since_unix_epoch': gucef_le_timestamp_in_ms_since_unix_epoch\n"
"        'gucef_datatypes::be_timestamp_in_ms_since_unix_epoch': gucef_be_timestamp_in_ms_since_unix_epoch\n"
"        'gucef_datatypes::boolean_ascii_string': gucef_core_dynamic\n"
"        'gucef_datatypes::boolean_utf8_string': gucef_core_dynamic\n"
"        'gucef_datatypes::ascii_string': gucef_core_dynamic\n"
"        'gucef_datatypes::utf8_string': gucef_core_dynamic\n"
"        'gucef_datatypes::utf16_le_string': gucef_core_dynamic\n"
"        'gucef_datatypes::utf16_be_string': gucef_core_dynamic\n"
"        'gucef_datatypes::utf32_string': gucef_core_dynamic\n"
"        'gucef_datatypes::datetime_iso8601_ascii_string': gucef_core_dynamic\n"
"        'gucef_datatypes::datetime_iso8601_utf8_string': gucef_core_dynamic\n"
"enums:\n"
"  gucef_datatypes:\n"
"    1: uint8\n"
"    2: int8\n"
"    3: le_uint16\n"
"    4: be_uint16\n"
"    5: le_int16\n"
"    6: be_int16\n"
"    7: le_uint32\n"
"    8: be_uint32\n"
"    9: le_int32\n"
"    10: be_int32\n"
"    11: le_uint64\n"
"    12: be_uint64\n"
"    13: le_int64\n"
"    14: be_int64\n"
"    15: le_float32\n"
"    16: be_float32\n"
"    17: le_float64\n"
"    18: be_float64\n"
"    21: ascii_string\n"
"    22: utf8_string\n"
"    23: utf16_le_string\n"
"    24: utf16_be_string\n"
"    25: utf32_le_string\n"
"    26: utf32_be_string\n"
"    30: boolean_int32\n"
"    31: boolean_ascii_string\n"
"    32: boolean_utf8_string\n"
"    33: boolean_utf16_le_string\n"
"    34: boolean_utf16_be_string\n"
"    35: boolean_utf32_le_string\n"
"    36: boolean_utf32_be_string\n"
"    40: binary_blob\n"
"    41: binary_bsob\n"
"    50: datetime_iso8601_ascii_string\n"
"    51: datetime_iso8601_utf8_string\n"
"    55: le_timestamp_in_secs_since_unix_epoch\n"
"    56: be_timestamp_in_secs_since_unix_epoch\n"
"    57: le_timestamp_in_ms_since_unix_epoch\n"
"    58: be_timestamp_in_ms_since_unix_epoch\n"
"    60: le_fraction_int32t2\n"
"    61: be_fraction_int32t2\n"
"    62: le_fraction_uint32t2\n"
"    63: be_fraction_uint32t2\n"
"    64: le_fraction_int64t2\n"
"    65: be_fraction_int64t2\n"
"    66: le_fraction_uint64t2\n"
"    67: be_fraction_uint64t2\n"
"    68: le_fraction_int64_int32\n"
"    69: be_fraction_int64_int32\n"
"    70: le_fraction_uint64_uint32\n"
"    71: be_fraction_uint64_uint32\n"
"types:\n"
"  gucef_uint8:\n"
"    seq:\n"
"        - id: data\n"
"          type: u1\n"
"  gucef_int8:\n"
"    seq:\n"
"        - id: data\n"
"          type: s1\n"
"  gucef_le_uint16:\n"
"    seq:\n"
"        - id: data\n"
"          type: u2\n"
"  gucef_be_uint16:\n"
"    seq:\n"
"        - id: data\n"
"          type: u2\n"
"          endian: be\n"
"  gucef_le_int16:\n"
"    seq:\n"
"        - id: data\n"
"          type: s2\n"
"  gucef_be_int16:\n"
"    seq:\n"
"        - id: data\n"
"          type: s2\n"
"          endian: be\n"
"  gucef_le_uint32:\n"
"    seq:\n"
"        - id: data\n"
"          type: u4\n"
"  gucef_be_uint32:\n"
"    seq:\n"
"        - id: data\n"
"          type: u4\n"
"          endian: be\n"
"  gucef_le_int32:\n"
"    seq:\n"
"        - id: data\n"
"          type: s4\n"
"  gucef_be_int32:\n"
"    seq:\n"
"        - id: data\n"
"          type: s4\n"
"          endian: be\n"
"  gucef_le_uint64:\n"
"    seq:\n"
"        - id: data\n"
"          type: u8\n"
"  gucef_be_uint64:\n"
"    seq:\n"
"        - id: data\n"
"          type: u8\n"
"          endian: be\n"
"  gucef_le_int64:\n"
"    seq:\n"
"        - id: data\n"
"          type: s8\n"
"  gucef_be_int64:\n"
"    seq:\n"
"        - id: data\n"
"          type: s8\n"
"          endian: be\n"
"  gucef_le_float32:\n"
"    seq:\n"
"        - id: data\n"
"          type: f4\n"
"  gucef_be_float32:\n"
"    seq:\n"
"        - id: data\n"
"          type: f4\n"
"          endian: be\n"
"  gucef_le_float64:\n"
"    seq:\n"
"        - id: data\n"
"          type: f8\n"
"  gucef_be_float64:\n"
"    seq:\n"
"        - id: data\n"
"          type: f8\n"
"          endian: be\n"
"  gucef_le_boolean_int32:\n"
"    seq:\n"
"        - id: data\n"
"          type: s4\n"
"  gucef_be_boolean_int32:\n"
"    seq:\n"
"        - id: data\n"
"          type: s4\n"
"          endian: be\n"
"  gucef_bsob:\n"
"    seq:\n"
"        - id: data\n"
"          type: u1\n"
"  gucef_le_timestamp_in_secs_since_unix_epoch:\n"
"    seq:\n"
"        - id: data\n"
"          type: u8\n"
"  gucef_be_timestamp_in_secs_since_unix_epoch:\n"
"    seq:\n"
"        - id: data\n"
"          type: u8\n"
"          endian: be\n"
"  gucef_le_timestamp_in_ms_since_unix_epoch:\n"
"    seq:\n"
"        - id: data\n"
"          type: u8\n"
"  gucef_be_timestamp_in_ms_since_unix_epoch:\n"
"    seq:\n"
"        - id: data\n"
"          type: u8\n"
"          endian: be\n"
"  gucef_le_fraction_int32t2:\n"
"    seq:\n"
"        - id: numerator\n"
"          type: s4\n"
"        - id: denominator\n"
"          type: s4\n"
"  gucef_be_fraction_int32t2:\n"
"    seq:\n"
"        - id: numerator\n"
"          type: s4\n"
"          endian: be\n"
"        - id: denominator\n"
"          type: s4\n"
"          endian: be\n"
"  gucef_le_fraction_uint32t2:\n"
"    seq:\n"
"        - id: numerator\n"
"          type: u4\n"
"        - id: denominator\n"
"          type: u4\n"
"  gucef_be_fraction_uint32t2:\n"
"    seq:\n"
"        - id: numerator\n"
"          type: u4\n"
"          endian: be\n"
"        - id: denominator\n"
"          type: u4\n"
"          endian: be\n"
"  gucef_le_fraction_int64_int32:\n"
"    seq:\n"
"        - id: numerator\n"
"          type: s8\n"
"        - id: denominator\n"
"          type: s4\n"
"  gucef_be_fraction_int64_int32:\n"
"    seq:\n"
"        - id: numerator\n"
"          type: s8\n"
"          endian: be\n"
"        - id: denominator\n"
"          type: s4\n"
"          endian: be\n"
"  gucef_le_fraction_uint64_uint32:\n"
"    seq:\n"
"        - id: numerator\n"
"          type: u8\n"
"        - id: denominator\n"
"          type: u4\n"
"  gucef_be_fraction_uint64_uint32:\n"
"    seq:\n"
"        - id: numerator\n"
"          type: u8\n"
"          endian: be\n"
"        - id: denominator\n"
"          type: u4\n"
"          endian: be\n";


const CORE::CString kaitaiSchema_pubsub_msgcontainer_header =
"meta:\n"
"  id: gucef_pubsub_msgcontainer_header\n"
"  endian: le\n"
"seq:\n"
"  - id: magic_text\n"
"    contents: PUBSUBMSGS\n"
"  - id: version\n"
"    type: u1\n"
"  - id: options\n"
"    type: u4\n";


const CORE::CString kaitaiSchema_pubsub_msgcontainer_footer =
"meta:\n"
"  id: gucef_pubsub_msgcontainer_footer\n"
"  endian: le\n"
"seq:\n"
"  - id: msg_offset_idx\n"
"    type: u4\n"
"    repeat: expr\n"
"    repeat-expr: mois\n"
"  - id: msg_offset_idx_size\n"
"    type: u4\n"
"  - id: magic_text\n"
"    contents: PUBSUBMSGS\n"
"instances:\n"
"  mois:\n"
"    pos: _io.size - 14\n"
"    type: u4\n";


const CORE::CString kaitaiSchema_pubsub_msgcontainer =
"meta:\n"
"  id: gucef_pubsub_msgcontainer\n"
"  endian: le\n"
"  imports:\n"
"  - gucef_core_variant\n"
"  - gucef_pubsub_msgcontainer_header\n"
"  - gucef_pubsub_msgcontainer_footer\n"
"seq:\n"
"  - id: header\n"
"    type: gucef_pubsub_msgcontainer_header\n"
"  - id: pubsubmsg\n"
"    type: gucef_pubsub_msg\n"
"    repeat: until\n"
"    repeat-until: _io.pos == _io.size - (msg_offset_idx_size * 4) - 4 - 10\n"
"  - id: footer\n"
"    type: gucef_pubsub_msgcontainer_footer\n"
"instances:\n"
"  msg_offset_idx_size:\n"
"    pos: _io.size - 14\n"
"    type: u4\n"
"types:\n"
"  gucef_pubsub_msg:\n"
"    seq:\n"
"      - id: msg_date_time\n"
"        type:\n"
"          switch-on: (_root.header.options & (1 << 2))\n"
"          cases:\n"
"            0: u4\n"
"            _: u8\n"
"        if: (_root.header.options & (1 << 1)) != 0\n"
"      - id: msgid\n"
"        type: gucef_core_variant\n"
"        if: (_root.header.options & (1 << 3)) != 0\n"
"      - id: msg_index\n"
"        type: gucef_core_variant\n"
"        if: (_root.header.options & (1 << 4)) != 0\n"
"      - id: primary_payload_included\n"
"        type: gucef_core_variant\n"
"        if: (_root.header.options & (1 << 5)) != 0\n"
"      - id: nr_of_kv_pairs\n"
"        type: u4\n"
"        if: (_root.header.options & (1 << 6)) != 0\n"
"      - id: key_value_pairs\n"
"        type: kv_pair\n"
"        repeat: expr\n"
"        repeat-expr: nr_of_kv_pairs\n"
"        if: (_root.header.options & (1 << 6)) != 0\n"
"      - id: nr_of_metadata_kv_pairs\n"
"        type: u4\n"
"        if: (_root.header.options & (1 << 7)) != 0\n"
"      - id: metadata_key_value_pairs\n"
"        type: kv_pair\n"
"        repeat: expr\n"
"        repeat-expr: nr_of_metadata_kv_pairs\n"
"        if: (_root.header.options & (1 << 7)) != 0\n"
"  kv_pair:\n"
"    seq:\n"
"      - id: key\n"
"        type: gucef_core_variant\n"
"      - id: value\n"
"        type: gucef_core_variant\n";


const CORE::CString pubsubMsgcontainerCodecTypeName = "gucef_pubsub_msgcontainer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformKaitaiTestsIfFeasible_PluginBootstrap( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& yamlCodec )
{GUCEF_TRACE;

    // Normally this would be managed via config however to keep the test self contained we do it programmatically
    
    yamlCodec.Unlink();
    
    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
    CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();
    CORE::CDStoreCodecPluginManager& codecPluginManager = coreGlobal->GetDStoreCodecPluginManager();
    CORE::CDataDrivenDStoreCodecFactory& ddCodecFactory = coreGlobal->GetDataDrivenDStoreCodecFactory();
        
    // Check if the yaml codec is available
    if ( !codecRegistry.TryLookup( "yaml", yamlCodec, false ) )
    {
        #ifdef GUCEF_DEBUG_MODE
        CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginYAML_d";
        CORE::CString secondPluginpath = "$MODULEDIR$/dstorepluginYAML";
        #else
        CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginYAML";
        CORE::CString secondPluginpath = "$MODULEDIR$/dstorepluginYAML_d";
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
                std::cout << "YAML codec not available\n";
                return;
            }
        }
    }

    codecRegistry.TryLookup( "yaml", yamlCodec, false );
}

/*-------------------------------------------------------------------------*/

void
PerformKaitaiTest_ConfigDrivenCodecSpecification( void )
{GUCEF_TRACE;

    try
    {
        CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
        CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
        CORE::CDStoreCodecRegistry& codecRegistry = coreGlobal->GetDStoreCodecRegistry();
        CORE::CDStoreCodecPluginManager& codecPluginManager = coreGlobal->GetDStoreCodecPluginManager();
        CORE::CDataDrivenDStoreCodecFactory& ddCodecFactory = coreGlobal->GetDataDrivenDStoreCodecFactory();

        ddCodecFactory.UnregisterAll();

        // Kaitai depends on a YAML codec to load schemas since that is the format for the schemas
        CORE::CDStoreCodecRegistry::TDStoreCodecPtr yamlCodec;
        PerformKaitaiTestsIfFeasible_PluginBootstrap( yamlCodec );
        ASSERT_TRUE( !yamlCodec.IsNULL() );
        
        // We dont want to test the entire global config loading but we know that the data driven data storage 
        // codec factory is actually a global factory so we can just test that subset of the config loading

        CORE::CUri coreDynamicSchemaUri;
        ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( coreDynamicSchemaUri, "application/test-kaitai-gucef_core_dynamic", kaitaiSchema_core_dynamic, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    

        CORE::CUri coreVariantSchemaUri;
        ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( coreVariantSchemaUri, "application/test-kaitai-gucef_core_variant", kaitaiSchema_core_variant, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    

        CORE::CUri pubsubMsgContainerHeaderSchemaUri;
        ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( pubsubMsgContainerHeaderSchemaUri, "application/test-kaitai-pubsub_msgcontainer_header", kaitaiSchema_pubsub_msgcontainer_header, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    

        CORE::CUri pubsubMsgContainerFooterSchemaUri;
        ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( pubsubMsgContainerFooterSchemaUri, "application/test-kaitai-pubsub_msgcontainer_footer", kaitaiSchema_pubsub_msgcontainer_footer, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    

        CORE::CUri pubsubMsgContainerSchemaUri;
        ASSERT_TRUE( CORE::CDataUriResourceAccessor::CreateDataUriFromText( pubsubMsgContainerSchemaUri, "application/test-kaitai-pubsub_msgcontainer", kaitaiSchema_pubsub_msgcontainer, CORE::CDataUriResourceAccessor::DATAURI_ENCODING_BASE64 ) );    


        CORE::CDataNode ddCodecFactoryConfig( "GlobalDataDrivenDStoreCodecFactory", GUCEF_DATATYPE_OBJECT );
        ddCodecFactoryConfig.SetAttribute( "autoInstantiateShareableCodecs", true );

        CORE::CDataNode* codecConfigMetaNode = ddCodecFactoryConfig.AddChild( "codecMeta", GUCEF_DATATYPE_ARRAY );
        ASSERT_TRUE( GUCEF_NULL != codecConfigMetaNode );

        CORE::CDataNode* pubsubContainerCodecConfigNode = codecConfigMetaNode->AddChild( "codecMetaDefinition", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( GUCEF_NULL != pubsubContainerCodecConfigNode );
        pubsubContainerCodecConfigNode->SetAttribute( "dataDrivenCodecTypeName", pubsubMsgcontainerCodecTypeName );
        pubsubContainerCodecConfigNode->SetAttribute( "baseCodecTypeName", "kaitai" );
        pubsubContainerCodecConfigNode->SetAttribute( "isShareable", "true" );
        CORE::CDataNode* pubsubContainerCodecResourcesConfigNode = pubsubContainerCodecConfigNode->AddChild( "resources", GUCEF_DATATYPE_ARRAY );
        ASSERT_TRUE( GUCEF_NULL != pubsubContainerCodecResourcesConfigNode );
        ASSERT_TRUE( GUCEF_NULL != pubsubContainerCodecResourcesConfigNode->AddValueAsChild( coreDynamicSchemaUri ) );
        ASSERT_TRUE( GUCEF_NULL != pubsubContainerCodecResourcesConfigNode->AddValueAsChild( coreVariantSchemaUri ) );        
        ASSERT_TRUE( GUCEF_NULL != pubsubContainerCodecResourcesConfigNode->AddValueAsChild( pubsubMsgContainerHeaderSchemaUri ) );
        ASSERT_TRUE( GUCEF_NULL != pubsubContainerCodecResourcesConfigNode->AddValueAsChild( pubsubMsgContainerFooterSchemaUri ) );        
        ASSERT_TRUE( GUCEF_NULL != pubsubContainerCodecResourcesConfigNode->AddValueAsChild( pubsubMsgContainerSchemaUri ) );        
        

        ASSERT_TRUE( ddCodecFactory.LoadConfig( ddCodecFactoryConfig ) );

    }
    catch( ... )
    {
        ERRORHERE;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformKaitaiSchemaLoadTest( void )
{  
    #ifdef GUCEF_VFS_DEBUG_MODE
    //MEMAN_LogStackToStdOut();
    //MEMAN_SetStackLogging( 1 );
    #endif /* GUCEF_VFS_DEBUG_MODE ? */
    
    try
    {
        KAITAI::CKaitaiGlobal* kaitai = KAITAI::CKaitaiGlobal::Instance();

        KAITAI::CKaitaiSchemaRegistry& schemaRegistry = kaitai->GetKaitaiSchemaRegistry();

        schemaRegistry.LoadSchemaFromString( kaitaiSchema_core_dynamic, "gucef" );
        schemaRegistry.LoadSchemaFromString( kaitaiSchema_pubsub_msgcontainer_header, "gucef" );
        
        GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "Finished all load unload tests" );
        
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "unhandled exception during test" );
        #ifdef GUCEF_VFS_DEBUG_MODE
        //GUCEF::MEMAN_PrintCallstack();
        #endif /* GUCEF_VFS_DEBUG_MODE ? */
        ERRORHERE;
    } 
}

/*-------------------------------------------------------------------------*/
