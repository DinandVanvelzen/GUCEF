/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_VFS_TESTAPP_TESTREMOTEENVIRONMENT_H
#define GUCEF_VFS_TESTAPP_TESTREMOTEENVIRONMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Records the events that a notifier raises (the VFS, an archive, a core) with the thread each was delivered on and the
 *  order they came in, for the cases that check the events.
 */
class CTestEventRecorder : public GUCEF::CORE::CObservingNotifier
{
    public:

    struct SRecord
    {
        GUCEF::CORE::CString eventName;
        GUCEF::CORE::CString path;                      /**< of an arrival event */
        GUCEF::CORE::CString revision;
        GUCEF::CORE::CString oldRevision;
        GUCEF::UInt64 firstByte;
        GUCEF::UInt64 byteCount;
        bool fileComplete;
        GUCEF::CORE::TFileSystemAccessResult result;
        GUCEF::CORE::CString message;
        GUCEF::VFS::EVfsFetchJobKind jobKind;
        GUCEF::CORE::CString mountPath;                 /**< of a connection state event of the VFS */
        bool connected;
        GUCEF::TThreadId thread;                        /**< the thread the event was delivered on */
        GUCEF::UInt32 sequence;
        SRecord( void );
    };
    /** The records of a kind; an index past the end gives an empty record, so that a case that found fewer events than it expected fails on its checks and does not end the process */
    class TRecordVector : public GUCEF::vector< SRecord >
    {
        public:

        const SRecord& operator[]( size_t index ) const
        {
            static const SRecord none;
            return index < this->size() ? GUCEF::vector< SRecord >::operator[]( index ) : none;
        }
    };

    CTestEventRecorder( void );

    virtual ~CTestEventRecorder() GUCEF_VIRTUAL_OVERRIDE;

    /** Listens to the VFS: all its events. The data of a connection state event is the one the VFS passes on (with the mount path) */
    void Listen( GUCEF::CORE::CNotifier* notifier );

    /** The number of events of the kind; for a path, those about it */
    GUCEF::UInt32 Count( const GUCEF::CORE::CEvent& eventId ) const;
    GUCEF::UInt32 Count( const GUCEF::CORE::CEvent& eventId, const GUCEF::CORE::CString& path ) const;

    /** The records of the kind and path (any path when empty) in the order they came in */
    TRecordVector Find( const GUCEF::CORE::CEvent& eventId, const GUCEF::CORE::CString& path = GUCEF::CORE::CString::Empty ) const;

    TRecordVector GetAll( void ) const;

    GUCEF::UInt32 GetTotal( void ) const;

    void Clear( void );

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier                 ,
                           const GUCEF::CORE::CEvent& eventid               ,
                           GUCEF::CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    mutable GUCEF::MT::CMutex m_lock;
    TRecordVector m_records;
    GUCEF::UInt32 m_sequence;
};

/*-------------------------------------------------------------------------*/

struct SRemoteEnvOptions
{
    GUCEF::UInt32 metadataTtlMs;
    GUCEF::UInt32 blockBytes;
    GUCEF::UInt64 cacheMaxBytes;
    bool useCache;
    GUCEF::UInt32 maxRunning;
    GUCEF::UInt32 maxQueued;
    GUCEF::UInt32 readAheadAfterBlocks;
    GUCEF::UInt32 readAheadBlocks;
    GUCEF::UInt32 maxBlocksPerFetch;
    bool offlineListings;
    bool verifyCompleteFiles;
    bool refetchPinned;
    GUCEF::UInt32 maxMetadataEntries;

