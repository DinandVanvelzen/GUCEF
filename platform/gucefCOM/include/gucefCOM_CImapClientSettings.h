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

#ifndef GUCEF_COM_CIMAPCLIENTSETTINGS_H
#define GUCEF_COM_CIMAPCLIENTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CEMAILCLIENTSETTINGS_H
#include "gucefCOM_CEMailClientSettings.h"
#define GUCEF_COM_CEMAILCLIENTSETTINGS_H
#endif /* GUCEF_COM_CEMAILCLIENTSETTINGS_H ? */

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

/** Settings of the IMAP client: IMAP4rev1 (RFC 3501) and IMAP4rev2 (RFC 9051) subset. Ports: 143 with STARTTLS, 993 with implicit TLS. */
class GUCEF_COM_PUBLIC_CPP CImapClientSettings : public CEMailClientSettings
{
    public:

    UInt32 idleRenewInMs;                           /**< IDLE is ended and started again at this interval, RFC 2177 asks for less than 29 minutes */
    UInt32 maxFetchResults;                         /**< the most messages one FETCH or SEARCH answer may hold */
    bool useLiteralPlus;                            /**< non synchronizing literals when the server offers LITERAL+ (not used yet: literals wait for the continuation) */

    CImapClientSettings( void );

    CImapClientSettings( const CImapClientSettings& src );

    virtual ~CImapClientSettings() GUCEF_VIRTUAL_OVERRIDE;

    CImapClientSettings& operator=( const CImapClientSettings& src );

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

#endif /* GUCEF_COM_CIMAPCLIENTSETTINGS_H ? */
