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

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#include "gucefCORE_CDataNodeBinarySerializer.h"
#define GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CDATANODEBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOMCORE_CRpcFrameCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CRpcFrameCodec::ResumeLabel = "GUCEF RPC resume 1";
const char* CRpcFrameCodec::ResumedLabel = "GUCEF RPC resumed 1";

/*-------------------------------------------------------------------------*/

SRpcLimits::SRpcLimits( void )
    : maxFrameSize( 16384 )
    , maxMessageSize( 16 * 1024 * 1024 )
    , receiveWindowBytes( 4 * 1024 * 1024 )
    , initialStreamCredit( 64 )
    , maxConcurrentCalls( 0 )
    , maxBindings( 64 )
    , resumeGraceMs( 600000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

SRpcContractOffer::SRpcContractOffer( void )
    : name()
    , major( 0 )
    , minor( 0 )
    , hash()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

SRpcContractWant::SRpcContractWant( void )
    : name()
    , major( 0 )
    , minMinor( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

SRpcFrameHeader::SRpcFrameHeader( void )
    : type( 0 )
    , flags( 0 )
    , seq( 0 )
    , ackSeq( 0 )
    , hasAck( false )
    , bodyOffset( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

SRpcHello::SRpcHello( void )
    : protocolMajor( GUCEF_RPC_PROTOCOL_MAJOR )
    , protocolMinor( GUCEF_RPC_PROTOCOL_MINOR )
    , features( 0 )
    , nodeName()
    , mode( GUCEF_RPC_HELLO_NEW )
    , resumeCount( 0 )
    , lastReceivedSeq( 0 )
    , oldestRetainedSeq( 0 )
    , nextSeq( 0 )
    , limits()
    , offers()
    , wants()
{GUCEF_TRACE;

    memset( sessionId, 0, sizeof( sessionId ) );
    memset( nonce, 0, sizeof( nonce ) );
    memset( proof, 0, sizeof( proof ) );
}

/*-------------------------------------------------------------------------*/

SRpcHelloAck::SRpcHelloAck( void )
    : protocolMajor( GUCEF_RPC_PROTOCOL_MAJOR )
    , protocolMinor( GUCEF_RPC_PROTOCOL_MINOR )
    , features( 0 )
    , nodeName()
    , result( GUCEF_RPC_HELLOACK_NEW )
    , refusal()
    , limits()
    , offers()
    , wants()
    , gapToInitiatorFrom( 0 )
    , gapToInitiatorTo( 0 )
    , gapToAcceptorFrom( 0 )
    , gapToAcceptorTo( 0 )
    , lastReceivedSeq( 0 )
    , oldestRetainedSeq( 0 )
    , nextSeq( 0 )
{GUCEF_TRACE;

    memset( sessionId, 0, sizeof( sessionId ) );
    memset( resumeToken, 0, sizeof( resumeToken ) );
    memset( proof, 0, sizeof( proof ) );
}

/*-------------------------------------------------------------------------*/

bool
RpcFrameTypeIsControl( UInt8 frameType )
{GUCEF_TRACE;

    return frameType >= GUCEF_RPC_FRAME_HELLO && frameType <= GUCEF_RPC_FRAME_ABORT;
}

/*-------------------------------------------------------------------------*/

const char*
RpcFrameTypeToString( UInt8 frameType )
{GUCEF_TRACE;

    switch ( frameType )
    {
        case GUCEF_RPC_FRAME_HELLO: return "HELLO";
        case GUCEF_RPC_FRAME_HELLO_ACK: return "HELLO_ACK";
        case GUCEF_RPC_FRAME_ACK: return "ACK";
        case GUCEF_RPC_FRAME_PING: return "PING";
        case GUCEF_RPC_FRAME_PONG: return "PONG";
        case GUCEF_RPC_FRAME_STREAM_CREDIT: return "STREAM_CREDIT";
        case GUCEF_RPC_FRAME_ABORT: return "ABORT";
        case GUCEF_RPC_FRAME_GET_CONTRACT: return "GET_CONTRACT";
        case GUCEF_RPC_FRAME_CONTRACT: return "CONTRACT";
        case GUCEF_RPC_FRAME_BIND: return "BIND";
        case GUCEF_RPC_FRAME_BIND_ACK: return "BIND_ACK";
        case GUCEF_RPC_FRAME_REQUEST: return "REQUEST";
        case GUCEF_RPC_FRAME_RESPONSE: return "RESPONSE";
        case GUCEF_RPC_FRAME_ACCEPTED: return "ACCEPTED";
        case GUCEF_RPC_FRAME_RESULT: return "RESULT";
        case GUCEF_RPC_FRAME_STREAM_MSG: return "STREAM_MSG";
        case GUCEF_RPC_FRAME_HALF_CLOSE: return "HALF_CLOSE";
        case GUCEF_RPC_FRAME_CANCEL: return "CANCEL";
        case GUCEF_RPC_FRAME_STATUS_QUERY: return "STATUS_QUERY";
        case GUCEF_RPC_FRAME_STATUS: return "STATUS";
        case GUCEF_RPC_FRAME_CONTINUATION: return "CONTINUATION";
        case GUCEF_RPC_FRAME_ERROR: return "ERROR";
        case GUCEF_RPC_FRAME_GOODBYE: return "GOODBYE";
        default: return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------*/

CRpcMetadataInternTable::CRpcMetadataInternTable( void )
    : m_keys()
    , m_index()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMetadataInternTable::Find( const CORE::CString& key ) const
{GUCEF_TRACE;

    GUCEF::map< CORE::CString, UInt32 >::const_iterator i = m_index.find( key );
    return i != m_index.end() ? (*i).second : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMetadataInternTable::Add( const CORE::CString& key )
{GUCEF_TRACE;

    if ( m_keys.size() >= MaxInternedKeys )
        return 0;
    UInt32 index = GUCEF_RPC_METADATA_FIRST_INTERNED + (UInt32) m_keys.size();
    m_keys.push_back( key );
    m_index[ key ] = index;
    return index;
}

/*-------------------------------------------------------------------------*/

bool
CRpcMetadataInternTable::Get( UInt32 index, CORE::CString& key ) const
{GUCEF_TRACE;

    if ( index < GUCEF_RPC_METADATA_FIRST_INTERNED || index - GUCEF_RPC_METADATA_FIRST_INTERNED >= m_keys.size() )
        return false;
    key = m_keys[ index - GUCEF_RPC_METADATA_FIRST_INTERNED ];
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcMetadataInternTable::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_keys.size();
}

/*-------------------------------------------------------------------------*/

void
CRpcMetadataInternTable::Clear( void )
{GUCEF_TRACE;

    m_keys.clear();
    m_index.clear();
}

/*-------------------------------------------------------------------------*/

void
CRpcMetadataInternTable::Truncate( UInt32 count )
{GUCEF_TRACE;

    while ( m_keys.size() > count )
    {
        m_index.erase( m_keys.back() );
        m_keys.pop_back();
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcFrameCodec::WriteHeader( CORE::CBinaryWireWriter& writer, UInt8 type, UInt8 flags, UInt64 seq, bool hasAck, UInt64 ackSeq )
{GUCEF_TRACE;

    writer.WriteUInt8( type );
    writer.WriteUInt8( hasAck ? (UInt8) ( flags | GUCEF_RPC_FRAMEFLAG_ACK ) : (UInt8) ( flags & ~GUCEF_RPC_FRAMEFLAG_ACK ) );
    writer.WriteVarUInt64( seq );
    if ( hasAck )
        writer.WriteVarUInt64( ackSeq );
}

/*-------------------------------------------------------------------------*/

bool
CRpcFrameCodec::ParseHeader( const UInt8* frame, UInt32 size, SRpcFrameHeader& header )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( frame, size );
    if ( !reader.ReadUInt8( header.type ) || !reader.ReadUInt8( header.flags ) || !reader.ReadVarUInt64( header.seq ) )
        return false;
    if ( 0 != ( header.flags & GUCEF_RPC_FRAMEFLAG_RESERVED_MASK ) )
        return false;
    header.hasAck = 0 != ( header.flags & GUCEF_RPC_FRAMEFLAG_ACK );
    header.ackSeq = 0;
    if ( header.hasAck && !reader.ReadVarUInt64( header.ackSeq ) )
        return false;
    // control frames are never sequenced, session frames always
    if ( RpcFrameTypeIsControl( header.type ) != ( 0 == header.seq ) )
        return false;
    header.bodyOffset = reader.GetPosition();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcFrameCodec::WriteStatus( CORE::CBinaryWireWriter& writer, const CRpcStatus& status )
{GUCEF_TRACE;

    writer.WriteVarUInt32( (UInt32) status.code );
    if ( RPC_STATUS_OK == status.code )
        return;
    writer.WriteString( status.message );
    if ( status.details.HasChildren() || status.details.GetAttCount() > 0 || status.details.HasValue() )
    {
        CORE::CDynamicBuffer details;
        UInt32 written = 0;
        CORE::CDataNodeBinarySerializerOptions options;
        if ( CORE::CDataNodeBinarySerializer::Serialize( options, status.details, 0, details, written ) )
        {
            writer.WriteSizedBytes( details.GetConstBufferPtr(), written );
            return;
        }
    }
    writer.WriteVarUInt32( 0 );
}

/*-------------------------------------------------------------------------*/

bool
CRpcFrameCodec::ReadStatus( CORE::CBinaryWireReader& reader, CRpcStatus& status )
{GUCEF_TRACE;

    UInt32 code = 0;
    if ( !reader.ReadVarUInt32( code ) )
        return false;
    status.code = (TRpcStatusCode) code;
    status.message.Clear();
    status.details.Clear();
    if ( RPC_STATUS_OK == code )
        return true;
    if ( !reader.ReadString( status.message ) )
        return false;
    const UInt8* details = GUCEF_NULL;
    UInt32 detailsSize = 0;
    if ( !reader.ReadSizedBytes( details, detailsSize ) )
        return false;
    if ( detailsSize > 0 )
    {
        CORE::CDynamicBuffer buffer;
        buffer.LinkTo( details, detailsSize );
        UInt32 bytesRead = 0;
        CORE::CDataNodeBinarySerializerOptions options;
        if ( !CORE::CDataNodeBinarySerializer::Deserialize( options, false, status.details, 0, buffer, bytesRead ) )
            status.details.Clear();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcFrameCodec::WriteRequestId( CORE::CBinaryWireWriter& writer, const CRpcRequestId& id )
{GUCEF_TRACE;

    writer.WriteUInt8( (UInt8) id.GetKind() );
    if ( CRpcRequestId::KIND_UINT64 == id.GetKind() )
        writer.WriteVarUInt64( id.GetUInt64() );
    else if ( CRpcRequestId::KIND_STRING == id.GetKind() )
        writer.WriteString( id.GetString() );
}

/*-------------------------------------------------------------------------*/

bool
CRpcFrameCodec::ReadRequestId( CORE::CBinaryWireReader& reader, CRpcRequestId& id )
{GUCEF_TRACE;

    UInt8 kind = 0;
    if ( !reader.ReadUInt8( kind ) )
        return false;
    switch ( kind )
    {
        case CRpcRequestId::KIND_NONE:
            id.Clear();
            return true;
        case CRpcRequestId::KIND_UINT64:
        {
            UInt64 value = 0;
            if ( !reader.ReadVarUInt64( value ) )
                return false;
            id = CRpcRequestId( value );
            return true;
        }
        case CRpcRequestId::KIND_STRING:
        {
            const char* utf8 = GUCEF_NULL;
            UInt32 size = 0;
            if ( !reader.ReadUtf8( utf8, size ) || size > CRpcRequestId::MaxStringBytes )
                return reader.Fail();
            id = CRpcRequestId( CORE::ToString( CORE::CUtf8String( utf8, size ) ) );
            return true;
        }
        default:
            return reader.Fail();
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcFrameCodec::WriteLimits( CORE::CBinaryWireWriter& writer, const SRpcLimits& limits )
{GUCEF_TRACE;

    writer.WriteVarUInt32( limits.maxFrameSize );
    writer.WriteVarUInt32( limits.maxMessageSize );
    writer.WriteVarUInt32( limits.receiveWindowBytes );
    writer.WriteVarUInt32( limits.initialStreamCredit );
    writer.WriteVarUInt32( limits.maxConcurrentCalls );
    writer.WriteVarUInt32( limits.maxBindings );
    writer.WriteVarUInt32( limits.resumeGraceMs );
}

/*-------------------------------------------------------------------------*/

bool
CRpcFrameCodec::ReadLimits( CORE::CBinaryWireReader& reader, SRpcLimits& limits )
{GUCEF_TRACE;

    return reader.ReadVarUInt32( limits.maxFrameSize ) &&
           reader.ReadVarUInt32( limits.maxMessageSize ) &&
           reader.ReadVarUInt32( limits.receiveWindowBytes ) &&
           reader.ReadVarUInt32( limits.initialStreamCredit ) &&
           reader.ReadVarUInt32( limits.maxConcurrentCalls ) &&
           reader.ReadVarUInt32( limits.maxBindings ) &&
           reader.ReadVarUInt32( limits.resumeGraceMs );
}

/*-------------------------------------------------------------------------*/

void
CRpcFrameCodec::WriteFixedBytes( CORE::CBinaryWireWriter& writer, const UInt8* bytes, UInt32 size )
{GUCEF_TRACE;

    writer.WriteSizedBytes( bytes, size );
}

/*-------------------------------------------------------------------------*/

bool
CRpcFrameCodec::ReadFixedBytes( CORE::CBinaryWireReader& reader, UInt8* bytes, UInt32 size )
{GUCEF_TRACE;

    const UInt8* data = GUCEF_NULL;
    UInt32 length = 0;
    if ( !reader.ReadSizedBytes( data, length ) || length != size )
        return reader.Fail();
    memcpy( bytes, data, size );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcFrameCodec::WriteHash( CORE::CBinaryWireWriter& writer, const CRpcContractHash& hash )
{GUCEF_TRACE;

    WriteFixedBytes( writer, hash.GetBytes(), CRpcContractHash::Size );
}

/*-------------------------------------------------------------------------*/

bool
CRpcFrameCodec::ReadHash( CORE::CBinaryWireReader& reader, CRpcContractHash& hash )
{GUCEF_TRACE;

    UInt8 bytes[ CRpcContractHash::Size ];
    if ( !ReadFixedBytes( reader, bytes, CRpcContractHash::Size ) )
        return false;
    hash.SetBytes( bytes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcFrameCodec::WriteMetadata( CORE::CBinaryWireWriter& writer, const CRpcMetadata& metadata, CRpcMetadataInternTable& table )
{GUCEF_TRACE;

    UInt32 count = metadata.GetCount();
    if ( count > CRpcMetadata::MaxEntries )
        return false;
    UInt32 start = writer.GetOffset();
    writer.WriteVarUInt32( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        const CORE::CString& key = metadata.GetKeyAt( i );
        const CORE::CVariant& value = metadata.GetValueAt( i );
        UInt32 wellKnown = CRpcMetadata::GetWellKnownKeyRef( key );
        UInt32 interned = 0 == wellKnown ? table.Find( key ) : 0;
        if ( 0 != wellKnown )
            writer.WriteVarUInt32( wellKnown );
        else if ( 0 != interned )
            writer.WriteVarUInt32( interned );
        else if ( 0 != table.Add( key ) )
        {
            writer.WriteVarUInt32( 0 );     // define
            writer.WriteString( key );
        }
        else
        {
            writer.WriteVarUInt32( 1 );     // inline
            writer.WriteString( key );
        }

        if ( value.IsBoolean() )
        {
            writer.WriteUInt8( 4 );
            writer.WriteBool( value.AsBool() );
        }
        else if ( value.IsUnsignedInteger() )
        {
            writer.WriteUInt8( 2 );
            writer.WriteVarUInt64( value.AsUInt64() );
        }
        else if ( value.IsSignedInteger() )
        {
            writer.WriteUInt8( 3 );
            writer.WriteVarInt64( value.AsInt64() );
        }
        else if ( value.IsBinary() )
        {
            writer.WriteUInt8( 1 );
            writer.WriteSizedBytes( value.AsVoidPtr(), value.ByteSize( false ) );
        }
        else
        {
            writer.WriteUInt8( 0 );
            writer.WriteString( value.AsString() );
        }
    }
    return writer.GetOffset() - start <= CRpcMetadata::MaxBytes;
}

/*-------------------------------------------------------------------------*/

bool
CRpcFrameCodec::ReadMetadata( CORE::CBinaryWireReader& reader, CRpcMetadata& metadata, CRpcMetadataInternTable& table, CORE::CString& error )
{GUCEF_TRACE;

    metadata.Clear();
    UInt32 start = reader.GetPosition();
    UInt32 count = 0;
    if ( !reader.ReadVarUInt32( count ) )
    {
        error = "malformed metadata";
        return false;
    }
    if ( count > CRpcMetadata::MaxEntries )
    {
        error = "more than 64 metadata entries";
        return false;
    }
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt32 keyRef = 0;
        CORE::CString key;
        if ( !reader.ReadVarUInt32( keyRef ) )
        {
            error = "malformed metadata key";
            return false;
        }
        if ( 0 == keyRef || 1 == keyRef )
        {
            if ( !reader.ReadString( key ) )
            {
                error = "malformed metadata key";
                return false;
            }
            if ( 0 == keyRef && 0 == table.Add( key ) )
            {
                error = "more than 256 interned metadata keys";
                return false;
            }
        }
        else if ( keyRef < GUCEF_RPC_METADATA_FIRST_INTERNED )
        {
            const char* wellKnown = CRpcMetadata::GetWellKnownKey( keyRef );
            if ( '\0' == *wellKnown )
            {
                error = "unknown well-known metadata key " + CORE::ToString( keyRef );
                return false;
            }
            key = wellKnown;
        }
        else if ( !table.Get( keyRef, key ) )
        {
            error = "unknown interned metadata key " + CORE::ToString( keyRef );
            return false;
        }

        UInt8 valueKind = 0;
        if ( !reader.ReadUInt8( valueKind ) )
        {
            error = "malformed metadata value";
            return false;
        }
        CORE::CVariant value;
        bool ok = true;
        switch ( valueKind )
        {
            case 0: { CORE::CString text; ok = reader.ReadString( text ); value = text; break; }
            case 1: { const UInt8* data = GUCEF_NULL; UInt32 size = 0; ok = reader.ReadSizedBytes( data, size ); if ( ok ) value.Set( data, size, GUCEF_DATATYPE_BINARY_BLOB ); break; }
            case 2: { UInt64 v = 0; ok = reader.ReadVarUInt64( v ); value = v; break; }
            case 3: { Int64 v = 0; ok = reader.ReadVarInt64( v ); value = v; break; }
            case 4: { bool v = false; ok = reader.ReadBool( v ); value = v; break; }
            default: ok = false; break;
        }
        if ( !ok )
        {
            error = "malformed metadata value";
            return false;
        }
        metadata.Set( key, value );
    }
    if ( reader.GetPosition() - start > CRpcMetadata::MaxBytes )
    {
        error = "metadata larger than 8 KB";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcFrameCodec::WriteHello( CORE::CBinaryWireWriter& writer, const SRpcHello& hello )
{GUCEF_TRACE;

    writer.WriteUInt8( hello.protocolMajor );
    writer.WriteUInt8( hello.protocolMinor );
    writer.WriteVarUInt64( hello.features );
    writer.WriteString( hello.nodeName );
    writer.WriteUInt8( hello.mode );
    if ( GUCEF_RPC_HELLO_RESUME == hello.mode )
    {
        WriteFixedBytes( writer, hello.sessionId, GUCEF_RPC_SESSIONID_SIZE );
        writer.WriteVarUInt64( hello.resumeCount );
        writer.WriteVarUInt64( hello.lastReceivedSeq );
        writer.WriteVarUInt64( hello.oldestRetainedSeq );
        writer.WriteVarUInt64( hello.nextSeq );
        WriteFixedBytes( writer, hello.nonce, GUCEF_RPC_NONCE_SIZE );
        WriteFixedBytes( writer, hello.proof, GUCEF_RPC_PROOF_SIZE );
    }
    WriteLimits( writer, hello.limits );
    if ( GUCEF_RPC_HELLO_NEW == hello.mode )
    {
        writer.WriteVarUInt32( (UInt32) hello.offers.size() );
        for ( UInt32 i=0; i<hello.offers.size(); ++i )
        {
            writer.WriteString( hello.offers[ i ].name );
            writer.WriteVarUInt32( hello.offers[ i ].major );
            writer.WriteVarUInt32( hello.offers[ i ].minor );
            WriteHash( writer, hello.offers[ i ].hash );
        }
        writer.WriteVarUInt32( (UInt32) hello.wants.size() );
        for ( UInt32 i=0; i<hello.wants.size(); ++i )
        {
            writer.WriteString( hello.wants[ i ].name );
            writer.WriteVarUInt32( hello.wants[ i ].major );
            writer.WriteVarUInt32( hello.wants[ i ].minMinor );
        }
    }
}

/*-------------------------------------------------------------------------*/

namespace {

bool
ReadOffersAndWants( CORE::CBinaryWireReader& reader, TRpcContractOfferVector& offers, TRpcContractWantVector& wants )
{
    UInt32 count = 0;
    if ( !reader.ReadVarUInt32( count ) || count > 4096 )
        return false;
    offers.clear();
    for ( UInt32 i=0; i<count; ++i )
    {
        SRpcContractOffer offer;
        if ( !reader.ReadString( offer.name ) || !reader.ReadVarUInt32( offer.major ) || !reader.ReadVarUInt32( offer.minor ) ||
             !CRpcFrameCodec::ReadHash( reader, offer.hash ) )
            return false;
        offers.push_back( offer );
    }
    if ( !reader.ReadVarUInt32( count ) || count > 4096 )
        return false;
    wants.clear();
    for ( UInt32 i=0; i<count; ++i )
    {
        SRpcContractWant want;
        if ( !reader.ReadString( want.name ) || !reader.ReadVarUInt32( want.major ) || !reader.ReadVarUInt32( want.minMinor ) )
            return false;
        wants.push_back( want );
    }
    return true;
}

void
WriteOffersAndWants( CORE::CBinaryWireWriter& writer, const TRpcContractOfferVector& offers, const TRpcContractWantVector& wants )
{
    writer.WriteVarUInt32( (UInt32) offers.size() );
    for ( UInt32 i=0; i<offers.size(); ++i )
    {
        writer.WriteString( offers[ i ].name );
        writer.WriteVarUInt32( offers[ i ].major );
        writer.WriteVarUInt32( offers[ i ].minor );
        CRpcFrameCodec::WriteHash( writer, offers[ i ].hash );
    }
    writer.WriteVarUInt32( (UInt32) wants.size() );
    for ( UInt32 i=0; i<wants.size(); ++i )
    {
        writer.WriteString( wants[ i ].name );
        writer.WriteVarUInt32( wants[ i ].major );
        writer.WriteVarUInt32( wants[ i ].minMinor );
    }
}

}

/*-------------------------------------------------------------------------*/

bool
CRpcFrameCodec::ReadHello( CORE::CBinaryWireReader& reader, SRpcHello& hello )
{GUCEF_TRACE;

    if ( !reader.ReadUInt8( hello.protocolMajor ) || !reader.ReadUInt8( hello.protocolMinor ) ||
         !reader.ReadVarUInt64( hello.features ) || !reader.ReadString( hello.nodeName ) || !reader.ReadUInt8( hello.mode ) )
        return false;
    if ( GUCEF_RPC_HELLO_RESUME == hello.mode )
    {
        if ( !ReadFixedBytes( reader, hello.sessionId, GUCEF_RPC_SESSIONID_SIZE ) ||
             !reader.ReadVarUInt64( hello.resumeCount ) ||
             !reader.ReadVarUInt64( hello.lastReceivedSeq ) ||
             !reader.ReadVarUInt64( hello.oldestRetainedSeq ) ||
             !reader.ReadVarUInt64( hello.nextSeq ) ||
             !ReadFixedBytes( reader, hello.nonce, GUCEF_RPC_NONCE_SIZE ) ||
             !ReadFixedBytes( reader, hello.proof, GUCEF_RPC_PROOF_SIZE ) )
            return false;
    }
    else if ( GUCEF_RPC_HELLO_NEW != hello.mode )
        return false;
    if ( !ReadLimits( reader, hello.limits ) )
        return false;
    if ( GUCEF_RPC_HELLO_NEW == hello.mode && !ReadOffersAndWants( reader, hello.offers, hello.wants ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcFrameCodec::WriteHelloAck( CORE::CBinaryWireWriter& writer, const SRpcHelloAck& ack )
{GUCEF_TRACE;

    writer.WriteUInt8( ack.protocolMajor );
    writer.WriteUInt8( ack.protocolMinor );
    writer.WriteVarUInt64( ack.features );
    writer.WriteString( ack.nodeName );
    writer.WriteUInt8( ack.result );
    switch ( ack.result )
    {
        case GUCEF_RPC_HELLOACK_REFUSED:
            WriteStatus( writer, ack.refusal );
            break;
        case GUCEF_RPC_HELLOACK_NEW:
        case GUCEF_RPC_HELLOACK_RESUME_GAP:
        case GUCEF_RPC_HELLOACK_SESSION_UNKNOWN:
            WriteFixedBytes( writer, ack.sessionId, GUCEF_RPC_SESSIONID_SIZE );
            WriteFixedBytes( writer, ack.resumeToken, GUCEF_RPC_RESUMETOKEN_SIZE );
            WriteLimits( writer, ack.limits );
            WriteOffersAndWants( writer, ack.offers, ack.wants );
            if ( GUCEF_RPC_HELLOACK_RESUME_GAP == ack.result )
            {
                writer.WriteVarUInt64( ack.gapToInitiatorFrom );
                writer.WriteVarUInt64( ack.gapToInitiatorTo );
                writer.WriteVarUInt64( ack.gapToAcceptorFrom );
                writer.WriteVarUInt64( ack.gapToAcceptorTo );
            }
            break;
        case GUCEF_RPC_HELLOACK_RESUMED:
            writer.WriteVarUInt64( ack.lastReceivedSeq );
            writer.WriteVarUInt64( ack.oldestRetainedSeq );
            writer.WriteVarUInt64( ack.nextSeq );
            WriteFixedBytes( writer, ack.proof, GUCEF_RPC_PROOF_SIZE );
            WriteLimits( writer, ack.limits );
            break;
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcFrameCodec::ReadHelloAck( CORE::CBinaryWireReader& reader, SRpcHelloAck& ack )
{GUCEF_TRACE;

    if ( !reader.ReadUInt8( ack.protocolMajor ) || !reader.ReadUInt8( ack.protocolMinor ) ||
         !reader.ReadVarUInt64( ack.features ) || !reader.ReadString( ack.nodeName ) || !reader.ReadUInt8( ack.result ) )
        return false;
    switch ( ack.result )
    {
        case GUCEF_RPC_HELLOACK_REFUSED:
            return ReadStatus( reader, ack.refusal );
        case GUCEF_RPC_HELLOACK_NEW:
        case GUCEF_RPC_HELLOACK_RESUME_GAP:
        case GUCEF_RPC_HELLOACK_SESSION_UNKNOWN:
            if ( !ReadFixedBytes( reader, ack.sessionId, GUCEF_RPC_SESSIONID_SIZE ) ||
                 !ReadFixedBytes( reader, ack.resumeToken, GUCEF_RPC_RESUMETOKEN_SIZE ) ||
                 !ReadLimits( reader, ack.limits ) ||
                 !ReadOffersAndWants( reader, ack.offers, ack.wants ) )
                return false;
            if ( GUCEF_RPC_HELLOACK_RESUME_GAP == ack.result )
                return reader.ReadVarUInt64( ack.gapToInitiatorFrom ) && reader.ReadVarUInt64( ack.gapToInitiatorTo ) &&
                       reader.ReadVarUInt64( ack.gapToAcceptorFrom ) && reader.ReadVarUInt64( ack.gapToAcceptorTo );
            return true;
        case GUCEF_RPC_HELLOACK_RESUMED:
            return reader.ReadVarUInt64( ack.lastReceivedSeq ) && reader.ReadVarUInt64( ack.oldestRetainedSeq ) &&
                   reader.ReadVarUInt64( ack.nextSeq ) && ReadFixedBytes( reader, ack.proof, GUCEF_RPC_PROOF_SIZE ) &&
                   ReadLimits( reader, ack.limits );
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcFrameCodec::ComputeResumeProof( const UInt8* resumeToken, const char* label, const UInt8* sessionId, UInt64 resumeCount,
                                    const UInt8* nonce, UInt64 lastReceivedSeq, UInt64 oldestRetainedSeq, UInt64 nextSeq,
                                    UInt8* proof )
{GUCEF_TRACE;

    CORE::CDynamicBuffer input;
    CORE::CBinaryWireWriter writer( input );
    writer.WriteBytes( label, (UInt32) strlen( label ) );
    writer.WriteBytes( sessionId, GUCEF_RPC_SESSIONID_SIZE );
    writer.WriteUInt64BigEndian( resumeCount );
    writer.WriteBytes( nonce, GUCEF_RPC_NONCE_SIZE );
    writer.WriteUInt64BigEndian( lastReceivedSeq );
    writer.WriteUInt64BigEndian( oldestRetainedSeq );
    writer.WriteUInt64BigEndian( nextSeq );
    CORE::CHmacSha256::Mac( resumeToken, GUCEF_RPC_RESUMETOKEN_SIZE, input.GetConstBufferPtr(), input.GetDataSize(), proof );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
