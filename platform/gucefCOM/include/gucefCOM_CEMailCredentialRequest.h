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

#ifndef GUCEF_COM_CEMAILCREDENTIALREQUEST_H
#define GUCEF_COM_CEMAILCREDENTIALREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_EMAILTYPES_H
#include "gucefCOM_EMailTypes.h"
#define GUCEF_COM_EMAILTYPES_H
#endif /* GUCEF_COM_EMAILTYPES_H ? */

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

/** What a secret is */
enum EMailCredentialType
{
    MAILCRED_PASSWORD = 0,
    MAILCRED_BEARER_TOKEN           /**< an OAuth 2.0 access token, sent with XOAUTH2 */
};

/*-------------------------------------------------------------------------*/

/** What a mail client asks the provider for */
class GUCEF_COM_PUBLIC_CPP CEMailCredentialRequest
{
    public:

    EMailProtocol protocol;
    CORE::CString host;
    CORE::CString userName;             /**< the user of the settings, the provider may know better */
    bool previousAttemptFailed;         /**< the credential of the last request was refused by the server: a token should be renewed */
    UInt32 requestId;                   /**< echoed by CredentialReadyEvent when the answer was pending */

    CEMailCredentialRequest( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILCREDENTIALREQUEST_H ? */
