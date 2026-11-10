/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CIRCCLIENT_H
#define GUCEF_COM_CIRCCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_COM_CIRCMESSAGE_H
#include "gucefCOM_CIrcMessage.h"
#define GUCEF_COM_CIRCMESSAGE_H
#endif /* GUCEF_COM_CIRCMESSAGE_H ? */

#ifndef GUCEF_COM_CIIRCTRANSPORT_H
#include "gucefCOM_CIIrcTransport.h"
#define GUCEF_COM_CIIRCTRANSPORT_H
#endif /* GUCEF_COM_CIIRCTRANSPORT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The data of every event of CIrcClient. What the fields hold per event:
 *      MessageEvent, NoticeEvent: sourceNick, target (channel or our nick), text
 *      JoinEvent: sourceNick, target (the channel)
 *      PartEvent: sourceNick (who left or was kicked), target (the channel), text (the reason)
 *      QuitEvent: sourceNick, text (the reason)
 *      NickChangedEvent: sourceNick (the old nick), text (the new nick)
 *      TopicEvent: sourceNick (who set it, empty when told on joining), target (the channel), text (the topic)
 *      NamesEvent: target (the channel), the members are in GetChannelMembers()
 *      ErrorEvent: text, the command of the message when it came from the server
 *      RawLineEvent: text (the whole line)
 *  The server time is from the server-time tag when the server sends it, the time of arrival otherwise.
 */
class GUCEF_COM_PUBLIC_CPP CIrcMessageEventData : public CORE::CICloneable
{
    public:

    CORE::CString sourceNick;
    CORE::CString target;
    CORE::CString text;
    CORE::CDateTime serverTime;
    CORE::CString msgId;
    CORE::CString account;
    CORE::CValueList tags;
    CIrcMessage message;            /**< the message this is about, empty for events that do not come from a line */

