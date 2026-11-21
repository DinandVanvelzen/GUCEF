/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MCPCI_CUPLOADSTORE_H
#define GUCEF_MCPCI_CUPLOADSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The snapshots that are on their way to a gateway: each upload is a file in the upload directory that grows by
 *  chunks of at most GetChunkBytes(), strictly in order, while its SHA-256 is computed as it grows. An upload that
 *  sees no chunk for the idle time is removed with its file. The number of uploads, their size and the chunk size
 *  are bounded, so are memory, files and disk.
 *
 *  An upload belongs to the machine that created it: other machines get UPLOAD_NOT_FOUND for it. One call at a time
 *  works on an upload: a second one gets UPLOAD_BUSY.
 *
 *  Thread-safe; the file work happens outside the lock.
 */
class CUploadStore
{
    public:

    enum EResult
    {
        UPLOAD_OK = 0,
        UPLOAD_NOT_FOUND,           /**< no such upload (of this machine) */
        UPLOAD_OFFSET_MISMATCH,     /**< the chunk does not start where the upload is: 'received' says where it is */
        UPLOAD_TOO_LARGE,           /**< more than the limit of an upload or of a chunk */
        UPLOAD_BUSY,                /**< another call works on the upload */
        UPLOAD_FULL,                /**< the most uploads at the same time are running */
        UPLOAD_INCOMPLETE,          /**< not all bytes are there yet */
        UPLOAD_HASH_MISMATCH,       /**< the bytes are not those of the SHA-256 the upload was created with */
        UPLOAD_BAD_REQUEST,         /**< an argument that cannot be right */
        UPLOAD_IO_ERROR             /**< the file could not be written */
    };

    /** The largest chunk there is */
    static const GUCEF::UInt32 MaxChunkBytes = 8 * 1024 * 1024;

    CUploadStore( void );

    ~CUploadStore();

    /** Where the files go; Start() makes it */
    void SetDirectory( const GUCEF::CORE::CString& directory );

    /**
     *  @param maxUploads       uploads at the same time
     *  @param maxUploadBytes   the size of one upload
     *  @param chunkBytes       the size of a chunk, at most MaxChunkBytes
     *  @param idleTimeoutSecs  an upload without a chunk for this long is removed
     */
    void SetLimits( GUCEF::UInt32 maxUploads, GUCEF::UInt64 maxUploadBytes, GUCEF::UInt32 chunkBytes, GUCEF::UInt32 idleTimeoutSecs );

    GUCEF::UInt32 GetChunkBytes( void ) const;

    GUCEF::UInt64 GetMaxUploadBytes( void ) const;

    /** Makes the directory, after removing it with what an earlier run left in it. @return false when it cannot be used */
    bool Start( GUCEF::CORE::CString& problem );

    /** Removes every upload and its file */
    void Stop( void );

    /** A new upload of 'size' bytes with the SHA-256 'sha256Hex' (64 hexadecimal digits) */
    EResult Create( const GUCEF::CORE::CString& ciId        ,
                    const GUCEF::CORE::CString& machineId   ,
                    GUCEF::UInt64 size                      ,
                    const GUCEF::CORE::CString& sha256Hex   ,
                    GUCEF::CORE::CString& uploadId          ,
                    GUCEF::CORE::CString& problem           );

    /** Adds a chunk at 'offset', which must be where the upload is. 'received' is where it is afterwards */
    EResult Write( const GUCEF::CORE::CString& uploadId     ,
                   const GUCEF::CORE::CString& machineId    ,
                   GUCEF::UInt64 offset                     ,
                   const GUCEF::CORE::CDynamicBuffer& data  ,
                   GUCEF::UInt64& received                  ,
                   GUCEF::CORE::CString& problem            );

    /** How far an upload is */
    EResult GetState( const GUCEF::CORE::CString& uploadId     ,
                      const GUCEF::CORE::CString& machineId    ,
                      GUCEF::UInt64& received                  ,
                      GUCEF::UInt64& size                      ,
                      GUCEF::CORE::CString& problem            );

    /**
     *  Checks that an upload is complete and has its SHA-256, and keeps others from it until EndCommit().
     *  @param path     its file
     *  @param ciId     the CI id it was created for
     */
    EResult BeginCommit( const GUCEF::CORE::CString& uploadId     ,
                         const GUCEF::CORE::CString& machineId    ,
                         GUCEF::CORE::CString& path               ,
                         GUCEF::CORE::CString& ciId               ,
                         GUCEF::CORE::CString& problem            );

    /** Ends a commit: the upload and its file are removed */
    void EndCommit( const GUCEF::CORE::CString& uploadId );

    /** Removes an upload of the machine and its file */
    EResult Remove( const GUCEF::CORE::CString& uploadId, const GUCEF::CORE::CString& machineId );

    /** Removes the uploads that saw nothing for the idle time. @return how many */
    GUCEF::UInt32 RemoveAbandoned( GUCEF::UInt64 nowInMs );

    GUCEF::UInt32 GetCount( void ) const;

    private:

    class CUpload
    {
        public:

        GUCEF::CORE::CString id;
        GUCEF::CORE::CString ciId;
        GUCEF::CORE::CString machineId;
        GUCEF::UInt64 size;
        GUCEF::CORE::CString sha256Hex;
        GUCEF::UInt64 received;
        GUCEF::CORE::CString path;
        GUCEF::UInt64 lastActivityInMs;
        bool busy;
        GUCEF::CORE::CSha256 hasher;

        CUpload( void );
    };
    typedef GUCEF::map< GUCEF::CORE::CString, CUpload* > TUploadMap;

    CUploadStore( const CUploadStore& src );                /**< not implemented */
    CUploadStore& operator=( const CUploadStore& src );     /**< not implemented */

    /** The upload of the machine, GUCEF_NULL when there is none; with the lock held */
    CUpload* Find( const GUCEF::CORE::CString& uploadId, const GUCEF::CORE::CString& machineId );

    void Delete( CUpload* upload );

    static GUCEF::UInt64 NowInMs( void );

    GUCEF::CORE::CString m_directory;
    GUCEF::UInt32 m_maxUploads;
    GUCEF::UInt64 m_maxUploadBytes;
    GUCEF::UInt32 m_chunkBytes;
    GUCEF::UInt32 m_idleTimeoutSecs;
    TUploadMap m_uploads;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CUPLOADSTORE_H ? */
