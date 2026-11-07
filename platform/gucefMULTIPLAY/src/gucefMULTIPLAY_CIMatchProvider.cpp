/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMULTIPLAY_CIMatchProvider.h"

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

const CORE::CEvent CIMatchProvider::MatchAddedEvent = "GUCEF::MULTIPLAY::CIMatchProvider::MatchAddedEvent";
const CORE::CEvent CIMatchProvider::MatchUpdatedEvent = "GUCEF::MULTIPLAY::CIMatchProvider::MatchUpdatedEvent";
const CORE::CEvent CIMatchProvider::MatchRemovedEvent = "GUCEF::MULTIPLAY::CIMatchProvider::MatchRemovedEvent";
const CORE::CEvent CIMatchProvider::SubmitResultEvent = "GUCEF::MULTIPLAY::CIMatchProvider::SubmitResultEvent";
const CORE::CEvent CIMatchProvider::ProviderStateEvent = "GUCEF::MULTIPLAY::CIMatchProvider::ProviderStateEvent";
const CORE::CEvent CIMatchProvider::ProviderErrorEvent = "GUCEF::MULTIPLAY::CIMatchProvider::ProviderErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
MatchOperationStatusToString( TMatchOperationStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case MATCHOPERATION_OK:         return "OK";
        case MATCHOPERATION_STALE:      return "STALE";
        case MATCHOPERATION_REJECTED:   return "REJECTED";
        case MATCHOPERATION_NOT_FOUND:  return "NOT_FOUND";
        default:                        return "OK";
    }
}

/*-------------------------------------------------------------------------*/

void
CIMatchProvider::RegisterEvents( void )
{GUCEF_TRACE;

    MatchAddedEvent.Initialize();
    MatchUpdatedEvent.Initialize();
    MatchRemovedEvent.Initialize();
    SubmitResultEvent.Initialize();
    ProviderStateEvent.Initialize();
    ProviderErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIMatchProvider::CIMatchProvider( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIMatchProvider::~CIMatchProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::HasCapability( TMatchCapabilities capability ) const
{GUCEF_TRACE;

    return 0 != ( GetCapabilities() & (UInt32) capability );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Invite( const CORE::CString& matchId, const CMatchParticipant& participant, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Respond( const CORE::CString& matchId, const CMatchAcceptance& acceptance, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Resign( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Nudge( const CORE::CString& matchId, const CORE::CString& userId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Remove( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Rematch( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Propose( const CORE::CString& matchId, const CMatchProposal& proposal, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Vote( const CORE::CString& matchId, const CMatchVote& vote, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportMatchAdded( const CMatchUpdate& update )
{GUCEF_TRACE;

    CMatchUpdate eData( update );
    eData.providerName = GetProviderName();
    return NotifyObservers( MatchAddedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportMatchUpdated( const CMatchUpdate& update )
{GUCEF_TRACE;

    CMatchUpdate eData( update );
    eData.providerName = GetProviderName();
    return NotifyObservers( MatchUpdatedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportMatchRemoved( const CORE::CString& matchId )
{GUCEF_TRACE;

    TMatchRemovedEventData eData( matchId );
    return NotifyObservers( MatchRemovedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportSubmitResult( const CMatchOperationResult& result )
{GUCEF_TRACE;

    CMatchOperationResult eData( result );
    return NotifyObservers( SubmitResultEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportProviderState( bool online, const CORE::CString& description )
{GUCEF_TRACE;

    CMatchProviderState eData;
    eData.online = online;
    eData.providerName = GetProviderName();
    eData.description = description;
    return NotifyObservers( ProviderStateEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportProviderError( const CORE::CString& description )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MatchProvider(" << GetProviderName() << "): " << description );
    TProviderErrorEventData eData( description );
    return NotifyObservers( ProviderErrorEvent, &eData );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
