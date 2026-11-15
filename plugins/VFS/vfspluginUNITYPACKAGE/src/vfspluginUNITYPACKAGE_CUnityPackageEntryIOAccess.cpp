/*
 *  vfspluginUNITYPACKAGE: GUCEF VFS plugin presenting a Unity package as the project tree it contains
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

#include "archive_entry.h"

#include "vfspluginUNITYPACKAGE_CUnityPackageEntryIOAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace UNITYPACKAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityPackageEntryIOAccess::CUnityPackageEntryIOAccess( const CORE::CString& packagePath      ,
                                                        const CORE::IOAccessPtr& sourceAccess ,
                                                        UInt32 entryIndex                     ,
                                                        UInt64 entrySize                      )
    : CORE::CIOAccess()
    , m_packagePath( packagePath )
    , m_sourceAccess( sourceAccess )
    , m_entryIndex( entryIndex )
    , m_size( entrySize )
    , m_pos( 0 )
    , m_reader()
{GUCEF_TRACE;

    PositionAtEntry();
}

/*-------------------------------------------------------------------------*/

CUnityPackageEntryIOAccess::~CUnityPackageEntryIOAccess()
{GUCEF_TRACE;

    m_reader.Close();
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageEntryIOAccess::PositionAtEntry( void )
{GUCEF_TRACE;

    m_pos = 0;
    if ( !m_reader.Open( m_packagePath, m_sourceAccess ) )
        return false;

    struct archive_entry* entry = GUCEF_NULL;
    if ( !m_reader.SeekToEntry( m_entryIndex, &entry ) )
    {
        m_reader.Close();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageEntryIOAccess::SkipForward( UInt64 bytes )
{GUCEF_TRACE;

    char buffer[ 16 * 1024 ];
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
CUnityPackageEntryIOAccess::Open( void )
{GUCEF_TRACE;

    if ( !m_reader.IsOpen() || 0 != m_pos )
        PositionAtEntry();
}

/*-------------------------------------------------------------------------*/

void
CUnityPackageEntryIOAccess::Close( void )
{GUCEF_TRACE;

    m_reader.Close();
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageEntryIOAccess::Opened( void ) const
{GUCEF_TRACE;

    return m_reader.IsOpen();
}

/*-------------------------------------------------------------------------*/

UInt32
CUnityPackageEntryIOAccess::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    if ( !m_reader.IsOpen() || 0 == esize )
        return 0;

    // Read whole elements only, the package provides a byte stream
    UInt64 wanted = (UInt64) esize * elements;
    UInt64 left = m_size > m_pos ? m_size - m_pos : 0;
    if ( wanted > left )
        wanted = left - ( left % esize );

    UInt64 total = m_reader.ReadEntryData( dest, wanted );
    m_pos += total;
    return (UInt32) ( total / esize );
}

/*-------------------------------------------------------------------------*/

UInt64
CUnityPackageEntryIOAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_pos;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityPackageEntryIOAccess::Seek( Int64 offset, Int32 origin )
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

    if ( (UInt64) target < m_pos || !m_reader.IsOpen() )
    {
        // The package is a stream, start over from the beginning
        if ( !PositionAtEntry() )
            return -1;
    }
    return SkipForward( (UInt64) target - m_pos ) ? 0 : -1;
}

/*-------------------------------------------------------------------------*/

char
CUnityPackageEntryIOAccess::GetChar( void )
{GUCEF_TRACE;

    char c = 0;
    Read( &c, 1, 1 );
    return c;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageEntryIOAccess::Eof( void ) const
{GUCEF_TRACE;

    return m_pos >= m_size;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageEntryIOAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageEntryIOAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageEntryIOAccess::IsValid( void )
{GUCEF_TRACE;

    return m_reader.IsOpen();
}

/*-------------------------------------------------------------------------*/

void
CUnityPackageEntryIOAccess::Flush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CUnityPackageEntryIOAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

MT::CICloneable*
CUnityPackageEntryIOAccess::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CUnityPackageEntryIOAccess( m_packagePath, m_sourceAccess, m_entryIndex, m_size );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITYPACKAGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
