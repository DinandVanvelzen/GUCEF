/*
 *  guidriverMyGUI: GUI backend using MyGUI
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "guidriverMyGUI_CMyGUIVfsDataStream.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMyGUIVfsDataStream::CMyGUIVfsDataStream( const CORE::CString& vfsPath )
    : MyGUI::IDataStream()
    , m_buffer()
    , m_position( 0 )
    , m_isValid( false )
{GUCEF_TRACE;

    m_isValid = VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( m_buffer, vfsPath );
}

/*-------------------------------------------------------------------------*/

CMyGUIVfsDataStream::CMyGUIVfsDataStream( const void* data, size_t size )
    : MyGUI::IDataStream()
    , m_buffer()
    , m_position( 0 )
    , m_isValid( true )
{GUCEF_TRACE;

    if ( size > 0 )
        m_buffer.CopyFrom( (UInt32) size, data );
}

/*-------------------------------------------------------------------------*/

CMyGUIVfsDataStream::~CMyGUIVfsDataStream()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMyGUIVfsDataStream::IsValid( void ) const
{GUCEF_TRACE;

    return m_isValid;
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIVfsDataStream::eof()
{GUCEF_TRACE;

    return m_position >= m_buffer.GetDataSize();
}

/*-------------------------------------------------------------------------*/

size_t
CMyGUIVfsDataStream::size()
{GUCEF_TRACE;

    return m_buffer.GetDataSize();
}

/*-------------------------------------------------------------------------*/

void
CMyGUIVfsDataStream::readline( std::string& source, MyGUI::Char delimiter )
{GUCEF_TRACE;

    source.clear();
    const char* data = static_cast< const char* >( m_buffer.GetConstBufferPtr() );
    const size_t total = m_buffer.GetDataSize();
    while ( m_position < total )
    {
        char c = data[ m_position++ ];
        if ( c == delimiter )
            break;
        source += c;
    }
}

/*-------------------------------------------------------------------------*/

size_t
CMyGUIVfsDataStream::read( void* buffer, size_t count )
{GUCEF_TRACE;

    const size_t total = m_buffer.GetDataSize();
    if ( m_position >= total || 0 == count )
        return 0;
    size_t available = total - m_position;
    if ( count > available )
        count = available;
    memcpy( buffer, static_cast< const char* >( m_buffer.GetConstBufferPtr() ) + m_position, count );
    m_position += count;
    return count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
