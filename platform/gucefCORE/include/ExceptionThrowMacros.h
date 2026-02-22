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

#ifndef GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#define GUCEF_CORE_EXCEPTIONTHROWMACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_STRINGLOGGINGMACROS_H
#include "gucefCORE_StringLoggingMacros.h"
#define GUCEF_CORE_STRINGLOGGINGMACROS_H
#endif /* GUCEF_CORE_STRINGLOGGINGMACROS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_LOG_EXCEPTIONS

/*--------------------------------------------------------------------------*/

#undef GUCEF_ETHROW
#define GUCEF_ETHROW( exceptionName )                                        \
{                                                                            \
    GUCEF_EXCEPTION_LOGSTR( ::GUCEF::CORE::LOGLEVEL_NORMAL,                  \
                            ::GUCEF::CORE::CString( "Throwing exception @ " ) + \
                            __FILE__                                          + \
                            ::GUCEF::CORE::Int32ToString( __LINE__ )         ); \
    throw exceptionName( __FILE__, __LINE__ );                               \
}

/*--------------------------------------------------------------------------*/

#undef GUCEF_EMSGTHROW
#define GUCEF_EMSGTHROW( exceptionName, errorMsg )                                       \
{                                                                                        \
    GUCEF_EXCEPTION_LOGSTR( ::GUCEF::CORE::LOGLEVEL_NORMAL,                              \
                            ::GUCEF::CORE::CString( "Throwing exception with message: " ) + \
                            ::GUCEF::CORE::CString( errorMsg )                            + \
                            ::GUCEF::CORE::CString( " @ " )                               + \
                            __FILE__                                                      + \
                            ::GUCEF::CORE::Int32ToString( __LINE__ )                     ); \
    throw exceptionName( __FILE__, __LINE__, (errorMsg) );                               \
}

/*--------------------------------------------------------------------------*/

#else /* GUCEF_LOG_EXCEPTIONS ? */

/*--------------------------------------------------------------------------*/

#undef GUCEF_ETHROW
#define GUCEF_ETHROW( exceptionName )  { throw exceptionName( __FILE__, __LINE__ ); }

/*--------------------------------------------------------------------------*/

#undef GUCEF_EMSGTHROW
#define GUCEF_EMSGTHROW( exceptionName, errorMsg ) { throw exceptionName( __FILE__, __LINE__, (errorMsg) ); }

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_LOG_EXCEPTIONS ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_EXCEPTIONTHROWMACROS_H ? */
