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

#include "matchprovidermail_CEMailDeliveryEvent.h"

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

const char*
MailDeliveryStateToString( TMailDeliveryState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case MAILDELIVERY_QUEUED:       return "QUEUED";
        case MAILDELIVERY_SENT:         return "SENT";
        case MAILDELIVERY_DELIVERED:    return "DELIVERED";
        case MAILDELIVERY_FAILED:       return "FAILED";
        default:                        return "NONE";
    }
}

/*-------------------------------------------------------------------------*/

CEMailDeliveryEvent::CEMailDeliveryEvent( void )
    : CORE::CICloneable()
    , matchId()
    , revision( 0 )
    , kind( MAILKIND_TURN )
    , state( MAILDELIVERY_NONE )
    , description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailDeliveryEvent::CEMailDeliveryEvent( const CEMailDeliveryEvent& src )
    : CORE::CICloneable( src )
    , matchId( src.matchId )
    , revision( src.revision )
    , kind( src.kind )
    , state( src.state )
    , description( src.description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailDeliveryEvent::~CEMailDeliveryEvent()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailDeliveryEvent&
CEMailDeliveryEvent::operator=( const CEMailDeliveryEvent& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        revision = src.revision;
        kind = src.kind;
        state = src.state;
        description = src.description;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailDeliveryEvent::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailDeliveryEvent( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CEMailDeliveryEvent::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailDeliveryEvent::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailDeliveryEvent";
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
