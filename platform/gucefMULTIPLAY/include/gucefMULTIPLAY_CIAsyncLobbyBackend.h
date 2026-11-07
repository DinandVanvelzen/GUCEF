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

#ifndef GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H
#define GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSTATE_H
#include "gucefMULTIPLAY_CLobbyState.h"
#define GUCEF_MULTIPLAY_CLOBBYSTATE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSTATE_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#include "gucefMULTIPLAY_CLobbySettingsSchema.h"
#define GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYPRESENTATION_H
#include "gucefMULTIPLAY_CLobbyPresentation.h"
#define GUCEF_MULTIPLAY_CLOBBYPRESENTATION_H
#endif /* GUCEF_MULTIPLAY_CLOBBYPRESENTATION_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#include "gucefMULTIPLAY_CLobbyCreateParams.h"
#define GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#endif /* GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#include "gucefMULTIPLAY_CGameListingEntry.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

#ifndef GUCEF_MULTIPLAY_CASYNCLOBBYVIEW_H
#include "gucefMULTIPLAY_CAsyncLobbyView.h"
#define GUCEF_MULTIPLAY_CASYNCLOBBYVIEW_H
#endif /* GUCEF_MULTIPLAY_CASYNCLOBBYVIEW_H ? */

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
 *  What a lobby service whose members answer slowly needs from the transport under it, for example a mailbox: the rows
 *  of its games and lobbies, a snapshot of one lobby for the local player, and the actions the lobby offers. The backend
 *  knows the rules and the protocol; the classes over it (CAsyncLobbyService, CAsyncLobby, CAsyncLobbyListingProvider)
 *  make a lobby service, lobbies and a listing provider of it, so that the screens of a normal multiplayer game show it.
 *
 *  Every action returns whether it was made; when it was not, 'error' says why in plain words for the player. A backend
 *  tells that its rows changed (ListingsChangedEvent) and that one lobby changed (LobbyChangedEvent, the lobby id).
 *  The ids of the lobbies are the ids of the rows (CGameListingEntry::listingId).
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CIAsyncLobbyBackend : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ListingsChangedEvent;     /**< no data */
    static const CORE::CEvent LobbyChangedEvent;        /**< TLobbyEventData: the id of the lobby that changed */

    typedef CORE::TCloneableString                      TLobbyEventData;
    typedef GUCEF::vector< CGameListingEntry >          TGameListingEntryVector;

    static void RegisterEvents( void );

    CIAsyncLobbyBackend( void );
    virtual ~CIAsyncLobbyBackend();

    /** The name of the backend, shown as the source of its rows */
    virtual const CORE::CString& GetBackendName( void ) const = 0;

    /** Whether the backend can be used now; when it cannot, 'reason' says why in plain words */
    virtual bool IsAvailable( CORE::CString& reason ) const = 0;

    /** The rows of all games and lobbies the local player has to do with */
    virtual void GetListings( TGameListingEntryVector& listings ) const = 0;

    /** The lobby with the id; false when there is none */
    virtual bool GetLobby( const CORE::CString& lobbyId, CAsyncLobbyView& view ) const = 0;

    /** Asks the source again now */
    virtual bool Refresh( void ) = 0;

    /** Makes a lobby; 'lobbyId' is the id of the new one */
    virtual bool CreateLobby( const CLobbyCreateParams& params, CORE::CString& lobbyId, CORE::CString& error ) = 0;

    /** The local player gives the answer to the invitation ('accept'); an answer that was given may be refused to be taken back */
    virtual bool Answer( const CORE::CString& lobbyId, bool accept, CORE::CString& error ) = 0;
    virtual bool SetSpectating( const CORE::CString& lobbyId, bool spectate, CORE::CString& error ) = 0;
    virtual bool Kick( const CORE::CString& lobbyId, const CORE::CString& memberId, CORE::CString& error ) = 0;
    virtual bool Start( const CORE::CString& lobbyId, CORE::CString& error ) = 0;
    virtual bool ForceStart( const CORE::CString& lobbyId, CORE::CString& error ) = 0;
    /** The local player leaves; for the one who made the lobby that closes it. Also dismisses a lobby that is over */
    virtual bool Leave( const CORE::CString& lobbyId, CORE::CString& error ) = 0;
    virtual bool Vote( const CORE::CString& lobbyId, const CORE::CString& voteId, bool yes, CORE::CString& error ) = 0;
    virtual bool Invite( const CORE::CString& lobbyId, const CORE::CString& name, const CORE::CString& contact, const CORE::CString& code, CORE::CString& error ) = 0;

    /** The local player closes the lobby for everybody, or proposes to (the members decide by vote); the lobby stays in the list of the player. The default: not offered */
    virtual bool Cancel( const CORE::CString& lobbyId, CORE::CString& error );

    protected:

    /** @return false if this object was destroyed by an observer of the event */
    bool NotifyListingsChanged( void );
    bool NotifyLobbyChanged( const CORE::CString& lobbyId );

    private:

    CIAsyncLobbyBackend( const CIAsyncLobbyBackend& src );
    CIAsyncLobbyBackend& operator=( const CIAsyncLobbyBackend& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H ? */
