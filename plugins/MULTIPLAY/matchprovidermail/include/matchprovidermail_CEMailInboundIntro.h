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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINBOUNDINTRO_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINBOUNDINTRO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H
#include "matchprovidermail_CEMailIntroRequest.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROREQUEST_H ? */
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

/** A request that came in and has not been answered, or that was answered */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailInboundIntro
{
    public:

    enum EStatus
    {
        STATUS_PENDING = 0 ,
        STATUS_ACCEPTED ,
        STATUS_DECLINED ,
        STATUS_CANCELLED                /**< the creator gave the game up before it began: kept until the player dismisses it or it runs out, so the list can say who cancelled */
    };

    CEMailIntroRequest request;
    CORE::CString fromAddress;          /**< lower case */
    Int64 receivedAt;
    EStatus status;
    bool keyChanged;                    /**< the key of the creator differs from the address book */
    CORE::CString knownKey;             /**< the key the address book holds for the creator, for the display */
    Int64 lastReplyAt;

    CEMailInboundIntro( void );
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINBOUNDINTRO_H ? */
