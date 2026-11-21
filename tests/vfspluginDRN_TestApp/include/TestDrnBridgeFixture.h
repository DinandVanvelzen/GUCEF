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

#ifndef GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEFIXTURE_H
#define GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEFIXTURE_H

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

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#include "vfspluginDRN_CDrnBridgeArchive.h"
#include "vfspluginDRN_CDrnBridgeGlobal.h"

#include "TestDRNGroup.h"
#include "TestArchiveConformanceFixture.h"
#include "TestFileSystemArchiveConformance.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNBRIDGETEST {

using namespace GUCEF;

/** A file system access on the file system of the operating system, to serve from */
CORE::TFileSystemAccessPtr
NativeFileSystem( void );

/**
 *  A share that knows the SHA-256 of its files, as the provider of a catalogue would: computed here (and remembered by revision), for files
 *  of up to a few MiB, as the share's own knowledge; the bridge never asks for a digest to be computed
 */
class CHashingProvider : public DRN::CDRNFileSystemAccessResourceProvider
{
    public:

    CHashingProvider( const CORE::CString& root );

    protected:

    virtual bool GetKnownSha256( const CORE::CString& name, const DRN::CDRNNamespaceEntry& entry, UInt8* digest ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CKnownDigest
    {
        public:
        CORE::CString revision;
        UInt8 digest[ 32 ];
    };

    typedef GUCEF::map< CORE::CString, CKnownDigest > TDigestMap;

    mutable MT::CMutex m_lock;
    mutable TDigestMap m_known;
};

/**
 *  A serving node and a client node, each with a thread of its own, joined in one session over a loopback link; the serving node
 *  shares a directory of the local file system as the share "files"
 */
class CLoopbackShare
{
    public:

    static const char* const ShareName;

    explicit CLoopbackShare( const CORE::CString& sessionName );

    ~CLoopbackShare();

    /**
     *  Whether the client node works on a thread of its own, default true. Without one the thread that calls Start() and makes the
     *  calls through the archive is the thread that pumps the client node, as the thread of the application's main loop does. Before Start()
     */
    void SetClientOwnThread( bool ownThread );

    /** @param provider the share to serve, NULL for the plain provider over the directory */
    bool Start( const CORE::CString& root, const DRN::CIDRNResourceProviderPtr& provider = DRN::CIDRNResourceProviderPtr() );

    void Stop( void );

    DRN::CDRNSession& Server( void );
    DRN::CDRNSession& Client( void );
    UInt32 GetServerPeerId( void ) const;
    UInt32 GetClientPeerId( void );
    const CORE::CString& GetSessionName( void ) const;

    /** The client leaves the session: the link to the serving node ends */
    void DropClient( void );

    /** The client joins the session again, waits for the link to the serving node; false when that did not work out in time */
    bool RejoinClient( void );

    DRN::CDRNResourceTransferStats GetServerStats( void );

    /** Makes the client's session known by name in the DRN session registry, for the drn:// form of a source */
    void RegisterClientSession( void );

    private:

    CLoopbackShare( const CLoopbackShare& src );
    CLoopbackShare& operator=( const CLoopbackShare& src );

    DRNTEST::CGroup m_group;
    CORE::CString m_sessionName;
    UInt32 m_serverPeerId;
    bool m_clientOwnThread;
    bool m_started;
    bool m_registered;
};

/** How the archive of a conformance fixture is mounted */
enum TMountRoute
{
    MOUNTROUTE_URI          ,   /**< drn://<session name>/sequencer/files, the session found in the DRN session registry */
    MOUNTROUTE_REGISTERED       /**< "<connection id>|files", the connection registered in code */
};

/** What the archive of a fixture is made with */
class CBridgeMountOptions
{
    public:

    TMountRoute route;
    UInt32 metadataTtlMs;
    UInt32 negativeTtlMs;
    UInt32 maxOpenHandles;
    UInt32 chunkCacheChunks;
    UInt32 readAheadChunks;
    CORE::CString verifyMode;       /**< "", "link", "tree" or "treeIfAvailable" */
    bool shareKnowsDigests;
    bool invalidateOnChange;        /**< the fixture tells the archive when it changes the share behind its back: the change notification that a later phase will deliver */
    bool clientOwnThread;           /**< the client node works on a thread of its own; false: the thread of the test pumps it and makes the calls */

    CBridgeMountOptions( void ) : route( MOUNTROUTE_URI ), metadataTtlMs( 3000 ), negativeTtlMs( 1000 ), maxOpenHandles( 12 ), chunkCacheChunks( 8 ), readAheadChunks( 4 ), verifyMode(), shareKnowsDigests( false ), invalidateOnChange( true ), clientOwnThread( true ) {}
};

/**
 *  The conformance fixture of the bridge archive: the reference tree is written to a directory by the fixture of the file system archive (whose tree writer
 *  and whose changes behind the archive this one reuses), that directory is shared by a serving node, and the archive of type "drn" is made by the VFS
 *  factory and mounted on the client node's session
 */
class CDrnBridgeConformanceFixture : public CArchiveConformanceFixture
{
    public:

    CDrnBridgeConformanceFixture( const CORE::CString& name, const CBridgeMountOptions& options );

    virtual ~CDrnBridgeConformanceFixture() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Setup( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Teardown( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual VFS::CArchive* GetArchive( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CArchiveConformanceTree& GetTree( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CArchiveConformanceCapabilities& GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool PutFile( const CORE::CString& path, const CORE::CDynamicBuffer& content ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RemoveFile( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetPathOutsideArchive( CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;

    VFSPLUGIN::DRNBRIDGE::CDrnBridgeArchive* GetBridge( void );

    CLoopbackShare* GetShare( void );

    const CORE::CString& GetRootDir( void ) const;

    private:

    CDrnBridgeConformanceFixture( const CDrnBridgeConformanceFixture& src );
    CDrnBridgeConformanceFixture& operator=( const CDrnBridgeConformanceFixture& src );

    void NoteChange( const CORE::CString& path );

    CORE::CString m_name;
    CBridgeMountOptions m_options;
    CFileSystemArchiveConformanceFixture m_local;
    CArchiveConformanceCapabilities m_capabilities;
    CLoopbackShare* m_share;
    VFS::CVFS::TArchivePtr m_archive;
    CORE::CString m_connectionId;
};

/*-------------------------------------------------------------------------*/

/** A mount of the bridge archive through the VFS factory, made from a configuration node as an application's configuration would make it */
VFS::CVFS::TArchivePtr
CreateBridgeArchive( const CORE::CString& archiveName, const CORE::CString& source, const CBridgeMountOptions& options, const CORE::CString& extraSettingKey = CORE::CString::Empty, const CORE::CString& extraSettingValue = CORE::CString::Empty );

/** The settings of the configuration form, for MountArchive() of the VFS */
VFS::CArchiveSettings
MakeBridgeArchiveSettings( const CORE::CString& archiveName, const CORE::CString& source, const CORE::CString& mountPath, const CBridgeMountOptions& options, const CORE::CString& extraSettingKey = CORE::CString::Empty, const CORE::CString& extraSettingValue = CORE::CString::Empty );

}; /* namespace DRNBRIDGETEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEFIXTURE_H ? */
