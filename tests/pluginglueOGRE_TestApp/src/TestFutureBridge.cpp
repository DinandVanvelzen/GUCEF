/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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

#include "OgreRoot.h"
#include "OgreArchiveManager.h"
#include "OgreResourceGroupManager.h"
#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMeshSerializer.h"
#include "OgreHardwareBufferManager.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreDataStream.h"
#include "OgreException.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H
#include "pluginglueOGRECOMMON_COgreIOAccessArchive.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_H
#include "pluginglueOGRE.h"
#define GUCEF_PLUGINGLUE_OGRE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_H ? */

#include "TestFutureBridge.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using PLUGINGLUE::OGRE::COgreFutureBridge;
using PLUGINGLUE::OGRE::COgreTaskWorkQueue;
using PLUGINGLUE::OGRECOMMON::COgreIOAccessArchive;

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

static const char* BridgeTestGroup = "OgreBridgeTestGroup";
static const char* BridgeTestLocation = "bridgememory";

/*-------------------------------------------------------------------------*/

namespace {

/** Records the finished requests the bridge tells of, with the thread that told */
class CBridgeEventRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CBridgeEventRecorder > TEventCallback;

    GUCEF::vector< UInt32 > taskIds;
    GUCEF::vector< TThreadId > threads;

    CBridgeEventRecorder( void ) : CORE::CObserver(), taskIds(), threads() {}

    virtual ~CBridgeEventRecorder()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    void Attach( COgreFutureBridge& bridge )
    {GUCEF_TRACE;

        TEventCallback callback( this, &CBridgeEventRecorder::OnRequestFinished );
        SubscribeTo( &bridge, COgreFutureBridge::RequestFinishedEvent, callback );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CBridgeEventRecorder";
        return classTypeName;
    }

    private:

    void OnRequestFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        if ( GUCEF_NULL != eventData )
            taskIds.push_back( static_cast< COgreFutureBridge::TRequestFinishedEventData* >( eventData )->GetData() );
        threads.push_back( MT::GetCurrentThreadID() );
    }
};

/*-------------------------------------------------------------------------*/

/** Records on which thread Ogre tells a resource listener that the resource was loaded */
class CLoadListener : public Ogre::Resource::Listener
{
    public:

    CLoadListener( void ) : loaded( 0 ), thread( 0 ) {}

    virtual void loadingComplete( Ogre::Resource* ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++loaded;
        thread = MT::GetCurrentThreadID();
    }

    UInt32 loaded;
    TThreadId thread;
};

/*-------------------------------------------------------------------------*/

/** The content of a .mesh file with one triangle, made with Ogre's own serializer */
bool
MakeTriangleMeshFile( CORE::CDynamicBuffer& meshFile )
{GUCEF_TRACE;

    Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual( "BridgeTestSource.mesh", Ogre::RGN_INTERNAL );
    Ogre::HardwareBufferManager& buffers = Ogre::HardwareBufferManager::getSingleton();

    mesh->sharedVertexData = OGRE_NEW Ogre::VertexData();
    mesh->sharedVertexData->vertexCount = 3;
    mesh->sharedVertexData->vertexDeclaration->addElement( 0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION );
    Ogre::HardwareVertexBufferSharedPtr vertices = buffers.createVertexBuffer( 3 * sizeof( float ), 3, Ogre::HBU_CPU_ONLY );
    const float positions[ 9 ] = { 0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f };
    vertices->writeData( 0, sizeof( positions ), positions, true );
    mesh->sharedVertexData->vertexBufferBinding->setBinding( 0, vertices );

    Ogre::SubMesh* subMesh = mesh->createSubMesh();
    subMesh->useSharedVertices = true;
    subMesh->indexData->indexCount = 3;
    subMesh->indexData->indexBuffer = buffers.createIndexBuffer( Ogre::HardwareIndexBuffer::IT_16BIT, 3, Ogre::HBU_CPU_ONLY );
    const Ogre::uint16 indices[ 3 ] = { 0, 1, 2 };
    subMesh->indexData->indexBuffer->writeData( 0, sizeof( indices ), indices, true );

    mesh->_setBounds( Ogre::AxisAlignedBox( 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f ) );
    mesh->_setBoundingSphereRadius( 1.0f );
    mesh->load();

    Ogre::MemoryDataStream* memory = OGRE_NEW Ogre::MemoryDataStream( 64 * 1024, true, false );
    Ogre::DataStreamPtr stream( memory );
    Ogre::MeshSerializer serializer;
    serializer.exportMesh( mesh.get(), stream );
    const size_t size = stream->tell();
    meshFile.CopyFrom( (UInt32) size, memory->getPtr() );

    Ogre::MeshManager::getSingleton().remove( mesh );
    return size > 0;
}

