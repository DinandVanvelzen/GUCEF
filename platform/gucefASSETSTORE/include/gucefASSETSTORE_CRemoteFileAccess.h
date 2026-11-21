/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CREMOTEFILEACCESS_H
#define GUCEF_ASSETSTORE_CREMOTEFILEACCESS_H

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

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#include "gucefASSETSTORE_CRemoteAssetFileSystem.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A file of a remote asset store service for reading, as the VFS gives it (CRemoteAssetArchive::GetFile()): the reads go through the
 *  block cache of CRemoteAssetFileSystem. Positioning follows the f-functions: Setpos() and Seek() return 0 on success. The file system
 *  is found by its id at every call, so an access that outlives the mount reads nothing and says why. Not for writing.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CRemoteFileAccess : public CORE::CIOAccess
{
    public:

    /** Opens the file of the service path; check IsValid(): false when the file is not there or the service cannot be reached */
    CRemoteFileAccess( const CORE::CString& fileSystemId, const CORE::CString& servicePath );

    virtual ~CRemoteFileAccess() GUCEF_VIRTUAL_OVERRIDE;

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Read( void *dest      ,
                         UInt32 esize    ,
                         UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 Seek( Int64 offset ,
                        Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Setpos( UInt64 position ) GUCEF_VIRTUAL_OVERRIDE;

    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Ends the reads that wait for the service: they fail with a message of a cancel */
    void Cancel( void );

    /** The status of the last read: OK, or what went wrong; the message says it in plain words */
    TAssetStoreStatus GetLastStatus( void ) const;
    const CORE::CString& GetLastMessage( void ) const;

    private:

    CRemoteFileAccess( const CRemoteFileAccess& src );
    CRemoteFileAccess& operator=( const CRemoteFileAccess& src );

    private:

    CORE::CString m_fileSystemId;
    CORE::CString m_path;
    CRemoteAssetFileSystem::SOpenFile m_file;
    UInt64 m_position;
    bool m_opened;
    bool m_failureLogged;
    CORE::CString m_openMessage;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CREMOTEFILEACCESS_H ? */
