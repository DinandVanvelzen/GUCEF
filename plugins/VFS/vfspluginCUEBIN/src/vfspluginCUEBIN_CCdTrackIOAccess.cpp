/*
 *  vfspluginCUEBIN: Generic GUCEF VFS plugin for CD images described by a CUE sheet (BIN/CUE)
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

#include <stdio.h>
#include <string.h>

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#include "vfspluginCUEBIN_CCdTrackIOAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace CUEBIN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCdTrackIOAccess::SLayout::SLayout( void )
    : fileOffset( 0 )
    , sectorCount( 0 )
    , sectorSize( 0 )
    , payloadOffset( 0 )
    , payloadSize( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCdTrackIOAccess::CCdTrackIOAccess( const CORE::CString& imagePath, const SLayout& layout, const CORE::CDynamicBuffer& header )
    : CORE::CIOAccess()
    , m_imagePath( imagePath )
    , m_layout( layout )
    , m_header( header )
    , m_file()
    , m_size( header.GetDataSize() + layout.sectorCount * layout.payloadSize )
    , m_pos( 0 )
{GUCEF_TRACE;

    Open();
}

/*-------------------------------------------------------------------------*/

CCdTrackIOAccess::~CCdTrackIOAccess()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CCdTrackIOAccess::Open( void )
{GUCEF_TRACE;

    if ( m_file.IsNULL() )
    {
        CORE::IOAccessPtr file( GUCEF_NEW CORE::CFileAccess( m_imagePath, "rb" ) );
        if ( file->Opened() )
            m_file = file;
    }
    m_pos = 0;
}

/*-------------------------------------------------------------------------*/

void
CCdTrackIOAccess::Close( void )
{GUCEF_TRACE;

    m_file.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CCdTrackIOAccess::Opened( void ) const
{GUCEF_TRACE;

    return !m_file.IsNULL();
}

/*-------------------------------------------------------------------------*/

UInt32
CCdTrackIOAccess::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    if ( m_file.IsNULL() || GUCEF_NULL == dest || 0 == esize || 0 == elements || m_pos >= m_size )
        return 0;

    // whole elements only, as fread()
    const UInt64 elementsLeft = ( m_size - m_pos ) / esize;
    if ( elementsLeft < elements )
        elements = (UInt32) elementsLeft;
    if ( 0 == elements )
        return 0;

    UInt8* out = static_cast< UInt8* >( dest );
    UInt64 remaining = (UInt64) elements * esize;
    UInt64 done = 0;
    const UInt64 headerSize = m_header.GetDataSize();

    // the header first
    if ( m_pos < headerSize )
    {
        const UInt64 count = headerSize - m_pos < remaining ? headerSize - m_pos : remaining;
        memcpy( out, m_header.GetConstBufferPtr( (UInt32) m_pos ), (size_t) count );
        out += count;
        done += count;
        remaining -= count;
        m_pos += count;
    }

    // then the payload, a sector piece at a time; when the payload is the whole sector the run is contiguous
    while ( remaining > 0 )
    {
        const UInt64 payloadPos = m_pos - headerSize;
        const UInt64 sector = payloadPos / m_layout.payloadSize;
        const UInt64 inSector = payloadPos % m_layout.payloadSize;
        UInt64 count = m_layout.payloadSize - inSector;
        if ( m_layout.payloadSize == m_layout.sectorSize )
            count = remaining;
        if ( count > remaining )
            count = remaining;

        const UInt64 fileOffset = m_layout.fileOffset + sector * m_layout.sectorSize + m_layout.payloadOffset + inSector;
        if ( 0 != m_file->Setpos( fileOffset ) )
            break;
        const UInt32 got = m_file->Read( out, 1, (UInt32) count );
        out += got;
        done += got;
        remaining -= got;
        m_pos += got;
        if ( got < count )
            break;                          // the image is shorter than the sheet says
    }
    return (UInt32) ( done / esize );
}

/*-------------------------------------------------------------------------*/

UInt32
CCdTrackIOAccess::Write( const void* srcdata, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CCdTrackIOAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_pos;
}

/*-------------------------------------------------------------------------*/

UInt64
CCdTrackIOAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

Int32
CCdTrackIOAccess::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    Int64 target = 0;
    switch ( origin )
    {
        case SEEK_SET: target = offset; break;
        case SEEK_CUR: target = (Int64) m_pos + offset; break;
        case SEEK_END: target = (Int64) m_size + offset; break;
        default: return -1;
    }

    // as fseek(): before the start fails, beyond the end is allowed (nothing to read there)
    if ( target < 0 )
        return -1;
    m_pos = (UInt64) target;
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CCdTrackIOAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    m_pos = position;
    return 0;
}

/*-------------------------------------------------------------------------*/

char
CCdTrackIOAccess::GetChar( void )
{GUCEF_TRACE;

    char c = 0;
    if ( 1 == Read( &c, 1, 1 ) )
        return c;
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CCdTrackIOAccess::Eof( void ) const
{GUCEF_TRACE;

    return m_pos >= m_size;
}

/*-------------------------------------------------------------------------*/

bool
CCdTrackIOAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCdTrackIOAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCdTrackIOAccess::IsValid( void )
{GUCEF_TRACE;

    return !m_file.IsNULL() && 0 != m_layout.payloadSize && m_layout.payloadSize + m_layout.payloadOffset <= m_layout.sectorSize;
}

/*-------------------------------------------------------------------------*/

void
CCdTrackIOAccess::Flush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CCdTrackIOAccess::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CCdTrackIOAccess( m_imagePath, m_layout, m_header );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CUEBIN */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
