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

#ifndef GUCEF_CORE_CMSGEXCEPTION_H
#define GUCEF_CORE_CMSGEXCEPTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CEXCEPTION_H
#include "CException.h"
#define GUCEF_CORE_CEXCEPTION_H
#endif /* GUCEF_CORE_CEXCEPTION_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

/**
 *  Generic exception class.
 *  Can be used to throw an exception that does not provide any additional information
 *  aside from the origin of the exception.
 *  Off course you can always inherit from this class to create specific exception
 *  sub-sets which may or may not provide additional information.
 */
class GUCEF_CORE_PUBLIC_CPP CMsgException : public CException
{
	public:

    CMsgException( const char* originFile   ,
                   const Int32 originLineNr ,
                   const char* errorMsg     );

    CMsgException( const char* originFile   ,
                   const Int32 originLineNr ,
                   const CString& errorMsg  );

    CMsgException( const CMsgException& src );

    virtual ~CMsgException() GUCEF_NOTHROW;

    CMsgException& operator=( const CMsgException& src );

    const char* GetErrorMsg( void ) const;

    virtual const char* what( void ) const GUCEF_NOTHROW;

    private:

    CMsgException( void );

    private:

    const char* m_errorMsgC;
    CString m_errorMsg;
    Int32 m_errorCode;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CMSGEXCEPTION_H ? */
