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

#ifndef GUCEF_DRN_CDRNRESOURCENAME_H
#include "gucefDRN_CDRNResourceName.h"
#define GUCEF_DRN_CDRNRESOURCENAME_H
#endif /* GUCEF_DRN_CDRNRESOURCENAME_H ? */

#include "gucefDRN_CDRNNamespaceCodec.h"

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

/** Limits in the extension of the hello answer: open handles, active reads, namespace requests, listings, upload rate */
static const UInt32 HelloLimitFieldCount = 5;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteText( CDRNWireWriter& writer, const CORE::CString& text )
{GUCEF_TRACE;

    // the bytes as they are, a name of a file system is not converted: length, bytes
    writer.WriteSizedBytes( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadText( CDRNWireReader& reader, CORE::CString& text )
{GUCEF_TRACE;

    const UInt8* data = GUCEF_NULL;
    UInt32 size = 0;
    if ( !reader.ReadSizedBytes( data, size ) )
        return false;
    if ( 0 == size )
        text.Clear();
    else
        text.Set( reinterpret_cast< const char* >( data ), size );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteHelloRequest( CDRNWireWriter& writer, UInt32 requestId, UInt32 maxVersion, UInt32 clientCaps )
{GUCEF_TRACE;

    writer.WriteVarUInt32( requestId );
    writer.WriteVarUInt32( maxVersion );
    writer.WriteVarUInt32( clientCaps );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadHelloRequest( CDRNWireReader& reader, UInt32& requestId, UInt32& maxVersion, UInt32& clientCaps )
{GUCEF_TRACE;

    return reader.ReadVarUInt32( requestId ) && reader.ReadVarUInt32( maxVersion ) && reader.ReadVarUInt32( clientCaps );
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteSharesRequest( CDRNWireWriter& writer, UInt32 requestId )
{GUCEF_TRACE;

    writer.WriteVarUInt32( requestId );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadSharesRequest( CDRNWireReader& reader, UInt32& requestId )
{GUCEF_TRACE;

    return reader.ReadVarUInt32( requestId );
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteStatRequestHeader( CDRNWireWriter& writer, UInt32 requestId, const CORE::CString& share, UInt32 count )
{GUCEF_TRACE;

    writer.WriteVarUInt32( requestId );
    WriteText( writer, share );
    writer.WriteVarUInt32( count );
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteStatRequestItem( CDRNWireWriter& writer, const CORE::CString& path, const CORE::CString& ifRevisionNot )
{GUCEF_TRACE;

    WriteText( writer, path );
    WriteText( writer, ifRevisionNot );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadStatRequestHeader( CDRNWireReader& reader, UInt32& requestId, CORE::CString& share, UInt32& count )
{GUCEF_TRACE;

    return reader.ReadVarUInt32( requestId ) && ReadText( reader, share ) && reader.ReadVarUInt32( count );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadStatRequestItem( CDRNWireReader& reader, CORE::CString& path, CORE::CString& ifRevisionNot )
{GUCEF_TRACE;

    return ReadText( reader, path ) && ReadText( reader, ifRevisionNot );
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteListRequest( CDRNWireWriter& writer, UInt32 requestId, const CORE::CString& share, const CORE::CString& dir,
                                      const CORE::CString& cursor, UInt32 maxEntries, const CORE::CString& ifRevisionNot )
{GUCEF_TRACE;

    writer.WriteVarUInt32( requestId );
    WriteText( writer, share );
    WriteText( writer, dir );
    WriteText( writer, cursor );
    writer.WriteVarUInt32( maxEntries );
    WriteText( writer, ifRevisionNot );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadListRequest( CDRNWireReader& reader, UInt32& requestId, CORE::CString& share, CORE::CString& dir,
                                     CORE::CString& cursor, UInt32& maxEntries, CORE::CString& ifRevisionNot )
{GUCEF_TRACE;

    return reader.ReadVarUInt32( requestId ) && ReadText( reader, share ) && ReadText( reader, dir ) && ReadText( reader, cursor ) &&
           reader.ReadVarUInt32( maxEntries ) && ReadText( reader, ifRevisionNot );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::PeekRequestId( const void* body, UInt32 size, UInt32& requestId )
{GUCEF_TRACE;

    CDRNWireReader reader( body, size );
    return reader.ReadVarUInt32( requestId );
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteAnswerHeader( CDRNWireWriter& writer, UInt32 requestId, UInt32 kind, TDRNResourceStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    writer.WriteVarUInt32( requestId );
    writer.WriteVarUInt32( kind );
    writer.WriteVarUInt32( (UInt32) status );
    WriteText( writer, message );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadAnswerHeader( CDRNWireReader& reader, UInt32& requestId, UInt32& kind, TDRNResourceStatus& status, CORE::CString& message )
{GUCEF_TRACE;

    UInt32 rawStatus = 0;
    if ( !reader.ReadVarUInt32( requestId ) || !reader.ReadVarUInt32( kind ) || !reader.ReadVarUInt32( rawStatus ) || !ReadText( reader, message ) )
        return false;
    // a status this build does not know is an error of the serving side we cannot name
    status = rawStatus < (UInt32) DRN_RESSTATUS_COUNT ? (TDRNResourceStatus) rawStatus : DRN_RESSTATUS_IO_ERROR;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteHelloBody( CDRNWireWriter& writer, const CDRNNamespaceInfo& info )
{GUCEF_TRACE;

    writer.WriteVarUInt32( info.version );
    writer.WriteVarUInt32( info.capabilities );
    writer.WriteVarUInt32( info.maxStatPaths );
    writer.WriteVarUInt32( info.maxPageEntries );
    writer.WriteVarUInt32( info.maxAnswerBytes );
    writer.WriteVarUInt32( info.listingLifetimeMs );

    // the extension: the limits of the asking member, in a block with a length of its own so that a reader that knows
    // fewer or more of them takes what it knows and skips the rest, and one that knows none never looks at it
    if ( info.limitsAnnounced )
    {
        CORE::CDynamicBuffer block;
        CDRNWireWriter blockWriter( block );
        blockWriter.WriteVarUInt32( HelloLimitFieldCount );
        blockWriter.WriteVarUInt32( info.maxOpenHandles );
        blockWriter.WriteVarUInt32( info.maxActiveReads );
        blockWriter.WriteVarUInt32( info.maxNamespaceRequests );
        blockWriter.WriteVarUInt32( info.maxNamespaceListings );
        blockWriter.WriteVarUInt32( info.maxUploadBytesPerSecond );
        writer.WriteSizedBytes( block.GetConstBufferPtr(), blockWriter.GetOffset() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadHelloBody( CDRNWireReader& reader, CDRNNamespaceInfo& info )
{GUCEF_TRACE;

    if ( !( reader.ReadVarUInt32( info.version ) && reader.ReadVarUInt32( info.capabilities ) && reader.ReadVarUInt32( info.maxStatPaths ) &&
            reader.ReadVarUInt32( info.maxPageEntries ) && reader.ReadVarUInt32( info.maxAnswerBytes ) && reader.ReadVarUInt32( info.listingLifetimeMs ) ) )
        return false;

    // no extension (an answer of a serving side that does not announce limits): the limits are unknown
    info.limitsAnnounced = false;
    info.maxOpenHandles = 0;
    info.maxActiveReads = 0;
    info.maxNamespaceRequests = 0;
    info.maxNamespaceListings = 0;
    info.maxUploadBytesPerSecond = 0;
    if ( 0 == reader.GetRemaining() )
        return true;

    // a block that is not what it says it is makes the whole answer malformed. What comes after the block is for a later
    // version and is not looked at
    const UInt8* block = GUCEF_NULL;
    UInt32 blockSize = 0;
    if ( !reader.ReadSizedBytes( block, blockSize ) )
        return false;
    CDRNWireReader blockReader( block, blockSize );
    UInt32 fieldCount = 0;
    if ( !blockReader.ReadVarUInt32( fieldCount ) )
        return false;
    UInt32 values[ HelloLimitFieldCount ] = { 0, 0, 0, 0, 0 };
    for ( UInt32 i=0; i<fieldCount && i<HelloLimitFieldCount; ++i )
        if ( !blockReader.ReadVarUInt32( values[ i ] ) )
            return false;
    info.maxOpenHandles = values[ 0 ];
    info.maxActiveReads = values[ 1 ];
    info.maxNamespaceRequests = values[ 2 ];
    info.maxNamespaceListings = values[ 3 ];
    info.maxUploadBytesPerSecond = values[ 4 ];
    info.limitsAnnounced = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteShare( CDRNWireWriter& writer, const CDRNNamespaceShareInfo& share )
{GUCEF_TRACE;

    WriteText( writer, share.name );
    writer.WriteVarUInt32( share.capabilities );
    writer.WriteVarUInt32( share.flags );
    writer.WriteVarUInt32( share.maxStatPaths );
    writer.WriteVarUInt32( share.maxPageEntries );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadShare( CDRNWireReader& reader, CDRNNamespaceShareInfo& share )
{GUCEF_TRACE;

    return ReadText( reader, share.name ) && CDRNResourceName::IsValidProviderName( share.name ) && reader.ReadVarUInt32( share.capabilities ) &&
           reader.ReadVarUInt32( share.flags ) && reader.ReadVarUInt32( share.maxStatPaths ) && reader.ReadVarUInt32( share.maxPageEntries );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadCount( CDRNWireReader& reader, UInt32 minItemBytes, UInt32 maxCount, UInt32& count )
{GUCEF_TRACE;

    if ( !reader.ReadVarUInt32( count ) )
        return false;
    if ( count > maxCount )
        return false;
    return (UInt64) count * minItemBytes <= (UInt64) reader.GetRemaining();
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteEntryBody( CDRNWireWriter& writer, const CDRNNamespaceEntry& entry )
{GUCEF_TRACE;

    UInt32 flags = entry.flags & ~DRN_NS_ENTRYFLAG_SHA256;
    if ( entry.hasSha256 )
        flags |= DRN_NS_ENTRYFLAG_SHA256;
    writer.WriteUInt8( entry.kind );
    writer.WriteVarUInt64( entry.size );
    writer.WriteVarUInt64( entry.modifiedUtcMs );
    writer.WriteVarUInt32( flags );
    WriteText( writer, entry.revision );
    if ( entry.hasSha256 )
        writer.WriteBytes( entry.sha256, 32 );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadEntryBody( CDRNWireReader& reader, CDRNNamespaceEntry& entry )
{GUCEF_TRACE;

    UInt8 kind = 0;
    if ( !reader.ReadUInt8( kind ) || !reader.ReadVarUInt64( entry.size ) || !reader.ReadVarUInt64( entry.modifiedUtcMs ) ||
         !reader.ReadVarUInt32( entry.flags ) || !ReadText( reader, entry.revision ) )
        return false;
    if ( DRN_NS_KIND_FILE != kind && DRN_NS_KIND_DIRECTORY != kind )
        return false;
    if ( entry.revision.ByteSize() > CDRNNamespaceEntry::MaxRevisionBytes + 1 )
        return false;
    entry.kind = kind;
    entry.hasSha256 = 0 != ( entry.flags & DRN_NS_ENTRYFLAG_SHA256 );
    if ( entry.hasSha256 && !reader.ReadBytes( entry.sha256, 32 ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteStatItem( CDRNWireWriter& writer, const CDRNNamespaceEntry& entry )
{GUCEF_TRACE;

    writer.WriteVarUInt32( (UInt32) entry.status );
    if ( DRN_RESSTATUS_OK == entry.status )
        WriteEntryBody( writer, entry );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadStatItem( CDRNWireReader& reader, CDRNNamespaceEntry& entry )
{GUCEF_TRACE;

    UInt32 rawStatus = 0;
    if ( !reader.ReadVarUInt32( rawStatus ) )
        return false;
    entry.status = rawStatus < (UInt32) DRN_RESSTATUS_COUNT ? (TDRNResourceStatus) rawStatus : DRN_RESSTATUS_IO_ERROR;
    if ( DRN_RESSTATUS_OK == entry.status )
        return ReadEntryBody( reader, entry );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteListHeader( CDRNWireWriter& writer, const CORE::CString& dirRevision, const CORE::CString& nextCursor, UInt32 count )
{GUCEF_TRACE;

    WriteText( writer, dirRevision );
    WriteText( writer, nextCursor );
    writer.WriteVarUInt32( count );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadListHeader( CDRNWireReader& reader, CORE::CString& dirRevision, CORE::CString& nextCursor, UInt32& count )
{GUCEF_TRACE;

    if ( !ReadText( reader, dirRevision ) || !ReadText( reader, nextCursor ) )
        return false;
    // an entry is at least name length, name, kind, size, time, flags and revision length
    return ReadCount( reader, 7, 0x00FFFFFF, count );
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceCodec::WriteListEntry( CDRNWireWriter& writer, const CDRNNamespaceEntry& entry )
{GUCEF_TRACE;

    WriteText( writer, entry.name );
    WriteEntryBody( writer, entry );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceCodec::ReadListEntry( CDRNWireReader& reader, CDRNNamespaceEntry& entry )
{GUCEF_TRACE;

    if ( !ReadText( reader, entry.name ) )
        return false;

    // a single name of the listed directory the client may append to its own paths: nothing that climbs or names a drive
    if ( !CDRNResourceName::IsValidResourceName( entry.name ) || entry.name.HasChar( '/' ) >= 0 )
        return false;
    entry.status = DRN_RESSTATUS_OK;
    return ReadEntryBody( reader, entry );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceCodec::GetMaxListEntryBytes( void )
{GUCEF_TRACE;

    // name (length prefix and bytes), kind, size, time, flags, revision (length prefix and bytes), digest
    return 2 + CDRNResourceName::MaxResourceNameLength + 1 + 10 + 10 + 5 + 2 + CDRNNamespaceEntry::MaxRevisionBytes + 32;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
