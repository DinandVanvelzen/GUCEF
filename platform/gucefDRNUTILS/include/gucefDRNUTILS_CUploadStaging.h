/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CUPLOADSTAGING_H
#define GUCEF_DRNUTILS_CUPLOADSTAGING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRNUTILS_CUPLOADRANGESET_H
#include "gucefDRNUTILS_CUploadRangeSet.h"
#define GUCEF_DRNUTILS_CUPLOADRANGESET_H
#endif /* GUCEF_DRNUTILS_CUPLOADRANGESET_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The bounded staging of uploads of a remote service: a client announces an upload (its files with sizes and SHA-256
 *  digests), writes blocks in any order and in as many sessions as it needs, asks which ranges arrived, and commits; the
 *  digests are checked at the commit and the service then takes the staged folder (a later command refers to the upload
 *  by its id). An upload is bound to its subject (the device): another subject sees UPLOAD_UNKNOWN.
 *  Layout: "<directory>/<uploadId>/files/<relative path>" and "<directory>/<uploadId>/upload.json" (the manifest with the
 *  received ranges, written atomically after every WriteBlock() that completed a block of blockSize or a file, and at
 *  state changes), so an upload survives a restart of the service. Relative paths are checked: '/' separated, no empty,
 *  "." or ".." segments, no absolute path, drive letter, backslash, control character or reserved device name.
 *  Bounds (0 = no maximum): staging bytes per subject and in total (the announced sizes count from Begin()), uploads open
 *  per subject, files per upload, bytes per file; an upload that is not committed within expiryMs of its last write is
 *  removed by ExpireOld(). Times: the wall clock in UTC milliseconds given by the caller (the manifest survives restarts).
 *  Commit() reads every file to check its digest: on a large upload that is slow, so a service calls it on a pool task.
 *  Thread safe; file I/O happens outside the lock except for the manifest of the upload in hand.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CUploadStaging
{
    public:

    enum EUploadState
    {
        UPLOADSTATE_RECEIVING = 0 ,
        UPLOADSTATE_COMMITTED ,         /**< digests checked; the folder waits for the service to take it (Release()) */
        UPLOADSTATE_ABORTED
    };

    struct GUCEF_DRNUTILS_PUBLIC_CPP SSettings
    {
        CORE::CString directory;
        UInt64 maxBytesPerSubject;      /**< default 50 GiB */
        UInt64 maxTotalBytes;           /**< default 0 */
        UInt32 maxUploadsPerSubject;    /**< default 1 */
        UInt32 maxFilesPerUpload;       /**< default 100000 */
        UInt64 maxFileBytes;            /**< default 0 */
        UInt64 expiryMs;                /**< default 48 h */
        UInt32 blockSize;               /**< default 262144 */

        SSettings( void );
    };

    struct GUCEF_DRNUTILS_PUBLIC_CPP SFileSpec
    {
        CORE::CString relativePath;
        UInt64 size;
        CORE::CString sha256Hex;        /**< 64 lower case hexadecimal characters */

        SFileSpec( void );
    };
    typedef GUCEF::vector< SFileSpec > TFileSpecVector;

    struct GUCEF_DRNUTILS_PUBLIC_CPP SUploadInfo
    {
        CORE::CString uploadId;
        CORE::CString subject;
        CORE::CString name;
        EUploadState state;
        TFileSpecVector files;
        GUCEF::vector< CUploadRangeSet > received;  /**< per file */
        UInt64 totalBytes;
        UInt64 receivedBytes;
        UInt64 createdUtcMs;
        UInt64 lastWriteUtcMs;
        UInt64 expiresUtcMs;

        SUploadInfo( void );
    };

    CUploadStaging( void );
    ~CUploadStaging();

    /** Uses the settings; reads the manifests of uploads left by an earlier run (removing torn or expired ones). False with the reason */
    bool Open( const SSettings& settings, CORE::CString& error );
    void Close( void );
    const SSettings& GetSettings( void ) const;

    /** Checks a relative path (see the class description) */
    static bool IsSafeRelativePath( const CORE::CString& relativePath );

    /**
     *  Announces an upload; its id in 'uploadId'. False with a detail code (CRemoteServiceDetailCodes: UNSAFE_PATH,
     *  QUOTA_EXCEEDED, STAGING_FULL, UPLOAD_LIMIT, OUT_OF_RANGE for a bad size or digest) and a message
     */
    bool Begin( const CORE::CString& subject, const CORE::CString& name, const TFileSpecVector& files, UInt64 nowUtcMs,
                CORE::CString& uploadId, CORE::CString& detailCode, CORE::CString& message );

    /** Writes bytes of a file at an offset (a block, or the last part of the file); false with UPLOAD_UNKNOWN, OUT_OF_RANGE, ... */
    bool WriteBlock( const CORE::CString& subject, const CORE::CString& uploadId, UInt32 fileIndex, UInt64 offset,
                     const void* data, UInt32 size, UInt64 nowUtcMs, CORE::CString& detailCode, CORE::CString& message );

    /** What arrived; false (UPLOAD_UNKNOWN) for an upload of another subject or none */
    bool Query( const CORE::CString& subject, const CORE::CString& uploadId, SUploadInfo& info ) const;

    /**
     *  Checks that every file is complete (UPLOAD_INCOMPLETE) and its SHA-256 (UPLOAD_DIGEST_MISMATCH, the file's received
     *  ranges are cleared so it is sent again); COMMITTED with the folder of the files in 'stagedDirectory'
     */
    bool Commit( const CORE::CString& subject, const CORE::CString& uploadId, CORE::CString& stagedDirectory,
                 CORE::CString& detailCode, CORE::CString& message );

    /** Removes an upload of the subject (any state); false when unknown */
    bool Abort( const CORE::CString& subject, const CORE::CString& uploadId );

    /** The service took the committed folder (or is done with it): forgets the upload and removes what is left */
    bool Release( const CORE::CString& uploadId );

    /** Removes uploads that are not committed and expired; how many */
    UInt32 ExpireOld( UInt64 nowUtcMs );

    UInt64 GetUsedBytes( const CORE::CString& subject ) const;
    UInt64 GetTotalUsedBytes( void ) const;
    UInt32 GetOpenCount( void ) const;
    UInt32 GetOpenCount( const CORE::CString& subject ) const;

    private:

    typedef GUCEF::map< CORE::CString, SUploadInfo > TUploadMap;

    CUploadStaging( const CUploadStaging& src );                /**< not implemented */
    CUploadStaging& operator=( const CUploadStaging& src );     /**< not implemented */

    bool SaveManifest( const SUploadInfo& info, CORE::CString& error ) const;
    bool LoadManifest( const CORE::CString& path, SUploadInfo& info, CORE::CString& error ) const;
    CORE::CString GetUploadDirectory( const CORE::CString& uploadId ) const;
    CORE::CString GetFilePath( const SUploadInfo& info, UInt32 fileIndex ) const;
    void RemoveUploadFiles( const CORE::CString& uploadId ) const;

    mutable MT::CMutex m_lock;
    SSettings m_settings;
    TUploadMap m_uploads;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CUPLOADSTAGING_H ? */
