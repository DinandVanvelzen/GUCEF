/*
 *  vfspluginLIBARCHIVE: Generic GUCEF VFS plugin for archives handled by libarchive
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

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "vfspluginLIBARCHIVE_CZipStoredEntryIndex.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace LIBARCHIVE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The most that is read for the directory of an archive: a limit against corrupt files */
static const UInt64 MaxDirectoryBytes = 256 * 1024 * 1024;

static UInt16
ReadU16( const UInt8* p )
{
    return (UInt16) ( p[ 0 ] | ( p[ 1 ] << 8 ) );
}

static UInt32
ReadU32( const UInt8* p )
{
    return (UInt32) p[ 0 ] | ( (UInt32) p[ 1 ] << 8 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 3 ] << 24 );
}

static UInt64
ReadU64( const UInt8* p )
{
    return (UInt64) ReadU32( p ) | ( (UInt64) ReadU32( p + 4 ) << 32 );
}

/** Reads exactly the given number of bytes at the offset */
static bool
ReadAt( CORE::CFileAccess& file, UInt64 offset, void* dest, UInt64 size )
{
    if ( 0 != size && ( 0 != file.Seek( (Int64) offset, SEEK_SET ) || size != file.Read( dest, 1, (UInt32) size ) ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CZipStoredEntryIndex::Load( const CORE::CString& archivePath ,
                            TEntryMap& entries               )
{GUCEF_TRACE;

    entries.clear();

    CORE::CFileAccess file;
    if ( !file.Open( archivePath, "rb" ) )
        return false;
    const UInt64 fileSize = file.GetSize();
    if ( fileSize < 22 )
        return false;

    // The end of central directory record is in the last 22 bytes plus the archive comment of up to 64 KiB
    const UInt64 tailSize = fileSize < 22 + 65535 ? fileSize : 22 + 65535;
    GUCEF::vector< UInt8 > tail( (size_t) tailSize );
    if ( !ReadAt( file, fileSize - tailSize, &tail[ 0 ], tailSize ) )
        return false;
    Int64 eocd = -1;
    for ( Int64 i=(Int64) tailSize-22; i>=0; --i )
    {
        const UInt8* p = &tail[ (size_t) i ];
        if ( 'P' == p[ 0 ] && 'K' == p[ 1 ] && 5 == p[ 2 ] && 6 == p[ 3 ] && (UInt64) i + 22 + ReadU16( p + 20 ) == tailSize )
        {
            eocd = i;
            break;
        }
    }
    if ( eocd < 0 )
        return false;

    const UInt8* record = &tail[ (size_t) eocd ];
    UInt64 entryCount = ReadU16( record + 10 );
    UInt64 directorySize = ReadU32( record + 12 );
    UInt64 directoryOffset = ReadU32( record + 16 );
    if ( 0xFFFF == entryCount || 0xFFFFFFFF == directorySize || 0xFFFFFFFF == directoryOffset )
    {
        // 64 bit sizes: the locator in front of the record names the zip64 record
        UInt8 locator[ 20 ];
        const UInt64 eocdOffset = fileSize - tailSize + (UInt64) eocd;
        if ( eocdOffset < 20 || !ReadAt( file, eocdOffset - 20, locator, 20 ) )
            return false;
        if ( 'P' != locator[ 0 ] || 'K' != locator[ 1 ] || 6 != locator[ 2 ] || 7 != locator[ 3 ] )
            return false;
        const UInt64 zip64Offset = ReadU64( locator + 8 );
        UInt8 zip64[ 56 ];
        if ( zip64Offset > fileSize - 56 || !ReadAt( file, zip64Offset, zip64, 56 ) )
            return false;
        if ( 'P' != zip64[ 0 ] || 'K' != zip64[ 1 ] || 6 != zip64[ 2 ] || 6 != zip64[ 3 ] )
            return false;
        entryCount = ReadU64( zip64 + 32 );
        directorySize = ReadU64( zip64 + 40 );
        directoryOffset = ReadU64( zip64 + 48 );
    }
    if ( directorySize > MaxDirectoryBytes || directoryOffset > fileSize || directorySize > fileSize - directoryOffset || entryCount > directorySize / 46 )
        return false;

    GUCEF::vector< UInt8 > directory( (size_t) directorySize );
    if ( directorySize > 0 && !ReadAt( file, directoryOffset, &directory[ 0 ], directorySize ) )
        return false;

    UInt64 at = 0;
    for ( UInt64 e=0; e<entryCount; ++e )
    {
        if ( at + 46 > directorySize )
            break;
        const UInt8* p = &directory[ (size_t) at ];
        if ( 'P' != p[ 0 ] || 'K' != p[ 1 ] || 1 != p[ 2 ] || 2 != p[ 3 ] )
            break;
        const UInt16 flags = ReadU16( p + 8 );
        const UInt16 method = ReadU16( p + 10 );
        UInt64 compressedSize = ReadU32( p + 20 );
        UInt64 size = ReadU32( p + 24 );
        const UInt16 nameLength = ReadU16( p + 28 );
        const UInt16 extraLength = ReadU16( p + 30 );
        const UInt16 commentLength = ReadU16( p + 32 );
        UInt64 headerOffset = ReadU32( p + 42 );
        const UInt64 recordSize = 46 + (UInt64) nameLength + extraLength + commentLength;
        if ( at + recordSize > directorySize )
            break;
        at += recordSize;

        // the 64 bit values come in the extra field, in this order, for the 32 bit values that are all ones
        const UInt8* extra = p + 46 + nameLength;
        for ( UInt32 x=0; x+4<=extraLength; )
        {
            const UInt16 id = ReadU16( extra + x );
            const UInt16 length = ReadU16( extra + x + 2 );
            if ( x + 4 + length > extraLength )
                break;
            if ( 1 == id )
            {
                UInt32 used = x + 4;
                if ( 0xFFFFFFFF == size && used + 8 <= x + 4 + length )    { size = ReadU64( extra + used ); used += 8; }
                if ( 0xFFFFFFFF == compressedSize && used + 8 <= x + 4 + length ) { compressedSize = ReadU64( extra + used ); used += 8; }
                if ( 0xFFFFFFFF == headerOffset && used + 8 <= x + 4 + length )   { headerOffset = ReadU64( extra + used ); }
            }
            x += 4 + length;
        }

        // only data that is not transformed in any way can be read directly
        if ( 0 != method || 0 != ( flags & 0x0041 ) || compressedSize != size || 0 == nameLength || '/' == p[ 46 + nameLength - 1 ] )
            continue;

        // the data follows the local header, which has a name and extra field of its own
        UInt8 local[ 30 ];
        if ( headerOffset > directoryOffset || directoryOffset - headerOffset < 30 || !ReadAt( file, headerOffset, local, 30 ) )
            continue;
        if ( 'P' != local[ 0 ] || 'K' != local[ 1 ] || 3 != local[ 2 ] || 4 != local[ 3 ] )
            continue;
        const UInt64 dataOffset = headerOffset + 30 + ReadU16( local + 26 ) + ReadU16( local + 28 );
        if ( dataOffset > directoryOffset || size > directoryOffset - dataOffset )
            continue;

        SStoredEntry entry;
        entry.dataOffset = dataOffset;
        entry.size = size;
        entries[ CORE::CString( reinterpret_cast< const char* >( p + 46 ), nameLength ) ] = entry;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LIBARCHIVE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
