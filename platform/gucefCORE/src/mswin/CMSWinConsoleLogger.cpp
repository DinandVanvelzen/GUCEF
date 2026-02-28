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

CMSWinConsoleLogger::CMSWinConsoleLogger( void )
    : CIConsoleLogger()
    , m_minimalLogLevel( LOGLEVEL_BELOW_NORMAL )
    , m_formatForUiPurpose( false )
    , m_consoleHandle( NULL )
    , m_ownedConsole( false )
    , m_originalConsoleAttribs( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE )
    , m_logFormatter( CCoreGlobal::Instance()->GetLogManager().CreateDefaultLoggingFormatter() )
{GUCEF_TRACE;

    HWND consoleWindow = ::GetConsoleWindow();
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
