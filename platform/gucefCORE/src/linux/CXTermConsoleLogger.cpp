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

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifdef GUCEF_LINUX_BUILD

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#endif /* GUCEF_LINUX_BUILD ? */

#include "CXTermConsoleLogger.h"

#ifdef GUCEF_LINUX_BUILD

#include "gucefCORE_pty.h"

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

CXTermConsoleLogger::CXTermConsoleLogger( bool initialize )
    : CIConsoleLogger()
    , m_minimalLogLevel( LOGLEVEL_BELOW_NORMAL )
    , m_formatForUiPurpose( false )
    , m_xtermpid( -1 )
    , m_masterfd( -1 )
    , m_slavefd( -1 )
    , m_slaveFptr( NULL )
    , m_logFormatter( CCoreGlobal::Instance()->GetLogManager().CreateDefaultLoggingFormatter() )
{GUCEF_TRACE;

    if ( initialize )
    {
        Initialize();
    }
}

/*-------------------------------------------------------------------------*/

CXTermConsoleLogger::~CXTermConsoleLogger()
{GUCEF_TRACE;

    if ( NULL != m_slaveFptr )
    {
        fclose( m_slaveFptr );
        m_slaveFptr = NULL;
    }
    if ( m_masterfd != -1 )
    {
        close( m_masterfd );
        m_masterfd = -1;
    }
    if ( m_slavefd != -1 )
    {
        close( m_slavefd );
        m_slavefd = -1;
    }
}

/*-------------------------------------------------------------------------*/

bool
CXTermConsoleLogger::Initialize( void )
{GUCEF_TRACE;

    if ( ( m_masterfd != -1 ) || ( m_slavefd != -1 ) )
    {
        // Do not initialize twice, already initialized
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "CXTermConsoleLogger: Initialize() called but initialization already occured" );
        return true;
    }

    if ( 0 != GUCEF_pty_open( &m_masterfd, &m_slavefd ) )
    {
        // some error occured opening the pseudo terminal
        m_masterfd = -1;
        m_slavefd = -1;
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CXTermConsoleLogger: failed to open pseudo terminal master and slave" );
        return false;
    }

    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CXTermConsoleLogger: opened pseudo terminal master and slave" );

    m_slaveFptr = fdopen( m_slavefd, "w" );
    if ( NULL == m_slaveFptr )
    {
        close( m_masterfd );
        m_masterfd = -1;
        close( m_slavefd );
        m_slavefd = -1;
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CXTermConsoleLogger: failed to open file stream based on slave file descriptor" );
        return false;
    }


    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "CXTermConsoleLogger: opened file stream to pseudo terminal slave" );

    // HERE: do any termios reconfiguration which is required
    // If you don't want the windowid to be echoed, you must
    // at least switch off echo here.


    char sopt[ 17+20 ];

    // Fire up an xterm on the master side of the pseudo tty
    //sprintf( sopt, "-title Logging -Sxx%d", m_masterfd );
    sprintf( sopt, "-Sxx%d", m_masterfd );

    fflush( stderr );
    fflush( stdout );
    m_xtermpid = fork();

    switch( m_xtermpid )
    {
        case -1:
        {
            close( m_slavefd );
            m_slavefd = -1;
            close( m_masterfd );
            m_masterfd = -1;

            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CXTermConsoleLogger: failed fork a xterm, fork returned -1" );
            return false;
        }
        case 0:
        {
            for (int i=0; i<100; i++)
            {
                if ( i != STDERR_FILENO && i != m_masterfd )
                {
                    close(i);
                }
            }
            execl("/usr/bin/xterm", "xterm", sopt, NULL);
            perror("/usr/bin/xterm");

            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CXTermConsoleLogger: failed fork a xterm, fork returned 0" );
            return false;
        }
        default:
        {
            CString pts_name = ptsname( m_masterfd );

            //close( m_masterfd );
            //m_masterfd = -1;

            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "CXTermConsoleLogger: starting xterm (pid " + Int32ToString( m_xtermpid ) + ") on " + pts_name );
            //fprintf( stderr, "starting xterm (pid %d) on %s\n", m_xtermpid, pts_name.C_String() );

//            // Discard windowid, which comes back as first line from xterm
//            // (this over-simple bit of code isn't really good enough...)
//            if ( read( m_slavefd, pts_name, sizeof(pts_name) - 1 ) < 0 )
//            {
//                perror( "read slavefd" );
//                return false;
//            }
        }
    }

    // now do i/o on slavefd to read/write the xterm
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "CXTermConsoleLogger: Successfully initialized" );
    return true;

}

