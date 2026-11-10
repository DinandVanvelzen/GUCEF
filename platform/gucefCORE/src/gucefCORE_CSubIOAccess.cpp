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

#include "gucefCORE_CSubIOAccess.h"      /* definition of the class implemented here */

#ifndef GUCEF_CORE_UCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */ 

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

CSubIOAccess::CSubIOAccess( void )
    : CIOAccess()    
    , m_parent()
    , m_offset( 0 )
    , m_size( 0 )
    , m_pos( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSubIOAccess::CSubIOAccess( IOAccessPtr parent  ,
                            const UInt64 offset ,
                            const UInt64 size   )
    : CIOAccess()    
    , m_parent()
    , m_offset( 0 )
    , m_size( 0 )
    , m_pos( 0 )
{GUCEF_TRACE;

    Load( parent, offset, size );
}

/*-------------------------------------------------------------------------*/

CSubIOAccess::~CSubIOAccess()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CSubIOAccess::Load( IOAccessPtr parent  ,
                    const UInt64 offset ,
                    const UInt64 size   )
{GUCEF_TRACE;

    if ( parent.IsNULL() )
        return false;

    // Guard against windows that run off the end of the parent, taking care to not overflow
    const UInt64 parentSize = parent->GetSize();
    if ( parentSize > 0 && ( offset > parentSize || size > parentSize - offset ) )
        return false;

    m_parent = parent;
    m_offset = offset;
    m_size = size;
    m_pos = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CSubIOAccess::GetOffset( void ) const
{GUCEF_TRACE;

    return m_offset;
}

/*-------------------------------------------------------------------------*/

CIOAccess::IOAccessPtr
CSubIOAccess::GetParent( void ) const
{GUCEF_TRACE;

    return m_parent;
}

/*-------------------------------------------------------------------------*/

void
CSubIOAccess::Open( void )
{GUCEF_TRACE;

    m_pos = 0;
}

/*-------------------------------------------------------------------------*/

void
CSubIOAccess::Close( void )
{GUCEF_TRACE;

    m_parent.Unlink();
    m_pos = 0;
}

/*-------------------------------------------------------------------------*/

bool
CSubIOAccess::Opened( void ) const
{GUCEF_TRACE;

    return !m_parent.IsNULL();
}

/*-------------------------------------------------------------------------*/

UInt32
CSubIOAccess::Read( void *dest      ,
                    UInt32 esize    ,
                    UInt32 elements )
{GUCEF_TRACE;

    if ( m_parent.IsNULL() || 0 == esize || 0 == elements )
        return 0;

    // Never read past the end of the window, partial elements are cut off. As with a file the position can be
    // beyond the end, there is nothing to read there
    if ( m_pos >= m_size )
        return 0;
    const UInt64 remaining = m_size - m_pos;
    const UInt64 elementsLeft = remaining / esize;
    if ( elementsLeft < elements )
        elements = (UInt32) elementsLeft;
    if ( 0 == elements )
        return 0;

    // The parent may be shared with other windows so it is always repositioned first
    if ( 0 != m_parent->Setpos( m_offset + m_pos ) )
        return 0;

    const UInt32 elementsRead = m_parent->Read( dest, esize, elements );
    m_pos += (UInt64) elementsRead * esize;
    return elementsRead;
}

/*-------------------------------------------------------------------------*/

UInt32
CSubIOAccess::Write( const void* srcdata ,
                     UInt32 esize        ,
                     UInt32 elements     )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CSubIOAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_pos;
}

/*-------------------------------------------------------------------------*/

UInt64
CSubIOAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

Int32
CSubIOAccess::Seek( Int64 offset ,
                    Int32 origin )
{GUCEF_TRACE;

    Int64 target = 0;
    switch ( origin )
    {
        case SEEK_SET: { target = offset; break; }
        case SEEK_CUR: { target = (Int64) m_pos + offset; break; }
        case SEEK_END: { target = (Int64) m_size + offset; break; }
        default: { return 1; }
    }

    // as fseek(): before the start fails, beyond the end is allowed (reads there return nothing)
    if ( target < 0 )
        return 1;

    m_pos = (UInt64) target;
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CSubIOAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    // as fsetpos(): any position, also beyond the end of the window where reads return nothing
    m_pos = position;
    return 0;
}

/*-------------------------------------------------------------------------*/

char
CSubIOAccess::GetChar( void )
{GUCEF_TRACE;

    char c = 0;
    if ( 1 == Read( &c, 1, 1 ) )
        return c;
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CSubIOAccess::Eof( void ) const
{GUCEF_TRACE;

    return m_pos >= m_size;
}

/*-------------------------------------------------------------------------*/

bool
CSubIOAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSubIOAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSubIOAccess::IsValid( void )
{GUCEF_TRACE;

    return !m_parent.IsNULL() && m_parent->IsValid();
}

/*-------------------------------------------------------------------------*/

void
CSubIOAccess::Flush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CICloneable*
CSubIOAccess::Clone( void ) const
{GUCEF_TRACE;

    if ( m_parent.IsNULL() )
        return GUCEF_NULL;

    return GUCEF_NEW CSubIOAccess( m_parent, m_offset, m_size );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
