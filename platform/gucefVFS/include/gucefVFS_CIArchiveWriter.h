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

#ifndef GUCEF_VFS_CIARCHIVEWRITER_H
#define GUCEF_VFS_CIARCHIVEWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
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
 *  Settings for a single entry added to an archive
 */
class GUCEF_VFS_PUBLIC_CPP CArchiveWriterEntrySettings
{
    public:

    enum ECompression
    {
        COMPRESSION_STORE = 0,  /**< store the data as is */
        COMPRESSION_DEFLATE     /**< compress the data, if the archive format supports it */
    };

    /** defaults to storing without compression and the current time as modification time */
    CArchiveWriterEntrySettings( void );

    ECompression compression;
    Int32 compressionLevel;             /**< format specific, -1 for the default level */
    CORE::CDateTime modificationTime;
};

/*-------------------------------------------------------------------------*/

/**
 *  Interface for writing archives of a given format, for example zip or tar.
 *  Implementations are provided by plugins and created via CVFS::CreateArchiveWriter()
 *
 *  Data is written as it is added, an entry at a time, so archives of any size can be created.
 *  Names inside the archive use '/' as directory separator and are stored as UTF-8.
 *  Usage: Open(), any number of AddFile() / AddFileFromPath() / AddDirectory() calls, Close().
 *  The output has to remain available until Close() is called and has to support seeking.
 */
class GUCEF_VFS_PUBLIC_CPP CIArchiveWriter
{
    public:

    CIArchiveWriter( void );

    virtual ~CIArchiveWriter();

    /**
     *  Starts a new archive written to the given output
     */
    virtual bool Open( CORE::CIOAccess& output ) = 0;

    /**
     *  Adds an entry with the content read from the given source, from its current position until its end
     */
    virtual bool AddFile( const CORE::CString& nameInArchive           ,
                          CORE::CIOAccess& source                      ,
                          const CArchiveWriterEntrySettings& settings ) = 0;

    /**
     *  Adds an entry with the content of the given file, using the modification time of that file.
     *  The default implementation opens the file and uses AddFile()
     */
    virtual bool AddFileFromPath( const CORE::CString& nameInArchive           ,
                                  const CORE::CString& filePath                ,
                                  const CArchiveWriterEntrySettings& settings );

    /**
     *  Adds an empty directory entry
     */
    virtual bool AddDirectory( const CORE::CString& nameInArchive      ,
                               const CORE::CDateTime& modificationTime ) = 0;

    /**
     *  Finishes the archive. Nothing can be added afterwards
     */
    virtual bool Close( void ) = 0;

    virtual bool IsOpen( void ) const = 0;

    /**
     *  @return the archive type written, for example "zip"
     */
    virtual CORE::CString GetArchiveType( void ) const = 0;

    private:

    CIArchiveWriter( const CIArchiveWriter& src );            /**< not supported */
    CIArchiveWriter& operator=( const CIArchiveWriter& src ); /**< not supported */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CIARCHIVEWRITER_H ? */
