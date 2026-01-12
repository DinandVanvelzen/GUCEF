/*
 *  guidriverRmlUI: GUI backend using RmlUI
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverRmlUI_CRmlUIStreamAdapter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIStreamAdapter::CRmlUIStreamAdapter( CORE::CIOAccess& access )
    : RmlUI::Core::Stream() ,
      m_access( &access )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRmlUIStreamAdapter::~CRmlUIStreamAdapter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRmlUIStreamAdapter::Close()
{GUCEF_TRACE;

    m_access->Close();
}

/*-------------------------------------------------------------------------*/

int
CRmlUIStreamAdapter::GetStreamMode() const
{GUCEF_TRACE;

    int mode = 0;
    if ( m_access->IsWriteable() ) mode &= MODE_WRITE;
    if ( m_access->IsReadable() ) mode &= MODE_READ;
    return mode;
}

/*-------------------------------------------------------------------------*/

const RmlUI::Core::URL&
CRmlUIStreamAdapter::GetSourceURL() const
{GUCEF_TRACE;

    static RmlUI::Core::URL dummy;
    return dummy;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIStreamAdapter::IsEOS() const
{GUCEF_TRACE;

    return m_access->Eof();
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::Length() const
{GUCEF_TRACE;

    return m_access->GetSize();
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::Tell() const
{GUCEF_TRACE;
    
    return m_access->Tell();
}

/*-------------------------------------------------------------------------*/
	
bool
CRmlUIStreamAdapter::Seek( long offset, int origin ) const
{GUCEF_TRACE;

    return 0 == m_access->Seek( offset, origin );
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::Read( void* buffer, size_t bytes ) const
{GUCEF_TRACE;

    return m_access->Read( buffer, 1, bytes );
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::Read( RmlUI::Core::Stream* stream, size_t bytes ) const
{GUCEF_TRACE;

    return stream->Write( this, bytes );
}

/*-------------------------------------------------------------------------*/
	
size_t
CRmlUIStreamAdapter::Read( RmlUI::Core::String& buffer, size_t bytes ) const
{GUCEF_TRACE;

    GUI::CString strBuffer;
    char* strBufferPtr = strBuffer.Reserve( bytes+1 );
    size_t bytesRead = m_access->Read( strBufferPtr, 1, bytes );
    strBufferPtr[ bytes ] = 0;
    buffer += strBufferPtr;
    return bytesRead;
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::Peek( void* buffer, size_t bytes ) const
{GUCEF_TRACE;

    GUI::Int32 bytesRead = (GUI::Int32) m_access->Read( buffer, 1, bytes );
    m_access->Seek( -bytesRead,  SEEK_CUR );
    return bytesRead; 
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::Write( const void* buffer, size_t bytes )
{GUCEF_TRACE;

    return m_access->Write( buffer, 1, bytes );
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::Write( const RmlUI::Core::Stream* stream, size_t bytes )
{GUCEF_TRACE;
    
    return stream->Read( this, bytes );
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::Write( const char* string )
{GUCEF_TRACE;

    return m_access->Write( GUI::CString( string ) ); 
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::Write( const RmlUI::Core::String& string )
{GUCEF_TRACE;

    return m_access->Write( GUI::CString( string.CString(), string.Length() ) );
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::Truncate( size_t bytes )
{GUCEF_TRACE;
    
    return 0;
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::PushFront( const void* buffer, size_t bytes )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::PushBack( const void* buffer, size_t bytes )
{GUCEF_TRACE;

    m_access->Seek( 0, SEEK_END );
    return m_access->Write( buffer, 1, bytes );
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::PopFront( size_t bytes )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

size_t
CRmlUIStreamAdapter::PopBack( size_t bytes )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIStreamAdapter::IsReadReady()
{GUCEF_TRACE;

    return m_access->IsReadable();
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIStreamAdapter::IsWriteReady()
{GUCEF_TRACE;

    return m_access->IsWriteable();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIStreamAdapter::SetStreamDetails( const RmlUI::Core::URL& url, int stream_mode )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRmlUIStreamAdapter::OnReferenceDeactivate()
{GUCEF_TRACE;

}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

