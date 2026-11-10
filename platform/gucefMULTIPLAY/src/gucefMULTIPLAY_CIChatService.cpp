/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefMULTIPLAY_CIChatService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIChatService::ChatMessageEvent = "GUCEF::MULTIPLAY::CIChatService::ChatMessageEvent";
const CORE::CEvent CIChatService::ChatPresenceEvent = "GUCEF::MULTIPLAY::CIChatService::ChatPresenceEvent";
const CORE::CEvent CIChatService::ChatTopicEvent = "GUCEF::MULTIPLAY::CIChatService::ChatTopicEvent";
const CORE::CEvent CIChatService::ChatErrorEvent = "GUCEF::MULTIPLAY::CIChatService::ChatErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIChatChannel::CIChatChannel( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIChatChannel::~CIChatChannel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChatChannel::CChatChannel( void )
    : CIChatChannel()
    , name()
    , topic()
    , members()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChatChannel::CChatChannel( const CORE::CString& channelName )
    : CIChatChannel()
    , name( channelName )
    , topic()
    , members()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChatChannel::~CChatChannel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CChatChannel::GetName( void ) const
{GUCEF_TRACE;

    return name;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CChatChannel::GetTopic( void ) const
{GUCEF_TRACE;

    return topic;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CChatChannel::GetMembers( void ) const
{GUCEF_TRACE;

    return members;
}

/*-------------------------------------------------------------------------*/

CChatPresenceEventData::CChatPresenceEventData( void )
    : CORE::CICloneable()
    , channel()
    , user()
    , presence( PRESENCE_JOINED )
    , reason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChatPresenceEventData::CChatPresenceEventData( const CORE::CString& channelName ,
                                                const CORE::CString& userName    ,
                                                TPresence newPresence            ,
                                                const CORE::CString& why         )
    : CORE::CICloneable()
    , channel( channelName )
    , user( userName )
    , presence( newPresence )
    , reason( why )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChatPresenceEventData::~CChatPresenceEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CChatPresenceEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CChatPresenceEventData( *this );
}

/*-------------------------------------------------------------------------*/

CChatTopicEventData::CChatTopicEventData( void )
    : CORE::CICloneable()
    , channel()
    , topic()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChatTopicEventData::CChatTopicEventData( const CORE::CString& channelName, const CORE::CString& channelTopic )
    : CORE::CICloneable()
    , channel( channelName )
    , topic( channelTopic )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChatTopicEventData::~CChatTopicEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CChatTopicEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CChatTopicEventData( *this );
}

/*-------------------------------------------------------------------------*/

void
CIChatService::RegisterEvents( void )
{GUCEF_TRACE;

    ChatMessageEvent.Initialize();
    ChatPresenceEvent.Initialize();
    ChatTopicEvent.Initialize();
    ChatErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIChatService::CIChatService( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIChatService::~CIChatService()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
