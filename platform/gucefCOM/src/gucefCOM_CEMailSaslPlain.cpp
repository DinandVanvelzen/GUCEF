/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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


#ifndef GUCEF_COM_CIEMAILSASLMECHANISM_H
#include "gucefCOM_CIEMailSaslMechanism.h"
#define GUCEF_COM_CIEMAILSASLMECHANISM_H
#endif /* GUCEF_COM_CIEMAILSASLMECHANISM_H ? */

#include "gucefCOM_CEMailSaslPlain.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailSaslPlain::CEMailSaslPlain( const CORE::CString& userName, const CORE::CString& password )
    : CIEMailSaslMechanism()
    , m_userName( userName )
    , m_password( password )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEMailSaslPlain::GetName( void ) const
{GUCEF_TRACE;

    static const CORE::CString name = "PLAIN";
    return name;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslPlain::HasInitialResponse( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslPlain::RevealsSecret( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslPlain::Start( CORE::CDynamicBuffer& initialResponse )
{GUCEF_TRACE;

    if ( m_userName.IsNULLOrEmpty() )
    {
        SetFailure( "No user name" );
        return false;
    }
    initialResponse.Clear();
    const char nul = '\0';
    initialResponse.Append( &nul, 1, true );
    initialResponse.Append( m_userName.C_String(), ByteLength( m_userName ), true );
    initialResponse.Append( &nul, 1, true );
    initialResponse.Append( m_password.C_String(), ByteLength( m_password ), true );
    return true;
}

/*-------------------------------------------------------------------------*/

EMailSaslState
CEMailSaslPlain::Step( const CORE::CDynamicBuffer& challenge, CORE::CDynamicBuffer& response )
{GUCEF_TRACE;

    // PLAIN has one message; a server that sends the empty challenge for the initial response gets it now
    response.Clear();
    if ( challenge.GetDataSize() > 0 )
    {
        SetFailure( "The server sent a challenge that PLAIN does not have" );
        return MAILSASL_FAILED;
    }
    Start( response );
    return MAILSASL_DONE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
