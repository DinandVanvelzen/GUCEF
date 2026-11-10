/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#include "gucefDRN_CDRNStreamFramer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Parses a frame header
 *  @return 1 = complete header, 0 = need more bytes, -1 = error
 */
static Int32
ParseFrameHeader( const UInt8* data, UInt32 size, UInt32 maxPacketSize, UInt32& headerSize, UInt32& packetSize )
{GUCEF_TRACE;
    if ( size < 1 )
        return 0;
    if ( CDRNStreamFramer::FrameMagic0 != data[ 0 ] )
        return -1;
    if ( size < 2 )
        return 0;
    if ( CDRNStreamFramer::FrameMagic1 != data[ 1 ] )
        return -1;

    UInt32 value = 0;
    UInt32 shift = 0;
    UInt32 pos = 2;
    while ( true )
    {
        if ( pos >= size )
            return 0;
        UInt8 b = data[ pos++ ];
        value |= (UInt32) ( b & 0x7F ) << shift;
        if ( 0 == ( b & 0x80 ) )
            break;
        shift += 7;
        if ( shift > 28 )
            return -1;
    }
    if ( value > maxPacketSize )
        return -1;
    headerSize = pos;
    packetSize = value;
    return 1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNStreamFramer::CDRNStreamFramer( UInt32 maxPacketSize )
    : m_maxPacketSize( maxPacketSize )
    , m_pending()
    , m_src( GUCEF_NULL )
    , m_srcSize( 0 )
    , m_srcPos( 0 )
    , m_parsingPending( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CDRNStreamFramer::WriteFrameHeader( UInt32 packetSize, UInt8* dest )
{GUCEF_TRACE;
    dest[ 0 ] = FrameMagic0;
    dest[ 1 ] = FrameMagic1;
    return 2 + CDRNWireWriter::EncodeVarUInt64( packetSize, dest + 2 );
}

/*-------------------------------------------------------------------------*/

void
CDRNStreamFramer::BeginChunk( const void* data, UInt32 size )
{GUCEF_TRACE;
    if ( m_pending.GetDataSize() > 0 )
    {
        // finish the incomplete frame from before: work on the internal buffer
        m_pending.Append( data, size, true );
        m_src = static_cast< const UInt8* >( m_pending.GetConstBufferPtr() );
        m_srcSize = m_pending.GetDataSize();
        m_parsingPending = true;
    }
    else
    {
        m_src = static_cast< const UInt8* >( data );
        m_srcSize = size;
        m_parsingPending = false;
    }
    m_srcPos = 0;
}

/*-------------------------------------------------------------------------*/

CDRNStreamFramer::TFrameResult
CDRNStreamFramer::NextPacket( const UInt8*& packet, UInt32& packetSize )
{GUCEF_TRACE;
    if ( GUCEF_NULL == m_src || m_srcPos >= m_srcSize )
        return FRAME_NEED_MORE_DATA;

    UInt32 headerSize = 0;
    UInt32 size = 0;
    Int32 result = ParseFrameHeader( m_src + m_srcPos, m_srcSize - m_srcPos, m_maxPacketSize, headerSize, size );
    if ( result < 0 )
        return FRAME_ERROR;
    if ( 0 == result || m_srcSize - m_srcPos - headerSize < size )
        return FRAME_NEED_MORE_DATA;

    packet = m_src + m_srcPos + headerSize;
    packetSize = size;
    m_srcPos += headerSize + size;
    return FRAME_PACKET;
}

/*-------------------------------------------------------------------------*/

void
CDRNStreamFramer::EndChunk( void )
{GUCEF_TRACE;
    UInt32 remaining = m_srcSize - m_srcPos;
    if ( m_parsingPending )
    {
        if ( 0 == remaining )
            m_pending.SetDataSize( 0 );
        else
        if ( m_srcPos > 0 )
        {
            UInt8* buffer = static_cast< UInt8* >( m_pending.GetBufferPtr() );
            memmove( buffer, buffer + m_srcPos, remaining );
            m_pending.SetDataSize( remaining );
        }
    }
    else
    if ( remaining > 0 )
    {
        m_pending.SetDataSize( 0 );
        m_pending.Append( m_src + m_srcPos, remaining, true );
    }

    // once the size of the incomplete packet is known make room for all of it at once
    if ( m_pending.GetDataSize() > 0 )
    {
        UInt32 headerSize = 0;
        UInt32 packetSize = 0;
        if ( 1 == ParseFrameHeader( static_cast< const UInt8* >( m_pending.GetConstBufferPtr() ), m_pending.GetDataSize(), m_maxPacketSize, headerSize, packetSize ) )
        {
            if ( m_pending.GetBufferSize() < headerSize + packetSize )
                m_pending.SetBufferSize( headerSize + packetSize, false );
        }
    }

    m_src = GUCEF_NULL;
    m_srcSize = 0;
    m_srcPos = 0;
    m_parsingPending = false;
}

/*-------------------------------------------------------------------------*/

void
CDRNStreamFramer::Reset( void )
{GUCEF_TRACE;
    m_pending.SetDataSize( 0 );
    m_src = GUCEF_NULL;
    m_srcSize = 0;
    m_srcPos = 0;
    m_parsingPending = false;
}

/*-------------------------------------------------------------------------*/

void
CDRNStreamFramer::SetMaxPacketSize( UInt32 maxPacketSize )
{GUCEF_TRACE;
    m_maxPacketSize = maxPacketSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNStreamFramer::GetMaxPacketSize( void ) const
{GUCEF_TRACE;
    return m_maxPacketSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNStreamFramer::GetPendingByteCount( void ) const
{GUCEF_TRACE;
    return m_pending.GetDataSize();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
