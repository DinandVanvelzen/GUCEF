/*
 *  assetpluginUNREAL: Range access to the bytes of a package: a VFS file or a block of memory
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

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H
#include "assetpluginUNREAL_CUnrealPackageSource.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealPackageSource::CUnrealPackageSource( void )
    : m_name()
    , m_isMemory( false )
    , m_open( false )
    , m_size( 0 )
    , m_base( 0 )
    , m_memory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealPackageSource::~CUnrealPackageSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageSource::OpenVfs( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    m_open = false;
    m_isMemory = false;
    m_name = vfsPath;
    m_size = 0;
    m_base = 0;
    VFS::TBasicVfsResourcePtr resource = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( vfsPath, "rb" );
    if ( resource.IsNULL() || resource->GetAccess().IsNULL() )
        return false;
    CORE::IOAccessPtr access = resource->GetAccess();
    m_size = access->GetSize();
    if ( 0 == m_size )
        m_size = VFS::CVfsGlobal::Instance()->GetVfs().GetFileSize( vfsPath );
    m_open = m_size > 0;
    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageSource::OpenVfsRange( const CORE::CString& vfsPath, UInt64 offset, UInt64 size )
{GUCEF_TRACE;

    if ( !OpenVfs( vfsPath ) )
        return false;
    if ( offset > m_size || size > m_size - offset || 0 == size )
    {
        m_open = false;
        return false;
    }
    m_base = offset;
    m_size = size;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageSource::OpenMemory( const void* bytes, UInt32 size, const CORE::CString& name )
{GUCEF_TRACE;

    m_isMemory = true;
    m_base = 0;
    m_name = name;
    m_memory.Clear( true );
    if ( size > 0 )
        m_memory.CopyFrom( size, bytes );
    m_size = size;
    m_open = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageSource::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealPackageSource::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealPackageSource::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageSource::OpenRange( UInt64 offset, UInt64 size, CUnrealArchiveReader& reader, UInt32 windowBytes ) const
{GUCEF_TRACE;

    if ( !m_open || offset > m_size || size > m_size - offset )
        return false;
    if ( 0 == windowBytes )
        windowBytes = 1024 * 1024;
    // a small range does not need a big window
    if ( size < windowBytes )
        windowBytes = (UInt32) ( size < 4096 ? 4096 : size );

    if ( m_isMemory )
    {
        CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( &m_memory, false ) );
        return reader.Open( access, offset, offset + size, windowBytes );
    }

    VFS::TBasicVfsResourcePtr resource = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( m_name, "rb" );
    if ( resource.IsNULL() || resource->GetAccess().IsNULL() )
        return false;
    if ( !reader.Open( resource->GetAccess(), offset, offset + size, windowBytes, m_base ) )
        return false;
    reader.SetKeepAlive( resource );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
