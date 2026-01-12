/*
 *  guidriverRmlUI: GUI backend using RmlUI
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_guidriverRmlUI_CRmlUIVFSINTERFACE_H
#define GUCEF_guidriverRmlUI_CRmlUIVFSINTERFACE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <RmlUI/Core.h>

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_guidriverRmlUI_MACROS_H
#include "guidriverRmlUI_macros.h"
#define GUCEF_guidriverRmlUI_MACROS_H
#endif /* GUCEF_guidriverRmlUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class which can be used to override RmlUI's basic C style file I/O with access to the GUCEF VFS
 */
class GUCEF_guidriverRmlUI_PUBLIC_CPP CRmlUIVFSInterface : public RmlUI::Core::FileInterface
{
    public:

	CRmlUIVFSInterface();

	virtual ~CRmlUIVFSInterface();

	/// Opens a file.
	/// @param file The file handle to write to.
	/// @return A valid file handle, or NULL on failure
	virtual RmlUI::Core::FileHandle Open( const RmlUI::Core::String& path );

	/// Closes a previously opened file.
	/// @param file The file handle previously opened through Open().
	virtual void Close( RmlUI::Core::FileHandle file );

	/// Reads data from a previously opened file.
	/// @param buffer The buffer to be read into.
	/// @param size The number of bytes to read into the buffer.
	/// @param file The handle of the file.
	/// @return The total number of bytes read into the buffer.
	virtual size_t Read( void* buffer, size_t size, RmlUI::Core::FileHandle file );

	/// Seeks to a point in a previously opened file.
	/// @param file The handle of the file to seek.
	/// @param offset The number of bytes to seek.
	/// @param origin One of either SEEK_SET (seek from the beginning of the file), SEEK_END (seek from the end of the file) or SEEK_CUR (seek from the current file position).
	/// @return True if the operation completed successfully, false otherwise.
	virtual bool Seek( RmlUI::Core::FileHandle file, long offset, int origin );

	/// Returns the current position of the file pointer.
	/// @param file The handle of the file to be queried.
	/// @return The number of bytes from the origin of the file.
	virtual size_t Tell( RmlUI::Core::FileHandle file );

	/// Called when this file interface is released.
	virtual void Release();

    private:

    RmlUI::Core::FileHandle FindOpenHandleSlot( void ) const;

    private:

    typedef std::map< RmlUI::Core::FileHandle, VFS::TBasicVfsResourcePtr > TFileHandleMap;
    typedef std::set< RmlUI::Core::FileHandle > THandleSet;

    VFS::CVFS* m_vfs;
    TFileHandleMap m_fileHandles;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_guidriverRmlUI_CRmlUIVFSINTERFACE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/

