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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTRODRAFT_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTRODRAFT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H
#include "matchprovidermail_CEMailIntroContact.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H ? */

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

/** A game that is waiting for the introductions of its players */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailIntroDraft
{
    public:

    CORE::CString matchId;              /**< the id of the match to come */
    CORE::CString title;
    CORE::CString creatorName;
    CORE::CString creatorAddress;
    Int64 createdAt;
    GUCEF::map< CORE::CString, CORE::CString > settings;    /**< what the application needs to make the game when the draft is complete; opaque here */
    GUCEF::vector< CEMailIntroContact > contacts;

    CEMailIntroDraft( void );

    CEMailIntroContact* FindContact( const CORE::CString& address );
    const CEMailIntroContact* FindContact( const CORE::CString& address ) const;
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTRODRAFT_H ? */
