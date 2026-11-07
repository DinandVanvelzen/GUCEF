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

#ifndef GUCEF_COM_EMAILTYPES_H
#define GUCEF_COM_EMAILTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

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

/** How a mail connection is protected */
enum EMailTlsMode
{
    MAILTLS_NONE = 0,               /**< plain text. Credentials are only sent when CEMailClientSettings::allowInsecureAuth is set */
    MAILTLS_STARTTLS,               /**< plain first, then upgraded with STARTTLS/STLS. The upgrade is required: a server that does not offer it fails the connection */
    MAILTLS_STARTTLS_IF_OFFERED,    /**< upgraded when the server offers it, plain otherwise. Credentials still need TLS unless allowInsecureAuth is set */
    MAILTLS_IMPLICIT                /**< TLS from the first byte (ports 465, 993, 995) */
};

/** The kind of failure of a mail operation, so that a caller can tell "try again later" from "give up" */
enum EMailErrorClass
{
    MAILERR_NONE = 0,
    MAILERR_TEMPORARY,              /**< the server asked to try again later (SMTP 4xx) */
    MAILERR_PERMANENT,              /**< the server refused (SMTP 5xx, IMAP NO/BAD, POP3 -ERR) */
    MAILERR_LOCAL,                  /**< refused here before anything was sent: invalid message, too large, too many recipients */
    MAILERR_AUTH,                   /**< authentication failed or no usable mechanism */
    MAILERR_TLS,                    /**< TLS failed or was required and not available */
    MAILERR_CONNECTION,             /**< connect failed or the server dropped the connection */
    MAILERR_TIMEOUT,                /**< the server did not answer in time */
    MAILERR_PROTOCOL,               /**< the server said something that is not in the protocol, or exceeded a limit */
    MAILERR_CANCELLED               /**< the caller cancelled */
};

typedef enum EMailErrorClass TMailErrorClass;

/** The mail protocols of this module */
enum EMailProtocol
{
    MAILPROTOCOL_SMTP = 0,
    MAILPROTOCOL_POP3,
    MAILPROTOCOL_IMAP
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What replaces a secret in a log */
#define GUCEF_COM_REDACTION_MARKER                      "***"

#define GUCEF_COM_MAIL_DEFAULT_CONNECT_TIMEOUT_MS       30000
#define GUCEF_COM_MAIL_DEFAULT_COMMAND_TIMEOUT_MS       60000
#define GUCEF_COM_MAIL_DEFAULT_DATA_TIMEOUT_MS          300000
#define GUCEF_COM_MAIL_DEFAULT_MAX_LINE_LENGTH          8192
#define GUCEF_COM_MAIL_DEFAULT_MAX_LITERAL_SIZE         ( 32 * 1024 * 1024 )
#define GUCEF_COM_MAIL_DEFAULT_MAX_RESPONSE_SIZE        ( 64 * 1024 * 1024 )
#define GUCEF_COM_MAIL_DEFAULT_MAX_RESPONSE_COUNT       100000
#define GUCEF_COM_MAIL_DEFAULT_MAX_MESSAGE_SIZE         ( 64 * 1024 * 1024 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_EMAILTYPES_H ? */
