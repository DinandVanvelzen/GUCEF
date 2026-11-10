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

#ifndef VFSPLUGIN_ZIP_CZIPIOACCESSS_H
#define VFSPLUGIN_ZIP_CZIPIOACCESSS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_VFSPLUGIN_ZIP_MACROS_H
#include "vfspluginZIP_macros.h"
#define GUCEF_VFSPLUGIN_ZIP_MACROS_H
#endif /* GUCEF_VFSPLUGIN_ZIP_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

// Forward declaration for zziplib to avoid header file dependency.
typedef struct zzip_dir		ZZIP_DIR;
typedef struct zzip_file	ZZIP_FILE;

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

class CZipIOAccess : public CORE::CIOAccess
{
    public:
    
    /**
     *  @param zipLock the lock of the archive, which the zziplib calls of all its files take: they share its file handle.
     *                 The archive outlives its open files, as its zziplib dir does
     */
    CZipIOAccess( ZZIP_FILE* zzipFile     , 
                  size_t uncompressedSize ,
                  MT::CMutex& zipLock     );

    virtual ~CZipIOAccess();   

    /**
     *      open the resource for I/O
     */
    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      close the resource
     */
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      is the resource opened for reading ?
     */
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      read a line of text
     *      The delimiter is the platform text file line
     *      delimiter.
     */
    virtual CORE::CString ReadLine( void ) GUCEF_VIRTUAL_OVERRIDE;                               

    /**
     *      Reads a string from the resource
     */
    virtual CORE::CString ReadString( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to read the specified number of bytes from the resource
     */
    virtual CORE::UInt32 Read( void *dest            ,
                               CORE::UInt32 esize    ,
                               CORE::UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Get the current offset in bytes
     */
    virtual CORE::UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      jump to a different part of the resource
     */
    virtual CORE::Int32 Seek( CORE::Int64 offset ,
                              CORE::Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Read a single character
     */
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      are we at the end of the resource ?
     */
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    /**
     *      Can we read from this resource?
     */
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    /**
     *      Can we write to this resource?
     */
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;                 
    
    /**
     *      Is the access to the resource a valid one or
     *      has something gone wrong ?
     */
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;
    
    /**
     *  @return returns the size of the resource if possible.
     */
    virtual CORE::UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    /** 
     *  Flushes all outstanding mutations on the I/O device
     */
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    private:
    
    CZipIOAccess( void );
    CZipIOAccess( const CZipIOAccess& src );
    CZipIOAccess& operator=( const CZipIOAccess& src );
    
    private:
    
    ZZIP_FILE* m_zzipFile;
    size_t m_size;
    MT::CMutex& m_zipLock;
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

#endif /* VFSPLUGIN_ZIP_CZIPIOACCESSS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 04-05-2005 :
        - Dinand: Initial version.

-----------------------------------------------------------------------------*/
