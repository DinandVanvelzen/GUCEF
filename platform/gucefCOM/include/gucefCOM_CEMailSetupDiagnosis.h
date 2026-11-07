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

#ifndef GUCEF_COM_CEMAILSETUPDIAGNOSIS_H
#define GUCEF_COM_CEMAILSETUPDIAGNOSIS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CEMAILPROVIDERPRESET_H
#include "gucefCOM_CEMailProviderPreset.h"
#define GUCEF_COM_CEMAILPROVIDERPRESET_H
#endif /* GUCEF_COM_CEMAILPROVIDERPRESET_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The step of the test of a mailbox that failed or succeeded: the test only logs in, selects the folder and does a NOOP, it sends and reads nothing */
enum EMailSetupStep
{
    MAILSETUP_STEP_INCOMING = 0 ,       /**< connect to the incoming server (IMAP or POP3), secure it and log in */
    MAILSETUP_STEP_FOLDER ,             /**< select the folder the games read (IMAP) */
    MAILSETUP_STEP_OUTGOING             /**< connect to the SMTP server, secure it and log in */
};

/** What went wrong, in the words a person uses */
enum EMailSetupProblem
{
    MAILSETUP_OK = 0 ,
    MAILSETUP_UNREACHABLE ,             /**< no connection: wrong server name or port, no internet, a firewall */
    MAILSETUP_TLS_PROBLEM ,             /**< the secure connection failed: wrong security setting or a certificate that cannot be trusted */
    MAILSETUP_LOGIN_REFUSED ,           /**< the incoming server refused the user name, the password or the token */
    MAILSETUP_FOLDER_NOT_FOUND ,        /**< the login worked, the folder does not exist */
    MAILSETUP_SMTP_REFUSED ,            /**< the outgoing server refused the login or the connection */
    MAILSETUP_NO_ANSWER ,               /**< the server did not answer in time */
    MAILSETUP_OTHER                     /**< anything else: the server said something unexpected */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Turns the outcome of one step of the test of a mailbox into a plain sentence and what to try, with advice for the kind of provider
 *  (an app password where the provider wants one, a new sign-in for an OAuth account, the usual servers of the provider). The texts hold
 *  no address, no user name, no password and no text of the server. Host names and ports may appear: they are settings of the person.
 */
class GUCEF_COM_PUBLIC_CPP CEMailSetupDiagnosis
{
    public:

    /** The problem a failure of the class means at the step */
    static EMailSetupProblem Classify( EMailSetupStep step, EMailErrorClass errorClass );

    /**
     *  The sentence for the person. 'preset' is the recognized provider (NULL for none), 'usesOAuth' tells that the account signs in with the
     *  provider instead of a password, 'host' and 'port' the server of the step (shown to let the person check them; empty for none)
     */
    static CORE::CString Describe( EMailSetupStep step                  ,
                                   EMailSetupProblem problem            ,
                                   const CEMailProviderPreset* preset    ,
                                   bool usesOAuth                       ,
                                   const CORE::CString& host            ,
                                   UInt16 port                          );

    /** Whether the problem means that the settings or the login have to change (as opposed to a server that cannot be reached for now) */
    static bool NeedsNewSettings( EMailSetupProblem problem );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILSETUPDIAGNOSIS_H ? */
