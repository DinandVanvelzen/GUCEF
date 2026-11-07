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


#include "matchprovidermail_CEMailInboxNotice.h"

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

CEMailInboxNotice::CEMailInboxNotice( void )
    : CORE::CICloneable()
    , code()
    , description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailInboxNotice::CEMailInboxNotice( const CEMailInboxNotice& src )
    : CORE::CICloneable( src )
    , code( src.code )
    , description( src.description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailInboxNotice::~CEMailInboxNotice()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailInboxNotice&
CEMailInboxNotice::operator=( const CEMailInboxNotice& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        code = src.code;
        description = src.description;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailInboxNotice::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailInboxNotice( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CEMailInboxNotice::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailInboxNotice::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailInboxNotice";
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
