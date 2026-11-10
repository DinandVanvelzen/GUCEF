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

#include "gucefCOM_CEMailSaslLogin.h"

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

CORE::CString
GetText( const CORE::CDynamicBuffer& buffer )
{
    return buffer.GetDataSize() > 0 ? CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() ) : CORE::CString();
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailSaslLogin::CEMailSaslLogin( const CORE::CString& userName, const CORE::CString& password )
    : CIEMailSaslMechanism()
    , m_userName( userName )
    , m_password( password )
    , m_step( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEMailSaslLogin::GetName( void ) const
{GUCEF_TRACE;

    static const CORE::CString name = "LOGIN";
    return name;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslLogin::HasInitialResponse( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslLogin::RevealsSecret( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailSaslLogin::Start( CORE::CDynamicBuffer& initialResponse )
{GUCEF_TRACE;

    initialResponse.Clear();
    if ( m_userName.IsNULLOrEmpty() )
    {
        SetFailure( "No user name" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

EMailSaslState
CEMailSaslLogin::Step( const CORE::CDynamicBuffer& challenge, CORE::CDynamicBuffer& response )
{GUCEF_TRACE;

    // "Username:" and "Password:" in any case; when the prompt is not recognised the order decides
    const CORE::CString prompt = GetText( challenge ).Lowercase();
    bool wantsUser = prompt.HasSubstr( "user" ) >= 0 || prompt.HasSubstr( "name" ) >= 0;
    bool wantsPassword = prompt.HasSubstr( "pass" ) >= 0;
    if ( !wantsUser && !wantsPassword )
        wantsUser = 0 == m_step;

    if ( m_step >= 2 )
    {
        SetFailure( "The server asked more than LOGIN has" );
        return MAILSASL_FAILED;
    }
    if ( wantsPassword )
    {
        SetBuffer( response, m_password );
        m_step = 2;
        return MAILSASL_DONE;
    }
    SetBuffer( response, m_userName );
    m_step = 1;
    return MAILSASL_CONTINUE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
