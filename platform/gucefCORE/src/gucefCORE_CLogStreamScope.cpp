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

#ifndef GUCEF_CORE_CLOGSTREAMSCOPE_H
#include "gucefCORE_CLogStreamScope.h"
#define GUCEF_CORE_CLOGSTREAMSCOPE_H
#endif /* GUCEF_CORE_CLOGSTREAMSCOPE_H ? */

#ifndef GUCEF_CORE_CLOGGINGGLOBAL_H
#include "gucefCORE_CLoggingGlobal.h"
#define GUCEF_CORE_CLOGGINGGLOBAL_H
#endif /* GUCEF_CORE_CLOGGINGGLOBAL_H ? */

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

CLogStreamScope::CLogStreamScope( const TLogMsgType logMsgType ,
                                  const Int32 logLevel         )
    : m_stream()
{GUCEF_TRACE;

    CLoggingGlobal* loggingGlobal = CLoggingGlobal::Instance();
    if ( GUCEF_NULL != loggingGlobal )
    {
        m_stream = loggingGlobal->Log( logMsgType, logLevel );
    }
}

/*-------------------------------------------------------------------------*/

CLogStreamScope::~CLogStreamScope()
{GUCEF_TRACE;

    if ( !m_stream.IsNULL() )
    {
        m_stream->WriteSegmentEnd();
    }
}

/*-------------------------------------------------------------------------*/

CLogStream*
CLogStreamScope::operator->( void )
{GUCEF_TRACE;

    return m_stream.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

CLogStream&
CLogStreamScope::operator*( void )
{GUCEF_TRACE;

    return *m_stream;
}

/*-------------------------------------------------------------------------*/

CLogStream&
CLogStreamScope::GetStream( void )
{GUCEF_TRACE;

    return *m_stream;
}

/*-------------------------------------------------------------------------*/

void
CLogStreamScope::FlushLogs( void )
{GUCEF_TRACE;

    CLoggingGlobal* loggingGlobal = CLoggingGlobal::Instance();
    if ( GUCEF_NULL != loggingGlobal )
    {
        loggingGlobal->FlushLogs();
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
