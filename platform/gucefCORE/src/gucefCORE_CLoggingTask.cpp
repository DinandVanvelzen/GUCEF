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
      m_loggerBackend( NULL ) ,
      m_mailbox()             ,
      m_mailList()            ,
      m_minLogLevel( 0 )
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
      m_minLogLevel( 0 )
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
                   const UInt32 threadId        ,
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
                                    const UInt32 threadId        ,
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
CLoggingTask::FlushLog( void )
{GUCEF_TRACE;

    m_mailbox.AddMail( MAILTYPE_FLUSHLOG, NULL );
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

    // This task does not need to perform any initialization/setup
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskCycleLog( const TLogMsgType logMsgType ,
                              const Int32 logLevel         ,
                              const CString& logMessage    ,
                              const UInt32 threadId        ,
                              const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( NULL != m_loggerBackend )
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
                                               const UInt32 threadId        ,
                                               const CTimestamp& timestamp  )

{GUCEF_TRACE;

    if ( NULL != m_loggerBackend )
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

    if ( NULL != m_loggerBackend )
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

    if ( IsReadyToProcessCycle() )
    {
        CLoggingMail* loggingMail = NULL;
        if ( m_mailbox.GetMailList( m_mailList           ,
                                    MAXMAILITEMSPERCYCLE ) )
        {
            TMailVector::iterator i = m_mailList.begin();
            while ( i != m_mailList.end() )
            {
                TLoggingMailBox::Mail& mailEntry = (*i);
                loggingMail = static_cast< CLoggingMail* >( mailEntry.data );

                switch ( mailEntry.eventid )
                {
                    case MAILTYPE_NEWLOGMSG :
                    {
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

                }

                GUCEF_DELETE loggingMail;
                mailEntry.data = loggingMail = GUCEF_NULL;

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

    TLoggingMailBox::Mail* entry = GUCEF_NULL;
    TMailVector::iterator i( m_mailList.begin() );
    while ( i != m_mailList.end() )
    {
        entry = &(*i);
        GUCEF_DELETE entry->data;
        ++i;
    }
    m_mailList.clear();
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
