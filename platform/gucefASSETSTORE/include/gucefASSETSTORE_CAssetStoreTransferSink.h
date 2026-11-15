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

#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFERSINK_H
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFERSINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */
#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */
#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H
#include "gucefASSETSTORE_CAssetStoreTransferContext.h"
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H ? */
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
 *  Where the body of an HTTP download goes: the ".part" file of the transfer context, written through the VFS.
 *
 *  The sink looks at the status of the response that the request is filling (the HTTP client sets it before the body
 *  arrives): the body of a 206 is appended to the bytes the file had, the body of a 200 replaces the file (the server
 *  did not resume), and the body of any other status (an error page) is not written. The file is opened at the first
 *  byte of a body that is kept. A write fails, which stops the request, when the context asks to stop.
 */
class GUCEF_ASSETSTORE_PRIVATE_CPP CAssetStoreTransferSink : public CORE::CIOAccess
{
    public:

    static const UInt32 SliceSize = 16384;     /**< bytes written between checks of the throttle and of a stop */

    CAssetStoreTransferSink( CAssetStoreTransferContext& context, const WEB::CHttpResponseData& response );

    virtual ~CAssetStoreTransferSink() GUCEF_VIRTUAL_OVERRIDE;

    using CORE::CIOAccess::Write;

    virtual UInt32 Write( const void* srcdata, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The bytes the ".part" file has, as far as this sink wrote them */
    UInt64 GetFileBytes( void ) const;

    /** The sink stopped the request because the context asked to stop */
    bool WasStopped( void ) const;

    /** A write to the file failed */
    bool HadWriteError( void ) const;

    /** The server sent the whole file instead of the rest: the file was started again */
    bool WasRestarted( void ) const;

    private:

    bool OpenFile( bool append );

    CAssetStoreTransferSink( const CAssetStoreTransferSink& src );              /**< not implemented */
    CAssetStoreTransferSink& operator=( const CAssetStoreTransferSink& src );   /**< not implemented */

    private:

    CAssetStoreTransferContext& m_context;
    const WEB::CHttpResponseData& m_response;
    VFS::TBasicVfsResourcePtr m_file;
    CORE::IOAccessPtr m_access;
    UInt64 m_fileBytes;
    bool m_opened;
    bool m_stopped;
    bool m_writeError;
    bool m_restarted;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFERSINK_H ? */
