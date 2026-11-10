/*
 *  vfspluginLIBARCHIVE: Generic GUCEF VFS plugin for archives handled by libarchive
 *  Copyright (C) 2026.  Dinand Vanvelzen
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
#include <stdio.h>

#include "archive.h"
#include "archive_entry.h"

#include "vfspluginLIBARCHIVE_CUtf8LocaleScope.h"

#include "vfspluginLIBARCHIVE_CLibArchiveArchive.h"
#include "vfspluginLIBARCHIVE_CLibArchiveEntryIOAccess.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLibArchiveEntryIOAccess::CLibArchiveEntryIOAccess( const CORE::CString& archivePath ,
                                                    UInt32 entryIndex                ,
                                                    UInt64 entrySize                 )
    : CORE::CIOAccess()
    , m_archivePath( archivePath )
    , m_entryIndex( entryIndex )
    , m_size( entrySize )
    , m_pos( 0 )
    , m_archive( GUCEF_NULL )
{GUCEF_TRACE;

    PositionAtEntry();
}

/*-------------------------------------------------------------------------*/

CLibArchiveEntryIOAccess::~CLibArchiveEntryIOAccess()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveEntryIOAccess::Release( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_archive )
    {
        archive_read_free( m_archive );
        m_archive = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveEntryIOAccess::PositionAtEntry( void )
{GUCEF_TRACE;

    CUtf8LocaleScope utf8Locale;

    Release();
    m_pos = 0;

    m_archive = CLibArchiveArchive::OpenForReading( m_archivePath );
    if ( GUCEF_NULL == m_archive )
        return false;

    // Skip the entries before ours, with a seekable file this skips the data without reading it
    struct archive_entry* entry = GUCEF_NULL;
    for ( UInt32 i=0; i<=m_entryIndex; ++i )
    {
        int result = archive_read_next_header( m_archive, &entry );
        if ( ARCHIVE_OK != result && ARCHIVE_WARN != result )
        {
            Release();
            return false;
        }
        if ( i < m_entryIndex )
            archive_read_data_skip( m_archive );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveEntryIOAccess::SkipForward( UInt64 bytes )
{GUCEF_TRACE;

    char buffer[ 64 * 1024 ];
    while ( bytes > 0 )
    {
        UInt32 chunk = bytes > sizeof( buffer ) ? (UInt32) sizeof( buffer ) : (UInt32) bytes;
        UInt32 bytesRead = Read( buffer, 1, chunk );
        if ( 0 == bytesRead )
            return false;
        bytes -= bytesRead;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveEntryIOAccess::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_archive || 0 != m_pos )
        PositionAtEntry();
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveEntryIOAccess::Close( void )
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveEntryIOAccess::Opened( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_archive;
}

/*-------------------------------------------------------------------------*/

UInt32
CLibArchiveEntryIOAccess::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    CUtf8LocaleScope utf8Locale;

    if ( GUCEF_NULL == m_archive || 0 == esize )
        return 0;

    // Read whole elements only, the archive provides a byte stream
    UInt64 wanted = (UInt64) esize * elements;
    UInt64 left = m_size > m_pos ? m_size - m_pos : 0;
    if ( wanted > left )
        wanted = left - ( left % esize );

    char* out = static_cast< char* >( dest );
    UInt64 total = 0;
    while ( total < wanted )
    {
        la_ssize_t bytesRead = archive_read_data( m_archive, out + total, (size_t) ( wanted - total ) );
        if ( bytesRead <= 0 )
            break;
        total += (UInt64) bytesRead;
    }
    m_pos += total;
    return (UInt32) ( total / esize );
}

/*-------------------------------------------------------------------------*/

UInt64
CLibArchiveEntryIOAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_pos;
}

/*-------------------------------------------------------------------------*/

Int32
CLibArchiveEntryIOAccess::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    Int64 target = 0;
    switch ( origin )
    {
        case SEEK_SET: target = offset; break;
        case SEEK_CUR: target = (Int64) m_pos + offset; break;
        case SEEK_END: target = (Int64) m_size + offset; break;
        default: return -1;
    }
    if ( target < 0 || (UInt64) target > m_size )
        return -1;

    if ( (UInt64) target < m_pos || GUCEF_NULL == m_archive )
    {
        // The archive is a stream, start over from the beginning of the entry
        if ( !PositionAtEntry() )
            return -1;
    }
    return SkipForward( (UInt64) target - m_pos ) ? 0 : -1;
}

/*-------------------------------------------------------------------------*/

char
CLibArchiveEntryIOAccess::GetChar( void )
{GUCEF_TRACE;

    char c = 0;
    Read( &c, 1, 1 );
    return c;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveEntryIOAccess::Eof( void ) const
{GUCEF_TRACE;

    return m_pos >= m_size;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveEntryIOAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveEntryIOAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveEntryIOAccess::IsValid( void )
{GUCEF_TRACE;

    return GUCEF_NULL != m_archive;
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveEntryIOAccess::Flush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CLibArchiveEntryIOAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

MT::CICloneable*
CLibArchiveEntryIOAccess::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CLibArchiveEntryIOAccess( m_archivePath, m_entryIndex, m_size );
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
