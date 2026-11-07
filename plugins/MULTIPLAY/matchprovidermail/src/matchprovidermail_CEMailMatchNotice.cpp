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


#include "matchprovidermail_CEMailMatchNotice.h"

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

CEMailMatchNotice::CEMailMatchNotice( void )
    : CORE::CICloneable()
    , matchId()
    , revision( 0 )
    , code()
    , description()
    , userId()
    , proposalId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailMatchNotice::CEMailMatchNotice( const CEMailMatchNotice& src )
    : CORE::CICloneable( src )
    , matchId( src.matchId )
    , revision( src.revision )
    , code( src.code )
    , description( src.description )
    , userId( src.userId )
    , proposalId( src.proposalId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailMatchNotice::~CEMailMatchNotice()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailMatchNotice&
CEMailMatchNotice::operator=( const CEMailMatchNotice& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        revision = src.revision;
        code = src.code;
        description = src.description;
        userId = src.userId;
        proposalId = src.proposalId;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailMatchNotice::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailMatchNotice( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CEMailMatchNotice::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailMatchNotice::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchNotice";
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
