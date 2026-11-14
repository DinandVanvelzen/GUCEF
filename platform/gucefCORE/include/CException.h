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

#ifndef GUCEF_CORE_CEXCEPTION_H
#define GUCEF_CORE_CEXCEPTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <exception>            /**< STL base class for all exceptions */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

// Get rid of compiler warning for MVC compiler:
//      warning C4275: non dll-interface class 'std::exception' used as base for dll-interface class 'GUCEF::CORE::CException'
#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4275 )
#endif

/*-------------------------------------------------------------------------*/

/**
 *  Generic exception class.
 *  Can be used to throw an exception that does not provide any additional information
 *  aside from the origin of the exception.
 *  Off course you can always inherit from this class to create specific exception
 *  sub-sets which may or may not provide additional information.
 */
class GUCEF_CORE_PUBLIC_CPP CException : public std::exception
{
	public:

    CException( const char* originFile   ,
                const Int32 originLineNr );

    CException( const CException& src );

    virtual ~CException() GUCEF_NOTHROW;

    CException& operator=( const CException& src );

    const char* GetOriginFile( void ) const;

    Int32 GetOriginLineNr( void ) const;

    virtual const char* what( void ) const GUCEF_NOTHROW;

    const char* What( void ) const;

    private:
    CException( void );

    private:

    const char* m_originFile;
    Int32 m_originLineNr;
};

/*-------------------------------------------------------------------------*/

#ifdef _MSC_VER
    #pragma warning( pop )
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CEXCEPTION_H ? */
