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

        m_descriptor = fileDescriptor->FindMessageTypeByName( msgTypeName );
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
    std::string m_stringFieldBuffer;
    std::vector<UInt8> m_byteFieldBuffer;

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
        , m_byteFieldBuffer()
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

        bool parseResult = ParseFields( descriptor, &coded_input );

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

    bool ParseFields( const google::protobuf::Descriptor* descriptor , 
                      google::protobuf::io::CodedInputStream* input  ) 
    {GUCEF_TRACE;

        m_readCallbacks.OnNodeBegin( m_readPrivData, descriptor->name().c_str(), GUCEF_DATATYPE_OBJECT );

        while ( input->BytesUntilLimit() > 0 ) 
        {
            uint32_t tag = input->ReadTag();
            int field_number = google::protobuf::internal::WireFormatLite::GetTagFieldNumber( tag );
            const google::protobuf::FieldDescriptor* field = descriptor->FindFieldByNumber( field_number );

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
                if ( !ParseRepeatedField( descriptor, field, input ) ) 
                {
                    return false;
                }
            } 
            else 
            {
                // Handle singular fields
                if ( !ParseSingularField( descriptor, field, input ) ) 
                {
                    return false;
                }
            }
        }

        m_readCallbacks.OnNodeEnd( m_readPrivData, descriptor->name().c_str() );

        return true;
    }

    /*---------------------------------------------------------------------------*/

    bool ParseRepeatedField( const google::protobuf::Descriptor* msgDescriptor ,
                             const google::protobuf::FieldDescriptor* field    , 
                             google::protobuf::io::CodedInputStream* input     ) 
    {GUCEF_TRACE;

        // Note that protobuf assumes all values on the wire to be little endian
        // also parse functions make no effort to convert to host endian, hence values are 
        // always assumed to be little endian and higher level code must convert to host endian if needed

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

                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT32;
                valueVar.union_data.int32_data = static_cast< int32_t >( value ) ;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
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
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_UINT32;
                valueVar.union_data.uint32_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
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
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT64;
                valueVar.union_data.int64_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
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
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_UINT64;
                valueVar.union_data.uint64_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_FIXED64:
            {
                uint64_t value = 0;
                if ( !input->ReadLittleEndian64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read fixed size uint64 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_UINT64;
                valueVar.union_data.uint64_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_FIXED32:
            {
                uint32_t value = 0;
                if ( !input->ReadLittleEndian32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read fixed size uint32 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_UINT32;
                valueVar.union_data.uint32_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
            {
                uint64_t value = 0;
                if ( !input->ReadLittleEndian64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read fixed size int64 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT64;
                valueVar.union_data.uint64_data = static_cast< Int64 >( value );
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
            {
                uint32_t value = 0;
                if ( !input->ReadLittleEndian32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read fixed size int32 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT32;
                valueVar.union_data.uint32_data = static_cast< Int32 >( value );                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SINT32:
            {
                int32_t value = 0;
                if ( !ReadSInt32Field( input, value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read zigZag int32 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT32;
                valueVar.union_data.int32_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );               
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SINT64:
            {
                int64_t value = 0;
                if ( !ReadSInt64Field( input, value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read zigZag int64 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT64;
                valueVar.union_data.int64_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );               
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
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_FLOAT32;
                valueVar.union_data.float32_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
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
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_FLOAT64;
                valueVar.union_data.float64_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
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
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_BOOLEAN_INT32;
                valueVar.union_data.int32_data = static_cast< Int32 >( value );
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_STRING: 
            {
                if ( !ReadStringField( input, m_stringFieldBuffer ) ) 
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read string value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_UTF8_STRING;
                valueVar.union_data.heap_data.heap_data_size = static_cast< UInt32 >( m_stringFieldBuffer.size() );
                valueVar.union_data.heap_data.heap_data_is_linked = 1;
                valueVar.union_data.heap_data.union_data.const_char_heap_data = m_stringFieldBuffer.c_str();
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_MESSAGE: 
            {
                const google::protobuf::Descriptor* nestedMsgDescriptor = field->message_type();
                if ( !ParseFields( nestedMsgDescriptor, input ) ) 
                {
                    return false;
                }
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_BYTES:
            {
                if GUCEF_PREDICT_FALSE( !ReadBytesField( input, m_byteFieldBuffer ) )
                {
                    SetError( "ProtoSAXParser:ParseRepeatedField: Failed to read byte array field" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_BINARY_BLOB;
                valueVar.union_data.heap_data.heap_data_size = static_cast< UInt32 >( m_byteFieldBuffer.size() );
                valueVar.union_data.heap_data.heap_data_is_linked = 1;
                valueVar.union_data.heap_data.union_data.void_heap_data = &m_byteFieldBuffer[0];                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }

            // Handle other types as needed
            default:
            {
                Int32 typeId = static_cast< Int32 >( field->type() );
                std::ostringstream debugStrStr;
                debugStrStr << "ProtoSAXParser:ParseRepeatedField: Unsupported field type for repeated field: " << typeId;
                std::string debugStr = debugStrStr.str();
                SetError( debugStr );
                return false;
            }
        }
        return true;
    }

    /*---------------------------------------------------------------------------*/

    bool ParseSingularField( const google::protobuf::Descriptor* msgDescriptor ,
                             const google::protobuf::FieldDescriptor* field    , 
                             google::protobuf::io::CodedInputStream* input     ) 
    {GUCEF_TRACE;

        // Note that protobuf assumes all values on the wire to be little endian
        // also parse functions make no effort to convert to host endian, hence values are 
        // always assumed to be little endian and higher level code must convert to host endian if needed

        switch ( field->type() ) 
        {
            case google::protobuf::FieldDescriptor::TYPE_INT32: 
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read int32 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT32;
                valueVar.union_data.int32_data = static_cast< Int32 >( value );
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_INT64: 
            {
                uint64_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read int64 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT64;
                valueVar.union_data.int64_data = static_cast< Int64 >( value );
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_UINT32: 
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read uint32 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_UINT32;
                valueVar.union_data.uint32_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_UINT64: 
            {
                uint64_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read uint64 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_UINT64;
                valueVar.union_data.uint64_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_FIXED64:
            {
                uint64_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read fixed size uint64 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_UINT64;
                valueVar.union_data.uint64_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_FIXED32:
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read fixed size uint32 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_UINT32;
                valueVar.union_data.uint32_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
            {
                uint64_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian64( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read fixed size int64 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT64;
                valueVar.union_data.uint64_data = static_cast< Int64 >( value );
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read fixed size int32 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT32;
                valueVar.union_data.uint32_data = static_cast< Int32 >( value );                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SINT32:
            {
                int32_t value = 0;
                if GUCEF_PREDICT_FALSE( !ReadSInt32Field( input, value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read zigZag int32 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT32;
                valueVar.union_data.int32_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );          
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_SINT64:
            {
                int64_t value = 0;
                if GUCEF_PREDICT_FALSE( !ReadSInt64Field( input, value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read zigZag int64 value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_INT64;
                valueVar.union_data.int64_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );                         
                break;
            }  
            case google::protobuf::FieldDescriptor::TYPE_FLOAT: 
            {
                float value = 0.0f;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian32( reinterpret_cast<uint32_t*>(&value) ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read float value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_FLOAT32;
                valueVar.union_data.float32_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_DOUBLE: 
            {
                double value = 0.0;
                if GUCEF_PREDICT_FALSE( !input->ReadLittleEndian64( reinterpret_cast<uint64_t*>(&value) ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read double value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_LE_FLOAT64;
                valueVar.union_data.float64_data = value;                
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_BOOL: 
            {
                uint32_t value = 0;
                if GUCEF_PREDICT_FALSE( !input->ReadVarint32( &value ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read bool value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_BOOLEAN_INT32;
                valueVar.union_data.int32_data = static_cast< Int32 >( value );
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_STRING: 
            {
                if GUCEF_PREDICT_FALSE( !ReadStringField( input, m_stringFieldBuffer ) ) 
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read string value" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_UTF8_STRING;
                valueVar.union_data.heap_data.heap_data_size = static_cast< UInt32 >( m_stringFieldBuffer.size() );
                valueVar.union_data.heap_data.heap_data_is_linked = 1;
                valueVar.union_data.heap_data.union_data.const_char_heap_data = m_stringFieldBuffer.c_str();
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_MESSAGE: 
            {
                const google::protobuf::Descriptor* nestedMsgDescriptor = field->message_type();
                if GUCEF_PREDICT_FALSE( !ParseFields( nestedMsgDescriptor, input ) ) 
                {
                    return false;
                }
                break;
            }
            case google::protobuf::FieldDescriptor::TYPE_BYTES:
            {
                if GUCEF_PREDICT_FALSE( !ReadBytesField( input, m_byteFieldBuffer ) )
                {
                    SetError( "ProtoSAXParser:ParseSingularField: Failed to read byte array field" );
                    return false;
                }
                TVariantData valueVar;
                valueVar.containedType = GUCEF_DATATYPE_BINARY_BLOB;
                valueVar.union_data.heap_data.heap_data_size = static_cast< UInt32 >( m_byteFieldBuffer.size() );
                valueVar.union_data.heap_data.heap_data_is_linked = 1;
                if ( valueVar.union_data.heap_data.heap_data_size > 0 )
                    valueVar.union_data.heap_data.union_data.void_heap_data = &m_byteFieldBuffer[0];                
                else
                    valueVar.union_data.heap_data.union_data.void_heap_data = GUCEF_NULL;
                m_readCallbacks.OnNodeAtt( m_readPrivData, msgDescriptor->name().c_str(), field->name().c_str(), &valueVar );
                break;
            }

            // Handle other types as needed
            default:
            {
                Int32 typeId = static_cast< Int32 >( field->type() );
                std::ostringstream debugStrStr;
                debugStrStr << "ProtoSAXParser:ParseSingularField: Unsupported field type for singular field: " << typeId;
                std::string debugStr = debugStrStr.str();
                SetError( debugStr );
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

