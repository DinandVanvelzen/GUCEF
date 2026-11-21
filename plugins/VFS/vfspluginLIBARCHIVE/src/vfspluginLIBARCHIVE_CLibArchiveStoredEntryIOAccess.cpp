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

#include <stdio.h>

#include "vfspluginLIBARCHIVE_CLibArchiveStoredEntryIOAccess.h"

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

CLibArchiveStoredEntryIOAccess::CLibArchiveStoredEntryIOAccess( const CORE::CString& archivePath ,
                                                                UInt64 dataOffset                ,
                                                                UInt64 size                      )
    : CORE::CIOAccess()
    , m_archivePath( archivePath )
    , m_dataOffset( dataOffset )
    , m_size( size )
    , m_pos( 0 )
    , m_file()
{GUCEF_TRACE;

    Open();
}

/*-------------------------------------------------------------------------*/

CLibArchiveStoredEntryIOAccess::~CLibArchiveStoredEntryIOAccess()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveStoredEntryIOAccess::Open( void )
{GUCEF_TRACE;

    Close();
    m_pos = 0;
    if ( m_file.Open( m_archivePath, "rb" ) )
    {
        // never trust offsets that point outside the file
        if ( m_dataOffset > m_file.GetSize() || m_size > m_file.GetSize() - m_dataOffset || 0 != m_file.Seek( (Int64) m_dataOffset, SEEK_SET ) )
            m_file.Close();
    }
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveStoredEntryIOAccess::Close( void )
{GUCEF_TRACE;

    m_file.Close();
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveStoredEntryIOAccess::Opened( void ) const
{GUCEF_TRACE;

    return m_file.Opened();
}

/*-------------------------------------------------------------------------*/

UInt32
CLibArchiveStoredEntryIOAccess::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    if ( !m_file.Opened() || 0 == esize )
        return 0;

    // Whole elements only, and never past the end of the entry
    UInt64 wanted = (UInt64) esize * elements;
    const UInt64 left = m_size > m_pos ? m_size - m_pos : 0;
    if ( wanted > left )
        wanted = left - ( left % esize );
    if ( 0 == wanted )
        return 0;
    const UInt32 elementsRead = m_file.Read( dest, esize, (UInt32) ( wanted / esize ) );
    m_pos += (UInt64) elementsRead * esize;
    return elementsRead;
}

/*-------------------------------------------------------------------------*/

UInt64
CLibArchiveStoredEntryIOAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_pos;
}

/*-------------------------------------------------------------------------*/

Int32
CLibArchiveStoredEntryIOAccess::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    if ( !m_file.Opened() )
        return -1;

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
    if ( 0 != m_file.Seek( (Int64) ( m_dataOffset + (UInt64) target ), SEEK_SET ) )
        return -1;
    m_pos = (UInt64) target;
    return 0;
}

/*-------------------------------------------------------------------------*/

char
CLibArchiveStoredEntryIOAccess::GetChar( void )
{GUCEF_TRACE;

    char c = 0;
    Read( &c, 1, 1 );
    return c;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveStoredEntryIOAccess::Eof( void ) const
{GUCEF_TRACE;

    return m_pos >= m_size;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveStoredEntryIOAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveStoredEntryIOAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveStoredEntryIOAccess::IsValid( void )
{GUCEF_TRACE;

    return m_file.Opened();
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveStoredEntryIOAccess::Flush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CLibArchiveStoredEntryIOAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

MT::CICloneable*
CLibArchiveStoredEntryIOAccess::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CLibArchiveStoredEntryIOAccess( m_archivePath, m_dataOffset, m_size );
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
