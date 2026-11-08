/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCFRAMECODEC_H
#define GUCEF_COMCORE_CRPCFRAMECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CBINARYWIREWRITER_H
#include "gucefCORE_CBinaryWireWriter.h"
#define GUCEF_CORE_CBINARYWIREWRITER_H
#endif /* GUCEF_CORE_CBINARYWIREWRITER_H ? */

#ifndef GUCEF_CORE_CBINARYWIREREADER_H
#include "gucefCORE_CBinaryWireReader.h"
#define GUCEF_CORE_CBINARYWIREREADER_H
#endif /* GUCEF_CORE_CBINARYWIREREADER_H ? */

#ifndef GUCEF_COMCORE_RPCPROTOCOL_H
#include "gucefCOMCORE_RpcProtocol.h"
#define GUCEF_COMCORE_RPCPROTOCOL_H
#endif /* GUCEF_COMCORE_RPCPROTOCOL_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#include "gucefCOMCORE_CRpcRequestId.h"
#define GUCEF_COMCORE_CRPCREQUESTID_H
#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */

#ifndef GUCEF_COMCORE_CRPCMETADATA_H
#include "gucefCOMCORE_CRpcMetadata.h"
#define GUCEF_COMCORE_CRPCMETADATA_H
#endif /* GUCEF_COMCORE_CRPCMETADATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Per direction and session: the metadata keys interned so far (gucefCOMCORE RPC design 6.9). Both sides assign the
 *  same indices because frames are processed in seq order on both sides.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcMetadataInternTable
{
    public:

    static const UInt32 MaxInternedKeys = 256;

    CRpcMetadataInternTable( void );

    /** The interned index of a key, 0 when it is not interned */
    UInt32 Find( const CORE::CString& key ) const;

    /** Interns a key; its index (64, 65, ...), 0 when the table is full */
    UInt32 Add( const CORE::CString& key );

    /** The key of an interned index; false when unknown */
    bool Get( UInt32 index, CORE::CString& key ) const;

    UInt32 GetCount( void ) const;
    void Clear( void );

    /** Takes back the keys interned after the first count (a frame that defined them was not sent) */
    void Truncate( UInt32 count );

    private:

    GUCEF::vector< CORE::CString > m_keys;
    GUCEF::map< CORE::CString, UInt32 > m_index;
};

/*-------------------------------------------------------------------------*/

/** HELLO (6.4.1) */
struct GUCEF_COMCORE_EXPORT_CPP SRpcHello
{
    UInt8 protocolMajor;
    UInt8 protocolMinor;
    UInt64 features;
    CORE::CString nodeName;
    UInt8 mode;
    UInt8 sessionId[ GUCEF_RPC_SESSIONID_SIZE ];
    UInt64 resumeCount;
    UInt64 lastReceivedSeq;
    UInt64 oldestRetainedSeq;
    UInt64 nextSeq;
    UInt8 nonce[ GUCEF_RPC_NONCE_SIZE ];
    UInt8 proof[ GUCEF_RPC_PROOF_SIZE ];
    SRpcLimits limits;
    TRpcContractOfferVector offers;
    TRpcContractWantVector wants;

    SRpcHello( void );
};

/** HELLO_ACK (6.4.2) */
struct GUCEF_COMCORE_EXPORT_CPP SRpcHelloAck
{
    UInt8 protocolMajor;
    UInt8 protocolMinor;
    UInt64 features;
    CORE::CString nodeName;
    UInt8 result;
    CRpcStatus refusal;
    UInt8 sessionId[ GUCEF_RPC_SESSIONID_SIZE ];
    UInt8 resumeToken[ GUCEF_RPC_RESUMETOKEN_SIZE ];
    SRpcLimits limits;
    TRpcContractOfferVector offers;
    TRpcContractWantVector wants;
    UInt64 gapToInitiatorFrom;
    UInt64 gapToInitiatorTo;
    UInt64 gapToAcceptorFrom;
    UInt64 gapToAcceptorTo;
    UInt64 lastReceivedSeq;
    UInt64 oldestRetainedSeq;
    UInt64 nextSeq;
    UInt8 proof[ GUCEF_RPC_PROOF_SIZE ];

