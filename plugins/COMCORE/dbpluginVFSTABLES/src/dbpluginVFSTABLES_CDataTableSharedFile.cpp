/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "dbpluginVFSTABLES_CDataTableSharedFile.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace VFSTABLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableSharedFile::CDataTableSharedFile( void )
    : resource()
    , access()
    , metaData()
    , readLock()
    , path()
    , size( 0 )
    , generation( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableSharedFile::~CDataTableSharedFile()
{GUCEF_TRACE;

    // the input goes before the resource that keeps it valid
    access.Unlink();
    metaData.Unlink();
    resource.Unlink();
}

/*-------------------------------------------------------------------------*/

CSharedFileReadAccess::CSharedFileReadAccess( const TDataTableSharedFilePtr& file )
    : CORE::CIOAccess()
    , m_file( file )
    , m_position( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSharedFileReadAccess::~CSharedFileReadAccess()
{GUCEF_TRACE;

    m_file.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CSharedFileReadAccess::Open( void )
{GUCEF_TRACE;

    m_position = 0;
}

/*-------------------------------------------------------------------------*/

void
CSharedFileReadAccess::Close( void )
{GUCEF_TRACE;

    m_file.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CSharedFileReadAccess::Opened( void ) const
{GUCEF_TRACE;

    return !m_file.IsNULL();
}

/*-------------------------------------------------------------------------*/

UInt32
CSharedFileReadAccess::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    if ( m_file.IsNULL() || GUCEF_NULL == dest || 0 == esize || 0 == elements )
        return 0;

    // whole elements only, within the file
    const UInt64 left = m_position < m_file->size ? m_file->size - m_position : 0;
    UInt64 wanted = (UInt64) esize * elements;
    if ( wanted > left )
        wanted = left - ( left % esize );
    if ( 0 == wanted )
        return 0;

    UInt32 elementsRead = 0;
    {
        MT::CScopeMutex lock( m_file->readLock );
        if ( 0 != m_file->access->Setpos( m_position ) )
            return 0;
        elementsRead = m_file->access->Read( dest, esize, (UInt32) ( wanted / esize ) );
    }
    m_position += (UInt64) elementsRead * esize;
    return elementsRead;
}

/*-------------------------------------------------------------------------*/

UInt32
CSharedFileReadAccess::Write( const void* srcdata, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CSharedFileReadAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

Int32
CSharedFileReadAccess::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    if ( m_file.IsNULL() )
        return -1;
    Int64 base = 0;
    switch ( origin )
    {
        case SEEK_SET: base = 0; break;
        case SEEK_CUR: base = (Int64) m_position; break;
        case SEEK_END: base = (Int64) m_file->size; break;
        default: return -1;
    }
    const Int64 target = base + offset;
    if ( target < 0 || (UInt64) target > m_file->size )
        return -1;
    m_position = (UInt64) target;
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CSharedFileReadAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    if ( m_file.IsNULL() || position > m_file->size )
        return 1;
    m_position = position;
    return 0;
}

/*-------------------------------------------------------------------------*/

char
CSharedFileReadAccess::GetChar( void )
{GUCEF_TRACE;

    char c = 0;
    if ( 1 == Read( &c, 1, 1 ) )
        return c;
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CSharedFileReadAccess::Eof( void ) const
{GUCEF_TRACE;

    return m_file.IsNULL() || m_position >= m_file->size;
}

/*-------------------------------------------------------------------------*/

bool
CSharedFileReadAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return !m_file.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CSharedFileReadAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSharedFileReadAccess::IsValid( void )
{GUCEF_TRACE;

    return !m_file.IsNULL() && !m_file->access.IsNULL();
}

/*-------------------------------------------------------------------------*/

UInt64
CSharedFileReadAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_file.IsNULL() ? 0 : m_file->size;
}

/*-------------------------------------------------------------------------*/

void
CSharedFileReadAccess::Flush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CSharedFileReadAccess::Clone( void ) const
{GUCEF_TRACE;

    CSharedFileReadAccess* clone = GUCEF_NEW CSharedFileReadAccess( m_file );
    clone->m_position = m_position;
    return clone;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
