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

#include "CStdLogger.h"

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

CStdLogger::CStdLogger( void )
    : CILogger()                                 
    , m_output( NULL )                           
    , m_minimalLogLevel( LOGLEVEL_BELOW_NORMAL ) 
    , m_logFormatter( CCoreGlobal::Instance()->GetLogManager().CreateDefaultLoggingFormatter() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStdLogger::CStdLogger( CIOAccess& output )
    : CILogger()                                 
    , m_output( &output )                        
    , m_minimalLogLevel( LOGLEVEL_BELOW_NORMAL )
    , m_logFormatter( CCoreGlobal::Instance()->GetLogManager().CreateDefaultLoggingFormatter() )
{GUCEF_TRACE;

    assert( &output != NULL );
    assert( m_output != NULL );
}

/*-------------------------------------------------------------------------*/

CStdLogger::~CStdLogger()
{GUCEF_TRACE;

    CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this );
    FlushLog();
}

/*-------------------------------------------------------------------------*/

void
CStdLogger::SetOutput( CIOAccess& output )
{GUCEF_TRACE;

    assert( &output != NULL );
    m_output = &output;
    m_output->Open();
}

/*-------------------------------------------------------------------------*/

void
CStdLogger::Log( const TLogMsgType logMsgType ,
                 const Int32 logLevel         ,
                 const CString& logMessage    ,
                 const UInt32 threadId        ,
                 const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( m_output != NULL )
    {
        if ( logLevel >= m_minimalLogLevel )
        {
            CString actualLogMsg( m_logFormatter->FormatLogMessage( logMsgType ,
                                                                    logLevel   ,
                                                                    logMessage ,
                                                                    threadId   ,
                                                                    timestamp  ) + "\n" );

            m_output->Write( actualLogMsg.C_String() ,
                             actualLogMsg.Length()   ,
                             1                       );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStdLogger::LogWithoutFormatting( const TLogMsgType logMsgType ,
                                  const Int32 logLevel         ,
                                  const CString& logMessage    ,
                                  const UInt32 threadId        ,
                                  const CTimestamp& timestamp  )
{GUCEF_TRACE;

    if ( m_output != NULL )
    {
        if ( logLevel >= m_minimalLogLevel )
        {
            CString actualLogMsg( logMessage.operator+( "\n" ) );
            m_output->Write( actualLogMsg.C_String() ,
                             actualLogMsg.Length()   ,
                             1                       );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStdLogger::FlushLog( void )
{GUCEF_TRACE;

    if ( m_output != NULL )
    {
        m_output->Flush();
    }
}

/*-------------------------------------------------------------------------*/

void
CStdLogger::SetMinimalLogLevel( const Int32 minimalLogLevel )
{GUCEF_TRACE;

    m_minimalLogLevel = minimalLogLevel;
}

/*-------------------------------------------------------------------------*/

Int32
CStdLogger::GetMinimalLogLevel( void ) const
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
