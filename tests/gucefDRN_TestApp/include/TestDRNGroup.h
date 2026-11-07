/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_DRN_TESTAPP_TESTDRNGROUP_H
#define GUCEF_DRN_TESTAPP_TESTDRNGROUP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#include "TestDRNTestUtils.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNTEST {

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/** A session with its members, each on a node of its own, all on loopback */
class CGroup
{
    public:

    GUCEF::vector< DRN::CDRNNode* > nodes;
    GUCEF::vector< DRN::CDRNSession* > sessions;
    GUCEF::vector< DRN::CDRNEndpoint > endpoints;
    DRN::CDRNNodeSettings nodeSettings;
    DRN::CDRNSessionSettings sessionSettings;
    CORE::CString channelType;
    CORE::CString params;

    CGroup( DRN::TDRNTopology topology, const CORE::CString& name, const CORE::CString& channel = "tcp", const CORE::CString& channelParams = CORE::CString::Empty )
        : nodes(), sessions(), endpoints(), nodeSettings(), sessionSettings(), channelType( channel ), params( channelParams )
    {
        nodeSettings.echoIntervalMs = 50;
        nodeSettings.pulseIntervalMs = 1;
        sessionSettings.sessionName = name;
        sessionSettings.topology = topology;
    }

    ~CGroup()
    {
        Stop();
    }

    DRN::CDRNSession& operator[]( UInt32 index ) { return *sessions[ index ]; }

    UInt32 GetSize( void ) const { return (UInt32) sessions.size(); }

    bool Start( UInt32 count )
    {
        if ( !AddNode() || !sessions[ 0 ]->Host( sessionSettings ) )
            return false;
        for ( UInt32 i=1; i<count; ++i )
            if ( !AddMember() )
                return false;
        return true;
    }

    bool AddMember( void )
    {
        if ( !AddNode() )
            return false;
        DRN::CDRNSessionSettings joinSettings;
        joinSettings.sessionName = sessionSettings.sessionName;
        if ( !sessions.back()->Join( DRN::CDRNEndpointList( endpoints[ 0 ] ), joinSettings ) )
            return false;
        return WaitForMembership();
    }

    bool WaitForMembership( void )
    {
        CMembership condition( *this );
        return PumpUntil( condition, 15000 );
    }

    void Stop( void )
    {
        for ( UInt32 i=0; i<nodes.size(); ++i )
        {
            nodes[ i ]->Stop();
            GUCEF_DELETE nodes[ i ];
        }
        nodes.clear();
        sessions.clear();
        endpoints.clear();
    }

    private:

    class CMembership : public CTestCondition
    {
        public:
        CGroup& m_group;
        explicit CMembership( CGroup& group ) : m_group( group ) {}
        virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
        {
            UInt32 count = m_group.GetSize();
            for ( UInt32 i=0; i<count; ++i )
            {
                DRN::CDRNSession& session = *m_group.sessions[ i ];
                if ( !session.IsActive() || session.GetMemberCount() != count )
                    return false;
                if ( DRN::DRN_TOPOLOGY_PEER_TO_PEER == m_group.sessionSettings.topology && session.GetDirectLinkCount() != count-1 )
                    return false;
            }
            return true;
        }
    };

    bool AddNode( void )
    {
        DRN::CDRNNode* node = GUCEF_NEW DRN::CDRNNode();
        node->SetSettings( nodeSettings );
        nodes.push_back( node );
        endpoints.push_back( LoopbackEndpoint( channelType, NextTestPort(), params ) );
        sessions.push_back( node->CreateSession() );
        return node->Listen( endpoints.back() );
    }
};

/*-------------------------------------------------------------------------*/

/** Records what a grid or stream tells the application, can vote on its own */
class CItemRecorder : public CORE::CObserver
{
    public:

    enum TAutoVote { VOTE_NOT, VOTE_YES, VOTE_NO };

    UInt32 keyChanged;
    UInt32 updates;
    UInt32 updatedKeys;
    UInt32 snapshots;
    UInt32 removed;
    UInt32 proposeAccepted;
    UInt32 proposeRefused;
    UInt32 lastProposeOutcome;
    CORE::CString lastRefusal;
    UInt32 votesStarted;
    UInt32 votesEnded;
    UInt32 lastVoteOutcome;
    UInt32 entriesEvents;
    TAutoVote autoVote;

