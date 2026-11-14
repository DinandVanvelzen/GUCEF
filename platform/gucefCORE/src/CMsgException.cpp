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

#include "CMsgException.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CMsgException::CMsgException( const char* originFile   ,
                              const Int32 originLineNr ,
                              const char* errorMsg     )
    : CException( originFile, originLineNr )             
    , m_errorMsgC( errorMsg ) // avoid memory allocs if we can here
    , m_errorMsg()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMsgException::CMsgException( const char* originFile   ,
                              const Int32 originLineNr ,
                              const CString& errorMsg  )
    : CException( originFile, originLineNr )             
    , m_errorMsgC( GUCEF_NULL )
    , m_errorMsg( errorMsg )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMsgException::CMsgException( const CMsgException& src )
    : CException( src )              
    , m_errorMsgC( src.m_errorMsgC )
    , m_errorMsg( src.m_errorMsg )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMsgException::~CMsgException() GUCEF_NOTHROW
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMsgException&
CMsgException::operator=( const CMsgException& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_errorMsg = src.m_errorMsg;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const char*
CMsgException::GetErrorMsg( void ) const
{GUCEF_TRACE;

    if ( m_errorMsg.IsNULLOrEmpty() )
        return m_errorMsgC;
    return m_errorMsg.C_String();
}

/*-------------------------------------------------------------------------*/

const char*
CMsgException::what( void ) const GUCEF_NOTHROW
{GUCEF_TRACE;

    static CString whatStr;       // TODO: thread safety?
    whatStr = "GUCEF message exception: " + CString( m_errorMsg ) + " - thrown from: " + CString( GetOriginFile() ) + ':' + Int32ToString( GetOriginLineNr() );
    return whatStr.C_String();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

