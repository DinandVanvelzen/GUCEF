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


#include "matchprovidermail_CEMailInboxMessage.h"

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

CEMailInboxMessage::CEMailInboxMessage( void )
    : CORE::CICloneable()
    , incoming()
    , message()
    , rawSize( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailInboxMessage::CEMailInboxMessage( const CEMailInboxMessage& src )
    : CORE::CICloneable( src )
    , incoming( src.incoming )
    , message( src.message )
    , rawSize( src.rawSize )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailInboxMessage::~CEMailInboxMessage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailInboxMessage&
CEMailInboxMessage::operator=( const CEMailInboxMessage& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        incoming = src.incoming;
        message = src.message;
        rawSize = src.rawSize;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailInboxMessage::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailInboxMessage( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CEMailInboxMessage::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailInboxMessage::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailInboxMessage";
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
