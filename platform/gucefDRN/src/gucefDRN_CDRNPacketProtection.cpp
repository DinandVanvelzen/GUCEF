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

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#include "gucefDRN_CDRNPacketProtection.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CDRNPacketProtection::TagSize;
const UInt32 CDRNPacketProtection::KeySize;

static const char RekeyInfo[] = "DRN 2 rekey";
static const UInt64 DefaultRekeyPackets = (UInt64) 1 << 31;
static const UInt64 DefaultRekeyBytes = (UInt64) 64 * 1024 * 1024 * 1024;
static const UInt32 MaxNonceSize = 24;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNPacketProtection::CKeyState::CKeyState( void )
    : instance()
    , phase( 0 )
    , valid( false )
{GUCEF_TRACE;

    memset( key, 0, sizeof( key ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNPacketProtection::CKeyState::Clear( void )
{GUCEF_TRACE;

    memset( key, 0, sizeof( key ) );
    instance.Unlink();
    phase = 0;
    valid = false;
}

/*-------------------------------------------------------------------------*/

CDRNPacketProtection::CDRNPacketProtection( void )
    : m_cipher()
    , m_nonceSize( 12 )
    , m_sendDirection( 0 )
    , m_sendEnabled( false )
    , m_send()
    , m_receive()
    , m_previousReceive()
    , m_nextReceive()
    , m_sendPackets( 0 )
    , m_sendBytes( 0 )
    , m_rekeyPackets( DefaultRekeyPackets )
    , m_rekeyBytes( DefaultRekeyBytes )
    , m_sendLastSequence( 0 )
    , m_sendWraps( 0 )
    , m_sendAny( false )
    , m_receiveHighest( 0 )
    , m_receiveAny( false )
    , m_failed( 0 )
    , m_sendRekeys( 0 )
    , m_receiveRekeys( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNPacketProtection::~CDRNPacketProtection()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNPacketProtection::Clear( void )
{GUCEF_TRACE;

    m_send.Clear();
    m_receive.Clear();
    m_previousReceive.Clear();
    m_nextReceive.Clear();
    m_cipher.Clear();
    m_sendEnabled = false;
    m_sendPackets = 0;
    m_sendBytes = 0;
    m_sendLastSequence = 0;
    m_sendWraps = 0;
    m_sendAny = false;
    m_receiveHighest = 0;
    m_receiveAny = false;
    m_sendRekeys = 0;
    m_receiveRekeys = 0;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketProtection::MakeInstance( CKeyState& state )
{GUCEF_TRACE;

    CORE::CCodecRegistry::TICodecPtr codec = CDRNCrypto::GetCodec( CORE::CoreCodecTypes::EncryptionCodec, m_cipher );
    if ( codec.IsNULL() )
        return false;
    CORE::CValueList params;
    params.Set( "key", CORE::CVariant( state.key, KeySize ) );
    state.instance.Unlink();
    if ( !codec->CreateInstance( params, state.instance ) || state.instance.IsNULL() )
        return false;
    if ( TagSize != state.instance->GetOverhead() && 0 != state.instance->GetOverhead() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNPacketProtection: Cipher \"" << m_cipher << "\" has a tag of " << state.instance->GetOverhead() << " bytes, DRN needs " << TagSize );
        state.instance.Unlink();
        return false;
    }
    state.valid = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketProtection::DeriveNext( const CKeyState& from, CKeyState& next )
{GUCEF_TRACE;

    next.Clear();
    if ( !CDRNCrypto::Hkdf( GUCEF_NULL, 0, from.key, KeySize, RekeyInfo, (UInt32) sizeof( RekeyInfo ) - 1, next.key, KeySize ) )
        return false;
    next.phase = (UInt8) ( from.phase ^ 1 );
    return MakeInstance( next );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketProtection::Setup( const CORE::CString& cipher, const UInt8* sendKey, const UInt8* receiveKey, UInt8 sendDirection )
{GUCEF_TRACE;

    Clear();
    m_cipher = cipher.Lowercase();
    m_nonceSize = m_cipher == "xchacha20-poly1305" ? 24 : 12;
    m_sendDirection = sendDirection;
    memcpy( m_send.key, sendKey, KeySize );
    memcpy( m_receive.key, receiveKey, KeySize );
    if ( !MakeInstance( m_send ) || !MakeInstance( m_receive ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNPacketProtection: No instance of cipher \"" << m_cipher << "\"" );
        Clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketProtection::IsReady( void ) const
{GUCEF_TRACE;

    return m_receive.valid && m_send.valid;
}

/*-------------------------------------------------------------------------*/

void
CDRNPacketProtection::SetSendEnabled( bool enabled )
{GUCEF_TRACE;

    m_sendEnabled = enabled && m_send.valid;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketProtection::IsSendEnabled( void ) const
{GUCEF_TRACE;

    return m_sendEnabled;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNPacketProtection::GetCipher( void ) const
{GUCEF_TRACE;

    return m_cipher;
}

/*-------------------------------------------------------------------------*/

void
CDRNPacketProtection::SetRekeyLimits( UInt64 packets, UInt64 bytes )
{GUCEF_TRACE;

    m_rekeyPackets = 0 != packets ? packets : DefaultRekeyPackets;
    m_rekeyBytes = 0 != bytes ? bytes : DefaultRekeyBytes;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketProtection::IsProtectedPacket( const UInt8* packet, UInt32 packetSize )
{GUCEF_TRACE;

    return GUCEF_NULL != packet && packetSize > 0 &&
           ( DRN_PROTOCOL_ID_PROTECTED_0 == packet[ 0 ] || DRN_PROTOCOL_ID_PROTECTED_1 == packet[ 0 ] );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPacketProtection::MeasureHeader( const UInt8* packet, UInt32 packetSize, UInt32& sequence )
{GUCEF_TRACE;

    CDRNWireReader reader( packet, packetSize );
    UInt8 protocol = 0;
    UInt64 value = 0;
    UInt32 ackSequence = 0;
    UInt32 ackBits = 0;
    if ( !reader.ReadUInt8( protocol ) || !reader.ReadVarUInt32( sequence ) || !reader.ReadVarUInt64( value ) ||
         !reader.ReadVarUInt32( ackSequence ) || !reader.ReadUInt32( ackBits ) || !reader.ReadVarUInt64( value ) ||
         !reader.ReadVarUInt64( value ) )
        return 0;
    return reader.GetPosition();
}

/*-------------------------------------------------------------------------*/

void
CDRNPacketProtection::MakeNonce( UInt8 direction, UInt64 fullSequence, UInt8* nonce ) const
{GUCEF_TRACE;

    memset( nonce, 0, m_nonceSize );
    nonce[ 0 ] = direction;
    for ( UInt32 i=0; i<8; ++i )
        nonce[ m_nonceSize - 1 - i ] = (UInt8) ( fullSequence >> ( 8 * i ) );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketProtection::Protect( const UInt8* packet, UInt32 packetSize, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    if ( !m_send.valid || m_send.instance.IsNULL() )
        return false;

    UInt32 sequence = 0;
    UInt32 headerSize = MeasureHeader( packet, packetSize, sequence );
    if ( 0 == headerSize || headerSize > DRN_MAX_PACKET_HEADER_SIZE )
        return false;

    // the sequence with its wraps: never twice the same under one key
    if ( m_sendAny && sequence < m_sendLastSequence )
        ++m_sendWraps;
    m_sendAny = true;
    m_sendLastSequence = sequence;
    UInt64 fullSequence = ( m_sendWraps << 32 ) | (UInt64) sequence;

    UInt8 header[ DRN_MAX_PACKET_HEADER_SIZE ];
    memcpy( header, packet, headerSize );
    header[ 0 ] = 0 == m_send.phase ? DRN_PROTOCOL_ID_PROTECTED_0 : DRN_PROTOCOL_ID_PROTECTED_1;

    UInt8 nonce[ MaxNonceSize ];
    MakeNonce( m_sendDirection, fullSequence, nonce );
    CORE::SCodecCallArgs args;
    memset( &args, 0, sizeof( args ) );
    args.structSize = sizeof( args );
    args.nonce = nonce;
    args.nonceSize = m_nonceSize;
    args.aad = header;
    args.aadSize = headerSize;

    output.SetDataSize( 0 );
    output.CopyFrom( 0, headerSize, header );
    if ( !m_send.instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, packet + headerSize, packetSize - headerSize, &args, output ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNPacketProtection: Encrypting a packet failed: " << m_send.instance->GetLastErrorMessage() );
        return false;
    }

    ++m_sendPackets;
    m_sendBytes += packetSize;
    if ( m_sendPackets >= m_rekeyPackets || m_sendBytes >= m_rekeyBytes )
    {
        CKeyState next;
        if ( !DeriveNext( m_send, next ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNPacketProtection: The next key could not be made, nothing more is sent" );
            m_send.Clear();
            m_sendEnabled = false;
            return true;
        }
        memcpy( m_send.key, next.key, KeySize );
        m_send.instance = next.instance;
        m_send.phase = next.phase;
        next.Clear();
        m_sendPackets = 0;
        m_sendBytes = 0;
        ++m_sendRekeys;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketProtection::Open( CKeyState& state                   ,
                            const UInt8* header                ,
                            UInt32 headerSize                  ,
                            const UInt8* body                  ,
                            UInt32 bodySize                    ,
                            UInt64 fullSequence                ,
                            CORE::CDynamicBuffer& output       )
{GUCEF_TRACE;

    UInt8 nonce[ MaxNonceSize ];
    MakeNonce( (UInt8) ( m_sendDirection ^ 1 ), fullSequence, nonce );
    CORE::SCodecCallArgs args;
    memset( &args, 0, sizeof( args ) );
    args.structSize = sizeof( args );
    args.nonce = nonce;
    args.nonceSize = m_nonceSize;
    args.aad = header;
    args.aadSize = headerSize;

    output.SetDataSize( 0 );
    output.CopyFrom( 0, headerSize, header );
    static_cast< UInt8* >( output.GetBufferPtr() )[ 0 ] = DRN_PROTOCOL_ID;
    return state.instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, body, bodySize, &args, output );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketProtection::Unprotect( const UInt8* packet, UInt32 packetSize, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    if ( !m_receive.valid || !IsProtectedPacket( packet, packetSize ) )
        return false;

    UInt32 sequence = 0;
    UInt32 headerSize = MeasureHeader( packet, packetSize, sequence );
    if ( 0 == headerSize || headerSize > DRN_MAX_PACKET_HEADER_SIZE || packetSize < headerSize + TagSize )
    {
        ++m_failed;
        return false;
    }

    // the sequence with its wraps, closest to the highest one seen
    UInt64 fullSequence = sequence;
    if ( m_receiveAny )
    {
        fullSequence = ( m_receiveHighest & ~(UInt64) 0xFFFFFFFF ) | (UInt64) sequence;
        if ( fullSequence + 0x80000000 < m_receiveHighest )
            fullSequence += (UInt64) 1 << 32;
        else
        if ( fullSequence > m_receiveHighest + 0x80000000 && fullSequence >= ( (UInt64) 1 << 32 ) )
            fullSequence -= (UInt64) 1 << 32;
    }

    UInt8 phase = DRN_PROTOCOL_ID_PROTECTED_0 == packet[ 0 ] ? 0 : 1;
    const UInt8* body = packet + headerSize;
    UInt32 bodySize = packetSize - headerSize;
    bool opened = false;
    if ( phase == m_receive.phase )
    {
        opened = Open( m_receive, packet, headerSize, body, bodySize, fullSequence, output );
    }
    else
    if ( m_previousReceive.valid && phase == m_previousReceive.phase && Open( m_previousReceive, packet, headerSize, body, bodySize, fullSequence, output ) )
    {
        opened = true;
    }
    else
    {
        // the sender may have gone to the next key
        if ( !m_nextReceive.valid && !DeriveNext( m_receive, m_nextReceive ) )
        {
            ++m_failed;
            return false;
        }
        if ( Open( m_nextReceive, packet, headerSize, body, bodySize, fullSequence, output ) )
        {
            m_previousReceive.Clear();
            memcpy( m_previousReceive.key, m_receive.key, KeySize );
            m_previousReceive.instance = m_receive.instance;
            m_previousReceive.phase = m_receive.phase;
            m_previousReceive.valid = true;
            memcpy( m_receive.key, m_nextReceive.key, KeySize );
            m_receive.instance = m_nextReceive.instance;
            m_receive.phase = m_nextReceive.phase;
            m_nextReceive.Clear();
            ++m_receiveRekeys;
            opened = true;
        }
    }

    if ( !opened )
    {
        ++m_failed;
        return false;
    }
    if ( !m_receiveAny || fullSequence > m_receiveHighest )
        m_receiveHighest = fullSequence;
    m_receiveAny = true;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNPacketProtection::GetFailedCount( void ) const
{GUCEF_TRACE;

    return m_failed;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPacketProtection::GetSendRekeyCount( void ) const
{GUCEF_TRACE;

    return m_sendRekeys;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPacketProtection::GetReceiveRekeyCount( void ) const
{GUCEF_TRACE;

    return m_receiveRekeys;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
