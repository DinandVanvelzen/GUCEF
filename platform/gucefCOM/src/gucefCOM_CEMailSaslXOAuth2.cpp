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

#include "gucefCOM_CEMailSaslXOAuth2.h"

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

void
SetBuffer( CORE::CDynamicBuffer& buffer, const CORE::CString& text )
{
    buffer.Clear();
    if ( ByteLength( text ) > 0 )
        buffer.CopyFrom( ByteLength( text ), text.C_String() );
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailSaslXOAuth2::CEMailSaslXOAuth2( const CORE::CString& userName, const CORE::CString& accessToken )
    : CIEMailSaslMechanism()
    , m_userName( userName )
    , m_accessToken( accessToken )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEMailSaslXOAuth2::GetName( void ) const
{GUCEF_TRACE;

    static const CORE::CString name = "XOAUTH2";
    return name;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslXOAuth2::HasInitialResponse( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslXOAuth2::RevealsSecret( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslXOAuth2::Start( CORE::CDynamicBuffer& initialResponse )
{GUCEF_TRACE;

    // a token or a user with a control character would end the exchange early or inject another field
    if ( m_userName.IsNULLOrEmpty() || m_accessToken.IsNULLOrEmpty() )
    {
        SetFailure( "No user name or access token" );
        return false;
    }
    const char* texts[ 2 ] = { m_userName.C_String(), m_accessToken.C_String() };
    for ( int t=0; t<2; ++t )
        for ( const char* c = texts[ t ]; '\0' != *c; ++c )
            if ( ( (UInt8) *c ) < 0x20 || 0x7F == (UInt8) *c )
            {
                SetFailure( "The user name or access token has a control character" );
                return false;
            }
    SetBuffer( initialResponse, "user=" + m_userName + "\x01" + "auth=Bearer " + m_accessToken + "\x01\x01" );
    return true;
}

/*-------------------------------------------------------------------------*/

EMailSaslState
CEMailSaslXOAuth2::Step( const CORE::CDynamicBuffer& challenge, CORE::CDynamicBuffer& response )
{GUCEF_TRACE;

    // the challenge is the failure of the server as JSON; the protocol wants an empty answer, after which the server ends the exchange
    response.Clear();
    SetFailure( "The server refused the access token" );
    return MAILSASL_FAILED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
