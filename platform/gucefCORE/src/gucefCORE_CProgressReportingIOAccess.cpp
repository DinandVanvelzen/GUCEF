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

#include "gucefCORE_CProgressReportingIOAccess.h"

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

CIIOProgressObserver::CIIOProgressObserver( void )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CIIOProgressObserver::~CIIOProgressObserver()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CProgressReportingIOAccess::CProgressReportingIOAccess( CIOAccess& wrappedAccess        ,
                                                        CIIOProgressObserver* observer  ,
                                                        UInt32 reportIntervalInBytes    )
    : CIOAccess()
    , m_wrappedAccess( &wrappedAccess )
    , m_observer( observer )
    , m_reportIntervalInBytes( reportIntervalInBytes )
    , m_totalBytesRead( 0 )
    , m_totalBytesWritten( 0 )
    , m_bytesSinceLastReport( 0 )
    , m_aborted( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CProgressReportingIOAccess::~CProgressReportingIOAccess()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

UInt64
CProgressReportingIOAccess::GetTotalBytesRead( void ) const
{GUCEF_TRACE;

    return m_totalBytesRead;
}

/*-------------------------------------------------------------------------*/

UInt64
CProgressReportingIOAccess::GetTotalBytesWritten( void ) const
{GUCEF_TRACE;

    return m_totalBytesWritten;
}

/*-------------------------------------------------------------------------*/

void
CProgressReportingIOAccess::SetAborted( bool aborted )
{GUCEF_TRACE;

    m_aborted = aborted;
}

/*-------------------------------------------------------------------------*/

bool
CProgressReportingIOAccess::IsAborted( void ) const
{GUCEF_TRACE;

    return m_aborted;
}

/*-------------------------------------------------------------------------*/

void
CProgressReportingIOAccess::Report( UInt64 bytesJustTransferred )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_observer )
        return;

    m_bytesSinceLastReport += bytesJustTransferred;
    if ( m_bytesSinceLastReport >= m_reportIntervalInBytes )
    {
        m_bytesSinceLastReport = 0;
        if ( !m_observer->OnIOProgress( m_totalBytesRead, m_totalBytesWritten ) )
            m_aborted = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CProgressReportingIOAccess::Open( void )
{GUCEF_TRACE;

    m_wrappedAccess->Open();
}

/*-------------------------------------------------------------------------*/

void
CProgressReportingIOAccess::Close( void )
{GUCEF_TRACE;

    m_wrappedAccess->Close();
}

/*-------------------------------------------------------------------------*/

bool
CProgressReportingIOAccess::Opened( void ) const
{GUCEF_TRACE;

    return m_wrappedAccess->Opened();
}

/*-------------------------------------------------------------------------*/

CString
CProgressReportingIOAccess::ReadLine( void )
{GUCEF_TRACE;

    if ( m_aborted )
        return CString();

    CString line = m_wrappedAccess->ReadLine();
    m_totalBytesRead += line.Length();
    Report( line.Length() );
    return line;
}

/*-------------------------------------------------------------------------*/

CString
CProgressReportingIOAccess::ReadString( void )
{GUCEF_TRACE;

    if ( m_aborted )
        return CString();

    CString str = m_wrappedAccess->ReadString();
    m_totalBytesRead += str.Length();
    Report( str.Length() );
    return str;
}

/*-------------------------------------------------------------------------*/

UInt32
CProgressReportingIOAccess::Read( void *dest      ,
                                  UInt32 esize    ,
                                  UInt32 elements )
{GUCEF_TRACE;

    if ( m_aborted )
        return 0;

    UInt32 elementsRead = m_wrappedAccess->Read( dest, esize, elements );
    UInt64 bytesRead = (UInt64) elementsRead * esize;
    m_totalBytesRead += bytesRead;
    Report( bytesRead );
    return elementsRead;
}

/*-------------------------------------------------------------------------*/

UInt32
CProgressReportingIOAccess::Write( const void* srcdata ,
                                   UInt32 esize        ,
                                   UInt32 elements     )
{GUCEF_TRACE;

    if ( m_aborted )
        return 0;

    UInt32 elementsWritten = m_wrappedAccess->Write( srcdata, esize, elements );
    UInt64 bytesWritten = (UInt64) elementsWritten * esize;
    m_totalBytesWritten += bytesWritten;
    Report( bytesWritten );
    return elementsWritten;
}

/*-------------------------------------------------------------------------*/

UInt64
CProgressReportingIOAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_wrappedAccess->Tell();
}

/*-------------------------------------------------------------------------*/

UInt64
CProgressReportingIOAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_wrappedAccess->GetSize();
}

/*-------------------------------------------------------------------------*/

Int32
CProgressReportingIOAccess::Seek( Int64 offset ,
                                  Int32 origin )
{GUCEF_TRACE;

    return m_wrappedAccess->Seek( offset, origin );
}

/*-------------------------------------------------------------------------*/

UInt32
CProgressReportingIOAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    return m_wrappedAccess->Setpos( position );
}

/*-------------------------------------------------------------------------*/

char
CProgressReportingIOAccess::GetChar( void )
{GUCEF_TRACE;

    if ( m_aborted )
        return '\0';

    char character = m_wrappedAccess->GetChar();
    ++m_totalBytesRead;
    Report( 1 );
    return character;
}

/*-------------------------------------------------------------------------*/

bool
CProgressReportingIOAccess::Eof( void ) const
{GUCEF_TRACE;

    return m_aborted || m_wrappedAccess->Eof();
}

/*-------------------------------------------------------------------------*/

bool
CProgressReportingIOAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return !m_aborted && m_wrappedAccess->IsReadable();
}

/*-------------------------------------------------------------------------*/

bool
CProgressReportingIOAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return !m_aborted && m_wrappedAccess->IsWriteable();
}

/*-------------------------------------------------------------------------*/

bool
CProgressReportingIOAccess::IsValid( void )
{GUCEF_TRACE;

    return m_wrappedAccess->IsValid();
}

/*-------------------------------------------------------------------------*/

void
CProgressReportingIOAccess::Flush( void )
{GUCEF_TRACE;

    m_wrappedAccess->Flush();
}

/*-------------------------------------------------------------------------*/

CICloneable*
CProgressReportingIOAccess::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CProgressReportingIOAccess( *m_wrappedAccess, m_observer, m_reportIntervalInBytes );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
