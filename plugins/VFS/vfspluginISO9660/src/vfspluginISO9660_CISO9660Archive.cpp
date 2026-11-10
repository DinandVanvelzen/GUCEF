/*
 *  vfspluginISO9660: Generic GUCEF VFS plugin for ISO 9660 disc images
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CSUBIOACCESS_H
#include "gucefCORE_CSubIOAccess.h"
#define GUCEF_CORE_CSUBIOACCESS_H
#endif /* GUCEF_CORE_CSUBIOACCESS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "vfspluginISO9660_CISO9660Archive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace ISO9660 {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const VFS::CString CISO9660Archive::ISO9660ArchiveTypeName = "iso9660";

static const VFS::UInt32 SystemAreaSectors = 16;
static const VFS::UInt32 DescriptorSectorSize = 2048;
static const VFS::UInt32 MaxVolumeDescriptors = 64;
static const VFS::UInt32 MaxDirectoryDepth = 128;
static const VFS::UInt32 MaxEntries = 4000000;
static const VFS::UInt32 MaxDirectoryBytes = 64 * 1024 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** ISO 9660 stores most numbers twice, little endian first and then big endian: we read the little endian one */
static VFS::UInt32
ReadLE32( const VFS::UInt8* p )
{
    return (VFS::UInt32) p[ 0 ] | ( (VFS::UInt32) p[ 1 ] << 8 ) | ( (VFS::UInt32) p[ 2 ] << 16 ) | ( (VFS::UInt32) p[ 3 ] << 24 );
}

/*-------------------------------------------------------------------------*/

static VFS::UInt16
ReadLE16( const VFS::UInt8* p )
{
    return (VFS::UInt16) ( (VFS::UInt16) p[ 0 ] | ( (VFS::UInt16) p[ 1 ] << 8 ) );
}

/*-------------------------------------------------------------------------*/

CISO9660Archive::CISO9660Archive( void )
    : CArchive()
    , m_archiveName()
    , m_archivePath()
    , m_volumeId()
    , m_image()
    , m_imageResource()
    , m_blockSize( DescriptorSectorSize )
    , m_joliet( false )
    , m_rockRidge( false )
    , m_entries()
    , m_dirs()
    , m_imageLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CISO9660Archive::~CISO9660Archive()
{GUCEF_TRACE;

    UnloadArchive();
}

/*-------------------------------------------------------------------------*/

VFS::CString
CISO9660Archive::NormalizePath( const VFS::CString& path )
{GUCEF_TRACE;

    VFS::CString result = path.ReplaceChar( '\\', '/' );
    result = result.CompactRepeatingChar( '/' );
    while ( result.Length() > 0 && result[ 0 ] == '/' )
        result = result.CutChars( 1, true, 0 );
    while ( result.Length() > 0 && result[ result.Length()-1 ] == '/' )
        result = result.CutChars( 1, false, 0 );
    return result.Lowercase();
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::ReadBytes( VFS::UInt64 offset, void* dest, VFS::UInt32 byteCount ) const
{GUCEF_TRACE;

    if ( 0 == byteCount )
        return true;

    MT::CScopeMutex lock( m_imageLock );
    if ( m_image.IsNULL() )
        return false;

    if ( 0 != m_image->Setpos( offset ) )
        return false;
    return 1 == m_image->Read( dest, byteCount, 1 );
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CISO9660Archive::DecodeRecordingTime( const VFS::UInt8* b )
{GUCEF_TRACE;

    // years since 1900, month, day, hour, minute, second and the offset to UTC in steps of 15 minutes
    const CORE::Int16 timezoneOffsetInMins = (CORE::Int16) ( ( (CORE::Int8) b[ 6 ] ) * 15 );
    return CORE::CDateTime( (CORE::Int16) ( 1900 + b[ 0 ] ), b[ 1 ], b[ 2 ], (CORE::Int8) b[ 3 ], (CORE::Int8) b[ 4 ], (CORE::Int8) b[ 5 ], 0, timezoneOffsetInMins );
}

/*-------------------------------------------------------------------------*/

VFS::CString
CISO9660Archive::DecodeJolietName( const VFS::UInt8* name, VFS::UInt32 nameLength )
{GUCEF_TRACE;

    // UCS-2 big endian, in practice UTF-16 big endian since surrogate pairs do occur in the wild
    CORE::CUtf8String utf8;
    VFS::UInt32 i = 0;
    while ( i + 1 < nameLength )
    {
        VFS::UInt32 unit = ( (VFS::UInt32) name[ i ] << 8 ) | name[ i+1 ];
        i += 2;
        if ( unit >= 0xD800 && unit <= 0xDBFF && i + 1 < nameLength )
        {
            VFS::UInt32 low = ( (VFS::UInt32) name[ i ] << 8 ) | name[ i+1 ];
            if ( low >= 0xDC00 && low <= 0xDFFF )
            {
                i += 2;
                unit = 0x10000 + ( ( unit - 0xD800 ) << 10 ) + ( low - 0xDC00 );
            }
        }
        utf8.Append( unit );
    }

    // Joliet names can carry the version suffix as well
    const Int32 semicolon = utf8.HasChar( ';', false );
    if ( semicolon >= 0 && (UInt32) semicolon + 1 < utf8.Length() )
    {
        bool allDigits = true;
        for ( UInt32 d = (UInt32) semicolon + 1; d < utf8.Length(); ++d )
        {
            const UInt32 codePoint = utf8.CodepointAtIndex( d );
            allDigits = allDigits && codePoint >= '0' && codePoint <= '9';
        }
        if ( allDigits )
            utf8 = utf8.SubstrFromRange( 0, (UInt32) semicolon );
    }
    return CORE::ToString( utf8 );
}

/*-------------------------------------------------------------------------*/

VFS::CString
CISO9660Archive::CleanIsoName( const VFS::UInt8* name, VFS::UInt32 nameLength )
{GUCEF_TRACE;

    CORE::CUtf8String result( (const char*) name, nameLength );

    // Remove the file version: "NAME.EXT;1" becomes "NAME.EXT"
    const Int32 semicolon = result.HasChar( ';', false );
    if ( semicolon >= 0 )
        result = result.SubstrFromRange( 0, (UInt32) semicolon );

    // A file without extension is stored as "NAME." in the plain names
    if ( result.Length() > 1 && result.CodepointAtIndex( result.Length()-1 ) == '.' )
        result = result.CutChars( 1, false );

    return CORE::ToString( result );
}

/*-------------------------------------------------------------------------*/

VFS::CString
CISO9660Archive::ReadRockRidgeName( const VFS::UInt8* systemUse, VFS::UInt32 systemUseSize ) const
{GUCEF_TRACE;

    // The system use area is a sequence of entries: two character signature, length, version, data.
    // The "NM" entries hold the real name, in parts when it is long. A "CE" entry continues the area elsewhere.
    // The name is collected as bytes: a multi byte character can be split over two NM entries
    GUCEF::vector< char > name;
    bool foundName = false;
    GUCEF::vector< VFS::UInt8 > continuation;
    const VFS::UInt8* area = systemUse;
    VFS::UInt32 areaSize = systemUseSize;

    for ( VFS::UInt32 hops = 0; hops < 16; ++hops )
    {
        VFS::UInt32 pos = 0;
        const VFS::UInt8* nextArea = GUCEF_NULL;
        VFS::UInt32 nextAreaLba = 0, nextAreaOffset = 0, nextAreaSize = 0;
        bool hasNext = false;

        while ( pos + 4 <= areaSize )
        {
            const VFS::UInt8* e = area + pos;
            const VFS::UInt32 entryLen = e[ 2 ];
            if ( entryLen < 4 || pos + entryLen > areaSize )
                break;

            if ( e[ 0 ] == 'N' && e[ 1 ] == 'M' && entryLen >= 5 )
            {
                const VFS::UInt8 flags = e[ 4 ];
                // bit 1: current directory, bit 2: parent directory, neither has a name of its own
                if ( 0 == ( flags & 0x06 ) )
                {
                    name.insert( name.end(), (const char*) ( e + 5 ), (const char*) ( e + entryLen ) );
                    foundName = true;
                }
            }
            else
            if ( e[ 0 ] == 'C' && e[ 1 ] == 'E' && entryLen >= 28 )
            {
                nextAreaLba = ReadLE32( e + 4 );
                nextAreaOffset = ReadLE32( e + 12 );
                nextAreaSize = ReadLE32( e + 20 );
                hasNext = true;
            }
            else
            if ( e[ 0 ] == 'S' && e[ 1 ] == 'T' )
            {
                break;
            }
            pos += entryLen;
        }

        if ( !hasNext || nextAreaSize == 0 || nextAreaSize > m_blockSize * 4 )
            break;

        continuation.resize( nextAreaSize );
        if ( !ReadBytes( (VFS::UInt64) nextAreaLba * m_blockSize + nextAreaOffset, &continuation[ 0 ], nextAreaSize ) )
            break;
        area = &continuation[ 0 ];
        areaSize = nextAreaSize;
        (void) nextArea;
    }

    if ( foundName )
        return CORE::ToString( name.empty() ? CORE::CUtf8String() : CORE::CUtf8String( &name[ 0 ], (UInt32) name.size() ) );
    return VFS::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::ReadVolumeDescriptors( void )
{GUCEF_TRACE;

    const VFS::UInt64 imageSize = m_image->GetSize();

    // Raw CD images with sync patterns and error correction data in every sector are not understood
    VFS::UInt8 head[ 16 ];
    if ( ReadBytes( 0, head, sizeof( head ) ) )
    {
        static const VFS::UInt8 sync[ 12 ] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };
        if ( 0 == memcmp( head, sync, sizeof( sync ) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ISO9660Archive: The image is a raw 2352 byte sector image (.bin). Convert it to a plain .iso with 2048 byte sectors first" );
            return false;
        }
    }

    bool havePrimary = false;
    VFS::UInt8 primary[ DescriptorSectorSize ];
    bool haveJoliet = false;
    VFS::UInt8 joliet[ DescriptorSectorSize ];

    for ( VFS::UInt32 i = 0; i < MaxVolumeDescriptors; ++i )
    {
        const VFS::UInt64 offset = (VFS::UInt64) ( SystemAreaSectors + i ) * DescriptorSectorSize;
        if ( imageSize > 0 && offset + DescriptorSectorSize > imageSize )
            break;

        VFS::UInt8 sector[ DescriptorSectorSize ];
        if ( !ReadBytes( offset, sector, DescriptorSectorSize ) )
            break;
        if ( 0 != memcmp( sector + 1, "CD001", 5 ) )
            break;

        const VFS::UInt8 type = sector[ 0 ];
        if ( type == 255 )
            break;

        if ( type == 1 && !havePrimary )
        {
            memcpy( primary, sector, DescriptorSectorSize );
            havePrimary = true;
        }
        else
        if ( type == 2 && !haveJoliet )
        {
            // Joliet marks its supplementary descriptor with one of three escape sequences: %/@ (level 1), %/C (level 2), %/E (level 3)
            const VFS::UInt8* esc = sector + 88;
            if ( esc[ 0 ] == '%' && esc[ 1 ] == '/' && ( esc[ 2 ] == '@' || esc[ 2 ] == 'C' || esc[ 2 ] == 'E' ) )
            {
                memcpy( joliet, sector, DescriptorSectorSize );
                haveJoliet = true;
            }
        }
    }

    if ( !havePrimary )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ISO9660Archive: No ISO 9660 primary volume descriptor found, this is not an ISO 9660 image" );
        return false;
    }

    const VFS::UInt8* chosen = haveJoliet ? joliet : primary;
    m_joliet = haveJoliet;

    m_blockSize = ReadLE16( chosen + 128 );
    if ( m_blockSize != 512 && m_blockSize != 1024 && m_blockSize != 2048 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ISO9660Archive: Unsupported logical block size " << m_blockSize );
        return false;
    }

    // The volume identifier is space padded, in Joliet it is UCS-2
    if ( haveJoliet )
        m_volumeId = DecodeJolietName( chosen + 40, 32 );
    else
        m_volumeId = CORE::ToString( CORE::CUtf8String( (const char*) ( chosen + 40 ), 32 ) );
    m_volumeId = m_volumeId.Trim( true ).Trim( false );

    const VFS::UInt8* rootRecord = chosen + 156;
    const VFS::UInt32 rootExtent = ReadLE32( rootRecord + 2 );
    const VFS::UInt32 rootSize = ReadLE32( rootRecord + 10 );

    // Rock Ridge announces itself with an SP entry in the first system use area of the root. Only useful when there is no Joliet.
    if ( !m_joliet )
    {
        VFS::UInt8 rootDirHead[ 256 ];
        VFS::UInt32 headSize = rootSize < sizeof( rootDirHead ) ? rootSize : (VFS::UInt32) sizeof( rootDirHead );
        if ( headSize >= 34 && ReadBytes( (VFS::UInt64) rootExtent * m_blockSize, rootDirHead, headSize ) )
        {
            const VFS::UInt32 len = rootDirHead[ 0 ];
            const VFS::UInt32 nameLen = rootDirHead[ 32 ];
            VFS::UInt32 sua = 33 + nameLen;
            if ( 0 == ( nameLen % 2 ) )
                ++sua;
            if ( len <= headSize && sua + 7 <= len )
            {
                const VFS::UInt8* sp = rootDirHead + sua;
                m_rockRidge = sp[ 0 ] == 'S' && sp[ 1 ] == 'P' && sp[ 2 ] == 7 && sp[ 4 ] == 0xBE && sp[ 5 ] == 0xEF;
            }
        }
    }

    return ReadDirectoryTree( rootExtent, rootSize );
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::ReadDirectoryTree( VFS::UInt32 rootExtent, VFS::UInt32 rootSize )
{GUCEF_TRACE;

    GUCEF::vector< TDirToVisit > dirsToVisit;
    TDirToVisit root;
    root.extent = rootExtent;
    root.size = rootSize;
    dirsToVisit.push_back( root );
    m_dirs[ VFS::CString::Empty ] = TKeyVector();

    GUCEF::set< VFS::UInt32 > visited;

    while ( !dirsToVisit.empty() )
    {
        TDirToVisit dir = dirsToVisit.back();
        dirsToVisit.pop_back();

        // A well formed image has no loops, a damaged one might
        if ( visited.find( dir.extent ) != visited.end() )
            continue;
        visited.insert( dir.extent );

        if ( !ParseDirectory( dir, dirsToVisit ) )
            return false;

        if ( m_entries.size() > MaxEntries )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ISO9660Archive: The image has more entries than supported" );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::ParseDirectory( const TDirToVisit& dir, GUCEF::vector< TDirToVisit >& dirsToVisit )
{GUCEF_TRACE;

    if ( dir.size > MaxDirectoryBytes )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ISO9660Archive: Directory \"" << dir.path << "\" is unreasonably large (" << dir.size << " bytes)" );
        return false;
    }

    GUCEF::vector< VFS::UInt8 > data( dir.size );
    if ( dir.size > 0 && !ReadBytes( (VFS::UInt64) dir.extent * m_blockSize, &data[ 0 ], dir.size ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ISO9660Archive: Failed to read directory \"" << dir.path << "\" at block " << dir.extent );
        return false;
    }

    const VFS::CString dirKey = NormalizePath( dir.path );
    TKeyVector& children = m_dirs[ dirKey ];

    // The records of one multi extent file follow each other, this holds the one being assembled
    TEntry pending;
    bool havePending = false;

    VFS::UInt32 pos = 0;
    while ( pos < dir.size )
    {
        const VFS::UInt32 recLen = data[ pos ];
        if ( 0 == recLen )
        {
            // records never straddle a block boundary, the remainder of the block is padding
            pos = ( pos / m_blockSize + 1 ) * m_blockSize;
            continue;
        }
        if ( recLen < 34 || pos + recLen > dir.size )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ISO9660Archive: Corrupt directory record in \"" << dir.path << "\" at offset " << pos );
            return false;
        }

        const VFS::UInt8* rec = &data[ pos ];
        pos += recLen;

        const VFS::UInt32 extent = ReadLE32( rec + 2 );
        const VFS::UInt32 size = ReadLE32( rec + 10 );
        const VFS::UInt8 flags = rec[ 25 ];
        const VFS::UInt32 nameLen = rec[ 32 ];
        const VFS::UInt8* nameBytes = rec + 33;
        if ( 33 + nameLen > recLen )
            continue;

        // "." and ".." are encoded as a name of a single 0x00 or 0x01 byte
        if ( nameLen == 1 && ( nameBytes[ 0 ] == 0x00 || nameBytes[ 0 ] == 0x01 ) )
            continue;

        VFS::CString name;
        if ( m_joliet )
        {
            name = DecodeJolietName( nameBytes, nameLen );
        }
        else
        {
            if ( m_rockRidge )
            {
                VFS::UInt32 sua = 33 + nameLen;
                if ( 0 == ( nameLen % 2 ) )
                    ++sua;
                if ( sua < recLen )
                    name = ReadRockRidgeName( rec + sua, recLen - sua );
            }
            if ( name.IsNULLOrEmpty() )
                name = CleanIsoName( nameBytes, nameLen );
        }
        if ( name.IsNULLOrEmpty() )
            continue;

        const bool isDir = 0 != ( flags & 0x02 );
        const bool continues = 0 != ( flags & 0x80 );

        TEntry entry;
        entry.name = name;
        entry.isDir = isDir;
        entry.extent = extent;
        entry.size = size;
        entry.modifiedTime = DecodeRecordingTime( rec + 18 );

        if ( !isDir )
        {
            // join the extents of files that are stored in several parts
            if ( havePending )
            {
                if ( pending.name == name )
                {
                    const VFS::UInt64 pendingBlocks = ( pending.size + m_blockSize - 1 ) / m_blockSize;
                    if ( (VFS::UInt64) pending.extent + pendingBlocks != extent )
                        pending.readable = false; // not contiguous, we cannot serve that as one run
                    pending.size += size;
                    entry = pending;
                }
                else
                {
                    havePending = false; // the earlier record was never finished, drop it
                }
            }
            if ( continues )
            {
                pending = entry;
                havePending = true;
                continue;
            }
            havePending = false;
        }

        const VFS::CString path = dir.path.IsNULLOrEmpty() ? name : dir.path + "/" + name;
        const VFS::CString key = NormalizePath( path );
        if ( m_entries.find( key ) != m_entries.end() )
            continue; // a duplicate name, the first one wins

        m_entries[ key ] = entry;
        children.push_back( key );

        if ( isDir )
        {
            m_dirs[ key ] = TKeyVector();
            TDirToVisit sub;
            sub.extent = extent;
            sub.size = size;
            sub.path = path;
            dirsToVisit.push_back( sub );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::LoadArchive( const VFS::CString& archiveName ,
                              const VFS::CString& archivePath ,
                              CORE::IOAccessPtr imageAccess   )
{GUCEF_TRACE;

    if ( imageAccess.IsNULL() )
        return false;

    UnloadArchive();

    m_image = imageAccess;
    m_archiveName = archiveName;
    m_archivePath = archivePath;

    if ( !ReadVolumeDescriptors() )
    {
        UnloadArchive();
        return false;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ISO9660Archive: Loaded image \"" << m_archiveName << "\" volume \"" << m_volumeId << "\" with " << m_entries.size()
        << " entries, names from " << ( m_joliet ? "Joliet" : ( m_rockRidge ? "Rock Ridge" : "ISO 9660" ) ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::LoadArchive( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    // This is a read only format
    if ( settings.GetWriteableRequested() )
        return false;

    CORE::IOAccessPtr image( GUCEF_NEW CORE::CFileAccess( settings.GetActualArchivePath(), "rb" ) );
    if ( !image->Opened() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ISO9660Archive: Failed to open image file: " << settings.GetActualArchivePath() );
        return false;
    }
    return LoadArchive( settings.GetArchiveName(), settings.GetArchivePath(), image );
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::LoadArchive( const VFS::CString& archiveName       ,
                              VFS::TBasicVfsResourcePtr vfsResource ,
                              const bool writeableRequest           )
{GUCEF_TRACE;

    if ( writeableRequest || vfsResource.IsNULL() || vfsResource->GetAccess().IsNULL() )
        return false;

    if ( !LoadArchive( archiveName, vfsResource->GetFilePath(), vfsResource->GetAccess() ) )
        return false;

    // The resource owns the access object and perhaps more, so keep it for as long as we use the image
    m_imageResource = vfsResource;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::UnloadArchive( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_imageLock );

    m_entries.clear();
    m_dirs.clear();
    m_volumeId.Clear();
    m_joliet = false;
    m_rockRidge = false;
    m_blockSize = DescriptorSectorSize;
    m_image.Unlink();
    m_imageResource.Unlink();
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::IOAccessPtr
CISO9660Archive::GetImageAccessForReading( void ) const
{GUCEF_TRACE;

    // A file on disk gets its own access per file window, so concurrent readers do not disturb each other.
    // Other kinds of images (memory, descriptors, windows on other archives) cannot be copied cheaply and are shared.
    // The image can be any kind of access handed to us by the caller and nothing in it tells us it is a file, so
    // the check is a run time type check. Without RTTI every image is treated as one that is shared, which is
    // correct but the readers then take turns.
    #ifdef GUCEF_RTTI_IS_SUPPORTED
    CORE::CFileAccess* fileImage = dynamic_cast< CORE::CFileAccess* >( m_image.GetPointerAlways() );
    if ( GUCEF_NULL != fileImage )
    {
        CORE::CICloneable* clone = fileImage->Clone();
        if ( GUCEF_NULL != clone )
            return CORE::IOAccessPtr( static_cast< CORE::CIOAccess* >( clone ) );
    }
    #endif /* GUCEF_RTTI_IS_SUPPORTED ? */
    return m_image;
}

/*-------------------------------------------------------------------------*/

CORE::IOAccessPtr
CISO9660Archive::OpenEntry( const TEntry& entry, const VFS::UInt32 memLoadSize ) const
{GUCEF_TRACE;

    if ( entry.isDir || !entry.readable )
        return CORE::IOAccessPtr();

    const VFS::UInt64 offset = (VFS::UInt64) entry.extent * m_blockSize;

    if ( entry.size <= memLoadSize )
    {
        CORE::CDynamicBuffer* buffer = GUCEF_NEW CORE::CDynamicBuffer( (VFS::UInt32) entry.size, true );
        if ( entry.size > 0 )
        {
            if ( !ReadBytes( offset, buffer->GetBufferPtr(), (VFS::UInt32) entry.size ) )
            {
                GUCEF_DELETE buffer;
                return CORE::IOAccessPtr();
            }
        }
        buffer->SetDataSize( (VFS::UInt32) entry.size );
        return CORE::DynamicBufferAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( buffer, true ) );
    }

    CORE::IOAccessPtr imageForFile = GetImageAccessForReading();
    if ( imageForFile.IsNULL() )
        return CORE::IOAccessPtr();

    CORE::SubIOAccessPtr window( GUCEF_NEW CORE::CSubIOAccess() );
    if ( !window->Load( imageForFile, offset, entry.size ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ISO9660Archive: File \"" << entry.name << "\" lies outside of the image" );
        return CORE::IOAccessPtr();
    }
    return window;
}

/*-------------------------------------------------------------------------*/

const CISO9660Archive::TEntry*
CISO9660Archive::FindEntry( const VFS::CString& path ) const
{GUCEF_TRACE;

    TEntryMap::const_iterator i = m_entries.find( NormalizePath( path ) );
    if ( i != m_entries.end() )
        return &(*i).second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

VFS::TBasicVfsResourcePtr
CISO9660Archive::GetFile( const VFS::CString& file      ,
                          const char* mode              ,
                          const VFS::UInt32 memLoadSize ,
                          const bool overwrite          )
{GUCEF_TRACE;

    // read only
    if ( GUCEF_NULL == mode || *mode != 'r' )
        return VFS::TBasicVfsResourcePtr();

    const TEntry* entry = FindEntry( file );
    if ( GUCEF_NULL == entry || entry->isDir )
        return VFS::TBasicVfsResourcePtr();

    CORE::IOAccessPtr access = OpenEntry( *entry, memLoadSize );
    if ( access.IsNULL() )
        return VFS::TBasicVfsResourcePtr();

    CORE::CString filePath = m_archivePath;
    CORE::AppendToPath( filePath, file );
    return VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( access, entry->name, filePath ), this );
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::StoreAsFile( const CORE::CString& filepath    ,
                              const CORE::CDynamicBuffer& data ,
                              const CORE::UInt64 offset        ,
                              const bool overwrite             )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CISO9660Archive::AppendDirectoryListing( TStringVector& outputList                    ,
                                         const VFS::CString& dirKey                   ,
                                         const VFS::CString& relativePrefix           ,
                                         const VFS::CString& mountLocation            ,
                                         bool wantDirs                                ,
                                         bool recursive                               ,
                                         bool includePathInName                       ,
                                         const CORE::CString::StringSet& nameFilters  ,
                                         UInt32 maxListEntries                        ) const
{GUCEF_TRACE;

    TDirMap::const_iterator d = m_dirs.find( dirKey );
    if ( d == m_dirs.end() )
        return;

    const TKeyVector& children = (*d).second;
    for ( TKeyVector::const_iterator c = children.begin(); c != children.end(); ++c )
    {
        if ( outputList.size() >= maxListEntries )
            return;

        TEntryMap::const_iterator e = m_entries.find( *c );
        if ( e == m_entries.end() )
            continue;
        const TEntry& entry = (*e).second;

        const VFS::CString relativePath = relativePrefix.IsNULLOrEmpty() ? entry.name : relativePrefix + "/" + entry.name;

        if ( entry.isDir == wantDirs && VFS::CVFS::FilterValidation( entry.name, nameFilters ) )
        {
            VFS::CString listed = entry.name;
            if ( includePathInName )
            {
                listed = CORE::CombinePath( mountLocation, relativePath );
                listed = listed.ReplaceChar( '\\', '/' );
            }
            if ( wantDirs )
                listed += '/';
            outputList.push_back( listed );
        }

        if ( recursive && entry.isDir )
        {
            AppendDirectoryListing( outputList, *c, relativePath, mountLocation, wantDirs, recursive, includePathInName, nameFilters, maxListEntries );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CISO9660Archive::GetFileList( TStringVector& outputList                   ,
                              const CORE::CString& mountLocation          ,
                              const CORE::CString& archiveLocation        ,
                              bool recursive                              ,
                              bool includePathInFilename                  ,
                              const CORE::CString::StringSet& nameFilters ,
                              UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    const VFS::CString dirKey = NormalizePath( archiveLocation );
    AppendDirectoryListing( outputList, dirKey, archiveLocation, mountLocation, false, recursive, includePathInFilename, nameFilters, maxListEntries );
}

/*-------------------------------------------------------------------------*/

void
CISO9660Archive::GetDirList( TStringVector& outputList                   ,
                             const CORE::CString& mountLocation          ,
                             const CORE::CString& archiveLocation        ,
                             bool recursive                              ,
                             bool includeParentDirInName                 ,
                             const CORE::CString::StringSet& nameFilters ,
                             UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    const VFS::CString dirKey = NormalizePath( archiveLocation );
    AppendDirectoryListing( outputList, dirKey, archiveLocation, mountLocation, true, recursive, includeParentDirInName, nameFilters, maxListEntries );
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::DeleteFile( const VFS::CString& filePath )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::MoveFile( const VFS::CString& oldFilePath ,
                           const VFS::CString& newFilePath ,
                           const bool overwrite            )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::GetFileMetaData( const VFS::CString& filePath      ,
                                  CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    metaData.Clear();

    const TEntry* entry = FindEntry( filePath );
    if ( GUCEF_NULL == entry )
        return false;

    metaData.resourceExists = true;
    metaData.name = entry->name;
    metaData.hasName = true;
    metaData.modifiedDateTime = entry->modifiedTime;
    metaData.hasModifiedDateTime = true;
    metaData.isReadOnly = true;
    metaData.hasIsReadOnly = true;
    if ( !entry->isDir )
    {
        metaData.resourceSizeInBytes = entry->size;
        metaData.hasResourceSizeInBytes = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::FileExists( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    const TEntry* entry = FindEntry( filePath );
    return GUCEF_NULL != entry && !entry->isDir;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::DirExists( const VFS::CString& dirPath ) const
{GUCEF_TRACE;

    const VFS::CString key = NormalizePath( dirPath );
    if ( key.IsNULLOrEmpty() )
        return !m_image.IsNULL(); // the root

    const TEntry* entry = FindEntry( dirPath );
    return GUCEF_NULL != entry && entry->isDir;
}

/*-------------------------------------------------------------------------*/

VFS::UInt64
CISO9660Archive::GetFileSize( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    const TEntry* entry = FindEntry( filePath );
    if ( GUCEF_NULL != entry && !entry->isDir )
        return entry->size;
    return 0;
}

/*-------------------------------------------------------------------------*/

VFS::CString
CISO9660Archive::GetFileHash( const VFS::CString& file ) const
{GUCEF_TRACE;

    const TEntry* entry = FindEntry( file );
    if ( GUCEF_NULL == entry || entry->isDir )
        return VFS::CString::Empty;

    CORE::IOAccessPtr access = OpenEntry( *entry, 0 );
    if ( access.IsNULL() )
        return VFS::CString::Empty;

    VFS::UInt8 digest[ 16 ];
    if ( 0 != CORE::md5fromfile( access->CStyleAccess(), digest ) )
    {
        char md5Str[ 48 ];
        CORE::md5tostring( digest, md5Str );
        return VFS::CString( md5Str );
    }
    return VFS::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CISO9660Archive::GetFileModificationTime( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    const TEntry* entry = FindEntry( filePath );
    if ( GUCEF_NULL != entry )
        return entry->modifiedTime;
    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CISO9660Archive::GetArchiveName( void ) const
{GUCEF_TRACE;

    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CISO9660Archive::GetType( void ) const
{GUCEF_TRACE;

    return ISO9660ArchiveTypeName;
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CISO9660Archive::GetVolumeId( void ) const
{GUCEF_TRACE;

    return m_volumeId;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::UsesJolietNames( void ) const
{GUCEF_TRACE;

    return m_joliet;
}

/*-------------------------------------------------------------------------*/

bool
CISO9660Archive::UsesRockRidgeNames( void ) const
{GUCEF_TRACE;

    return m_rockRidge;
}

/*-------------------------------------------------------------------------*/

VFS::UInt32
CISO9660Archive::GetEntryCount( void ) const
{GUCEF_TRACE;

    return (VFS::UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

void
CISO9660Archive::DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != objectToBeDestroyed )
    {
        objectToBeDestroyed->GetAccess().Unlink();
        GUCEF_DELETE objectToBeDestroyed;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ISO9660 */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
