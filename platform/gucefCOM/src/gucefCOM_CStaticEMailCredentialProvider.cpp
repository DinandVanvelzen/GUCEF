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

#ifndef GUCEF_COM_CEMAILCREDENTIALREQUEST_H
#include "gucefCOM_CEMailCredentialRequest.h"
#define GUCEF_COM_CEMAILCREDENTIALREQUEST_H
#endif /* GUCEF_COM_CEMAILCREDENTIALREQUEST_H ? */

#ifndef GUCEF_COM_CEMAILCREDENTIAL_H
#include "gucefCOM_CEMailCredential.h"
#define GUCEF_COM_CEMAILCREDENTIAL_H
#endif /* GUCEF_COM_CEMAILCREDENTIAL_H ? */

#include "gucefCOM_CStaticEMailCredentialProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStaticEMailCredentialProvider::CStaticEMailCredentialProvider( void )
    : CIEMailCredentialProvider()
    , m_credential()
    , m_rejectionCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStaticEMailCredentialProvider::CStaticEMailCredentialProvider( EMailCredentialType type, const CORE::CString& userName, const CORE::CString& secret )
    : CIEMailCredentialProvider()
    , m_credential()
    , m_rejectionCount( 0 )
{GUCEF_TRACE;

    SetCredential( type, userName, secret );
}

/*-------------------------------------------------------------------------*/

CStaticEMailCredentialProvider::~CStaticEMailCredentialProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CStaticEMailCredentialProvider::SetCredential( EMailCredentialType type, const CORE::CString& userName, const CORE::CString& secret )
{GUCEF_TRACE;

    m_credential.type = type;
    m_credential.userName = userName;
    m_credential.secret = secret;
}

/*-------------------------------------------------------------------------*/

CIEMailCredentialProvider::EResult
CStaticEMailCredentialProvider::GetCredential( const CEMailCredentialRequest& request, CEMailCredential& credential )
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
CStaticEMailCredentialProvider::ReportRejected( const CEMailCredentialRequest& request )
{GUCEF_TRACE;

    ++m_rejectionCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CStaticEMailCredentialProvider::GetRejectionCount( void ) const
{GUCEF_TRACE;

    return m_rejectionCount;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStaticEMailCredentialProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CStaticEMailCredentialProvider";
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
