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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H
#include "matchprovidermail_CEMailIntroContact.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACT_H ? */

#include "matchprovidermail_CEMailIntroDraft.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
Lower( const CORE::CString& text )
{GUCEF_TRACE;

    return text.Lowercase();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailIntroDraft::CEMailIntroDraft( void )
    : matchId()
    , title()
    , creatorName()
    , creatorAddress()
    , createdAt( 0 )
    , settings()
    , contacts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailIntroContact*
CEMailIntroDraft::FindContact( const CORE::CString& address )
{GUCEF_TRACE;

    const CORE::CString wanted = Lower( address );
    for ( size_t i=0; i<contacts.size(); ++i )
        if ( contacts[ i ].address == wanted )
            return &contacts[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CEMailIntroContact*
CEMailIntroDraft::FindContact( const CORE::CString& address ) const
{GUCEF_TRACE;

    const CORE::CString wanted = Lower( address );
    for ( size_t i=0; i<contacts.size(); ++i )
        if ( contacts[ i ].address == wanted )
            return &contacts[ i ];
    return GUCEF_NULL;
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
