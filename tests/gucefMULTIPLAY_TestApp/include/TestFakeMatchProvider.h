/*
 *  gucefMULTIPLAY_TestApp: test header: a scriptable fake match provider for tests of the match book and what
 *  uses it
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
 *  Test support. The fake keeps the matches "at the provider" in memory. The test plays the other participants with
 *  InjectMatch() and InjectTurn(), which change the state at the provider without telling the book; RefreshAll() and
 *  Refresh() then report what changed as events, like a real provider would.
 *
 *  What can be scripted:
 *  - SetOnline( false ): every call fails (the future fails, the refresh reports an error); SetOnline( true ) tells the book
 *  - FailNext( operation, count ): the next calls of an operation fail
 *  - SetDelayTicks( n ): calls complete only after n calls of Tick(), in the order they were made (no timers, no threads)
 *  - SetDuplicateEvents( n ): every added/updated event is raised n more times
 *  - SetAnnounceOwnSubmits( true ): a Submit() also raises MatchUpdatedEvent
 *  - stale and idempotent submits behave as the interface says: a newer revision at the provider answers STALE with the
 *    current preview, a repeated submission of the package at the provider answers OK again
 *  Nothing here opens a socket or starts a process.
 */

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H
#define GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H

#ifdef __cplusplus

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

namespace GUCEF {
namespace TESTFW {

enum EFakeMatchOperation
{
    FAKEMATCHOP_REFRESH  = 0 ,
    FAKEMATCHOP_CREATE   = 1 ,
    FAKEMATCHOP_FETCH    = 2 ,
    FAKEMATCHOP_SUBMIT   = 3 ,
    FAKEMATCHOP_OTHER    = 4 ,      /**< invite, respond, resign, nudge, remove, rematch, propose, vote */
    FAKEMATCHOP_COUNT    = 5
};

class CFakeMatchProvider : public MULTIPLAY::CIMatchProvider
{
    public:

    CORE::CString providerName;
    UInt32 refreshAllCount;
    UInt32 refreshCount;
    UInt32 submitCount;             /**< Submit() calls that were accepted for processing */
    UInt32 fetchCount;
    UInt32 createCount;
    UInt32 resignCount;
    UInt32 nudgeCount;
    UInt32 removeCount;
    UInt32 rematchCount;
    UInt32 proposeCount;
    UInt32 voteCount;
    UInt32 inviteCount;
    UInt32 respondCount;
    UInt32 startCount;
    UInt32 stopCount;
    CORE::CString lastRefreshedMatchId;
    CORE::CString lastNudgedUserId;

    CFakeMatchProvider( void )
        : MULTIPLAY::CIMatchProvider()
        , providerName( "fake" )
        , refreshAllCount( 0 ), refreshCount( 0 ), submitCount( 0 ), fetchCount( 0 ), createCount( 0 )
        , resignCount( 0 ), nudgeCount( 0 ), removeCount( 0 ), rematchCount( 0 ), proposeCount( 0 ), voteCount( 0 )
        , inviteCount( 0 ), respondCount( 0 ), startCount( 0 ), stopCount( 0 )
        , lastRefreshedMatchId(), lastNudgedUserId()
        , m_started( false ), m_online( true )
        , m_capabilities( MULTIPLAY::MATCHCAPABILITY_PREVIEW_POLL | MULTIPLAY::MATCHCAPABILITY_NUDGE | MULTIPLAY::MATCHCAPABILITY_RESIGN | MULTIPLAY::MATCHCAPABILITY_REMATCH )
        , m_delayTicks( 0 ), m_duplicateEvents( 0 ), m_announceOwnSubmits( false )
        , m_matches(), m_announced(), m_pending()
    {
        for ( int i=0; i<(int) FAKEMATCHOP_COUNT; ++i )
            m_failures[ i ] = 0;
    }

    virtual ~CFakeMatchProvider() {}

    /* ---- scripting ---- */

    void SetOnline( bool online )
    {
        if ( m_online == online )
            return;
        m_online = online;
        ReportProviderState( online, online ? "online" : "offline" );
    }

    bool IsOnline( void ) const { return m_online; }

    void FailNext( EFakeMatchOperation operation, UInt32 count ) { m_failures[ operation ] = count; }

    void SetDelayTicks( UInt32 ticks ) { m_delayTicks = ticks; }

    void SetDuplicateEvents( UInt32 extra ) { m_duplicateEvents = extra; }

    void SetAnnounceOwnSubmits( bool announce ) { m_announceOwnSubmits = announce; }

    void SetCapabilities( UInt32 capabilities ) { m_capabilities = capabilities; }

