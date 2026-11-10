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

#include <stdlib.h>
#include <stdio.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefGUI_CUDim.h"

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

CUDim::CUDim( void )
    : m_relativeValue( 0.0 ) ,
      m_pixelValue( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUDim::CUDim( const Float32 relativeValue ,
              const Int32 pixelValue      )
    : m_relativeValue( relativeValue ) ,
      m_pixelValue( pixelValue )
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

CUDim::CUDim( const CUDim& src )
    : m_relativeValue( src.m_relativeValue ) ,
      m_pixelValue( src.m_pixelValue )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUDim::~CUDim()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUDim&
CUDim::operator=( const CUDim& src )
{GUCEF_TRACE;
    
    if ( this != &src )
    {
        m_relativeValue = src.m_relativeValue;
        m_pixelValue = src.m_pixelValue;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CUDim::operator==( const CUDim& other ) const
{GUCEF_TRACE;

    return m_relativeValue == other.m_relativeValue &&
           m_pixelValue == other.m_pixelValue;
}

/*-------------------------------------------------------------------------*/

bool
CUDim::operator!=( const CUDim& other ) const
{GUCEF_TRACE;

    return m_relativeValue != other.m_relativeValue ||
           m_pixelValue != other.m_pixelValue;
}

/*-------------------------------------------------------------------------*/

void
CUDim::SetRelativeValue( const Float32 relativeValue ) 
{GUCEF_TRACE;

    m_relativeValue = relativeValue;
}

/*-------------------------------------------------------------------------*/

Float32
CUDim::GetRelativeValue( void ) const
{GUCEF_TRACE;

    return m_relativeValue;
}

/*-------------------------------------------------------------------------*/
    
void
CUDim::SetPixelValue( const Int32 pixelValue )
{GUCEF_TRACE;

    m_pixelValue = pixelValue;
}

/*-------------------------------------------------------------------------*/
    
Int32
CUDim::GetPixelValue( void ) const
{GUCEF_TRACE;

    return m_pixelValue;
}

/*-------------------------------------------------------------------------*/

Int32
CUDim::Resolve( const Int32 parentSize ) const
{GUCEF_TRACE;

    Float32 value = m_relativeValue * (Float32) parentSize + (Float32) m_pixelValue;
    return (Int32) ( value < 0.0f ? value - 0.5f : value + 0.5f );
}

/*-------------------------------------------------------------------------*/

CString
CUDim::ToString( void ) const
{GUCEF_TRACE;

    // as short as it can be: 0.5 and not 0.500000
    char relative[ 32 ];
    snprintf( relative, sizeof( relative ), "%g", (double) m_relativeValue );
    return CString( relative ) + ',' + CORE::Int32ToString( m_pixelValue );
}

/*-------------------------------------------------------------------------*/

bool
CUDim::FromString( const CString& str, CUDim& dim )
{GUCEF_TRACE;

    CString::StringVector parts = str.ParseElements( ',', true );
    if ( 1 == parts.size() )
    {
        CString pixels = parts[ 0 ].Trim( true ).Trim( false );
        if ( pixels.IsNULLOrEmpty() )
            return false;
        char* end = GUCEF_NULL;
        long value = strtol( pixels.C_String(), &end, 10 );
        if ( '\0' != *end )
            return false;
        dim = CUDim( 0.0f, (Int32) value );
        return true;
    }
    if ( 2 != parts.size() )
        return false;

    CString relative = parts[ 0 ].Trim( true ).Trim( false );
    CString pixels = parts[ 1 ].Trim( true ).Trim( false );
    if ( relative.IsNULLOrEmpty() || pixels.IsNULLOrEmpty() )
        return false;
    char* end = GUCEF_NULL;
    double rel = strtod( relative.C_String(), &end );
    if ( '\0' != *end )
        return false;
    long pix = strtol( pixels.C_String(), &end, 10 );
    if ( '\0' != *end )
        return false;
    dim = CUDim( (Float32) rel, (Int32) pix );
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
