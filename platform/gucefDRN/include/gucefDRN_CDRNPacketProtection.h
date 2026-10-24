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

#ifndef GUCEF_DRN_CDRNPACKETPROTECTION_H
#define GUCEF_DRN_CDRNPACKETPROTECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Encrypts and authenticates the packets of a link with an AEAD cipher of the codec registry
 *  (EncryptionCodec: "xchacha20-poly1305", "chacha20-poly1305", "aes-256-gcm"), one key per direction.
 *
 *  A protected packet is the packet header in the clear, its first byte DRN_PROTOCOL_ID_PROTECTED_0 or _1 (the key
 *  phase), followed by the encrypted body and the tag. The header is the associated data, so it cannot be changed.
 *  The nonce is the direction byte (0 initiator to acceptor, 1 acceptor to initiator), zeros, and the 64 bit
 *  sequence of the packet: the 32 bit header sequence extended with its wraps, so a nonce never repeats under one key.
 *  After a rekey limit (2^31 packets or 64 GB by default) the sender derives the next key (HKDF-SHA256 of the key
 *  with "DRN 2 rekey") and flips the phase; the receiver follows when a packet of the other phase opens with it.
 *  Packets of the previous phase still open, for packets that arrive late.
 *  Protect() and Unprotect() allocate nothing once their output buffer has grown.
 */
class GUCEF_DRN_EXPORT_CPP CDRNPacketProtection
{
    public:

    static const UInt32 TagSize = DRN_PACKET_PROTECTION_OVERHEAD;
    static const UInt32 KeySize = 32;

    CDRNPacketProtection( void );

    ~CDRNPacketProtection();

    /**
     *  Prepares both directions
     *  @param sendDirection 0 for the initiator of the link, 1 for the acceptor
     */
    bool Setup( const CORE::CString& cipher, const UInt8* sendKey, const UInt8* receiveKey, UInt8 sendDirection );

    /** Forgets the keys */
    void Clear( void );

    /** Keys are there: protected packets can be opened */
    bool IsReady( void ) const;

    /** Packets that go out are protected */
    void SetSendEnabled( bool enabled );
    bool IsSendEnabled( void ) const;

    const CORE::CString& GetCipher( void ) const;

    /** For tests: rekey after fewer packets or bytes */
    void SetRekeyLimits( UInt64 packets, UInt64 bytes );

    /** Protects a whole plain packet (header and body) into output */
    bool Protect( const UInt8* packet, UInt32 packetSize, CORE::CDynamicBuffer& output );

    /** Opens a protected packet into output, a plain packet again; false if it does not authenticate */
    bool Unprotect( const UInt8* packet, UInt32 packetSize, CORE::CDynamicBuffer& output );

    static bool IsProtectedPacket( const UInt8* packet, UInt32 packetSize );

    /** The size of the header of a packet, plain or protected, and its sequence; 0 if it is malformed */
    static UInt32 MeasureHeader( const UInt8* packet, UInt32 packetSize, UInt32& sequence );

    UInt64 GetFailedCount( void ) const;
    UInt32 GetSendRekeyCount( void ) const;
    UInt32 GetReceiveRekeyCount( void ) const;

    private:

    class CKeyState
    {
        public:
        UInt8 key[ KeySize ];
        CORE::CICodecInstancePtr instance;
        UInt8 phase;
        bool valid;
        CKeyState( void );
        void Clear( void );
    };

    CDRNPacketProtection( const CDRNPacketProtection& src );
    CDRNPacketProtection& operator=( const CDRNPacketProtection& src );

    bool MakeInstance( CKeyState& state );
    bool DeriveNext( const CKeyState& from, CKeyState& next );
    void MakeNonce( UInt8 direction, UInt64 fullSequence, UInt8* nonce ) const;
    bool Open( CKeyState& state, const UInt8* header, UInt32 headerSize, const UInt8* body, UInt32 bodySize, UInt64 fullSequence, CORE::CDynamicBuffer& output );

    CORE::CString m_cipher;
    UInt32 m_nonceSize;
    UInt8 m_sendDirection;
    bool m_sendEnabled;
    CKeyState m_send;
    CKeyState m_receive;
    CKeyState m_previousReceive;
    CKeyState m_nextReceive;
    UInt64 m_sendPackets;
    UInt64 m_sendBytes;
    UInt64 m_rekeyPackets;
    UInt64 m_rekeyBytes;
    UInt32 m_sendLastSequence;
    UInt64 m_sendWraps;
    bool m_sendAny;
    UInt64 m_receiveHighest;
    bool m_receiveAny;
    UInt64 m_failed;
    UInt32 m_sendRekeys;
    UInt32 m_receiveRekeys;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNPACKETPROTECTION_H ? */
