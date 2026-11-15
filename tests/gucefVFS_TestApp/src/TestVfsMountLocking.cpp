/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVfsMountLocking.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A thread that mounts a directory of the file system as an archive, once */
class CMountThread : public MT::CActiveObject
{
    public:

    CMountThread( const CORE::CString& directory, const CORE::CString& archiveName, const CORE::CString& mountPath )
        : MT::CActiveObject()
        , m_directory( directory )
        , m_archiveName( archiveName )
        , m_mountPath( mountPath )
        , m_started( 0 )
        , m_done( 0 )
        , m_mounted( 0 )
    {GUCEF_TRACE;

    }

    virtual ~CMountThread() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        Deactivate( false, true );
    }

    bool IsStarted( void ) const { return 0 != m_started.Load(); }
    bool IsDone( void ) const { return 0 != m_done.Load(); }
    bool IsMounted( void ) const { return 0 != m_mounted.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_started.Store( 1 );
        const bool mounted = VFS::CVfsGlobal::Instance()->GetVfs().AddRoot( m_directory, m_archiveName, false, false, m_mountPath );
        m_mounted.Store( mounted ? 1 : 0 );
        m_done.Store( 1 );
        return true;
    }

    private:

    CORE::CString m_directory;
    CORE::CString m_archiveName;
    CORE::CString m_mountPath;
    MT::CTAtomic< UInt32 > m_started;
    MT::CTAtomic< UInt32 > m_done;
    MT::CTAtomic< UInt32 > m_mounted;
};

UInt64
NowMs( void )
{
    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsMountLockingTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsMountLocking" );

    GUCEF_TESTFW_TESTCASE( "Mount: a file system archive whose directory watcher waits for the pulse generator does not hold the VFS; the thread that holds the pulse generator reads the VFS meanwhile, and the mount completes once it lets go" )
    try
    {
        const CORE::CString directory = CORE::CombinePath( CORE::TempDir(), "gucefvfs-mountlocking-" + CORE::ToString( (UInt32) MT::PrecisionTickCount() ) );
        ASSERT_TRUE( CORE::CreateDirs( directory ) );
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        CORE::PulseGeneratorPtr pulse = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        CMountThread mount( directory, "MountLockingTest", "mountlockingtest" );
        UInt64 readMs = 0;
        bool otherMountFound = true;
        {
            // this thread holds the pulse generator as the pulse thread does while it notifies
            CORE::CNotifierScopeLock pulseLock( pulse.GetPointerAlways() );
            ASSERT_TRUE( mount.Activate( GUCEF_NULL, 1, 1 ) );
            for ( UInt32 waited=0; waited<5000 && !mount.IsStarted(); waited += 5 )
                MT::ThreadDelay( 5 );
            // the mount is now making its archive, whose watcher waits for the lock this thread holds
            MT::ThreadDelay( 300 );
            ASSERT_FALSE( mount.IsDone() );
            const UInt64 start = NowMs();
            otherMountFound = vfs.IsMountedArchive( "/nosuchmountlockingtest/" );
            readMs = NowMs() - start;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "VfsMountLocking: a read of the VFS while a mount waited for the pulse generator took " << readMs << " ms" );
        ASSERT_FALSE( otherMountFound );
        ASSERT_TRUE( readMs < 5000 );

        // let go: the mount completes
        for ( UInt32 waited=0; waited<30000 && !mount.IsDone(); waited += 5 )
            MT::ThreadDelay( 5 );
        ASSERT_TRUE( mount.IsDone() && mount.IsMounted() );
        ASSERT_TRUE( vfs.UnmountArchiveByName( "MountLockingTest" ) );
        CORE::DeleteDir( directory, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
