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

#ifndef GUCEF_COM_CMAILCLIENTSETTINGS_H
#define GUCEF_COM_CMAILCLIENTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CTLSSETTINGS_H
#include "gucefCOMCORE_CTlsSettings.h"
#define GUCEF_COMCORE_CTLSSETTINGS_H
#endif /* GUCEF_COMCORE_CTLSSETTINGS_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

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

/**
 *  What the three mail clients share: where the server is, how the connection is protected, who logs in, how long
 *  to wait and how much a server may send before it is cut off.
 *
 *  In config: the members by the same name as attributes or child values, the lists comma separated, the TLS settings
 *  in the child "Tls", tlsMode as none, starttls, starttls-if-offered or implicit. SaveConfig() leaves the password out
 *  unless saveSecrets is set: applications that care use a CIMailCredentialProvider and keep it out of files.
 */
class GUCEF_COM_PUBLIC_CPP CMailClientSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString host;
    UInt16 port;                                    /**< 0: the usual port of the protocol for the tlsMode */
    EMailTlsMode tlsMode;                           /**< STARTTLS by default: the upgrade is required */
    COMCORE::CTlsSettings tls;                      /**< how TLS is done: certificates are verified against the trust store of the system, the host name is checked */
    CORE::CString userName;
    CORE::CString password;                         /**< or the access token when the account uses OAuth 2.0, see authMechanisms */
    bool passwordIsToken;                           /**< the secret is an OAuth 2.0 access token: XOAUTH2 */
    CORE::CString::StringVector authMechanisms;     /**< in order of preference, empty: SCRAM-SHA-256, PLAIN, LOGIN (XOAUTH2 for a token) */
    bool allowInsecureAuth;                         /**< allows sending the credentials without TLS, false by default */
    UInt32 connectTimeoutInMs;                      /**< until the server is ready for commands, TLS and login included */
    UInt32 commandTimeoutInMs;                      /**< no answer to a command */
    UInt32 idleTimeoutInMs;                         /**< 0: no limit on a silent connection that is waiting for nothing */
    UInt32 maxLineLength;                           /**< a line of the server longer than this ends the connection */
    UInt32 maxLiteralSize;                          /**< the largest counted block (IMAP literal) accepted */
    UInt32 maxResponseSize;                         /**< the largest response (IMAP: lines and literals of one response; POP3: a message) */
    UInt32 maxResponseCount;                        /**< the most lines/untagged responses to one command */
    UInt32 maxMessageSize;                          /**< the largest message that is sent or received */
    bool saveSecrets;                               /**< SaveConfig() writes the password */

    CMailClientSettings( void );

    CMailClientSettings( const CMailClientSettings& src );

    virtual ~CMailClientSettings() GUCEF_VIRTUAL_OVERRIDE;

    CMailClientSettings& operator=( const CMailClientSettings& src );

    /** The port of the protocol for a TLS mode */
    virtual UInt16 GetDefaultPort( EMailTlsMode mode ) const = 0;

    /** port, or the default of the protocol when port is 0 */
    UInt16 GetEffectivePort( void ) const;

    /** The mechanisms to try for the kind of secret, 'authMechanisms' when set */
    void GetPreferredMechanisms( CORE::CString::StringVector& mechanisms ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    static CORE::CString TlsModeToString( EMailTlsMode mode );

    static bool TlsModeFromString( const CORE::CString& text, EMailTlsMode& mode );

    protected:

    /** A list written as comma separated text, and read back */
    static CORE::CString JoinList( const CORE::CString::StringVector& list );
    static void ReadList( const CORE::CDataNode& config, const char* name, CORE::CString::StringVector& list );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILCLIENTSETTINGS_H ? */