    CIrcMessageEventData( void );
    CIrcMessageEventData( const CIrcMessageEventData& src );
    virtual ~CIrcMessageEventData();
    CIrcMessageEventData& operator=( const CIrcMessageEventData& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  A generic IRCv3 client for any application: chat, lobbies, bots.
 *
 *  Registration negotiates capabilities with CAP LS 302 / CAP REQ / CAP END around NICK and USER, with SASL PLAIN
 *  when an account is set. 001 means registered. A nick in use during registration is retried with the
 *  alternatives from the settings and after those with an underscore added. PING is answered with PONG, and
 *  a silent server is pinged and given up on after the ping timeout.
 *
 *  The transport comes from the IRC transport factory of CComGlobal: "tcp", or "websocket" when the settings ask
 *  for it and gucefWEB registered it.
 */
class GUCEF_COM_PUBLIC_CPP CIrcClient : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ConnectedEvent;       /**< the transport is connected, registration starts */
    static const CORE::CEvent RegisteredEvent;      /**< the server accepted us (001) */
    static const CORE::CEvent DisconnectedEvent;
    static const CORE::CEvent MessageEvent;         /**< PRIVMSG */
    static const CORE::CEvent NoticeEvent;          /**< NOTICE */
    static const CORE::CEvent JoinEvent;
    static const CORE::CEvent PartEvent;            /**< PART and KICK */
    static const CORE::CEvent QuitEvent;
    static const CORE::CEvent NickChangedEvent;
    static const CORE::CEvent TopicEvent;
    static const CORE::CEvent NamesEvent;           /**< the member list of a channel is complete */
    static const CORE::CEvent ErrorEvent;
    static const CORE::CEvent RawLineEvent;         /**< every line from the server, before it is handled */

    static void RegisterEvents( void );

    CIrcClient( void );

    explicit CIrcClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CIrcClient();

    /** Connects and registers with the given settings */
    bool Connect( const CIrcClientSettings& settings );

    /** Sends QUIT with the message, when connected, and drops the connection */
    void Disconnect( const CORE::CString& message = CORE::CString::Empty );

    bool IsConnected( void ) const;

    bool IsRegistered( void ) const;

    bool Join( const CORE::CString& channel, const CORE::CString& key = CORE::CString::Empty );

    bool Part( const CORE::CString& channel, const CORE::CString& reason = CORE::CString::Empty );

    bool SetTopic( const CORE::CString& channel, const CORE::CString& topic );

    bool PrivMsg( const CORE::CString& target, const CORE::CString& text );

    bool Notice( const CORE::CString& target, const CORE::CString& text );

    /** Sends a line as it is, without CR LF */
    bool SendRaw( const CORE::CString& line );

    /** Sends a message */
    bool Send( const CIrcMessage& message );

    /** The channels we are in */
    void GetChannels( CORE::CString::StringVector& channels ) const;

    /** The nicks in a channel we are in, without their mode prefixes */
    bool GetChannelMembers( const CORE::CString& channel, CORE::CString::StringVector& members ) const;

    /** The mode prefixes of a member of a channel, for example "@" for an operator */
    bool GetChannelMemberPrefix( const CORE::CString& channel, const CORE::CString& nick, CORE::CString& prefix ) const;

    bool GetTopic( const CORE::CString& channel, CORE::CString& topic ) const;

    const CORE::CString& GetNick( void ) const;

    /** Changes the nick: right away before registration, through the server after it */
    bool SetNick( const CORE::CString& nick );

    /** Whether the server enabled the capability for us */
    bool HasCapability( const CORE::CString& name ) const;

    const CIrcClientSettings& GetSettings( void ) const;

    /** Whether two nicks or channel names are the same, the rfc1459 way: case and []\~ versus {}|^ do not matter */
    static bool NamesAreEqual( const CORE::CString& a, const CORE::CString& b );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SMember
    {
        CORE::CString nick;             /**< as the server writes it */
        CORE::CString prefixes;         /**< the mode prefixes, for example @ or @+ */
    };
    typedef struct SMember TMember;
    typedef GUCEF::map< CORE::CString, TMember > TMemberMap;      /**< folded nick -> member */

    struct SChannel
    {
        CORE::CString name;
        CORE::CString topic;
        TMemberMap members;
        TMemberMap pendingMembers;      /**< while a NAMES reply is coming in */
        bool namesInProgress;
    };
    typedef struct SChannel TChannel;
    typedef GUCEF::map< CORE::CString, TChannel > TChannelMap;

    CIrcClient( const CIrcClient& src );
    CIrcClient& operator=( const CIrcClient& src );

    void Reset( void );
    bool SendLine( const CORE::CString& line );
    bool IsOwnNick( const CORE::CString& nick ) const;
    TChannel* FindChannel( const CORE::CString& channel );
    const TChannel* FindChannel( const CORE::CString& channel ) const;
    void AddMember( TChannel& channel, const CORE::CString& nickWithPrefixes, bool pending );
    void RemoveMember( TChannel& channel, const CORE::CString& nick );

    void FillEventData( const CIrcMessage& message, CIrcMessageEventData& eventData ) const;
    bool Notify( const CORE::CEvent& eventId, const CIrcMessage& message, const CORE::CString& sourceNick, const CORE::CString& target, const CORE::CString& text );
    bool ReportError( const CORE::CString& text, const CIrcMessage* message );

    bool OnLine( const CORE::CString& line );
    bool OnMessage( const CIrcMessage& message );
    bool OnCap( const CIrcMessage& message );
    bool OnAuthenticate( const CIrcMessage& message );
    bool OnNumeric( const CIrcMessage& message, UInt32 numeric );
    bool OnTransportConnected( void );
    bool OnTransportDisconnected( void );
    void OnTimer( void );
    void EndCapabilityNegotiation( void );
    void RequestCapabilities( const CORE::CString::StringSet& offered );
    bool TryNextNick( void );

    static CORE::CString FoldName( const CORE::CString& name );
    static CORE::CString SanitizeText( const CORE::CString& text );
    static UInt64 GetTimeInMs( void );

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CIrcClientSettings m_settings;
    CIIrcTransportPtr m_transport;
    CORE::CTimer m_timer;
    bool m_isConnected;
    bool m_isRegistered;
    bool m_capNegotiationDone;
    bool m_saslInProgress;
    CORE::CString m_nick;
    UInt32 m_nickAttempt;
    CORE::CString::StringSet m_offeredCapabilities;
    CORE::CString::StringSet m_enabledCapabilities;
    TChannelMap m_channels;
    UInt64 m_lastReceivedInMs;
    UInt64 m_pingSentInMs;
    bool m_pingSent;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIRCCLIENT_H ? */
