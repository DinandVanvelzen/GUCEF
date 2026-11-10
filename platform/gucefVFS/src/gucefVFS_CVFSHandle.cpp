/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#include "gucefVFS_CVFSHandle.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CVFSHandle::CVFSHandle( CORE::IOAccessPtr fileAccess  ,
                        const CORE::CString& filename ,
                        const CORE::CString& filePath )
    : m_fileAccess( fileAccess ) ,
      m_filename( filename )     ,
      m_bufferPtr()              ,
      m_filePath( filePath )     ,
      m_vfsPath()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVFSHandle::CVFSHandle( CORE::IOAccessPtr fileAccess  ,
                        const CORE::CString& filename ,
                        const CORE::CString& filePath ,
                        TDynamicBufferPtr& bufferPtr  )
    : m_fileAccess( fileAccess ) ,
      m_filename( filename )     ,
      m_bufferPtr( bufferPtr )   ,
      m_filePath( filePath )     ,
      m_vfsPath()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVFSHandle::~CVFSHandle()
{GUCEF_TRACE;

    /* dont do anything here, this is just a storage / encapsulation class */
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVFSHandle::GetFilename( void ) const
{GUCEF_TRACE;

    return m_filename;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVFSHandle::GetFilePath( void ) const
{GUCEF_TRACE;

    return m_filePath;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVFSHandle::GetVfsPath( void ) const
{GUCEF_TRACE;

    return m_vfsPath;
}

/*-------------------------------------------------------------------------*/

void
CVFSHandle::SetVfsPath( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    m_vfsPath = vfsPath;
}

/*-------------------------------------------------------------------------*/

CORE::IOAccessPtr
CVFSHandle::GetAccess( void )
{GUCEF_TRACE;

    return m_fileAccess;
}

/*-------------------------------------------------------------------------*/

bool
CVFSHandle::IsLoadedInMemory( void ) const
{GUCEF_TRACE;

    return !m_bufferPtr.IsNULL();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
