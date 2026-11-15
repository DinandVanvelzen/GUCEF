/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWENGINEHOST_H
#define GUCEF_WEB_CWEBVIEWENGINEHOST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_WEB_CIWEBVIEWENGINELOOP_H
#include "gucefWEB_CIWebViewEngineLoop.h"
#define GUCEF_WEB_CIWEBVIEWENGINELOOP_H
#endif /* GUCEF_WEB_CIWEBVIEWENGINELOOP_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOMMAND_H
#include "gucefWEB_CWebViewCommand.h"
#define GUCEF_WEB_CWEBVIEWCOMMAND_H
#endif /* GUCEF_WEB_CWEBVIEWCOMMAND_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CWebViewEngineBase;

/**
 *  The engine thread of one engine: a dedicated thread obtained through the gucefCORE task manager (a singular task on
 *  the named pool "WebView.<engine>", as the reader thread of the Redis pubsub backend), so the task manager knows it,
 *  counts it and stops it. The foreign loop of the engine runs on it (CIWebViewEngineLoop).
 *
 *  - Start: lazily, at the first view of the engine.
 *  - Commands in: Post() puts a command in the mailbox and wakes the loop through the engine's own mechanism; the
 *    thread drains the mailbox in order. Commands of one view keep their order.
 *  - Results and events out: futures complete and events are notified on this thread.
 *  - Rules: no engine object is touched from another thread; this thread never waits on another GUCEF thread.
 *  - Idle: the thread sleeps in the loop. It wakes for work, for the deadline of a deferred decision and, when the
 *    idle wake (CWebViewGlobalSettings::engineIdleWakeMs) is not 0, once per that time to notice a stop request of
 *    the task manager itself; Stop() wakes it at once.
 *  - Watchdog: CheckWatchdog() (any thread) finds a command that waits or runs longer than the watchdog time; the
 *    host is then stalled: pending futures fail with WEBVIEWERROR_ENGINE_STALLED and it takes no commands any more.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewEngineHost : public CORE::CTaskConsumer                                      ,
                                                   public CORE::CTSharedPtrCreator< CWebViewEngineHost, MT::CMutex >
{
    public:

    static const CString TaskType;

    /** The loop is not owned: the engine keeps it until the thread has ended */
    CWebViewEngineHost( CWebViewEngineBase* engine, CIWebViewEngineLoop* loop, UInt32 idleWakeMs );

    virtual ~CWebViewEngineHost() GUCEF_VIRTUAL_OVERRIDE;

    /** Starts the thread on the named pool of the task manager; false when the pool refused */
    bool Start( const CString& threadPoolName );

    /** Any thread: queues a command and wakes the engine thread. False when stopping or stalled (the command is not queued) */
    bool Post( const CWebViewCommand& command );

    /** Any thread: wakes the engine thread, for example after a deadline changed */
    void Wake( void );

    /**
     *  Asks the thread to stop (the loop is woken) and waits for it up to 'timeoutMs'; commands still queued fail with
     *  WEBVIEWERROR_SHUTTING_DOWN. False when the thread did not end in time. On the engine thread itself it only asks.
     */
    bool Stop( UInt32 timeoutMs );

    bool IsStarted( void ) const;

    /** True between the start of the thread and its end */
    bool IsRunning( void ) const;

    bool IsStopping( void ) const;

    bool IsEngineThread( void ) const;

    TThreadId GetEngineThreadId( void ) const;

    /**
     *  Any thread: true when a command has waited for or run on the engine thread longer than 'watchdogMs' (0 never).
     *  The first such call fails the pending commands and returns true with 'newlyStalled' set
     */
    bool CheckWatchdog( UInt32 watchdogMs, bool& newlyStalled );

    bool IsStalled( void ) const;

    UInt32 GetPendingCommandCount( void ) const;

    /** The number of commands carried out, for tests */
    UInt32 GetExecutedCommandCount( void ) const;

    /** Milliseconds of a monotonic clock, the time base of the hosts and views */
    static UInt64 NowMs( void );

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CORE::CTaskPtr task, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    void ProcessMail( void );

    void FailQueued( TWebViewError error, const CString& message );

    CWebViewEngineHost( const CWebViewEngineHost& src );                /**< not implemented */
    CWebViewEngineHost& operator=( const CWebViewEngineHost& src );     /**< not implemented */

    private:

    CWebViewEngineBase* m_engine;
    CIWebViewEngineLoop* m_loop;
    UInt32 m_idleWakeMs;
    MT::CTMailBox< CWebViewCommand > m_mail;
    MT::CTAtomic< UInt32 > m_started;
    MT::CTAtomic< UInt32 > m_running;
    MT::CTAtomic< UInt32 > m_stopping;
    MT::CTAtomic< UInt32 > m_stalled;
    MT::CTAtomic< UInt64 > m_busySinceMs;           /**< when the command being carried out started; 0 when none */
    MT::CTAtomic< UInt64 > m_firstUnservedMs;       /**< when the oldest command not taken yet was posted; 0 when none */
    MT::CTAtomic< UInt32 > m_executed;
    MT::CTAtomic< TThreadId > m_threadId;
    MT::CMutex m_executingLock;
    CORE::CFutureResultSource* m_executing;         /**< the future of the command being carried out, for the watchdog */
};

typedef CORE::CTSharedPtr< CWebViewEngineHost, MT::CMutex > TWebViewEngineHostPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWENGINEHOST_H ? */
