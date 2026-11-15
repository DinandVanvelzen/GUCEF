/*
 *  guidriverOgre_TestApp: Test application for the window backend Ogre (guidriverOgre)
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

#include <stdio.h>
#include <string.h>

#if ( defined( __linux__ ) )
#include <dirent.h>
#include <dlfcn.h>
#endif

#include <Ogre.h>
#include <OgreCodec.h>
#include <OgreRenderWindow.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreWorkQueue.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_H
#include "pluginglueOGRE.h"
#define GUCEF_PLUGINGLUE_OGRE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_H ? */

#include "TestOgreSupport.h"
#include "TestOgreWorkQueueWindow.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using PLUGINGLUE::OGRE::COgreTaskWorkQueue;
using PLUGINGLUE::OGRE::COgreFutureBridge;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** What a task saw on the thread it ran on */
class CThreadProbe
{
    public:

    CThreadProbe( void ) : lock(), ran( 0 ), thread( 0 ), registered( false ), glContext( GUCEF_NULL ) {}

    UInt32 Ran( void ) const
    {
        MT::CScopeMutex scope( lock );
        return ran;
    }

    mutable MT::CMutex lock;
    UInt32 ran;
    TThreadId thread;
    bool registered;
    void* glContext;
};

/*-------------------------------------------------------------------------*/

/** The current EGL context of the calling thread, NULL when there is none or EGL is not loaded */
void*
CurrentEglContext( void )
{GUCEF_TRACE;

    #if ( defined( __linux__ ) )
    // EGL is loaded by the render system plugin, with a local scope: asked for by its handle
    void* egl = ::dlopen( "libEGL.so.1", RTLD_NOW | RTLD_NOLOAD );
    if ( GUCEF_NULL == egl )
        return GUCEF_NULL;
    typedef void* (*TGetCurrentContext)( void );
    TGetCurrentContext getCurrentContext = (TGetCurrentContext) ::dlsym( egl, "eglGetCurrentContext" );
    void* context = GUCEF_NULL != getCurrentContext ? getCurrentContext() : GUCEF_NULL;
    ::dlclose( egl );
    return context;
    #else
    return GUCEF_NULL;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Lets go of the EGL context and surface of the calling thread */
void
ReleaseEglContext( void )
{GUCEF_TRACE;

    #if ( defined( __linux__ ) )
    void* egl = ::dlopen( "libEGL.so.1", RTLD_NOW | RTLD_NOLOAD );
    if ( GUCEF_NULL == egl )
        return;
    typedef void* (*TGetCurrentDisplay)( void );
    typedef unsigned int (*TMakeCurrent)( void*, void*, void*, void* );
    TGetCurrentDisplay getCurrentDisplay = (TGetCurrentDisplay) ::dlsym( egl, "eglGetCurrentDisplay" );
    TMakeCurrent makeCurrent = (TMakeCurrent) ::dlsym( egl, "eglMakeCurrent" );
    if ( GUCEF_NULL != getCurrentDisplay && GUCEF_NULL != makeCurrent )
    {
        void* display = getCurrentDisplay();
        if ( GUCEF_NULL != display )
            makeCurrent( display, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL );
    }
    ::dlclose( egl );
    #endif
}

/*-------------------------------------------------------------------------*/

/** A task that lets go of the EGL context of its thread */
class CReleaseEglTask
{
    public:

    explicit CReleaseEglTask( CThreadProbe& probe ) : m_probe( &probe ) {}

    void operator()( void )
    {
        ReleaseEglContext();
        MT::CScopeMutex lock( m_probe->lock );
        ++m_probe->ran;
        m_probe->glContext = CurrentEglContext();
    }

    private:

    CThreadProbe* m_probe;
};

/*-------------------------------------------------------------------------*/

/** Records the thread it runs on, whether the queue registered it with the render system, and its GL context */
class CProbeTask
{
    public:

    CProbeTask( CThreadProbe& probe, COgreTaskWorkQueue& queue ) : m_probe( &probe ), m_queue( &queue ) {}

    void operator()( void )
    {
        const bool registered = m_queue->GetCore()->IsCurrentThreadRegisteredWithRenderSystem();
        void* context = CurrentEglContext();
        MT::CScopeMutex lock( m_probe->lock );
        ++m_probe->ran;
        m_probe->thread = MT::GetCurrentThreadID();
        m_probe->registered = registered;
        m_probe->glContext = context;
    }

    private:

    CThreadProbe* m_probe;
    COgreTaskWorkQueue* m_queue;
};

/*-------------------------------------------------------------------------*/

bool
WaitForProbe( const CThreadProbe& probe, UInt32 count )
{GUCEF_TRACE;

    const UInt64 start = MT::PrecisionTickCount();
    while ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 10000 )
    {
        if ( probe.Ran() >= count )
            return true;
        MT::PrecisionDelay( 2 );
    }
    return false;
}

}

/*-------------------------------------------------------------------------*/

UInt32
CountProcessThreadsNamed( const char* namePrefix )
{GUCEF_TRACE;

    #if ( defined( __linux__ ) )
    UInt32 count = 0;
    DIR* tasks = ::opendir( "/proc/self/task" );
    if ( GUCEF_NULL == tasks )
        return 0;
    const size_t prefixLength = strlen( namePrefix );
    struct dirent* entry = GUCEF_NULL;
    while ( GUCEF_NULL != ( entry = ::readdir( tasks ) ) )
    {
        if ( '.' == entry->d_name[ 0 ] )
            continue;
        char path[ 300 ];
        snprintf( path, sizeof( path ), "/proc/self/task/%s/comm", entry->d_name );
        FILE* file = fopen( path, "r" );
        if ( GUCEF_NULL == file )
            continue;
        char name[ 64 ];
        memset( name, 0, sizeof( name ) );
        if ( GUCEF_NULL != fgets( name, sizeof( name ), file ) && 0 == strncmp( name, namePrefix, prefixLength ) )
            ++count;
        fclose( file );
    }
    ::closedir( tasks );
    return count;
    #else
    return 0;
    #endif
}

/*-------------------------------------------------------------------------*/

void
PerformOgreWorkQueueWindowTests( GUIDRIVEROGRE::COgreWindowContext* window )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "guidriverOgre.WorkQueue" );

    COgreTaskWorkQueue* queue = PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup().GetTaskWorkQueue();

    GUCEF_TESTFW_TESTCASE( "Work queue: the first render window started the queue of the Root on the GUCEF thread pool, and Ogre made no worker threads of its own" )
    try
    {
        ASSERT_TRUE( GUCEF_NULL != queue && Ogre::Root::getSingleton().getWorkQueue() == queue );
        if ( GUCEF_NULL == queue )
            GUCEF_TESTFW_SKIP( "the Root has another work queue" );
        ASSERT_TRUE( queue->IsRunning() && !queue->IsRunningTasksOnCallingThread() );
        CORE::ThreadPoolPtr pool = queue->GetThreadPool();
        ASSERT_TRUE( !pool.IsNULL() && "OgreWork" == pool->GetThreadPoolName() );
        ASSERT_TRUE( 2 == queue->GetCore()->GetNrOfWorkers() );

        #if ( defined( __linux__ ) )
        // Ogre names its own workers; the threads of the pool carry the name of the pool
        ASSERT_TRUE( 0 == CountProcessThreadsNamed( "[Ogre] Worker" ) );
        ASSERT_TRUE( CountProcessThreadsNamed( "OgreWork" ) >= 2 );
        #endif
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    if ( GUCEF_NULL == queue )
        return;

    GUCEF_TESTFW_TESTCASE( "Work queue: main thread tasks run in the frames of the window, at the end of the frame, on the render thread" )
    try
    {
        CThreadProbe probe;
        queue->addMainThreadTask( CProbeTask( probe, *queue ) );
        MT::PrecisionDelay( 20 );
        ASSERT_TRUE( 0 == probe.Ran() );
        ASSERT_TRUE( window->RenderFrame() );
        ASSERT_TRUE( 1 == probe.Ran() && MT::GetCurrentThreadID() == probe.thread );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Future bridge: a texture is read through the GucefVFS archive and decoded by gucefIMAGE on a worker thread, made on the render thread in a frame, and has its pixels" )
    if ( !Ogre::ArchiveManager::getSingleton().hasArchiveFactory( "GucefVFS" ) || !Ogre::Codec::isCodecRegistered( "ogwtex" ) )
        GUCEF_TESTFW_SKIP( "OgrePlugin_GucefVFS or OgreCodec_GucefIMAGE is not loaded" );
    try
    {
        const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "guidriverOgre_TestApp_WorkQueue" );
        CORE::DeleteDir( dir, true );
        ASSERT_TRUE( CORE::CreateDirs( dir ) );
        {
            CORE::CFileAccess file( CORE::CombinePath( dir, "green.ogwtex" ), "wb" );
            const UInt8 data[ 8 ] = { 'O', 'G', 'W', '1', 0, 255, 0, 255 };
            ASSERT_TRUE( file.IsValid() && 1 == file.Write( data, 8, 1 ) );
        }
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.AddRoot( dir, "guidriverOgreTestWorkQueue", false, false, "/ogreworkqueuetest" ) );
        Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
        groups.createResourceGroup( "OgreWorkQueueTest", false );
        groups.addResourceLocation( "/ogreworkqueuetest", "GucefVFS", "OgreWorkQueueTest", false, true );
        groups.initialiseResourceGroup( "OgreWorkQueueTest" );

        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().create( "green.ogwtex", "OgreWorkQueueTest" );
        const UInt64 tasksBefore = queue->GetCore()->GetNrOfTasksRun();
        COgreFutureBridge& bridge = PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetFutureBridge();
        CORE::CFutureResult future = bridge.LoadResourceAsync( texture );

        // decoded in the background: prepared, not yet a GPU texture
        const UInt64 start = MT::PrecisionTickCount();
        while ( queue->GetCore()->GetNrOfTasksRun() == tasksBefore && MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 10000 )
            MT::PrecisionDelay( 2 );
        ASSERT_TRUE( queue->GetCore()->GetNrOfTasksRun() == tasksBefore + 1 && 0 == queue->GetCore()->GetNrOfTasksFailed() );
        ASSERT_TRUE( texture->isPrepared() && !texture->isLoaded() );
        ASSERT_FALSE( future.IsReady() );

        // the frame makes it
        ASSERT_TRUE( window->RenderFrame() );
        ASSERT_TRUE( future.IsReady() );
        CORE::CTaskPtr task = future.GetResult( 0 );
        ASSERT_TRUE( !task.IsNULL() && CORE::TASKSTATUS_FINISHED == task->GetTaskStatus() );
        ASSERT_TRUE( texture->isLoaded() && 4 == texture->getWidth() && 4 == texture->getHeight() );

        UInt8 pixels[ 4 * 4 * 4 ];
        memset( pixels, 0, sizeof( pixels ) );
        Ogre::PixelBox box( 4, 4, 1, Ogre::PF_BYTE_RGBA, pixels );
        texture->getBuffer()->blitToMemory( box );
        ASSERT_TRUE( 0 == pixels[ 0 ] && 255 == pixels[ 1 ] && 0 == pixels[ 2 ] && 255 == pixels[ 3 ] );

        Ogre::TextureManager::getSingleton().remove( texture );
        texture.reset();
        groups.destroyResourceGroup( "OgreWorkQueueTest" );
        vfs.UnmountArchiveByName( "guidriverOgreTestWorkQueue" );
        CORE::DeleteDir( dir, true );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "guidriverOgre_TestApp: " << e.getFullDescription() );
        ERRORHERE;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Work queue: worker threads that may use the render system register with it between preExtraThreadsStarted() and postExtraThreadsStarted(), each with a GL context of its own sharing with the main one" )
    try
    {
        // the default of this build: the render system is for the render thread only
        ASSERT_FALSE( queue->GetWorkersCanAccessRenderSystem() );
        ASSERT_TRUE( 0 == queue->GetCore()->GetNrOfThreadsRegisteredWithRenderSystem() );
        void* mainContext = CurrentEglContext();
        Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();

        // One worker thread of a pool of its own: the EGL support of Ogre 14.6.3 makes the context of a registered
        // thread on the window surface of the main context, which only one thread can have current at a time
        queue->shutdown();
        queue->SetThreadPoolName( "OgreWorkRenderSystem" );
        queue->setWorkerThreadCount( 1 );
        queue->SetWorkersCanAccessRenderSystem( true );
        queue->startup();
        ASSERT_TRUE( 1 == queue->GetCore()->GetNrOfThreadsRegisteredWithRenderSystem() );
        ASSERT_TRUE( 1 == queue->GetCore()->GetNrOfWorkers() );
        ASSERT_FALSE( queue->GetCore()->IsCurrentThreadRegisteredWithRenderSystem() );

        CThreadProbe probe;
        queue->addTask( CProbeTask( probe, *queue ) );
        ASSERT_TRUE( WaitForProbe( probe, 1 ) );
        {
            MT::CScopeMutex lock( probe.lock );
            ASSERT_TRUE( probe.registered );
            ASSERT_TRUE( probe.thread != MT::GetCurrentThreadID() );
            if ( GUCEF_NULL != mainContext )
                ASSERT_TRUE( GUCEF_NULL != probe.glContext && mainContext != probe.glContext );
        }

        // the worker lets go of the window surface, the render thread takes its context back and draws
        CThreadProbe released;
        queue->addTask( CReleaseEglTask( released ) );
        ASSERT_TRUE( WaitForProbe( released, 1 ) );
        renderSystem->postExtraThreadsStarted();
        if ( GUCEF_NULL != mainContext )
            ASSERT_TRUE( mainContext == CurrentEglContext() );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( window->RenderFrame( &pixels ) );

        queue->shutdown();
        ASSERT_TRUE( 0 == queue->GetCore()->GetNrOfThreadsRegisteredWithRenderSystem() );
        queue->SetWorkersCanAccessRenderSystem( false );
        queue->SetThreadPoolName( "OgreWork" );
        queue->setWorkerThreadCount( 2 );
        queue->startup();
        ASSERT_TRUE( 0 == queue->GetCore()->GetNrOfThreadsRegisteredWithRenderSystem() );
        ASSERT_TRUE( window->RenderFrame( &pixels ) );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "guidriverOgre_TestApp: " << e.getFullDescription() );
        ERRORHERE;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
