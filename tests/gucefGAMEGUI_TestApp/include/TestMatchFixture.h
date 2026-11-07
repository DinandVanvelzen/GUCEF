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

#ifndef GUCEF_GAMEGUI_TESTMATCHFIXTURE_H
#define GUCEF_GAMEGUI_TESTMATCHFIXTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#include "../../gucefMULTIPLAY_TestApp/include/TestMatchSupport.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H ? */

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H
#include "../../gucefMULTIPLAY_TestApp/include/TestFakeMatchProvider.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H ? */

#include "gucefGAMEGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Registers the fake match provider with the factory of the module for the life of the scope */
class CFakeProviderScope
{
    public:

    CFakeProviderScope( void )
        : m_factory()
    {
        GUCEF::MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().RegisterConcreteFactory( "fake", &m_factory );
    }

    ~CFakeProviderScope()
    {
        GUCEF::MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().UnregisterConcreteFactory( "fake" );
    }

    private:

    CFakeProviderScope( const CFakeProviderScope& src );
    CFakeProviderScope& operator=( const CFakeProviderScope& src );

    GUCEF::TESTFW::TFakeMatchProviderFactory m_factory;
};

/*-------------------------------------------------------------------------*/

/**
 *  A match book of the local player "user-me" with a fake provider and a manual clock, and matches that are put in it
 *  without signatures (the book trusts its providers, the previews are not signed here)
 */
struct SMatchBookFixture
{
    GUCEF::TESTFW::CManualMatchClock clock;
    GUCEF::MULTIPLAY::CMatchBook book;
    GUCEF::MULTIPLAY::CIMatchProviderPtr providerPtr;
    GUCEF::TESTFW::CFakeMatchProvider* provider;
    GUCEF::UInt32 counter;

    enum EKind
    {
        KIND_YOURS = 0  ,   /**< active, the local player is on turn */
        KIND_THEIRS     ,   /**< active, Bob is on turn */
        KIND_INVITED    ,   /**< the local player is invited */
        KIND_FORMING    ,   /**< forming */
        KIND_FINISHED
    };

    SMatchBookFixture( void ) : clock(), book(), providerPtr(), provider( GUCEF_NULL ), counter( 0 ) {}

    bool Setup( void )
    {
        book.SetClock( &clock );
        book.SetLocalUserId( "user-me" );
        providerPtr = GUCEF::MULTIPLAY::CMultiplayGlobal::Instance()->GetMatchProviderFactory().Create( "fake" );
        if ( providerPtr.IsNULL() )
            return false;
        provider = static_cast< GUCEF::TESTFW::CFakeMatchProvider* >( providerPtr.GetPointerAlways() );
        return book.AddProvider( providerPtr );
    }

    static GUCEF::MULTIPLAY::CMatchParticipant MakeParticipant( const char* userId, const char* name, GUCEF::UInt32 slot, GUCEF::MULTIPLAY::TMatchParticipantStatus status )
    {
        GUCEF::MULTIPLAY::CMatchParticipant participant;
        participant.userId = userId;
        participant.displayName = name;
        participant.slot = slot;
        participant.status = status;
        return participant;
    }

    /** Puts a match at the provider (announced by the next RefreshAll()) */
    GUCEF::MULTIPLAY::CMatchInfo AddMatch( const GUCEF::CORE::CString& title, EKind kind, GUCEF::Int64 deadline = 0, GUCEF::Int64 lastActivity = 0, GUCEF::UInt32 turn = 3 )
    {
        using namespace GUCEF;
        MULTIPLAY::CMatchInfo info;
        info.matchId = MULTIPLAY::CMatchId::Generate();
        info.title = title;
        info.gameId = "testgame";
        info.gameVersion = "1.0";
        info.rulesetId = "standard";
        info.contentHash = "00";
        info.backend = "fake";
        info.mode = MULTIPLAY::MATCHMODE_ASYNC;
        info.createdAt = 1700000000000LL;
        info.creatorUserId = "user-bob";
        info.status = KIND_FORMING == kind ? MULTIPLAY::MATCHSTATUS_FORMING : ( KIND_FINISHED == kind ? MULTIPLAY::MATCHSTATUS_FINISHED : MULTIPLAY::MATCHSTATUS_ACTIVE );
        info.participants.push_back( MakeParticipant( "user-me", "Me", 0, KIND_INVITED == kind ? MULTIPLAY::MATCHPARTICIPANT_INVITED : MULTIPLAY::MATCHPARTICIPANT_ACTIVE ) );
        info.participants.push_back( MakeParticipant( "user-bob", "Bob", 1, MULTIPLAY::MATCHPARTICIPANT_ACTIVE ) );
        info.participants.push_back( MakeParticipant( "user-carol", "Carol", 2, MULTIPLAY::MATCHPARTICIPANT_ACTIVE ) );

        ++counter;
        MULTIPLAY::CMatchPackage package = TESTFW::MakeTestPackage( 1, GUCEF_NULL, "start" );
        MULTIPLAY::CMatchPreview preview;
        preview.matchId = info.matchId;
        preview.revision = 1;
        preview.turn = turn;
        preview.status = info.status;
        preview.deadline = deadline;
        preview.summary = "Summary";
        preview.lastActivity = lastActivity;
        if ( KIND_YOURS == kind )
            preview.currentUserIds.insert( "user-me" );
        else
        if ( KIND_THEIRS == kind )
            preview.currentUserIds.insert( "user-bob" );
        provider->InjectMatch( info, preview, package );
        return info;
    }