    /** Puts a match at the provider the way another participant would have; nothing is announced until a refresh */
    void InjectMatch( const MULTIPLAY::CMatchInfo& info, const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package )
    {
        SMatch& match = m_matches[ info.matchId ];
        match.info = info;
        match.packages.clear();
        match.previews.clear();
        match.packages[ package.revision ] = package;
        match.previews[ package.revision ] = preview;
        match.latestRevision = package.revision;
    }

    /** The next turn of a match, played by another participant */
    void InjectTurn( const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package )
    {
        SMatch& match = m_matches[ preview.matchId ];
        match.packages[ package.revision ] = package;
        match.previews[ package.revision ] = preview;
        if ( package.revision > match.latestRevision )
            match.latestRevision = package.revision;
    }

    /** The match is gone at the provider; reported at the next refresh */
    void InjectRemoval( const CORE::CString& matchId ) { m_removed.insert( matchId ); m_matches.erase( matchId ); }

    bool HasMatch( const CORE::CString& matchId ) const { return m_matches.find( matchId ) != m_matches.end(); }

    UInt32 GetLatestRevision( const CORE::CString& matchId ) const
    {
        TMatchMap::const_iterator i = m_matches.find( matchId );
        return i == m_matches.end() ? 0 : (*i).second.latestRevision;
    }

    /** Completes the calls that wait for their delay; returns how many were completed */
    UInt32 Tick( void )
    {
        UInt32 done = 0;
        TPendingVector ready;
        for ( size_t i=0; i<m_pending.size(); )
        {
            if ( m_pending[ i ].ticksLeft > 0 )
                --m_pending[ i ].ticksLeft;
            if ( 0 == m_pending[ i ].ticksLeft )
            {
                ready.push_back( m_pending[ i ] );
                m_pending.erase( m_pending.begin() + i );
            }
            else
                ++i;
        }
        for ( size_t i=0; i<ready.size(); ++i )
        {
            Execute( ready[ i ] );
            ++done;
        }
        return done;
    }

    UInt32 GetPendingCount( void ) const { return (UInt32) m_pending.size(); }

    /** Raises the events directly, as a push channel or a late arriving mail would: what the provider reports without being asked */
    void RaiseAdded( const MULTIPLAY::CMatchUpdate& update ) { ReportMatchAdded( update ); }

    void RaiseUpdated( const MULTIPLAY::CMatchUpdate& update ) { ReportMatchUpdated( update ); }

    void RaiseRemoved( const CORE::CString& matchId ) { ReportMatchRemoved( matchId ); }

    void RaiseError( const CORE::CString& description ) { ReportProviderError( description ); }

    /* ---- CIMatchProvider ---- */

    virtual bool Start( void ) GUCEF_VIRTUAL_OVERRIDE { m_started = true; ++startCount; return true; }

    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE { m_started = false; ++stopCount; }

    virtual bool IsStarted( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_started; }

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_capabilities; }

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE { return providerName; }

    virtual bool RefreshAll( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++refreshAllCount;
        if ( !m_started )
            return false;
        if ( !m_online || ConsumeFailure( FAKEMATCHOP_REFRESH ) )
        {
            ReportProviderError( "The refresh failed" );
            return false;
        }
        SPending op;
        op.kind = OP_REFRESH_ALL;
        Queue( op );
        return true;
    }

    virtual bool Refresh( const CORE::CString& matchId ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++refreshCount;
        lastRefreshedMatchId = matchId;
        if ( !m_started )
            return false;
        if ( !m_online || ConsumeFailure( FAKEMATCHOP_REFRESH ) )
        {
            ReportProviderError( "The refresh failed" );
            return false;
        }
        SPending op;
        op.kind = OP_REFRESH;
        op.matchId = matchId;
        Queue( op );
        return true;
    }

    virtual bool CreateMatch( const MULTIPLAY::CMatchRecord& record, const MULTIPLAY::CMatchPackage& firstPackage, const MULTIPLAY::CMatchPreview& firstPreview, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++createCount;
        SPending op;
        op.kind = OP_CREATE;
        op.matchId = record.info.matchId;
        op.record = record;
        op.package = firstPackage;
        op.preview = firstPreview;
        op.failure = ConsumeFailure( FAKEMATCHOP_CREATE );
        return QueueWithFuture( op, future );
    }

    virtual bool Fetch( const CORE::CString& matchId, UInt32 revision, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++fetchCount;
        SPending op;
        op.kind = OP_FETCH;
        op.matchId = matchId;
        op.revision = revision;
        op.failure = ConsumeFailure( FAKEMATCHOP_FETCH );
        return QueueWithFuture( op, future );
    }

    virtual bool Submit( const CORE::CString& matchId, UInt32 expectedRevision, const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++submitCount;
        SPending op;
        op.kind = OP_SUBMIT;
        op.matchId = matchId;
        op.revision = expectedRevision;
        op.preview = preview;
        op.package = package;
        op.failure = ConsumeFailure( FAKEMATCHOP_SUBMIT );
        return QueueWithFuture( op, future );
    }

