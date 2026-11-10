/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefCORE_CUserActionLogger.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUserActionLogger::CUserActionLogger( const CString& filePath )
    : CILogger()
    , m_filePath( RelativePath( filePath ) )
    , m_file()
    , m_logger()
{GUCEF_TRACE;

    CreatePathDirectories( m_filePath );
    if ( m_file.Open( m_filePath, "ab" ) )
    {
        m_logger.SetOutput( m_file );
        m_logger.Log( LOG_USER, LOGLEVEL_CRITICAL, "---- a new session ----", MT::GetCurrentTaskID(), CTimestamp::NowUTCTime() );
        m_logger.FlushLog();
    }
}

/*-------------------------------------------------------------------------*/

CUserActionLogger::~CUserActionLogger()
{GUCEF_TRACE;

    m_logger.FlushLog();
}

/*-------------------------------------------------------------------------*/

bool
CUserActionLogger::IsOpen( void ) const
{GUCEF_TRACE;

    return m_file.Opened();
}

/*-------------------------------------------------------------------------*/

const CString&
CUserActionLogger::GetFilePath( void ) const
{GUCEF_TRACE;

    return m_filePath;
}

/*-------------------------------------------------------------------------*/

void
CUserActionLogger::Log( const TLogMsgType logMsgType ,
                        const Int32 logLevel         ,
                        const CString& logMessage    ,
                        const UInt32 threadId        ,
                        const CTimestamp& timestamp  )
{GUCEF_TRACE;

    // each action goes to the file at once: what led to a crash, or a program that had to be killed, is what the log is for
    if ( LOG_USER == logMsgType && IsOpen() )
    {
        m_logger.Log( logMsgType, logLevel, logMessage, threadId, timestamp );
        m_logger.FlushLog();
    }
}

/*-------------------------------------------------------------------------*/

void
CUserActionLogger::Log( const TLogMsgType logMsgType     ,
                        const Int32 logLevel             ,
                        const CVariantStream& logMessage ,
                        const UInt32 threadId            ,
                        const CTimestamp& timestamp      )
{GUCEF_TRACE;

    // each action goes to the file at once: what led to a crash, or a program that had to be killed, is what the log is for
    if ( LOG_USER == logMsgType && IsOpen() )
    {
        m_logger.Log( logMsgType, logLevel, logMessage, threadId, timestamp );
        m_logger.FlushLog();
    }
}

/*-------------------------------------------------------------------------*/

void
CUserActionLogger::LogWithoutFormatting( const TLogMsgType logMsgType ,
                                         const Int32 logLevel         ,
                                         const CString& logMessage    ,
                                         const UInt32 threadId        ,
                                         const CTimestamp& timestamp  )
{GUCEF_TRACE;

    // each action goes to the file at once: what led to a crash, or a program that had to be killed, is what the log is for
    if ( LOG_USER == logMsgType && IsOpen() )
    {
        m_logger.LogWithoutFormatting( logMsgType, logLevel, logMessage, threadId, timestamp );
        m_logger.FlushLog();
    }
}

/*-------------------------------------------------------------------------*/

void
CUserActionLogger::LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                         const Int32 logLevel             ,
                                         const CVariantStream& logMessage ,
                                         const UInt32 threadId            ,
                                         const CTimestamp& timestamp      )
{GUCEF_TRACE;

    // each action goes to the file at once: what led to a crash, or a program that had to be killed, is what the log is for
    if ( LOG_USER == logMsgType && IsOpen() )
    {
        m_logger.LogWithoutFormatting( logMsgType, logLevel, logMessage, threadId, timestamp );
        m_logger.FlushLog();
    }
}

/*-------------------------------------------------------------------------*/

void
CUserActionLogger::FlushLog( void )
{GUCEF_TRACE;

    m_logger.FlushLog();
}

/*-------------------------------------------------------------------------*/

void
CUserActionLogger::SetMinimalLogLevel( const Int32 logLevel )
{GUCEF_TRACE;

    m_logger.SetMinimalLogLevel( logLevel );
}

/*-------------------------------------------------------------------------*/

Int32
CUserActionLogger::GetMinimalLogLevel( void ) const
{GUCEF_TRACE;

    return m_logger.GetMinimalLogLevel();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
