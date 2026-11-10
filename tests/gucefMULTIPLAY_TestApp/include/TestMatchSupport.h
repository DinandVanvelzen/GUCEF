/*
 *  gucefMULTIPLAY_TestApp: test header: support for tests of matches: a manual clock, a recording match alert sink,
 *  a recorder of the events of a match book and builders of signed test matches
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

/*
 *  Test support. Nothing here starts a process or shows anything to the user: the sink only records what it is given.
 *  The identities need the "ed25519" signature codec (a plugin the test application loads).
 */

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H
#define GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H

#ifdef __cplusplus

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

namespace GUCEF {
namespace TESTFW {

/** A clock the test sets: both the wall time and the minute of the day */
class CManualMatchClock : public MULTIPLAY::CIMatchClock
{
    public:

    Int64 nowMs;
    Int32 minuteOfDay;

    CManualMatchClock( void ) : MULTIPLAY::CIMatchClock(), nowMs( 1700000000000LL ), minuteOfDay( 12 * 60 ) {}

    virtual Int64 GetNowMs( void ) const GUCEF_VIRTUAL_OVERRIDE { return nowMs; }

    virtual Int32 GetLocalMinuteOfDay( void ) const GUCEF_VIRTUAL_OVERRIDE { return minuteOfDay; }

    /** Moves both the wall time and the minute of the day on */
    void AdvanceMs( Int64 ms )
    {
        nowMs += ms;
        minuteOfDay = (Int32)( ( minuteOfDay + ( ms / 60000 ) ) % 1440 );
    }

    /** Moves the clock forward to the next time it is that minute of the day (not at all when it is that minute now) */
    void SetMinuteOfDay( Int32 minute )
    {
        Int64 difference = ( (Int64) minute - minuteOfDay + 1440 ) % 1440;
        nowMs += difference * 60000;
        minuteOfDay = minute;
    }
};

/*-------------------------------------------------------------------------*/

/** A sink that records what it is given and does nothing else */
class CRecordingMatchAlertSink : public MULTIPLAY::CIMatchAlertSink
{
    public:

    MULTIPLAY::TMatchAlertVector delivered;
    CORE::CString::StringVector cleared;
    bool interrupting;

    explicit CRecordingMatchAlertSink( bool isInterrupting ) : MULTIPLAY::CIMatchAlertSink(), delivered(), cleared(), interrupting( isInterrupting ) {}

    virtual void Deliver( const MULTIPLAY::CMatchAlert& alert ) GUCEF_VIRTUAL_OVERRIDE { delivered.push_back( alert ); }

    virtual void Clear( const CORE::CString& matchId ) GUCEF_VIRTUAL_OVERRIDE { cleared.push_back( matchId ); }

    virtual bool IsInterrupting( void ) const GUCEF_VIRTUAL_OVERRIDE { return interrupting; }

    virtual const CORE::CString& GetSinkName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "recording";
        return name;
    }

    UInt32 CountOfKind( MULTIPLAY::TMatchAlertKind kind ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<delivered.size(); ++i )
            if ( delivered[ i ].kind == kind )
                ++count;
        return count;
    }

    UInt32 CountClearedFor( const CORE::CString& matchId ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<cleared.size(); ++i )
            if ( cleared[ i ] == matchId )
                ++count;
        return count;
    }
};

/*-------------------------------------------------------------------------*/

/** Records the events of a match book */
class CMatchBookRecorder : public CORE::CObserver
{
    public:

    struct STurn { CORE::CString matchId; UInt32 revision; };
    typedef GUCEF::vector< STurn > TTurnVector;

    UInt32 changed;
    UInt32 added;
    UInt32 updated;
    UInt32 removed;
    UInt32 opened;
    UInt32 flags;
    TTurnVector yourTurn;
    CORE::CString::StringVector finished;
    CORE::CString::StringVector invites;
    UInt32 problems;
    CORE::CString lastProblem;

