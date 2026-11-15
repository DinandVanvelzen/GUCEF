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

#include <assert.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#include "gucefCORE_CMultiLogger.h"

#ifndef GUCEF_CORE_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_ESSENTIALS_H
#endif /* GUCEF_CORE_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CMultiLogger::CMultiLogger( void )
    : CILogger()
    , m_loggers()
    , m_loggerCount( 0 )
    , m_msgTypeEnablers()
    , m_minLogLevel( LOGLEVEL_BELOW_NORMAL ) 
{GUCEF_TRACE;

    m_msgTypeEnablers[ LOG_ERROR ] = true;
    m_msgTypeEnablers[ LOG_WARNING ] = true;
    m_msgTypeEnablers[ LOG_STANDARD ] = true;
    m_msgTypeEnablers[ LOG_USER ] = true;
    m_msgTypeEnablers[ LOG_SYSTEM ] = true;
    m_msgTypeEnablers[ LOG_DEV ] = true;
    m_msgTypeEnablers[ LOG_DEBUG ] = true;
    m_msgTypeEnablers[ LOG_SERVICE ] = true;
    m_msgTypeEnablers[ LOG_PROTECTED ] = true;
    m_msgTypeEnablers[ LOG_CALLSTACK ] = true;
    m_msgTypeEnablers[ LOG_EXCEPTION ] = true;
    m_msgTypeEnablers[ LOG_CONSOLE ] = true;
}

/*-------------------------------------------------------------------------*/

