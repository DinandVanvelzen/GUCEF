/*
 *  dstorepluginPROTOBUF: Plugin adding the ability to use libprotobuf for the protobuf format
 *
 *  Copyright (C) 1998 - 2025.  Dinand Vanvelzen
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

#include <stdlib.h>     /* memory management */
#include <stdio.h>      /* standard I/O utils */
#include <memory>
#include <string>
#include <unordered_map>

#include <google/protobuf/message.h>          // Base class for all protobuf messages
#include <google/protobuf/descriptor.h>       // Describes the structure of protobuf messages
#include <google/protobuf/descriptor.pb.h>    // Protobuf definitions for descriptors
#include <google/protobuf/dynamic_message.h>  // Dynamic message handling
#include <google/protobuf/compiler/importer.h>// Importing .proto files at runtime
#include <google/protobuf/stubs/logging.h>    // Logging utils for log redirection
#include <google/protobuf/io/zero_copy_stream.h> // Zero-copy stream for protobuf
#include <google/protobuf/io/coded_stream.h>  // Coded stream for reading/writing protobuf messages

#include "DLLMainDSTOREpluginPROTOBUF.h"    /* gucefCORE DSTORE codec plugin API */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* gucefCORE macros, used here for the export and callspec macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_C_LOGGING_H
#include "gucefCORE_c_logging.h"
#define GUCEF_CORE_C_LOGGING_H
#endif /* GUCEF_CORE_C_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/* make sure no structure packing is enabled, this allows us to use the structures directly */
#if (defined(_WIN32) || defined(__WIN32__))
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif /* WIN32 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DSTOREPLUGIN {
namespace PROTOBUF {

using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define VERSION_MAJOR_FIELD     0
#define VERSION_MINOR_FIELD     1
#define VERSION_PATCH_FIELD     0
#define VERSION_RELEASE_FIELD   0

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( !defined stricmp )
  #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #define stricmp   _stricmp
  #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    #define stricmp   strcasecmp
  #endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static TGucefCoreCApi g_libApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_HIDDEN ProtoErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector 
{
    public:

    virtual void AddError( const std::string& filename, int line, int column, const std::string& message ) GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;
        
        GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, ( "ProtoErrorCollector:AddError: Error in " + filename + " at " + std::to_string(line) + ":" + std::to_string(column) + ": " + message ).c_str() );
    }

    virtual void AddWarning( const std::string& filename, int line, int column, const std::string& message ) GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;
        
        GUCEF_C_WARNING_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, ( "ProtoErrorCollector:AddError: Warning in " + filename + " at " + std::to_string(line) + ":" + std::to_string(column) + ": " + message ).c_str() );
    }
};

/*---------------------------------------------------------------------------*/

class GUCEF_HIDDEN CustomZeroCopyInputStream : public google::protobuf::io::ZeroCopyInputStream 
{
    private:

    TIOAccess* m_access;
    char m_buffer[ 4096 ];
    size_t m_bufferSize;
    size_t m_bufferPos;

    public:
    
    CustomZeroCopyInputStream( TIOAccess* access )
        : m_access( access ) 
        , m_bufferSize(0) 
        , m_bufferPos(0) 
    {GUCEF_TRACE;
        
        memset( m_buffer, 0, sizeof(m_buffer) );
    }

    virtual ~CustomZeroCopyInputStream() GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

    }

    virtual bool 
    Next( const void** data , 
          int* size         ) GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_access || 
             GUCEF_NULL == data     ||
             GUCEF_NULL == size      )
            return false;   
        
        if ( m_bufferPos >= m_bufferSize ) 
        {
            m_bufferSize = m_access->read( m_access, m_buffer, 1, sizeof(m_buffer) );
            m_bufferPos = 0;
        }

        if (m_bufferSize == 0) {
            return false;
        }

        *data = m_buffer + m_bufferPos;
        *size = static_cast<int>( m_bufferSize - m_bufferPos );
        m_bufferPos = m_bufferSize;
        return true;
    }

    virtual void 
    BackUp( int count ) GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

        m_bufferPos -= count;
    }

    virtual bool 
    Skip( int count ) GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

        m_bufferPos += count;
        if ( m_bufferPos > m_bufferSize ) 
        {
            m_bufferPos = m_bufferSize;
            return false;
        }
        return true;
    }

    virtual int64_t 
    ByteCount() const GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

        return m_bufferPos;
    }
};

/*---------------------------------------------------------------------------*/

