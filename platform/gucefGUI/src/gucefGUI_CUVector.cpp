/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#include "gucefGUI_CUVector.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CUVector::CUVector( void )
    : m_x() ,
      m_y()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUVector::CUVector( const CUDim& x ,
                    const CUDim& y )
    : m_x( x ) ,
      m_y( y )
{GUCEF_TRACE;

    
}

/*-------------------------------------------------------------------------*/

CUVector::CUVector( const CUVector& src )
    : m_x( src.m_x ) ,
      m_y( src.m_y )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUVector::~CUVector()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUVector&
CUVector::operator=( const CUVector& src )
{GUCEF_TRACE;
    
    if ( this != &src )
    {
        m_x = src.m_x;
        m_y = src.m_y;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CUVector::operator==( const CUVector& other ) const
{GUCEF_TRACE;

    return m_x == other.m_x &&
           m_y == other.m_y;
}

/*-------------------------------------------------------------------------*/

bool
CUVector::operator!=( const CUVector& other ) const
{GUCEF_TRACE;

    return m_x != other.m_x ||
           m_y != other.m_y;
}

/*-------------------------------------------------------------------------*/

void
CUVector::SetX( const CUDim& newX )
{GUCEF_TRACE;

    m_x = newX;
}
    
/*-------------------------------------------------------------------------*/
    
const CUDim&
CUVector::GetX( void ) const
{GUCEF_TRACE;

    return m_x;
}

/*-------------------------------------------------------------------------*/
    
void
CUVector::SetY( const CUDim& newY )
{GUCEF_TRACE;

    m_y = newY;
}

/*-------------------------------------------------------------------------*/
    
const CUDim&
CUVector::GetY( void ) const
{GUCEF_TRACE;

    return m_y;
}

/*-------------------------------------------------------------------------*/

CString
CUVector::ToString( void ) const
{GUCEF_TRACE;

    return m_x.ToString() + ' ' + m_y.ToString();
}

/*-------------------------------------------------------------------------*/

bool
CUVector::FromString( const CString& str, CUVector& vec )
{GUCEF_TRACE;

    CString::StringVector parts = str.Trim( true ).Trim( false ).ParseElements( ' ', false );
    if ( 2 != parts.size() )
        return false;
    CUDim x, y;
    if ( !CUDim::FromString( parts[ 0 ], x ) || !CUDim::FromString( parts[ 1 ], y ) )
        return false;
    vec = CUVector( x, y );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
