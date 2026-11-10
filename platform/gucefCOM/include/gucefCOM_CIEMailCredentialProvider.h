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

#ifndef GUCEF_COM_CIEMAILCREDENTIALPROVIDER_H
#define GUCEF_COM_CIEMAILCREDENTIALPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_COM_EMAILTYPES_H
#include "gucefCOM_EMailTypes.h"
#define GUCEF_COM_EMAILTYPES_H
#endif /* GUCEF_COM_EMAILTYPES_H ? */

#ifndef GUCEF_COM_CEMAILCREDENTIALREQUEST_H
#include "gucefCOM_CEMailCredentialRequest.h"
#define GUCEF_COM_CEMAILCREDENTIALREQUEST_H
#endif /* GUCEF_COM_CEMAILCREDENTIALREQUEST_H ? */

#ifndef GUCEF_COM_CEMAILCREDENTIAL_H
#include "gucefCOM_CEMailCredential.h"
#define GUCEF_COM_CEMAILCREDENTIAL_H
#endif /* GUCEF_COM_CEMAILCREDENTIAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The source of the credentials of a mail account, implemented by the application. This is how an OAuth 2.0 access
 *  token that expires and is renewed (by gucefAUTH or by anything else) reaches the mail clients without
 *  gucefCOM depending on the authentication module: the client asks when it is about to log in, and asks
 *  again, with previousAttemptFailed, when the server refused what it was given.
 *
 *  An answer that needs time (a renewal over the network) is CRED_PENDING: the client waits, within its connect
 *  timeout, for CredentialReadyEvent (data: the request id as UInt32) and asks again then.
 */
class GUCEF_COM_PUBLIC_CPP CIEMailCredentialProvider : public CORE::CNotifier
{
    public:

    static const CORE::CEvent CredentialReadyEvent;

    typedef CORE::TCloneableUInt32 TCredentialReadyEventData;

    enum EResult
    {
        CRED_READY = 0,         /**< 'credential' is filled in */
        CRED_PENDING,           /**< not yet: CredentialReadyEvent follows */
        CRED_FAILED             /**< there is no credential, the client fails with an authentication error */
    };

    static void RegisterEvents( void );

    CIEMailCredentialProvider( void );

    virtual ~CIEMailCredentialProvider() GUCEF_VIRTUAL_OVERRIDE;

    virtual EResult GetCredential( const CEMailCredentialRequest& request, CEMailCredential& credential ) = 0;

    /** The server refused the credential that was given for this request. A provider with a token renews it now */
    virtual void ReportRejected( const CEMailCredentialRequest& request ) = 0;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIEMAILCREDENTIALPROVIDER_H ? */
