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

#include "gucefMULTIPLAY_CILobby.h"

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

const CORE::CEvent CILobby::LobbyStateChangedEvent = "GUCEF::MULTIPLAY::CILobby::LobbyStateChangedEvent";
const CORE::CEvent CILobby::MemberJoinedEvent = "GUCEF::MULTIPLAY::CILobby::MemberJoinedEvent";
const CORE::CEvent CILobby::MemberLeftEvent = "GUCEF::MULTIPLAY::CILobby::MemberLeftEvent";
const CORE::CEvent CILobby::VoteStartedEvent = "GUCEF::MULTIPLAY::CILobby::VoteStartedEvent";
const CORE::CEvent CILobby::VoteEndedEvent = "GUCEF::MULTIPLAY::CILobby::VoteEndedEvent";
const CORE::CEvent CILobby::CountdownEvent = "GUCEF::MULTIPLAY::CILobby::CountdownEvent";
const CORE::CEvent CILobby::LaunchEvent = "GUCEF::MULTIPLAY::CILobby::LaunchEvent";
const CORE::CEvent CILobby::LobbyClosedEvent = "GUCEF::MULTIPLAY::CILobby::LobbyClosedEvent";
const CORE::CEvent CILobby::RequestRefusedEvent = "GUCEF::MULTIPLAY::CILobby::RequestRefusedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLobbyVoteEventData::CLobbyVoteEventData( void )
    : CORE::CICloneable()
    , vote()
    , passed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyVoteEventData::CLobbyVoteEventData( const CLobbyVote& voteData, bool votePassed )
    : CORE::CICloneable()
    , vote( voteData )
    , passed( votePassed )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyVoteEventData::~CLobbyVoteEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CLobbyVoteEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CLobbyVoteEventData( *this );
}

/*-------------------------------------------------------------------------*/

CLobbyRefusalEventData::CLobbyRefusalEventData( void )
    : CORE::CICloneable()
    , what()
    , reason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyRefusalEventData::CLobbyRefusalEventData( const CORE::CString& refusedWhat, const CORE::CString& refusalReason )
    : CORE::CICloneable()
    , what( refusedWhat )
    , reason( refusalReason )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyRefusalEventData::~CLobbyRefusalEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CLobbyRefusalEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CLobbyRefusalEventData( *this );
}

/*-------------------------------------------------------------------------*/

void
CILobby::RegisterEvents( void )
{GUCEF_TRACE;

    LobbyStateChangedEvent.Initialize();
    MemberJoinedEvent.Initialize();
    MemberLeftEvent.Initialize();
    VoteStartedEvent.Initialize();
    VoteEndedEvent.Initialize();
    CountdownEvent.Initialize();
    LaunchEvent.Initialize();
    LobbyClosedEvent.Initialize();
    RequestRefusedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CILobby::CILobby( void )
    : CORE::CObservingNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CILobby::~CILobby()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CILobby::IsBoss( void ) const
{GUCEF_TRACE;

    const CORE::CString& localMemberId = GetLocalMemberId();
    return !localMemberId.IsNULLOrEmpty() && GetState().bossId == localMemberId;
}

/*-------------------------------------------------------------------------*/

bool
CILobby::Ban( const CORE::CString& /* memberId */ )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CILobby::ReportPlayer( const CORE::CString& /* memberId */, const CORE::CString& /* reason */ )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
