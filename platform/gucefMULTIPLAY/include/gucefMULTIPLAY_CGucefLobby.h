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

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBY_H
#define GUCEF_MULTIPLAY_CGUCEFLOBBY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#include "gucefMULTIPLAY_CILobby.h"
#define GUCEF_MULTIPLAY_CILOBBY_H
#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CCHATMESSAGE_H
#include "gucefMULTIPLAY_CChatMessage.h"
#define GUCEF_MULTIPLAY_CCHATMESSAGE_H
#endif /* GUCEF_MULTIPLAY_CCHATMESSAGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

namespace DRN { class CDRNSession; class CDRNDataGrid; class CDRNDataStream; }

namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A lobby of the built-in `gucef` lobby server, as a member sees it: the member's own DRN node joined to the
 *  lobby session with the token the directory gave. The state is read from the grids of the session, requests go
 *  out as writes the server's rules check, as proposals put to a vote and as commands; refusals come back as
 *  RequestRefusedEvent. CGucefLobbyService makes these.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGucefLobby : public CILobby
{
    public:

    static const CORE::CEvent LobbyReadyEvent;      /**< the session is joined and the whole state arrived */
    static const CORE::CEvent LobbyFailedEvent;     /**< TCloneableString: joining failed */
    static const CORE::CEvent ChatEntryEvent;       /**< CChatMessage: a message of the lobby chat */

    static void RegisterEvents( void );

    CGucefLobby( const CORE::CString& lobbyId, const CORE::CString& localMemberId );

    virtual ~CGucefLobby();

    /** Joins the lobby session at the endpoints with the token */
    bool Start( const DRN::CDRNEndpointList& endpoints, const CORE::CString& token, bool asSpectator );

    bool IsReady( void ) const;

    bool IsMember( void ) const;

    const CORE::CString& GetLobbyId( void ) const;

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

    virtual bool Ban( const CORE::CString& memberId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ReportPlayer( const CORE::CString& memberId, const CORE::CString& reason ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool TransferBoss( const CORE::CString& memberId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetSlotKind( UInt32 slotIndex, TLobbySlotKind kind, const CORE::CString& aiType ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RequestStart( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ForceStart( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Leave( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Says something in the lobby chat */
    bool SendChat( const CORE::CString& text );

    /** The node the member uses for the lobby */
    DRN::CDRNNode& GetNode( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGucefLobby( const CGucefLobby& src );
    CGucefLobby& operator=( const CGucefLobby& src );

    bool SendCommand( const CORE::CString& verb, const CORE::CVariant* arg1 = GUCEF_NULL, const CORE::CVariant* arg2 = GUCEF_NULL, const CORE::CVariant* arg3 = GUCEF_NULL );
    bool WriteMemberKey( const CORE::CString& field, const CORE::CVariant& value );
    bool Propose( UInt32 gridIndex, DRN::CDRNDataGrid* grid, const CORE::CString& key, const CORE::CVariant& value );
    void OnJoined( void );
    void FindItems( void );
    void Rebuild( void );
    void CheckReady( void );
    void UpdateProposalsSubscription( void );
    void OnAppMessage( const CORE::CDynamicBuffer& payload );
    void OnChatEntries( DRN::CDRNDataStream& stream );
    void OnProposeResult( UInt32 gridIndex, UInt32 proposalId, bool accepted, UInt32 outcome, const CORE::CString& reason );
    void Detach( const CORE::CString* closeReason );
    static CORE::CString WhatOf( UInt32 gridIndex, const CORE::CString& key );

    private:

    DRN::CDRNNode m_node;
    DRN::CDRNSession* m_session;
    DRN::CDRNDataGrid* m_lobbyGrid;
    DRN::CDRNDataGrid* m_membersGrid;
    DRN::CDRNDataGrid* m_slotsGrid;
    DRN::CDRNDataGrid* m_schemaGrid;
    DRN::CDRNDataGrid* m_proposalsGrid;
    DRN::CDRNDataStream* m_chat;
    DRN::CDRNDataStream* m_commands;
    CORE::CString m_lobbyId;
    CORE::CString m_localMemberId;
    CLobbyState m_state;
    CLobbySettingsSchema m_schema;
    Int32 m_countdown;
    typedef GUCEF::map< UInt32, CORE::CString > TProposalKeyMap;
    struct SProposals
    {
        UInt32 nextId;              /**< the id the grid gives its next proposal, they count from 1 */
        TProposalKeyMap keys;
    };
    typedef struct SProposals TProposals;
    TProposals m_proposals[ 3 ];   /**< the lobby, members and proposals grids */
    bool m_joined;
    bool m_ready;
    bool m_attached;
    bool m_subscribedToProposals;
    bool m_asSpectator;
    UInt32 m_startCount;
    CORE::CString m_lastEndedVoteId;
    CORE::CString::StringSet m_startedVoteIds;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBY_H ? */
