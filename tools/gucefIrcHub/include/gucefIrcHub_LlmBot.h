/*
 *  gucefIrcHub: IRC hub with LLM bots for a family IRC server
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

#ifndef GUCEF_IRCHUB_LLMBOT_H
#define GUCEF_IRCHUB_LLMBOT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <set>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COM_CIRCCLIENT_H
#include "gucefCOM_CIrcClient.h"
#define GUCEF_COM_CIRCCLIENT_H
#endif /* GUCEF_COM_CIRCCLIENT_H ? */

#ifndef GUCEF_COM_CIRCCLIENTSETTINGS_H
#include "gucefCOM_CIrcClientSettings.h"
#define GUCEF_COM_CIRCCLIENTSETTINGS_H
#endif /* GUCEF_COM_CIRCCLIENTSETTINGS_H ? */

#include "gucefIrcHub_LlmVendorClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFIrcHub {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SBotSettings
{
    GUCEF::CORE::CString nick;                          /**< also the account it logs in with */
    GUCEF::CORE::CString password;
    GUCEF::CORE::CString displayName;                   /**< what the model is, for people: "Claude (Anthropic, claude-opus-5-5)" */
    GUCEF::CORE::CString room;                          /**< its own channel, where it answers every message; empty: none */
    GUCEF::CORE::CString::StringVector channels;        /**< shared channels, where it answers when addressed */
    GUCEF::CORE::CString systemPrompt;                  /**< empty: the default of DefaultSystemPrompt() */
    SLlmSettings llm;
    GUCEF::UInt32 dailyTokensPerAccount;                /**< tokens a person may use per day (UTC), 0: no limit */
    GUCEF::UInt32 maxHistoryTurns;                      /**< messages a conversation remembers */
    GUCEF::UInt32 maxChannelReplyLines;                 /**< lines of an answer in a channel; the rest is cut off */
    GUCEF::UInt32 maxPrivateReplyLines;                 /**< lines of an answer in a private chat */

    SBotSettings( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An LLM as a member of the IRC server: one IRC connection, logged in with SASL as its own account.
 *
 *  It answers every message in its room, messages addressed to it ("nick: ..." or "@nick") in the shared channels,
 *  and every private message. Each channel and each private chat is a conversation with its own memory of the last
 *  maxHistoryTurns messages. A message that arrives while the model works on an answer in the same conversation is
 *  answered after it, together with any others that arrived meanwhile.
 *
 *  Commands, as a message of their own (in a shared channel addressed to it): !help, !reset (forget the conversation),
 *  !model, !usage.
 *
 *  The requests to the model run on the thread pool given; their answers are sent from the thread that pulses the
 *  application. A lost connection is made again, with increasing delays up to a minute.
 */
class CLlmBot : public GUCEF::CORE::CObserver
{
    public:

    /**
     *  @param settings        the bot
     *  @param server          the server to connect to; nick, account and channels come from the bot settings
     *  @param operName        an operator block that exempts the bot from flood limits; empty: none
     *  @param operPassword    its password
     *  @param otherBots       the nicks of the other bots, whose messages are never answered (no bot loops)
     *  @param threadPoolName  the thread pool the requests to the model run on
     */
    CLlmBot( const SBotSettings& settings                         ,
             const GUCEF::COM::CIrcClientSettings& server         ,
             const GUCEF::CORE::CString& operName                 ,
             const GUCEF::CORE::CString& operPassword             ,
             const GUCEF::CORE::CString::StringVector& otherBots  ,
             const GUCEF::CORE::CString& threadPoolName           );

    virtual ~CLlmBot() GUCEF_VIRTUAL_OVERRIDE;

    bool Start( void );

    /** Disconnects and waits up to waitInMs for requests to the model that are running */
    void Stop( GUCEF::UInt32 waitInMs );

    const SBotSettings& GetSettings( void ) const;

    /** The system prompt for a bot without one of its own */
    static GUCEF::CORE::CString DefaultSystemPrompt( const SBotSettings& settings );

    /** Runs one request to the model; called on a thread of the pool */
    bool RunJob( GUCEF::UInt32 jobId );

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier                 ,
                           const GUCEF::CORE::CEvent& eventId               ,
                           GUCEF::CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SConversation
    {
        TChatTurnVector history;
        TChatTurnVector pending;                /**< messages that arrived while a request ran */
        GUCEF::CORE::CString pendingNick;       /**< who wrote the last of them */
        GUCEF::CORE::CString pendingAccount;
        bool busy;

        SConversation( void ) : history(), pending(), pendingNick(), pendingAccount(), busy( false ) {}
    };
    typedef std::map< GUCEF::CORE::CString, SConversation > TConversationMap;

    struct SJob
    {
        GUCEF::UInt32 id;
        GUCEF::CORE::CString conversationKey;
        GUCEF::CORE::CString replyTarget;       /**< channel or nick */
        GUCEF::CORE::CString replyNick;         /**< addressed in the answer in a channel */
        GUCEF::CORE::CString account;           /**< whose usage it counts as */
        GUCEF::CORE::CString system;
        TChatTurnVector turns;
        SLlmResult result;
        bool done;

        SJob( void ) : id( 0 ), conversationKey(), replyTarget(), replyNick(), account(), system(), turns(), result(), done( false ) {}
    };
    typedef std::map< GUCEF::UInt32, SJob > TJobMap;
    typedef std::map< GUCEF::CORE::CString, GUCEF::UInt64 > TUsageMap;

    CLlmBot( const CLlmBot& src );
    CLlmBot& operator=( const CLlmBot& src );

    void OnTimer( void );
    void OnRegistered( void );
    void OnDisconnected( void );
    void OnMessage( const GUCEF::COM::CIrcMessageEventData& data );
    bool HandleCommand( const GUCEF::CORE::CString& command, const GUCEF::CORE::CString& conversationKey,
                        const GUCEF::CORE::CString& replyTarget, const GUCEF::CORE::CString& replyNick, const GUCEF::CORE::CString& account );
    void Ask( const GUCEF::CORE::CString& conversationKey, const GUCEF::CORE::CString& replyTarget,
              const GUCEF::CORE::CString& replyNick, const GUCEF::CORE::CString& account, const GUCEF::CORE::CString& text );
    void StartJob( const GUCEF::CORE::CString& conversationKey, SConversation& conversation, const GUCEF::CORE::CString& replyTarget,
                   const GUCEF::CORE::CString& replyNick, const GUCEF::CORE::CString& account );
    void FinishJob( SJob& job );
    void Reply( const GUCEF::CORE::CString& target, const GUCEF::CORE::CString& nick, const GUCEF::CORE::CString& text );
    void SendTyping( const GUCEF::CORE::CString& target, bool active );
    void TrimHistory( TChatTurnVector& history ) const;
    GUCEF::CORE::CString SystemPromptFor( const GUCEF::CORE::CString& conversationKey ) const;
    GUCEF::UInt64 UsageToday( const GUCEF::CORE::CString& account );
    bool IsChannel( const GUCEF::CORE::CString& target ) const;
    bool IsSharedChannel( const GUCEF::CORE::CString& channel ) const;
    bool IsOtherBot( const GUCEF::CORE::CString& nickOrAccount ) const;

    SBotSettings m_settings;
    GUCEF::COM::CIrcClientSettings m_ircSettings;
    GUCEF::CORE::CString m_operName;
    GUCEF::CORE::CString m_operPassword;
    GUCEF::CORE::CString::StringVector m_otherBots;
    GUCEF::CORE::CString m_threadPoolName;
    GUCEF::COM::CIrcClient m_irc;
    GUCEF::CORE::CTimer m_timer;
    bool m_started;
    GUCEF::UInt32 m_reconnectDelayInTicks;
    GUCEF::UInt32 m_ticksUntilReconnect;        /**< 0: no reconnect planned */
    TConversationMap m_conversations;
    TUsageMap m_usage;                          /**< tokens per account today */
    GUCEF::CORE::CString m_usageDay;
    GUCEF::CORE::CString m_reportedModel;       /**< the model the API says answered */
    GUCEF::UInt32 m_ticks;
    GUCEF::UInt32 m_nextJobId;
    TJobMap m_jobs;                             /**< shared with the threads of the pool */
    GUCEF::UInt32 m_jobsRunning;
    mutable GUCEF::MT::CMutex m_jobLock;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFIrcHub */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IRCHUB_LLMBOT_H ? */
