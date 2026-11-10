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

#include "gucefCORE_CCodecInstanceBase.h"

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

CCodecInstanceBase::CCodecInstanceBase( void )
    : CICodecInstance()
    , m_lastStatus( GUCEF_CODEC_STATUS_OK )
{GUCEF_TRACE;

    m_lastErrorMessage[ 0 ] = '\0';
}

/*-------------------------------------------------------------------------*/

CCodecInstanceBase::~CCodecInstanceBase()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TCodecStatus
CCodecInstanceBase::GetLastStatus( void ) const
{GUCEF_TRACE;

    return m_lastStatus;
}

/*-------------------------------------------------------------------------*/

CString
CCodecInstanceBase::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return CString( m_lastErrorMessage );
}

/*-------------------------------------------------------------------------*/

bool
CCodecInstanceBase::SetStatus( TCodecStatus status, const char* message )
{GUCEF_TRACE;

    m_lastStatus = status;
    if ( GUCEF_NULL != message )
    {
        strncpy( m_lastErrorMessage, message, sizeof( m_lastErrorMessage ) - 1 );
        m_lastErrorMessage[ sizeof( m_lastErrorMessage ) - 1 ] = '\0';
    }
    else
    {
        m_lastErrorMessage[ 0 ] = '\0';
    }
    return GUCEF_CODEC_STATUS_OK == status;
}

/*-------------------------------------------------------------------------*/

bool
CCodecInstanceBase::SetOk( void )
{GUCEF_TRACE;

    m_lastStatus = GUCEF_CODEC_STATUS_OK;
    m_lastErrorMessage[ 0 ] = '\0';
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCodecInstanceBase::CheckDirection( UInt32 flags, bool canEncode, bool canDecode, bool& encode )
{GUCEF_TRACE;

    encode = 0 != ( flags & GUCEF_CODEC_PROCESS_ENCODE );
    if ( encode == ( 0 != ( flags & GUCEF_CODEC_PROCESS_DECODE ) ) )
        return SetStatus( GUCEF_CODEC_STATUS_INVALID_PARAMETER, "Exactly one of the encode and decode flags must be set" );
    if ( encode && !canEncode )
        return SetStatus( GUCEF_CODEC_STATUS_UNSUPPORTED, "The codec does not encode" );
    if ( !encode && !canDecode )
        return SetStatus( GUCEF_CODEC_STATUS_UNSUPPORTED, "The codec does not decode" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCodecInstanceBase::ReserveOutput( CDynamicBuffer& output, UInt32 size, UInt8*& out )
{GUCEF_TRACE;

    out = GUCEF_NULL;
    UInt32 dataSize = output.GetDataSize();
    if ( size > GUCEF_UINT32MAX - dataSize )
        return SetStatus( GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL, "The output would exceed 4 GB" );

    UInt32 needed = dataSize + size;
    if ( output.GetBufferSize() < needed )
    {
        // only grows, a buffer that is reused keeps its size and is not touched again
        output.SetBufferSize( needed, false );
        if ( output.GetBufferSize() < needed )
            return SetStatus( GUCEF_CODEC_STATUS_FAILED, "Unable to make room for the output" );
    }
    if ( output.GetBufferSize() > 0 )
        out = static_cast< UInt8* >( output.GetBufferPtr( dataSize ) );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
