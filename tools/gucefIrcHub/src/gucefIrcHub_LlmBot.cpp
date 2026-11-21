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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CASYNC_H
#include "gucefCORE_CASync.h"
#define GUCEF_CORE_CASYNC_H
#endif /* GUCEF_CORE_CASYNC_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefIrcHub_TextUtils.h"
#include "gucefIrcHub_LlmBot.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFIrcHub {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 TimerIntervalInMs = 200;
static const UInt32 TicksPerSecond = 1000 / TimerIntervalInMs;
static const UInt32 FirstReconnectDelayInTicks = 5 * TicksPerSecond;
static const UInt32 MaxReconnectDelayInTicks = 60 * TicksPerSecond;
/** bytes of text per IRC message: the server adds the prefix and the target to the 512 bytes of a line */
static const size_t MaxLineBytes = 400;
static const char* PrivateKeyPrefix = "private:";
/** clients drop a typing indicator that is not repeated within a few seconds */
static const UInt32 TypingRepeatInTicks = 3 * TicksPerSecond;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

SBotSettings::SBotSettings( void )
    : nick()
    , password()
    , displayName()
    , room()
    , channels()
    , systemPrompt()
    , llm()
    , dailyTokensPerAccount( 0 )
    , maxHistoryTurns( 20 )
    , maxChannelReplyLines( 15 )
    , maxPrivateReplyLines( 60 )
{
}

/*-------------------------------------------------------------------------*/

static CORE::CString
LowerCase( const CORE::CString& text )
{
    std::string result = text.STL_String();
    for ( size_t i=0; i<result.size(); ++i )
    {
        if ( result[ i ] >= 'A' && result[ i ] <= 'Z' )
            result[ i ] = (char) ( result[ i ] - 'A' + 'a' );
    }
    return CORE::CString( result );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLlmBot::CLlmBot( const SBotSettings& settings                ,
                  const COM::CIrcClientSettings& server       ,
                  const CORE::CString& operName               ,
                  const CORE::CString& operPassword           ,
                  const CORE::CString::StringVector& otherBots,
                  const CORE::CString& threadPoolName         )
    : CORE::CObserver()
    , m_settings( settings )
    , m_ircSettings( server )
    , m_operName( operName )
    , m_operPassword( operPassword )
    , m_otherBots( otherBots )
    , m_threadPoolName( threadPoolName )
    , m_irc()
    , m_timer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), TimerIntervalInMs )
    , m_started( false )
    , m_reconnectDelayInTicks( FirstReconnectDelayInTicks )
    , m_ticksUntilReconnect( 0 )
    , m_conversations()
    , m_usage()
    , m_usageDay()
    , m_reportedModel()
    , m_ticks( 0 )
    , m_nextJobId( 1 )
    , m_jobs()
    , m_jobsRunning( 0 )
    , m_jobLock()
{GUCEF_TRACE;

    m_ircSettings.nick = settings.nick;
    m_ircSettings.alternativeNicks.clear();
    m_ircSettings.userName = settings.nick;
    m_ircSettings.realName = settings.displayName.IsNULLOrEmpty() ? settings.nick : settings.displayName;
    m_ircSettings.saslAccount = settings.nick;
    m_ircSettings.saslPassword = settings.password;
    m_ircSettings.autoJoinChannels.clear();
    if ( !settings.room.IsNULLOrEmpty() )
        m_ircSettings.autoJoinChannels.push_back( settings.room );
    for ( size_t i=0; i<settings.channels.size(); ++i )
        m_ircSettings.autoJoinChannels.push_back( settings.channels[ i ] );

    SubscribeTo( &m_irc );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CLlmBot::~CLlmBot()
{GUCEF_TRACE;

    Stop( 0 );
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const SBotSettings&
CLlmBot::GetSettings( void ) const
{
    return m_settings;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLlmBot::GetClassTypeName( void ) const
{
    static const CORE::CString classTypeName = "GUCEFIrcHub::CLlmBot";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CLlmBot::Start( void )
{GUCEF_TRACE;

    m_started = true;
    m_ticksUntilReconnect = 0;
    m_timer.SetEnabled( true );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefIrcHub: Bot " + m_settings.nick + " (" + m_settings.displayName +
                      ") connects to " + m_ircSettings.server + ":" + CORE::UInt32ToString( m_ircSettings.port ) );
    if ( !m_irc.Connect( m_ircSettings ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefIrcHub: Bot " + m_settings.nick + " could not start connecting; trying again later" );
        m_ticksUntilReconnect = m_reconnectDelayInTicks;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::Stop( UInt32 waitInMs )
{GUCEF_TRACE;

    if ( !m_started )
        return;
    m_started = false;
    m_ticksUntilReconnect = 0;
    m_timer.SetEnabled( false );
    if ( m_irc.IsConnected() )
        m_irc.Disconnect( "Going offline" );

    // requests that run hold a pointer to this bot
    UInt32 waitedInMs = 0;
    while ( true )
    {
        {
            MT::CScopeMutex lock( m_jobLock );
            if ( 0 == m_jobsRunning )
                break;
        }
        if ( waitedInMs >= waitInMs && 0 != waitInMs )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefIrcHub: Bot " + m_settings.nick + " stops while requests to the model still run" );
            break;
        }
        MT::ThreadDelay( 100 );
        waitedInMs += 100;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLlmBot::DefaultSystemPrompt( const SBotSettings& settings )
{GUCEF_TRACE;

    CORE::CString prompt = "You are " + settings.nick + ", a chat bot in the private IRC chat of one family";
    if ( !settings.displayName.IsNULLOrEmpty() )
        prompt += "; you are " + settings.displayName;
    prompt += ".\n"
              "- Write plain text: IRC shows no Markdown, so no headings, tables, bold or code fences. Simple lists with "
              "dashes are fine.\n"
              "- Keep answers short, a few sentences, unless someone asks for more or the question needs it.\n"
              "- Children use this chat too: keep everything suitable for all ages.\n"
              "- If you are not sure of a fact, say so.";
    return prompt;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLlmBot::SystemPromptFor( const CORE::CString& conversationKey ) const
{GUCEF_TRACE;

    CORE::CString prompt = m_settings.systemPrompt.IsNULLOrEmpty() ? DefaultSystemPrompt( m_settings ) : m_settings.systemPrompt;
    const std::string key = conversationKey.STL_String();
    const std::string privatePrefix = PrivateKeyPrefix;
    if ( 0 == key.compare( 0, privatePrefix.size(), privatePrefix ) )
    {
        prompt += "\n\nThis is a private chat with " + CORE::CString( key.substr( privatePrefix.size() ) ) + ".";
    }
    else
    {
        prompt += "\n\nThis is the channel " + conversationKey + ". Every message starts with the name of the person who "
                  "wrote it, followed by a colon. Your answer is shown with the name of the person you answer in front of it, "
                  "so do not start with a name yourself.";
    }
    return prompt;
}

/*-------------------------------------------------------------------------*/

bool
CLlmBot::IsChannel( const CORE::CString& target ) const
{
    return target.ByteSize() > 1 && ( '#' == target.C_String()[ 0 ] || '&' == target.C_String()[ 0 ] );
}

/*-------------------------------------------------------------------------*/

bool
CLlmBot::IsSharedChannel( const CORE::CString& channel ) const
{
    for ( size_t i=0; i<m_settings.channels.size(); ++i )
    {
        if ( COM::CIrcClient::NamesAreEqual( m_settings.channels[ i ], channel ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLlmBot::IsOtherBot( const CORE::CString& nickOrAccount ) const
{
    for ( size_t i=0; i<m_otherBots.size(); ++i )
    {
        if ( COM::CIrcClient::NamesAreEqual( m_otherBots[ i ], nickOrAccount ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::OnNotify( CORE::CNotifier* notifier    ,
                   const CORE::CEvent& eventId  ,
                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == &m_timer )
    {
        if ( CORE::CTimer::TimerUpdateEvent == eventId )
            OnTimer();
        return;
    }
    if ( notifier != &m_irc )
        return;

    const COM::CIrcMessageEventData* data = static_cast< const COM::CIrcMessageEventData* >( eventData );
    if ( COM::CIrcClient::MessageEvent == eventId && GUCEF_NULL != data )
    {
        OnMessage( *data );
    }
    else
    if ( COM::CIrcClient::RegisteredEvent == eventId )
    {
        OnRegistered();
    }
    else
    if ( COM::CIrcClient::DisconnectedEvent == eventId )
    {
        OnDisconnected();
    }
    else
    if ( COM::CIrcClient::JoinEvent == eventId && GUCEF_NULL != data )
    {
        if ( COM::CIrcClient::NamesAreEqual( data->sourceNick, m_irc.GetNick() ) )
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefIrcHub: Bot " + m_settings.nick + " joined " + data->target );
    }
    else
    if ( COM::CIrcClient::ErrorEvent == eventId && GUCEF_NULL != data )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefIrcHub: Bot " + m_settings.nick + ": " + data->text );
    }
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::OnRegistered( void )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefIrcHub: Bot " + m_settings.nick + " is logged in" );
    m_reconnectDelayInTicks = FirstReconnectDelayInTicks;
    // a bot (+B) shows as one in clients; the operator block exempts it from the flood limits for long answers
    m_irc.SendRaw( "MODE " + m_irc.GetNick() + " +B" );
    if ( !m_operName.IsNULLOrEmpty() && !m_operPassword.IsNULLOrEmpty() )
        m_irc.SendRaw( "OPER " + m_operName + " " + m_operPassword );
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::OnDisconnected( void )
{GUCEF_TRACE;

    if ( !m_started )
        return;
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefIrcHub: Bot " + m_settings.nick + " lost the connection; connecting again in " +
                       CORE::UInt32ToString( m_reconnectDelayInTicks / TicksPerSecond ) + "s" );
    m_ticksUntilReconnect = m_reconnectDelayInTicks;
    m_reconnectDelayInTicks *= 2;
    if ( m_reconnectDelayInTicks > MaxReconnectDelayInTicks )
        m_reconnectDelayInTicks = MaxReconnectDelayInTicks;

    // conversations that waited for an answer can ask again once connected
    TConversationMap::iterator i = m_conversations.begin();
    while ( i != m_conversations.end() )
    {
        (*i).second.pending.clear();
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::OnTimer( void )
{GUCEF_TRACE;

    if ( 0 != m_ticksUntilReconnect )
    {
        --m_ticksUntilReconnect;
        if ( 0 == m_ticksUntilReconnect && m_started && !m_irc.IsConnected() )
        {
            if ( !m_irc.Connect( m_ircSettings ) )
                OnDisconnected();
        }
    }

    // the answers of the model, sent from this thread
    std::vector< SJob > finished;
    CORE::CString::StringVector typingTargets;
    {
        MT::CScopeMutex lock( m_jobLock );
        ++m_ticks;
        if ( 0 == m_ticks % TypingRepeatInTicks )
        {
            TJobMap::iterator t = m_jobs.begin();
            while ( t != m_jobs.end() )
            {
                if ( !(*t).second.done )
                    typingTargets.push_back( (*t).second.replyTarget );
                ++t;
            }
        }
        TJobMap::iterator i = m_jobs.begin();
        while ( i != m_jobs.end() )
        {
            if ( (*i).second.done )
            {
                finished.push_back( (*i).second );
                m_jobs.erase( i++ );
            }
            else
            {
                ++i;
            }
        }
    }
    for ( size_t i=0; i<typingTargets.size(); ++i )
        SendTyping( typingTargets[ i ], true );
    for ( size_t i=0; i<finished.size(); ++i )
        FinishJob( finished[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::OnMessage( const COM::CIrcMessageEventData& data )
{GUCEF_TRACE;

    // our own messages come back (echo-message), other bots are never answered, and messages replayed from the
    // history (in a batch) were answered when they were new
    if ( COM::CIrcClient::NamesAreEqual( data.sourceNick, m_irc.GetNick() ) )
        return;
    if ( IsOtherBot( data.sourceNick ) || ( !data.account.IsNULLOrEmpty() && IsOtherBot( data.account ) ) )
        return;
    CORE::CString batch;
    if ( data.message.GetTag( "batch", batch ) )
        return;

    // every client logs in, so the account is known; without account-tag the nick is the account
    const CORE::CString account = data.account.IsNULLOrEmpty() ? LowerCase( data.sourceNick ) : data.account;
    CORE::CString text = data.text;

    if ( !IsChannel( data.target ) )
    {
        // a private message
        const CORE::CString key = PrivateKeyPrefix + account;
        if ( HandleCommand( text, key, data.sourceNick, CORE::CString::Empty, account ) )
            return;
        Ask( key, data.sourceNick, CORE::CString::Empty, account, text );
        return;
    }

    const bool isRoom = !m_settings.room.IsNULLOrEmpty() && COM::CIrcClient::NamesAreEqual( data.target, m_settings.room );
    CORE::CString rest;
    const bool addressed = IsAddressedTo( text, m_irc.GetNick(), rest );
    if ( addressed )
    {
        text = rest;
    }
    else
    if ( isRoom )
    {
        // in its room it answers everything, except what is said to someone else in it
        CORE::CString::StringVector members;
        m_irc.GetChannelMembers( data.target, members );
        CORE::CString addressee;
        if ( StartsWithAddressOf( text, members, addressee ) && !COM::CIrcClient::NamesAreEqual( addressee, m_irc.GetNick() ) )
            return;
    }
    else
    {
        return;
    }
    if ( !isRoom && !IsSharedChannel( data.target ) )
        return;

    const CORE::CString key = LowerCase( data.target );
    if ( HandleCommand( text, key, data.target, data.sourceNick, account ) )
        return;
    if ( text.IsNULLOrEmpty() )
    {
        Reply( data.target, data.sourceNick, "Yes? Ask me anything. !help tells what else I can do." );
        return;
    }
    Ask( key, data.target, data.sourceNick, account, data.sourceNick + ": " + text );
}

/*-------------------------------------------------------------------------*/

bool
CLlmBot::HandleCommand( const CORE::CString& command        ,
                        const CORE::CString& conversationKey,
                        const CORE::CString& replyTarget    ,
                        const CORE::CString& replyNick      ,
                        const CORE::CString& account        )
{GUCEF_TRACE;

    const CORE::CString name = LowerCase( CORE::CString( TrimWhitespace( command.STL_String() ) ) );
    if ( "!help" == name )
    {
        CORE::CString help = "I am " + m_settings.displayName + ". ";
        if ( !m_settings.room.IsNULLOrEmpty() )
            help += "In " + m_settings.room + " and in a private message I answer everything";
        else
            help += "In a private message I answer everything";
        if ( !m_settings.channels.empty() )
            help += "; in the other channels only what starts with \"" + m_settings.nick + ":\"";
        help += ". !reset makes me forget this conversation, !model tells which model I am, !usage how much you used today.";
        Reply( replyTarget, replyNick, help );
        return true;
    }
    if ( "!reset" == name )
    {
        SConversation& conversation = m_conversations[ conversationKey ];
        conversation.history.clear();
        conversation.pending.clear();
        Reply( replyTarget, replyNick, "Done: I forgot this conversation." );
        return true;
    }
    if ( "!model" == name )
    {
        CORE::CString model = m_settings.llm.model;
        if ( !m_reportedModel.IsNULLOrEmpty() )
            model = m_reportedModel;
        if ( model.IsNULLOrEmpty() )
            model = "the model the server runs; ask me something first, then I know its name";
        Reply( replyTarget, replyNick, m_settings.displayName + ": " + model + " (" + m_settings.llm.vendor + ")" );
        return true;
    }
    if ( "!usage" == name )
    {
        CORE::CString text = "You used " + CORE::UInt64ToString( UsageToday( account ) ) + " tokens of " + m_settings.nick + " today (UTC)";
        if ( m_settings.dailyTokensPerAccount > 0 )
            text += ", of " + CORE::UInt32ToString( m_settings.dailyTokensPerAccount ) + " per day";
        Reply( replyTarget, replyNick, text + "." );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt64
CLlmBot::UsageToday( const CORE::CString& account )
{GUCEF_TRACE;

    const CORE::CString today( CORE::CDateTime::NowUTCDateTime().ToIso8601DateTimeString( false, false ).STL_String().substr( 0, 8 ) );
    if ( today != m_usageDay )
    {
        m_usage.clear();
        m_usageDay = today;
    }
    TUsageMap::const_iterator i = m_usage.find( account );
    return i != m_usage.end() ? (*i).second : 0;
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::Ask( const CORE::CString& conversationKey ,
              const CORE::CString& replyTarget     ,
              const CORE::CString& replyNick       ,
              const CORE::CString& account         ,
              const CORE::CString& text            )
{GUCEF_TRACE;

    if ( m_settings.dailyTokensPerAccount > 0 && UsageToday( account ) >= m_settings.dailyTokensPerAccount )
    {
        Reply( replyTarget, replyNick, "You reached your limit of " + CORE::UInt32ToString( m_settings.dailyTokensPerAccount ) +
               " tokens for " + m_settings.nick + " today. It starts again at midnight (UTC); the other bots still answer." );
        return;
    }

    SConversation& conversation = m_conversations[ conversationKey ];
    SChatTurn turn;
    turn.role = "user";
    turn.content = text;
    if ( conversation.busy )
    {
        conversation.pending.push_back( turn );
        conversation.pendingNick = replyNick;
        conversation.pendingAccount = account;
        return;
    }
    conversation.history.push_back( turn );
    StartJob( conversationKey, conversation, replyTarget, replyNick, account );
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::TrimHistory( TChatTurnVector& history ) const
{
    while ( history.size() > m_settings.maxHistoryTurns && !history.empty() )
        history.erase( history.begin() );
    // a conversation starts with a message of a person
    while ( !history.empty() && "user" != history.front().role )
        history.erase( history.begin() );
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::StartJob( const CORE::CString& conversationKey ,
                   SConversation& conversation          ,
                   const CORE::CString& replyTarget     ,
                   const CORE::CString& replyNick       ,
                   const CORE::CString& account         )
{GUCEF_TRACE;

    TrimHistory( conversation.history );
    conversation.busy = true;

    UInt32 jobId = 0;
    {
        MT::CScopeMutex lock( m_jobLock );
        jobId = m_nextJobId++;
        SJob& job = m_jobs[ jobId ];
        job.id = jobId;
        job.conversationKey = conversationKey;
        job.replyTarget = replyTarget;
        job.replyNick = replyNick;
        job.account = account;
        job.system = SystemPromptFor( conversationKey );
        job.turns = conversation.history;
        ++m_jobsRunning;
    }
    SendTyping( replyTarget, true );
    // a chain of CASync runs once it is submitted
    CORE::CASync async( m_threadPoolName );
    async.QueueMemberCallback( this, &CLlmBot::RunJob, jobId );
    async.Submit();
}

/*-------------------------------------------------------------------------*/

bool
CLlmBot::RunJob( UInt32 jobId )
{GUCEF_TRACE;

    CORE::CString system;
    TChatTurnVector turns;
    {
        MT::CScopeMutex lock( m_jobLock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i == m_jobs.end() )
        {
            --m_jobsRunning;
            return false;
        }
        system = (*i).second.system;
        turns = (*i).second.turns;
    }

    SLlmResult result;
    Chat( m_settings.llm, system, turns, result );

    MT::CScopeMutex lock( m_jobLock );
    TJobMap::iterator i = m_jobs.find( jobId );
    if ( i != m_jobs.end() )
    {
        (*i).second.result = result;
        (*i).second.done = true;
    }
    --m_jobsRunning;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::FinishJob( SJob& job )
{GUCEF_TRACE;

    SendTyping( job.replyTarget, false );
    SConversation& conversation = m_conversations[ job.conversationKey ];
    conversation.busy = false;

    const UInt64 tokens = (UInt64) job.result.inputTokens + job.result.outputTokens;
    UsageToday( job.account );
    m_usage[ job.account ] += tokens;

    if ( !job.result.answer.IsNULLOrEmpty() )
    {
        // what was asked and answered stays out of the log
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefIrcHub: Bot " + m_settings.nick + " answered " + job.account + " in " +
                          ( IsChannel( job.replyTarget ) ? job.replyTarget : CORE::CString( "a private chat" ) ) + " (" +
                          CORE::UInt64ToString( tokens ) + " tokens)" );
        if ( !job.result.model.IsNULLOrEmpty() )
            m_reportedModel = job.result.model;
        SChatTurn turn;
        turn.role = "assistant";
        turn.content = job.result.answer;
        conversation.history.push_back( turn );
        Reply( job.replyTarget, job.replyNick, job.result.answer );
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefIrcHub: Bot " + m_settings.nick + " got no answer from the model: " + job.result.error );
        // the question stays out of the memory, so a conversation keeps taking turns
        if ( !conversation.history.empty() && "user" == conversation.history.back().role )
            conversation.history.pop_back();
        if ( job.result.refused )
            Reply( job.replyTarget, job.replyNick, "Sorry, I can't help with that one." );
        else
            Reply( job.replyTarget, job.replyNick, "Sorry, I could not get an answer from " + m_settings.displayName + ": " + job.result.error );
    }

    // the messages that came in meanwhile, as one
    if ( !conversation.pending.empty() && m_irc.IsRegistered() )
    {
        SChatTurn turn;
        turn.role = "user";
        for ( size_t i=0; i<conversation.pending.size(); ++i )
            turn.content += ( i > 0 ? "\n" : "" ) + conversation.pending[ i ].content;
        conversation.pending.clear();
        conversation.history.push_back( turn );
        StartJob( job.conversationKey, conversation, job.replyTarget,
                  job.replyNick.IsNULLOrEmpty() ? CORE::CString::Empty : conversation.pendingNick, conversation.pendingAccount );
    }
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::Reply( const CORE::CString& target, const CORE::CString& nick, const CORE::CString& text )
{GUCEF_TRACE;

    if ( !m_irc.IsRegistered() )
        return;

    std::vector< std::string > lines;
    SplitForIrc( text.STL_String(), MaxLineBytes, lines );
    const bool inChannel = IsChannel( target );
    const size_t maxLines = inChannel ? m_settings.maxChannelReplyLines : m_settings.maxPrivateReplyLines;
    const size_t totalLines = lines.size();
    if ( maxLines > 0 && lines.size() > maxLines )
        lines.resize( maxLines );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        CORE::CString line( lines[ i ] );
        if ( 0 == i && inChannel && !nick.IsNULLOrEmpty() )
            line = nick + ": " + line;
        m_irc.PrivMsg( target, line );
    }
    if ( lines.size() < totalLines )
    {
        m_irc.PrivMsg( target, "(cut off after " + CORE::UInt32ToString( (UInt32) lines.size() ) + " of " + CORE::UInt32ToString( (UInt32) totalLines ) +
                       " lines" + ( inChannel ? CORE::CString( "; ask me in a private message for long answers)" ) : CORE::CString( ")" ) ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CLlmBot::SendTyping( const CORE::CString& target, bool active )
{GUCEF_TRACE;

    // the typing indicator of IRCv3 (+typing client tag), which many clients show
    if ( m_irc.IsRegistered() && m_irc.HasCapability( "message-tags" ) )
        m_irc.SendRaw( "@+typing=" + CORE::CString( active ? "active" : "done" ) + " TAGMSG " + target );
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFIrcHub */

/*-------------------------------------------------------------------------*/
