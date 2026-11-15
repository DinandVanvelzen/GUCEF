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

#ifndef GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#define GUCEF_CORE_EXCEPTIONCLASSMACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CEXCEPTION_H
#include "CException.h"
#define GUCEF_CORE_CEXCEPTION_H
#endif /* GUCEF_CORE_CEXCEPTION_H ? */

#ifndef GUCEF_CORE_CMSGEXCEPTION_H
#include "CMsgException.h"
#define GUCEF_CORE_CMSGEXCEPTION_H
#endif /* GUCEF_CORE_CMSGEXCEPTION_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Macro for defining an exception class. Usefull for defining an exception
 *  class within the scope of your class definition. This makes exceptions
 *  easy to find instead of hidden away in comment remarks but without
 *  mucking up your class definition readability with exception definitions
 */
#define GUCEF_DEFINE_EXCEPTION( exportAPI, exceptionName )         \
                                                                   \
    class exportAPI exceptionName : public GUCEF::CORE::CException \
    {                                                              \
        public:                                                    \
        exceptionName( const char* originFile ,                    \
                       const int originLineNr );                   \
                                                                   \
        exceptionName( const exceptionName& src );                 \
                                                                   \
        virtual ~exceptionName() GUCEF_NOTHROW;                                \
                                                                   \
        private:                                                   \
        exceptionName& operator=( const exceptionName& src );      \
    };

/*--------------------------------------------------------------------------*/

/*
 *  Macro for defining a message exception class.
 *  Usefull for defining an exception class within the scope of
 *  your class definition. This makes exceptions
 *  easy to find instead of hidden away in comment remarks but without
 *  mucking up your class definition readability with exception definitions
 */
#define GUCEF_DEFINE_MSGEXCEPTION( exportAPI, exceptionName )         \
                                                                      \
    class exportAPI exceptionName : public GUCEF::CORE::CMsgException \
    {                                                                 \
        public:                                                       \
        exceptionName( const char* originFile ,                       \
                       const int originLineNr ,                       \
                       const char* errorMsg   );                      \
                                                                      \
        exceptionName( const char* originFile  ,                      \
                       const int originLineNr  ,                      \
                       const CString& errorMsg );                     \
                                                                      \
        exceptionName( const exceptionName& src );                    \
                                                                      \
        virtual ~exceptionName() GUCEF_NOTHROW;                                   \
                                                                      \
        private:                                                      \
        exceptionName( void );                                        \
        const exceptionName& operator=(const exceptionName&);         \
    };

/*--------------------------------------------------------------------------*/

/*
 *  Macro for defining a message exception class inlined
 *  This makes exceptions easy to find instead of hidden away in comment
 *  remarks but without mucking up your class definition readability
 *  with exception definitions
 */
#define GUCEF_DEFINE_INLINED_MSGEXCEPTION( exceptionName )               \
                                                                         \
    class exceptionName : public GUCEF::CORE::CMsgException              \
    {                                                                    \
        public:                                                          \
        exceptionName( const char* originFile ,                          \
                       const int originLineNr ,                          \
                       const char* errorMsg   )                          \
                                                                         \
            : GUCEF::CORE::CMsgException( originFile   ,                 \
                                          originLineNr ,                 \
                                          errorMsg     )                 \
        {                                                                \
                                                                         \
        }                                                                \
                                                                         \
        exceptionName( const char* originFile  ,                         \
                       const int originLineNr  ,                         \
                       const CString& errorMsg )                         \
                                                                         \
            : GUCEF::CORE::CMsgException( originFile   ,                 \
                                          originLineNr ,                 \
                                          errorMsg     )                 \
        {                                                                \
                                                                         \
        }                                                                \
                                                                         \
        exceptionName( const exceptionName& src )                        \
            : GUCEF::CORE::CMsgException( src )                          \
        {                                                                \
                                                                         \
        }                                                                \
                                                                         \
        virtual ~exceptionName() GUCEF_NOTHROW                           \
        {                                                                \
                                                                         \
        }                                                                \
                                                                         \
        private:                                                         \
                                                                         \
        exceptionName( void );                                           \
                                                                         \
        const exceptionName& operator=( const exceptionName& src );      \
    };

/*--------------------------------------------------------------------------*/

/*
 *  macro for implementing an exception class within the scope of
 *  your class. This allows you to use per class typed exceptions.
 *  The macro will help to implement a number of exceptions classes
 *  with ease and consistancy
 */
#define GUCEF_IMPLEMENT_EXCEPTION( ownerScopeClass, exceptionName )         \
                                                                            \
ownerScopeClass::exceptionName::exceptionName( const char* originFile ,     \
                                               const int originLineNr )     \
    : GUCEF::CORE::CException( originFile   ,                               \
                               originLineNr )                               \
{                                                                           \
}                                                                           \
                                                                            \
ownerScopeClass::exceptionName::exceptionName( const exceptionName& src )   \
    : GUCEF::CORE::CException( src )                                        \
{                                                                           \
}                                                                           \
                                                                            \
ownerScopeClass::exceptionName::~exceptionName() GUCEF_NOTHROW             \
{                                                                           \
}

/*--------------------------------------------------------------------------*/

/*
 *  macro for implementing an exception class within the scope of
 *  your class. This allows you to use per class typed exceptions.
 *  The macro will help to implement a number of exceptions classes
 *  with ease and consistancy
 */
#define GUCEF_IMPLEMENT_MSGEXCEPTION( ownerScopeClass, exceptionName )      \
                                                                            \
ownerScopeClass::exceptionName::exceptionName( const char* originFile ,     \
                                               const int originLineNr ,     \
                                               const char* errorMsg   )     \
    : GUCEF::CORE::CMsgException( originFile   ,                            \
                                  originLineNr ,                            \
                                  errorMsg     )                            \
{                                                                           \
}                                                                           \
                                                                            \
ownerScopeClass::exceptionName::exceptionName( const char* originFile  ,    \
                                               const int originLineNr  ,    \
                                               const CString& errorMsg )    \
    : GUCEF::CORE::CMsgException( originFile   ,                            \
                                  originLineNr ,                            \
                                  errorMsg     )                            \
{                                                                           \
}                                                                           \
                                                                            \
ownerScopeClass::exceptionName::exceptionName( const exceptionName& src )   \
    : GUCEF::CORE::CMsgException( src )                                     \
{                                                                           \
}                                                                           \
                                                                            \
ownerScopeClass::exceptionName::~exceptionName() GUCEF_NOTHROW             \
{                                                                           \
}

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_EXCEPTIONCLASSMACROS_H ? */
