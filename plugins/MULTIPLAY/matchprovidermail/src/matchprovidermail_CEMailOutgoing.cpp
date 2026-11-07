/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H
#include "matchprovidermail_EMailMatchTypes.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_EMAILMATCHTYPES_H ? */

#include "matchprovidermail_CEMailOutgoing.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailOutgoing::CEMailOutgoing( void )
    : kind( MAILKIND_TURN )
    , matchId()
    , revision( 0 )
    , turn( 0 )
    , title()
    , from()
    , to()
    , cc()
    , inReplyTo()
    , references()
    , actorName()
    , nextName()
    , note()
    , bodyText()
    , packageSha256()
    , wantTransfer()
    , rematchOf()
    , linkText()
    , transfer( MAILTRANSFER_ATTACHMENT )
    , data( GUCEF_NULL )
    , dataLabel()
    , dataRevisionName()
    , extraData( GUCEF_NULL )
    , extraLabel()
    , extraRevisionName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