    CMatchBookRecorder( void ) : CORE::CObserver(), changed( 0 ), added( 0 ), updated( 0 ), removed( 0 ), opened( 0 ), flags( 0 ), yourTurn(), finished(), invites(), problems( 0 ), lastProblem() {}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::TESTFW::CMatchBookRecorder";
        return classTypeName;
    }

    UInt32 CountYourTurn( const CORE::CString& matchId, UInt32 revision ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<yourTurn.size(); ++i )
            if ( yourTurn[ i ].matchId == matchId && yourTurn[ i ].revision == revision )
                ++count;
        return count;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CMatchBook::MatchBookChangedEvent == eventId )
        {
            const MULTIPLAY::CMatchBookChange* change = static_cast< const MULTIPLAY::CMatchBookChange* >( eventData );
            ++changed;
            if ( MULTIPLAY::MATCHBOOKCHANGE_ADDED == change->kind ) ++added;
            else if ( MULTIPLAY::MATCHBOOKCHANGE_UPDATED == change->kind ) ++updated;
            else if ( MULTIPLAY::MATCHBOOKCHANGE_REMOVED == change->kind ) ++removed;
            else if ( MULTIPLAY::MATCHBOOKCHANGE_OPENED == change->kind ) ++opened;
            else ++flags;
        }
        else
        if ( MULTIPLAY::CMatchBook::YourTurnEvent == eventId )
        {
            const MULTIPLAY::CMatchTurnEventData* turn = static_cast< const MULTIPLAY::CMatchTurnEventData* >( eventData );
            STurn t;
            t.matchId = turn->matchId;
            t.revision = turn->revision;
            yourTurn.push_back( t );
        }
        else
        if ( MULTIPLAY::CMatchBook::MatchFinishedEvent == eventId )
            finished.push_back( static_cast< const CORE::TCloneableString* >( eventData )->GetData() );
        else
        if ( MULTIPLAY::CMatchBook::InviteReceivedEvent == eventId )
            invites.push_back( static_cast< const CORE::TCloneableString* >( eventData )->GetData() );
        else
        if ( MULTIPLAY::CMatchBook::ProviderProblemEvent == eventId )
        {
            ++problems;
            lastProblem = static_cast< const MULTIPLAY::CMatchProviderState* >( eventData )->description;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** A new identity with a key pair; false without an "ed25519" codec */
inline bool
MakeTestIdentity( const char* displayName, MULTIPLAY::CPlayerIdentity& identity )
{
    identity = MULTIPLAY::CPlayerIdentity();
    identity.displayName = displayName;
    return identity.GenerateKeyPair();
}

/*-------------------------------------------------------------------------*/

/**
 *  The info of a match of the given identities in turn order: the first is the creator and takes part, the others are invited
 *  (or take part as well when allActive is set)
 */
inline MULTIPLAY::CMatchInfo
MakeTestMatchInfo( const GUCEF::vector< MULTIPLAY::CPlayerIdentity >& identities, const char* title, bool allActive )
{
    MULTIPLAY::CMatchInfo info;
    info.matchId = MULTIPLAY::CMatchId::Generate();
    info.title = title;
    info.gameId = "testgame";
    info.gameVersion = "1.0";
    info.rulesetId = "standard";
    info.contentHash = "00";
    info.backend = "fake";
    info.mode = MULTIPLAY::MATCHMODE_ASYNC;
    info.status = MULTIPLAY::MATCHSTATUS_ACTIVE;
    info.createdAt = 1700000000000LL;
    for ( size_t i=0; i<identities.size(); ++i )
    {
        MULTIPLAY::CMatchParticipant participant;
        participant.userId = identities[ i ].userId;
        participant.displayName = identities[ i ].displayName;
        participant.slot = (UInt32) i;
        participant.publicKey = identities[ i ].publicKey;
        participant.status = ( 0 == i || allActive ) ? MULTIPLAY::MATCHPARTICIPANT_ACTIVE : MULTIPLAY::MATCHPARTICIPANT_INVITED;
        participant.contact = identities[ i ].displayName + "@example.invalid";
        info.participants.push_back( participant );
    }
    info.creatorUserId = identities[ 0 ].userId;
    return info;
}

/*-------------------------------------------------------------------------*/

/** A package of the given revision that follows the given previous package (NULL for the first) */
inline MULTIPLAY::CMatchPackage
MakeTestPackage( UInt32 revision, const MULTIPLAY::CMatchPackage* previous, const char* payloadText )
{
    MULTIPLAY::CMatchPackage package;
    package.SetPayload( payloadText, (UInt32) strlen( payloadText ) );
    package.revision = revision;
    if ( GUCEF_NULL != previous )
    {
        package.baseRevision = previous->revision;
        package.baseSha256 = previous->sha256;
    }
    return package;
}

/*-------------------------------------------------------------------------*/

/** A preview signed by the issuer for the package, with the given participants on turn */
inline MULTIPLAY::CMatchPreview
MakeTestPreview( const MULTIPLAY::CMatchInfo& info                ,
                 const MULTIPLAY::CPlayerIdentity& issuer         ,
                 const MULTIPLAY::CMatchPackage& package          ,
                 const MULTIPLAY::CPlayerIdentity* nextHolder     ,
                 MULTIPLAY::TMatchStatus status                   ,
                 Int64 lastActivity                               )
{
    MULTIPLAY::CMatchPreview preview;
    preview.matchId = info.matchId;
    preview.revision = package.revision;
    preview.turn = package.revision;
    if ( GUCEF_NULL != nextHolder )
        preview.currentUserIds.insert( nextHolder->userId );
    preview.status = status;
    preview.packageSha256 = package.sha256;
    preview.baseSha256 = package.baseSha256;
    preview.summary = "turn";
    preview.lastActivity = lastActivity;
    preview.Sign( issuer );
    return preview;
}

}; /* namespace TESTFW */
}; /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTMATCHSUPPORT_H ? */
