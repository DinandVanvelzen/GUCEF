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

#include "gucefCORE_CAndroidSystemLogger.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

#include <android/log.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CILOGGINGFORMATTER_H
#include "gucefCORE_CILoggingFormatter.h"
#define GUCEF_CORE_CILOGGINGFORMATTER_H
#endif /* GUCEF_CORE_CILOGGINGFORMATTER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef void ( *TAndroidLogWriteFunc )( android_LogPriority, const char*, const char* );

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static android_LogPriority
LogTypeAndLevelToAndroidPrio( const TLogMsgType logMsgType ,
                              const Int32 logLevel         )
{
    switch ( logMsgType )
    {
        case ELogMsgType::LOG_EXCEPTION:
        case ELogMsgType::LOG_ERROR:
        {
            if ( logLevel < LOGLEVEL_CRITICAL )
            {
                if ( logLevel >= LOGLEVEL_NORMAL )
                {
                    return ANDROID_LOG_ERROR;
                }
                return ANDROID_LOG_WARN;
            }
            return ANDROID_LOG_FATAL;
        }
        case ELogMsgType::LOG_DEV:
        case ELogMsgType::LOG_DEBUG:
        case ELogMsgType::LOG_CALLSTACK:
        {
            if ( logLevel >= LOGLEVEL_NORMAL )
            {
                return ANDROID_LOG_DEBUG;
            }
            return ANDROID_LOG_VERBOSE;
        }
        case ELogMsgType::LOG_USER:
        case ELogMsgType::LOG_STANDARD:
        case ELogMsgType::LOG_SYSTEM:
        case ELogMsgType::LOG_SERVICE:
        {
            if ( logLevel >= LOGLEVEL_NORMAL )
            {
                return ANDROID_LOG_INFO;
            }
            if ( logLevel >= LOGLEVEL_BELOW_NORMAL )
            {
                return ANDROID_LOG_DEBUG;
            }
            return ANDROID_LOG_VERBOSE;
        }
        case ELogMsgType::LOG_PROTECTED:
        {
            // Android does not support encypted logs
            return ANDROID_LOG_SILENT;
        }
        case ELogMsgType::LOG_UNKNOWNTYPE:
        {
            return ANDROID_LOG_UNKNOWN;
        }
        default:
        {
            return ANDROID_LOG_DEFAULT;
        }
    }
}

/*-------------------------------------------------------------------------*/

CAndroidSystemLogger::CAndroidSystemLogger( void )
    : CILogger()        
    , m_tag()           
    , m_logFunc( GUCEF_NULL )
    , m_logFormatter( CCoreGlobal::Instance()->GetLogManager().CreateDefaultLoggingFormatter() )
{GUCEF_TRACE;

    m_tag = "GUCEF:AndroidSystemLogger:" + PointerToString( this );

    // We load the log module dynamically to avoid adding dependencies
    void* logModule = LoadModuleDynamicly( "log.so" );
    if ( NULL != logModule )
    {
        // Find the address of the Android log function
        void* funcAddr = GetFunctionAddress( logModule                                     ,
                                             "__android_log_write"                         ,
                                             sizeof(android_LogPriority)+(2*sizeof(char*)) ).objPtr;
        if ( GUCEF_NULL != funcAddr )
        {
            m_logFunc = funcAddr;
        }
    }
}

/*-------------------------------------------------------------------------*/

CAndroidSystemLogger::~CAndroidSystemLogger()
{GUCEF_TRACE;

    CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this );
    FlushLog();

    delete m_logFormatter;
    m_logFormatter = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CAndroidSystemLogger::Log( const TLogMsgType logMsgType ,
                           const Int32 logLevel         ,
                           const CString& logMessage    ,
                           const UInt32 threadId        ,
                           const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_logFunc && logLevel >= m_minimalLogLevel )
    {
        CString formattedLogMsg( m_logFormatter->FormatLogMessage( logMsgType ,
                                                                   logLevel   ,
                                                                   logMessage ,
                                                                   threadId   ,
                                                                   timestamp  ) + "\n" );

        android_LogPriority androidLogPrio = LogTypeAndLevelToAndroidPrio( logMsgType, logLevel );
        ( (TAndroidLogWriteFunc) m_logFunc )( androidLogPrio, m_tag.C_String(), formattedLogMsg.C_String() );
    }
}

/*-------------------------------------------------------------------------*/

void
CAndroidSystemLogger::LogWithoutFormatting( const TLogMsgType logMsgType ,
                                            const Int32 logLevel         ,
                                            const CString& logMessage    ,
                                            const UInt32 threadId        ,
                                            const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_logFunc && logLevel >= m_minimalLogLevel )
    {
        android_LogPriority androidLogPrio = LogTypeAndLevelToAndroidPrio( logMsgType, logLevel );
        ( (TAndroidLogWriteFunc) m_logFunc )( androidLogPrio, m_tag.C_String(), logMessage.C_String() );
    }
}

/*-------------------------------------------------------------------------*/

void
CAndroidSystemLogger::Log( const TLogMsgType logMsgType     ,
                           const Int32 logLevel             ,
                           const CVariantStream& logMessage ,
                           const UInt32 threadId            ,
                           const CTimestamp& timestamp      )
{GUCEF_TRACE;

    Log( logMsgType, logLevel, logMessage.AsString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CAndroidSystemLogger::LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                            const Int32 logLevel             ,
                                            const CVariantStream& logMessage ,
                                            const UInt32 threadId            ,
                                            const CTimestamp& timestamp      )
{GUCEF_TRACE;

    LogWithoutFormatting( logMsgType, logLevel, logMessage.AsString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CAndroidSystemLogger::FlushLog( void )
{GUCEF_TRACE;

    // Android does not support direct flush control of its system log
}

/*-------------------------------------------------------------------------*/

void 
CAndroidSystemLogger::SetMinimalLogLevel( const Int32 logLevel ) 
{GUCEF_TRACE;

    m_minimalLogLevel = logLevel;
}

/*-------------------------------------------------------------------------*/

Int32 
CAndroidSystemLogger::GetMinimalLogLevel( void ) const 
{GUCEF_TRACE;

    return m_minimalLogLevel;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ? */
