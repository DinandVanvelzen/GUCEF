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

#ifndef GUCEF_COM_CIMAILCREDENTIALPROVIDER_H
#define GUCEF_COM_CIMAILCREDENTIALPROVIDER_H

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

#ifndef GUCEF_COM_MAILTYPES_H
#include "gucefCOM_MailTypes.h"
#define GUCEF_COM_MAILTYPES_H
#endif /* GUCEF_COM_MAILTYPES_H ? */

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

/** What a mail client asks the provider for */
class GUCEF_COM_PUBLIC_CPP CMailCredentialRequest
{
    public:

    EMailProtocol protocol;
    CORE::CString host;
    CORE::CString userName;             /**< the user of the settings, the provider may know better */
    bool previousAttemptFailed;         /**< the credential of the last request was refused by the server: a token should be renewed */
    UInt32 requestId;                   /**< echoed by CredentialReadyEvent when the answer was pending */

    CMailCredentialRequest( void );
};

/** What the provider gives */
class GUCEF_COM_PUBLIC_CPP CMailCredential
{
    public:

    EMailCredentialType type;
    CORE::CString userName;
    CORE::CString secret;               /**< the password or the access token. Never logged by the clients */

    CMailCredential( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The source of the credentials of a mail account, implemented by the application. This is how an OAuth 2.0 access
 *  token that expires and is renewed (by gucefAUTH or by anything else) reaches the mail clients without
 *  gucefCOM depending on the authentication module: the client asks when it is about to log in, and asks
 *  again, with previousAttemptFailed, when the server refused what it was given.
 *
 *  An answer that needs time (a renewal over the network) is CRED_PENDING: the client waits, within its connect
 *  timeout, for CredentialReadyEvent (data: the request id as UInt32) and asks again then.
 */
class GUCEF_COM_PUBLIC_CPP CIMailCredentialProvider : public CORE::CNotifier
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

    CIMailCredentialProvider( void );

    virtual ~CIMailCredentialProvider() GUCEF_VIRTUAL_OVERRIDE;

    virtual EResult GetCredential( const CMailCredentialRequest& request, CMailCredential& credential ) = 0;

    /** The server refused the credential that was given for this request. A provider with a token renews it now */
    virtual void ReportRejected( const CMailCredentialRequest& request ) = 0;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** A fixed user name and password or token */
class GUCEF_COM_PUBLIC_CPP CStaticMailCredentialProvider : public CIMailCredentialProvider
{
    public:

    CStaticMailCredentialProvider( void );

    CStaticMailCredentialProvider( EMailCredentialType type, const CORE::CString& userName, const CORE::CString& secret );

    virtual ~CStaticMailCredentialProvider() GUCEF_VIRTUAL_OVERRIDE;

    void SetCredential( EMailCredentialType type, const CORE::CString& userName, const CORE::CString& secret );

    virtual EResult GetCredential( const CMailCredentialRequest& request, CMailCredential& credential ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReportRejected( const CMailCredentialRequest& request ) GUCEF_VIRTUAL_OVERRIDE;

    /** How often the server refused the credential */
    UInt32 GetRejectionCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CMailCredential m_credential;
    UInt32 m_rejectionCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAILCREDENTIALPROVIDER_H ? */
