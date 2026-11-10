/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINCOMING_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINCOMING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H
#include "matchprovidermail_EMailMatchTypes.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a mail about a match says in its headers: all that is needed to decide whether to read the rest */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailIncoming
{
    public:

    bool valid;                     /**< the headers of a mail of this protocol */
    TMailMatchKind kind;
    CORE::CString matchId;
    UInt32 revision;
    UInt32 protocolVersion;
    CORE::CString messageId;        /**< with angle brackets, empty when the mail has none */
    CORE::CString fromAddress;      /**< the address of the first From, as written, empty when there is none */
    CORE::CString packageSha256;    /**< X-Redux-Package, empty when absent */
    CORE::CString wantTransfer;     /**< X-Redux-Transfer of a resend request: the form the answer should have, empty for the match's own */
    bool automatic;                 /**< a bounce, an automatic reply or a bulk message: never answered, never read */

    CEMailIncoming( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINCOMING_H ? */