class GUCEF_HIDDEN CustomSourceTree : public google::protobuf::compiler::SourceTree 
{
    private:

    TVariantMapApi* m_loadedResources;

    public:

    CustomSourceTree( void )
        : m_loadedResources( GUCEF_NULL ) 
    {GUCEF_TRACE;

    }

    bool Init( TVariantMapApi* loadedResources )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == loadedResources )
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "CustomSourceTree:Init: Invalid parameters" );
            return false;
        }
        m_loadedResources = loadedResources;
        return true;
    }
    
    virtual google::protobuf::io::ZeroCopyInputStream* 
    Open( const std::string& filename ) GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_loadedResources ) 
            return GUCEF_NULL;

        TVariantData keyVar;
        keyVar.containedType = GUCEF_DATATYPE_UTF8_STRING;
        keyVar.union_data.heap_data.heap_data_size = (UInt32) filename.size();
        keyVar.union_data.heap_data.heap_data_is_linked = 1;
        keyVar.union_data.heap_data.union_data.char_heap_data = (char*) filename.c_str();
        
        TVariantData valueVar;
        memset( &valueVar, 0, sizeof( valueVar ) );
        
        m_loadedResources->at_key( m_loadedResources->privateData, &keyVar, &valueVar );     
        
        // Create a ZeroCopyInputStream from the resource data if we have any
        if ( GUCEF_DATATYPE_ASCII_STRING == valueVar.containedType ||
             GUCEF_DATATYPE_UTF8_STRING == valueVar.containedType  || 
             GUCEF_DATATYPE_BINARY_BLOB == valueVar.containedType   )
        {            
            return new google::protobuf::io::ArrayInputStream( valueVar.union_data.heap_data.union_data.void_heap_data , 
                                                               (int) valueVar.union_data.heap_data.heap_data_size      );
        } 
        if ( GUCEF_DATATYPE_BINARY_BSOB == valueVar.containedType )
        {
            return new google::protobuf::io::ArrayInputStream( valueVar.union_data.bsob_data , 
                                                               (int) GUCEF_VARIANT_BSOB_SIZE );
        }        

        return GUCEF_NULL;
    }
};

/*---------------------------------------------------------------------------*/

class GUCEF_HIDDEN CDataDrivenCodecInfo
{
    public:
    
    std::string m_codecTypeName;
    CustomSourceTree m_sourceTree;
    ProtoErrorCollector m_errorCollector;
    google::protobuf::compiler::Importer m_importer;
    const google::protobuf::Descriptor* m_descriptor;
    google::protobuf::DynamicMessageFactory m_msgFactory;
    const google::protobuf::Message* m_message;

    bool Init( TDataDrivenDStoreCodecMeta* codecMeta ,
               TVariantMapApi* loadedResources       ) 
    {GUCEF_TRACE;

        if ( GUCEF_NULL == codecMeta                             ||
             GUCEF_NULL == codecMeta->base_codec_type_name       ||
             GUCEF_NULL == codecMeta->data_driven_codec_typename ||
             GUCEF_NULL == loadedResources                        )
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "DataDrivenCodecInfo:Init: Invalid parameters" );
            return false;
        }

        m_codecTypeName = codecMeta->data_driven_codec_typename;

        
        if ( !m_sourceTree.Init( loadedResources ) )
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "DataDrivenCodecInfo:Init: Failed to initialize source tree" );
            return false;
        }

        if ( GUCEF_NULL == m_importer.Import( m_codecTypeName ) ) 
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, ( "DataDrivenCodecInfo:Init: Failed to import proto file: " + m_codecTypeName ).c_str() );
            return false;
        }

        m_descriptor = m_importer.pool()->FindMessageTypeByName( m_codecTypeName );
        if ( GUCEF_NULL == m_descriptor ) 
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, ( "DataDrivenCodecInfo:Init: Failed to find message type: " + m_codecTypeName ).c_str() );
            return false;
        }

        m_message = m_msgFactory.GetPrototype( m_descriptor );
        if ( GUCEF_NULL == m_message ) 
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, ( "DataDrivenCodecInfo:Init: Failed to generate message prototype for type: " + m_codecTypeName ).c_str() );
            return false;
        }

        return true;
    }

    CDataDrivenCodecInfo( void ) 
        : m_codecTypeName()
        , m_sourceTree()
        , m_errorCollector()
        , m_importer( &m_sourceTree, &m_errorCollector )
        , m_descriptor( GUCEF_NULL )
        , m_msgFactory()
        , m_message( GUCEF_NULL )
    {GUCEF_TRACE;

    }

    private:

    CDataDrivenCodecInfo( const CDataDrivenCodecInfo& );
    CDataDrivenCodecInfo& operator=( const CDataDrivenCodecInfo& );
};

