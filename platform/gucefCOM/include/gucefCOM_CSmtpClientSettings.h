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

#ifndef GUCEF_COM_CSMTPCLIENTSETTINGS_H
#define GUCEF_COM_CSMTPCLIENTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMAILCLIENTSETTINGS_H
#include "gucefCOM_CMailClientSettings.h"
#define GUCEF_COM_CMAILCLIENTSETTINGS_H
#endif /* GUCEF_COM_CMAILCLIENTSETTINGS_H ? */

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

/** Settings of the SMTP client: SMTP submission (RFC 6409) and relay (RFC 5321). Ports: 587 with STARTTLS, 465 with implicit TLS, 25 plain. */
class GUCEF_COM_PUBLIC_CPP CSmtpClientSettings : public CMailClientSettings
{
    public:

    CORE::CString heloName;                         /**< the name EHLO announces; set it to the host name or address of this computer ("localhost" when empty) */
    bool usePipelining;                             /**< MAIL and RCPT in one write when the server offers PIPELINING */
    bool failIfAnyRecipientRejected;                /**< false: the message goes to the recipients that were accepted and the rejected ones are reported */
    bool allowSmtpUtf8;                             /**< international addresses are sent when the server offers SMTPUTF8 */
    UInt32 dataTimeoutInMs;                         /**< the wait for the answer to the end of the message data */
    UInt32 maxRecipients;                           /**< a message with more envelope recipients is refused before anything is sent */

    CSmtpClientSettings( void );

    CSmtpClientSettings( const CSmtpClientSettings& src );

    virtual ~CSmtpClientSettings() GUCEF_VIRTUAL_OVERRIDE;

    CSmtpClientSettings& operator=( const CSmtpClientSettings& src );

    virtual UInt16 GetDefaultPort( EMailTlsMode mode ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_COM_CSMTPCLIENTSETTINGS_H ? */
