/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_GAMEGUI_TESTAPP_TESTFAKEASYNCLOBBYBACKEND_H
#define GUCEF_GAMEGUI_TESTAPP_TESTFAKEASYNCLOBBYBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H
#include "gucefMULTIPLAY_CIAsyncLobbyBackend.h"
#define GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H
#endif /* GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A backend of lobbies whose members answer slowly, for tests of what is made over it: the rows and lobbies are set by the
 *  test, every action is recorded as a line ("answer <id> yes") and refused with 'refusal' when that is not empty. Used by the tests
 *  of the lobby service and by the tests of the forms that show it, which is why it is here and not in the common test headers.
 */
class CFakeAsyncLobbyBackend : public GUCEF::MULTIPLAY::CIAsyncLobbyBackend
{
    public:

    GUCEF::CORE::CString name;
    bool available;
    GUCEF::CORE::CString unavailableReason;
    GUCEF::CORE::CString refusal;                           /**< not empty: every action fails with this reason */
    GUCEF::vector< GUCEF::MULTIPLAY::CGameListingEntry > rows;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::MULTIPLAY::CAsyncLobbyView > lobbies;
    GUCEF::vector< GUCEF::CORE::CString > calls;
    GUCEF::MULTIPLAY::CLobbyCreateParams lastCreate;
    GUCEF::CORE::CString nextLobbyId;
    GUCEF::UInt32 refreshes;

    CFakeAsyncLobbyBackend( void )
        : GUCEF::MULTIPLAY::CIAsyncLobbyBackend()
        , name( "fake" ), available( true ), unavailableReason(), refusal(), rows(), lobbies(), calls(), lastCreate(), nextLobbyId( "made-1" ), refreshes( 0 )
    {
    }

    virtual const GUCEF::CORE::CString& GetBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE { return name; }

    virtual bool IsAvailable( GUCEF::CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !available )
            reason = unavailableReason;
        return available;
    }

    virtual void GetListings( TGameListingEntryVector& listings ) const GUCEF_VIRTUAL_OVERRIDE { listings = rows; }

    virtual bool GetLobby( const GUCEF::CORE::CString& lobbyId, GUCEF::MULTIPLAY::CAsyncLobbyView& view ) const GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::map< GUCEF::CORE::CString, GUCEF::MULTIPLAY::CAsyncLobbyView >::const_iterator found = lobbies.find( lobbyId );
        if ( found == lobbies.end() )
            return false;
        view = (*found).second;
        return true;
    }

    virtual bool Refresh( void ) GUCEF_VIRTUAL_OVERRIDE { ++refreshes; return true; }

    virtual bool CreateLobby( const GUCEF::MULTIPLAY::CLobbyCreateParams& params, GUCEF::CORE::CString& lobbyId, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        calls.push_back( "create " + params.name );
        lastCreate = params;
        if ( !refusal.IsNULLOrEmpty() ) { error = refusal; return false; }
        lobbyId = nextLobbyId;
        GUCEF::MULTIPLAY::CAsyncLobbyView view;
        view.state.lobbyId = lobbyId;
        view.state.name = params.name;
        view.localMemberId = "me";
        GUCEF::MULTIPLAY::CLobbyMember me;
        me.id = "me";
        me.name = "Me";
        view.state.members.push_back( me );
        view.state.bossId = "me";
        lobbies[ lobbyId ] = view;
        return true;
    }

    virtual bool Answer( const GUCEF::CORE::CString& lobbyId, bool accept, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE { return Record( "answer " + lobbyId + ( accept ? " yes" : " no" ), error ); }
    virtual bool SetSpectating( const GUCEF::CORE::CString& lobbyId, bool spectate, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE { return Record( "spectate " + lobbyId + ( spectate ? " yes" : " no" ), error ); }
    virtual bool Kick( const GUCEF::CORE::CString& lobbyId, const GUCEF::CORE::CString& memberId, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE { return Record( "kick " + lobbyId + " " + memberId, error ); }
    virtual bool Start( const GUCEF::CORE::CString& lobbyId, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE { return Record( "start " + lobbyId, error ); }
    virtual bool ForceStart( const GUCEF::CORE::CString& lobbyId, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE { return Record( "forcestart " + lobbyId, error ); }
    virtual bool Leave( const GUCEF::CORE::CString& lobbyId, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE { return Record( "leave " + lobbyId, error ); }
    virtual bool Cancel( const GUCEF::CORE::CString& lobbyId, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE { return Record( "cancel " + lobbyId, error ); }
    virtual bool Vote( const GUCEF::CORE::CString& lobbyId, const GUCEF::CORE::CString& voteId, bool yes, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE { return Record( "vote " + lobbyId + " " + voteId + ( yes ? " yes" : " no" ), error ); }
    virtual bool Invite( const GUCEF::CORE::CString& lobbyId, const GUCEF::CORE::CString& who, const GUCEF::CORE::CString& contact, const GUCEF::CORE::CString& code, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        return Record( "invite " + lobbyId + " " + who + " " + contact + " " + code, error );
    }

    /** A lobby changed at the source: set its snapshot and tell */
    void ChangeLobby( const GUCEF::CORE::CString& lobbyId, const GUCEF::MULTIPLAY::CAsyncLobbyView& view )
    {
        lobbies[ lobbyId ] = view;
        NotifyLobbyChanged( lobbyId );
    }

    void ChangeRows( void ) { NotifyListingsChanged(); }

    bool Called( const GUCEF::CORE::CString& line ) const
    {
        for ( size_t i=0; i<calls.size(); ++i )
            if ( calls[ i ] == line )
                return true;
        return false;
    }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CFakeAsyncLobbyBackend";
        return classTypeName;
    }

    private:

    bool Record( const GUCEF::CORE::CString& line, GUCEF::CORE::CString& error )
    {
        calls.push_back( line );
        if ( !refusal.IsNULLOrEmpty() )
        {
            error = refusal;
            return false;
        }
        return true;
    }
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_TESTAPP_TESTFAKEASYNCLOBBYBACKEND_H ? */
