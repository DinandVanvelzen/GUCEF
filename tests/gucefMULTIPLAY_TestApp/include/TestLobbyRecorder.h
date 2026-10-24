/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTLOBBYRECORDER_H
#define GUCEF_MULTIPLAY_TESTAPP_TESTLOBBYRECORDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#include "gucefMULTIPLAY_CILobby.h"
#define GUCEF_MULTIPLAY_CILOBBY_H
#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Records what a lobby tells its member */
class CLobbyRecorder : public GUCEF::CORE::CObserver
{
    public:

    GUCEF::UInt32 stateChanges;
    GUCEF::UInt32 joined;
    GUCEF::UInt32 left;
    GUCEF::UInt32 votesStarted;
    GUCEF::UInt32 votesEnded;
    GUCEF::UInt32 launches;
    GUCEF::UInt32 closed;
    GUCEF::UInt32 refusals;
    bool lastVotePassed;
    GUCEF::MULTIPLAY::CLobbyVote lastVote;
    GUCEF::vector< GUCEF::Int32 > countdowns;
    GUCEF::MULTIPLAY::CLaunchDescription launch;
    GUCEF::CORE::CString closeReason;
    GUCEF::CORE::CString refusedWhat;
    GUCEF::CORE::CString refusedReason;
    GUCEF::CORE::CString::StringSet changedPaths;

    CLobbyRecorder( void )
        : GUCEF::CORE::CObserver()
        , stateChanges( 0 ), joined( 0 ), left( 0 ), votesStarted( 0 ), votesEnded( 0 ), launches( 0 ), closed( 0 ), refusals( 0 )
        , lastVotePassed( false ), lastVote(), countdowns(), launch(), closeReason(), refusedWhat(), refusedReason(), changedPaths()
    {
    }

    void Watch( GUCEF::MULTIPLAY::CILobbyPtr lobby )
    {
        if ( !lobby.IsNULL() )
            SubscribeTo( lobby.GetPointerAlways() );
    }

    bool SawCountdown( GUCEF::Int32 value ) const
    {
        for ( size_t i=0; i<countdowns.size(); ++i )
            if ( countdowns[ i ] == value )
                return true;
        return false;
    }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CLobbyRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF::MULTIPLAY::CILobby::LobbyStateChangedEvent == eventId )
        {
            ++stateChanges;
            const GUCEF::MULTIPLAY::CILobby::TLobbyStateChangedEventData* paths = static_cast< const GUCEF::MULTIPLAY::CILobby::TLobbyStateChangedEventData* >( eventData );
            for ( size_t i=0; i<paths->GetData().size(); ++i )
                changedPaths.insert( paths->GetData()[ i ] );
        }
        else
        if ( GUCEF::MULTIPLAY::CILobby::MemberJoinedEvent == eventId ) ++joined;
        else
        if ( GUCEF::MULTIPLAY::CILobby::MemberLeftEvent == eventId ) ++left;
        else
        if ( GUCEF::MULTIPLAY::CILobby::VoteStartedEvent == eventId )
        {
            ++votesStarted;
            lastVote = static_cast< const GUCEF::MULTIPLAY::CLobbyVoteEventData* >( eventData )->vote;
        }
        else
        if ( GUCEF::MULTIPLAY::CILobby::VoteEndedEvent == eventId )
        {
            ++votesEnded;
            lastVotePassed = static_cast< const GUCEF::MULTIPLAY::CLobbyVoteEventData* >( eventData )->passed;
        }
        else
        if ( GUCEF::MULTIPLAY::CILobby::CountdownEvent == eventId )
        {
            countdowns.push_back( static_cast< const GUCEF::MULTIPLAY::CILobby::TCountdownEventData* >( eventData )->GetData() );
        }
        else
        if ( GUCEF::MULTIPLAY::CILobby::LaunchEvent == eventId )
        {
            ++launches;
            launch = *static_cast< const GUCEF::MULTIPLAY::CLaunchDescription* >( eventData );
        }
        else
        if ( GUCEF::MULTIPLAY::CILobby::LobbyClosedEvent == eventId )
        {
            ++closed;
            closeReason = static_cast< const GUCEF::MULTIPLAY::CILobby::TLobbyClosedEventData* >( eventData )->GetData();
        }
        else
        if ( GUCEF::MULTIPLAY::CILobby::RequestRefusedEvent == eventId )
        {
            ++refusals;
            const GUCEF::MULTIPLAY::CLobbyRefusalEventData* refusal = static_cast< const GUCEF::MULTIPLAY::CLobbyRefusalEventData* >( eventData );
            refusedWhat = refusal->what;
            refusedReason = refusal->reason;
        }
    }
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTLOBBYRECORDER_H ? */
