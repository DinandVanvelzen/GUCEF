/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include "gucefCORE_CTextLineConsoleLogger.h"

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

CTextLineConsoleLogger::CTextLineConsoleLogger( void )
    : CIConsoleLogger()
    , m_minimalLogLevel( LOGLEVEL_BELOW_NORMAL )
    , m_formatForUiPurpose( false )
    , m_logFormatter( CCoreGlobal::Instance()->GetLogManager().CreateDefaultLoggingFormatter() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTextLineConsoleLogger::~CTextLineConsoleLogger()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTextLineConsoleLogger::Log( const TLogMsgType logMsgType ,
                             const Int32 logLevel         ,
                             const CString& logMessage    ,
                             const UInt32 threadId        ,
                             const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( !m_formatForUiPurpose )
    {
        if ( logLevel >= m_minimalLogLevel )
        {
            WriteLine( m_logFormatter->FormatLogMessage( logMsgType ,
                                                         logLevel   ,
                                                         logMessage ,
                                                         threadId   ,
                                                         timestamp  ) );
        }
    }
    else
    if ( logMsgType == LOG_CONSOLE )
    {
        WriteLine( logMessage );
    }
}

/*-------------------------------------------------------------------------*/

void
CTextLineConsoleLogger::LogWithoutFormatting( const TLogMsgType logMsgType ,
                                              const Int32 logLevel         ,
                                              const CString& logMessage    ,
                                              const UInt32 threadId        ,
                                              const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( !m_formatForUiPurpose )
    {
        if ( logLevel >= m_minimalLogLevel )
            WriteLine( logMessage );
    }
    else
    if ( logMsgType == LOG_CONSOLE )
    {
        WriteLine( logMessage );
    }
}

/*-------------------------------------------------------------------------*/

void
CTextLineConsoleLogger::Log( const TLogMsgType logMsgType     ,
                             const Int32 logLevel             ,
                             const CVariantStream& logMessage ,
                             const UInt32 threadId            ,
                             const CTimestamp& timestamp      )
{GUCEF_TRACE;

    Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CTextLineConsoleLogger::LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                              const Int32 logLevel             ,
                                              const CVariantStream& logMessage ,
                                              const UInt32 threadId            ,
                                              const CTimestamp& timestamp      )
{GUCEF_TRACE;

    LogWithoutFormatting( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

void
CTextLineConsoleLogger::FlushLog( void )
{GUCEF_TRACE;

    OnFlushLog();
}

/*-------------------------------------------------------------------------*/

void
CTextLineConsoleLogger::OnFlushLog( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTextLineConsoleLogger::SetMinimalLogLevel( const Int32 minimalLogLevel )
{GUCEF_TRACE;

    m_minimalLogLevel = minimalLogLevel;
}

/*-------------------------------------------------------------------------*/

Int32
CTextLineConsoleLogger::GetMinimalLogLevel( void ) const
{GUCEF_TRACE;

    return m_minimalLogLevel;
}

/*-------------------------------------------------------------------------*/

void
CTextLineConsoleLogger::SetFormatAsConsoleUI( bool formatForUiPurpose )
{GUCEF_TRACE;

    m_formatForUiPurpose = formatForUiPurpose;
}

/*-------------------------------------------------------------------------*/

bool
CTextLineConsoleLogger::GetFormatAsConsoleUI( void ) const
{GUCEF_TRACE;

    return m_formatForUiPurpose;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
