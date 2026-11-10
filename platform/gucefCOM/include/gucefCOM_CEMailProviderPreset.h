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

#ifndef GUCEF_COM_CEMAILPROVIDERPRESET_H
#define GUCEF_COM_CEMAILPROVIDERPRESET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_EMAILTYPES_H
#include "gucefCOM_EMailTypes.h"
#define GUCEF_COM_EMAILTYPES_H
#endif /* GUCEF_COM_EMAILTYPES_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

#ifndef GUCEF_COM_CEMAILSERVERPRESET_H
#include "gucefCOM_CEMailServerPreset.h"
#define GUCEF_COM_CEMAILSERVERPRESET_H
#endif /* GUCEF_COM_CEMAILSERVERPRESET_H ? */

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What is known about the mail of one provider: the domains of its addresses, the IMAP, SMTP and POP3 servers with port and security, what
 *  kind of password it wants, the OAuth provider of gucefAUTH that signs in to it (by name; empty when it has none) with the scopes mail
 *  needs, the steps in plain words and a web page that explains them (shown as text: nothing here opens a browser).
 *
 *  Presets are data: CEMailProviderPresets holds the table, compiled in defaults that a configuration overrides or extends. The server
 *  names and ports of the defaults are the public documentation of the providers; they were not tried against the real services.
 */
class GUCEF_COM_PUBLIC_CPP CEMailProviderPreset
{
    public:

    CORE::CString id;                       /**< lower case letters and digits, 1 to 32 characters: "gmail" */
    CORE::CString displayName;              /**< "Gmail and Google Workspace" */
    CORE::CString::StringVector domains;    /**< lower case domains of the addresses of the provider; a subdomain of one of them belongs to it too */
    CEMailServerPreset imap;
    CEMailServerPreset smtp;
    CEMailServerPreset pop3;
    EMailPasswordPolicy passwordPolicy;
    CORE::CString oauthProvider;            /**< name of the gucefAUTH provider that signs in to this mail ("google", "microsoft"); empty: none */
    CORE::CString::StringVector oauthScopes;    /**< what mail needs besides the login scopes of the provider */
    bool serverFieldsEditable;              /**< the person is expected to fill in the servers (custom, bridge): the form shows them as fields to edit */
    CORE::CString passwordHelp;             /**< the steps for the password of this provider, plain words */
    CORE::CString helpUrl;                  /**< a web page that explains the steps; shown as text, never opened by itself */

    CEMailProviderPreset( void );

    /** Whether the preset can be used; 'reason' says what is wrong (without repeating the value) */
    bool IsValid( CORE::CString& reason ) const;

    /** Whether the address domain (lower case, validated) is one of the domains of the preset or a subdomain of one */
    bool MatchesDomain( const CORE::CString& domain ) const;

    /** Whether the provider has an OAuth sign-in at all */
    bool HasOAuth( void ) const;

    /** One entry of the table as a node: the attributes id, displayName, domains (comma list), imapHost, imapPort, imapSecurity, smtp..., pop3..., passwordPolicy, oauthProvider, oauthScopes, serverFieldsEditable, passwordHelp, helpUrl */
    bool SaveConfig( CORE::CDataNode& node ) const;

    /** Reads an entry. The id is the attribute id, else the name of the node (so a JSON object keyed by id works). False when the entry is not usable; nothing is changed then */
    bool LoadConfig( const CORE::CDataNode& node );

    static CORE::CString PasswordPolicyToString( EMailPasswordPolicy policy );
    static bool PasswordPolicyFromString( const CORE::CString& text, EMailPasswordPolicy& policy );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILPROVIDERPRESET_H ? */
