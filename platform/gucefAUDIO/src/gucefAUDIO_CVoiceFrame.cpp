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

#include <string.h>

#include "gucefAUDIO_CVoiceFrame.h"

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

CVoiceFrame::CVoiceFrame( void )
    : m_streamId( 0 )
    , m_timestamp( 0 )
    , m_captureTickCount( 0 )
    , m_payloadSize( 0 )
    , m_redundantSize( 0 )
    , m_sequenceNumber( 0 )
    , m_redundantTimestampOffset( 0 )
    , m_payloadType( 0 )
    , m_redundantPayloadType( 0 )
    , m_marker( false )
{

}

/*-------------------------------------------------------------------------*/

CVoiceFrame::CVoiceFrame( const CVoiceFrame& src )
    : m_streamId( 0 )
    , m_timestamp( 0 )
    , m_captureTickCount( 0 )
    , m_payloadSize( 0 )
    , m_redundantSize( 0 )
    , m_sequenceNumber( 0 )
    , m_redundantTimestampOffset( 0 )
    , m_payloadType( 0 )
    , m_redundantPayloadType( 0 )
    , m_marker( false )
{
    *this = src;
}

/*-------------------------------------------------------------------------*/

CVoiceFrame&
CVoiceFrame::operator=( const CVoiceFrame& src )
{
    if ( this != &src )
    {
        m_streamId = src.m_streamId;
        m_timestamp = src.m_timestamp;
        m_captureTickCount = src.m_captureTickCount;
        m_payloadSize = src.m_payloadSize;
        m_redundantSize = src.m_redundantSize;
        m_sequenceNumber = src.m_sequenceNumber;
        m_redundantTimestampOffset = src.m_redundantTimestampOffset;
        m_payloadType = src.m_payloadType;
        m_redundantPayloadType = src.m_redundantPayloadType;
        m_marker = src.m_marker;
        if ( 0 != m_payloadSize )
            memcpy( m_payload, src.m_payload, m_payloadSize );
        if ( 0 != m_redundantSize )
            memcpy( m_redundantPayload, src.m_redundantPayload, m_redundantSize );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVoiceFrame::~CVoiceFrame()
{

}

/*-------------------------------------------------------------------------*/

UInt32 CVoiceFrame::GetStreamId( void ) const { return m_streamId; }
void CVoiceFrame::SetStreamId( UInt32 streamId ) { m_streamId = streamId; }
UInt16 CVoiceFrame::GetSequenceNumber( void ) const { return m_sequenceNumber; }
void CVoiceFrame::SetSequenceNumber( UInt16 sequenceNumber ) { m_sequenceNumber = sequenceNumber; }
UInt32 CVoiceFrame::GetTimestamp( void ) const { return m_timestamp; }
void CVoiceFrame::SetTimestamp( UInt32 timestamp ) { m_timestamp = timestamp; }
bool CVoiceFrame::GetMarker( void ) const { return m_marker; }
void CVoiceFrame::SetMarker( bool marker ) { m_marker = marker; }
UInt8 CVoiceFrame::GetPayloadType( void ) const { return m_payloadType; }
void CVoiceFrame::SetPayloadType( UInt8 payloadType ) { m_payloadType = payloadType; }
const UInt8* CVoiceFrame::GetPayload( void ) const { return m_payload; }
UInt32 CVoiceFrame::GetPayloadSize( void ) const { return m_payloadSize; }
UInt8* CVoiceFrame::GetPayloadBuffer( void ) { return m_payload; }
bool CVoiceFrame::HasRedundantBlock( void ) const { return 0 != m_redundantSize; }
UInt16 CVoiceFrame::GetRedundantTimestampOffset( void ) const { return m_redundantTimestampOffset; }
UInt8 CVoiceFrame::GetRedundantPayloadType( void ) const { return m_redundantPayloadType; }
const UInt8* CVoiceFrame::GetRedundantPayload( void ) const { return m_redundantPayload; }
UInt32 CVoiceFrame::GetRedundantSize( void ) const { return m_redundantSize; }
UInt64 CVoiceFrame::GetCaptureTickCount( void ) const { return m_captureTickCount; }
void CVoiceFrame::SetCaptureTickCount( UInt64 ticks ) { m_captureTickCount = ticks; }

/*-------------------------------------------------------------------------*/

bool
CVoiceFrame::SetPayload( const UInt8* data, UInt32 size )
{
    if ( size > MaxPayloadSize )
        return false;
    if ( 0 != size )
        memcpy( m_payload, data, size );
    m_payloadSize = size;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceFrame::SetPayloadSize( UInt32 size )
{
    if ( size > MaxPayloadSize )
        return false;
    m_payloadSize = size;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVoiceFrame::SetRedundantBlock( UInt16 timestampOffset, UInt8 payloadType, const UInt8* data, UInt32 size )
{
    if ( 0 == size || size > MaxRedundantSize || timestampOffset > MaxRedundantTimestampOffset || payloadType > 127 )
        return false;
    memcpy( m_redundantPayload, data, size );
    m_redundantSize = size;
    m_redundantTimestampOffset = timestampOffset;
    m_redundantPayloadType = payloadType;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVoiceFrame::ClearRedundantBlock( void )
{
    m_redundantSize = 0;
    m_redundantTimestampOffset = 0;
    m_redundantPayloadType = 0;
}

/*-------------------------------------------------------------------------*/

void
CVoiceFrame::Clear( void )
{
    m_streamId = 0;
    m_timestamp = 0;
    m_captureTickCount = 0;
    m_payloadSize = 0;
    m_sequenceNumber = 0;
    m_payloadType = 0;
    m_marker = false;
    ClearRedundantBlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
