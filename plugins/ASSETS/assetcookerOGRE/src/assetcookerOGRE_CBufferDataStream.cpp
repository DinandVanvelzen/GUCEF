/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "assetcookerOGRE_CBufferDataStream.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBufferDataStream::CBufferDataStream( void )
    : Ogre::DataStream( Ogre::DataStream::WRITE )
    , m_buffer()
    , m_position( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBufferDataStream::~CBufferDataStream()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

size_t
CBufferDataStream::read( void* buf, size_t count )
{GUCEF_TRACE;

    const size_t size = (size_t) m_buffer.GetDataSize();
    if ( m_position >= size )
        return 0;
    if ( count > size - m_position )
        count = size - m_position;
    m_buffer.CopyTo( (UInt32) m_position, (UInt32) count, buf );
    m_position += count;
    return count;
}

/*-------------------------------------------------------------------------*/

size_t
CBufferDataStream::write( const void* buf, size_t count )
{GUCEF_TRACE;

    if ( 0 == count )
        return 0;
    const UInt32 written = m_buffer.CopyFrom( (UInt32) m_position, (UInt32) count, buf );
    m_position += written;
    mSize = (size_t) m_buffer.GetDataSize();
    return written;
}

/*-------------------------------------------------------------------------*/

void
CBufferDataStream::skip( long count )
{GUCEF_TRACE;

    if ( count < 0 && (size_t) -count > m_position )
        m_position = 0;
    else
        m_position = (size_t) ( (long) m_position + count );
}

/*-------------------------------------------------------------------------*/

void
CBufferDataStream::seek( size_t pos )
{GUCEF_TRACE;

    m_position = pos;
}

/*-------------------------------------------------------------------------*/

size_t
CBufferDataStream::tell( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

bool
CBufferDataStream::eof( void ) const
{GUCEF_TRACE;

    return m_position >= (size_t) m_buffer.GetDataSize();
}

/*-------------------------------------------------------------------------*/

void
CBufferDataStream::close( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CBufferDataStream::GetBuffer( void ) const
{GUCEF_TRACE;

    return m_buffer;
}

/*-------------------------------------------------------------------------*/

bool
CBufferDataStream::StoreInVfs( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString folder = ASSETCONVERTER::CVfsPaths::FolderOf( vfsPath );
    if ( !folder.IsNULLOrEmpty() )
        vfs.CreateDir( folder );
    return vfs.StoreAsFile( vfsPath, m_buffer, 0, true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
