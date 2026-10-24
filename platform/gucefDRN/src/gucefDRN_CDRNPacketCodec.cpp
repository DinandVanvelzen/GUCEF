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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNPacketCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNPacketHeader::CDRNPacketHeader( void )
    : sequence( 0 )
    , sendTime( 0 )
    , ackSequence( 0 )
    , ackBits( 0 )
    , echoTime( 0 )
    , echoHold( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNPacketHeader::Clear( void )
{GUCEF_TRACE;
    sequence = 0;
    sendTime = 0;
    ackSequence = 0;
    ackBits = 0;
    echoTime = 0;
    echoHold = 0;
}

/*-------------------------------------------------------------------------*/

CDRNMessageView::CDRNMessageView( void )
    : msgClass( 0 )
    , opcode( 0 )
    , lane( 0 )
    , laneSequence( 0 )
    , payload( GUCEF_NULL )
    , payloadSize( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNPacketCodec::WriteHeader( CDRNWireWriter& writer, const CDRNPacketHeader& header )
{GUCEF_TRACE;
    writer.WriteUInt8( DRN_PROTOCOL_ID );
    writer.WriteVarUInt32( header.sequence );
    writer.WriteVarUInt64( header.sendTime );
    writer.WriteVarUInt32( header.ackSequence );
    writer.WriteUInt32( header.ackBits );
    writer.WriteVarUInt64( header.echoTime );
    writer.WriteVarUInt64( header.echoHold );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketCodec::ReadHeader( CDRNWireReader& reader, CDRNPacketHeader& header )
{GUCEF_TRACE;
    UInt8 protocol = 0;
    if ( !reader.ReadUInt8( protocol ) || DRN_PROTOCOL_ID != protocol )
        return false;

    return reader.ReadVarUInt32( header.sequence )     &&
           reader.ReadVarUInt64( header.sendTime )     &&
           reader.ReadVarUInt32( header.ackSequence )  &&
           reader.ReadUInt32( header.ackBits )         &&
           reader.ReadVarUInt64( header.echoTime )     &&
           reader.ReadVarUInt64( header.echoHold );
}

/*-------------------------------------------------------------------------*/

void
CDRNPacketCodec::WriteMessage( CDRNWireWriter& writer   ,
                               UInt8 msgClass           ,
                               UInt32 opcode            ,
                               UInt8 lane               ,
                               UInt32 laneSequence      ,
                               const void* payload      ,
                               UInt32 payloadSize       )
{GUCEF_TRACE;
    writer.WriteUInt8( msgClass );
    writer.WriteVarUInt32( opcode );
    writer.WriteUInt8( lane );
    if ( DRNIsReliableLane( lane ) )
        writer.WriteVarUInt32( laneSequence );
    writer.WriteSizedBytes( payload, payloadSize );
}

/*-------------------------------------------------------------------------*/

bool
CDRNPacketCodec::ReadMessage( CDRNWireReader& reader, CDRNMessageView& message )
{GUCEF_TRACE;
    if ( !reader.ReadUInt8( message.msgClass ) ||
         !reader.ReadVarUInt32( message.opcode ) ||
         !reader.ReadUInt8( message.lane ) )
        return false;

    if ( message.lane >= DRN_LANE_COUNT )
        return false;

    message.laneSequence = 0;
    if ( DRNIsReliableLane( message.lane ) )
    {
        if ( !reader.ReadVarUInt32( message.laneSequence ) )
            return false;
    }
    return reader.ReadSizedBytes( message.payload, message.payloadSize );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNPacketCodec::GetMessageSize( UInt32 opcode, UInt8 lane, UInt32 laneSequence, UInt32 payloadSize )
{GUCEF_TRACE;
    UInt32 size = 1 + CDRNWireWriter::GetVarUInt32Size( opcode ) + 1;
    if ( DRNIsReliableLane( lane ) )
        size += CDRNWireWriter::GetVarUInt32Size( laneSequence );
    size += CDRNWireWriter::GetVarUInt32Size( payloadSize ) + payloadSize;
    return size;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
