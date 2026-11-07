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


#include "matchprovidermail_CEMailInboxRefusal.h"

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

CEMailInboxRefusal::CEMailInboxRefusal( void )
    : CORE::CICloneable()
    , matchId()
    , revision( 0 )
    , kind( MAILKIND_UNKNOWN )
    , reason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailInboxRefusal::CEMailInboxRefusal( const CEMailInboxRefusal& src )
    : CORE::CICloneable( src )
    , matchId( src.matchId )
    , revision( src.revision )
    , kind( src.kind )
    , reason( src.reason )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailInboxRefusal::~CEMailInboxRefusal()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailInboxRefusal&
CEMailInboxRefusal::operator=( const CEMailInboxRefusal& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        revision = src.revision;
        kind = src.kind;
        reason = src.reason;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailInboxRefusal::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailInboxRefusal( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CEMailInboxRefusal::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailInboxRefusal::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailInboxRefusal";
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
