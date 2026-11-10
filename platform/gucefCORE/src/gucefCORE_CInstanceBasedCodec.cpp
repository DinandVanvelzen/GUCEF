/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#include "gucefCORE_CInstanceBasedCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How much of the input a one shot call of a streaming codec takes per step */
static const UInt32 StreamChunkSize = 4096;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CInstanceBasedCodec::CInstanceBasedCodec( const CString& familyName ,
                                          const CString& typeName   ,
                                          UInt32 capabilities       )
    : CICodec()
    , m_familyName( familyName )
    , m_typeName( typeName )
    , m_capabilities( capabilities )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CInstanceBasedCodec::CInstanceBasedCodec( const CInstanceBasedCodec& src )
    : CICodec( src )
    , m_familyName( src.m_familyName )
    , m_typeName( src.m_typeName )
    , m_capabilities( src.m_capabilities )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CInstanceBasedCodec::~CInstanceBasedCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CInstanceBasedCodec::Encode( CIOAccess& source ,
                             CIOAccess& dest   )
{GUCEF_TRACE;

    CValueList noParams;
    return Run( true, source, dest, noParams );
}

/*-------------------------------------------------------------------------*/

bool
CInstanceBasedCodec::Decode( CIOAccess& source ,
                             CIOAccess& dest   )
{GUCEF_TRACE;

    CValueList noParams;
    return Run( false, source, dest, noParams );
}

/*-------------------------------------------------------------------------*/

bool
CInstanceBasedCodec::Encode( CIOAccess& source        ,
                             CIOAccess& dest          ,
                             const CValueList& params )
{GUCEF_TRACE;

    return Run( true, source, dest, params );
}

/*-------------------------------------------------------------------------*/

bool
CInstanceBasedCodec::Decode( CIOAccess& source        ,
                             CIOAccess& dest          ,
                             const CValueList& params )
{GUCEF_TRACE;

    return Run( false, source, dest, params );
}

/*-------------------------------------------------------------------------*/

bool
CInstanceBasedCodec::CreateInstance( const CValueList& params     ,
                                     CICodecInstancePtr& instance )
{GUCEF_TRACE;

    CString errorMessage;
    TCodecStatus status = CreateInstanceImpl( params, instance, errorMessage );
    if ( GUCEF_CODEC_STATUS_OK == status && instance.IsNULL() )
        status = GUCEF_CODEC_STATUS_FAILED;
    SetLastStatus( status, errorMessage.C_String() );
    return GUCEF_CODEC_STATUS_OK == status;
}

/*-------------------------------------------------------------------------*/

bool
CInstanceBasedCodec::Run( bool encode               ,
                          CIOAccess& source         ,
                          CIOAccess& dest           ,
                          const CValueList& params  )
{GUCEF_TRACE;

    const UInt32 capability = encode ? GUCEF_CODEC_CAPABILITY_ENCODE : GUCEF_CODEC_CAPABILITY_DECODE;
    if ( 0 == ( m_capabilities & capability ) )
    {
        SetLastStatus( GUCEF_CODEC_STATUS_UNSUPPORTED, encode ? "The codec does not encode" : "The codec does not decode" );
        return false;
    }

    CICodecInstancePtr instance;
    CString errorMessage;
    TCodecStatus status = CreateInstanceImpl( params, instance, errorMessage );
    if ( GUCEF_CODEC_STATUS_OK != status || instance.IsNULL() )
    {
        SetLastStatus( GUCEF_CODEC_STATUS_OK == status ? GUCEF_CODEC_STATUS_FAILED : status, errorMessage.C_String() );
        return false;
    }

    // For one shot use the per call arguments come from the parameters
    SCodecCallArgs args;
    memset( &args, 0, sizeof( args ) );
    args.structSize = (UInt32) sizeof( args );
    TryGetBinaryParam( params, "nonce", args.nonce, args.nonceSize );
    TryGetBinaryParam( params, "aad", args.aad, args.aadSize );

    const UInt32 direction = encode ? GUCEF_CODEC_PROCESS_ENCODE : GUCEF_CODEC_PROCESS_DECODE;
    CDynamicBuffer output;
    bool success = true;
    if ( 0 != ( m_capabilities & GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY ) )
    {
        CDynamicBuffer input;
        input.Append( source );
        success = instance->Process( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL | direction, input.GetConstBufferPtr(), input.GetDataSize(), &args, output );
        if ( success && output.GetDataSize() > 0 )
            success = output.GetDataSize() == dest.Write( output.GetConstBufferPtr(), 1, output.GetDataSize() );
    }
    else
    {
        UInt8 chunk[ StreamChunkSize ];
        UInt32 flags = GUCEF_CODEC_PROCESS_BEGIN | direction;
        bool done = false;
        while ( success && !done )
        {
            UInt32 bytesRead = source.Read( chunk, 1, StreamChunkSize );
            if ( 0 == bytesRead )
            {
                flags |= GUCEF_CODEC_PROCESS_FINAL;
                done = true;
            }
            success = instance->Process( flags, chunk, bytesRead, &args, output );
            flags = GUCEF_CODEC_PROCESS_UPDATE | direction;

            if ( success && output.GetDataSize() > 0 )
            {
                success = output.GetDataSize() == dest.Write( output.GetConstBufferPtr(), 1, output.GetDataSize() );
                output.Clear( true );
            }
        }
    }

    if ( !success && GUCEF_CODEC_STATUS_OK == instance->GetLastStatus() )
    {
        SetLastStatus( GUCEF_CODEC_STATUS_FAILED, "Unable to write the output" );
        return false;
    }
    SetLastStatus( instance->GetLastStatus(), success ? GUCEF_NULL : instance->GetLastErrorMessage().C_String() );
    return success;
}

/*-------------------------------------------------------------------------*/

UInt32
CInstanceBasedCodec::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

CString
CInstanceBasedCodec::GetType( void ) const
{GUCEF_TRACE;

    return m_typeName;
}

/*-------------------------------------------------------------------------*/

CString
CInstanceBasedCodec::GetFamilyName( void ) const
{GUCEF_TRACE;

    return m_familyName;
}

/*-------------------------------------------------------------------------*/

bool
CInstanceBasedCodec::TryGetBinaryParam( const CValueList& params ,
                                        const char* name         ,
                                        const void*& data        ,
                                        UInt32& dataSize         )
{GUCEF_TRACE;

    if ( !params.HasKey( name ) )
        return false;

    const CVariant& value = params.GetValue( name );
    if ( value.IsNULLOrEmpty() && !value.IsBinary() )
        return false;

    data = value.AsVoidPtr();
    dataSize = value.IsString() ? value.ByteSize( false ) : value.ByteSize();
    return GUCEF_NULL != data || 0 == dataSize;
}

/*-------------------------------------------------------------------------*/

bool
CInstanceBasedCodec::TryGetUInt32Param( const CValueList& params ,
                                        const char* name         ,
                                        UInt32& value            )
{GUCEF_TRACE;

    if ( !params.HasKey( name ) )
        return false;

    const CVariant& paramValue = params.GetValue( name );
    if ( paramValue.IsString() )
    {
        // text must be a number as a whole, so a typing error is not taken for 0
        CString text = paramValue.AsString();
        if ( text.IsNULLOrEmpty() )
            return false;
        for ( UInt32 i=0; i<text.Length(); ++i )
        {
            char c = text[ i ];
            if ( c < '0' || c > '9' )
                return false;
        }
    }
    else
    if ( !paramValue.IsInteger() )
        return false;

    value = paramValue.AsUInt32( 0, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CInstanceBasedCodec::IsHexOutputRequested( const CValueList& params ,
                                           bool defaultHex          ,
                                           bool& isValid            )
{GUCEF_TRACE;

    isValid = true;
    if ( !params.HasKey( "output" ) )
        return defaultHex;

    CString output = params.GetValue( "output" ).AsString().Lowercase();
    if ( "hex" == output )
        return true;
    if ( "raw" == output )
        return false;
    isValid = false;
    return defaultHex;
}

/*-------------------------------------------------------------------------*/

void
CInstanceBasedCodec::AppendHex( const UInt8* bytes        ,
                                UInt32 byteCount          ,
                                CDynamicBuffer& output    )
{GUCEF_TRACE;

    static const char* hexDigits = "0123456789abcdef";
    char pair[ 2 ];
    for ( UInt32 i=0; i<byteCount; ++i )
    {
        pair[ 0 ] = hexDigits[ bytes[ i ] >> 4 ];
        pair[ 1 ] = hexDigits[ bytes[ i ] & 0x0F ];
        output.Append( pair, 2, true );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
