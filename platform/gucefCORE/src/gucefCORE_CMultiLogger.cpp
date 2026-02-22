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
        m_msgTypeEnablers = src.m_msgTypeEnablers;
        m_minLogLevel = src.m_minLogLevel; 
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt32 
CMultiLogger::GetLoggerCount( void ) const
{GUCEF_TRACE;
    
    return (UInt32) m_loggers.size();
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::AddLogger( CILogger* loggerImp )
{GUCEF_TRACE;

    // Do not add bogus loggers
    if ( GUCEF_NULL != loggerImp )
    {
        m_loggers.insert( loggerImp );
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "MultiLogger: Added logger" );
    }
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "MultiLogger: Null given for logger" );
    }
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::RemoveLogger( CILogger* loggerImp )
{GUCEF_TRACE;

    // Do not pass in bogus pointers
    assert( GUCEF_NULL != loggerImp );

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "MultiLogger: Removing logger" );

    m_loggers.erase( loggerImp );
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::ClearLoggers( void )
{GUCEF_TRACE;

    m_loggers.clear();
}

/*-------------------------------------------------------------------------*/

 bool
 CMultiLogger::IsLoggingEnabled( const TLogMsgType logMsgType ,
                                 const Int32 logLevel         ) const
{GUCEF_TRACE;

    bool retValue = false;
    if ( logLevel < m_minLogLevel )
    {
        retValue = (*m_msgTypeEnablers.find( logMsgType )).second;
    }

    return retValue;
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::SetMinimalLogLevel( const Int32 logLevel )
{GUCEF_TRACE;

    m_minLogLevel = logLevel;
    TLoggerList::const_iterator i = m_loggers.begin();
    while ( i != m_loggers.end() )
    {
        CILogger* logger = (*i);
        if ( GUCEF_NULL != logger )
        {
            logger->SetMinimalLogLevel( logLevel );
        }
        ++i;
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
         MT::GetCurrentTaskID()      ,
         CTimestamp::NowUTCTime()    );
}

/*-------------------------------------------------------------------------*/

void
CMultiLogger::Log( const TLogMsgType logMsgType ,
                   const Int32 logLevel         ,
                   const CString& logMessage    ,
                   const UInt32 threadId        ,
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
        if ( (*m_msgTypeEnablers.find( logMsgType )).second )
        {
            TLoggerList::const_iterator i = m_loggers.begin();
            while ( i != m_loggers.end() )
            {
                CILogger* logger = (*i);
                if ( GUCEF_NULL != logger )
                {
                    logger->Log( logMsgType ,
                                 logLevel   ,
                                 logMessage ,
                                 threadId   ,
                                 timestamp  );
                }
                ++i;
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
                                    const UInt32 threadId        ,
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
        if ( (*m_msgTypeEnablers.find( logMsgType )).second )
        {
            TLoggerList::const_iterator i = m_loggers.begin();
            while ( i != m_loggers.end() )
            {
                CILogger* logger = (*i);
                if ( GUCEF_NULL != logger )
                {
                    logger->LogWithoutFormatting( logMsgType ,
                                                  logLevel   ,
                                                  logMessage ,
                                                  threadId   ,
                                                  timestamp  );
                }
                ++i;
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
CMultiLogger::FlushLog( void )
{GUCEF_TRACE;

    TLoggerList::const_iterator i = m_loggers.begin();
    while ( i != m_loggers.end() )
    {
        CILogger* logger = (*i);
        if ( GUCEF_NULL != logger )
        {
            logger->FlushLog();
        }
        ++i;
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
