/*
 *  vfspluginLIBARCHIVE: Generic GUCEF VFS plugin for archives handled by libarchive
 *  Copyright (C) 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_VFSPLUGIN_LIBARCHIVE_CLIBARCHIVEWRITER_H
#define GUCEF_VFSPLUGIN_LIBARCHIVE_CLIBARCHIVEWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CIARCHIVEWRITER_H
#include "gucefVFS_CIArchiveWriter.h"
#define GUCEF_VFS_CIARCHIVEWRITER_H
#endif /* GUCEF_VFS_CIARCHIVEWRITER_H ? */

#ifndef GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H
#include "vfspluginLIBARCHIVE_macros.h"
#define GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H
#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H ? */

struct archive;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace LIBARCHIVE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes archives using libarchive. Data is written to the output as entries are added.
 *  Formats whose headers need the size of an entry up front get sources of unknown size spooled to a
 *  temporary file first.
 */
class CLibArchiveWriter : public VFS::CIArchiveWriter
{
    public:

    enum EFormat
    {
        FORMAT_TAR = 0,     /**< ustar, with pax extended headers where needed (long or UTF-8 names, big files) */
        FORMAT_TAR_GZIP,    /**< as FORMAT_TAR, compressed with gzip */
        FORMAT_ZIP,         /**< zip, store or deflate per entry, ZIP64 where needed */
        FORMAT_7ZIP,        /**< 7-Zip using deflate */
        FORMAT_CPIO         /**< cpio in the SVR4 "newc" format */
    };

    CLibArchiveWriter( EFormat format );

    /** closes the archive if still open */
    virtual ~CLibArchiveWriter() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Open( CORE::CIOAccess& output ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AddFile( const CORE::CString& nameInArchive                ,
                          CORE::CIOAccess& source                           ,
                          const VFS::CArchiveWriterEntrySettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AddDirectory( const CORE::CString& nameInArchive      ,
                               const CORE::CDateTime& modificationTime ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsOpen( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetArchiveType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CORE::CString GetFormatTypeName( EFormat format );

    private:

    CLibArchiveWriter( const CLibArchiveWriter& src );            /**< not supported */
    CLibArchiveWriter& operator=( const CLibArchiveWriter& src ); /**< not supported */

    bool AddFileOfKnownSize( const CORE::CString& name                        ,
                             CORE::CIOAccess& source                          ,
                             UInt64 size                                      ,
                             const VFS::CArchiveWriterEntrySettings& settings );

    void LogError( const CORE::CString& operation ) const;

    EFormat m_format;
    struct archive* m_archive;
    CORE::CIOAccess* m_output;
};

/*-------------------------------------------------------------------------*/

/** A writer for one specific format, which allows creating the writers with a factory */
template < CLibArchiveWriter::EFormat format >
class CTLibArchiveFormatWriter : public CLibArchiveWriter
{
    public:

    CTLibArchiveFormatWriter( void )
        : CLibArchiveWriter( format )
    {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LIBARCHIVE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_CLIBARCHIVEWRITER_H ? */
