/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CRpcPrincipal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcPrincipal::CRpcPrincipal( void )
    : transport()
    , subject()
    , isAuthenticated( false )
    , roles()
    , attributes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcPrincipal::CRpcPrincipal( const CORE::CString& transportType, const CORE::CString& subjectName, bool authenticated )
    : transport( transportType )
    , subject( subjectName )
    , isAuthenticated( authenticated )
    , roles()
    , attributes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcPrincipal::CRpcPrincipal( const CRpcPrincipal& src )
    : transport( src.transport )
    , subject( src.subject )
    , isAuthenticated( src.isAuthenticated )
    , roles( src.roles )
    , attributes( src.attributes )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcPrincipal::~CRpcPrincipal()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcPrincipal&
CRpcPrincipal::operator=( const CRpcPrincipal& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        transport = src.transport;
        subject = src.subject;
        isAuthenticated = src.isAuthenticated;
        roles = src.roles;
        attributes = src.attributes;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CRpcPrincipal::HasRole( const CORE::CString& role ) const
{GUCEF_TRACE;

    return roles.find( role ) != roles.end();
}

/*-------------------------------------------------------------------------*/

bool
CRpcPrincipal::MayResumeAs( const CRpcPrincipal& other ) const
{GUCEF_TRACE;

    if ( !isAuthenticated )
        return true;
    if ( subject != other.subject || !other.isAuthenticated )
        return false;
    CORE::CString ownKey = attributes.GetValueAlways( transport + ".publicKey" ).AsString();
    if ( !ownKey.IsNULLOrEmpty() && ownKey != other.attributes.GetValueAlways( other.transport + ".publicKey" ).AsString() )
        return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
