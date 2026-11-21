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

#ifndef GUCEF_VFS_TESTAPP_TESTREMOTEBACKEND_H
#define GUCEF_VFS_TESTAPP_TESTREMOTEBACKEND_H

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

#include "TestRemoteEnvironment.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A remote place that the tests control: a tree of files in memory whose content is a function of the file and its
 *  version, with a link that can be cut, a delay per call, a version that can be moved (which changes the revision and the
 *  bytes), failures that can be injected and counters of what reached it. It is what CIVfsRemoteBackend is for a test: the
 *  generic remote archive layer runs against it with no transport involved.
 *
 *  The revision of a file is "<size>-<version>": strong, the content never changes without it. Digests are given by the
 *  place for the files that ask for it (ProvideDigest()).
 */
class CTestRemoteBackend : public GUCEF::VFS::CIVfsRemoteBackend ,
                           public CTestRemotePlace
{
    public:

    CTestRemoteBackend( void );

    virtual ~CTestRemoteBackend() GUCEF_VIRTUAL_OVERRIDE;

    // the tree

    /** Adds a file of 'size' bytes whose content is made from 'contentId'; the directories above it exist */
    void AddFile( const GUCEF::CORE::CString& path, GUCEF::UInt32 contentId, GUCEF::UInt64 size );

    void AddDirectory( const GUCEF::CORE::CString& path );

    void RemoveFile( const GUCEF::CORE::CString& path );

    /** Removes every file and directory */
    void Clear( void );

    /** Puts a file with the bytes (a new version when it was there): the content is not made from an id */
    void PutFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CDynamicBuffer& content );

    /** Moves the version of a file: the revision and the bytes are other ones, the size stays */
    virtual void ChangeFile( const GUCEF::CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;

    /** Moves the version and gives the file another size */
    virtual void ChangeFileSize( const GUCEF::CORE::CString& path, GUCEF::UInt64 newSize ) GUCEF_VIRTUAL_OVERRIDE;

    /** Makes the place give the digest of the file (of the algorithm) in its answers; MD5 is the default of a call without an algorithm */
    virtual void ProvideDigest( const GUCEF::CORE::CString& path, GUCEF::VFS::CVfsDigest::EAlgorithm algorithm = GUCEF::VFS::CVfsDigest::ALGORITHM_SHA256 ) GUCEF_VIRTUAL_OVERRIDE;

    /** Makes the place give a digest that is not the one of the content (a damaged remote file) */
    virtual void ProvideWrongDigest( const GUCEF::CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;

    /** The digest of the content the file has now */
    virtual GUCEF::VFS::CVfsDigest ComputeDigest( const GUCEF::CORE::CString& path, GUCEF::VFS::CVfsDigest::EAlgorithm algorithm ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The bytes the file has now, as the place gives them */
    virtual bool FillExpected( const GUCEF::CORE::CString& path, GUCEF::UInt64 offset, void* dest, GUCEF::UInt32 count ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the bytes are the content the file has now at the offset */
    virtual bool CheckExpected( const GUCEF::CORE::CString& path, GUCEF::UInt64 offset, const void* data, GUCEF::UInt32 count ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::UInt64 GetSize( const GUCEF::CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::CString GetRevision( const GUCEF::CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    // the controls

    /** Cuts or restores the link: a call to a place that is cut answers CONNECTION_PENDING, IsLinkUp() answers false. Tells the listener */
    virtual void SetLinkUp( bool linkUp ) GUCEF_VIRTUAL_OVERRIDE;

    /** The time every call takes before it answers */
    virtual void SetDelayMs( GUCEF::UInt32 delayMs ) GUCEF_VIRTUAL_OVERRIDE;

    /** The next 'count' calls of the operation fail with the result */
    virtual void FailNext( EOperation operation, GUCEF::UInt32 count, GUCEF::CORE::TFileSystemAccessResult result ) GUCEF_VIRTUAL_OVERRIDE;

    /** The next fetch delivers 'bytes' bytes and then fails with CONNECTION_PENDING (the link dropped in the middle of a run); the one after is whole */
    virtual void CutNextFetchAfter( GUCEF::UInt64 bytes ) GUCEF_VIRTUAL_OVERRIDE;

    /** The fetches deliver bytes that are not the content: a damaged remote file. The digest the place gives stays the right one */
    virtual void SetCorruptContent( bool corrupt ) GUCEF_VIRTUAL_OVERRIDE;

    /** The core that is told when the link changes (no more than a weak reference) */
    void SetLinkListener( const GUCEF::CORE::CTWeakPtr< GUCEF::VFS::CVfsRemoteArchiveCore, GUCEF::MT::CMutex >& listener );

    /** Called while the next fetch is under way, once it has delivered its first byte and before it returns: for the cases that change the file during a run */
    virtual void SetOnFetchHook( GUCEF::UInt32 afterFetchNumber, const GUCEF::CORE::CString& pathToChange ) GUCEF_VIRTUAL_OVERRIDE;

    // the counters

    virtual GUCEF::UInt32 GetStatCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetListCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetOpenCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetFetchCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt64 GetBytesServed( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void ResetCounters( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The thread that made the latest call; the calls made on a thread that must not wait say whether they waited */
    GUCEF::TThreadId GetLastCallThreadId( void ) const;

    /** The calls that are waiting out the delay now, in the place */
    GUCEF::UInt32 GetCallsInDelay( void ) const;

    /** Whether the core that was told of the link (the core of the archive that is mounted, or was) still exists */
    bool IsCoreAlive( void ) const;

    // CIVfsRemoteBackend

    virtual bool IsLinkUp( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetPlaceDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::VFS::CVfsCallResult Stat( const GUCEF::CORE::CString& path, GUCEF::VFS::CVfsRemoteEntry& entry ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::VFS::CVfsCallResult List( const GUCEF::CORE::CString& dirPath, GUCEF::VFS::CVfsRemoteEntry::TVector& entries, bool& dirExists ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::VFS::CIVfsBlockSourcePtr OpenBlockSource( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& revision, GUCEF::VFS::CVfsCallResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    /** Used by the sources this place makes */
    GUCEF::VFS::CVfsCallResult FetchFor( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& revision, GUCEF::UInt64 offset, GUCEF::UInt32 length, void* buffer, GUCEF::UInt32& bytesRead );

    private:

    struct SFile
    {
        GUCEF::UInt32 contentId;
        GUCEF::UInt64 size;
        GUCEF::UInt32 version;
        bool provideDigest;
        bool wrongDigest;
        GUCEF::VFS::CVfsDigest::EAlgorithm digestAlgorithm;
        bool explicitBytes;
        GUCEF::vector< GUCEF::UInt8 > bytes;
        SFile( void ) : contentId( 0 ), size( 0 ), version( 0 ), provideDigest( false ), wrongDigest( false ), digestAlgorithm( GUCEF::VFS::CVfsDigest::ALGORITHM_SHA256 ), explicitBytes( false ), bytes() {}
    };
    typedef GUCEF::map< GUCEF::CORE::CString, SFile > TFileMap;
    typedef GUCEF::set< GUCEF::CORE::CString > TDirSet;

    static GUCEF::UInt32 ContentIdOf( const SFile& file );
    static void FillFileBytes( const SFile& file, GUCEF::UInt64 offset, void* dest, GUCEF::UInt32 count );
    static GUCEF::VFS::CVfsDigest DigestOfFile( const SFile& file, GUCEF::VFS::CVfsDigest::EAlgorithm algorithm );
    GUCEF::CORE::CString RevisionOf( const SFile& file ) const;
    bool DirectoryExistsLocked( const GUCEF::CORE::CString& dirPath ) const;
    GUCEF::VFS::CVfsCallResult BeginCall( EOperation operation, bool& linkWasUp );
    void EntryOf( const GUCEF::CORE::CString& path, const SFile& file, GUCEF::VFS::CVfsRemoteEntry& entry ) const;

    private:

    mutable GUCEF::MT::CMutex m_lock;
    TFileMap m_files;
    TDirSet m_dirs;
    bool m_linkUp;
    GUCEF::UInt32 m_delayMs;
    GUCEF::UInt32 m_failCount[ 4 ];
    GUCEF::CORE::TFileSystemAccessResult m_failResult[ 4 ];
    GUCEF::UInt64 m_cutAfterBytes;
    bool m_cutArmed;
    bool m_corrupt;
    GUCEF::UInt32 m_hookAfterFetch;
    GUCEF::CORE::CString m_hookPath;
    GUCEF::CORE::CTWeakPtr< GUCEF::VFS::CVfsRemoteArchiveCore, GUCEF::MT::CMutex > m_listener;
    GUCEF::UInt32 m_counts[ 4 ];
    GUCEF::UInt64 m_bytesServed;
    GUCEF::TThreadId m_lastCallThread;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > m_callsInDelay;
};

typedef GUCEF::CORE::CTSharedPtr< CTestRemoteBackend, GUCEF::MT::CMutex > CTestRemoteBackendPtr;

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTREMOTEBACKEND_H ? */
