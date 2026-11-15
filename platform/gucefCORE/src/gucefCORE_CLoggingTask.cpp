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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTASKDELEGATOR_H
#include "gucefCORE_CTaskDelegator.h"
#define GUCEF_CORE_CTASKDELEGATOR_H
#endif /* GUCEF_CORE_CTASKDELEGATOR_H ? */

#include "gucefCORE_CLoggingTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define MAXMAILITEMSPERCYCLE    50

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTITLITIES                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

CLoggingTask::CLoggingTask( void )
    : CTaskConsumer()         ,
      CILogger()              ,
      m_loggerBackend( GUCEF_NULL ) ,
      m_mailbox()             ,
      m_mailList()            ,
      m_minLogLevel( 0 ) ,
      m_workQueue() ,
      m_queuedWork( 0 ) ,
      m_closed( false ) ,
      m_hasRun( false ) ,
      m_taskThreadId( 0 ) ,
      m_taskDelegator()
{GUCEF_TRACE;

    m_mailList.reserve( MAXMAILITEMSPERCYCLE );
}

/*-------------------------------------------------------------------------*/

CLoggingTask::CLoggingTask( CILogger& loggerBackend )
    : CTaskConsumer()                   ,
      CILogger()                        ,
      m_loggerBackend( &loggerBackend ) ,
      m_mailbox()                       ,
      m_mailList()                      ,
      m_minLogLevel( 0 ) ,
      m_workQueue() ,
      m_queuedWork( 0 ) ,
      m_closed( false ) ,
      m_hasRun( false ) ,
      m_taskThreadId( 0 ) ,
      m_taskDelegator()
{GUCEF_TRACE;

    m_mailList.reserve( MAXMAILITEMSPERCYCLE );
}

/*-------------------------------------------------------------------------*/

