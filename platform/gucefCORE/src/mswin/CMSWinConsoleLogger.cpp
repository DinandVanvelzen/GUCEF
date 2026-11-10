/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <assert.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

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

#ifdef GUCEF_MSWIN_BUILD
#include <windows.h>
#include <wincon.h>          /* for COORD */
#include <conio.h>           /* need conio.h under WIN32 for clrscr() */
#endif /* GUCEF_MSWIN_BUILD ? */

#include "CMSWinConsoleLogger.h"

#ifndef GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#include "gucefCORE_CStandardOutputWriter.h"
#define GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#endif /* GUCEF_CORE_CSTANDARDOUTPUTWRITER_H ? */

#ifdef GUCEF_MSWIN_BUILD
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

CMSWinConsoleLogger::CMSWinConsoleLogger( bool ownWindow )
    : CIConsoleLogger()
    , m_minimalLogLevel( LOGLEVEL_BELOW_NORMAL )
    , m_formatForUiPurpose( false )
    , m_consoleHandle( NULL )
    , m_ownedConsole( false )
    , m_leftParentConsole( false )
    , m_originalConsoleAttribs( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE )
    , m_logFormatter( CCoreGlobal::Instance()->GetLogManager().CreateDefaultLoggingFormatter() )
{GUCEF_TRACE;

    HWND consoleWindow = ::GetConsoleWindow();
    if ( NULL != consoleWindow && ownWindow )
    {
        // a process has one console: the one it was started from is let go for a window of its own
        ::FreeConsole();
        m_leftParentConsole = true;
        consoleWindow = NULL;
    }
    if ( NULL == consoleWindow )
    {
        ::AllocConsole();
        m_ownedConsole = true;
    }
    m_consoleHandle = ::GetStdHandle( STD_OUTPUT_HANDLE );

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if ( ::GetConsoleScreenBufferInfo( m_consoleHandle, &csbi ) )
        m_originalConsoleAttribs = csbi.wAttributes;

    //if ( consoleHandle != NULL )
    //{
    //    HMENU hMenu = ::GetSystemMenu( consoleHandle, FALSE);
    //    if (hMenu != NULL) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
    //}
}

/*-------------------------------------------------------------------------*/

CMSWinConsoleLogger::~CMSWinConsoleLogger()
{GUCEF_TRACE;

    CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this );
    FlushLog();

    if ( m_ownedConsole )
        ::FreeConsole();
    if ( m_leftParentConsole )
        ::AttachConsole( ATTACH_PARENT_PROCESS );
}

/*-------------------------------------------------------------------------*/

void
CMSWinConsoleLogger::SetFormatAsConsoleUI( bool formatForUiPurpose )
{GUCEF_TRACE;
    
    m_formatForUiPurpose = formatForUiPurpose;
}

/*-------------------------------------------------------------------------*/
    
bool
CMSWinConsoleLogger::GetFormatAsConsoleUI( void ) const
{GUCEF_TRACE;

    return m_formatForUiPurpose;
}

/*-------------------------------------------------------------------------*/

WORD
CMSWinConsoleLogger::GetColorForMsgType( const TLogMsgType logMsgType ) const
{GUCEF_TRACE;

    switch ( logMsgType )
    {
        case LOG_ERROR:     return FOREGROUND_RED | FOREGROUND_INTENSITY;                                           /* bright red     */
        case LOG_EXCEPTION: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;                        /* bright magenta */
        case LOG_WARNING:   return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;                       /* bright yellow  */
        case LOG_SYSTEM:    return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;                      /* bright cyan    */
        case LOG_DEV:       return FOREGROUND_GREEN | FOREGROUND_INTENSITY;                                        /* bright green   */
        case LOG_DEBUG:     return FOREGROUND_INTENSITY;                                                           /* dark gray      */
        case LOG_CALLSTACK: return FOREGROUND_GREEN | FOREGROUND_BLUE;                                             /* dark cyan      */
        case LOG_SERVICE:   return FOREGROUND_BLUE | FOREGROUND_INTENSITY;                                         /* bright blue    */
        case LOG_PROTECTED: return FOREGROUND_RED | FOREGROUND_BLUE;                                               /* dark magenta   */
        case LOG_USER:      return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;     /* bright white   */
        case LOG_CONSOLE:   return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;     /* bright white   */
        case LOG_STANDARD:
        default:            return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;                            /* light gray     */
    }
}

/*-------------------------------------------------------------------------*/

void
CMSWinConsoleLogger::Log( const TLogMsgType logMsgType ,
                          const Int32 logLevel         ,
                          const CString& logMessage    ,
                          const UInt32 threadId        ,
                          const CTimestamp& timestamp  )
{GUCEF_TRACE;

    // stdout carries a protocol from the claim on: the console of stdout is not written to then, also when this
    // logger was made before the claim
    if ( CStandardOutputWriter::IsClaimed() )
        return;

    if ( !m_formatForUiPurpose )
    {
        if ( logLevel >= m_minimalLogLevel )
        {
            CString actualLogMsg( m_logFormatter->FormatLogMessage( logMsgType ,
                                                                    logLevel   ,
                                                                    logMessage ,
                                                                    threadId   ,
                                                                    timestamp  ) + "\n" );

            DWORD charsWritten = 0;
            ::SetConsoleTextAttribute( m_consoleHandle, GetColorForMsgType( logMsgType ) );
            ::WriteConsoleA( m_consoleHandle, actualLogMsg.C_String(), (DWORD)actualLogMsg.Length(), &charsWritten, NULL );
            ::SetConsoleTextAttribute( m_consoleHandle, m_originalConsoleAttribs );
        }
    }
    else
    {
        if ( logMsgType == CORE::LOG_CONSOLE )
        {
            DWORD charsWritten = 0;
            ::SetConsoleTextAttribute( m_consoleHandle, GetColorForMsgType( logMsgType ) );
            ::WriteConsoleA( m_consoleHandle, logMessage.C_String(), (DWORD)logMessage.Length(), &charsWritten, NULL );
            ::SetConsoleTextAttribute( m_consoleHandle, m_originalConsoleAttribs );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMSWinConsoleLogger::LogWithoutFormatting( const TLogMsgType logMsgType ,
                                           const Int32 logLevel         ,
                                           const CString& logMessage    ,
                                           const UInt32 threadId        ,
                                           const CTimestamp& timestamp  )
{GUCEF_TRACE;

    // stdout carries a protocol from the claim on: the console of stdout is not written to then, also when this
    // logger was made before the claim
    if ( CStandardOutputWriter::IsClaimed() )
        return;

    if ( !m_formatForUiPurpose )
    {
        if ( logLevel >= m_minimalLogLevel || m_formatForUiPurpose )
        {
            DWORD charsWritten = 0;
            ::SetConsoleTextAttribute( m_consoleHandle, GetColorForMsgType( logMsgType ) );
            ::WriteConsoleA( m_consoleHandle, logMessage.C_String(), (DWORD)logMessage.Length(), &charsWritten, NULL );
            ::SetConsoleTextAttribute( m_consoleHandle, m_originalConsoleAttribs );
        }
    }
    else
    {
        if ( logMsgType == CORE::LOG_CONSOLE )
        {
            DWORD charsWritten = 0;
            ::SetConsoleTextAttribute( m_consoleHandle, GetColorForMsgType( logMsgType ) );
            ::WriteConsoleA( m_consoleHandle, logMessage.C_String(), (DWORD)logMessage.Length(), &charsWritten, NULL );
            ::SetConsoleTextAttribute( m_consoleHandle, m_originalConsoleAttribs );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMSWinConsoleLogger::Log( const TLogMsgType logMsgType     ,
                          const Int32 logLevel             ,
                          const CVariantStream& logMessage ,
                          const UInt32 threadId            ,
                          const CTimestamp& timestamp      )
{GUCEF_TRACE;

    // @TODO: We should avoid the ToString conversion here and instead have a way to
    // write the CVariantStream content directly to the console
    Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CMSWinConsoleLogger::LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                           const Int32 logLevel             ,
                                           const CVariantStream& logMessage ,
                                           const UInt32 threadId            ,
                                           const CTimestamp& timestamp      )
{GUCEF_TRACE;

    LogWithoutFormatting( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CMSWinConsoleLogger::FlushLog( void )
{GUCEF_TRACE;

    ::FlushConsoleInputBuffer( m_consoleHandle );
}

/*-------------------------------------------------------------------------*/

void
CMSWinConsoleLogger::SetMinimalLogLevel( const Int32 minimalLogLevel )
{GUCEF_TRACE;

    m_minimalLogLevel = minimalLogLevel;
}

/*-------------------------------------------------------------------------*/

Int32
CMSWinConsoleLogger::GetMinimalLogLevel( void ) const
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
#endif /* GUCEF_MSWIN_BUILD ? */
