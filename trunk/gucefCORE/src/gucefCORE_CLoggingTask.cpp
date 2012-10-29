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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

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
      m_mailList()
{GUCEF_TRACE;

    m_mailList.reserve( MAXMAILITEMSPERCYCLE );
}

/*-------------------------------------------------------------------------*/

CLoggingTask::CLoggingTask( CILogger& loggerBackend )
    : CTaskConsumer()                   ,
      CILogger()                        ,
      m_loggerBackend( &loggerBackend ) ,
      m_mailbox()                       ,
      m_mailList()
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

    LockData();
    m_loggerBackend = &loggerBackend;
    UnlockData();
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::Log( const TLogMsgType logMsgType ,
                   const Int32 logLevel         ,
                   const CString& logMessage    ,
                   const UInt32 threadId        )
{GUCEF_TRACE;

    CLoggingMail logMsg;
    logMsg.logLevel = logLevel;
    logMsg.logMessage = logMessage;
    logMsg.logMsgType = logMsgType;
    logMsg.threadId = threadId;
    logMsg.withoutFormatting = false;

    m_mailbox.AddMail( MAILTYPE_NEWLOGMSG, &logMsg );
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::LogWithoutFormatting( const TLogMsgType logMsgType ,
                                    const Int32 logLevel         ,
                                    const CString& logMessage    ,
                                    const UInt32 threadId        )
{GUCEF_TRACE;

    CLoggingMail logMsg;
    logMsg.logLevel = logLevel;
    logMsg.logMessage = logMessage;
    logMsg.logMsgType = logMsgType;
    logMsg.threadId = threadId;
    logMsg.withoutFormatting = true;

    m_mailbox.AddMail( MAILTYPE_NEWLOGMSG, &logMsg );
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::StartTask( void )
{GUCEF_TRACE;

    return CCoreGlobal::Instance()->GetTaskManager().StartTask( *this );
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::StopTask( void )
{GUCEF_TRACE;

    return CCoreGlobal::Instance()->GetTaskManager().RequestTaskToStop( GetTaskId() );
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::FlushLog( void )
{GUCEF_TRACE;

    m_mailbox.AddMail( MAILTYPE_FLUSHLOG, NULL );
}

/*-------------------------------------------------------------------------*/

CString
CLoggingTask::GetType( void ) const
{GUCEF_TRACE;

    return "LoggingTask";
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskStart( CICloneable* taskdata )
{GUCEF_TRACE;

    // This task does not need to perform any initialization/setup
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskCycleLog( const TLogMsgType logMsgType ,
                              const Int32 logLevel         ,
                              const CString& logMessage    ,
                              const UInt32 threadId        )
{GUCEF_TRACE;

    if ( NULL != m_loggerBackend )
    {
        m_loggerBackend->Log( logMsgType ,
                              logLevel   ,
                              logMessage ,
                              threadId   );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLoggingTask::OnTaskCycleLogWithoutFormatting( const TLogMsgType logMsgType ,
                                               const Int32 logLevel         ,
                                               const CString& logMessage    ,
                                               const UInt32 threadId        )

{GUCEF_TRACE;

    if ( NULL != m_loggerBackend )
    {
        m_loggerBackend->LogWithoutFormatting( logMsgType ,
                                               logLevel   ,
                                               logMessage ,
                                               threadId   );
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

bool
CLoggingTask::OnTaskCycle( CICloneable* taskdata )
{GUCEF_TRACE;

    if ( IsReadyToProcessCycle() )
    {
        CLoggingMail* loggingMail = NULL;
        if ( m_mailbox.GetMailList( m_mailList           ,
                                    MAXMAILITEMSPERCYCLE ) )
        {
            while ( !m_mailList.empty() )
            {
                TLoggingMailBox::TMailElement& mailEntry = m_mailList[ m_mailList.size()-1 ];
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
                                                  loggingMail->threadId    ) ) return true;
                        }
                        break;
                    }
                    case MAILTYPE_FLUSHLOG :
                    {
                        if ( !OnTaskCycleLogFlush() ) return true;
                        break;
                    }

                }

                delete loggingMail;
                loggingMail = NULL;

                m_mailList.pop_back();
            }
        }
    }

    // This is an infinate task so we always return false
    //  which indicates we are not done.
    return false;
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::LockData( void )
{GUCEF_TRACE;

    m_mailbox.LockData();
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::UnlockData( void )
{GUCEF_TRACE;

    m_mailbox.UnlockData();
}

/*-------------------------------------------------------------------------*/

void
CLoggingTask::OnTaskEnd( CICloneable* taskdata )
{GUCEF_TRACE;

    TLoggingMailBox::TMailElement* entry;
    TMailList::iterator i( m_mailList.begin() );
    while ( i != m_mailList.end() )
    {
        entry = &(*i);
        delete entry->data;
        m_mailList.erase( i );
        i = m_mailList.begin();
    }
}

/*-------------------------------------------------------------------------*/

CLoggingTask::CLoggingMail::CLoggingMail( void )
    : CICloneable()                              ,
      logMsgType( CLogManager::LOG_UNKNOWNTYPE ) ,
      logLevel( LOGLEVEL_NORMAL )                ,
      logMessage()                               ,
      threadId( 0 )                              ,
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
      withoutFormatting( src.withoutFormatting )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

MT::CICloneable*
CLoggingTask::CLoggingMail::Clone( void ) const
{GUCEF_TRACE;

    return new CLoggingMail( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
