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

#ifndef GUCEF_DSTOREPLUGIN_PROTOBUF_CRESOURCEWRITINGINFO_H
#define GUCEF_DSTOREPLUGIN_PROTOBUF_CRESOURCEWRITINGINFO_H

/*-------------------------------------------------------------------------*/

class GUCEF_HIDDEN CResourceWritingInfo
{
    public:

    struct SMessageContext
    {
        google::protobuf::Message* message;                    
        const google::protobuf::Descriptor* descriptor;       
        const google::protobuf::FieldDescriptor* fieldDesc;   
        std::string nodeName;

        SMessageContext( void )
            : message( GUCEF_NULL )
            , descriptor( GUCEF_NULL )
            , fieldDesc( GUCEF_NULL )
            , nodeName()
        {}
    };

    TIOAccess* m_access;
    CDataDrivenCodecInfo* m_codecInfo;
    google::protobuf::Message* m_rootMessage;
    std::vector< SMessageContext > m_contextStack;
    bool m_hasError;
    std::string m_errorMessage;

    void SetError( const std::string& message )
    {GUCEF_TRACE;

        m_hasError = true;
        m_errorMessage = message;
        GUCEF_C_ERROR_LOG( g_libApi, GUCEF_LOGLEVEL_NORMAL, message.c_str() );
    }

