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

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_VIEW_H
#include "gucefCORE_CDynamicBufferView.h"
#define GUCEF_CORE_CDYNAMICBUFFER_VIEW_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_VIEW_H ? */

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

CDynamicBufferView::CDynamicBufferView( void )
    : m_buffer()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView::CDynamicBufferView( const void* externalBuffer ,
                                         UInt32 bufferSize          )
    : m_buffer()
{GUCEF_TRACE;

    m_buffer.LinkTo( externalBuffer, bufferSize );
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView::CDynamicBufferView( const CDynamicBuffer& src )
    : m_buffer()
{GUCEF_TRACE;

    m_buffer.LinkTo( src );
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView::CDynamicBufferView( const CDynamicBufferView& src )
    : m_buffer()
{GUCEF_TRACE;

    m_buffer.LinkTo( src.m_buffer );
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView&
CDynamicBufferView::operator=( const CDynamicBufferView& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_buffer.LinkTo( src.m_buffer );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView::~CDynamicBufferView()
{GUCEF_TRACE;

    /* m_buffer destructor will skip deallocation because it is linked */
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView&
CDynamicBufferView::LinkTo( const void* externalBuffer ,
                             UInt32 bufferSize          )
{GUCEF_TRACE;

    m_buffer.LinkTo( externalBuffer, bufferSize );
    return *this;
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView&
CDynamicBufferView::LinkTo( const CDynamicBuffer& src )
{GUCEF_TRACE;

    m_buffer.LinkTo( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView&
CDynamicBufferView::LinkTo( const CVariant& src )
{GUCEF_TRACE;

    m_buffer.LinkTo( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView&
CDynamicBufferView::LinkTo( const CAsciiString& src, bool includeNullTerm )
{GUCEF_TRACE;

    m_buffer.LinkTo( src, includeNullTerm );
    return *this;
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView&
CDynamicBufferView::LinkTo( const CUtf8String& src, bool includeNullTerm )
{GUCEF_TRACE;

    m_buffer.LinkTo( src, includeNullTerm );
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBufferView::GetBufferSize( void ) const
{GUCEF_TRACE;

    return m_buffer.GetBufferSize();
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBufferView::GetDataSize( void ) const
{GUCEF_TRACE;

    return m_buffer.GetDataSize();
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBufferView::GetRemainingDataSize( UInt32 offset ) const
{GUCEF_TRACE;

    return m_buffer.GetRemainingDataSize( offset );
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBufferView::GetUnusedBufferSize( void ) const
{GUCEF_TRACE;

    return m_buffer.GetUnusedBufferSize();
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBufferView::GetRemainingBufferSize( UInt32 offset ) const
{GUCEF_TRACE;

    return m_buffer.GetRemainingBufferSize( offset );
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBufferView::IsLinked( void ) const
{GUCEF_TRACE;

    return m_buffer.IsLinked();
}

/*-------------------------------------------------------------------------*/

const void*
CDynamicBufferView::GetConstBufferPtr( UInt32 offset ) const
{GUCEF_TRACE;

    return m_buffer.GetConstBufferPtr( offset );
}

/*-------------------------------------------------------------------------*/

char
CDynamicBufferView::operator[]( UInt32 index ) const
{GUCEF_TRACE;

    return m_buffer[ index ];
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBufferView::operator==( const CDynamicBuffer& other ) const
{GUCEF_TRACE;

    return m_buffer == other;
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBufferView::operator!=( const CDynamicBuffer& other ) const
{GUCEF_TRACE;

    return m_buffer != other;
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBufferView::operator<( const CDynamicBuffer& other ) const
{GUCEF_TRACE;

    return m_buffer < other;
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBufferView::operator>( const CDynamicBuffer& other ) const
{GUCEF_TRACE;

    return m_buffer > other;
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBufferView::CopyTo( UInt32 offset ,
                             UInt32 size   ,
                             void* dest    ) const
{GUCEF_TRACE;

    return m_buffer.CopyTo( offset, size, dest );
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBufferView::CopyTo( void* dest ) const
{GUCEF_TRACE;

    return m_buffer.CopyTo( dest );
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBufferView::CopyTo( CDynamicBuffer& dest ) const
{GUCEF_TRACE;

    return m_buffer.CopyTo( dest );
}

/*-------------------------------------------------------------------------*/

Int32
CDynamicBufferView::Find( const void* data  ,
                           UInt32 size       ,
                           UInt32 offset     ) const
{GUCEF_TRACE;

    return m_buffer.Find( data, size, offset );
}

/*-------------------------------------------------------------------------*/

Int32
CDynamicBufferView::FindPerElement( const void* searchData                ,
                                     UInt32 searchDataSizeInElements       ,
                                     UInt32 elementSize                    ,
                                     UInt32 offsetInBytes                  ) const
{GUCEF_TRACE;

    return m_buffer.FindPerElement( searchData, searchDataSizeInElements, elementSize, offsetInBytes );
}

/*-------------------------------------------------------------------------*/

CUtf8String::StringVector
CDynamicBufferView::ParseUtf8StringElements( UInt32 seperatorCodePoint ,
                                              bool addEmptyElements     ) const
{GUCEF_TRACE;

    return m_buffer.ParseUtf8StringElements( seperatorCodePoint, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf8String::StringSet
CDynamicBufferView::ParseUniqueUtf8StringElements( UInt32 seperatorCodePoint ,
                                                    bool addEmptyElements     ) const
{GUCEF_TRACE;

    return m_buffer.ParseUniqueUtf8StringElements( seperatorCodePoint, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CVariant
CDynamicBufferView::AsVariant( UInt32 bufferOffset, UInt8 varType ) const
{GUCEF_TRACE;

    return m_buffer.AsVariant( bufferOffset, varType );
}

/*-------------------------------------------------------------------------*/

CDynamicBufferView::operator const CDynamicBuffer&( void ) const
{GUCEF_TRACE;

    return m_buffer;
}

/*-------------------------------------------------------------------------*/

const CDynamicBuffer&
CDynamicBufferView::AsBuffer( void ) const
{GUCEF_TRACE;

    return m_buffer;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
