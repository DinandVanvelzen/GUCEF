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
#include <iostream>
#include <sstream>
#include <set>
#include <map>
#include <unordered_map>

#include <google/protobuf/message.h>          // Base class for all protobuf messages
#include <google/protobuf/descriptor.h>       // Describes the structure of protobuf messages
#include <google/protobuf/descriptor.pb.h>    // Protobuf definitions for descriptors
#include <google/protobuf/dynamic_message.h>  // Dynamic message handling
#include <google/protobuf/compiler/importer.h>// Importing .proto files at runtime
#include <google/protobuf/stubs/logging.h>    // Logging utils for log redirection
#include <google/protobuf/io/zero_copy_stream.h> // Zero-copy stream for protobuf
#include <google/protobuf/io/coded_stream.h>  // Coded stream for reading/writing protobuf messages
#include <google/protobuf/wire_format_lite.h> // used for reading/writing protobuf messages with encodings

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
        
        std::ostringstream logStrStr;
        logStrStr << "ProtoErrorCollector: Error : " << filename << " at " << line << ":" << column << " : " << message;
        std::string logStr = logStrStr.str();
        
        GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, logStr.c_str() );
    }

    virtual void AddWarning( const std::string& filename, int line, int column, const std::string& message ) GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;
        
        std::ostringstream logStrStr;
        logStrStr << "ProtoErrorCollector: Warning : " << filename << " at " << line << ":" << column << " : " << message;
        std::string logStr = logStrStr.str();

        GUCEF_C_WARNING_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, logStr.c_str() );
    }
};

/*---------------------------------------------------------------------------*/

class GUCEF_HIDDEN CustomZeroCopyInputStream : public google::protobuf::io::ZeroCopyInputStream 
{
    private:

    TIOAccess* m_access;
    char m_buffer[ 4096 ];
    UInt64 m_totalBytesRead;

    public:
    
    CustomZeroCopyInputStream( TIOAccess* access )
        : m_access( access ) 
        , m_totalBytesRead( 0 ) 
    {GUCEF_TRACE;
        
        memset( m_buffer, 0, sizeof(m_buffer) );
        if GUCEF_PREDICT_FALSE( GUCEF_NULL == m_access )
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "CustomZeroCopyInputStream: NULL passed for IOAccess" );
            return;
        }
        if ( 0 == m_access->opened( m_access ) )
        {
            if GUCEF_PREDICT_FALSE( !m_access->open( m_access ) )
            {
                GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "CustomZeroCopyInputStream: Failed to open IOAccess" );
                return;
            }
        }

        #ifdef GUCEF_DEBUG_MODE    
        UInt64 resourcePos = static_cast< size_t >( m_access->tell( m_access ) );
        UInt64 resourceSize = static_cast< size_t >( m_access->size( m_access ) );
        std::ostringstream debugStrStr;
        debugStrStr << "CustomZeroCopyInputStream: Access init to resource of " << resourceSize << " bytes at offset " << resourcePos;
        std::string debugStr = debugStrStr.str();
        GUCEF_C_DEBUG_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, debugStr.c_str() );
        #endif /* GUCEF_DEBUG_MODE ? */

    }

    virtual ~CustomZeroCopyInputStream() GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

    }

    virtual bool 
    Next( const void** data , 
          int* size         ) GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

        if GUCEF_PREDICT_FALSE( GUCEF_NULL == m_access || 
             GUCEF_NULL == data     ||
             GUCEF_NULL == size      )
            return false;   

        int bytesRead = m_access->read( m_access, m_buffer, 1, sizeof(m_buffer) );
        if GUCEF_PREDICT_FALSE( 0 == bytesRead )
        {
            Int32 isEof = m_access->eof( m_access );
            if ( 0 != isEof )
            {
                GUCEF_C_DEBUG_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "CustomZeroCopyInputStream: Finished reading resource" );
                return false;
            }
            else
            {
                GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "CustomZeroCopyInputStream: Failed to read bytes from IOAccess" );
                return false;
            }
        }

        m_totalBytesRead += bytesRead;

        *size = bytesRead;
        *data = m_buffer;
        return true;
    }

    virtual void 
    BackUp( int count ) GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

        if GUCEF_PREDICT_FALSE( GUCEF_NULL == m_access )
            return;   
            
        Int32 errorCode = m_access->seek( m_access, -count, SEEK_SET );
        if GUCEF_PREDICT_FALSE( 0 != errorCode )
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "CustomZeroCopyInputStream: Failed to seek in IOAccess" );
        }
    }

    virtual bool 
    Skip( int count ) GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

        if GUCEF_PREDICT_FALSE( GUCEF_NULL == m_access )
            return false;   
            
        Int32 errorCode = m_access->seek( m_access, count, SEEK_SET );
        if GUCEF_PREDICT_FALSE( 0 != errorCode )
        {
            Int32 isEof = m_access->eof( m_access );
            if ( 0 != isEof )
            {
                GUCEF_C_DEBUG_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "CustomZeroCopyInputStream: Finished reading resource" );
                return false;
            }
            else
            {
                GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "CustomZeroCopyInputStream: Failed to seek in IOAccess" );
                return false;
            }
        }
        return true;
    }

    virtual int64_t 
    ByteCount() const GUCEF_VIRTUAL_OVERRIDE 
    {GUCEF_TRACE;

        return m_totalBytesRead;
    }

    UInt64 MaxBytesAvailable( void ) const
    {GUCEF_TRACE;

        if GUCEF_PREDICT_FALSE( GUCEF_NULL == m_access )
            return 0;           
    
        UInt64 resourceSize = static_cast< UInt64 >( m_access->size( m_access ) );
        return resourceSize;
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
    const google::protobuf::Descriptor* m_msgDescriptor;
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

        const google::protobuf::FileDescriptor* fileDescriptor = m_importer.Import( m_codecTypeName );
        if ( GUCEF_NULL == fileDescriptor ) 
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, ( "DataDrivenCodecInfo:Init: Failed to import proto file: " + m_codecTypeName ).c_str() );
            return false;
        }

        std::string msgTypeName;
        int messageTypeCount = fileDescriptor->message_type_count();
        for ( int i=0; i<messageTypeCount; ++i ) 
        {
            const google::protobuf::Descriptor* descriptor = fileDescriptor->message_type( i );
            if ( GUCEF_NULL != descriptor ) 
            {
                GUCEF_C_DEBUG_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, ( "DataDrivenCodecInfo:Init: Loaded message type: " + descriptor->name() ).c_str() );
                if ( msgTypeName.empty() )
                    msgTypeName = descriptor->name();
            }
        }

        m_msgDescriptor = fileDescriptor->FindMessageTypeByName( msgTypeName );
        if ( GUCEF_NULL == m_msgDescriptor ) 
        {
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, ( "DataDrivenCodecInfo:Init: Failed to find message type: " + m_codecTypeName ).c_str() );
            return false;
        }

        m_message = m_msgFactory.GetPrototype( m_msgDescriptor );
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
        , m_msgDescriptor( GUCEF_NULL )
        , m_msgFactory()
        , m_message( GUCEF_NULL )
    {GUCEF_TRACE;

    }

    private:

    CDataDrivenCodecInfo( const CDataDrivenCodecInfo& );
    CDataDrivenCodecInfo& operator=( const CDataDrivenCodecInfo& );
};

