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

#ifndef GUCEF_MULTIPLAY_CIRCCHATSERVICE_H
#define GUCEF_MULTIPLAY_CIRCCHATSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_COM_CIRCCLIENT_H
#include "gucefCOM_CIrcClient.h"
#define GUCEF_COM_CIRCCLIENT_H
#endif /* GUCEF_COM_CIRCCLIENT_H ? */

#ifndef GUCEF_COM_CIRCCLIENTSETTINGS_H
#include "gucefCOM_CIrcClientSettings.h"
#define GUCEF_COM_CIRCCLIENTSETTINGS_H
#endif /* GUCEF_COM_CIRCCLIENTSETTINGS_H ? */

#ifndef GUCEF_MULTIPLAY_CICHATSERVICE_H
#include "gucefMULTIPLAY_CIChatService.h"
#define GUCEF_MULTIPLAY_CICHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CICHATSERVICE_H ? */

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
 *  Chat over IRC with COM's CIrcClient.
 *
 *  A channel "name" of the application is the IRC channel "#<prefix>-<name>" (just "#<name>" without a prefix),
 *  so the channels of a game do not mix with others on a shared server; a name that starts with '#' is used as is.
 *  Direct messages are a PRIVMSG to the nick. Presence comes from JOIN, PART, KICK and QUIT.
 *  The nick is the one in the IRC settings, or else made from the display name of the identity.
 *  Without the echo-message capability the service reports the messages the local user sends itself.
 *
 *  Config: channelPrefix (default "gucef") and an "Irc" child with the CIrcClientSettings
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CIrcChatService : public CIChatService
{
    public:

    static const CORE::CString ServiceName;         /**< "irc" */
    static const CORE::CString DefaultChannelPrefix; /**< "gucef" */

    CIrcChatService( void );

    explicit CIrcChatService( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CIrcChatService();

    virtual bool Connect( const CPlayerIdentity& identity, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the server accepted the user */
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool JoinChannel( const CORE::CString& channel ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LeaveChannel( const CORE::CString& channel ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetChannels( CORE::CString::StringVector& channels ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CIChatChannel* GetChannel( const CORE::CString& channel ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendToChannel( const CORE::CString& channel, const CORE::CString& text ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendDirect( const CORE::CString& user, const CORE::CString& text ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetLocalUserName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetServiceName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetIrcSettings( const COM::CIrcClientSettings& settings );

    const COM::CIrcClientSettings& GetIrcSettings( void ) const;

    void SetChannelPrefix( const CORE::CString& prefix );

    const CORE::CString& GetChannelPrefix( void ) const;

    /** The IRC channel of an application channel name */
    CORE::CString ToIrcChannel( const CORE::CString& channel ) const;

    /** The application channel name of an IRC channel: without the prefix when it has it, else the IRC name */
    CORE::CString FromIrcChannel( const CORE::CString& ircChannel ) const;

    /** A nick IRC accepts made from a display name */
    static CORE::CString MakeNick( const CORE::CString& displayName );

    COM::CIrcClient& GetIrcClient( void );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CORE::CString, CChatChannel > TChannelMap;

    CIrcChatService( const CIrcChatService& src );
    CIrcChatService& operator=( const CIrcChatService& src );

    void RegisterEventHandlers( void );
    CChatChannel* FindChannel( const CORE::CString& ircChannel );
    static bool IsChannelName( const CORE::CString& target );
    static CORE::CString FoldName( const CORE::CString& name );
    bool EchoesOwnMessages( void ) const;

    bool OnRegistered( void );
    bool OnDisconnected( void );
    bool OnMessage( const COM::CIrcMessageEventData& data );
    bool OnJoin( const COM::CIrcMessageEventData& data );
    bool OnPart( const COM::CIrcMessageEventData& data );
    bool OnQuit( const COM::CIrcMessageEventData& data );
    bool OnNickChanged( const COM::CIrcMessageEventData& data );
    bool OnTopic( const COM::CIrcMessageEventData& data );
    bool OnNames( const COM::CIrcMessageEventData& data );
    bool OnError( const COM::CIrcMessageEventData* data );

    private:

    COM::CIrcClient m_client;
    COM::CIrcClientSettings m_settings;
    CORE::CString m_channelPrefix;
    CORE::CFutureResultSource m_connectResult;
    bool m_connectPending;
    TChannelMap m_channels;                         /**< by the folded IRC channel name */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CIChatService, CIrcChatService, MT::CMutex > TIrcChatServiceFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CIRCCHATSERVICE_H ? */