/*---------------------------------------------------------------------------*/

class GUCEF_HIDDEN ProtoSAXParser 
{
    private:

    bool has_error_;
    std::string error_message_;
    void* m_readPrivData;
    TReadHandlers m_readCallbacks;
    CDataDrivenCodecInfo* m_codecInfo;

    private:

    void SetError( const std::string& message ) 
    {GUCEF_TRACE;

        has_error_ = true;
        error_message_ = message;

        GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, message.c_str() );
    }

    public:
    
    ProtoSAXParser( void ) 
        : has_error_( false ) 
        , error_message_()
        , m_readPrivData( GUCEF_NULL )
        , m_readCallbacks()
        , m_codecInfo( GUCEF_NULL )
    {GUCEF_TRACE;

        memset( &m_readCallbacks, 0, sizeof( m_readCallbacks ) );
    }

    bool Init( CDataDrivenCodecInfo* codecInfo    ,
               const TReadHandlers* readCallbacks ,
               void* readPrivData                 )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == codecInfo     || 
             GUCEF_NULL == readCallbacks || 
             GUCEF_NULL == readPrivData   )
        {
            SetError( "ProtoSAXParser:Init: Invalid parameters" );
            return false;
        }

        m_codecInfo = codecInfo;
        m_readPrivData = readPrivData;
        m_readCallbacks = *readCallbacks;
        return true;
    }

    bool ParseMessage( const google::protobuf::Descriptor* descriptor , 
                       const google::protobuf::Message* prototype     , 
                       TIOAccess* access                              ) 
    {GUCEF_TRACE;

        if ( GUCEF_NULL == descriptor ||  
             GUCEF_NULL == prototype  || 
             GUCEF_NULL == access      )
        {
            SetError( "Invalid parameters for ParseMessage" );
            return false;
        }

        CustomZeroCopyInputStream zero_copy_input( access );
        google::protobuf::io::CodedInputStream coded_input( &zero_copy_input );
        std::unique_ptr<google::protobuf::Message> message( prototype->New() );

        return ParseFields( message.get(), &coded_input );
    }

    bool HasError( void ) const 
    {GUCEF_TRACE;

        return has_error_;
    }

    std::string GetErrorMessage( void ) const 
    {GUCEF_TRACE;

        return error_message_;
    }

    /*---------------------------------------------------------------------------*/

    bool ParseFields(google::protobuf::Message* message, google::protobuf::io::CodedInputStream* input) 
    {GUCEF_TRACE;

        const google::protobuf::Descriptor* descriptor = message->GetDescriptor();
        const google::protobuf::Reflection* reflection = message->GetReflection();

        while ( input->BytesUntilLimit() > 0 ) 
        {
            uint32_t tag = input->ReadTag();
            int field_number = google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag);
            const google::protobuf::FieldDescriptor* field = descriptor->FindFieldByNumber(field_number);

            if ( GUCEF_NULL == field ) 
            {
                // Skip unknown field
                if ( !google::protobuf::internal::WireFormatLite::SkipField( input, tag ) ) 
                {
                    SetError("Failed to skip unknown field");
                    return false;
                }
                continue;
            }

            if ( field->is_repeated() ) 
            {
                // Handle repeated fields
                if ( !ParseRepeatedField(message, reflection, field, input ) ) 
                {
                    return false;
                }
            } 
            else 
            {
                // Handle singular fields
                if ( !ParseSingularField(message, reflection, field, input ) ) 
                {
                    return false;
                }
            }
        }

        return true;
    }

    /*---------------------------------------------------------------------------*/

    bool ParseRepeatedField( google::protobuf::Message* message             , 
                             const google::protobuf::Reflection* reflection ,
                             const google::protobuf::FieldDescriptor* field , 
                             google::protobuf::io::CodedInputStream* input  ) 
    {GUCEF_TRACE;

        switch ( field->type() ) 
        {
            case google::protobuf::FieldDescriptor::TYPE_INT32: 
            {
                uint32_t value = 0;
                if ( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read int32 value" );
                    return false;
                }
                
                reflection->AddInt32( message, field, static_cast< int32_t >( value ) );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_UINT32: 
            {
                uint32_t value = 0;
                if ( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read uint32 value" );
                    return false;
                }
                reflection->AddUInt32( message, field, value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_INT64: 
            {
                uint64_t value = 0;
                if ( !input->ReadVarint64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read int64 value" );
                    return false;
                }
                reflection->AddInt64( message, field, static_cast< int64_t >( value ) );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_UINT64: 
            {
                uint64_t value = 0;
                if ( !input->ReadVarint64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read uint64 value" );
                    return false;
                }
                reflection->AddUInt64( message, field, value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_FLOAT: 
            {
                float value = 0.0f;
                if ( !input->ReadLittleEndian32( reinterpret_cast<uint32_t*>(&value) ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read float value" );
                    return false;
                }
                reflection->AddFloat( message, field, value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_DOUBLE: 
            {
                double value = 0.0;
                if ( !input->ReadLittleEndian64( reinterpret_cast<uint64_t*>(&value) ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read double value" );
                    return false;
                }
                reflection->AddDouble( message, field, value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_BOOL: 
            {
                uint32_t value = 0;
                if ( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read bool value" );
                    return false;
                }
                reflection->AddBool( message, field, static_cast< bool >( value ) );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_STRING: 
            {
                std::string value;
                if ( !input->ReadString( &value, input->BytesUntilLimit() ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read string value" );
                    return false;
                }
                reflection->AddString( message, field, value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_MESSAGE: 
            {
                google::protobuf::Message* nested_message = reflection->AddMessage( message, field );
                if ( !ParseFields(nested_message, input ) ) 
                {
                    return false;
                }
                break;
            }

            // Handle other types as needed
            default:
            {
                SetError( "Unsupported field type for repeated field" );
                return false;
            }
        }
        return true;
    }

    /*---------------------------------------------------------------------------*/

    bool ParseSingularField( google::protobuf::Message* message, 
                             const google::protobuf::Reflection* reflection,
                             const google::protobuf::FieldDescriptor* field, 
                             google::protobuf::io::CodedInputStream* input ) 
    {GUCEF_TRACE;

        switch ( field->type() ) 
        {
            case google::protobuf::FieldDescriptor::TYPE_INT32: 
            {
                uint32_t value = 0;
                if ( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read int32 value" );
                    return false;
                }
                reflection->SetInt32( message, field, static_cast< int32_t >( value ) );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_INT64: 
            {
                uint64_t value = 0;
                if ( !input->ReadVarint64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read int64 value" );
                    return false;
                }
                reflection->SetInt64( message, field, static_cast< int64_t >( value ) );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_UINT32: 
            {
                uint32_t value = 0;
                if ( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read uint32 value" );
                    return false;
                }
                reflection->SetUInt32( message, field, value);
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_UINT64: 
            {
                uint64_t value = 0;
                if ( !input->ReadVarint64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read uint64 value" );
                    return false;
                }
                reflection->SetUInt64( message, field, value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_FLOAT: 
            {
                float value = 0.0f;
                if ( !input->ReadLittleEndian32( reinterpret_cast<uint32_t*>(&value) ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read float value" );
                    return false;
                }
                reflection->SetFloat( message, field, value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_DOUBLE: 
            {
                double value = 0.0;
                if (!input->ReadLittleEndian64( reinterpret_cast<uint64_t*>(&value) ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read double value" );
                    return false;
                }
                reflection->SetDouble( message, field, value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_BOOL: 
            {
                uint32_t value = 0;
                if ( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read bool value" );
                    return false;
                }
                reflection->SetBool( message, field, static_cast< bool >( value ) );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_STRING: 
            {
                std::string value;
                if ( !input->ReadString(&value, input->BytesUntilLimit() ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read string value" );
                    return false;
                }
                reflection->SetString(message, field, value);
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_MESSAGE: 
            {
                google::protobuf::Message* nested_message = reflection->MutableMessage( message, field );
                if ( !ParseFields( nested_message, input ) ) 
                {
                    return false;
                }
                break;
            }

            // Handle other types as needed
            default:
            {
                SetError( "ProtoSAXParser:ParseSingularField: Unsupported field type for singular field" );
                return false;
            }
        }
        return true;
    }
};

/*---------------------------------------------------------------------------*/

struct SDestFileData
{
    TIOAccess* fptr;
    //yaml_parser_t parser;
    //yaml_emitter_t emitter;
    char activeNodeIsValueNode;
    char* base64EncodeBuffer;
    UInt32 base64EncodeBufferSize;
};
typedef struct SDestFileData TDestFileData;

/*---------------------------------------------------------------------------*/

class GUCEF_HIDDEN CResourceReadingInfo
{
    public:
    
    TIOAccess* m_access;
    ProtoSAXParser m_parser;
    CDataDrivenCodecInfo* m_codecInfo;

    bool InitResource( CDataDrivenCodecInfo* codecInfo ,
                       TIOAccess* access               )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == codecInfo || 
             GUCEF_NULL == access     )
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "ResourceReadingInfo:InitResource: Invalid parameters" );
            return false;
        }

        m_access = access;
        m_codecInfo = codecInfo;

        return true;
    }

    bool InitParser( const TReadHandlers* readCallbacks ,
                     void* readPrivData                 )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == readCallbacks || 
             GUCEF_NULL == readPrivData    )
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "ResourceReadingInfo:InitParser: Invalid parameters" );
            return false;
        }
        return m_parser.Init( m_codecInfo, readCallbacks, readPrivData );
    }

    bool ParseMessage( void ) 
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_codecInfo && 
             GUCEF_NULL != m_access     )
        {
            return m_parser.ParseMessage( m_codecInfo->m_descriptor , 
                                          m_codecInfo->m_message    , 
                                          m_access                  );
        }
        return false;
    }
    
    CResourceReadingInfo( void ) 
        : m_access( GUCEF_NULL ) 
        , m_parser()
        , m_codecInfo( GUCEF_NULL )
    {GUCEF_TRACE;
        
    }

    private:

    CResourceReadingInfo( const CResourceReadingInfo& );
    CResourceReadingInfo& operator=( const CResourceReadingInfo& );
};

/*---------------------------------------------------------------------------*/

void 
gucefLogRedirect( google::protobuf::LogLevel level , 
                  const char* filename             , 
                  int line                         , 
                  const std::string& message       ) 
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_libApi.Log )
        return;
    
    switch ( level )
    {
        case google::protobuf::LOGLEVEL_INFO:
            GUCEF_C_SYSTEM_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, message.c_str() );
            break;
        case google::protobuf::LOGLEVEL_WARNING:
            GUCEF_C_WARNING_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, message.c_str() );
            break;
        case google::protobuf::LOGLEVEL_ERROR:
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, message.c_str() );
            break;
        case google::protobuf::LOGLEVEL_FATAL:
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_CRITICAL, message.c_str() );
            break;
        default:
            GUCEF_C_SYSTEM_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, message.c_str() );
            break;
    }
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Init( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    // libprotobuf does not require global initialization itself
    if ( GUCEF_NULL != plugdata )
        *plugdata = GUCEF_NULL;

    // link the gucefCORE API
    if ( GUCEF_NULL != libApi )
        g_libApi = *libApi;

    // Hook up the protobuf logging to the gucefCORE logging
    google::protobuf::SetLogHandler( &gucefLogRedirect );

    return 1;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Shutdown( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    // libprotobuf itself does not require global cleanup
    google::protobuf::SetLogHandler( GUCEF_NULL );
    memset( &g_libApi, 0, sizeof g_libApi );
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Open( void** plugdata    ,
                           void** filedata    ,
                           TIOAccess* outFile ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return 0;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Close( void** plugdata ,
                            void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;


}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Begin_Node_Store( void** plugdata      ,
                             void** filedata      ,
                             const char* nodename ,
                             Int32 nodeType       ,
                             UInt32 attscount     ,
                             UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;


}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Store( void** plugdata      ,
                           void** filedata      ,
                           const char* nodename ,
                           UInt32 attscount     ,
                           UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;


}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Store_Node_Att( void** plugdata              ,
                           void** filedata              ,
                           const char* nodename         ,
                           UInt32 attscount             ,
                           UInt32 attindex              ,
                           const char* attname          ,
                           const TVariantData* attvalue ,
                           UInt32 haschildren           ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;


}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Begin_Node_Children( void** plugdata      ,
                                void** filedata      ,
                                const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{

}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Children( void** plugdata      ,
                              void** filedata      ,
                              const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{

}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Src_File_Open( void** plugdata  ,
                          void** codecdata ,
                          void** filedata  ,
                          TIOAccess* file  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    *plugdata = GUCEF_NULL;

    if ( GUCEF_NULL != codecdata &&
         GUCEF_NULL != *codecdata )
    {
        CDataDrivenCodecInfo* codecInfo = static_cast< CDataDrivenCodecInfo* >( *codecdata );

        CResourceReadingInfo* resourceReadingInfo = GUCEF_NEW CResourceReadingInfo();
        if ( GUCEF_NULL != resourceReadingInfo )
        {
            if ( resourceReadingInfo->InitResource( codecInfo, file ) )
            {
                *filedata = resourceReadingInfo;
                return 1;
            }

            GUCEF_DELETE resourceReadingInfo;
        }    
    }
    return 0;    
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Src_File_Close( void** plugdata  ,
                           void** codecdata ,
                           void** filedata  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( filedata != GUCEF_NULL && *filedata != GUCEF_NULL )
    {
        CResourceReadingInfo* resourceReadingInfo = static_cast< CResourceReadingInfo* >( *filedata );
        GUCEF_DELETE resourceReadingInfo;
        *filedata = GUCEF_NULL;
    }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Set_Read_Handlers( void** plugdata                ,
                              void** codecdata               ,
                              void** filedata                ,
                              const TReadHandlers* rhandlers ,
                              void* privdata                 ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( filedata != GUCEF_NULL && *filedata != GUCEF_NULL )
    {
        CResourceReadingInfo* resourceReadingInfo = static_cast< CResourceReadingInfo* >( *filedata );

        if ( resourceReadingInfo->InitParser( rhandlers, privdata ) )
        {
            *filedata = resourceReadingInfo;
        }
        else
        {
            *filedata = GUCEF_NULL;
            GUCEF_C_ERROR_LOG(g_libApi, GUCEF_LOGLEVEL_NORMAL, "DSTOREPLUG_Set_Read_Handlers: Failed to init parser ");
        }
    }
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Start_Reading( void** plugdata  ,
                          void** codecdata ,
                          void** filedata  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( codecdata != GUCEF_NULL && *codecdata != GUCEF_NULL &&
         filedata != GUCEF_NULL && *filedata != GUCEF_NULL    )
    {        
        CResourceReadingInfo* resourceReadingInfo = static_cast< CResourceReadingInfo* >( *filedata );

        if ( resourceReadingInfo->ParseMessage() )
        {
            // no error is no error code aka 0
            return 0;
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt8 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Create_Data_Driven_Codec( const void* plugdata                  ,
                                     TDataDrivenDStoreCodecMeta* codecMeta ,
                                     TVariantMapApi* loadedResources       ,
                                     void** dataDrivenCodecPrivateData     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == dataDrivenCodecPrivateData )
        return 0;

    CDataDrivenCodecInfo* codecInfo = GUCEF_NEW CDataDrivenCodecInfo();
    if ( GUCEF_NULL == codecInfo )
        return 0;
    
    if ( !codecInfo->Init( codecMeta, loadedResources ) )
    {
        GUCEF_DELETE codecInfo;
        return 0;
    }

    *dataDrivenCodecPrivateData = codecInfo;
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt8 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Destroy_Data_Driven_Codec( const void* plugdata                  ,
                                      TDataDrivenDStoreCodecMeta* codecMeta ,
                                      void** dataDrivenCodecPrivateData     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == dataDrivenCodecPrivateData )
        return 0;

    CDataDrivenCodecInfo* codecInfo = static_cast< CDataDrivenCodecInfo* >( *dataDrivenCodecPrivateData );
    if ( GUCEF_NULL == codecInfo )
        return 0;
    
    try
    {
        GUCEF_DELETE codecInfo;
    }
    catch ( const std::exception& e )
    {
        std::string errMsg( "DSTOREPLUG_Destroy_Data_Driven_Codec: Exception deleting codec info: " );
        errMsg += e.what();
        GUCEF_C_EXCEPTION_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, errMsg.c_str() );
        return 0;
    }

    *dataDrivenCodecPrivateData = GUCEF_NULL;
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt8 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Type_Is_Data_Driven( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return 1;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Type( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "protobuf";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Name( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCORE DSTORE codec plugin utilizing libprotobuf for reading/writing the protobuf format";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Copyright( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen. 1998 - 2025. Licensed under the Apache License, Version 2.0 (the \"License\")";
}

/*---------------------------------------------------------------------------*/

const CORE::TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Version( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    static CORE::TVersion version;
    version.major = VERSION_MAJOR_FIELD;
    version.minor = VERSION_MINOR_FIELD;
    version.patch = VERSION_PATCH_FIELD;
    version.release = VERSION_RELEASE_FIELD;

    return &version;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROTOBUF */
}; /* namespace DSTOREPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/* restore structure packing */
#if (defined(_WIN32) || defined(__WIN32__))
#pragma pack(pop)
#else
#pragma pack()
#endif /* WIN32 */

/*-------------------------------------------------------------------------*/