/*-------------------------------------------------------------------------*/

CORE::IOAccessPtr
MakeResource( const CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    CORE::CDynamicBuffer* buffer = GUCEF_NEW CORE::CDynamicBuffer( content );
    return CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( buffer, true ) );
}

/*-------------------------------------------------------------------------*/

/** Runs the main thread tasks of the Root's queue, as the frames of a window do, until the future is ready */
bool
PumpUntilReady( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{GUCEF_TRACE;

    const UInt64 start = MT::PrecisionTickCount();
    while ( !future.IsReady() )
    {
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > timeoutInMs )
            return false;
        Ogre::Root::getSingleton().getWorkQueue()->processMainThreadTasks();
        MT::PrecisionDelay( 2 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::TTaskStatus
StatusOf( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CTaskPtr task = future.GetResult( 0 );
    return task.IsNULL() ? CORE::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

}

/*-------------------------------------------------------------------------*/

static void
TestLoadInTheBackground( COgreTaskWorkQueue& queue, COgreFutureBridge& bridge )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Future bridge: a mesh from memory (GucefIOAccess archive) is read on a worker thread and loaded on the render thread, where its future is completed and the event told" )
        try
        {
            CBridgeEventRecorder recorder;
            recorder.Attach( bridge );
            CLoadListener listener;

            Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().create( "triangle.mesh", BridgeTestGroup );
            mesh->addListener( &listener );
            ASSERT_FALSE( mesh->isLoaded() );

            const UInt64 tasksRunBefore = queue.GetCore()->GetNrOfTasksRun();
            CORE::CFutureResult future = bridge.LoadResourceAsync( mesh );
            ASSERT_TRUE( future.HasAFuture() );

            // the background part runs on the pool, the rest waits for the render thread
            const UInt64 start = MT::PrecisionTickCount();
            while ( queue.GetCore()->GetNrOfTasksRun() == tasksRunBefore && MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 10000 )
                MT::PrecisionDelay( 2 );
            ASSERT_TRUE( queue.GetCore()->GetNrOfTasksRun() == tasksRunBefore + 1 );
            ASSERT_TRUE( 0 == queue.GetCore()->GetNrOfTasksFailed() );
            MT::PrecisionDelay( 20 );
            ASSERT_FALSE( future.IsReady() );
            ASSERT_TRUE( mesh->isPrepared() );
            ASSERT_FALSE( mesh->isLoaded() );
            ASSERT_TRUE( recorder.taskIds.empty() );

            ASSERT_TRUE( PumpUntilReady( future, 10000 ) );
            ASSERT_TRUE( CORE::TASKSTATUS_FINISHED == StatusOf( future ) );
            ASSERT_TRUE( mesh->isLoaded() );
            ASSERT_TRUE( 3 == mesh->sharedVertexData->vertexCount );
            ASSERT_TRUE( 1 == mesh->getNumSubMeshes() && 3 == mesh->getSubMesh( 0 )->indexData->indexCount );
            ASSERT_TRUE( 1 == listener.loaded && MT::GetCurrentThreadID() == listener.thread );
            ASSERT_TRUE( 1 == recorder.taskIds.size() && future.GetTaskId() == recorder.taskIds[ 0 ] );
            ASSERT_TRUE( MT::GetCurrentThreadID() == recorder.threads[ 0 ] );

            mesh->removeListener( &listener );
            Ogre::MeshManager::getSingleton().remove( mesh );
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "pluginglueOGRE_TestApp: " << e.getFullDescription() );
            ERRORHERE;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestSynchronousForms( COgreTaskWorkQueue& queue, COgreFutureBridge& bridge )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Future bridge: the synchronous forms do the same work on the calling thread, without the work queue" )
        try
        {
            const UInt64 tasksRunBefore = queue.GetCore()->GetNrOfTasksRun();
            const UInt64 mainTasksRunBefore = queue.GetCore()->GetNrOfMainThreadTasksRun();
            CLoadListener listener;
            Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().create( "triangle.mesh", BridgeTestGroup );
            mesh->addListener( &listener );
            ASSERT_TRUE( bridge.LoadResource( mesh ) );
            ASSERT_TRUE( mesh->isLoaded() && 3 == mesh->sharedVertexData->vertexCount );
            ASSERT_TRUE( 1 == listener.loaded && MT::GetCurrentThreadID() == listener.thread );
            ASSERT_TRUE( tasksRunBefore == queue.GetCore()->GetNrOfTasksRun() );
            ASSERT_TRUE( mainTasksRunBefore == queue.GetCore()->GetNrOfMainThreadTasksRun() );
            mesh->removeListener( &listener );
            Ogre::MeshManager::getSingleton().remove( mesh );

            Ogre::MeshPtr prepared = Ogre::MeshManager::getSingleton().create( "triangle.mesh", BridgeTestGroup );
            ASSERT_TRUE( bridge.PrepareResource( prepared ) );
            ASSERT_TRUE( prepared->isPrepared() && !prepared->isLoaded() );
            Ogre::MeshManager::getSingleton().remove( prepared );

            // a resource that is not there fails, and says so
            Ogre::MeshPtr missing = Ogre::MeshManager::getSingleton().create( "missing.mesh", BridgeTestGroup );
            ASSERT_FALSE( bridge.LoadResource( missing ) );
            Ogre::MeshManager::getSingleton().remove( missing );
            ASSERT_FALSE( bridge.LoadResource( Ogre::ResourcePtr() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestFailureCancellationAndDrop( COgreTaskWorkQueue& queue, COgreFutureBridge& bridge )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Future bridge: a failed load ends the future with the reason, a cancellation asked for ends it cancelled, and a request the queue drops ends cancelled" )
        try
        {
            CBridgeEventRecorder recorder;
            recorder.Attach( bridge );

            // the file is not there
            Ogre::MeshPtr missing = Ogre::MeshManager::getSingleton().create( "missing.mesh", BridgeTestGroup );
            CORE::CFutureResult failed = bridge.LoadResourceAsync( missing );
            ASSERT_TRUE( PumpUntilReady( failed, 10000 ) );
            ASSERT_TRUE( CORE::TASKSTATUS_EXCEPTION_DURING_RUN == StatusOf( failed ) );
            ASSERT_TRUE( failed.HasNoFuture() );
            ASSERT_FALSE( failed.GetResult( 0 )->GetTaskStatusExtraInfo().IsNULLOrEmpty() );
            ASSERT_TRUE( 1 == recorder.taskIds.size() && failed.GetTaskId() == recorder.taskIds[ 0 ] );
            Ogre::MeshManager::getSingleton().remove( missing );

            // cancelled while it waits in the paused queue, and dropped by the queue before it ran:
            // a queue without worker threads runs the background part at once, so nothing waits
            Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().create( "triangle.mesh", BridgeTestGroup );
            if ( !queue.IsRunningTasksOnCallingThread() )
            {
                queue.setPaused( true );
                CORE::CFutureResult cancelled = bridge.LoadResourceAsync( mesh );
                ASSERT_TRUE( cancelled.RequestCancellation() );
                queue.setPaused( false );
                ASSERT_TRUE( PumpUntilReady( cancelled, 10000 ) );
                ASSERT_TRUE( CORE::TASKSTATUS_CANCELLED == StatusOf( cancelled ) );
                ASSERT_FALSE( mesh->isLoaded() );
                ASSERT_TRUE( 2 == recorder.taskIds.size() );

                queue.setPaused( true );
                CORE::CFutureResult dropped = bridge.LoadResourceAsync( mesh );
                ASSERT_FALSE( dropped.IsReady() );
                ASSERT_TRUE( 1 == queue.AbortQueuedTasks() );
                ASSERT_TRUE( dropped.IsReady() );
                ASSERT_TRUE( CORE::TASKSTATUS_CANCELLED == StatusOf( dropped ) );
                queue.setPaused( false );
                ASSERT_FALSE( mesh->isLoaded() );
                ASSERT_TRUE( 2 == recorder.taskIds.size() );
            }
            Ogre::MeshManager::getSingleton().remove( mesh );

            // a queue that takes no tasks: the request fails at once
            queue.setRequestsAccepted( false );
            CORE::CFutureResult refused = bridge.LoadResourceGroupAsync( BridgeTestGroup );
            ASSERT_TRUE( refused.IsReady() && CORE::TASKSTATUS_QUEUEING_FAILED == StatusOf( refused ) );
            queue.setRequestsAccepted( true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestGroupLoad( COgreFutureBridge& bridge )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Future bridge: a resource group is prepared, then loaded, through futures" )
        try
        {
            Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
            Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().create( "triangle.mesh", BridgeTestGroup );
            groups.initialiseResourceGroup( BridgeTestGroup );

            CORE::CFutureResult prepared = bridge.PrepareResourceGroupAsync( BridgeTestGroup );
            ASSERT_TRUE( PumpUntilReady( prepared, 10000 ) );
            ASSERT_TRUE( CORE::TASKSTATUS_FINISHED == StatusOf( prepared ) );
            ASSERT_TRUE( mesh->isPrepared() );

            CORE::CFutureResult loaded = bridge.LoadResourceGroupAsync( BridgeTestGroup );
            ASSERT_TRUE( PumpUntilReady( loaded, 10000 ) );
            ASSERT_TRUE( CORE::TASKSTATUS_FINISHED == StatusOf( loaded ) );
            ASSERT_TRUE( mesh->isLoaded() );
            Ogre::MeshManager::getSingleton().remove( mesh );

            CORE::CFutureResult unknown = bridge.LoadResourceGroupAsync( "NoSuchGroup" );
            ASSERT_TRUE( PumpUntilReady( unknown, 10000 ) );
            ASSERT_TRUE( CORE::TASKSTATUS_EXCEPTION_DURING_RUN == StatusOf( unknown ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformFutureBridgeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "pluginglueOGRE.FutureBridge" );

    COgreTaskWorkQueue* queue = PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup().GetTaskWorkQueue();
    Ogre::DefaultHardwareBufferManager* bufferManager = GUCEF_NULL;
    bool setUp = false;
    GUCEF_TESTFW_TESTCASE( "Future bridge: set up, a mesh file in memory served by the GucefIOAccess archive, and the running queue of the Root" )
        try
        {
            // the Root starts its queue with the first window, here there is none
            if ( GUCEF_NULL != queue && !queue->IsRunning() )
                queue->startup();
            ASSERT_TRUE( GUCEF_NULL != queue && queue->IsRunning() );
            ASSERT_TRUE( Ogre::ArchiveManager::getSingleton().hasArchiveFactory( "GucefIOAccess" ) );
            if ( GUCEF_NULL == queue || !queue->IsRunning() || !Ogre::ArchiveManager::getSingleton().hasArchiveFactory( "GucefIOAccess" ) )
                GUCEF_TESTFW_SKIP( "the queue of the Root does not run or OgrePlugin_GucefVFS is not loaded" );

            // meshes need buffers: in memory, there is no render system here
            if ( GUCEF_NULL == Ogre::HardwareBufferManager::getSingletonPtr() )
                bufferManager = OGRE_NEW Ogre::DefaultHardwareBufferManager();

            CORE::CDynamicBuffer meshFile;
            ASSERT_TRUE( MakeTriangleMeshFile( meshFile ) );
            Ogre::Archive* archive = Ogre::ArchiveManager::getSingleton().load( BridgeTestLocation, "GucefIOAccess", true );
            COgreIOAccessArchive* memory = COgreIOAccessArchive::FromArchive( archive, "GucefIOAccess" );
            ASSERT_TRUE( GUCEF_NULL != memory );
            if ( GUCEF_NULL != memory )
                ASSERT_TRUE( memory->AddResource( "triangle.mesh", MakeResource( meshFile ) ) );

            Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
            groups.createResourceGroup( BridgeTestGroup );
            groups.addResourceLocation( BridgeTestLocation, "GucefIOAccess", BridgeTestGroup, true );
            ASSERT_TRUE( groups.resourceExists( BridgeTestGroup, "triangle.mesh" ) );
            setUp = true;
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "pluginglueOGRE_TestApp: " << e.getFullDescription() );
            ERRORHERE;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( setUp )
    {
        COgreFutureBridge& bridge = PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetFutureBridge();
        TestLoadInTheBackground( *queue, bridge );
        TestSynchronousForms( *queue, bridge );
        TestFailureCancellationAndDrop( *queue, bridge );
        TestGroupLoad( bridge );
    }

    GUCEF_TESTFW_TESTCASE( "Future bridge: shut down of the Root with a request in flight cancels its future, after the tasks were dropped" )
        try
        {
            if ( !setUp )
                GUCEF_TESTFW_SKIP( "not set up" );

            // a request whose main thread part never gets a frame: the Root goes first
            COgreFutureBridge& bridge = PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetFutureBridge();
            Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().create( "triangle.mesh", BridgeTestGroup );
            CORE::CFutureResult inFlight = bridge.LoadResourceAsync( mesh );
            mesh.reset();
            const UInt64 start = MT::PrecisionTickCount();
            while ( 0 == queue->GetCore()->GetNrOfQueuedMainThreadTasks() && MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 10000 )
                MT::PrecisionDelay( 2 );
            ASSERT_TRUE( 1 == queue->GetCore()->GetNrOfQueuedMainThreadTasks() );
            ASSERT_FALSE( inFlight.IsReady() );

            // as Root::shutdown() does first
            queue->shutdown();
            ASSERT_TRUE( inFlight.IsReady() );
            ASSERT_TRUE( CORE::TASKSTATUS_CANCELLED == StatusOf( inFlight ) );

            Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup( BridgeTestGroup );
            queue->startup();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( GUCEF_NULL != bufferManager )
        OGRE_DELETE bufferManager;
}

/*-------------------------------------------------------------------------*/