CLoggingTask::~CLoggingTask()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::SetLoggerBackend( CILogger& loggerBackend )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_loggerBackend = &loggerBackend;
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::Log( const TLogMsgType logMsgType ,
                   const Int32 logLevel         ,
                   const CString& logMessage    ,
                   const TSystemThreadId threadId        ,
                   const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( logLevel >= m_minLogLevel )
    {
        CLoggingMail logMsg;
        logMsg.logLevel = logLevel;
        logMsg.logMessage = logMessage;
        logMsg.logMsgType = logMsgType;
        logMsg.threadId = threadId;
        logMsg.timestamp = timestamp;
        logMsg.withoutFormatting = false;

        m_mailbox.AddMail( MAILTYPE_NEWLOGMSG, &logMsg );
    }
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::LogWithoutFormatting( const TLogMsgType logMsgType ,
                                    const Int32 logLevel         ,
                                    const CString& logMessage    ,
                                    const TSystemThreadId threadId        ,
                                    const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( logLevel >= m_minLogLevel )
    {
        CLoggingMail logMsg;
        logMsg.logLevel = logLevel;
        logMsg.logMessage = logMessage;
        logMsg.logMsgType = logMsgType;
        logMsg.threadId = threadId;
        logMsg.timestamp = timestamp;
        logMsg.withoutFormatting = true;

        m_mailbox.AddMail( MAILTYPE_NEWLOGMSG, &logMsg );
    }
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::Log( const TLogMsgType logMsgType     ,
                   const Int32 logLevel             ,
                   const CVariantStream& logMessage ,
                   const TSystemThreadId threadId   ,
                   const CTimestamp& timestamp      )
{GUCEF_TRACE;

    if ( logLevel >= m_minLogLevel )
    {
        CLoggingMail logMsg;
        logMsg.logLevel = logLevel;
        logMsg.logMessage = logMessage.ToString();
        logMsg.logMsgType = logMsgType;
        logMsg.threadId = threadId;
        logMsg.timestamp = timestamp;
        logMsg.withoutFormatting = false;

        m_mailbox.AddMail( MAILTYPE_NEWLOGMSG, &logMsg );
    }
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                    const Int32 logLevel             ,
                                    const CVariantStream& logMessage ,
                                    const TSystemThreadId threadId   ,
                                    const CTimestamp& timestamp      )
{GUCEF_TRACE;

    if ( logLevel >= m_minLogLevel )
    {
        CLoggingMail logMsg;
        logMsg.logLevel = logLevel;
        logMsg.logMessage = logMessage.ToString();
        logMsg.logMsgType = logMsgType;
        logMsg.threadId = threadId;
        logMsg.timestamp = timestamp;
        logMsg.withoutFormatting = true;

        m_mailbox.AddMail( MAILTYPE_NEWLOGMSG, &logMsg );
    }
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::FlushLog( void )
{GUCEF_TRACE;

    m_mailbox.AddMail( MAILTYPE_FLUSHLOG, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::SetMinimalLogLevel( const Int32 logLevel )
{GUCEF_TRACE;

    m_minLogLevel = logLevel;
}

/*-------------------------------------------------------------------------*/

Int32 
CLoggingTask::GetMinimalLogLevel( void ) const
{GUCEF_TRACE;

    return m_minLogLevel;
}

/*-------------------------------------------------------------------------*/

CString
CLoggingTask::GetType( void ) const
{GUCEF_TRACE;

    return "LoggingTask";
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskStart( CTaskPtr task )
{GUCEF_TRACE;

    // Mail is taken again, and work handed over from now on is for this thread (see RunOnTaskThread()). A task its owner
    // closed before it ever ran was given up: its owner delivers what it would have
    MT::CObjectScopeLock lock( this );
    if ( m_closed && !m_hasRun )
        return false;
    m_hasRun = true;
    m_closed = false;
    m_mailbox.SetAcceptsNewMail( true );
    m_taskThreadId = MT::GetCurrentThreadID();
    m_taskDelegator = GetTaskDelegator();
    return true;
}

/*-------------------------------------------------------------------------*/

CLoggingTask::CIWork::~CIWork()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::RunOnTaskThread( CIWork& work )
{GUCEF_TRACE;

    // With a working semaphore the caller sleeps until the work was done, without one it looks every millisecond
    const bool canWait = CTaskManager::IsTaskDelegatorWakeUpAvailable();
    MT::CSemaphore done( 0 );
    MT::CTAtomic< bool > finished( false );

    bool queued = false;
    TTaskDelegatorBasicPtr delegator;
    {
        MT::CObjectScopeLock lock( this );
        if ( !m_closed && MT::GetCurrentThreadID() != m_taskThreadId )
        {
            m_workQueue.push_back( CQueuedWork( &work, canWait ? &done : GUCEF_NULL, &finished ) );
            m_queuedWork.Store( (UInt32) m_workQueue.size(), MT::MEMORDER_RELEASE );
            delegator = m_taskDelegator;
            queued = true;
        }
    }
    if ( !queued )
    {
        // this is the thread of the task, or the task delivers no more: nothing else delivers meanwhile
        work.OnLoggingTaskWork( false );
        return;
    }

    // the thread of the task may wait for its next cycle: a pulse request ends that wait, it takes the work now
    if ( !delegator.IsNULL() )
    {
        PulseGeneratorPtr pulseGenerator = delegator->GetPulseGenerator();
        if ( !pulseGenerator.IsNULL() )
            pulseGenerator->RequestImmediatePulseThroughDriver();
    }

    if ( canWait )
    {
        while ( !done.Wait() )
            MT::ThreadDelay( 1 );
    }
    // 'finished' is set after the post: once it is set the thread of the task touches neither of them any more
    UInt32 spins = 0;
    while ( !finished.Load( MT::MEMORDER_ACQUIRE ) )
    {
        ++spins;
        if ( spins > 1000 || !canWait )
            MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::IsRunning( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return !m_closed && 0 != m_taskThreadId;
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::IsTaskThread( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return !m_closed && 0 != m_taskThreadId && MT::GetCurrentThreadID() == m_taskThreadId;
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::CloseAndDeliverQueued( void )
{GUCEF_TRACE;

    TMailVector queued;
    TWorkQueue work;
    {
        MT::CObjectScopeLock lock( this );
        if ( m_closed )
            return;
        m_closed = true;
        m_mailbox.SetAcceptsNewMail( false );
        m_mailbox.GetMailList( queued );
        work.swap( m_workQueue );
        m_queuedWork.Store( 0, MT::MEMORDER_RELEASE );
        m_taskThreadId = 0;
        m_taskDelegator.Unlink();
    }

    // the work first, as the cycle would have; it may come from within a delivery of the calling thread
    for ( TWorkQueue::size_type w=0; w<work.size(); ++w )
        work[ w ].Run( false );

    // in order, as the cycle would have
    TMailVector::iterator i = queued.begin();
    while ( i != queued.end() )
    {
        TLoggingMailBox::Mail& mailEntry = (*i);
        switch ( mailEntry.eventid )
        {
            case MAILTYPE_NEWLOGMSG :
            {
                CLoggingMail* loggingMail = static_cast< CLoggingMail* >( mailEntry.data );
                if ( GUCEF_NULL != loggingMail )
                {
                    OnTaskCycleLog( loggingMail->logMsgType  ,
                                    loggingMail->logLevel    ,
                                    loggingMail->logMessage  ,
                                    loggingMail->threadId    ,
                                    loggingMail->timestamp   );
                }
                break;
            }
            case MAILTYPE_FLUSHLOG :
            {
                OnTaskCycleLogFlush();
                break;
            }
            default :
                break;
        }
        GUCEF_DELETE mailEntry.data;
        mailEntry.data = GUCEF_NULL;
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::DoQueuedWork( bool betweenMessages )
{GUCEF_TRACE;

    TWorkQueue work;
    {
        MT::CObjectScopeLock lock( this );
        work.swap( m_workQueue );
        m_queuedWork.Store( 0, MT::MEMORDER_RELEASE );
    }
    for ( TWorkQueue::size_type w=0; w<work.size(); ++w )
        work[ w ].Run( betweenMessages );
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::DropMailList( TMailVector& mailList )
{GUCEF_TRACE;

    TMailVector::iterator i( mailList.begin() );
    while ( i != mailList.end() )
    {
        GUCEF_DELETE (*i).data;
        (*i).data = GUCEF_NULL;
        ++i;
    }
    mailList.clear();
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskCycleLog( const TLogMsgType logMsgType ,
                              const Int32 logLevel         ,
                              const CString& logMessage    ,
                              const TSystemThreadId threadId        ,
                              const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_loggerBackend )
    {
        m_loggerBackend->Log( logMsgType ,
                              logLevel   ,
                              logMessage ,
                              threadId   ,
                              timestamp  );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskCycleLogWithoutFormatting( const TLogMsgType logMsgType ,
                                               const Int32 logLevel         ,
                                               const CString& logMessage    ,
                                               const TSystemThreadId threadId        ,
                                               const CTimestamp& timestamp  )

{GUCEF_TRACE;

    if ( GUCEF_NULL != m_loggerBackend )
    {
        m_loggerBackend->LogWithoutFormatting( logMsgType ,
                                               logLevel   ,
                                               logMessage ,
                                               threadId   ,
                                               timestamp  );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskCycleLog( const TLogMsgType logMsgType     ,
                              const Int32 logLevel             ,
                              const CVariantStream& logMessage ,
                              const TSystemThreadId threadId   ,
                              const CTimestamp& timestamp      )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_loggerBackend )
    {
        m_loggerBackend->Log( logMsgType ,
                              logLevel   ,
                              logMessage ,
                              threadId   ,
                              timestamp  );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskCycleLogWithoutFormatting( const TLogMsgType logMsgType     ,
                                               const Int32 logLevel             ,
                                               const CVariantStream& logMessage ,
                                               const TSystemThreadId threadId   ,
                                               const CTimestamp& timestamp      )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_loggerBackend )
    {
        m_loggerBackend->LogWithoutFormatting( logMsgType ,
                                               logLevel   ,
                                               logMessage ,
                                               threadId   ,
                                               timestamp  );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskCycleLogFlush( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_loggerBackend )
    {
        m_loggerBackend->FlushLog();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::IsReadyToProcessCycle( void ) const
{
    return true;
}

/*-------------------------------------------------------------------------*/

void 
CLoggingTask::OnTaskEnding( CTaskPtr task     ,
                            bool willBeForced )
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskCycle( CTaskPtr task )
{GUCEF_TRACE;

    // Work handed over (RunOnTaskThread()) goes before the next batch: one load per batch, the lock only with work
    if ( 0 != m_queuedWork.Load( MT::MEMORDER_ACQUIRE ) )
        DoQueuedWork( true );

    if ( IsReadyToProcessCycle() )
    {
        if ( m_mailbox.GetMailList( m_mailList           ,
                                    MAXMAILITEMSPERCYCLE ) )
        {
            TMailVector::iterator i = m_mailList.begin();
            while ( i != m_mailList.end() )
            {
                TLoggingMailBox::Mail& mailEntry = (*i);
                switch ( mailEntry.eventid )
                {
                    case MAILTYPE_NEWLOGMSG :
                    {
                        CLoggingMail* loggingMail = static_cast< CLoggingMail* >( mailEntry.data );
                        if ( 0 != loggingMail )
                        {
                            if ( !OnTaskCycleLog( loggingMail->logMsgType  ,
                                                  loggingMail->logLevel    ,
                                                  loggingMail->logMessage  ,
                                                  loggingMail->threadId    ,
                                                  loggingMail->timestamp   ) ) return true;
                        }
                        break;
                    }
                    case MAILTYPE_FLUSHLOG :
                    {
                        if ( !OnTaskCycleLogFlush() ) return true;
                        break;
                    }
                    default :
                        break;
                }

                GUCEF_DELETE mailEntry.data;
                mailEntry.data = GUCEF_NULL;

                ++i;
            }
            m_mailList.clear();
        }
    }

    // This is an infinite task so we always return false
    //  which indicates we are not done.
    return false;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CLoggingTask::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_mailbox.GetLock().Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CLoggingTask::Unlock( void ) const
{GUCEF_TRACE;

    return m_mailbox.GetLock().Unlock(); 
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::OnTaskEnded( CTaskPtr task  ,
                           bool wasForced )
{GUCEF_TRACE;

    // What a cycle left (an end by force, or a cycle that ended the task) is not delivered. What is still queued, and
    // the work that waits, is delivered and done now, unless the owner did that already
    DropMailList( m_mailList );
    CloseAndDeliverQueued();
}

/*-------------------------------------------------------------------------*/

CLoggingTask::CLoggingMail::CLoggingMail( void )
    : CICloneable()                              ,
      logMsgType( LOG_UNKNOWNTYPE ) ,
      logLevel( LOGLEVEL_NORMAL )                ,
      logMessage()                               ,
      threadId( 0 )                              ,
      timestamp()                                ,
      withoutFormatting( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLoggingTask::CLoggingMail::CLoggingMail( const CLoggingMail& src )
    : CICloneable()                              ,
      logMsgType( src.logMsgType )               ,
      logLevel( src.logLevel )                   ,
      logMessage( src.logMessage )               ,
      threadId( src.threadId )                   ,
      timestamp( src.timestamp )                 ,
      withoutFormatting( src.withoutFormatting )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

MT::CICloneable*
CLoggingTask::CLoggingMail::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CLoggingMail( *this );
}

/*-------------------------------------------------------------------------*/

CLoggingTask::CQueuedWork::CQueuedWork( CIWork* workToDo                  ,
                                        MT::CSemaphore* doneSignal        ,
                                        MT::CTAtomic< bool >* finishedFlag )
    : work( workToDo )         ,
      done( doneSignal )       ,
      finished( finishedFlag )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::CQueuedWork::Run( bool betweenMessages )
{GUCEF_TRACE;

    // the waiting thread owns all three: the post comes before the flag, and nothing is touched after the flag
    work->OnLoggingTaskWork( betweenMessages );
    if ( GUCEF_NULL != done )
        done->Post();
    finished->Store( true, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