    SRpcHelloAck( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The frame layer of the RPC protocol (gucefCOMCORE RPC design 6): headers, status blocks, request ids, limits,
 *  metadata with key interning, HELLO and HELLO_ACK, and the resume proof (8.2). Stateless: static members only.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcFrameCodec
{
    public:

    static void WriteHeader( CORE::CBinaryWireWriter& writer, UInt8 type, UInt8 flags, UInt64 seq, bool hasAck, UInt64 ackSeq );

    /** False for a malformed header or reserved flag bits (the caller ABORTs with PROTOCOL_ERROR / INTERNAL) */
    static bool ParseHeader( const UInt8* frame, UInt32 size, SRpcFrameHeader& header );

    /** varint code; when not OK: string message, bytes details (the CDataNode binary form, empty = none) */
    static void WriteStatus( CORE::CBinaryWireWriter& writer, const CRpcStatus& status );
    static bool ReadStatus( CORE::CBinaryWireReader& reader, CRpcStatus& status );

    /** u8 kind (0 none, 1 UInt64, 2 string), then varint or string */
    static void WriteRequestId( CORE::CBinaryWireWriter& writer, const CRpcRequestId& id );
    static bool ReadRequestId( CORE::CBinaryWireReader& reader, CRpcRequestId& id );

    static void WriteLimits( CORE::CBinaryWireWriter& writer, const SRpcLimits& limits );
    static bool ReadLimits( CORE::CBinaryWireReader& reader, SRpcLimits& limits );

    static void WriteHash( CORE::CBinaryWireWriter& writer, const CRpcContractHash& hash );
    static bool ReadHash( CORE::CBinaryWireReader& reader, CRpcContractHash& hash );

    /** Metadata (6.9) with the interning table of the sending direction */
    static bool WriteMetadata( CORE::CBinaryWireWriter& writer, const CRpcMetadata& metadata, CRpcMetadataInternTable& table );
    static bool ReadMetadata( CORE::CBinaryWireReader& reader, CRpcMetadata& metadata, CRpcMetadataInternTable& table, CORE::CString& error );

    static void WriteHello( CORE::CBinaryWireWriter& writer, const SRpcHello& hello );
    static bool ReadHello( CORE::CBinaryWireReader& reader, SRpcHello& hello );
    static void WriteHelloAck( CORE::CBinaryWireWriter& writer, const SRpcHelloAck& ack );
    static bool ReadHelloAck( CORE::CBinaryWireReader& reader, SRpcHelloAck& ack );

    /**
     *  proof = HMAC-SHA256( resumeToken, label || sessionId || u64be resumeCount || nonce || u64be lastReceivedSeq
     *                       || u64be oldestRetainedSeq || u64be nextSeq )
     *  with label "GUCEF RPC resume 1" for the initiator and "GUCEF RPC resumed 1" for the acceptor (8.2)
     */
    static void ComputeResumeProof( const UInt8* resumeToken, const char* label, const UInt8* sessionId, UInt64 resumeCount,
                                    const UInt8* nonce, UInt64 lastReceivedSeq, UInt64 oldestRetainedSeq, UInt64 nextSeq,
                                    UInt8* proof );

    static const char* ResumeLabel;         /**< "GUCEF RPC resume 1" */
    static const char* ResumedLabel;        /**< "GUCEF RPC resumed 1" */

    private:

    static void WriteFixedBytes( CORE::CBinaryWireWriter& writer, const UInt8* bytes, UInt32 size );
    static bool ReadFixedBytes( CORE::CBinaryWireReader& reader, UInt8* bytes, UInt32 size );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCFRAMECODEC_H ? */
