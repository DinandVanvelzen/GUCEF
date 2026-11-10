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

#ifndef GUCEF_MULTIPLAY_CASYNCLOBBY_H
#define GUCEF_MULTIPLAY_CASYNCLOBBY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#include "gucefMULTIPLAY_CILobby.h"
#define GUCEF_MULTIPLAY_CILOBBY_H
#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H
#include "gucefMULTIPLAY_CIAsyncLobbyBackend.h"
#define GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H
#endif /* GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A lobby of a CIAsyncLobbyBackend, as the lobby forms use it: the state is the snapshot the backend gives, every request
 *  of the lobby is an action of the backend, and a change of the lobby at the backend shows as the events of any lobby.
 *
 *  A request the backend refuses (or one the kind of lobby does not have: a slot, a faction, a colour, the boss) returns false
 *  and notifies RequestRefusedEvent with the reason. The lobby closes (LobbyClosedEvent, once) when the snapshot says it is over.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CAsyncLobby : public CILobby
{
    public:

    CAsyncLobby( CIAsyncLobbyBackend& backend, const CORE::CString& lobbyId );
    virtual ~CAsyncLobby();

    const CORE::CString& GetLobbyId( void ) const;

    /** Takes the snapshot of the backend again and tells what changed; done whenever the backend says the lobby changed */
    void Update( void );

    virtual const CLobbyState& GetState( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CLobbySettingsSchema& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetLocalMemberId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSetting( const CORE::CString& key, const CORE::CString& value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ProposeSetting( const CORE::CString& key, const CORE::CString& value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Vote( const CORE::CString& voteId, bool yes ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetReady( bool ready ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSpectator( bool spectate ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RequestSlot( UInt32 slotIndex ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetFaction( const CORE::CString& faction ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetColour( const CORE::CString& colour ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Kick( const CORE::CString& memberId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool TransferBoss( const CORE::CString& memberId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSlotKind( UInt32 slotIndex, TLobbySlotKind kind, const CORE::CString& aiType ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RequestStart( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ForceStart( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Leave( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Invite( const CORE::CString& name, const CORE::CString& contact, const CORE::CString& code ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Cancel( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CLobbyPresentation GetPresentation( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** Only the standard events (the backend is destroyed) arrive here, the others have their own handler */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAsyncLobby > TEventCallback;

    CAsyncLobby( const CAsyncLobby& src );
    CAsyncLobby& operator=( const CAsyncLobby& src );

    bool Refuse( const char* what, const CORE::CString& reason );
    bool Finish( const char* what, bool done, const CORE::CString& error );
    bool RefuseUnsupported( const char* what );
    void CollectChanges( const CAsyncLobbyView& before, const CAsyncLobbyView& after, CORE::CString::StringVector& paths ) const;

    void OnBackendLobbyChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    CIAsyncLobbyBackend* m_backend;
    CORE::CString m_lobbyId;
    CAsyncLobbyView m_view;
    bool m_hasView;
    bool m_closedNotified;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CASYNCLOBBY_H ? */
