/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#include "gucefAUDIO_CRtpHeader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRtpHeader::CRtpHeader( void )
    : m_timestamp( 0 )
    , m_ssrc( 0 )
    , m_payloadOffset( FixedHeaderSize )
    , m_payloadSize( 0 )
    , m_paddingSize( 0 )
    , m_sequenceNumber( 0 )
    , m_payloadType( 0 )
    , m_csrcCount( 0 )
    , m_marker( false )
    , m_hasPadding( false )
    , m_hasExtension( false )
{

}

/*-------------------------------------------------------------------------*/

bool CRtpHeader::GetMarker( void ) const { return m_marker; }
void CRtpHeader::SetMarker( bool marker ) { m_marker = marker; }
UInt8 CRtpHeader::GetPayloadType( void ) const { return m_payloadType; }
void CRtpHeader::SetPayloadType( UInt8 payloadType ) { m_payloadType = payloadType; }
UInt16 CRtpHeader::GetSequenceNumber( void ) const { return m_sequenceNumber; }
void CRtpHeader::SetSequenceNumber( UInt16 sequenceNumber ) { m_sequenceNumber = sequenceNumber; }
UInt32 CRtpHeader::GetTimestamp( void ) const { return m_timestamp; }
void CRtpHeader::SetTimestamp( UInt32 timestamp ) { m_timestamp = timestamp; }
UInt32 CRtpHeader::GetSsrc( void ) const { return m_ssrc; }
void CRtpHeader::SetSsrc( UInt32 ssrc ) { m_ssrc = ssrc; }
bool CRtpHeader::HasPadding( void ) const { return m_hasPadding; }
bool CRtpHeader::HasExtension( void ) const { return m_hasExtension; }
UInt32 CRtpHeader::GetCsrcCount( void ) const { return m_csrcCount; }
UInt32 CRtpHeader::GetPayloadOffset( void ) const { return m_payloadOffset; }
UInt32 CRtpHeader::GetPayloadSize( void ) const { return m_payloadSize; }
UInt32 CRtpHeader::GetPaddingSize( void ) const { return m_paddingSize; }

/*-------------------------------------------------------------------------*/

Int32
CRtpHeader::Write( UInt8* buffer, UInt32 capacity ) const
{
    if ( capacity < FixedHeaderSize || m_payloadType > 127 )
        return -1;

    buffer[ 0 ] = (UInt8) ( RtpVersion << 6 );                                      // V=2, P=0, X=0, CC=0
    buffer[ 1 ] = (UInt8) ( ( m_marker ? 0x80 : 0x00 ) | m_payloadType );
    buffer[ 2 ] = (UInt8) ( m_sequenceNumber >> 8 );
    buffer[ 3 ] = (UInt8) ( m_sequenceNumber & 0xFF );
    buffer[ 4 ] = (UInt8) ( m_timestamp >> 24 );
    buffer[ 5 ] = (UInt8) ( ( m_timestamp >> 16 ) & 0xFF );
    buffer[ 6 ] = (UInt8) ( ( m_timestamp >> 8 ) & 0xFF );
    buffer[ 7 ] = (UInt8) ( m_timestamp & 0xFF );
    buffer[ 8 ] = (UInt8) ( m_ssrc >> 24 );
    buffer[ 9 ] = (UInt8) ( ( m_ssrc >> 16 ) & 0xFF );
    buffer[ 10 ] = (UInt8) ( ( m_ssrc >> 8 ) & 0xFF );
    buffer[ 11 ] = (UInt8) ( m_ssrc & 0xFF );
    return (Int32) FixedHeaderSize;
}

/*-------------------------------------------------------------------------*/

bool
CRtpHeader::Read( const UInt8* packet, UInt32 size )
{
    if ( size < FixedHeaderSize )
        return false;
    if ( ( packet[ 0 ] >> 6 ) != RtpVersion )
        return false;

    const bool padding = 0 != ( packet[ 0 ] & 0x20 );
    const bool extension = 0 != ( packet[ 0 ] & 0x10 );
    const UInt32 csrcCount = packet[ 0 ] & 0x0F;

    UInt32 offset = FixedHeaderSize + 4 * csrcCount;
    if ( offset > size )
        return false;

    if ( extension )
    {
        // profile specific identifier (16 bits) and the length in 32 bit words, not counting these 4 bytes
        if ( offset + 4 > size )
            return false;
        const UInt32 words = ( (UInt32) packet[ offset + 2 ] << 8 ) | (UInt32) packet[ offset + 3 ];
        offset += 4 + 4 * words;
        if ( offset > size )
            return false;
    }

    UInt32 paddingSize = 0;
    if ( padding )
    {
        // the last byte counts the bytes of padding, itself included
        paddingSize = packet[ size - 1 ];
        if ( 0 == paddingSize || paddingSize > size - offset )
            return false;
    }

    m_marker = 0 != ( packet[ 1 ] & 0x80 );
    m_payloadType = packet[ 1 ] & 0x7F;
    m_sequenceNumber = (UInt16) ( ( (UInt32) packet[ 2 ] << 8 ) | (UInt32) packet[ 3 ] );
    m_timestamp = ( (UInt32) packet[ 4 ] << 24 ) | ( (UInt32) packet[ 5 ] << 16 ) | ( (UInt32) packet[ 6 ] << 8 ) | (UInt32) packet[ 7 ];
    m_ssrc = ( (UInt32) packet[ 8 ] << 24 ) | ( (UInt32) packet[ 9 ] << 16 ) | ( (UInt32) packet[ 10 ] << 8 ) | (UInt32) packet[ 11 ];
    m_hasPadding = padding;
    m_hasExtension = extension;
    m_csrcCount = (UInt8) csrcCount;
    m_payloadOffset = offset;
    m_paddingSize = paddingSize;
    m_payloadSize = size - offset - paddingSize;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