CMultiLogger::~CMultiLogger()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMultiLogger::CMultiLogger( const CMultiLogger& src )
    : CILogger( src )
    , m_loggers( src.m_loggers )
    , m_loggerCount( src.m_loggerCount )
    , m_msgTypeEnablers( src.m_msgTypeEnablers )
    , m_minLogLevel( src.m_minLogLevel ) 
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMultiLogger&
CMultiLogger::operator=( const CMultiLogger& src )
{GUCEF_TRACE;
    
    if ( &src != this )
    {
        m_loggers = src.m_loggers;
        m_loggerCount = src.m_loggerCount;
        m_msgTypeEnablers = src.m_msgTypeEnablers;
        m_minLogLevel = src.m_minLogLevel; 
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt32 
CMultiLogger::GetLoggerCount( void ) const
{GUCEF_TRACE;
    
    return m_loggerCount;
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::AddLogger( CILogger* loggerImp, bool whileLogging )
{GUCEF_TRACE;

    // Do not add bogus loggers
    // Nothing is logged here: the log manager calls this under its own lock, which a log statement must not be made
    // under (see CLogManager::AddLogger()); the log manager logs the change once it gave up that lock
    if ( GUCEF_NULL == loggerImp )
        return;

    if ( !whileLogging )
        CompactLoggers();
    for ( TLoggerList::size_type i=0; i<m_loggers.size(); ++i )
    {
        if ( loggerImp == m_loggers[ i ] )
            return;
    }

    // at the end: a delivery in progress (by index) reaches it, and no logger moves under that delivery
    m_loggers.push_back( loggerImp );
    ++m_loggerCount;
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::RemoveLogger( CILogger* loggerImp, bool whileLogging )
{GUCEF_TRACE;

    // Do not pass in bogus pointers
    assert( GUCEF_NULL != loggerImp );

    // nothing is logged here, see AddLogger()
    for ( TLoggerList::size_type i=0; i<m_loggers.size(); ++i )
    {
        if ( loggerImp == m_loggers[ i ] )
        {
            // within a delivery the others keep their place: a logger that moved forward would miss the message
            if ( whileLogging )
                m_loggers[ i ] = GUCEF_NULL;
            else
                m_loggers.erase( m_loggers.begin() + i );
            --m_loggerCount;
            break;
        }
    }
    if ( !whileLogging )
        CompactLoggers();
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::ClearLoggers( bool whileLogging )
{GUCEF_TRACE;

    if ( whileLogging )
    {
        for ( TLoggerList::size_type i=0; i<m_loggers.size(); ++i )
            m_loggers[ i ] = GUCEF_NULL;
    }
    else
    {
        m_loggers.clear();
    }
    m_loggerCount = 0;
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::CompactLoggers( void )
{GUCEF_TRACE;

    if ( m_loggerCount == (UInt32) m_loggers.size() )
        return;

    TLoggerList::size_type kept = 0;
    for ( TLoggerList::size_type i=0; i<m_loggers.size(); ++i )
    {
        if ( GUCEF_NULL != m_loggers[ i ] )
        {
            m_loggers[ kept ] = m_loggers[ i ];
            ++kept;
        }
    }
    m_loggers.resize( kept );
}

/*-------------------------------------------------------------------------*/

 bool
 CMultiLogger::IsLoggingEnabled( const TLogMsgType logMsgType ,
                                 const Int32 logLevel         ) const
{GUCEF_TRACE;

    // the same test as Log() makes
    return logLevel >= m_minLogLevel && IsLogMsgTypeEnabled( logMsgType );
}

/*-------------------------------------------------------------------------*/

bool
CMultiLogger::IsLogMsgTypeEnabled( const TLogMsgType logMsgType ) const
{GUCEF_TRACE;

    GUCEF::map< TLogMsgType, bool >::const_iterator i = m_msgTypeEnablers.find( logMsgType );
    return i != m_msgTypeEnablers.end() && (*i).second;
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::SetMinimalLogLevel( const Int32 logLevel )
{GUCEF_TRACE;

    m_minLogLevel = logLevel;
    for ( TLoggerList::size_type i=0; i<m_loggers.size(); ++i )
    {
        CILogger* logger = m_loggers[ i ];
        if ( GUCEF_NULL != logger )
        {
            logger->SetMinimalLogLevel( logLevel );
        }
    }
}

/*-------------------------------------------------------------------------*/

Int32 
CMultiLogger::GetMinimalLogLevel( void ) const
{GUCEF_TRACE;

    return m_minLogLevel;
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::Log( const TLogMsgType logMsgType ,
                   const Int32 logLevel         ,
                   const CString& logMessage    )
{GUCEF_TRACE;

    Log( logMsgType                  ,
         logLevel                    ,
         logMessage                  ,
         MT::GetCurrentThreadSystemID() ,
         CTimestamp::NowUTCTime()    );
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::Log( const TLogMsgType logMsgType ,
                   const Int32 logLevel         ,
                   const CString& logMessage    ,
                   const TSystemThreadId threadId        ,
                   const CTimestamp& timestamp  )
{GUCEF_TRACE;

    // The loglevel must be such so that the message given falls under the global logging
    // cut off.
    // Additionally if a thread generates log message via logging logic then this
    // would result in a endless loop. To this end the "m_busyLogging" flag is used
    // if this flag is true then log messages will be dropped since the thread is within
    // the logger->Log() call and thus the message is generated due to the logging
    // activity itself.
    if ( logLevel >= m_minLogLevel )
    {
        if ( IsLogMsgTypeEnabled( logMsgType ) )
        {
            for ( TLoggerList::size_type i=0; i<m_loggers.size(); ++i )
            {
                CILogger* logger = m_loggers[ i ];
                if ( GUCEF_NULL != logger )
                {
                    logger->Log( logMsgType ,
                                 logLevel   ,
                                 logMessage ,
                                 threadId   ,
                                 timestamp  );
                }
            }

            // We want to make certain that errors are always in the log file.
            // We might crash moments later which might cause some loggers not
            // to write the error entry to their respective output media
            if ( LOG_ERROR == logMsgType || LOG_EXCEPTION == logMsgType )
            {
                FlushLog();
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::LogWithoutFormatting( const TLogMsgType logMsgType ,
                                    const Int32 logLevel         ,
                                    const CString& logMessage    ,
                                    const TSystemThreadId threadId        ,
                                    const CTimestamp& timestamp  )
{GUCEF_TRACE;

    // The loglevel must be such so that the message given falls under the global logging
    // cut off.
    // Additionally if a thread generates log message via logging logic then this
    // would result in a endless loop. To this end the "m_busyLogging" flag is used
    // if this flag is true then log messages will be dropped since the thread is within
    // the logger->Log() call and thus the message is generated due to the logging
    // activity itself.
    if ( logLevel >= m_minLogLevel )
    {
        if ( IsLogMsgTypeEnabled( logMsgType ) )
        {
            for ( TLoggerList::size_type i=0; i<m_loggers.size(); ++i )
            {
                CILogger* logger = m_loggers[ i ];
                if ( GUCEF_NULL != logger )
                {
                    logger->LogWithoutFormatting( logMsgType ,
                                                  logLevel   ,
                                                  logMessage ,
                                                  threadId   ,
                                                  timestamp  );
                }
            }

            // We want to make certain that errors are always in the log file.
            // We might crash moments later which might cause some loggers not
            // to write the error entry to their respective output media
            if ( LOG_ERROR == logMsgType )
            {
                FlushLog();
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::Log( const TLogMsgType logMsgType     ,
                   const Int32 logLevel             ,
                   const CVariantStream& logMessage ,
                   const TSystemThreadId threadId   ,
                   const CTimestamp& timestamp      )
{GUCEF_TRACE;

    if ( logLevel >= m_minLogLevel )
    {
        if ( IsLogMsgTypeEnabled( logMsgType ) )
        {
            for ( TLoggerList::size_type i=0; i<m_loggers.size(); ++i )
            {
                CILogger* logger = m_loggers[ i ];
                if ( GUCEF_NULL != logger )
                {
                    logger->Log( logMsgType ,
                                 logLevel   ,
                                 logMessage ,
                                 threadId   ,
                                 timestamp  );
                }
            }

            if ( LOG_ERROR == logMsgType || LOG_EXCEPTION == logMsgType )
            {
                FlushLog();
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                    const Int32 logLevel             ,
                                    const CVariantStream& logMessage ,
                                    const TSystemThreadId threadId   ,
                                    const CTimestamp& timestamp      )
{GUCEF_TRACE;

    if ( logLevel >= m_minLogLevel )
    {
        if ( IsLogMsgTypeEnabled( logMsgType ) )
        {
            for ( TLoggerList::size_type i=0; i<m_loggers.size(); ++i )
            {
                CILogger* logger = m_loggers[ i ];
                if ( GUCEF_NULL != logger )
                {
                    logger->LogWithoutFormatting( logMsgType ,
                                                  logLevel   ,
                                                  logMessage ,
                                                  threadId   ,
                                                  timestamp  );
                }
            }

            if ( LOG_ERROR == logMsgType )
            {
                FlushLog();
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void 
CMultiLogger::FlushLog( void )
{GUCEF_TRACE;

    for ( TLoggerList::size_type i=0; i<m_loggers.size(); ++i )
    {
        CILogger* logger = m_loggers[ i ];
        if ( GUCEF_NULL != logger )
        {
            logger->FlushLog();
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
