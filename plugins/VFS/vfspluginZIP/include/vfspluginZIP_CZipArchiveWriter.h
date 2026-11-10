/*
 *  vfspluginZIP: Generic GUCEF VFS plugin for ZIP archives
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef VFSPLUGIN_ZIP_CZIPARCHIVEWRITER_H
#define VFSPLUGIN_ZIP_CZIPARCHIVEWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CIARCHIVEWRITER_H
#include "gucefVFS_CIArchiveWriter.h"
#define GUCEF_VFS_CIARCHIVEWRITER_H
#endif /* GUCEF_VFS_CIARCHIVEWRITER_H ? */

#ifndef GUCEF_VFSPLUGIN_ZIP_MACROS_H
#include "vfspluginZIP_macros.h"
#define GUCEF_VFSPLUGIN_ZIP_MACROS_H
#endif /* GUCEF_VFSPLUGIN_ZIP_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace ZIP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes ZIP archives using minizip.
 *  ZIP64 is used automatically where needed: for entries of 4 GiB or more, entries of unknown size,
 *  archives larger than 4 GiB and archives with 65535 or more entries.
 *  Names are stored as UTF-8. Besides the DOS date each entry carries its modification time in UTC
 *  with a resolution of seconds in an extended timestamp field, plus unix style permissions.
 */
class CZipArchiveWriter : public VFS::CIArchiveWriter
{
    public:

    CZipArchiveWriter( void );

    /** closes the archive if still open */
    virtual ~CZipArchiveWriter() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Open( CORE::CIOAccess& output ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AddFile( const CORE::CString& nameInArchive                ,
                          CORE::CIOAccess& source                           ,
                          const VFS::CArchiveWriterEntrySettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AddDirectory( const CORE::CString& nameInArchive      ,
                               const CORE::CDateTime& modificationTime ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsOpen( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetArchiveType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool OpenEntry( const CORE::CString& nameInArchive          ,
                    const CORE::CDateTime& modificationTime     ,
                    bool isDirectory                            ,
                    int method                                  ,
                    int level                                   ,
                    bool needsZip64                             );

    void* m_zipFile;
    CORE::CIOAccess* m_output;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ZIP */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* VFSPLUGIN_ZIP_CZIPARCHIVEWRITER_H ? */
