/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CLOGGINGTASK_H
#define GUCEF_CORE_CLOGGINGTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  class which provides a threaded wrapper for logging backends
 *  You can either provide a backend to be called or you an derive from
 *  this class and implement the appropriote handler member functions.
 */
class GUCEF_CORE_PUBLIC_CPP CLoggingTask : public CTaskConsumer ,
                                           public CILogger
{
    public:

    /**
     *  Default constructor which allows you to create the task consumer
     *  consumer and then hook up a logger backend at a later time.
     */
    CLoggingTask( void );
    
    CLoggingTask( CILogger& loggerBackend );
    
    virtual ~CLoggingTask();

    /** 
     *  Adds a log message to the mailbox of the threaded logger.
     *  The actual logging backend invocation will be performed within
     *  the thread dedicated to logging.
     */
    virtual void Log( const TLogMsgType logMsgType ,
                      const Int32 logLevel         ,
                      const CString& logMessage    ,
                      const TSystemThreadId threadId        ,
                      const CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    /** 
     *  Adds a log message to the mailbox of the threaded logger.
     *  The actual logging backend invocation will be performed within
     *  the thread dedicated to logging.
     *  This variant accepts a CVariantStream for structured log data.
     */
    virtual void Log( const TLogMsgType logMsgType     ,
                      const Int32 logLevel             ,
                      const CVariantStream& logMessage ,
                      const TSystemThreadId threadId   ,
                      const CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE;

    /** 
     *  Adds a log message to the mailbox of the threaded logger.
     *  The actual logging backend invocation will be performed within
     *  the thread dedicated to logging.
     */
    virtual void LogWithoutFormatting( const TLogMsgType logMsgType ,
                                       const Int32 logLevel         ,
                                       const CString& logMessage    ,
                                       const TSystemThreadId threadId        ,
                                       const CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    /** 
     *  Adds a log message to the mailbox of the threaded logger.
     *  The actual logging backend invocation will be performed within
     *  the thread dedicated to logging.
     *  This variant accepts a CVariantStream for structured log data.
     */
    virtual void LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                       const Int32 logLevel             ,
                                       const CVariantStream& logMessage ,
                                       const TSystemThreadId threadId   ,
                                       const CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetLoggerBackend( CILogger& loggerBackend );

    /**
     *  Work that the thread of the task does between two batches of log messages, see RunOnTaskThread()
     */
    class GUCEF_CORE_PUBLIC_CPP CIWork
    {
        public:

        virtual ~CIWork();

        /**
         *  @param betweenMessages true when the thread of the task does the work from its queue: it delivers no log
         *         message meanwhile. False when the work is done at once on the calling thread (see RunOnTaskThread()),
         *         which may be within the delivery of a log message.
         */
        virtual void OnLoggingTaskWork( bool betweenMessages ) = 0;
    };

    /**
     *  Has the thread of the task do the work and returns once it was done. That thread does it before its next batch
     *  of log messages: after the messages it delivers now, before the messages it delivers next, also those handed to
     *  the task before this call. For what may only be done while that thread delivers nothing, such as a change of
     *  what the backend delivers to, without a lock on the delivery of each message: the cost is taken by this call,
     *  the delivery pays one atomic load per batch. Before the task runs the work waits for it.
     *  The work is done at once on the calling thread when that is the thread of the task (a logger the task calls),
     *  and when the task delivers no more (see CloseAndDeliverQueued()).
     */
    void RunOnTaskThread( CIWork& work );

    /** Whether the calling thread is the thread of the task, while the task runs */
    bool IsTaskThread( void ) const;

    /** Whether the task runs: it started (OnTaskStart()) and did not end or get closed since */
    bool IsRunning( void ) const;

    /**
     *  For when the task ended: delivers on the calling thread, in order, what was handed to the task and not
     *  delivered yet, does the work that waits in the queue (see RunOnTaskThread()), and takes nothing more: from then
     *  on a log message handed to the task is dropped and work is done at once on the calling thread, until the task
     *  starts again. The task does this itself when it ends (OnTaskEnded()); an owner that keeps its own delivery
     *  apart from the task's with a lock of its own calls it first, under that lock. Called before the task ever
     *  started, the task gives up: it does not start any more (OnTaskStart() fails), its owner delivers instead.
     */
    void CloseAndDeliverQueued( void );

    /**
     *  Returns the type of task this task consumer can handle
     */
    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnTaskStart( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CTaskPtr task     ,
                               bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CTaskPtr task  ,
                              bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycleLog( const TLogMsgType logMsgType ,
                                 const Int32 logLevel         ,
                                 const CString& logMessage    ,
                                 const TSystemThreadId threadId        ,
                                 const CTimestamp& timestamp  );

    virtual bool OnTaskCycleLog( const TLogMsgType logMsgType     ,
                                 const Int32 logLevel             ,
                                 const CVariantStream& logMessage ,
                                 const TSystemThreadId threadId   ,
                                 const CTimestamp& timestamp      );

    virtual bool OnTaskCycleLogWithoutFormatting( const TLogMsgType logMsgType ,
                                                  const Int32 logLevel         ,
                                                  const CString& logMessage    ,
                                                  const TSystemThreadId threadId        ,
                                                  const CTimestamp& timestamp  );

    virtual bool OnTaskCycleLogWithoutFormatting( const TLogMsgType logMsgType     ,
                                                  const Int32 logLevel             ,
                                                  const CVariantStream& logMessage ,
                                                  const TSystemThreadId threadId   ,
                                                  const CTimestamp& timestamp      );

    virtual bool OnTaskCycleLogFlush( void );

    virtual bool IsReadyToProcessCycle( void ) const;

    private:
    
    enum EMailType
    {
        MAILTYPE_NEWLOGMSG ,
        MAILTYPE_FLUSHLOG  ,
        
        MAILTYPE_UNKNOWN
    };
    typedef enum EMailType TMailType;
    typedef MT::CTMailBox< TMailType > TLoggingMailBox;
    typedef MT::CTMailBox< TMailType >::TMailVector TMailVector;

    class CLoggingMail : public CICloneable
    {
        public:
        
        TLogMsgType logMsgType;
        Int32 logLevel;
        CString logMessage;
        TSystemThreadId threadId;
        CTimestamp timestamp;
        bool withoutFormatting;
        
        virtual CICloneable* Clone( void ) const;
        
        CLoggingMail( void );
        
        CLoggingMail( const CLoggingMail& src );
    };

    /** Work handed to the thread of the task (RunOnTaskThread()), with what tells the waiting thread it was done */
    class CQueuedWork
    {
        public:

        CIWork* work;
        MT::CSemaphore* done;               /**< posted once the work was done; NULL when the waiter polls 'finished' */
        MT::CTAtomic< bool >* finished;     /**< set last, the waiter returns only once it is set */

        /** Does the work and tells the waiting thread; nothing of it is touched afterwards */
        void Run( bool betweenMessages );

        CQueuedWork( CIWork* workToDo, MT::CSemaphore* doneSignal, MT::CTAtomic< bool >* finishedFlag );
    };
    typedef GUCEF::vector< CQueuedWork > TWorkQueue;

    /** Does the work handed over (RunOnTaskThread()), in the order it was handed over */
    void DoQueuedWork( bool betweenMessages );

    /** Deletes the data of the mail in the list, without delivering it */
    static void DropMailList( TMailVector& mailList );

    private:
    
    CILogger* m_loggerBackend;
    TLoggingMailBox m_mailbox;
    TMailVector m_mailList;
    Int32 m_minLogLevel;
    TWorkQueue m_workQueue;                 /**< work handed over, see RunOnTaskThread(); under the mailbox lock */
    MT::CTAtomic< UInt32 > m_queuedWork;    /**< the size of m_workQueue, read by the thread of the task once per cycle without the lock */
    bool m_closed;                          /**< no more mail is taken, see CloseAndDeliverQueued(); under the mailbox lock */
    bool m_hasRun;                          /**< OnTaskStart() ran once; under the mailbox lock */
    TThreadId m_taskThreadId;               /**< the thread of the task while it runs, else 0; under the mailbox lock */
    TTaskDelegatorBasicPtr m_taskDelegator; /**< woken for work handed over; under the mailbox lock */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CLoggingTask, MT::CMutex > CLoggingTaskPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CLOGGINGTASK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-02-2007 :
        - Dinand: Added this class

---------------------------------------------------------------------------*/
