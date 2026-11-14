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

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "CException.h"

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

CException::CException( const char* m_originFile   ,
                        const Int32 m_originLineNr )
    : m_originFile( m_originFile )     ,
      m_originLineNr( m_originLineNr )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CException::CException( const CException& src )
    : m_originFile( src.m_originFile )     ,
      m_originLineNr( src.m_originLineNr )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CException::~CException() GUCEF_NOTHROW
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CException&
CException::operator=( const CException& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_originFile = src.m_originFile;
        m_originLineNr = src.m_originLineNr;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const char*
CException::GetOriginFile( void ) const
{GUCEF_TRACE;

    return m_originFile;
}

/*-------------------------------------------------------------------------*/

Int32
CException::GetOriginLineNr( void ) const
{GUCEF_TRACE;

    return m_originLineNr;
}

/*-------------------------------------------------------------------------*/

const char*
CException::what( void ) const GUCEF_NOTHROW
{GUCEF_TRACE;

    static CString whatStr;
    whatStr = "Basic GUCEF exception thrown from: " + CString( m_originFile ) + ':' + Int32ToString( m_originLineNr );
    return whatStr.C_String();
}

/*-------------------------------------------------------------------------*/

const char*
CException::What( void ) const
{GUCEF_TRACE;

    return what();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
