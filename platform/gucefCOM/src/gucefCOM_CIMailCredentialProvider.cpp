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

#include "gucefCOM_CIMailCredentialProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIMailCredentialProvider::CredentialReadyEvent = "GUCEF::COM::CIMailCredentialProvider::CredentialReadyEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailCredentialRequest::CMailCredentialRequest( void )
    : protocol( MAILPROTOCOL_SMTP )
    , host()
    , userName()
    , previousAttemptFailed( false )
    , requestId( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailCredential::CMailCredential( void )
    : type( MAILCRED_PASSWORD )
    , userName()
    , secret()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIMailCredentialProvider::RegisterEvents( void )
{GUCEF_TRACE;

    CredentialReadyEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIMailCredentialProvider::CIMailCredentialProvider( void )
    : CORE::CNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIMailCredentialProvider::~CIMailCredentialProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIMailCredentialProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CIMailCredentialProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CStaticMailCredentialProvider::CStaticMailCredentialProvider( void )
    : CIMailCredentialProvider()
    , m_credential()
    , m_rejectionCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStaticMailCredentialProvider::CStaticMailCredentialProvider( EMailCredentialType type, const CORE::CString& userName, const CORE::CString& secret )
    : CIMailCredentialProvider()
    , m_credential()
    , m_rejectionCount( 0 )
{GUCEF_TRACE;

    SetCredential( type, userName, secret );
}

/*-------------------------------------------------------------------------*/

CStaticMailCredentialProvider::~CStaticMailCredentialProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CStaticMailCredentialProvider::SetCredential( EMailCredentialType type, const CORE::CString& userName, const CORE::CString& secret )
{GUCEF_TRACE;

    m_credential.type = type;
    m_credential.userName = userName;
    m_credential.secret = secret;
}

/*-------------------------------------------------------------------------*/

CIMailCredentialProvider::EResult
CStaticMailCredentialProvider::GetCredential( const CMailCredentialRequest& request, CMailCredential& credential )
{GUCEF_TRACE;

    if ( m_credential.secret.IsNULLOrEmpty() )
        return CRED_FAILED;
    credential = m_credential;
    if ( credential.userName.IsNULLOrEmpty() )
        credential.userName = request.userName;
    return CRED_READY;
}

/*-------------------------------------------------------------------------*/

void
CStaticMailCredentialProvider::ReportRejected( const CMailCredentialRequest& request )
{GUCEF_TRACE;

    ++m_rejectionCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CStaticMailCredentialProvider::GetRejectionCount( void ) const
{GUCEF_TRACE;

    return m_rejectionCount;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStaticMailCredentialProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CStaticMailCredentialProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
