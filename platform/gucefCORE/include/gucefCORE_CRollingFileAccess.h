/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CROLLINGFILEACCESS_H
#define GUCEF_CORE_CROLLINGFILEACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>
#include <map>

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Simple implementation of a file that rolls over to a new file based on a set threshold
 *  Currently implemented threshold options:
 *      - size of the resource in bytes
 */
class GUCEF_CORE_PUBLIC_CPP CRollingFileAccess : public CIOAccess
{
    public:

    CRollingFileAccess( const CString& file     ,
                        const char* mode = "rb" );

    CRollingFileAccess( const CRollingFileAccess& src );

    CRollingFileAccess( void );

    virtual ~CRollingFileAccess() GUCEF_VIRTUAL_OVERRIDE;

    /**
      *      open the resource for I/O
      */
    virtual bool Open( const CString& file     ,
                       const char* mode = "rb" );

    /**
      *      open the resource for I/O
      */
    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      close the recource
        */
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      is the recource opened for reading ?
        */
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      read a line of text
        */
    virtual CString ReadLine( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      Reads a string from the recource
        */
    virtual CString ReadString( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      Attempts to read the specified number of bytes from the recourse
        */
    virtual UInt32 Read( void *dest      ,
                         UInt32 esize    ,
                         UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      Attempts to write the specified number of bytes to the recourse
        *      using srcdata as the data source.
        */
    virtual UInt32 Write( const void* srcdata ,
                          UInt32 esize        ,
                          UInt32 elements     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 Write( CIOAccess& sourceData   ,
                          Int64 bytesToWrite = -1 ) GUCEF_VIRTUAL_OVERRIDE;
        
    virtual UInt32 Write( const CString& string ,
                          bool addEol = true    ) GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      Get the current offset in bytes
        */
    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      Get the size of the resource in bytes
        */
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      jump to a different part of the recource
        */
    virtual Int32 Seek( Int64 offset ,
                        Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      jump to the given offset in the recource
        */
    virtual UInt32 Setpos( UInt64 position ) GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      Read a single character
        */
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      are we at the end of the recource ?
        */
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
        *      Is the access to the resource a valid one or
        *      has something gone wrong ?
        */
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool SetFileToUse( const CString& filename          ,
                       const char* mode = "rb"          ,
                       bool moveIfCurrentlyOpen = false );
    
    const CString& GetFilename( void ) const;

    void SetRolloverSizeThreshold( UInt64 sizeThreshold );

    UInt64 GetRolloverSizeThreshold( void ) const;

    void SetMaxRolloverFilesBeforeDeletion( Int32 deleteThreshold );

    Int32 GetMaxRolloverFilesBeforeDeletion( void ) const;

    private:

    typedef GUCEF::map< UInt64, CString > UInt64ToStringMap;

    CString GenerateCurrentFilename( const CString& baseName, UInt32& fileIndex, bool asWildcardFilter ) const;

    void GetExistingMatchingFiles( UInt64ToStringMap& files ) const;

    void EnforceFileSizeThreshold( void );

    void PerformRolledoverFilesCleanup( void );
    
    private:

    UInt64 m_maxFileSize;
    UInt32 m_currentFileIndex;
    mutable UInt64 m_offset;
    CString m_baseFilename;
    CString m_fileMode;
    CFileAccess m_currentFile;
    Int32 m_maxRolloverFilesBeforeDeletion;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CROLLINGFILEACCESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 25-08-2005 :
        - Replaced the read/write bools with a const char* for a mode.
          This allows for more options plus a more fopen() like parameter list
- 26-04-2005 :
        - Fixed an bug in Close(): Calling Close() twice caused an access violation
- 26-03-2005 :
        - Designed and implemented this class.

-----------------------------------------------------------------------------*/
