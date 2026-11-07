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

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESSTORE_H
#define DBPLUGIN_VFSTABLES_CVFSTABLESSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESSETTINGS_H
#include "dbpluginVFSTABLES_CVfsTablesSettings.h"
#define DBPLUGIN_VFSTABLES_CVFSTABLESSETTINGS_H
#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESSETTINGS_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESSTATS_H
#include "dbpluginVFSTABLES_CVfsTablesStats.h"
#define DBPLUGIN_VFSTABLES_CVFSTABLESSTATS_H
#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESSTATS_H ? */

#ifndef DBPLUGIN_VFSTABLES_CDATATABLESHAREDFILE_H
#include "dbpluginVFSTABLES_CDataTableSharedFile.h"
#define DBPLUGIN_VFSTABLES_CDATATABLESHAREDFILE_H
#endif /* DBPLUGIN_VFSTABLES_CDATATABLESHAREDFILE_H ? */

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
 *  The storage side of one open vfstables database, shared by its client and every table opened from it: the VFS root,
 *  the listing and footer caches (14.4, 14.6), the replacement of files (14.5) and the buffer pool of the tables.
 *
 *  Files are replaced and deleted under one lock, which also takes the changed files out of the caches and counts a
 *  replace generation. A reader notes the generation, opens the file without the lock, and looks a footer up in the cache
 *  only when the generation is still the same. A cached footer is used only for an open file of the size it was read
 *  from and after CITableFileMetaData::VerifySource() found the footer bytes of the open file equal to those it was
 *  read from, so a file another process replaced, by one of the same size within the same second included, is never
 *  read with the footer of the file before it. Metadata of codecs that cannot verify a source is not cached. New and
 *  deleted files of another process are seen after Refresh() or, with listingCacheTtlMs, after the listing expires.
 *
 *  Thread safe. Tables hold the store, so it lives as long as the last of them.
 */
class DBPLUGIN_VFSTABLES_PUBLIC_CPP CVfsTablesStore
{
    public:

    typedef CORE::CTableCodecRegistry::TTableCodecPtr TTableCodecPtr;

    /** The directory below the root that holds the table definitions */
    static const CORE::CString TablesDirName;

    /** What a temporary file name has between the final name and the writer id */
    static const CORE::CString TemporaryFileMarker;

    CVfsTablesStore( const CORE::CString& root              ,
                     bool readOnly                          ,
                     const CVfsTablesSettings& settings     ,
                     UInt64 bufferPoolMaxBytes              ,
                     const CORE::ThreadPoolPtr& threadPool  );

    ~CVfsTablesStore();

    /** The VFS path of the database root, without a trailing '/' */
    const CORE::CString& GetRoot( void ) const;

    bool IsReadOnly( void ) const;

    const CVfsTablesSettings& GetSettings( void ) const;

    CORE::CDataTableBufferPool& GetBufferPool( void );

    /** The pool async work of the tables runs on when the caller gives none */
    CORE::ThreadPoolPtr GetThreadPool( void ) const;

    /** The VFS path of a path relative to the root */
    CORE::CString MakePath( const CORE::CString& relativePath ) const;

    /**
     *  The names in a directory: sub directories (without a trailing '/') or files, sorted bytewise. A directory that
     *  does not exist has none. From the listing cache when it holds the directory and the listing has not expired.
     */
    void ListDirectory( const CORE::CString& dirPath, bool directories, CORE::CString::StringVector& names );

    /** Whether the file exists: from a cached listing of its directory when there is one, else asked of the VFS */
    bool FileExists( const CORE::CString& path );

    /**
     *  Opens a file for reading by readers that share it, with its metadata: from the footer cache when it holds the
     *  file as it is now (verified against the open file), else read through the codec and put in the cache. NOT_FOUND
     *  for a file that does not exist. With a requiredGeneration other than GUCEF_UINT64MAX the file is only opened when
     *  no file was replaced or deleted since that generation, and with requiredMetaData only when the open file holds
     *  that metadata (when its codec can verify a source), else IO_ERROR: a reader that must see the version it
     *  planned with, also when another process replaced the file.
     */
    CORE::TDataTableStatus OpenSharedFile( const CORE::CString& path                                        ,
                                           const TTableCodecPtr& codec                                      ,
                                           const CORE::CValueList& codecOptions                             ,
                                           TDataTableSharedFilePtr& file                                    ,
                                           bool& metaDataFromCache                                          ,
                                           CORE::CString& error                                             ,
                                           UInt64 requiredGeneration = GUCEF_UINT64MAX                      ,
                                           const CORE::CITableFileMetaDataPtr& requiredMetaData = CORE::CITableFileMetaDataPtr() );

