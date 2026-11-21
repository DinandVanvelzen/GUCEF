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

#ifndef GUCEF_VFS_TESTAPP_TESTREMOTEARCHIVEFIXTURE_H
#define GUCEF_VFS_TESTAPP_TESTREMOTEARCHIVEFIXTURE_H

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
#include "TestRemoteBackend.h"
#include "TestRemoteArchive.h"
#include "TestVfsBlockCacheUtils.h"
#include "TestArchiveConformanceFixture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The environment of the cases of the remote archive suite: a test place with a small tree, a block cache in a folder of
 *  its own, a scheduler with a pool of its own, and the test remote archive mounted in the VFS at "/remote/". The events
 *  of the VFS are recorded. The test thread is the thread that pumps the pulse generator, as the main thread of a single
 *  thread application is: events are delivered when a case calls Pump() (or waits for one).
 */
class CRemoteTestEnv : public CRemoteEnv
{
    public:

    explicit CRemoteTestEnv( const char* name );

    virtual ~CRemoteTestEnv() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Setup( const SRemoteEnvOptions& options = SRemoteEnvOptions() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Teardown( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Unmounts the archive and mounts it again: a new archive and a new core on the same place. With 'reopenCache' the cache is closed and opened again on its folder (a restart of the process) */
    virtual bool Restart( bool reopenCache ) GUCEF_VIRTUAL_OVERRIDE;

    /** Unmounts the archive; the events of the archive stop */
    virtual bool Unmount( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Lets go of the cache (its folder is closed); the files stay. Restart() with 'reopenCache' opens it again */
    virtual void CloseCache( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CTestRemoteBackend& Backend( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::VFS::CVfsRemoteArchiveCore* Core( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::VFS::CVfsBlockCachePtr Cache( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::VFS::CVfsFetchScheduler& Scheduler( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CTestEventRecorder& Events( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const GUCEF::CORE::CString& GetCacheFolder( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const SRemoteEnvOptions& GetOptions( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRemoteTestEnv( const CRemoteTestEnv& src );
    CRemoteTestEnv& operator=( const CRemoteTestEnv& src );

    bool OpenCache( void );
    bool MountArchive( void );
    void RegisterPlaceNow( void );

    private:

    GUCEF::CORE::CString m_name;
    SRemoteEnvOptions m_options;
    CBlockCacheTestFolder* m_folder;
    CTestRemoteBackendPtr m_backend;
    GUCEF::VFS::CVfsBlockCachePtr m_cache;
    GUCEF::VFS::CVfsFetchScheduler* m_scheduler;
    CTestRemoteArchive::TFactory m_factory;
    CTestEventRecorder m_events;
    bool m_factoryRegistered;
    bool m_setUp;
};

/*-------------------------------------------------------------------------*/

/**
 *  The reference tree of the archive conformance suite behind the test place, for the generic suite
 *  (PerformArchiveConformanceSuite()): the remote archive runs the same cases the file system archive does. The metadata of
 *  the archive is not kept (time to live 0) so that the changes the suite makes behind the archive show at once.
 */
class CRemoteArchiveConformanceFixture : public CArchiveConformanceFixture
{
    public:

    CRemoteArchiveConformanceFixture( void );

    virtual ~CRemoteArchiveConformanceFixture() GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Setup( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Teardown( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::VFS::CArchive* GetArchive( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CArchiveConformanceTree& GetTree( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CArchiveConformanceCapabilities& GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool PutFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CDynamicBuffer& content ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RemoveFile( const GUCEF::CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRemoteArchiveConformanceFixture( const CRemoteArchiveConformanceFixture& src );
    CRemoteArchiveConformanceFixture& operator=( const CRemoteArchiveConformanceFixture& src );

    CRemoteTestEnv* m_env;
    CArchiveConformanceTree m_tree;
    CArchiveConformanceCapabilities m_capabilities;
    GUCEF::VFS::CArchive* m_archive;
    GUCEF::CORE::CString::StringSet m_mutated;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTREMOTEARCHIVEFIXTURE_H ? */
