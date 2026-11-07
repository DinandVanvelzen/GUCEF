/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILDELIVERYEVENT_H
#include "matchprovidermail_CEMailDeliveryEvent.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILDELIVERYEVENT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILDELIVERYEVENT_H ? */

#include "matchprovidermail_CEMailOutboxItem.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 g_maxItemFileBytes = 8 * 1024 * 1024;
static const UInt32 g_maxRecipients = 64;
static const UInt32 g_maxAddressBytes = 320;
static const char g_itemMagic[ 4 ] = { 'G', 'M', 'O', 'B' };
static const UInt8 g_itemVersion = 1;
static const char* const g_itemExtension = ".rdxout";

static void
WriteStrings( MULTIPLAY::CMatchBinaryWriter& writer, const CORE::CString::StringVector& list )
{GUCEF_TRACE;

    writer.WriteUInt32( (UInt32) list.size() );
    for ( size_t i=0; i<list.size(); ++i )
        writer.WriteString( list[ i ] );
}

static bool
ReadStrings( MULTIPLAY::CMatchBinaryReader& reader, CORE::CString::StringVector& list )
{GUCEF_TRACE;

    list.clear();
    const UInt32 count = reader.ReadUInt32();
    if ( !reader.IsOk() || count > g_maxRecipients )
        return false;
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CString text;
        if ( !reader.ReadString( text, g_maxAddressBytes ) )
            return false;
        list.push_back( text );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailOutboxItem::CEMailOutboxItem( void )
    : id( 0 )
    , kind( MAILKIND_TURN )
    , matchId()
    , revision( 0 )
    , messageId()
    , envelopeFrom()
    , recipients()
    , awaiting()
    , packageSha256()
    , needsConfirmation( false )
    , state( MAILDELIVERY_QUEUED )
    , attempts( 0 )
    , createdAt( 0 )
    , nextAttemptAt( 0 )
    , lastSentAt( 0 )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CEMailOutboxItem::ToBytes( CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    bytes.Clear();
    MULTIPLAY::CMatchBinaryWriter writer( bytes );
    writer.WriteRaw( g_itemMagic, 4 );
    writer.WriteUInt8( g_itemVersion );
    writer.WriteUInt32( id );
    writer.WriteUInt8( (UInt8) kind );
    writer.WriteString( matchId );
    writer.WriteUInt32( revision );
    writer.WriteString( messageId );
    writer.WriteString( envelopeFrom );
    WriteStrings( writer, recipients );
    WriteStrings( writer, awaiting );
    writer.WriteString( packageSha256 );
    writer.WriteBool( needsConfirmation );
    writer.WriteUInt8( (UInt8) state );
    writer.WriteUInt32( attempts );
    writer.WriteInt64( createdAt );
    writer.WriteInt64( nextAttemptAt );
    writer.WriteInt64( lastSentAt );
    writer.WriteBytes( message.GetConstBufferPtr(), message.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailOutboxItem::FromBytes( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == data || size < 8 || size > g_maxItemFileBytes )
        return false;
    MULTIPLAY::CMatchBinaryReader reader( data, size );
    char magic[ 4 ];
    if ( !reader.ReadRaw( magic, 4 ) || 0 != memcmp( magic, g_itemMagic, 4 ) || g_itemVersion != reader.ReadUInt8() )
        return false;
    CEMailOutboxItem read;
    read.id = reader.ReadUInt32();
    const UInt8 kindValue = reader.ReadUInt8();
    if ( !reader.IsOk() || kindValue >= (UInt8) MAILKIND_UNKNOWN )
        return false;
    read.kind = (TMailMatchKind) kindValue;
    if ( !reader.ReadString( read.matchId, 36 ) || !MULTIPLAY::CMatchId::IsValid( read.matchId ) )
        return false;
    read.revision = reader.ReadUInt32();
    if ( !reader.ReadString( read.messageId, 998 ) || !reader.ReadString( read.envelopeFrom, g_maxAddressBytes ) )
        return false;
    if ( !ReadStrings( reader, read.recipients ) || !ReadStrings( reader, read.awaiting ) || read.recipients.empty() )
        return false;
    if ( !reader.ReadString( read.packageSha256, 64 ) )
        return false;
    read.needsConfirmation = reader.ReadBool();
    const UInt8 stateValue = reader.ReadUInt8();
    if ( !reader.IsOk() || stateValue > (UInt8) MAILDELIVERY_FAILED )
        return false;
    read.state = (TMailDeliveryState) stateValue;
    read.attempts = reader.ReadUInt32();
    read.createdAt = reader.ReadInt64();
    read.nextAttemptAt = reader.ReadInt64();
    read.lastSentAt = reader.ReadInt64();
    UInt32 messageSize = 0;
    const UInt8* message = reader.ReadBytesInPlace( messageSize, g_maxItemFileBytes );
    if ( !reader.IsAtEndAndOk() || GUCEF_NULL == message || 0 == messageSize )
        return false;
    read.message.Append( message, messageSize );
    *this = read;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