/*-------------------------------------------------------------------------*/

void
CXTermConsoleLogger::SetFormatAsConsoleUI( bool formatForUiPurpose )
{GUCEF_TRACE;

    m_formatForUiPurpose = formatForUiPurpose;
}

/*-------------------------------------------------------------------------*/

bool
CXTermConsoleLogger::GetFormatAsConsoleUI( void ) const
{GUCEF_TRACE;

    return m_formatForUiPurpose;
}

/*-------------------------------------------------------------------------*/

void
CXTermConsoleLogger::Log( const TLogMsgType logMsgType ,
                          const Int32 logLevel         ,
                          const CString& logMessage    ,
                          const UInt32 threadId        ,
                          const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( m_slavefd != -1 && m_xtermpid != -1 && m_slaveFptr != NULL )
    {
        if ( !m_formatForUiPurpose )
        {
            if ( logLevel >= m_minimalLogLevel )
            {
                CString actualLogMsg( m_logFormatter->FormatLogMessage( logMsgType ,
                                                                        logLevel   ,
                                                                        logMessage ,
                                                                        threadId   ,
                                                                        timestamp  ) + "\n" );

                fprintf( m_slaveFptr, "%s", actualLogMsg.C_String() );
            }
        }
        else
        {
            if ( logMsgType == CORE::LOG_CONSOLE )
            {
                CString actualLogMsg( logMessage + "\n" );
                fprintf( m_slaveFptr, "%s", actualLogMsg.C_String() );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CXTermConsoleLogger::LogWithoutFormatting( const TLogMsgType logMsgType ,
                                           const Int32 logLevel         ,
                                           const CString& logMessage    ,
                                           const UInt32 threadId        ,
                                           const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( m_slavefd != -1 && m_xtermpid != -1 && m_slaveFptr != NULL )
    {
        if ( !m_formatForUiPurpose )
        {
            if ( logLevel >= m_minimalLogLevel )
            {
                CString actualLogMsg( logMessage + "\n" );
                fprintf( m_slaveFptr, "%s", actualLogMsg.C_String() );
            }
        }
        else
        {
            if ( logMsgType == CORE::LOG_CONSOLE )
            {
                CString actualLogMsg( logMessage + "\n" );
                fprintf( m_slaveFptr, "%s", actualLogMsg.C_String() );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CXTermConsoleLogger::Log( const TLogMsgType logMsgType     ,
                          const Int32 logLevel             ,
                          const CVariantStream& logMessage ,
                          const UInt32 threadId            ,
                          const CTimestamp& timestamp      )
{GUCEF_TRACE;

    Log( logMsgType, logLevel, logMessage.AsString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CXTermConsoleLogger::LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                           const Int32 logLevel             ,
                                           const CVariantStream& logMessage ,
                                           const UInt32 threadId            ,
                                           const CTimestamp& timestamp      )
{GUCEF_TRACE;

    LogWithoutFormatting( logMsgType, logLevel, logMessage.AsString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CXTermConsoleLogger::FlushLog( void )
{GUCEF_TRACE;

    if ( m_slavefd != -1 && m_slaveFptr != NULL )
    {
        fflush( m_slaveFptr );
    }
}

/*-------------------------------------------------------------------------*/

void
CXTermConsoleLogger::SetMinimalLogLevel( const Int32 minimalLogLevel )
{GUCEF_TRACE;

    m_minimalLogLevel = minimalLogLevel;
}

/*-------------------------------------------------------------------------*/

Int32
CXTermConsoleLogger::GetMinimalLogLevel( void ) const
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
#endif /* GUCEF_LINUX_BUILD ? */
