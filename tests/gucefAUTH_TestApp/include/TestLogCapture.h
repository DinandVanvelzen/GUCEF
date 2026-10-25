/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_AUTH_TESTAPP_TESTLOGCAPTURE_H
#define GUCEF_AUTH_TESTAPP_TESTLOGCAPTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Collects everything that is logged while it exists, to prove what does and does not end up in a log */
class CLogCapture
{
    public:

    CLogCapture( void )
        : m_buffer()
        , m_access( m_buffer )
        , m_logger( m_access )
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &m_logger );
    }

    ~CLogCapture()
    {
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &m_logger );
    }

    /** Whether the text was logged */
    bool Contains( const GUCEF::CORE::CString& text )
    {
        GUCEF::CORE::CLogStreamScope::FlushLogs();
        GUCEF::CORE::CString all = Text();
        return all.HasSubstr( text ) >= 0;
    }

    GUCEF::CORE::CString Text( void )
    {
        GUCEF::CORE::CLogStreamScope::FlushLogs();
        GUCEF::CORE::CString all;
        if ( m_buffer.GetDataSize() > 0 )
            all = GUCEF::CORE::CString( m_buffer.AsConstTypePtr< char >(), m_buffer.GetDataSize() );
        return all;
    }

    private:

    CLogCapture( const CLogCapture& src );
    CLogCapture& operator=( const CLogCapture& src );

    private:

    GUCEF::CORE::CDynamicBuffer m_buffer;
    GUCEF::CORE::CDynamicBufferAccess m_access;
    GUCEF::CORE::CStdLogger m_logger;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTLOGCAPTURE_H ? */