    /** The next revision of a match with another player on turn */
    void Play( const GUCEF::MULTIPLAY::CMatchInfo& info, const char* holder, GUCEF::UInt32 revision, GUCEF::MULTIPLAY::TMatchStatus status = GUCEF::MULTIPLAY::MATCHSTATUS_ACTIVE, GUCEF::Int64 deadline = 0 )
    {
        using namespace GUCEF;
        MULTIPLAY::CMatchPackage package = TESTFW::MakeTestPackage( revision, GUCEF_NULL, "next" );
        MULTIPLAY::CMatchPreview preview;
        preview.matchId = info.matchId;
        preview.revision = revision;
        preview.turn = revision;
        preview.status = status;
        preview.deadline = deadline;
        preview.summary = "Summary";
        if ( GUCEF_NULL != holder )
            preview.currentUserIds.insert( holder );
        provider->InjectTurn( preview, package );
    }

    bool Start( void )
    {
        return book.Start();
    }
};

/*-------------------------------------------------------------------------*/

/** Records the events of a notifier as text lines: "<label>", "<label>:<match id>" or, for an action, "<label>:<match id>|<action number>" */
class CEventLog : public GUCEF::CORE::CObserver
{
    public:

    typedef GUCEF::CORE::CTEventHandlerFunctor< CEventLog > TEventCallback;

    GUCEF::CORE::CString::StringVector lines;

    CEventLog( void ) : GUCEF::CORE::CObserver(), lines(), m_labels() {}

    virtual ~CEventLog() { UnsubscribeAllFromObserver(); }

    void Watch( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, const GUCEF::CORE::CString& label )
    {
        m_labels[ eventId.GetName() ] = label;
        TEventCallback callback( this, &CEventLog::OnEvent );
        SubscribeTo( notifier, eventId, callback );
    }

    /** How many lines are the label, alone or with data */
    GUCEF::UInt32 Count( const GUCEF::CORE::CString& label ) const
    {
        GUCEF::UInt32 count = 0;
        for ( size_t i=0; i<lines.size(); ++i )
            if ( lines[ i ] == label || 0 == lines[ i ].HasSubstr( label + ":", true ) )
                ++count;
        return count;
    }

    GUCEF::CORE::CString Last( void ) const { return lines.empty() ? GUCEF::CORE::CString::Empty : lines.back(); }

    void OnEvent( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData )
    {
        GUCEF::CORE::CString line = m_labels[ eventId.GetName() ];
        if ( GUCEF_NULL != eventData )
        {
            if ( GUCEF::GAMEGUI::CMatchListForm::MatchActionEvent == eventId )
            {
                const GUCEF::GAMEGUI::CMatchActionEventData* action = static_cast< const GUCEF::GAMEGUI::CMatchActionEventData* >( eventData );
                line += ":" + action->matchId + "|" + GUCEF::CORE::Int32ToString( (GUCEF::Int32) action->action );
            }
            else
            {
                line += ":" + static_cast< GUCEF::CORE::TCloneableString* >( eventData )->GetData();
            }
        }
        lines.push_back( line );
    }

    private:

    GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CString > m_labels;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_TESTMATCHFIXTURE_H ? */
