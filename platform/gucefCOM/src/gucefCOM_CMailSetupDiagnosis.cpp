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


#include "gucefCOM_CMailSetupDiagnosis.h"

#ifndef GUCEF_COM_CMAILPROVIDERPRESETS_H
#include "gucefCOM_CMailProviderPresets.h"
#define GUCEF_COM_CMAILPROVIDERPRESETS_H
#endif /* GUCEF_COM_CMAILPROVIDERPRESETS_H ? */

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

/** "The incoming server" or "The outgoing server" */
CORE::CString
ServerName( EMailSetupStep step )
{
    return MAILSETUP_STEP_OUTGOING == step ? "The outgoing server" : "The incoming server";
}

/** The server with its name and port when they are known: " (imap.example.org, port 993)" */
CORE::CString
ServerAddress( const CORE::CString& host, UInt16 port )
{
    if ( host.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    CORE::CString text = " (" + host;
    if ( 0 != port )
        text += ", port " + CORE::UInt32ToString( port );
    return text + ")";
}

/** What to try when the provider refused the login, for the kind of account */
CORE::CString
LoginAdvice( const CMailProviderPreset* preset, bool usesOAuth )
{
    if ( usesOAuth )
    {
        const CORE::CString name = GUCEF_NULL != preset && preset->HasOAuth() ? CMailProviderPresets::GetOAuthDisplayName( preset->oauthProvider ) : CORE::CString( "your mail provider" );
        return "Your sign-in with " + name + " was refused or has run out: sign in again in the mailbox settings.";
    }
    if ( GUCEF_NULL != preset )
    {
        if ( MAILPASSWORD_APP_PASSWORD == preset->passwordPolicy )
            return preset->displayName + " needs an app password, not your usual password. Make one in the security settings of the account (this needs 2-step verification on the account) and type it as the password.";
        if ( MAILPASSWORD_MAY_BE_REFUSED == preset->passwordPolicy )
        {
            CORE::CString text = preset->displayName + " turned off password sign-in for many accounts. Check the password; if it is right, this account needs the sign-in with ";
            text += preset->HasOAuth() ? CMailProviderPresets::GetOAuthDisplayName( preset->oauthProvider ) : CORE::CString( "its provider" );
            return text + ".";
        }
    }
    return "Check the user name and the password (many providers want the whole mail address as the user name) and that your provider allows mail programs to log in.";
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

EMailSetupProblem
CMailSetupDiagnosis::Classify( EMailSetupStep step, EMailErrorClass errorClass )
{GUCEF_TRACE;

    switch ( errorClass )
    {
        case MAILERR_NONE: return MAILSETUP_OK;
        case MAILERR_CONNECTION: return MAILSETUP_UNREACHABLE;
        case MAILERR_TLS: return MAILSETUP_TLS_PROBLEM;
        case MAILERR_TIMEOUT: return MAILSETUP_NO_ANSWER;
        case MAILERR_AUTH:
            return MAILSETUP_STEP_OUTGOING == step ? MAILSETUP_SMTP_REFUSED : MAILSETUP_LOGIN_REFUSED;
        case MAILERR_PERMANENT:
            if ( MAILSETUP_STEP_OUTGOING == step )
                return MAILSETUP_SMTP_REFUSED;
            return MAILSETUP_STEP_FOLDER == step ? MAILSETUP_FOLDER_NOT_FOUND : MAILSETUP_LOGIN_REFUSED;
        case MAILERR_TEMPORARY:
            return MAILSETUP_STEP_OUTGOING == step ? MAILSETUP_SMTP_REFUSED : MAILSETUP_OTHER;
        default:
            return MAILSETUP_OTHER;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailSetupDiagnosis::Describe( EMailSetupStep step               ,
                               EMailSetupProblem problem         ,
                               const CMailProviderPreset* preset ,
                               bool usesOAuth                    ,
                               const CORE::CString& host         ,
                               UInt16 port                       )
{GUCEF_TRACE;

    const CORE::CString server = ServerName( step );
    const CORE::CString where = ServerAddress( host, port );
    const bool known = GUCEF_NULL != preset && !preset->serverFieldsEditable && preset->imap.IsSet();
    switch ( problem )
    {
        case MAILSETUP_OK:
        {
            if ( MAILSETUP_STEP_FOLDER == step )
                return "The folder the games read was found.";
            return server + " accepted the login.";
        }
        case MAILSETUP_UNREACHABLE:
        {
            CORE::CString text = server + where + " could not be reached. Check the name of the server and the port, that you are connected to the internet and that nothing blocks the connection.";
            if ( known )
            {
                const CMailServerPreset& usual = MAILSETUP_STEP_OUTGOING == step ? preset->smtp : preset->imap;
                text += " The usual server for " + preset->displayName + " is " + usual.host + ".";
            }
            return text;
        }
        case MAILSETUP_TLS_PROBLEM:
        {
            return "The secure connection to the " + CORE::CString( MAILSETUP_STEP_OUTGOING == step ? "outgoing" : "incoming" ) + " server" + where +
                   " could not be made. Check the setting for security: SSL/TLS goes with the ports 993 (IMAP), 995 (POP3) and 465 (SMTP), STARTTLS with 143, 110 and 587. The certificate of the server may also not be trusted.";
        }
        case MAILSETUP_LOGIN_REFUSED:
        {
            return server + where + " refused the login. " + LoginAdvice( preset, usesOAuth );
        }
        case MAILSETUP_FOLDER_NOT_FOUND:
        {
            return "The login worked, but the mail folder that the games read is not on the server. Use INBOX, or type the name of the folder as your mail program shows it.";
        }
        case MAILSETUP_SMTP_REFUSED:
        {
            return "The outgoing server" + where + " refused the login or the connection. " + LoginAdvice( preset, usesOAuth );
        }
        case MAILSETUP_NO_ANSWER:
        {
            return server + where + " did not answer in time. It may be busy or the port may be wrong. Try again in a moment, and check the port and the security setting.";
        }
        default:
        {
            return server + where + " said something the game does not understand, or the connection broke. Check the name of the server, the port and the security setting.";
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMailSetupDiagnosis::NeedsNewSettings( EMailSetupProblem problem )
{GUCEF_TRACE;

    return MAILSETUP_LOGIN_REFUSED == problem || MAILSETUP_SMTP_REFUSED == problem || MAILSETUP_TLS_PROBLEM == problem || MAILSETUP_FOLDER_NOT_FOUND == problem;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