/*---------------------------------------------------------------------------*/

#include "CResourceWritingInfo.h"            /* CResourceWritingInfo class for write support */

/*---------------------------------------------------------------------------*/

class GUCEF_HIDDEN ProtoSAXParser 
{
    private:

    bool has_error_;
    std::string error_message_;
    void* m_readPrivData;
    TReadHandlers m_readCallbacks;
    CDataDrivenCodecInfo* m_codecInfo;
    std::string m_stringFieldBuffer;
    std::string m_stringFieldBuffer2;
    std::vector<UInt8> m_byteFieldBuffer;
    std::vector<UInt8> m_byteFieldBuffer2;

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
        , m_stringFieldBuffer()
        , m_stringFieldBuffer2()
        , m_byteFieldBuffer()
        , m_byteFieldBuffer2()
    {GUCEF_TRACE;

        memset( &m_readCallbacks, 0, sizeof( m_readCallbacks ) );
    }

    bool Init( CDataDrivenCodecInfo* codecInfo    ,
               const TReadHandlers* readCallbacks ,
               void* readPrivData                 )
    {GUCEF_TRACE;

        if GUCEF_PREDICT_FALSE( GUCEF_NULL == codecInfo     || 
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

    bool ParseMessage( const google::protobuf::Descriptor* msgDescriptor , 
                       const google::protobuf::Message* prototype        , 
                       TIOAccess* access                                 ) 
    {GUCEF_TRACE;

        if GUCEF_PREDICT_FALSE( GUCEF_NULL == msgDescriptor ||  
                                GUCEF_NULL == prototype     || 
                                GUCEF_NULL == access         )
        {
            SetError( "Invalid parameters for ParseMessage" );
            return false;
        }

        m_readCallbacks.OnTreeBegin( m_readPrivData );
        
        CustomZeroCopyInputStream zero_copy_input( access );
        google::protobuf::io::CodedInputStream coded_input( &zero_copy_input );
        coded_input.PushLimit( static_cast< int >( zero_copy_input.MaxBytesAvailable() ) );
                        
        //std::string rawDataStr = "ProtoSAXParser: Raw message payload: \n";
        //const void* raw_data = GUCEF_NULL;
        //int size;
        //if ( coded_input.GetDirectBufferPointer( &raw_data, &size ) ) 
        //{
        //    char printBuffer[ 128 ];
        //    const uint8_t* bytes = static_cast<const uint8_t*>( raw_data );
        //    for ( int i=0; i<size; ++i ) 
        //    {
        //        sprintf( printBuffer, "Byte %d: 0x%02X\n", i, bytes[i] );
        //        rawDataStr += printBuffer;
        //    }
        //}
        //GUCEF_C_DEBUG_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, rawDataStr.c_str() );

        bool parseResult = ParseMsgFields( msgDescriptor, GUCEF_NULL, &coded_input );

        m_readCallbacks.OnTreeEnd( m_readPrivData );

        return parseResult;
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

    bool ReadSInt32Field( google::protobuf::io::CodedInputStream* input, int32_t& value ) 
    {GUCEF_TRACE;

        value = 0;
        uint32_t raw_value = 0;
        if GUCEF_PREDICT_TRUE( input->ReadVarint32( &raw_value ) ) 
        {
            value = google::protobuf::internal::WireFormatLite::ZigZagDecode32( raw_value );
            return true;
        }
        else
        {
            SetError( "Failed to read sint32 value" );
            return false;
        }
    }

    /*---------------------------------------------------------------------------*/

    bool ReadSInt64Field( google::protobuf::io::CodedInputStream* input, int64_t& value ) 
    {GUCEF_TRACE;

        value = 0;
        uint64_t raw_value = 0;
        if GUCEF_PREDICT_TRUE( input->ReadVarint64( &raw_value ) ) 
        {
            value = google::protobuf::internal::WireFormatLite::ZigZagDecode64( raw_value );
            return true;
        }
        else
        {
            SetError( "Failed to read sint64 value" );
            return false;
        }
    }

    /*---------------------------------------------------------------------------*/

    bool ReadStringField( google::protobuf::io::CodedInputStream* input, std::string& value ) 
    {GUCEF_TRACE;

        value.clear();
        
        // Read the length of the string (length-delimited field)
        uint32_t length = 0;
        if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &length ) ) 
        {
            SetError( "Failed to read string length" );
            return false;
        }

        // Read the string data
        if GUCEF_PREDICT_FALSE( !input->ReadString( &value, length ) ) 
        {
            SetError( "Failed to read string data" );
            return false;
        }

        return true;
    }

    /*---------------------------------------------------------------------------*/

    bool ReadBytesField( google::protobuf::io::CodedInputStream* input, std::vector< UInt8 >& byteBuffer ) 
    {GUCEF_TRACE;

        try
        {
            byteBuffer.clear();
        
            // Read the number of bytes (length-delimited field)
            uint32_t nrOfBytes = 0;
            if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &nrOfBytes ) ) 
            {
                SetError( "Failed to read blob nr of bytes" );
                return false;
            }

            if ( nrOfBytes > 0 )
            {
                byteBuffer.resize( nrOfBytes );

                // Read the string data
                if GUCEF_PREDICT_FALSE( !input->ReadRaw( &byteBuffer[ 0 ], nrOfBytes ) ) 
                {                
                    SetError( "Failed to read blob" );
                    byteBuffer.clear();
                    return false;
                }
            }
            return true;
        }
        catch ( const std::exception& )
        {
            SetError( "Failed to read blob: exception" );
            return false;
        }
    }

    /*---------------------------------------------------------------------------*/

    bool ParseField( const google::protobuf::FieldDescriptor* field    , 
                     google::protobuf::io::CodedInputStream* input     ,
                     TVariantData& variant                             ,
                     bool& wasComplex                                  ,
                     std::string& stringFieldBuffer                    ,
                     std::vector< UInt8 >& byteFieldBuffer             ) 
    {GUCEF_TRACE;

        // Note that protobuf assumes all values on the wire to be little endian
        // also parse functions make no effort to convert to host endian, hence values are 
        // always assumed to be little endian and higher level code must convert to host endian if needed

        memset( &variant, 0, sizeof( variant ) );

        switch ( field->type() ) 
        {
            case google::protobuf::FieldDescriptor::TYPE_INT32: 
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read int32 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_INT32;
                variant.union_data.int32_data = static_cast< Int32 >( value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_INT64: 
            {
                uint64_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read int64 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_INT64;
                variant.union_data.int64_data = static_cast< Int64 >( value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_UINT32: 
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read uint32 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_UINT32;
                variant.union_data.uint32_data = value;                
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_UINT64: 
            {
                uint64_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read uint64 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_UINT64;
                variant.union_data.uint64_data = value;                
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_FIXED64:
            {
                uint64_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read fixed size uint64 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_UINT64;
                variant.union_data.uint64_data = value;                
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_FIXED32:
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read fixed size uint32 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_UINT32;
                variant.union_data.uint32_data = value;                
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
            {
                uint64_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read fixed size int64 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_INT64;
                variant.union_data.uint64_data = static_cast< Int64 >( value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read fixed size int32 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_INT32;
                variant.union_data.uint32_data = static_cast< Int32 >( value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SINT32:
            {
                int32_t value = 0;
                if GUCEF_PREDICT_FALSE( !ReadSInt32Field( input, value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read zigZag int32 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_INT32;
                variant.union_data.int32_data = value;                
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SINT64:
            {
                int64_t value = 0;
                if GUCEF_PREDICT_FALSE( !ReadSInt64Field( input, value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read zigZag int64 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_INT64;
                variant.union_data.int64_data = value;                
                break;
            }  
            case google::protobuf::FieldDescriptor::TYPE_FLOAT: 
            {
                float value = 0.0f;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian32( reinterpret_cast<uint32_t*>(&value) ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read float value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_FLOAT32;
                variant.union_data.float32_data = value;                
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_DOUBLE: 
            {
                double value = 0.0;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian64( reinterpret_cast<uint64_t*>(&value) ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read double value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_FLOAT64;
                variant.union_data.float64_data = value;                
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_BOOL: 
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read bool value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_BOOLEAN_INT32;
                variant.union_data.int32_data = static_cast< Int32 >( value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_STRING: 
            {
                if GUCEF_PREDICT_FALSE( !ReadStringField( input, m_stringFieldBuffer ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read string value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_UTF8_STRING;
                variant.union_data.heap_data.heap_data_size = static_cast< UInt32 >( stringFieldBuffer.size() );
                variant.union_data.heap_data.heap_data_is_linked = 1;
                variant.union_data.heap_data.union_data.const_char_heap_data = stringFieldBuffer.c_str();
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_MESSAGE: 
            {
                // A message is a complex type where we will invoke callbacks ourselves
                // set the flag so that we dont invoke callbacks incorrectly
                wasComplex = true;

                const google::protobuf::Descriptor* nestedMsgDescriptor = field->message_type();
                if GUCEF_PREDICT_TRUE( GUCEF_NULL != nestedMsgDescriptor )
                {
                    uint32_t nestedMsgSizeInBytes = 0;
                    if GUCEF_PREDICT_TRUE( input->ReadVarint32( &nestedMsgSizeInBytes ) ) 
                    {
                        google::protobuf::io::CodedInputStream::Limit limit = input->PushLimit( nestedMsgSizeInBytes );                        
                        if GUCEF_PREDICT_FALSE( !ParseMsgFields( nestedMsgDescriptor, field, input ) ) 
                        {
                            return false;
                        }
                        input->PopLimit( limit );
                    }
                    else
                    {
                        SetError( "ProtoSAXParser:ParseField: Failed to read nested message size" );
                        return false;
                    }
                }
                else
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to find nested message descriptor" );
                    return false;
                }
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_ENUM:
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read enum int32 value" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_LE_INT32;
                variant.union_data.int32_data = static_cast< Int32 >( value );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_BYTES:
            {
                if GUCEF_PREDICT_FALSE( !ReadBytesField( input, byteFieldBuffer ) )
                {
                    SetError( "ProtoSAXParser:ParseField: Failed to read byte array field" );
                    return false;
                }
                variant.containedType = GUCEF_DATATYPE_BINARY_BLOB;
                variant.union_data.heap_data.heap_data_size = static_cast< UInt32 >( byteFieldBuffer.size() );
                variant.union_data.heap_data.heap_data_is_linked = 1;
                if ( variant.union_data.heap_data.heap_data_size > 0 )
                    variant.union_data.heap_data.union_data.void_heap_data = &byteFieldBuffer[0];                
                else
                    variant.union_data.heap_data.union_data.void_heap_data = GUCEF_NULL;
                break;
            }

            // Handle other types as needed
            default:
            {
                Int32 typeId = static_cast< Int32 >( field->type() );
                std::ostringstream debugStrStr;
                debugStrStr << "ProtoSAXParser:ParseField: Unsupported field type for singular field: " << typeId;
                std::string debugStr = debugStrStr.str();
                SetError( debugStr );
                return false;
            }
        }
        return true;
    }

    /*---------------------------------------------------------------------------*/

    bool ParseField( const google::protobuf::FieldDescriptor* field    , 
                     google::protobuf::io::CodedInputStream* input     ,
                     TVariantData& variant                             ,
                     bool& wasComplex                                  ) 
    {GUCEF_TRACE;

        return ParseField( field, input, variant, wasComplex, m_stringFieldBuffer, m_byteFieldBuffer );
    }

    /*---------------------------------------------------------------------------*/

    bool ParseMapField( const google::protobuf::FieldDescriptor* mapEntryField ,
                        const uint32_t mapEntryFieldTag                        ,
                        google::protobuf::io::CodedInputStream* input          ) 
    {GUCEF_TRACE;

        // Note that there is NO explicit marker to indicate 'this is a map'
        // Nor is there any indicator of 'the map will have x entries'
        // Maps are implicitly defined by their contents which are explicit via the MapEntry message wire type
        // said wire type combined with the field number of the map entry field gives a unique consistent id for
        // all the entries belonging to a given map and thus implicitly denotes the map itself
        
        // Ensure the field is a map
        if GUCEF_PREDICT_FALSE( !mapEntryField->is_map() ) 
        {
            SetError( "ParseMapField: Field is not a map" );
            return false;
        }

        // Get the descriptor for the MapEntry message
        const google::protobuf::Descriptor* mapEntryDescriptor = mapEntryField->message_type();
        if GUCEF_PREDICT_FALSE( GUCEF_NULL == mapEntryDescriptor ) 
        {
            SetError( "ParseMapField: Failed to get MapEntry descriptor" );
            return false;
        }

        // Get the key and value field descriptors
        // Protobuf guarantees that the key and value fields of a MapEntry message are always at fixed indices:
        //      The key field is always at index 0.
        //      The value field is always at index 1.
        const google::protobuf::FieldDescriptor* keyField = mapEntryDescriptor->field( 0 );
        const google::protobuf::FieldDescriptor* valueField = mapEntryDescriptor->field( 1 );
        if GUCEF_PREDICT_FALSE( GUCEF_NULL == keyField || GUCEF_NULL ==  valueField ) 
        {
            SetError( "ParseMapField: Failed to get key or value field descriptor" );
            return false;
        }

        // Parse each MapEntry, we already started reading the first one, its tag, to get here
        // there will be x more Map entries potentialy which in total make up the map
        while ( input->BytesUntilLimit() > 0 ) 
        {
            // Read the size of the MapEntry
            uint32_t mapEntrySize = 0;
            if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &mapEntrySize ) ) 
            {
                SetError( "ParseMapField: Failed to read MapEntry size" );
                return false;
            }

            // Push a limit for the MapEntry
            google::protobuf::io::CodedInputStream::Limit limit = input->PushLimit( mapEntrySize );

            // Initialize variables for key and value
            TVariantData keyVar;
            TVariantData valueVar;
            bool keyParsed = false;
            bool valueParsed = false;

            // Parse the fields within the MapEntry
            while ( input->BytesUntilLimit() > 0 ) 
            {
                // The field ordering is not garantueed. You can have key,value or value,key
                // read the tag of the FIELD. Note that this is NOT the tag of the MapEntry itself
                uint32_t tag = input->ReadTag();
                if ( 0 == tag ) 
                {
                    // End of map field
                    return true;
                }
                int fieldNumber = google::protobuf::internal::WireFormatLite::GetTagFieldNumber( tag );

                switch ( fieldNumber )
                {
                    case 1:
                    {
                        // Parse the key
                        bool wasComplex = false;
                        if GUCEF_PREDICT_FALSE( !ParseField( keyField, input, keyVar, wasComplex, m_stringFieldBuffer, m_byteFieldBuffer ) || wasComplex ) 
                        {
                            SetError( "ParseMapField: Failed to parse map key" );
                            input->PopLimit( limit );
                            return false;
                        }
                        keyParsed = true;
                        break;
                    }
                    case 2:
                    {
                        // Parse the value
                        bool wasComplex = false;
                        if GUCEF_PREDICT_FALSE( !ParseField( valueField, input, valueVar, wasComplex, m_stringFieldBuffer2, m_byteFieldBuffer2 ) || wasComplex ) 
                        {
                            SetError( "ParseMapField: Failed to parse map value" );
                            input->PopLimit( limit );
                            return false;
                        }
                        valueParsed = true;
                        break;
                    }
                    default:
                    {
                        // Skip unknown field
                        if ( !google::protobuf::internal::WireFormatLite::SkipField( input, tag ) )
                        {
                            SetError("ParseMapField: Failed to skip unknown field");
                            input->PopLimit(limit);
                            return false;
                        }
                        continue;
                    }
                }
            }

            // Ensure both key and value were parsed
            if ( GUCEF_PREDICT_FALSE( !keyParsed || !valueParsed ) ) 
            {
                SetError( "ParseMapField: Missing key or value in MapEntry" );
                input->PopLimit( limit );
                return false;
            }

            // Store the key-value pair
            m_readCallbacks.OnNodeAtt( m_readPrivData, mapEntryField->name().c_str(), &keyVar, &valueVar );

            // Pop the limit for the MapEntry
            input->PopLimit( limit );

            // In order to know if we are done with the map we need to read the next tag
            // as long as the tag value remains the same we are reading additional map entries
            // 
            // Let's peek ahead at the next tag without advancing the cursor unless the tag matches your expectation.
            // the ExpectTag() function provides this out of the box
            if ( !input->ExpectTag( mapEntryFieldTag ) )
            {
                // End of map
                break;
            }
        }

        return true;
    }

    /*---------------------------------------------------------------------------*/

    void EmitDefaultsForUnseenFields( const google::protobuf::Descriptor* msgDescriptor           ,
                                      const google::protobuf::FieldDescriptor* msgFieldDescriptor ,
                                      const std::set< int >& seenFieldNumbers                     )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == msgDescriptor )
            return;

        const char* nodeName = GUCEF_NULL != msgFieldDescriptor ? msgFieldDescriptor->name().c_str() : msgDescriptor->name().c_str();

        for ( int i = 0; i < msgDescriptor->field_count(); ++i )
        {
            const google::protobuf::FieldDescriptor* field = msgDescriptor->field( i );
            if ( GUCEF_NULL == field )
                continue;

            // Skip fields that were already seen on the wire
            if ( seenFieldNumbers.find( field->number() ) != seenFieldNumbers.end() )
                continue;

            // Skip repeated, map, and message fields - only emit defaults for singular scalars
            if ( field->is_repeated() )
                continue;
            if ( field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE )
                continue;

            // Skip oneof fields - a oneof can legitimately have no member set in proto3
            // and we should never emit defaults for the non-chosen alternatives
            if ( GUCEF_NULL != field->containing_oneof() )
                continue;

            // Build the key variant with the field name
            TVariantData keyVar;
            memset( &keyVar, 0, sizeof( keyVar ) );
            keyVar.containedType = GUCEF_DATATYPE_UTF8_STRING;
            keyVar.union_data.heap_data.heap_data_is_linked = 1;
            keyVar.union_data.heap_data.heap_data_size = static_cast< UInt32 >( field->name().size() );
            keyVar.union_data.heap_data.union_data.const_char_heap_data = field->name().c_str();

            // Build the value variant with the proto3 default (zero-value)
            TVariantData valueVar;
            memset( &valueVar, 0, sizeof( valueVar ) );

            switch ( field->type() )
            {
                case google::protobuf::FieldDescriptor::TYPE_INT32:
                case google::protobuf::FieldDescriptor::TYPE_SINT32:
                case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
                    valueVar.containedType = GUCEF_DATATYPE_LE_INT32;
                    valueVar.union_data.int32_data = 0;
                    break;
                case google::protobuf::FieldDescriptor::TYPE_INT64:
                case google::protobuf::FieldDescriptor::TYPE_SINT64:
                case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
                    valueVar.containedType = GUCEF_DATATYPE_LE_INT64;
                    valueVar.union_data.int64_data = 0;
                    break;
                case google::protobuf::FieldDescriptor::TYPE_UINT32:
                case google::protobuf::FieldDescriptor::TYPE_FIXED32:
                    valueVar.containedType = GUCEF_DATATYPE_LE_UINT32;
                    valueVar.union_data.uint32_data = 0;
                    break;
                case google::protobuf::FieldDescriptor::TYPE_UINT64:
                case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                    valueVar.containedType = GUCEF_DATATYPE_LE_UINT64;
                    valueVar.union_data.uint64_data = 0;
                    break;
                case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                    valueVar.containedType = GUCEF_DATATYPE_LE_FLOAT32;
                    valueVar.union_data.float32_data = 0.0f;
                    break;
                case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                    valueVar.containedType = GUCEF_DATATYPE_LE_FLOAT64;
                    valueVar.union_data.float64_data = 0.0;
                    break;
                case google::protobuf::FieldDescriptor::TYPE_BOOL:
                    valueVar.containedType = GUCEF_DATATYPE_BOOLEAN_INT32;
                    valueVar.union_data.int32_data = 0;
                    break;
                case google::protobuf::FieldDescriptor::TYPE_STRING:
                    valueVar.containedType = GUCEF_DATATYPE_UTF8_STRING;
                    valueVar.union_data.heap_data.heap_data_size = 0;
                    valueVar.union_data.heap_data.heap_data_is_linked = 1;
                    valueVar.union_data.heap_data.union_data.const_char_heap_data = "";
                    break;
                case google::protobuf::FieldDescriptor::TYPE_BYTES:
                    valueVar.containedType = GUCEF_DATATYPE_BINARY_BLOB;
                    valueVar.union_data.heap_data.heap_data_size = 0;
                    valueVar.union_data.heap_data.heap_data_is_linked = 1;
                    valueVar.union_data.heap_data.union_data.void_heap_data = GUCEF_NULL;
                    break;
                case google::protobuf::FieldDescriptor::TYPE_ENUM:
                {
                    valueVar.containedType = GUCEF_DATATYPE_LE_INT32;
                    valueVar.union_data.int32_data = 0;
                    m_readCallbacks.OnNodeBegin( m_readPrivData, field->name().c_str(), GUCEF_DATATYPE_ENUM );
                    m_readCallbacks.OnNodeValue( m_readPrivData, field->name().c_str(), &valueVar );
                    m_readCallbacks.OnNodeEnd( m_readPrivData, field->name().c_str() );
                    continue;
                }
                default:
                    continue;
            }

            m_readCallbacks.OnNodeAtt( m_readPrivData, nodeName, &keyVar, &valueVar );
        }
    }

    /*---------------------------------------------------------------------------*/

    bool ParseMsgFields( const google::protobuf::Descriptor* msgDescriptor           , 
                         const google::protobuf::FieldDescriptor* msgFieldDescriptor ,
                         google::protobuf::io::CodedInputStream* input               ) 
    {GUCEF_TRACE;

        // If the message is the root message, we need to use the root message descriptor
        // otherwise the message itself is a field and we should use the field name for the node name
        const char* nodeName = GUCEF_NULL != msgFieldDescriptor ? msgFieldDescriptor->name().c_str() : msgDescriptor->name().c_str();
        
        m_readCallbacks.OnNodeBegin( m_readPrivData, nodeName, GUCEF_DATATYPE_OBJECT );

        std::set< int > seenFieldNumbers;

        while ( input->BytesUntilLimit() > 0 ) 
        {
            uint32_t tag = input->ReadTag();

            #ifdef GUCEF_DEBUG_MODE
            int wireType = google::protobuf::internal::WireFormatLite::GetTagWireType( tag );
            if GUCEF_PREDICT_FALSE( wireType > google::protobuf::internal::WireFormatLite::WIRETYPE_FIXED32 ) 
            {
                SetError( "Invalid wire type encountered" );
                return false;
            }
            #endif

            int field_number = google::protobuf::internal::WireFormatLite::GetTagFieldNumber( tag );
            const google::protobuf::FieldDescriptor* field = msgDescriptor->FindFieldByNumber( field_number );

            if ( GUCEF_NULL == field ) 
            {
                // Skip unknown field
                if ( !google::protobuf::internal::WireFormatLite::SkipField( input, tag ) ) 
                {
                    SetError( "Failed to skip unknown field" );
                    return false;
                }
                continue;
            }

            seenFieldNumbers.insert( field_number );

            if ( field->is_repeated() ) 
            {
                // Handle repeated fields
                if ( !ParseRepeatedField( msgDescriptor, msgFieldDescriptor, field, tag, input ) ) 
                {
                    return false;
                }
            } 
            else 
            {
                // Handle singular fields
                if ( !ParseSingularField( msgDescriptor, msgFieldDescriptor, field, input ) ) 
                {
                    return false;
                }
            }
        }

        // Emit default values for singular scalar fields not seen on the wire
        // In proto3 all scalar defaults are zero-values. This ensures round-trip fidelity
        // and that explicitly-set default values are preserved in the data tree
        EmitDefaultsForUnseenFields( msgDescriptor, msgFieldDescriptor, seenFieldNumbers );

        m_readCallbacks.OnNodeEnd( m_readPrivData, nodeName );

        return true;
    }

    /*---------------------------------------------------------------------------*/

    int GetGucefNodeDataType( const google::protobuf::FieldDescriptor* field )
    {GUCEF_TRACE;
        
        switch ( field->type() ) 
        {
            case google::protobuf::FieldDescriptor::TYPE_SINT32:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_INT32;
            case google::protobuf::FieldDescriptor::TYPE_SINT64:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_INT64;
            case google::protobuf::FieldDescriptor::TYPE_INT32:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_INT32;
            case google::protobuf::FieldDescriptor::TYPE_UINT32:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_UINT32;
            case google::protobuf::FieldDescriptor::TYPE_INT64:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_INT64;
            case google::protobuf::FieldDescriptor::TYPE_UINT64:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_UINT64;
            case google::protobuf::FieldDescriptor::TYPE_FIXED32:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_UINT32;
            case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_UINT64;
            case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_INT32;
            case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_INT64;
            case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_FLOAT32;
            case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_LE_FLOAT64;
            case google::protobuf::FieldDescriptor::TYPE_BOOL:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_BOOLEAN_INT32;
            case google::protobuf::FieldDescriptor::TYPE_STRING:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_UTF8_STRING;
            case google::protobuf::FieldDescriptor::TYPE_BYTES:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_BINARY_BLOB;
            case google::protobuf::FieldDescriptor::TYPE_GROUP: // Groups are deprecated in proto3, but if used, treat as object
            case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
            {
                if ( field->is_map() )
                    return GUCEF_DATATYPE_MAP;
                else
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                else
                    return GUCEF_DATATYPE_OBJECT;
            }
            case google::protobuf::FieldDescriptor::TYPE_ENUM:
                if ( field->is_repeated() )
                    return GUCEF_DATATYPE_ARRAY;
                return GUCEF_DATATYPE_ENUM;
            default:
                return GUCEF_DATATYPE_UNKNOWN;
        }
    }

    /*---------------------------------------------------------------------------*/

    bool ParseRepeatedField( const google::protobuf::Descriptor* msgDescriptor           ,
                             const google::protobuf::FieldDescriptor* msgFieldDescriptor ,
                             const google::protobuf::FieldDescriptor* field              ,
                             const uint32_t fieldTag                                     ,
                             google::protobuf::io::CodedInputStream* input               ) 
    {GUCEF_TRACE;

        // Note that protobuf assumes all values on the wire to be little endian
        // Parse functions make no effort to convert to host endian, hence values are
        // always assumed to be little endian and higher-level code must convert to host endian if needed.

        // In Protobuf, repeated fields can be encoded in two ways: packed and non-packed. 
        // The choice between the two depends on the type of data being encoded and the desired trade-offs between encoding size and compatibility.

        int nodeType = GetGucefNodeDataType( field );
        
        m_readCallbacks.OnNodeBegin( m_readPrivData, field->name().c_str(), nodeType );
        m_readCallbacks.OnNodeChildrenBegin( m_readPrivData, field->name().c_str() );
        
        if ( field->is_packed() ) 
        {
            // Handle packed repeated fields

            // Definition: 
            //        In packed encoding, all the elements of a repeated field are serialized as a single length-delimited block. This means the field's tag appears once, followed by the length of the packed data, and then the serialized values.
            // Supported Types: Packed encoding is only applicable to primitive numeric types, such as:
            //      int32, int64, uint32, uint64
            //	    sint32, sint64
            //      fixed32, fixed64
            //      sfixed32, sfixed64
            //      float, double
            //      bool

            uint32_t repeatedFieldBlockByteSize = 0;
            if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &repeatedFieldBlockByteSize ) ) 
            {
                SetError( "ParseRepeatedField: Failed to read packed field length" );
                return false;
            }
            google::protobuf::io::CodedInputStream::Limit limit = input->PushLimit( repeatedFieldBlockByteSize );

            while ( input->BytesUntilLimit() > 0 ) 
            {
                bool wasComplex = false;
                TVariantData valueVar;
                if GUCEF_PREDICT_FALSE( !ParseField( field, input, valueVar, wasComplex ) ) 
                {
                    SetError( "ParseRepeatedField: Failed to parse repeated field" );
                    return false;
                }
                if ( !wasComplex )
                {
                    m_readCallbacks.OnNodeValue( m_readPrivData, field->name().c_str(), &valueVar );
                }
                else
                {
                    SetError( "ParseRepeatedField: Unexpected complex field type for a 'packed' field" );
                }
            }

            input->PopLimit( limit );
        } 
        else 
        {
            // Handle non-packed repeated fields

            // Definition: 
            //      In non-packed encoding, each element of the repeated field is serialized as a separate key-value pair. This means the field's tag is repeated for every element.
            // Supported Types: Non-packed encoding is supported for all types, including:
            //      Primitive numeric types (same as packed fields).
            //      Non-numeric types, such as string, bytes, and message.
            
            if ( GUCEF_DATATYPE_MAP == nodeType )
            {
                if GUCEF_PREDICT_FALSE( !ParseMapField( field, fieldTag, input ) )
                {
                    SetError( "ParseRepeatedField: Failed to parse map field" );
                    return false;
                }
            }
            else
            {
                while ( input->BytesUntilLimit() > 0 ) 
                {
                    bool wasComplex = false;
                    TVariantData valueVar;
                    if GUCEF_PREDICT_FALSE( !ParseField( field, input, valueVar, wasComplex ) ) 
                    {
                        SetError( "ParseRepeatedField: Failed to parse repeated field" );
                        return false;
                    }
                    if ( !wasComplex )
                    {                        
                        m_readCallbacks.OnNodeValue( m_readPrivData, field->name().c_str(), &valueVar );
                    }

                    // In order to know if we are done with the repeated field we need to read the next tag
                    // as long as the wire tag value remains the same we are reading additional repeat entries of the same field
                    // 
                    // Let's peek ahead at the next tag without advancing the cursor unless the tag matches your expectation.
                    // the ExpectTag() function provides this out of the box
                    if ( !input->ExpectTag( fieldTag ) )
                    {
                        // End of repeated field entries
                        break;
                    }
                }
            }
        }

        m_readCallbacks.OnNodeChildrenEnd( m_readPrivData, field->name().c_str() );
        m_readCallbacks.OnNodeEnd( m_readPrivData, field->name().c_str() );

        return true;
    }

    /*---------------------------------------------------------------------------*/

    bool ParseSingularField( const google::protobuf::Descriptor* msgDescriptor           ,
                             const google::protobuf::FieldDescriptor* msgFieldDescriptor ,
                             const google::protobuf::FieldDescriptor* field              ,  
                             google::protobuf::io::CodedInputStream* input               ) 
    {GUCEF_TRACE;

        // Note that protobuf assumes all values on the wire to be little endian
        // also parse functions make no effort to convert to host endian, hence values are 
        // always assumed to be little endian and higher level code must convert to host endian if needed

        TVariantData keyVar;
        memset( &keyVar, 0, sizeof( keyVar ) );
        keyVar.containedType = GUCEF_DATATYPE_UTF8_STRING;
        keyVar.union_data.heap_data.heap_data_is_linked = 1;
        keyVar.union_data.heap_data.heap_data_size = static_cast< UInt32 >( field->name().size() );
        keyVar.union_data.heap_data.union_data.const_char_heap_data = field->name().c_str();

        bool wasComplex = false;
        TVariantData valueVar;
        if GUCEF_PREDICT_FALSE( !ParseField( field, input, valueVar, wasComplex ) ) 
        {
            SetError( "ParseSingularField: Failed to parse field" );
            return false;
        }
        if ( !wasComplex )
        {
            switch ( field->type() )
            {
                case google::protobuf::FieldDescriptor::TYPE_ENUM:
                {
                    // For enumerations we could convey the value itself as an attribute but that would cause us to lose the 
                    // context of it being part of an enumeration which indicates an additional constraint on the value
                    // as such we will convey an enum differently as a node with ENUM as the node type and the value of the node set to the 
                    // value of the enum. That way we can still convey the enum type and value
                    m_readCallbacks.OnNodeBegin( m_readPrivData, field->name().c_str(), GUCEF_DATATYPE_ENUM );
                    m_readCallbacks.OnNodeValue( m_readPrivData, field->name().c_str(), &valueVar );
                    m_readCallbacks.OnNodeEnd( m_readPrivData, field->name().c_str() );
                    break;
                }
                default:
                {
                    // if the field is part of a nested message instead of the root message we need to use the message field descriptor
                    // to get the correct name for the node
                    const char* nodeName = GUCEF_NULL != msgFieldDescriptor ? msgFieldDescriptor->name().c_str() : msgDescriptor->name().c_str();
                    
                    // All other types are just considered attributes
                    m_readCallbacks.OnNodeAtt( m_readPrivData, nodeName, &keyVar, &valueVar );
                    break;
                }
            }
        }

        return true;
    }
};

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
            return m_parser.ParseMessage( m_codecInfo->m_msgDescriptor , 
                                          m_codecInfo->m_message       , 
                                          m_access                     );
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
    
    std::ostringstream logStrStr;
    logStrStr << "protobuf: " << filename << " at " << line << " : " << message;
    std::string logStr = logStrStr.str();

    switch ( level )
    {
        case google::protobuf::LOGLEVEL_INFO:
            GUCEF_C_SYSTEM_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, logStr.c_str() );
            break;
        case google::protobuf::LOGLEVEL_WARNING:
            GUCEF_C_WARNING_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, logStr.c_str() );
            break;
        case google::protobuf::LOGLEVEL_ERROR:
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, logStr.c_str() );
            break;
        case google::protobuf::LOGLEVEL_FATAL:
            GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_CRITICAL, logStr.c_str() );
            break;
        default:
            GUCEF_C_SYSTEM_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, logStr.c_str() );
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
                           void** codecdata   ,
                           void** filedata    ,
                           TIOAccess* outFile ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == filedata )
        return 0;

    *filedata = GUCEF_NULL;

    if ( GUCEF_NULL == codecdata || GUCEF_NULL == *codecdata || GUCEF_NULL == outFile )
    {
        GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "DSTOREPLUG_Dest_File_Open: Invalid parameters" );
        return 0;
    }

    CDataDrivenCodecInfo* codecInfo = static_cast< CDataDrivenCodecInfo* >( *codecdata );

    CResourceWritingInfo* writingInfo = GUCEF_NEW CResourceWritingInfo();
    if ( GUCEF_NULL != writingInfo )
    {
        if ( writingInfo->Init( codecInfo, outFile ) )
        {
            *filedata = writingInfo;
            return 1;
        }
        GUCEF_DELETE writingInfo;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Close( void** plugdata  ,
                            void** codecdata ,
                            void** filedata  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == filedata || GUCEF_NULL == *filedata )
        return;

    CResourceWritingInfo* writingInfo = static_cast< CResourceWritingInfo* >( *filedata );

    if ( !writingInfo->SerializeToOutput() )
    {
        GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, "DSTOREPLUG_Dest_File_Close: Failed to serialize message to output" );
    }

    GUCEF_DELETE writingInfo;
    *filedata = GUCEF_NULL;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Begin_Node_Store( void** plugdata      ,
                             void** codecdata     ,
                             void** filedata      ,
                             const char* nodename ,
                             Int32 nodeType       ,
                             UInt32 attscount     ,
                             UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == filedata || GUCEF_NULL == *filedata )
        return;

    CResourceWritingInfo* writingInfo = static_cast< CResourceWritingInfo* >( *filedata );

    if ( writingInfo->m_contextStack.empty() )
    {
        writingInfo->PushContext( writingInfo->m_rootMessage               , 
                                 writingInfo->m_codecInfo->m_msgDescriptor , 
                                 GUCEF_NULL                                , 
                                 nodename != GUCEF_NULL ? nodename : ""    );
    }
    else
    {
        google::protobuf::Message* parentMsg = writingInfo->CurrentMessage();
        const google::protobuf::Descriptor* parentDesc = writingInfo->CurrentDescriptor();

        if ( GUCEF_NULL == parentMsg || GUCEF_NULL == parentDesc || GUCEF_NULL == nodename )
            return;

        const google::protobuf::FieldDescriptor* field = parentDesc->FindFieldByName( nodename );
        if ( GUCEF_NULL == field )
        {
            // Check if we're inside a repeated message array context.
            // In that case, each child node should create a new message in the repeated field.
            if ( !writingInfo->m_contextStack.empty() )
            {
                const CResourceWritingInfo::SMessageContext& ctx = writingInfo->m_contextStack.back();
                if ( GUCEF_NULL != ctx.fieldDesc && 
                     ctx.fieldDesc->is_repeated() && 
                     ctx.fieldDesc->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE &&
                     !ctx.fieldDesc->is_map() )
                {
                    // We're inside a repeated message array - create a new message for this child
                    const google::protobuf::Reflection* reflection = parentMsg->GetReflection();
                    google::protobuf::Message* childMsg = reflection->AddMessage( parentMsg, ctx.fieldDesc );
                    if ( GUCEF_NULL != childMsg )
                    {
                        writingInfo->TrackMessageField( parentMsg, ctx.fieldDesc );
                        writingInfo->PushContext( childMsg, ctx.fieldDesc->message_type(), ctx.fieldDesc, nodename );
                        return;
                    }
                }
            }
            
            writingInfo->PushContext( GUCEF_NULL, GUCEF_NULL, GUCEF_NULL, nodename );
            return;
        }

        if ( field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE )
        {
            // For map fields, we don't create MapEntry messages here.
            // Each attribute on the map node will create its own MapEntry in Store_Node_Att.
            if ( field->is_map() )
            {
                // Push a context with the parent message and the map field descriptor
                writingInfo->PushContext( parentMsg, parentDesc, field, nodename );
            }
            // For repeated message fields (arrays of messages), don't create a message for the array container.
            // Each child node will create its own message when processed.
            else if ( field->is_repeated() )
            {
                // Push a context that remembers we're in a repeated message field
                writingInfo->PushContext( parentMsg, parentDesc, field, nodename );
            }
            else
            {
                const google::protobuf::Reflection* reflection = parentMsg->GetReflection();
                google::protobuf::Message* childMsg = GUCEF_NULL;
                
                childMsg = reflection->MutableMessage( parentMsg, field );
                
                if ( GUCEF_NULL != childMsg )
                {
                    writingInfo->TrackMessageField( parentMsg, field );
                    writingInfo->PushContext( childMsg, field->message_type(), field, nodename );
                }
                else
                    writingInfo->PushContext( GUCEF_NULL, GUCEF_NULL, field, nodename );
            }
        }
        else
        {
            writingInfo->PushContext( parentMsg, parentDesc, field, nodename );
        }
    }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Store( void** plugdata      ,
                           void** codecdata     ,
                           void** filedata      ,
                           const char* nodename ,
                           UInt32 attscount     ,
                           UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == filedata || GUCEF_NULL == *filedata )
        return;

    CResourceWritingInfo* writingInfo = static_cast< CResourceWritingInfo* >( *filedata );
    writingInfo->PopContext();
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Store_Node_Att( void** plugdata              ,
                           void** codecdata             ,
                           void** filedata              ,
                           const char* nodename         ,
                           UInt32 attscount             ,
                           UInt32 attindex              ,
                           const char* attname          ,
                           const TVariantData* attvalue ,
                           UInt32 haschildren           ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == filedata || GUCEF_NULL == *filedata || GUCEF_NULL == attvalue )
        return;

    CResourceWritingInfo* writingInfo = static_cast< CResourceWritingInfo* >( *filedata );

    // Handle child value nodes of repeated scalar arrays FIRST, before NULL context checks.
    // When Begin_Node_Store("") is called for array child values, it pushes a NULL context.
    // We need to check the parent context to find the repeated scalar field.
    if ( GUCEF_NULL == attname && writingInfo->m_contextStack.size() >= 2 )
    {
        const CResourceWritingInfo::SMessageContext& parentCtx = writingInfo->m_contextStack[ writingInfo->m_contextStack.size() - 2 ];
        if ( GUCEF_NULL != parentCtx.fieldDesc && 
             GUCEF_NULL != parentCtx.message &&
             parentCtx.fieldDesc->is_repeated() && 
             parentCtx.fieldDesc->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE )
        {
            // We're a child value of a repeated scalar array
            writingInfo->SetFieldFromVariant( parentCtx.message, parentCtx.fieldDesc, attvalue );
            return;
        }
    }

    google::protobuf::Message* currentMsg = writingInfo->CurrentMessage();
    const google::protobuf::Descriptor* currentDesc = writingInfo->CurrentDescriptor();

    if ( GUCEF_NULL == currentMsg || GUCEF_NULL == currentDesc )
        return;

    if ( GUCEF_NULL == attname )
    {
        // Check the current context for other cases (oneof, etc.)
        if ( !writingInfo->m_contextStack.empty() )
        {
            const CResourceWritingInfo::SMessageContext& ctx = writingInfo->m_contextStack.back();
            if ( GUCEF_NULL != ctx.fieldDesc )
            {
                // For repeated scalar fields (arrays) where the context is directly on the array node
                if ( ctx.fieldDesc->is_repeated() && ctx.fieldDesc->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE )
                {
                    writingInfo->SetFieldFromVariant( currentMsg, ctx.fieldDesc, attvalue );
                }
                // For singular scalar fields (oneof or direct value nodes)
                else if ( ctx.fieldDesc->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE )
                {
                    if ( writingInfo->m_contextStack.size() >= 2 )
                    {
                        const CResourceWritingInfo::SMessageContext& parentCtx = writingInfo->m_contextStack[ writingInfo->m_contextStack.size() - 2 ];
                        if ( GUCEF_NULL != parentCtx.message )
                            writingInfo->SetFieldFromVariant( parentCtx.message, ctx.fieldDesc, attvalue );
                    }
                }
            }
        }
        return;
    }

    // Check if the current context is a map field
    // For map fields, each attribute represents a key-value pair, and we need to create a MapEntry message
    if ( !writingInfo->m_contextStack.empty() )
    {
        const CResourceWritingInfo::SMessageContext& ctx = writingInfo->m_contextStack.back();
        if ( GUCEF_NULL != ctx.fieldDesc && ctx.fieldDesc->is_map() )
        {
            // We're in a map field context. Each attribute is a map entry.
            // The parent message is the one that contains the map field.
            // We need to create a new MapEntry message and populate its key and value fields.
            
            const google::protobuf::Reflection* reflection = currentMsg->GetReflection();
            if ( GUCEF_NULL == reflection )
                return;
            
            // Add a new MapEntry message to the repeated field
            google::protobuf::Message* mapEntryMsg = reflection->AddMessage( currentMsg, ctx.fieldDesc );
            if ( GUCEF_NULL == mapEntryMsg )
                return;
            
            const google::protobuf::Descriptor* mapEntryDesc = mapEntryMsg->GetDescriptor();
            if ( GUCEF_NULL == mapEntryDesc )
                return;
            
            // Get the key field descriptor (always at index 0)
            const google::protobuf::FieldDescriptor* keyField = mapEntryDesc->field( 0 );
            // Get the value field descriptor (always at index 1)
            const google::protobuf::FieldDescriptor* valueField = mapEntryDesc->field( 1 );
            
            if ( GUCEF_NULL != keyField && GUCEF_NULL != valueField )
            {
                // Create a variant for the key (the attribute name)
                TVariantData keyVar;
                memset( &keyVar, 0, sizeof( keyVar ) );
                keyVar.containedType = GUCEF_DATATYPE_UTF8_STRING;
                keyVar.union_data.heap_data.heap_data_is_linked = 1;
                keyVar.union_data.heap_data.heap_data_size = static_cast< UInt32 >( strlen( attname ) );
                keyVar.union_data.heap_data.union_data.const_char_heap_data = attname;
                
                // Set the key field in the MapEntry message
                writingInfo->SetFieldFromVariant( mapEntryMsg, keyField, &keyVar );
                // Set the value field in the MapEntry message
                writingInfo->SetFieldFromVariant( mapEntryMsg, valueField, attvalue );
                
                // Track that we've set this map field
                writingInfo->TrackMessageField( currentMsg, ctx.fieldDesc );
            }
            return;
        }
    }

    const google::protobuf::FieldDescriptor* field = currentDesc->FindFieldByName( attname );
    if ( GUCEF_NULL == field )
        return;

    writingInfo->SetFieldFromVariant( currentMsg, field, attvalue );
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Begin_Node_Children( void** plugdata      ,
                                void** codecdata     ,
                                void** filedata      ,
                                const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Children( void** plugdata      ,
                              void** codecdata     ,
                              void** filedata      ,
                              const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

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
        return 1; // basic failure error code

    CDataDrivenCodecInfo* codecInfo = GUCEF_NEW CDataDrivenCodecInfo();
    if ( GUCEF_NULL == codecInfo )
        return 2; // basic failure error code
    
    if ( !codecInfo->Init( codecMeta, loadedResources ) )
    {
        GUCEF_DELETE codecInfo;
        return 3; // basic failure error code
    }

    *dataDrivenCodecPrivateData = codecInfo;
    return 0; // 0 == success
}

/*---------------------------------------------------------------------------*/

UInt8 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Destroy_Data_Driven_Codec( const void* plugdata                  ,
                                      TDataDrivenDStoreCodecMeta* codecMeta ,
                                      void** dataDrivenCodecPrivateData     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == dataDrivenCodecPrivateData )
        return 1; // basic failure error code

    CDataDrivenCodecInfo* codecInfo = static_cast< CDataDrivenCodecInfo* >( *dataDrivenCodecPrivateData );
    if ( GUCEF_NULL == codecInfo )
        return 2; // basic failure error code
    
    try
    {
        GUCEF_DELETE codecInfo;
    }
    catch ( const std::exception& e )
    {
        std::string errMsg( "DSTOREPLUG_Destroy_Data_Driven_Codec: Exception deleting codec info: " );
        errMsg += e.what();
        GUCEF_C_EXCEPTION_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, errMsg.c_str() );
        return 3; // basic failure error code
    }

    *dataDrivenCodecPrivateData = GUCEF_NULL;
    return 0;  // 0 == success
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

