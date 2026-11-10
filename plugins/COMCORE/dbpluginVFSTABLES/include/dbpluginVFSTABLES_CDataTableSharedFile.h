/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
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

#ifndef DBPLUGIN_VFSTABLES_CDATATABLESHAREDFILE_H
#define DBPLUGIN_VFSTABLES_CDATATABLESHAREDFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CITABLEFILEMETADATA_H
#include "gucefCORE_CITableFileMetaData.h"
#define GUCEF_CORE_CITABLEFILEMETADATA_H
#endif /* GUCEF_CORE_CITABLEFILEMETADATA_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef DBPLUGIN_VFSTABLES_MACROS_H
#include "dbpluginVFSTABLES_macros.h"
#define DBPLUGIN_VFSTABLES_MACROS_H
#endif /* DBPLUGIN_VFSTABLES_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace VFSTABLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A partition file opened once for a scan (section 14.6 of the data tables design): the VFS resource and its input,
 *  the metadata the scan plans with, and the lock its readers take around each positioned read. The open input is
 *  what keeps the version of the file the scan started on: on POSIX a replacement of the path does not reach it.
 *  Shared by the readers of the file through CSharedFileReadAccess; it closes with the last of them.
 */
class DBPLUGIN_VFSTABLES_PUBLIC_CPP CDataTableSharedFile
{
    public:

    VFS::TBasicVfsResourcePtr resource;         /**< keeps the input valid: a small file may be held in a buffer of the archive */
    CORE::IOAccessPtr access;
    CORE::CITableFileMetaDataPtr metaData;      /**< what the readers of the file use, as the scan planned it */
    MT::CMutex readLock;                        /**< held around every Setpos + Read of the input */
    CORE::CString path;                         /**< VFS path */
    UInt64 size;                                /**< size of the input when it was opened */
    UInt64 generation;                          /**< replace generation of the database when it was opened */

    CDataTableSharedFile( void );

    ~CDataTableSharedFile();

    private:

    CDataTableSharedFile( const CDataTableSharedFile& src );            /**< not implemented */
    CDataTableSharedFile& operator=( const CDataTableSharedFile& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CDataTableSharedFile, MT::CMutex >   TDataTableSharedFilePtr;

/*-------------------------------------------------------------------------*/

/**
 *  The input one reader of a shared file reads through: read only, with a position of its own. Every read is one
 *  Setpos + Read of the shared input under its lock, so readers on several threads share one open file, and the
 *  decoding of what they read happens outside the lock. Holds the shared file.
 */
class DBPLUGIN_VFSTABLES_PUBLIC_CPP CSharedFileReadAccess : public CORE::CIOAccess
{
    public:

    explicit CSharedFileReadAccess( const TDataTableSharedFilePtr& file );

    virtual ~CSharedFileReadAccess() GUCEF_VIRTUAL_OVERRIDE;

    /** Rewinds to the start */
    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Releases the shared file; reads fail afterwards */
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    /** Read only: always 0 */
    virtual UInt32 Write( const void* srcdata, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** 0 on success, as fseek; positions past the end are refused */
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;

    /** 0 on success, as fsetpos; positions past the end are refused */
    virtual UInt32 Setpos( UInt64 position ) GUCEF_VIRTUAL_OVERRIDE;

    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Another reader of the same shared file, at the same position */
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CSharedFileReadAccess( const CSharedFileReadAccess& src );              /**< not implemented */
    CSharedFileReadAccess& operator=( const CSharedFileReadAccess& src );   /**< not implemented */

    TDataTableSharedFilePtr m_file;
    UInt64 m_position;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_VFSTABLES_CDATATABLESHAREDFILE_H ? */
