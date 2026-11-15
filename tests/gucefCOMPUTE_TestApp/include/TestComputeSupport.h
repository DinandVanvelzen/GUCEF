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

#ifndef GUCEF_COMPUTE_TESTAPP_TESTCOMPUTESUPPORT_H
#define GUCEF_COMPUTE_TESTAPP_TESTCOMPUTESUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMPUTE_H
#include "gucefCOMPUTE.h"
#define GUCEF_COMPUTE_H
#endif /* GUCEF_COMPUTE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Support for the tests of gucefCOMPUTE: test devices, the built-in test kernels, a gate kernel that blocks until the
 *  test opens it (to keep work pending), a test backend that presents the CPU implementation as a discrete GPU, a
 *  log capture, an event counter and threads for concurrency tests.
 */

namespace GUCEF {
namespace COMPUTE {
namespace TEST {

/** Registers the built-in test kernels and the gate kernel; called once by the test application */
bool RegisterComputeTestKernels( void );

void UnregisterComputeTestKernels( void );

/** A device of the CPU backend with its own pulse generator for events when one is given */
TComputeDevicePtr CreateCpuTestDevice( UInt32 queueCount = 1, const CORE::PulseGeneratorPtr& eventPulseGenerator = CORE::PulseGeneratorPtr() );

/** The id of the gate kernel: one group that waits until the gate opens, then writes 1 to slot 0 */
extern const char* GateKernelId;

void CloseGate( void );
void OpenGate( void );
/** True once a gate kernel waits at the closed gate; waits at most 'timeoutInMs' */
bool WaitUntilGateEntered( UInt32 timeoutInMs );

/** Records a dispatch of the gate kernel into 'list', writing into 'flag' (a buffer of at least 4 bytes) */
bool RecordGate( CComputeCommandList& list, TComputeDevicePtr device, const TComputeBufferPtr& flag );

/** A buffer of 'bytes' bytes of storage on the device; null on failure */
TComputeBufferPtr MakeBuffer( TComputeDevicePtr device, UInt64 bytes, TComputeMemoryLocation location = MEMORYLOCATION_UNIFIED );

/**
 *  Waits until 'expected' devices exist, at most 'timeoutInMs'. Devices whose last reference is held by finished work
 *  go on the completion task just after the future of that work completed, so counts settle shortly after
 */
bool WaitForDeviceCount( UInt32 expected, UInt32 timeoutInMs );

/** Waits for the future at most 'timeoutInMs' milliseconds; false when it did not complete */
bool AwaitFuture( const CORE::CFutureResult& future, UInt32 timeoutInMs );

/*-------------------------------------------------------------------------*/

/** A backend "testgpu" whose runtime can be absent, offering the CPU implementation as one discrete GPU */
class CTestComputeBackend : public CComputeBackend
{
    public:

    static const char* Name;

    CTestComputeBackend( void );
    virtual ~CTestComputeBackend();

    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Initialize( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Shutdown( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool EnumerateDevices( TComputeDeviceInfoVector& devices ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TComputeResult CreateDevice( const CComputeDeviceParams& params, TComputeDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE;

    /** The device info the backend enumerates */
    static CComputeDeviceInfo GetDeviceInfo( void );

    MT::CTAtomic< UInt32 > initializeCalls;
    MT::CTAtomic< UInt32 > shutdownCalls;
};

/*-------------------------------------------------------------------------*/

/** Keeps the text of every log message logged while it is attached to the log manager */
class CLogCapture : public CORE::CILogger
{
    public:

    CLogCapture( void );
    virtual ~CLogCapture();

    /** Logs still on their way to the loggers are delivered first */
    CString GetText( void );

    /** How often the text occurs in the captured messages */
    UInt32 CountOccurrences( const CString& text );

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CString m_text;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

/** Counts one event of a notifier */
class CEventCounter : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CEventCounter > TEventCallback;

    CEventCounter( CORE::CNotifier* notifier, const CORE::CEvent& eventId );
    virtual ~CEventCounter();

    void OnEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    UInt32 GetCount( void ) const;
    CString GetLastText( void ) const;

    private:

    MT::CTAtomic< UInt32 > m_count;
    CString m_lastText;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

/** Work that a test runs on a thread of its own */
class CTestThreadBody
{
    public:
    virtual ~CTestThreadBody();
    virtual void Run( void ) = 0;
};

/**
 *  Runs each body on its own dedicated task of the pool "ComputeTestThreads", the calling thread not included, and
 *  returns when all are done. False when a thread could not be had or the bodies did not end within 'timeoutInMs'
 */
bool RunOnTestThreads( CTestThreadBody** bodies, UInt32 count, UInt32 timeoutInMs );

/** One body on a dedicated task of the pool "ComputeTestThreads", started and joined by the test */
class CTestThread
{
    public:

    CTestThread( void );
    ~CTestThread();

    bool Start( CTestThreadBody* body );

    /** True when the body ended within 'timeoutInMs' */
    bool Join( UInt32 timeoutInMs );

    private:

    CORE::CTaskConsumerPtr m_task;
    MT::CTAtomic< UInt32 >* m_done;
};

/*-------------------------------------------------------------------------*/

/**
 *  A queue in the manner of a GPU queue whose native timeline signals before the copies to host memory: the device
 *  side completes at submission, the commands (including the ReadToHost copies) run in OnFinishHostTransfers(), so
 *  results reach host memory only when the host-visible timeline advances. A submission can be made to fail.
 */
class CTestDeferredQueue : public CComputeQueue
{
    public:

    explicit CTestDeferredQueue( CComputeDevice* device );
    virtual ~CTestDeferredQueue();

    virtual UInt64 GetCompletedValue( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TComputeResult WaitForValue( UInt64 value, UInt32 timeoutInMs ) GUCEF_VIRTUAL_OVERRIDE;

    /** The native timeline: what the device completed */
    UInt64 GetDeviceCompletedValue( void ) const;

    MT::CTAtomic< UInt32 > failNextSubmit;
    MT::CTAtomic< UInt32 > hostTransfers;

    protected:

    virtual TComputeResult OnSubmit( const CComputeCommandList& list, UInt64 fenceValue ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TComputeResult OnFinishHostTransfers( UInt64 fenceValue, CString& errorText ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    MT::CMutex m_lock;
    GUCEF::map< UInt64, CComputeCommandList > m_submitted;
    MT::CTAtomic< UInt64 > m_deviceCompleted;
};

/** A device of backend "deferred" (a discrete GPU) with one CTestDeferredQueue; buffers are host memory */
class CTestDeferredDevice : public CComputeDevice
{
    public:

    static TComputeDevicePtr Create( void );

    virtual ~CTestDeferredDevice();

    CTestDeferredQueue& GetDeferredQueue( void );

    protected:

    virtual TComputeResult OnCreateBuffer( const CComputeBufferDesc& desc, TComputeBufferPtr& buffer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TComputeResult OnCreateKernel( const TComputeKernelDeclPtr& decl, const TComputeKernelArtefactPtr& artefact, TComputeKernelPtr& kernel ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTestDeferredDevice( const CComputeDeviceInfo& info );

    CTestDeferredQueue* m_queue;
};

}  /* namespace TEST */
}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_TESTAPP_TESTCOMPUTESUPPORT_H ? */