    CItemRecorder( void )
        : keyChanged( 0 ), updates( 0 ), updatedKeys( 0 ), snapshots( 0 ), removed( 0 ), proposeAccepted( 0 ), proposeRefused( 0 )
        , lastProposeOutcome( 0 ), lastRefusal(), votesStarted( 0 ), votesEnded( 0 ), lastVoteOutcome( 0 ), entriesEvents( 0 ), autoVote( VOTE_NOT )
    {}

    virtual ~CItemRecorder() { SignalUpcomingObserverDestruction(); }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( DRN::CDRNDataGrid::GridKeyChangedEvent == eventid )
            ++keyChanged;
        else
        if ( DRN::CDRNDataGrid::GridUpdatedEvent == eventid )
        {
            ++updates;
            updatedKeys += (UInt32) static_cast< DRN::CDRNGridUpdatedEventData* >( eventdata )->GetData().size();
        }
        else
        if ( DRN::CDRNDataItem::SnapshotCompleteEvent == eventid )
            ++snapshots;
        else
        if ( DRN::CDRNDataItem::ItemRemovedEvent == eventid )
            ++removed;
        else
        if ( DRN::CDRNDataGrid::ProposeResultEvent == eventid )
        {
            const DRN::CDRNProposeResult& result = static_cast< DRN::CDRNProposeResultEventData* >( eventdata )->GetData();
            if ( result.accepted )
                ++proposeAccepted;
            else
            {
                ++proposeRefused;
                lastRefusal = result.reason;
            }
            lastProposeOutcome = result.voteOutcome;
        }
        else
        if ( DRN::CDRNDataGrid::VoteStartedEvent == eventid )
        {
            ++votesStarted;
            if ( VOTE_NOT != autoVote )
            {
                const DRN::CDRNVoteInfo& info = static_cast< DRN::CDRNVoteEventData* >( eventdata )->GetData();
                static_cast< DRN::CDRNDataGrid* >( notifier )->CastVote( info.voteId, VOTE_YES == autoVote );
            }
        }
        else
        if ( DRN::CDRNDataGrid::VoteEndedEvent == eventid )
        {
            ++votesEnded;
            lastVoteOutcome = static_cast< DRN::CDRNVoteEventData* >( eventdata )->GetData().outcome;
        }
        else
        if ( DRN::CDRNDataStream::EntriesAvailableEvent == eventid )
            ++entriesEvents;
    }
};

/*-------------------------------------------------------------------------*/

/** Waits for grid values (or their absence) on several members */
class CGridCondition : public CTestCondition
{
    public:

    class CCheck
    {
        public:
        DRN::CDRNSession* session;
        CORE::CString gridName;
        UInt32 owner;
        CORE::CVariant key;
        CORE::CVariant expected;
        bool missing;
    };

    GUCEF::vector< CCheck > checks;

    void Expect( DRN::CDRNSession& session, const CORE::CString& gridName, UInt32 owner, const CORE::CVariant& key, const CORE::CVariant& expected )
    {
        CCheck check;
        check.session = &session;
        check.gridName = gridName;
        check.owner = owner;
        check.key = key;
        check.expected = expected;
        check.missing = false;
        checks.push_back( check );
    }

    void ExpectMissing( DRN::CDRNSession& session, const CORE::CString& gridName, UInt32 owner, const CORE::CVariant& key )
    {
        CCheck check;
        check.session = &session;
        check.gridName = gridName;
        check.owner = owner;
        check.key = key;
        check.missing = true;
        checks.push_back( check );
    }

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 i=0; i<checks.size(); ++i )
        {
            const CCheck& check = checks[ i ];
            DRN::CDRNDataGrid* grid = check.session->FindGrid( check.gridName, check.owner );
            if ( GUCEF_NULL == grid || !grid->IsSnapshotComplete() )
                return false;
            CORE::CVariant value;
            bool has = grid->GetValue( check.key, value );
            if ( check.missing ? has : ( !has || !DRN::CDRNVariantCodec::IsSame( value, check.expected ) ) )
                return false;
        }
        return true;
    }
};

/*-------------------------------------------------------------------------*/

}; /* namespace DRNTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_TESTAPP_TESTDRNGROUP_H ? */