    SRemoteEnvOptions( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  What the cases of the remote archive suite do to the remote place behind the archive: they change its files, cut the link, slow it down, make
 *  a call fail, give files digests, and ask what reached it. A place that a test arrangement provides (a memory place for the generic layer, a
 *  share of another node for the bridge) implements this so that one set of cases runs against each of them.
 *
 *  Files have the content that FillExpected() describes: for a file and the version it has now, the same bytes every time. Revisions are strong
 *  (they change when the content does).
 */
class CTestRemotePlace
{
    public:

    enum EOperation
    {
        OP_STAT = 0,
        OP_LIST,
        OP_OPEN,
        OP_FETCH
    };

    virtual ~CTestRemotePlace() {}

    /** Moves the version of a file: the revision and the bytes are other ones, the size stays */
    virtual void ChangeFile( const GUCEF::CORE::CString& path ) = 0;

    /** Moves the version and gives the file another size */
    virtual void ChangeFileSize( const GUCEF::CORE::CString& path, GUCEF::UInt64 newSize ) = 0;

    /** Makes the place give the digest of the file (of the algorithm) in its answers */
    virtual void ProvideDigest( const GUCEF::CORE::CString& path, GUCEF::VFS::CVfsDigest::EAlgorithm algorithm = GUCEF::VFS::CVfsDigest::ALGORITHM_SHA256 ) = 0;

    /** Makes the place give a digest that is not the one of the content (a damaged remote file) */
    virtual void ProvideWrongDigest( const GUCEF::CORE::CString& path ) = 0;

    /** The digest of the content the file has now */
    virtual GUCEF::VFS::CVfsDigest ComputeDigest( const GUCEF::CORE::CString& path, GUCEF::VFS::CVfsDigest::EAlgorithm algorithm ) const = 0;

    /** The bytes the file has now, as the place gives them */
    virtual bool FillExpected( const GUCEF::CORE::CString& path, GUCEF::UInt64 offset, void* dest, GUCEF::UInt32 count ) const = 0;

    /** Whether the bytes are the content the file has now at the offset */
    virtual bool CheckExpected( const GUCEF::CORE::CString& path, GUCEF::UInt64 offset, const void* data, GUCEF::UInt32 count ) const = 0;

    virtual GUCEF::UInt64 GetSize( const GUCEF::CORE::CString& path ) const = 0;

    virtual GUCEF::CORE::CString GetRevision( const GUCEF::CORE::CString& path ) const = 0;

    /** Cuts or restores the link: a call to a place that is cut cannot be answered, the archive sees the link go */
    virtual void SetLinkUp( bool linkUp ) = 0;

    /** The time every call takes before it answers */
    virtual void SetDelayMs( GUCEF::UInt32 delayMs ) = 0;

    /** The next 'count' calls of the operation fail with the result (as far as the transport can say it) */
    virtual void FailNext( EOperation operation, GUCEF::UInt32 count, GUCEF::CORE::TFileSystemAccessResult result ) = 0;

    /** The next fetch delivers 'bytes' bytes and then fails (the link dropped in the middle of a run); the one after is whole */
    virtual void CutNextFetchAfter( GUCEF::UInt64 bytes ) = 0;

    /** The fetches deliver bytes that are not the content: a damaged remote file. The digest the place gives stays the right one */
    virtual void SetCorruptContent( bool corrupt ) = 0;

    /** Called while the next fetch is under way, once it has delivered its first byte and before it returns: for the cases that change the file during a run */
    virtual void SetOnFetchHook( GUCEF::UInt32 afterFetchNumber, const GUCEF::CORE::CString& pathToChange ) = 0;

    // what reached the place

    virtual GUCEF::UInt32 GetStatCount( void ) const = 0;
    virtual GUCEF::UInt32 GetListCount( void ) const = 0;
    virtual GUCEF::UInt32 GetOpenCount( void ) const = 0;
    virtual GUCEF::UInt32 GetFetchCount( void ) const = 0;
    virtual GUCEF::UInt64 GetBytesServed( void ) const = 0;
    virtual void ResetCounters( void ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  The environment of the cases of the remote archive suite: a remote place with a small tree, a block cache in a folder of its own, a scheduler with a
 *  pool of its own, and an archive mounted in the VFS at "/remote/". The events of the VFS are recorded. The test thread is the thread that pumps the
 *  pulse generator, as the main thread of a single thread application is: events are delivered when a case calls Pump() (or waits for one).
 *
 *  This is the interface the cases use; what makes the place and the archive is the arrangement that implements it.
 */
class CRemoteEnv
{
    public:

    /** The names of the tree */
    static const char* const ReadmeFile;        /**< 1000 bytes */
    static const char* const NotesFile;         /**< 70000 bytes: two blocks of 64 KiB */
    static const char* const BigFile;           /**< 1000000 bytes: sixteen blocks */
    static const char* const MidFile;           /**< 300000 bytes: five blocks */
    static const char* const EmptyFile;         /**< 0 bytes */
    static const char* const TopFile;           /**< 100 bytes */
    static const char* const DeepFile;          /**< 5000 bytes, below data/sub/deep */
    static const char* const MountPath;         /**< "/remote/" */
    static const char* const ArchiveName;

    CRemoteEnv( void ) {}

    virtual ~CRemoteEnv() {}

    virtual bool Setup( const SRemoteEnvOptions& options = SRemoteEnvOptions() ) = 0;

    virtual void Teardown( void ) = 0;

    /** Unmounts the archive and mounts it again: a new archive and a new core on the same place. With 'reopenCache' the cache is closed and opened again on its folder (a restart of the process) */
    virtual bool Restart( bool reopenCache ) = 0;

    /** Unmounts the archive; the events of the archive stop */
    virtual bool Unmount( void ) = 0;

    /** Lets go of the cache (its folder is closed); the files stay. Restart() with 'reopenCache' opens it again */
    virtual void CloseCache( void ) = 0;

    virtual CTestRemotePlace& Backend( void ) = 0;
    virtual GUCEF::VFS::CVfsRemoteArchiveCore* Core( void ) = 0;
    virtual GUCEF::VFS::CVfsBlockCachePtr Cache( void ) = 0;
    virtual GUCEF::VFS::CVfsFetchScheduler& Scheduler( void ) = 0;
    virtual CTestEventRecorder& Events( void ) = 0;
    virtual const GUCEF::CORE::CString& GetCacheFolder( void ) const = 0;
    virtual const SRemoteEnvOptions& GetOptions( void ) const = 0;

    /** What the message of a failure names of the place (the share and its owner), for the cases that check that a message says which place cannot be reached */
    virtual GUCEF::CORE::CString GetPlaceText( void ) const;

    /** What a read ends with when CutNextFetchAfter() cut the run: the transport result of the arrangement */
    virtual GUCEF::CORE::TFileSystemAccessResult GetCutFetchResult( void ) const;

    /** Whether the transport has handles of its own whose use shows in the statistics (handlesOpen, handlesPeak, ...) */
    virtual bool HasHandleStatistics( void ) const;

    GUCEF::VFS::CVFS& Vfs( void );

    /** The VFS path of a path of the archive */
    GUCEF::CORE::CString VfsPath( const GUCEF::CORE::CString& relative ) const;

    /** Delivers what the pulse generator holds: the events that were raised on other threads reach their observers on this thread */
    static void Pump( void );

    /** Pumps until the recorder has at least 'atLeast' events of the kind (about the path when given), or the time is up */
    bool WaitForEvents( const GUCEF::CORE::CEvent& eventId, const GUCEF::CORE::CString& relativePath, GUCEF::UInt32 atLeast, GUCEF::UInt32 timeoutMs );

    /** Pumps until the scheduler has nothing queued or running, then once more to deliver what it raised */
    bool WaitUntilIdle( GUCEF::UInt32 timeoutMs );

    /** Reads the whole file through the VFS with the policy of the calling thread; the bytes that came and the result of the read */
    bool ReadAll( const GUCEF::CORE::CString& relativePath, GUCEF::CORE::CDynamicBuffer& content, GUCEF::VFS::CVfsCallResult* readResult = GUCEF_NULL );

    /** Whether the bytes are the content of the file in the place at the offset */
    bool Matches( const GUCEF::CORE::CString& relativePath, GUCEF::UInt64 offset, const GUCEF::CORE::CDynamicBuffer& content, GUCEF::UInt32 count );

    private:

    CRemoteEnv( const CRemoteEnv& src );
    CRemoteEnv& operator=( const CRemoteEnv& src );
};

/** Makes the environment of a case: the arrangement that the suite runs against */
typedef CRemoteEnv* ( *TRemoteEnvFactory )( const char* name );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTREMOTEENVIRONMENT_H ? */
