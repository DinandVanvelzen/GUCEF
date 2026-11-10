/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CFileSystemEntryListIterator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFileSystemEntryListIterator::CEntry::CEntry( void )
    : name()
    , path()
    , isDirectory( false )
    , isFile( false )
    , isSymlink( false )
    , hasMetaData( false )
    , metaData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemEntryListIterator::CFileSystemEntryListIterator( void )
    : IFileSystemIterator()
    , m_dirPath()
    , m_entries()
    , m_index( 0 )
    , m_iterating( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemEntryListIterator::CFileSystemEntryListIterator( const CString& dirPath, const TEntryVector& entries )
    : IFileSystemIterator()
    , m_dirPath( dirPath )
    , m_entries( entries )
    , m_index( 0 )
    , m_iterating( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemEntryListIterator::~CFileSystemEntryListIterator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CFileSystemEntryListIterator::SetEntries( const CString& dirPath, const TEntryVector& entries )
{GUCEF_TRACE;

    m_dirPath = dirPath;
    m_entries = entries;
    m_index = 0;
    m_iterating = false;
}

/*-------------------------------------------------------------------------*/

void
CFileSystemEntryListIterator::AddEntry( const CEntry& entry )
{GUCEF_TRACE;

    m_entries.push_back( entry );
}

/*-------------------------------------------------------------------------*/

const CFileSystemEntryListIterator::TEntryVector&
CFileSystemEntryListIterator::GetEntries( void ) const
{GUCEF_TRACE;

    return m_entries;
}

/*-------------------------------------------------------------------------*/

const CString&
CFileSystemEntryListIterator::GetDirPath( void ) const
{GUCEF_TRACE;

    return m_dirPath;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemEntryListIterator::TryReadAll( IFileSystemIterator& source, const CString& dirPath, TEntryVector& entries )
{GUCEF_TRACE;

    entries.clear();
    if ( !source.FindFirst( dirPath ) )
        return true;
    do
    {
        CEntry entry;
        entry.name = source.GetResourceName();
        entry.path = source.GetResourcePath();
        entry.isDirectory = source.IsADirectory();
        entry.isFile = source.IsAFile();
        entry.isSymlink = source.IsSymlink();
        entry.hasMetaData = source.TryReadMetaData( entry.metaData );
        entries.push_back( entry );
    }
    while ( source.FindNext() );
    source.FindClose();
    return true;
}

/*-------------------------------------------------------------------------*/

const CFileSystemEntryListIterator::CEntry*
CFileSystemEntryListIterator::GetCurrent( void ) const
{GUCEF_TRACE;

    if ( !m_iterating || m_index >= m_entries.size() )
        return GUCEF_NULL;
    return &m_entries[ m_index ];
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemEntryListIterator::FindFirst( const CString& dirPath )
{GUCEF_TRACE;

    m_index = 0;
    m_iterating = !m_entries.empty();
    return m_iterating;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemEntryListIterator::FindNext( void )
{GUCEF_TRACE;

    if ( !m_iterating )
        return false;
    ++m_index;
    if ( m_index >= m_entries.size() )
    {
        m_iterating = false;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemEntryListIterator::FindClose( void )
{GUCEF_TRACE;

    m_iterating = false;
    m_index = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemEntryListIterator::IsADirectory( void ) const
{GUCEF_TRACE;

    const CEntry* entry = GetCurrent();
    return GUCEF_NULL != entry && entry->isDirectory;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemEntryListIterator::IsAFile( void ) const
{GUCEF_TRACE;

    const CEntry* entry = GetCurrent();
    return GUCEF_NULL != entry && entry->isFile;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemEntryListIterator::IsSymlink( void ) const
{GUCEF_TRACE;

    const CEntry* entry = GetCurrent();
    return GUCEF_NULL != entry && entry->isSymlink;
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemEntryListIterator::GetResourceName( void ) const
{GUCEF_TRACE;

    const CEntry* entry = GetCurrent();
    return GUCEF_NULL != entry ? entry->name : CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
CFileSystemEntryListIterator::GetResourcePath( void ) const
{GUCEF_TRACE;

    const CEntry* entry = GetCurrent();
    return GUCEF_NULL != entry ? entry->path : CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemEntryListIterator::TryReadMetaData( CResourceMetaData& metaData )
{GUCEF_TRACE;

    const CEntry* entry = GetCurrent();
    if ( GUCEF_NULL == entry || !entry->hasMetaData )
        return false;
    metaData = entry->metaData;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