    /** A temporary file name next to the final one: "<final>.tmp-<writerId>-<counter>", never ending like the final name */
    CORE::CString MakeTemporaryPath( const CORE::CString& finalPath );

    /**
     *  Makes a written file durable when its input can (syncWrites, a file of the file system) and closes it. False
     *  when the sync was asked for and failed
     */
    bool FinishTemporaryFile( CORE::IOAccessPtr& access );

    /**
     *  Moves the temporary file over the final path in one atomic step (CVFS::MoveFileAtomically: a rename on a file
     *  system mount, never a copy), retried with a backoff while it is refused until replaceRetryTimeoutMs (an open file
     *  on Windows); takes the final path out of the footer cache and adds it to the cached listings. IO_ERROR when it
     *  could not be done: the final file is as it was, also for readers holding it, the caller removes the temporary file
     */
    CORE::TDataTableStatus ReplaceFile( const CORE::CString& temporaryPath, const CORE::CString& finalPath, CORE::CString& error );

    /** Deletes the file, also from the caches; OK when it did not exist */
    CORE::TDataTableStatus DeleteFile( const CORE::CString& path, CORE::CString& error );

    /** Removes a temporary file of a write that failed */
    void RemoveTemporaryFile( const CORE::CString& temporaryPath );

    /** Removes the temporary files of this writer id below the root that a writer which stopped left behind; the count */
    UInt32 RemoveLeftoverTemporaryFiles( void );

    /** Whether a file name is a temporary file name (of any writer) */
    static bool IsTemporaryFileName( const CORE::CString& name );

    /** The lock that serializes the changes of one table, by name: two tables objects of one table share it */
    MT::CMutex& GetTableWriteLock( const CORE::CString& tableName );

    /** The replace generation: counts the files replaced and deleted since the store was made */
    UInt64 GetGeneration( void ) const;

    /** Forgets the cached listings and footers */
    void ClearCaches( void );

    void GetStats( CVfsTablesStats& stats ) const;

    void ResetStats( void );

    private:

    struct SListing
    {
        CORE::CString::StringSet names;
        UInt64 listedAtTick;
    };
    typedef GUCEF::map< CORE::CString, SListing > TListingMap;

    struct SFooterEntry
    {
        CORE::CITableFileMetaDataPtr metaData;
        UInt64 size;
        UInt64 bytes;
        UInt64 lastUse;
    };
    typedef GUCEF::map< CORE::CString, SFooterEntry > TFooterMap;
    typedef GUCEF::map< UInt64, CORE::CString > TFooterUseMap;
    typedef GUCEF::map< CORE::CString, MT::CMutex* > TWriteLockMap;

    CVfsTablesStore( const CVfsTablesStore& src );              /**< not implemented */
    CVfsTablesStore& operator=( const CVfsTablesStore& src );   /**< not implemented */

    /** Under the replace lock: the caches after the file at the path was put in place or deleted */
    void OnFileChangedLocked( const CORE::CString& path, bool deleted );

    bool IsListingExpired( const SListing& listing, UInt64 nowTick ) const;

    void PutFooterInCache( const CORE::CString& path, const CORE::CITableFileMetaDataPtr& metaData, UInt64 size );

    CORE::CString m_root;
    bool m_readOnly;
    CVfsTablesSettings m_settings;
    CORE::CString m_escapedWriterId;
    CORE::CDataTableBufferPool m_bufferPool;
    CORE::ThreadPoolPtr m_threadPool;
    MT::CTAtomic< UInt64 > m_temporaryCounter;

    mutable MT::CMutex m_replaceLock;           /**< held while a file is replaced or deleted, and to read the generation */
    UInt64 m_generation;                        /**< counts the replaces and deletes, changed under m_replaceLock */

    mutable MT::CMutex m_listingLock;
    TListingMap m_listings;                     /**< keyed by directory path + "|d" (directories) or "|f" (files) */
    UInt64 m_tickResolution;

    mutable MT::CMutex m_footerLock;
    TFooterMap m_footers;
    TFooterUseMap m_footerUse;                  /**< least recently used first */
    UInt64 m_footerBytes;
    UInt64 m_footerUseCounter;

    MT::CMutex m_writeLocksLock;
    TWriteLockMap m_writeLocks;

    mutable MT::CMutex m_statsLock;
    CVfsTablesStats m_stats;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CVfsTablesStore, MT::CMutex >    TVfsTablesStorePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESSTORE_H ? */
