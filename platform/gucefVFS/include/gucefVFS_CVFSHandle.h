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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#define GUCEF_VFS_CVFSHANDLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"         /* often used gucefVFS macros */
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  @TODO: Rework and rename this class
 */
class GUCEF_VFS_PUBLIC_CPP CVFSHandle
{
    public:

    typedef CORE::CTSharedPtr< CORE::CDynamicBuffer, MT::CMutex > TDynamicBufferPtr;

    CVFSHandle( CORE::IOAccessPtr fileAccess ,
                const CString& filename      ,
                const CString& filePath      );

    CVFSHandle( CORE::IOAccessPtr fileAccess ,
                const CString& filename      ,
                const CString& filePath      ,
                TDynamicBufferPtr& bufferPtr );

    ~CVFSHandle();

    CORE::IOAccessPtr GetAccess( void );

    const CString& GetFilename( void ) const;

    const CString& GetFilePath( void ) const;

    /**
     *  The full path within the VFS under which the resource was requested, like "/disc/data/file.res".
     *  This is set by the VFS when it hands out the resource, and is empty for resources that are
     *  created by other means. Unlike the file path, which each archive type fills in as it sees fit,
     *  this path can be used to find other resources in the VFS, for example files that sit next to it.
     */
    const CString& GetVfsPath( void ) const;

    void SetVfsPath( const CString& vfsPath );

    bool IsLoadedInMemory( void ) const;

    private:

    CVFSHandle( void );                              /**< not implemented */
    CVFSHandle( const CVFSHandle& src );             /**< not implemented */
    CVFSHandle& operator=( const CVFSHandle& src );  /**< not implemented */

    private:

    TDynamicBufferPtr m_bufferPtr;
    CORE::IOAccessPtr m_fileAccess;
    CString m_filename;
    CString m_filePath;
    CString m_vfsPath;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CVFSHandle, MT::CMutex >         TVfsResourcePtr;
typedef TVfsResourcePtr::TBasicSharedPtrBase                TBasicVfsResourcePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSHANDLE_H ? */
