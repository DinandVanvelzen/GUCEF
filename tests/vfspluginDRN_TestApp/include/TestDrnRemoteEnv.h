/*
 *  vfspluginDRN_TestApp: tests of the DRN VFS plugin
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


#ifndef GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNREMOTEENV_H
#define GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNREMOTEENV_H

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

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#include "vfspluginDRN_CDrnBridgeArchive.h"

#include "TestRemoteEnvironment.h"
#include "TestVfsBlockCacheUtils.h"
#include "TestDRNGroup.h"
#include "TestDrnUtilsTcpProxy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNBRIDGETEST {

using namespace GUCEF;

/** What the serving side calls to end the link that the client reads through (a link that drops in the middle of a transfer) */
class ILinkCutter
{
    public:

    virtual ~ILinkCutter() {}
    virtual void CutLink( void ) = 0;
};

/** What the serving side calls to change a file while it is being read */
class IPlaceHook
{
    public:

    virtual ~IPlaceHook() {}
    virtual void OnPlaceHook( const CORE::CString& path ) = 0;
};

/**
 *  The share as the serving node gives it, with the faults the cases need: a delay per call, a call that fails once, a transfer that ends the link after
 *  a number of bytes, a file that changes under a transfer, digests the share knows. The calls come from several threads of the serving node.
 */
class CPlaceProvider : public DRN::CDRNFileSystemAccessResourceProvider
{
    public:

    CPlaceProvider( const CORE::CString& root, ILinkCutter* cutter, IPlaceHook* hook );

    virtual ~CPlaceProvider() GUCEF_VIRTUAL_OVERRIDE;

    void SetDelayMs( UInt32 delayMs );

    /** The next 'count' calls of the operation fail (a transfer error); with 'hang' they last longer than a call of the client waits instead */
    void FailNext( CTestRemotePlace::EOperation operation, UInt32 count, bool hang );

    /** The reads of the next transfer deliver 'bytes' bytes, then the transfer fails; with 'cutLink' the link is cut as well (the cutter that was given) */
    void CutNextReadsAfter( UInt64 bytes, bool cutLink );

    /** After the read with this number (of those since the hook was set) 'path' is changed through the hook; 0 clears it */
    void SetChangeHook( UInt32 afterReadNumber, const CORE::CString& path );

    void ProvideDigest( const CORE::CString& path );
    void ProvideWrongDigest( const CORE::CString& path );

    /** The reads deliver bytes that are not the content of the files (their digests stay those of the content) */
    void SetCorrupt( bool corrupt );

    /** What the share says the revision of a file is, without counting as a call */
    CORE::CString RevisionOf( const CORE::CString& name );

    UInt32 GetStatCount( void ) const;
    UInt32 GetListCount( void ) const;
    UInt32 GetReadCount( void ) const;
    void ResetReadCount( void );