    virtual bool Invite( const CORE::CString& matchId, const MULTIPLAY::CMatchParticipant& participant, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE { ++inviteCount; return QueueOther( matchId, future ); }

    virtual bool Respond( const CORE::CString& matchId, const MULTIPLAY::CMatchAcceptance& acceptance, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE { ++respondCount; return QueueOther( matchId, future ); }

    virtual bool Resign( const CORE::CString& matchId, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !HasCapability( MULTIPLAY::MATCHCAPABILITY_RESIGN ) ) return false;
        ++resignCount; return QueueOther( matchId, future );
    }

    virtual bool Nudge( const CORE::CString& matchId, const CORE::CString& userId, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !HasCapability( MULTIPLAY::MATCHCAPABILITY_NUDGE ) ) return false;
        ++nudgeCount; lastNudgedUserId = userId; return QueueOther( matchId, future );
    }

    virtual bool Remove( const CORE::CString& matchId, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE { ++removeCount; return QueueOther( matchId, future ); }

    virtual bool Rematch( const CORE::CString& matchId, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !HasCapability( MULTIPLAY::MATCHCAPABILITY_REMATCH ) ) return false;
        ++rematchCount; return QueueOther( matchId, future );
    }

    virtual bool Propose( const CORE::CString& matchId, const MULTIPLAY::CMatchProposal& proposal, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE { ++proposeCount; return QueueOther( matchId, future ); }

    virtual bool Vote( const CORE::CString& matchId, const MULTIPLAY::CMatchVote& vote, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE { ++voteCount; return QueueOther( matchId, future ); }

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE { config.SetAttribute( "name", providerName ); return true; }

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE
    {
        providerName = config.GetAttributeValueOrChildValueByName( "name" ).AsString( providerName, true );
        return true;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::TESTFW::CFakeMatchProvider";
        return classTypeName;
    }

    private:

    enum EKind { OP_REFRESH_ALL, OP_REFRESH, OP_CREATE, OP_FETCH, OP_SUBMIT, OP_OTHER };

    struct SPending
    {
        EKind kind;
        CORE::CString matchId;
        UInt32 revision;
        MULTIPLAY::CMatchRecord record;
        MULTIPLAY::CMatchPreview preview;
        MULTIPLAY::CMatchPackage package;
        CORE::CFutureResultSource source;
        bool hasSource;
        bool failure;
        UInt32 ticksLeft;

        SPending( void ) : kind( OP_OTHER ), matchId(), revision( 0 ), record(), preview(), package(), source(), hasSource( false ), failure( false ), ticksLeft( 0 ) {}
    };
    typedef GUCEF::vector< SPending > TPendingVector;

    struct SMatch
    {
        MULTIPLAY::CMatchInfo info;
        GUCEF::map< UInt32, MULTIPLAY::CMatchPackage > packages;
        GUCEF::map< UInt32, MULTIPLAY::CMatchPreview > previews;
        UInt32 latestRevision;

        SMatch( void ) : info(), packages(), previews(), latestRevision( 0 ) {}
    };
    typedef GUCEF::map< CORE::CString, SMatch > TMatchMap;
    typedef GUCEF::map< CORE::CString, UInt32 > TRevisionMap;

    bool ConsumeFailure( EFakeMatchOperation operation )
    {
        if ( m_failures[ operation ] > 0 )
        {
            --m_failures[ operation ];
            return true;
        }
        return false;
    }

    void Queue( SPending& op )
    {
        op.ticksLeft = m_delayTicks;
        if ( 0 == op.ticksLeft )
            Execute( op );
        else
            m_pending.push_back( op );
    }

    bool QueueWithFuture( SPending& op, CORE::CFutureResult& future )
    {
        CORE::CFutureResultSource source;
        future = source.GetFuture();
        op.source = source;
        op.hasSource = true;
        if ( !m_online )
            op.failure = true;
        Queue( op );
        return true;
    }

    bool QueueOther( const CORE::CString& matchId, CORE::CFutureResult& future )
    {
        SPending op;
        op.kind = OP_OTHER;
        op.matchId = matchId;
        op.failure = ConsumeFailure( FAKEMATCHOP_OTHER );
        return QueueWithFuture( op, future );
    }

    MULTIPLAY::CMatchUpdate MakeUpdate( const SMatch& match ) const
    {
        MULTIPLAY::CMatchUpdate update;
        GUCEF::map< UInt32, MULTIPLAY::CMatchPreview >::const_iterator p = match.previews.find( match.latestRevision );
        if ( p != match.previews.end() )
            update.preview = (*p).second;
        update.info = match.info;
        update.hasInfo = true;
        return update;
    }

    void Announce( const SMatch& match )
    {
        MULTIPLAY::CMatchUpdate update = MakeUpdate( match );
        TRevisionMap::iterator known = m_announced.find( update.preview.matchId );
        bool isNew = known == m_announced.end();
        if ( !isNew && (*known).second >= update.preview.revision )
            return;
        m_announced[ update.preview.matchId ] = update.preview.revision;
        for ( UInt32 repeat=0; repeat<=m_duplicateEvents; ++repeat )
        {
            bool ok = isNew ? ReportMatchAdded( update ) : ReportMatchUpdated( update );
            if ( !ok )
                return;
        }
    }

    void Complete( SPending& op, MULTIPLAY::CMatchOperationResult& result )
    {
        if ( !op.hasSource )
            return;
        if ( op.failure )
            op.source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The call failed" );
        else
            op.source.Complete( result.Clone(), true );
    }

    void Execute( SPending& op )
    {
        MULTIPLAY::CMatchOperationResult result;
        result.matchId = op.matchId;
        switch ( op.kind )
        {
            case OP_REFRESH_ALL:
            {
                if ( !m_online )
                    return;
                TMatchMap::const_iterator i = m_matches.begin();
                while ( i != m_matches.end() )
                {
                    Announce( (*i).second );
                    ++i;
                }
                CORE::CString::StringSet::iterator r = m_removed.begin();
                while ( r != m_removed.end() )
                {
                    m_announced.erase( (*r) );
                    ReportMatchRemoved( (*r) );
                    ++r;
                }
                m_removed.clear();
                return;
            }
            case OP_REFRESH:
            {
                TMatchMap::const_iterator i = m_matches.find( op.matchId );
                if ( m_online && i != m_matches.end() )
                    Announce( (*i).second );
                return;
            }
            case OP_CREATE:
            {
                if ( !op.failure )
                {
                    InjectMatch( op.record.info, op.preview, op.package );
                    result.preview = op.preview;
                    result.hasPreview = true;
                    Announce( m_matches[ op.matchId ] );
                }
                Complete( op, result );
                return;
            }
            case OP_FETCH:
            {
                TMatchMap::const_iterator i = m_matches.find( op.matchId );
                if ( i == m_matches.end() || (*i).second.packages.find( op.revision ) == (*i).second.packages.end() )
                    result.status = MULTIPLAY::MATCHOPERATION_NOT_FOUND;
                else
                {
                    result.package = (*i).second.packages.find( op.revision )->second;
                    result.hasPackage = true;
                }
                Complete( op, result );
                return;
            }
            case OP_SUBMIT:
            {
                if ( !op.failure )
                {
                    TMatchMap::iterator i = m_matches.find( op.matchId );
                    if ( i == m_matches.end() )
                        result.status = MULTIPLAY::MATCHOPERATION_NOT_FOUND;
                    else
                    {
                        SMatch& match = (*i).second;
                        GUCEF::map< UInt32, MULTIPLAY::CMatchPackage >::const_iterator held = match.packages.find( op.package.revision );
                        if ( held != match.packages.end() && (*held).second.sha256 == op.package.sha256 )
                        {
                            // the same package again: the answer is OK again
                            result.preview = match.previews[ op.package.revision ];
                            result.hasPreview = true;
                        }
                        else
                        if ( match.latestRevision > op.revision )
                        {
                            result.status = MULTIPLAY::MATCHOPERATION_STALE;
                            result.preview = match.previews[ match.latestRevision ];
                            result.hasPreview = true;
                        }
                        else
                        if ( op.preview.revision != op.revision + 1 || op.package.revision != op.preview.revision )
                        {
                            result.status = MULTIPLAY::MATCHOPERATION_REJECTED;
                            result.reason = "The revision does not follow";
                        }
                        else
                        {
                            match.packages[ op.package.revision ] = op.package;
                            match.previews[ op.package.revision ] = op.preview;
                            match.latestRevision = op.package.revision;
                            result.preview = op.preview;
                            result.hasPreview = true;
                            if ( m_announceOwnSubmits )
                                Announce( match );
                            else
                                m_announced[ op.matchId ] = op.package.revision;
                        }
                    }
                    ReportSubmitResult( result );
                }
                Complete( op, result );
                return;
            }
            default:
            {
                Complete( op, result );
                return;
            }
        }
    }

    private:

    bool m_started;
    bool m_online;
    UInt32 m_capabilities;
    UInt32 m_delayTicks;
    UInt32 m_duplicateEvents;
    bool m_announceOwnSubmits;
    UInt32 m_failures[ FAKEMATCHOP_COUNT ];
    TMatchMap m_matches;
    TRevisionMap m_announced;
    CORE::CString::StringSet m_removed;
    TPendingVector m_pending;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< MULTIPLAY::CIMatchProvider, CFakeMatchProvider, MT::CMutex > TFakeMatchProviderFactory;

}; /* namespace TESTFW */
}; /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTFAKEMATCHPROVIDER_H ? */
