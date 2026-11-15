/*
 *  gucefCOMPUTE_TestApp: GUCEF test application for the Compute module
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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#include "TestComputeSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {
namespace TEST {

/*-------------------------------------------------------------------------*/

const char* GateKernelId = "COMPUTETEST.Gate.U32";
const char* CTestComputeBackend::Name = "testgpu";

namespace {

MT::CTAtomic< UInt32 > g_gateOpen( 1 );
MT::CTAtomic< UInt32 > g_gateEntered( 0 );
class CGateKernel : public CCpuComputeKernel
{
    public:
    virtual void ExecuteGroups( const CCpuDispatchContext& context, UInt32 firstGroup, UInt32 groupCount ) const GUCEF_VIRTUAL_OVERRIDE
    {
        g_gateEntered.Store( 1, MT::MEMORDER_RELEASE );
        while ( 0 == g_gateOpen.Load( MT::MEMORDER_ACQUIRE ) )
            MT::PrecisionDelay( 1 );
        UInt32* flag = context.GetBufferAs< UInt32 >( 0 );
        flag[ 0 ] = 1;
    }
};

CCpuTestKernels* g_testKernels = GUCEF_NULL;
CORE::CTFactory< CCpuComputeKernel, CGateKernel, MT::CMutex >* g_gateFactory = GUCEF_NULL;
int g_gateOwner = 0;

/** Runs one test body; the consumer ends after one cycle */
class CTestThreadTask : public CORE::CTaskConsumer
{
    public:

    explicit CTestThreadTask( CTestThreadBody* body ) : CORE::CTaskConsumer(), done( 0 ), m_body( body ) {}

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "GUCEF::COMPUTE::TEST::CTestThreadTask"; }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CString typeName = "GUCEF::COMPUTE::TEST::CTestThreadTask";
        return typeName;
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        try
        {
            m_body->Run();
        }
        catch ( ... )
        {
        }
        done.Store( 1, MT::MEMORDER_RELEASE );
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        done.Store( 1, MT::MEMORDER_RELEASE );
    }

    MT::CTAtomic< UInt32 > done;

    private:

    CTestThreadBody* m_body;
};

typedef CORE::CTSharedPtr< CTestThreadTask, MT::CMutex > TTestThreadTaskPtr;

}

/*-------------------------------------------------------------------------*/

bool
RegisterComputeTestKernels( void )
{
    CComputeKernelRegistry& registry = CComputeGlobal::Instance()->GetKernelRegistry();
    if ( GUCEF_NULL == g_testKernels )
    {
        g_testKernels = GUCEF_NEW CCpuTestKernels();
        if ( !g_testKernels->Register( registry ) )
            return false;
    }
    if ( GUCEF_NULL == g_gateFactory )
    {
        g_gateFactory = GUCEF_NEW CORE::CTFactory< CCpuComputeKernel, CGateKernel, MT::CMutex >();
        CComputeKernelDecl decl( GateKernelId, 1 );
        decl.GetSignature().SetWorkgroupSize( 1, 1, 1 );
        decl.GetSignature().AddBinding( 0, COMPUTEACCESS_WRITE, ELEMENTTYPE_UINT32 );
        decl.SetDeterminism( DETERMINISM_EXACT_INTEGER );
        if ( !registry.RegisterKernel( decl, &g_gateOwner ) ||
             !registry.RegisterArtefact( GateKernelId, CComputeKernelArtefact::ForCpu( g_gateFactory ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
UnregisterComputeTestKernels( void )
{
    CComputeGlobal* global = CComputeGlobal::InstanceIfCreated();
    if ( GUCEF_NULL != global )
        global->GetKernelRegistry().UnregisterModuleKernels( &g_gateOwner );
    GUCEF_DELETE g_testKernels;
    g_testKernels = GUCEF_NULL;
    GUCEF_DELETE g_gateFactory;
    g_gateFactory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

TComputeDevicePtr
CreateCpuTestDevice( UInt32 queueCount, const CORE::PulseGeneratorPtr& eventPulseGenerator )
{
    CComputeDeviceParams params;
    params.SetQueueCount( queueCount );
    params.SetEventPulseGenerator( eventPulseGenerator );
    TComputeDevicePtr device;
    CComputeGlobal::Instance()->GetBackendRegistry().CreateDevice( "cpu", params, device );
    return device;
}

/*-------------------------------------------------------------------------*/

void
CloseGate( void )
{
    g_gateEntered.Store( 0, MT::MEMORDER_RELEASE );
    g_gateOpen.Store( 0, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

void
OpenGate( void )
{
    g_gateOpen.Store( 1, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

bool
WaitUntilGateEntered( UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; ++waited )
    {
        if ( 0 != g_gateEntered.Load( MT::MEMORDER_ACQUIRE ) )
            return true;
        MT::PrecisionDelay( 1 );
    }
    return 0 != g_gateEntered.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

bool
RecordGate( CComputeCommandList& list, TComputeDevicePtr device, const TComputeBufferPtr& flag )
{
    TComputeKernelPtr gate;
    if ( !device->CreateKernel( GateKernelId, gate ) )
        return false;
    CComputeArguments arguments;
    arguments.SetBuffer( 0, flag, 0, 4 );
    return COMPUTERESULT_SUCCESS == list.Dispatch( gate, arguments, CComputeDispatchSize::ForGroups( 1 ) );
}

/*-------------------------------------------------------------------------*/

TComputeBufferPtr
MakeBuffer( TComputeDevicePtr device, UInt64 bytes, TComputeMemoryLocation location )
{
    TComputeBufferPtr buffer;
    device->CreateBuffer( CComputeBufferDesc( bytes, BUFFERUSAGE_STORAGE | BUFFERUSAGE_COPY_SRC | BUFFERUSAGE_COPY_DST, location ), buffer );
    return buffer;
}

/*-------------------------------------------------------------------------*/

bool
WaitForDeviceCount( UInt32 expected, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; ++waited )
    {
        if ( CComputeGlobal::Instance()->GetDeviceCount() == expected )
            return true;
        MT::PrecisionDelay( 1 );
    }
    return CComputeGlobal::Instance()->GetDeviceCount() == expected;
}

/*-------------------------------------------------------------------------*/

bool
AwaitFuture( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs; ++waited )
    {
        if ( future.IsReady() )
            return true;
        MT::PrecisionDelay( 1 );
    }
    return future.IsReady();
}

/*-------------------------------------------------------------------------*/

CTestComputeBackend::CTestComputeBackend( void )
    : CComputeBackend()
    , initializeCalls( 0 )
    , shutdownCalls( 0 )
{
}

/*-------------------------------------------------------------------------*/

CTestComputeBackend::~CTestComputeBackend()
{
}

/*-------------------------------------------------------------------------*/

CString
CTestComputeBackend::GetName( void ) const
{
    return Name;
}

/*-------------------------------------------------------------------------*/

Int32
CTestComputeBackend::GetPriority( void ) const
{
    return 10;
}

/*-------------------------------------------------------------------------*/

bool
CTestComputeBackend::Initialize( void )
{
    initializeCalls.FetchAdd( 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestComputeBackend::Shutdown( void )
{
    shutdownCalls.FetchAdd( 1 );
}

/*-------------------------------------------------------------------------*/

CComputeDeviceInfo
CTestComputeBackend::GetDeviceInfo( void )
{
    CComputeDeviceInfo info = CCpuComputeDevice::GetHostDeviceInfo( false );
    info.SetBackendName( Name );
    info.SetDeviceName( "Test GPU" );
    info.SetDeviceType( DEVICETYPE_DISCRETE_GPU );
    info.SetVendorId( 0x10DE );
    info.SetDeviceId( 0x2C02 );
    return info;
}

/*-------------------------------------------------------------------------*/

bool
CTestComputeBackend::EnumerateDevices( TComputeDeviceInfoVector& devices )
{
    devices.clear();
    devices.push_back( GetDeviceInfo() );
    return true;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CTestComputeBackend::CreateDevice( const CComputeDeviceParams& params, TComputeDevicePtr& device )
{
    const CComputeDeviceInfo info = GetDeviceInfo();
    if ( !params.GetDeviceIdentifier().IsNULLOrEmpty() && params.GetDeviceIdentifier() != info.GetIdentifier() )
        return COMPUTERESULT_INVALID_ARGUMENT;
    device = CCpuComputeDevice::Create( info, params );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CLogCapture::CLogCapture( void )
    : CORE::CILogger()
    , m_text()
    , m_lock()
{
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( this );
}

/*-------------------------------------------------------------------------*/

CLogCapture::~CLogCapture()
{
    CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this );
}

/*-------------------------------------------------------------------------*/

CString
CLogCapture::GetText( void )
{
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
    MT::CScopeMutex lock( m_lock );
    return m_text;
}

/*-------------------------------------------------------------------------*/

UInt32
CLogCapture::CountOccurrences( const CString& text )
{
    const CString all = GetText();
    UInt32 count = 0;
    Int32 at = all.HasSubstr( text, 0, true );
    while ( at >= 0 )
    {
        ++count;
        at = all.HasSubstr( text, at + 1, true );
    }
    return count;
}

/*-------------------------------------------------------------------------*/

void
CLogCapture::Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{
    MT::CScopeMutex lock( m_lock );
    m_text += logMessage;
    m_text += "\n";
}

/*-------------------------------------------------------------------------*/

void
CLogCapture::Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{
    Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CLogCapture::LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{
    Log( logMsgType, logLevel, logMessage, threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CLogCapture::LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{
    Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CLogCapture::FlushLog( void )
{
}

/*-------------------------------------------------------------------------*/

void
CLogCapture::SetMinimalLogLevel( const Int32 logLevel )
{
}

/*-------------------------------------------------------------------------*/

Int32
CLogCapture::GetMinimalLogLevel( void ) const
{
    return CORE::LOGLEVEL_EVERYTHING;
}

/*-------------------------------------------------------------------------*/

CEventCounter::CEventCounter( CORE::CNotifier* notifier, const CORE::CEvent& eventId )
    : CORE::CObserver()
    , m_count( 0 )
    , m_lastText()
    , m_lock()
{
    TEventCallback callback( this, &CEventCounter::OnEvent );
    SubscribeTo( notifier, eventId, callback );
}

/*-------------------------------------------------------------------------*/

CEventCounter::~CEventCounter()
{
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CEventCounter::OnEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{
    CORE::TCloneableString* text = static_cast< CORE::TCloneableString* >( eventData );
    {
        MT::CScopeMutex lock( m_lock );
        m_lastText = GUCEF_NULL != text ? CString( *text ) : CString();
    }
    m_count.FetchAdd( 1 );
}

/*-------------------------------------------------------------------------*/

UInt32
CEventCounter::GetCount( void ) const
{
    return m_count.Load();
}

/*-------------------------------------------------------------------------*/

CString
CEventCounter::GetLastText( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_lastText;
}

/*-------------------------------------------------------------------------*/

CTestThreadBody::~CTestThreadBody()
{
}

/*-------------------------------------------------------------------------*/

bool
RunOnTestThreads( CTestThreadBody** bodies, UInt32 count, UInt32 timeoutInMs )
{
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "ComputeTestThreads" );
    if ( pool.IsNULL() )
        return false;

    GUCEF::vector< TTestThreadTaskPtr > tasks;
    bool allStarted = true;
    for ( UInt32 i=0; i<count; ++i )
    {
        TTestThreadTaskPtr task( GUCEF_NEW CTestThreadTask( bodies[ i ] ) );
        if ( pool->StartTaskWithConsumer( task ).HasNoFuture() )
        {
            allStarted = false;
            break;
        }
        tasks.push_back( task );
    }

    bool allDone = false;
    for ( UInt32 waited=0; waited<=timeoutInMs && !allDone; ++waited )
    {
        allDone = true;
        for ( size_t i=0; i<tasks.size(); ++i )
            if ( 0 == tasks[ i ]->done.Load( MT::MEMORDER_ACQUIRE ) )
                allDone = false;
        if ( !allDone )
            MT::PrecisionDelay( 1 );
    }
    return allStarted && allDone;
}

/*-------------------------------------------------------------------------*/

CTestThread::CTestThread( void )
    : m_task()
    , m_done( GUCEF_NULL )
{
}

/*-------------------------------------------------------------------------*/

CTestThread::~CTestThread()
{
    Join( 60000 );
}

/*-------------------------------------------------------------------------*/

bool
CTestThread::Start( CTestThreadBody* body )
{
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "ComputeTestThreads" );
    if ( pool.IsNULL() )
        return false;
    TTestThreadTaskPtr task( GUCEF_NEW CTestThreadTask( body ) );
    if ( pool->StartTaskWithConsumer( task ).HasNoFuture() )
        return false;
    m_done = &task->done;
    m_task = task;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestThread::Join( UInt32 timeoutInMs )
{
    if ( GUCEF_NULL == m_done )
        return true;
    for ( UInt32 waited=0; waited<timeoutInMs; ++waited )
    {
        if ( 0 != m_done->Load( MT::MEMORDER_ACQUIRE ) )
            break;
        MT::PrecisionDelay( 1 );
    }
    const bool done = 0 != m_done->Load( MT::MEMORDER_ACQUIRE );
    if ( done )
    {
        m_done = GUCEF_NULL;
        m_task.Unlink();
    }
    return done;
}

/*-------------------------------------------------------------------------*/

CTestDeferredQueue::CTestDeferredQueue( CComputeDevice* device )
    : CComputeQueue( device, 0, EXECUTIONPOLICY_DEFAULT, false )
    , failNextSubmit( 0 )
    , hostTransfers( 0 )
    , m_lock()
    , m_submitted()
    , m_deviceCompleted( 0 )
{
}

/*-------------------------------------------------------------------------*/

CTestDeferredQueue::~CTestDeferredQueue()
{
    ShutdownQueue();
}

/*-------------------------------------------------------------------------*/

UInt64
CTestDeferredQueue::GetCompletedValue( void ) const
{
    return GetHostTimelineValue();
}

/*-------------------------------------------------------------------------*/

UInt64
CTestDeferredQueue::GetDeviceCompletedValue( void ) const
{
    return m_deviceCompleted.Load();
}

/*-------------------------------------------------------------------------*/

TComputeResult
CTestDeferredQueue::WaitForValue( UInt64 value, UInt32 timeoutInMs )
{
    // the device side is done at once; whoever waits first does the host side
    AdvanceHostTimeline( m_deviceCompleted.Load() );
    if ( GetHostTimelineValue() >= value )
        return COMPUTERESULT_SUCCESS;
    if ( IsLost() )
        return COMPUTERESULT_DEVICE_LOST;
    return COMPUTERESULT_TIMEOUT;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CTestDeferredQueue::OnSubmit( const CComputeCommandList& list, UInt64 fenceValue )
{
    if ( 0 != failNextSubmit.Exchange( 0 ) )
        return COMPUTERESULT_BACKEND_ERROR;
    {
        MT::CScopeMutex lock( m_lock );
        m_submitted[ fenceValue ] = list;
    }
    m_deviceCompleted.Store( fenceValue );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CTestDeferredQueue::OnFinishHostTransfers( UInt64 fenceValue, CString& errorText )
{
    CComputeCommandList list;
    {
        MT::CScopeMutex lock( m_lock );
        GUCEF::map< UInt64, CComputeCommandList >::iterator i = m_submitted.find( fenceValue );
        if ( i == m_submitted.end() )
        {
            errorText = "no such submission";
            return COMPUTERESULT_BACKEND_ERROR;
        }
        list = (*i).second;
        m_submitted.erase( i );
    }
    CComputeCommandList::SCommand command;
    UInt32 cursor = 0;
    while ( list.ReadCommand( cursor, command ) )
    {
        UInt8* target = GUCEF_NULL != command.buffer ? static_cast< CCpuComputeBuffer* >( command.buffer )->GetHostMemory() + command.offset : GUCEF_NULL;
        switch ( command.type )
        {
            case CComputeCommandList::COMMANDTYPE_WRITE_FROM_HOST: memcpy( target, command.hostSource, static_cast< size_t >( command.size ) ); break;
            case CComputeCommandList::COMMANDTYPE_READ_TO_HOST: memcpy( command.hostDestination, target, static_cast< size_t >( command.size ) ); break;
            case CComputeCommandList::COMMANDTYPE_FILL:
                for ( UInt64 w=0; w<command.size/4; ++w )
                    memcpy( target + w * 4, &command.fillPattern, 4 );
                break;
            default: errorText = "the test queue runs writes, reads and fills only"; return COMPUTERESULT_UNSUPPORTED;
        }
    }
    hostTransfers.FetchAdd( 1 );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CTestDeferredDevice::CTestDeferredDevice( const CComputeDeviceInfo& info )
    : CComputeDevice( info, CComputeDeviceParams() )
    , m_queue( GUCEF_NULL )
{
    m_queue = GUCEF_NEW CTestDeferredQueue( this );
    AddQueue( m_queue->CreateSharedPtr() );
}

/*-------------------------------------------------------------------------*/

CTestDeferredDevice::~CTestDeferredDevice()
{
    ShutdownDevice();
}

/*-------------------------------------------------------------------------*/

TComputeDevicePtr
CTestDeferredDevice::Create( void )
{
    CComputeDeviceInfo info = CCpuComputeDevice::GetHostDeviceInfo( false );
    info.SetBackendName( "deferred" );
    info.SetDeviceName( "Deferred test GPU" );
    info.SetDeviceType( DEVICETYPE_DISCRETE_GPU );
    CTestDeferredDevice* device = GUCEF_NEW CTestDeferredDevice( info );
    return device->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

CTestDeferredQueue&
CTestDeferredDevice::GetDeferredQueue( void )
{
    return *m_queue;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CTestDeferredDevice::OnCreateBuffer( const CComputeBufferDesc& desc, TComputeBufferPtr& buffer )
{
    CCpuComputeBuffer* created = CCpuComputeBuffer::Create( this, desc );
    if ( GUCEF_NULL == created )
        return COMPUTERESULT_OUT_OF_HOST_MEMORY;
    buffer = created->CreateSharedPtr();
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CTestDeferredDevice::OnCreateKernel( const TComputeKernelDeclPtr& decl, const TComputeKernelArtefactPtr& artefact, TComputeKernelPtr& kernel )
{
    return COMPUTERESULT_UNSUPPORTED;
}

}  /* namespace TEST */
}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