    bool Init( CDataDrivenCodecInfo* codecInfo ,
               TIOAccess* access               )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == codecInfo || 
             GUCEF_NULL == access     )
        {
            SetError( "CResourceWritingInfo:Init: Invalid parameters" );
            return false;
        }

        m_access = access;
        m_codecInfo = codecInfo;

        if ( GUCEF_NULL == m_codecInfo->m_message )
        {
            SetError( "CResourceWritingInfo:Init: No message prototype available" );
            return false;
        }

        m_rootMessage = m_codecInfo->m_message->New();
        if ( GUCEF_NULL == m_rootMessage )
        {
            SetError( "CResourceWritingInfo:Init: Failed to create new message instance" );
            return false;
        }

        return true;
    }

    google::protobuf::Message* CurrentMessage( void ) const
    {GUCEF_TRACE;

        if ( !m_contextStack.empty() )
            return m_contextStack.back().message;
        return GUCEF_NULL;
    }

    const google::protobuf::Descriptor* CurrentDescriptor( void ) const
    {GUCEF_TRACE;

        if ( !m_contextStack.empty() )
            return m_contextStack.back().descriptor;
        return GUCEF_NULL;
    }

    void PushContext( google::protobuf::Message* msg                      ,
                      const google::protobuf::Descriptor* desc            ,
                      const google::protobuf::FieldDescriptor* fieldDesc ,
                      const std::string& nodeName                         )
    {GUCEF_TRACE;

        SMessageContext ctx;
        ctx.message = msg;
        ctx.descriptor = desc;
        ctx.fieldDesc = fieldDesc;
        ctx.nodeName = nodeName;
        m_contextStack.push_back( ctx );
    }

    void PopContext( void )
    {GUCEF_TRACE;

        if ( !m_contextStack.empty() )
            m_contextStack.pop_back();
    }

    bool SetFieldFromVariant( google::protobuf::Message* msg                     ,
                              const google::protobuf::FieldDescriptor* field     ,
                              const TVariantData* attvalue                       )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == msg || GUCEF_NULL == field || GUCEF_NULL == attvalue )
            return false;

        const google::protobuf::Reflection* reflection = msg->GetReflection();
        if ( GUCEF_NULL == reflection )
            return false;

        switch ( field->type() )
        {
            case google::protobuf::FieldDescriptor::TYPE_INT32:
            case google::protobuf::FieldDescriptor::TYPE_SINT32:
            case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
            {
                Int32 value = 0;
                switch ( attvalue->containedType )
                {
                    case GUCEF_DATATYPE_INT8:          value = attvalue->union_data.int8_data; break;
                    case GUCEF_DATATYPE_UINT8:         value = attvalue->union_data.uint8_data; break;
                    case GUCEF_DATATYPE_INT16:         value = attvalue->union_data.int16_data; break;
                    case GUCEF_DATATYPE_UINT16:        value = attvalue->union_data.uint16_data; break;
                    case GUCEF_DATATYPE_INT32:         value = attvalue->union_data.int32_data; break;
                    case GUCEF_DATATYPE_UINT32:        value = static_cast< Int32 >( attvalue->union_data.uint32_data ); break;
                    case GUCEF_DATATYPE_INT64:         value = static_cast< Int32 >( attvalue->union_data.int64_data ); break;
                    case GUCEF_DATATYPE_UINT64:        value = static_cast< Int32 >( attvalue->union_data.uint64_data ); break;
                    case GUCEF_DATATYPE_BOOLEAN_INT32:  value = attvalue->union_data.int32_data; break;
                    default:                           value = attvalue->union_data.int32_data; break;
                }
                if ( field->is_repeated() )
                    reflection->AddInt32( msg, field, value );
                else
                    reflection->SetInt32( msg, field, value );
                return true;
            }
            case google::protobuf::FieldDescriptor::TYPE_INT64:
            case google::protobuf::FieldDescriptor::TYPE_SINT64:
            case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
            {
                Int64 value = 0;
                switch ( attvalue->containedType )
                {
                    case GUCEF_DATATYPE_INT8:          value = attvalue->union_data.int8_data; break;
                    case GUCEF_DATATYPE_UINT8:         value = attvalue->union_data.uint8_data; break;
                    case GUCEF_DATATYPE_INT16:         value = attvalue->union_data.int16_data; break;
                    case GUCEF_DATATYPE_UINT16:        value = attvalue->union_data.uint16_data; break;
                    case GUCEF_DATATYPE_INT32:         value = attvalue->union_data.int32_data; break;
                    case GUCEF_DATATYPE_UINT32:        value = attvalue->union_data.uint32_data; break;
                    case GUCEF_DATATYPE_INT64:         value = attvalue->union_data.int64_data; break;
                    case GUCEF_DATATYPE_UINT64:        value = static_cast< Int64 >( attvalue->union_data.uint64_data ); break;
                    case GUCEF_DATATYPE_BOOLEAN_INT32:  value = attvalue->union_data.int32_data; break;
                    default:                           value = attvalue->union_data.int64_data; break;
                }
                if ( field->is_repeated() )
                    reflection->AddInt64( msg, field, value );
                else
                    reflection->SetInt64( msg, field, value );
                return true;
            }
            case google::protobuf::FieldDescriptor::TYPE_UINT32:
            case google::protobuf::FieldDescriptor::TYPE_FIXED32:
            {
                UInt32 value = 0;
                switch ( attvalue->containedType )
                {
                    case GUCEF_DATATYPE_INT8:          value = static_cast< UInt32 >( attvalue->union_data.int8_data ); break;
                    case GUCEF_DATATYPE_UINT8:         value = attvalue->union_data.uint8_data; break;
                    case GUCEF_DATATYPE_INT16:         value = static_cast< UInt32 >( attvalue->union_data.int16_data ); break;
                    case GUCEF_DATATYPE_UINT16:        value = attvalue->union_data.uint16_data; break;
                    case GUCEF_DATATYPE_INT32:         value = static_cast< UInt32 >( attvalue->union_data.int32_data ); break;
                    case GUCEF_DATATYPE_UINT32:        value = attvalue->union_data.uint32_data; break;
                    case GUCEF_DATATYPE_BOOLEAN_INT32:  value = static_cast< UInt32 >( attvalue->union_data.int32_data ); break;
                    default:                           value = attvalue->union_data.uint32_data; break;
                }
                if ( field->is_repeated() )
                    reflection->AddUInt32( msg, field, value );
                else
                    reflection->SetUInt32( msg, field, value );
                return true;
            }
            case google::protobuf::FieldDescriptor::TYPE_UINT64:
            case google::protobuf::FieldDescriptor::TYPE_FIXED64:
            {
                UInt64 value = 0;
                switch ( attvalue->containedType )
                {
                    case GUCEF_DATATYPE_INT32:         value = static_cast< UInt64 >( attvalue->union_data.int32_data ); break;
                    case GUCEF_DATATYPE_UINT32:        value = attvalue->union_data.uint32_data; break;
                    case GUCEF_DATATYPE_INT64:         value = static_cast< UInt64 >( attvalue->union_data.int64_data ); break;
                    case GUCEF_DATATYPE_UINT64:        value = attvalue->union_data.uint64_data; break;
                    default:                           value = attvalue->union_data.uint64_data; break;
                }
                if ( field->is_repeated() )
                    reflection->AddUInt64( msg, field, value );
                else
                    reflection->SetUInt64( msg, field, value );
                return true;
            }
            case google::protobuf::FieldDescriptor::TYPE_FLOAT:
            {
                float value = 0.0f;
                switch ( attvalue->containedType )
                {
                    case GUCEF_DATATYPE_FLOAT32:       value = attvalue->union_data.float32_data; break;
                    case GUCEF_DATATYPE_FLOAT64:       value = static_cast< float >( attvalue->union_data.float64_data ); break;
                    case GUCEF_DATATYPE_INT32:         value = static_cast< float >( attvalue->union_data.int32_data ); break;
                    default:                           value = attvalue->union_data.float32_data; break;
                }
                if ( field->is_repeated() )
                    reflection->AddFloat( msg, field, value );
                else
                    reflection->SetFloat( msg, field, value );
                return true;
            }
            case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
            {
                double value = 0.0;
                switch ( attvalue->containedType )
                {
                    case GUCEF_DATATYPE_FLOAT32:       value = attvalue->union_data.float32_data; break;
                    case GUCEF_DATATYPE_FLOAT64:       value = attvalue->union_data.float64_data; break;
                    case GUCEF_DATATYPE_INT32:         value = static_cast< double >( attvalue->union_data.int32_data ); break;
                    case GUCEF_DATATYPE_INT64:         value = static_cast< double >( attvalue->union_data.int64_data ); break;
                    default:                           value = attvalue->union_data.float64_data; break;
                }
                if ( field->is_repeated() )
                    reflection->AddDouble( msg, field, value );
                else
                    reflection->SetDouble( msg, field, value );
                return true;
            }
            case google::protobuf::FieldDescriptor::TYPE_BOOL:
            {
                bool value = false;
                switch ( attvalue->containedType )
                {
                    case GUCEF_DATATYPE_BOOLEAN_INT32:  value = ( 0 != attvalue->union_data.int32_data ); break;
                    case GUCEF_DATATYPE_INT32:         value = ( 0 != attvalue->union_data.int32_data ); break;
                    case GUCEF_DATATYPE_UINT32:        value = ( 0 != attvalue->union_data.uint32_data ); break;
                    default:                           value = ( 0 != attvalue->union_data.int32_data ); break;
                }
                if ( field->is_repeated() )
                    reflection->AddBool( msg, field, value );
                else
                    reflection->SetBool( msg, field, value );
                return true;
            }
            case google::protobuf::FieldDescriptor::TYPE_STRING:
            {
                std::string value;
                if ( GUCEF_DATATYPE_UTF8_STRING == attvalue->containedType || 
                     GUCEF_DATATYPE_ASCII_STRING == attvalue->containedType )
                {
                    if ( GUCEF_NULL != attvalue->union_data.heap_data.union_data.char_heap_data && attvalue->union_data.heap_data.heap_data_size > 0 )
                        value.assign( attvalue->union_data.heap_data.union_data.char_heap_data, attvalue->union_data.heap_data.heap_data_size );
                }
                if ( field->is_repeated() )
                    reflection->AddString( msg, field, value );
                else
                    reflection->SetString( msg, field, value );
                return true;
            }
            case google::protobuf::FieldDescriptor::TYPE_BYTES:
            {
                std::string value;
                if ( GUCEF_DATATYPE_BINARY_BLOB == attvalue->containedType )
                {
                    if ( GUCEF_NULL != attvalue->union_data.heap_data.union_data.void_heap_data && attvalue->union_data.heap_data.heap_data_size > 0 )
                        value.assign( static_cast< const char* >( attvalue->union_data.heap_data.union_data.void_heap_data ), attvalue->union_data.heap_data.heap_data_size );
                }
                else if ( GUCEF_DATATYPE_BINARY_BSOB == attvalue->containedType )
                {
                    value.assign( reinterpret_cast< const char* >( attvalue->union_data.bsob_data ), GUCEF_VARIANT_BSOB_SIZE );
                }
                if ( field->is_repeated() )
                    reflection->AddString( msg, field, value );
                else
                    reflection->SetString( msg, field, value );
                return true;
            }
            case google::protobuf::FieldDescriptor::TYPE_ENUM:
            {
                Int32 value = 0;
                switch ( attvalue->containedType )
                {
                    case GUCEF_DATATYPE_INT32:         value = attvalue->union_data.int32_data; break;
                    case GUCEF_DATATYPE_UINT32:        value = static_cast< Int32 >( attvalue->union_data.uint32_data ); break;
                    default:                           value = attvalue->union_data.int32_data; break;
                }
                const google::protobuf::EnumValueDescriptor* enumValue = field->enum_type()->FindValueByNumber( value );
                if ( GUCEF_NULL != enumValue )
                {
                    if ( field->is_repeated() )
                        reflection->AddEnum( msg, field, enumValue );
                    else
                        reflection->SetEnum( msg, field, enumValue );
                }
                return true;
            }
            case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
            {
                return true;
            }
            default:
            {
                std::ostringstream errStr;
                errStr << "CResourceWritingInfo:SetFieldFromVariant: Unsupported field type: " << static_cast< int >( field->type() );
                SetError( errStr.str() );
                return false;
            }
        }
    }

    bool SerializeToOutput( void )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_rootMessage || GUCEF_NULL == m_access )
        {
            SetError( "CResourceWritingInfo:SerializeToOutput: No message or access available" );
            return false;
        }

        std::string serialized;
        if ( !m_rootMessage->SerializeToString( &serialized ) )
        {
            SetError( "CResourceWritingInfo:SerializeToOutput: Failed to serialize message" );
            return false;
        }

        if ( !serialized.empty() )
        {
            UInt32 bytesWritten = m_access->write( m_access, serialized.data(), 1, static_cast< UInt32 >( serialized.size() ) );
            if ( bytesWritten != static_cast< UInt32 >( serialized.size() ) )
            {
                SetError( "CResourceWritingInfo:SerializeToOutput: Failed to write serialized data to IOAccess" );
                return false;
            }
        }

        return true;
    }

    CResourceWritingInfo( void ) 
        : m_access( GUCEF_NULL ) 
        , m_codecInfo( GUCEF_NULL )
        , m_rootMessage( GUCEF_NULL )
        , m_contextStack()
        , m_hasError( false )
        , m_errorMessage()
    {GUCEF_TRACE;
            
    }

    ~CResourceWritingInfo( void )
    {GUCEF_TRACE;

        delete m_rootMessage;
        m_rootMessage = GUCEF_NULL;
    }

    private:

    CResourceWritingInfo( const CResourceWritingInfo& );
    CResourceWritingInfo& operator=( const CResourceWritingInfo& );
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DSTOREPLUGIN_PROTOBUF_CRESOURCEWRITINGINFO_H ? */
