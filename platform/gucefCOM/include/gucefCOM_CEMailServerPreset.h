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

#ifndef GUCEF_COM_CEMAILSERVERPRESET_H
#define GUCEF_COM_CEMAILSERVERPRESET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_EMAILTYPES_H
#include "gucefCOM_EMailTypes.h"
#define GUCEF_COM_EMAILTYPES_H
#endif /* GUCEF_COM_EMAILTYPES_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What kind of password a provider wants from a mail program */
enum EMailPasswordPolicy
{
    MAILPASSWORD_NORMAL = 0 ,           /**< the password of the mailbox */
    MAILPASSWORD_APP_PASSWORD ,         /**< a password made for this program in the security settings of the account (the usual password is refused) */
    MAILPASSWORD_MAY_BE_REFUSED         /**< the provider turned password sign-in off for many accounts: it may work, it may be refused */
};

/*-------------------------------------------------------------------------*/

/** One server of a provider: host, port (0: the usual port of the protocol and security) and how the connection is protected */
class GUCEF_COM_PUBLIC_CPP CEMailServerPreset
{
    public:

    CORE::CString host;
    UInt16 port;
    EMailTlsMode security;

    CEMailServerPreset( void );

    /** Whether the preset names a server */
    bool IsSet( void ) const;

    /** The port to use: 'port', else the usual port of the protocol for the security */
    UInt16 GetEffectivePort( EMailProtocol protocol ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILSERVERPRESET_H ? */