    virtual DRN::TDRNResourceStatus Resolve( const CORE::CString& name, DRN::CDRNResourceInfo& info ) GUCEF_VIRTUAL_OVERRIDE;
    virtual DRN::TDRNResourceStatus Read( const CORE::CString& name, const DRN::CDRNResourceInfo& resolved, UInt64 offset, UInt32 size, void* destination, UInt32& bytesRead ) GUCEF_VIRTUAL_OVERRIDE;
    virtual DRN::TDRNResourceStatus Stat( const CORE::CString& name, DRN::CDRNNamespaceEntry& entry ) GUCEF_VIRTUAL_OVERRIDE;
    virtual DRN::TDRNResourceStatus OpenEnumerator( const CORE::CString& dirName, DRN::CIDRNNamespaceEnumeratorPtr& enumerator ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool GetKnownSha256( const CORE::CString& name, const DRN::CDRNNamespaceEntry& entry, UInt8* digest ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** Waits the delay; true when this call is one that has to fail */
    bool Prelude( CTestRemotePlace::EOperation operation ) const;

    mutable MT::CMutex m_lock;
    UInt32 m_delayMs;
    mutable UInt32 m_failCount[ 4 ];
    mutable bool m_failHangs[ 4 ];
    mutable UInt64 m_cutAfterBytes;
    mutable bool m_cutArmed;
    mutable bool m_cutLink;
    mutable UInt64 m_cutServed;
    mutable UInt32 m_hookAfter;
    mutable UInt32 m_hookReads;
    CORE::CString m_hookPath;
    CORE::CString::StringSet m_provided;
    CORE::CString::StringSet m_wrong;
    bool m_corrupt;
    mutable UInt32 m_statCount;
    mutable UInt32 m_listCount;
    mutable UInt32 m_readCount;
    ILinkCutter* m_cutter;
    IPlaceHook* m_hook;
};

/**
 *  The arrangement of the remote archive suite on the bridge: a directory with the files of the suite (their bytes a function of file and version) that a
 *  serving node shares, a client node in the same session and the archive of type "drn" mounted in the VFS on the client's session, with a block cache in
 *  a folder of its own. The link between the nodes is a loopback link, or a loopback link through a fault proxy that can cut it.
 *
 *  The counts of what reached the place are those of the share (stats and lists), and those of the bridge (runs and bytes fetched, handles opened).
 */
class CDrnRemoteEnv : public CRemoteEnv ,
                      public CTestRemotePlace ,
                      public ILinkCutter ,
                      public IPlaceHook
{
    public:

    /** @param viaProxy the client joins the session through a TCP fault proxy, and a link that is cut is cut there */
    CDrnRemoteEnv( const char* name, bool viaProxy );

    virtual ~CDrnRemoteEnv() GUCEF_VIRTUAL_OVERRIDE;

    // CRemoteEnv

    virtual bool Setup( const SRemoteEnvOptions& options = SRemoteEnvOptions() ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Teardown( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Restart( bool reopenCache ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Unmount( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void CloseCache( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CTestRemotePlace& Backend( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual VFS::CVfsRemoteArchiveCore* Core( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual VFS::CVfsBlockCachePtr Cache( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual VFS::CVfsFetchScheduler& Scheduler( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CTestEventRecorder& Events( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetCacheFolder( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const SRemoteEnvOptions& GetOptions( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetPlaceText( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult GetCutFetchResult( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool HasHandleStatistics( void ) const GUCEF_VIRTUAL_OVERRIDE;

    // CTestRemotePlace

    virtual void ChangeFile( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void ChangeFileSize( const CORE::CString& path, UInt64 newSize ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void ProvideDigest( const CORE::CString& path, VFS::CVfsDigest::EAlgorithm algorithm = VFS::CVfsDigest::ALGORITHM_SHA256 ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void ProvideWrongDigest( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;
    virtual VFS::CVfsDigest ComputeDigest( const CORE::CString& path, VFS::CVfsDigest::EAlgorithm algorithm ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool FillExpected( const CORE::CString& path, UInt64 offset, void* dest, UInt32 count ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CheckExpected( const CORE::CString& path, UInt64 offset, const void* data, UInt32 count ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetSize( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetRevision( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetLinkUp( bool linkUp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetDelayMs( UInt32 delayMs ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void FailNext( EOperation operation, UInt32 count, CORE::TFileSystemAccessResult result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void CutNextFetchAfter( UInt64 bytes ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetCorruptContent( bool corrupt ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetOnFetchHook( UInt32 afterFetchNumber, const CORE::CString& pathToChange ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetStatCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetListCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetOpenCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetFetchCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetBytesServed( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void ResetCounters( void ) GUCEF_VIRTUAL_OVERRIDE;

    // the serving side's calls back

    virtual void CutLink( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPlaceHook( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;

    // what a test that needs more than the suite asks for

    /** Before Setup(): the client node has no thread of its own and the thread that makes the calls pumps it, as the main thread of a single thread application does */
    void SetClientOwnThread( bool ownThread );

    /** Before Setup(): another archive specific setting for the mount of the archive (a key of CDrnBridgeSettings and its value) */
    void AddBridgeSetting( const CORE::CString& key, const CORE::CString& value );

    /** The client leaves the session and a new client node and session of the same name take its place (the registry names the new one) */
    bool ReplaceClientSession( void );

    /** The serving node and the client node end and a serving node of the same session name, with the same files, takes the place; the client joins it again */
    bool RestartServer( void );

    /** The share as it is served now */
    CPlaceProvider* GetProvider( void );

    /** The session of the serving node, of the client node, and the peer id the client sees the serving node by */
    DRN::CDRNSession& Server( void );
    DRN::CDRNSession& Client( void );
    UInt32 GetServerPeerId( void ) const;
    const CORE::CString& GetSessionName( void ) const;

    /** The archive that is mounted, NULL when none is */
    VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* GetBridge( void );

    /** The proxy of the link, NULL for a loopback link */
    DRNUTILSTEST::CTestTcpFaultProxy* GetProxy( void );

    /** Runs a call on the thread that pumps the proxy: the proxy is not thread safe */
    void LockProxy( void );
    void UnlockProxy( void );

    /** Whether the client has an operational link to the serving node */
    bool IsClientLinkUp( void );

    const CORE::CString& GetServedRoot( void ) const;

    private:

    CDrnRemoteEnv( const CDrnRemoteEnv& src );
    CDrnRemoteEnv& operator=( const CDrnRemoteEnv& src );

    class CProxyPump;
    struct SFileState
    {
        UInt32 contentId;
        UInt64 size;
        UInt32 version;
        SFileState( void ) : contentId( 0 ), size( 0 ), version( 0 ) {}
    };
    typedef GUCEF::map< CORE::CString, SFileState > TFileStateMap;

    void AddFile( const CORE::CString& path, UInt32 contentId, UInt64 size );
    bool WriteFile( const CORE::CString& path, const SFileState& state );
    bool MountArchive( void );
    bool JoinClient( void );
    bool StartServing( void );
    void StopServing( void );
    void WaitStep( void );
    void CarryCounters( void );
    UInt32 ClientOpenCount( void ) const;
    CORE::CString FilePath( const CORE::CString& path ) const;

    CORE::CString m_name;
    bool m_viaProxy;
    bool m_clientOwnThread;
    GUCEF::vector< CORE::CString > m_extraKeys;
    GUCEF::vector< CORE::CString > m_extraValues;
    SRemoteEnvOptions m_options;
    CBlockCacheTestFolder* m_cacheFolder;
    CBlockCacheTestFolder* m_serveFolder;
    DRNTEST::CGroup* m_group;
    UInt32 m_serverPeerId;
    CORE::CString m_sessionName;
    bool m_registered;
    CPlaceProvider* m_provider;
    DRN::CIDRNResourceProviderPtr m_providerPtr;
    CTestEventRecorder m_events;
    TFileStateMap m_files;
    mutable MT::CMutex m_filesLock;

    // the proxy and what pumps it
    CORE::PulseGeneratorPtr m_proxyPulse;
    DRNUTILSTEST::CTestTcpFaultProxy* m_proxy;
    CProxyPump* m_pump;
    MT::CMutex m_proxyLock;

    // counters carried over archives that were unmounted
    UInt64 m_carriedFetches;
    UInt64 m_carriedBytes;
    UInt64 m_carriedOpens;
    UInt64 m_baseFetches;
    UInt64 m_baseBytes;
    UInt64 m_baseOpens;
    UInt32 m_baseStats;
    UInt32 m_baseLists;
    bool m_corrupt;
    bool m_mounted;
    bool m_setUp;
};

/** The two arrangements of the remote archive suite on the bridge, as factories for CRemoteEnv */
CRemoteEnv* MakeLoopbackRemoteEnv( const char* name );
CRemoteEnv* MakeProxiedRemoteEnv( const char* name );

}; /* namespace DRNBRIDGETEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNREMOTEENV_H ? */
