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

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#include "gucefCORE_CDefaultCodecInstance.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDefaultCodecInstance::CDefaultCodecInstance( const TICodecPtr& codec, const CValueList& params )
    : CICodecInstance()
    , m_codec( codec )
    , m_params( params )
    , m_input()
    , m_lastStatus( GUCEF_CODEC_STATUS_OK )
    , m_lastErrorMessage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDefaultCodecInstance::~CDefaultCodecInstance()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDefaultCodecInstance::SetStatus( TCodecStatus status, const CString& message )
{GUCEF_TRACE;

    m_lastStatus = status;
    m_lastErrorMessage = message;
}

/*-------------------------------------------------------------------------*/

bool
CDefaultCodecInstance::Process( UInt32 flags                ,
                                const void* input           ,
                                UInt32 inputSize            ,
                                const SCodecCallArgs* args  ,
                                CDynamicBuffer& output      )
{GUCEF_TRACE;

    const bool encode = 0 != ( flags & GUCEF_CODEC_PROCESS_ENCODE );
    if ( encode == ( 0 != ( flags & GUCEF_CODEC_PROCESS_DECODE ) ) )
    {
        SetStatus( GUCEF_CODEC_STATUS_INVALID_PARAMETER, "Exactly one of the encode and decode flags must be set" );
        return false;
    }
    if ( m_codec.IsNULL() )
    {
        SetStatus( GUCEF_CODEC_STATUS_FAILED, "The instance has no codec" );
        return false;
    }

    if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) )
        m_input.Clear( true );
    if ( GUCEF_NULL != input && inputSize > 0 )
        m_input.Append( input, inputSize, true );

    if ( 0 == ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
    {
        SetStatus( GUCEF_CODEC_STATUS_OK, CString::Empty );
        return true;
    }

    // The message is complete: run it through the codec with the parameters of this call
    CValueList callParams( m_params );
    if ( GUCEF_NULL != args && args->structSize >= sizeof( SCodecCallArgs ) )
    {
        // a value list keeps several values per key, the call arguments replace what the parameters had
        if ( GUCEF_NULL != args->nonce )
        {
            callParams.Delete( "nonce" );
            callParams.Set( "nonce", CVariant( args->nonce, args->nonceSize ) );
        }
        if ( GUCEF_NULL != args->aad )
        {
            callParams.Delete( "aad" );
            callParams.Set( "aad", CVariant( args->aad, args->aadSize ) );
        }
    }

    CDynamicBufferAccess inputAccess( m_input );
    CDynamicBufferAccess outputAccess( output );
    outputAccess.Setpos( output.GetDataSize() );

    bool success = encode ? m_codec->Encode( inputAccess, outputAccess, callParams ) : m_codec->Decode( inputAccess, outputAccess, callParams );
    TCodecStatus status = m_codec->GetLastStatus();
    if ( !success && GUCEF_CODEC_STATUS_OK == status )
        status = GUCEF_CODEC_STATUS_FAILED;
    if ( success )
        status = GUCEF_CODEC_STATUS_OK;
    SetStatus( status, success ? CString::Empty : m_codec->GetLastErrorMessage() );

    m_input.Clear( true );
    return success;
}

/*-------------------------------------------------------------------------*/

UInt32
CDefaultCodecInstance::GetOverhead( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

TCodecStatus
CDefaultCodecInstance::GetLastStatus( void ) const
{GUCEF_TRACE;

    return m_lastStatus;
}

/*-------------------------------------------------------------------------*/

CString
CDefaultCodecInstance::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return m_lastErrorMessage;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
