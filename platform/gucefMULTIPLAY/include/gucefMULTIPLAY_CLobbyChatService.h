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

#ifndef GUCEF_MULTIPLAY_CLOBBYCHATSERVICE_H
#define GUCEF_MULTIPLAY_CLOBBYCHATSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CICHATSERVICE_H
#include "gucefMULTIPLAY_CIChatService.h"
#define GUCEF_MULTIPLAY_CICHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CICHATSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#include "gucefMULTIPLAY_CILobbyService.h"
#define GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CILOBBYSERVICE_H ? */

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

class CGucefLobbyService;

/**
 *  Chat over the built-in lobby server, registered as "lobby":
 *      the chat of a lobby is the channel "lobby:<lobby id>", the chat stream of the lobby session of an attached
 *      CGucefLobby;
 *      other channels are global ones of the server's directory session, with their last messages kept for those
 *      who join later;
 *      direct messages go through the directory to the user with that name or user id.
 *  The directory connection is that of a `gucef` lobby service given with SetLobbyService(), or one of its own made
 *  from the config.
 *
 *  Config: server, password (as CGucefLobbyService)
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyChatService : public CIChatService
{
    public:

    static const CORE::CString ServiceName;         /**< "lobby" */
    static const CORE::CString LobbyChannelPrefix;  /**< "lobby:" */

    CLobbyChatService( void );

    virtual ~CLobbyChatService();

    /** Uses the directory connection of this `gucef` lobby service */
    bool SetLobbyService( CILobbyServicePtr service );

    /** Makes the chat of the lobby the channel "lobby:<lobby id>", the lobby is a CGucefLobby */
    bool AttachLobby( CILobbyPtr lobby );

    virtual bool Connect( const CPlayerIdentity& identity, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool JoinChannel( const CORE::CString& channel ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LeaveChannel( const CORE::CString& channel ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetChannels( CORE::CString::StringVector& channels ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CIChatChannel* GetChannel( const CORE::CString& channel ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendToChannel( const CORE::CString& channel, const CORE::CString& text ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendDirect( const CORE::CString& user, const CORE::CString& text ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetLocalUserName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetServiceName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CORE::CString, CChatChannel > TChannelMap;

    CLobbyChatService( const CLobbyChatService& src );
    CLobbyChatService& operator=( const CLobbyChatService& src );

    CGucefLobbyService* GetGucefService( void ) const;
    bool IsLobbyChannel( const CORE::CString& channel ) const;
    void UpdateLobbyMembers( void );

    private:

    CILobbyServicePtr m_service;
    CILobbyPtr m_lobby;
    CORE::CDataNode m_serviceConfig;
    CORE::CString m_localUserName;
    TChannelMap m_channels;
    UInt32 m_messageCount;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CIChatService, CLobbyChatService, MT::CMutex > TLobbyChatServiceFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CLOBBYCHATSERVICE_H ? */
