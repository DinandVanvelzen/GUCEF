/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H
#include "gucefDRN_CDRNResourceSessionComponent.h"
#define GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CDRNRESOURCESESSIONCOMPONENT_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACECODEC_H
#include "gucefDRN_CDRNNamespaceCodec.h"
#define GUCEF_DRN_CDRNNAMESPACECODEC_H
#endif /* GUCEF_DRN_CDRNNAMESPACECODEC_H ? */

#ifndef GUCEF_DRN_CDRNREQUESTGATE_H
#include "gucefDRN_CDRNRequestGate.h"
#define GUCEF_DRN_CDRNREQUESTGATE_H
#endif /* GUCEF_DRN_CDRNREQUESTGATE_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H
#include "gucefDRN_CDRNResourceIoTaskConsumer.h"
#define GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H
#endif /* GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H ? */

#include "gucefDRN_CDRNNamespaceClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the calls share, so that a call on the pool outlives the client object that made it */
class CDRNNamespaceClient::CState
{
    public:

    CDRNSession* session;
    UInt32 peerId;
    UInt32 defaultTimeoutMs;
    mutable MT::CMutex lock;
    bool haveInfo;
    CDRNNamespaceInfo info;
    CORE::CString threadPoolName;
    CDRNRequestGate requestGate;        /**< STAT and LIST requests under way, within what the member announced */
    CDRNRequestGate listingGate;        /**< ListAll() listings under way, within what the member announced */
    bool selfThrottle;

    CState( CDRNSession& s, UInt32 peer ) : session( &s ), peerId( peer ), defaultTimeoutMs( 15000 ), lock(), haveInfo( false ), info(), threadPoolName( CDRNNamespaceClient::DefaultThreadPoolName )
        , requestGate( 0 ), listingGate( 0 ), selfThrottle( true ) {}

    /** A place among the listings the member allows, kept for as long as the object lives */
    class CListingTurn
    {
        public:

        CListingTurn( CState& s, UInt32 timeoutInMs ) : state( s ), counted( false ), held( true )
        {GUCEF_TRACE;

            if ( state.selfThrottle )
            {
                held = state.listingGate.Acquire( state.TimeoutOf( timeoutInMs ), state.session->GetNode() );
                counted = held;
            }
        }

        ~CListingTurn()
        {GUCEF_TRACE;

            if ( counted )
                state.listingGate.Release();
        }

        bool IsHeld( void ) const { return held; }

        private:

        CListingTurn( const CListingTurn& src );
        CListingTurn& operator=( const CListingTurn& src );

        CState& state;
        bool counted;
        bool held;
    };

    static CDRNNamespaceResult TimedOut( const char* what )
    {GUCEF_TRACE;

        return CDRNNamespaceResult( DRN_NSRESULT_TIMEOUT, DRN_RESSTATUS_TIMEOUT, CORE::CString( "timed out waiting for " ) + what );
    }

    UInt32 TimeoutOf( UInt32 timeoutInMs ) const
    {GUCEF_TRACE;

        MT::CScopeMutex scopeLock( lock );
        return 0 != timeoutInMs ? timeoutInMs : defaultTimeoutMs;
    }

    static CDRNNamespaceResult MakeResult( TDRNResourceStatus status, const CORE::CString& message )
    {GUCEF_TRACE;

        return CDRNNamespaceResult( CDRNNamespaceResult::CodeFromStatus( status ), status, message );
    }

    static CDRNNamespaceResult Malformed( const char* what )
    {GUCEF_TRACE;

        return CDRNNamespaceResult( DRN_NSRESULT_INTEGRITY, DRN_RESSTATUS_PROTOCOL_ERROR, CORE::CString( "the answer of the member is malformed: " ) + what );
    }

    /** Sends the request in 'body' (made with the request id) and waits for the answer on this thread */
    CDRNNamespaceResult Roundtrip( UInt32 opcode, UInt32 requestId, const CORE::CDynamicBuffer& body, UInt32 timeoutInMs, CORE::CDynamicBuffer& answer, bool counted = false )
    {GUCEF_TRACE;

        if ( session->GetNode().IsWorkingOnThisThread() )
            return CDRNNamespaceResult( DRN_NSRESULT_WRONG_THREAD, DRN_RESSTATUS_TIMEOUT, "a blocking call on a thread that works on the node would never be answered" );
        UInt32 timeout = TimeoutOf( timeoutInMs );

        // a request the member counts against its limit (STAT, LIST) waits for its turn first, first come first served,
        // so that no more of them are under way than the member announced. The wait is part of the timeout of the call
        bool holdsTurn = false;
        if ( counted && selfThrottle )
        {
            UInt64 startedMs = CDRNClock::NowInMilliseconds();
            if ( !requestGate.Acquire( timeout, session->GetNode() ) )
                return TimedOut( "the member's request budget" );
            holdsTurn = true;
            UInt64 waitedMs = CDRNClock::NowInMilliseconds() - startedMs;
            if ( waitedMs >= timeout )
            {
                requestGate.Release();
                return TimedOut( "the member's request budget" );
            }
            timeout -= (UInt32) waitedMs;
        }
        CORE::CString message;
        TDRNResourceStatus status = session->GetResources().CallNamespace( peerId, opcode, requestId, body, timeout, message, answer );
        if ( holdsTurn )
            requestGate.Release();
        return MakeResult( status, message );
    }

    CDRNNamespaceResult Negotiate( CDRNNamespaceInfo& result, UInt32 timeoutInMs )
    {GUCEF_TRACE;

        UInt32 id = session->GetResources().GetNextNamespaceRequestId();
        CORE::CDynamicBuffer body;
        CDRNWireWriter writer( body );
        CDRNNamespaceCodec::WriteHelloRequest( writer, id, DRN_NS_VERSION, 0 );
        CORE::CDynamicBuffer answer;
        CDRNNamespaceResult outcome = Roundtrip( DRN_RES_NS_HELLO, id, body, timeoutInMs, answer );
        if ( !outcome.IsOk() )
            return outcome;
        CDRNWireReader reader( answer );
        CDRNNamespaceInfo info;
        if ( !CDRNNamespaceCodec::ReadHelloBody( reader, info ) )
            return Malformed( "hello" );
        if ( info.version < 1 || info.version > DRN_NS_VERSION || 0 == info.maxStatPaths || 0 == info.maxPageEntries )
            return CDRNNamespaceResult( DRN_NSRESULT_UNSUPPORTED, DRN_RESSTATUS_UNSUPPORTED, "the member speaks no namespace version in common" );
        result = info;
        MT::CScopeMutex scopeLock( lock );
        this->info = info;
        haveInfo = true;

        // what the member announced is what is kept to from now on; unknown (0): no limit, as before
        requestGate.SetLimit( info.limitsAnnounced ? info.maxNamespaceRequests : 0 );
        listingGate.SetLimit( info.limitsAnnounced ? info.maxNamespaceListings : 0 );
        return outcome;
    }

    CDRNNamespaceResult EnsureInfo( CDRNNamespaceInfo& result, UInt32 timeoutInMs )
    {GUCEF_TRACE;

        {
            MT::CScopeMutex scopeLock( lock );
            if ( haveInfo )
            {
                result = info;
                return CDRNNamespaceResult();
            }
        }
        return Negotiate( result, timeoutInMs );
    }

    CDRNNamespaceResult Shares( GUCEF::vector< CDRNNamespaceShareInfo >& shares, UInt32 timeoutInMs )
    {GUCEF_TRACE;

        shares.clear();
        UInt32 id = session->GetResources().GetNextNamespaceRequestId();
        CORE::CDynamicBuffer body;
        CDRNWireWriter writer( body );
        CDRNNamespaceCodec::WriteSharesRequest( writer, id );
        CORE::CDynamicBuffer answer;
        CDRNNamespaceResult outcome = Roundtrip( DRN_RES_NS_SHARES, id, body, timeoutInMs, answer );
        if ( !outcome.IsOk() )
            return outcome;
        CDRNWireReader reader( answer );
        UInt32 count = 0;
        if ( !CDRNNamespaceCodec::ReadCount( reader, 5, 4096, count ) )
            return Malformed( "shares" );
        for ( UInt32 i=0; i<count; ++i )
        {
            CDRNNamespaceShareInfo share;
            if ( !CDRNNamespaceCodec::ReadShare( reader, share ) )
            {
                shares.clear();
                return Malformed( "share" );
            }
            shares.push_back( share );
        }
        return outcome;
    }

    /** One request of a batch of paths, appended to 'results' */
    CDRNNamespaceResult StatBatch( const CORE::CString& share, const CORE::CString::StringVector& paths, const CORE::CString::StringVector& known,
                                   UInt32 first, UInt32 count, GUCEF::vector< CDRNNamespaceEntry >& results, UInt32 timeoutInMs )
    {GUCEF_TRACE;

        UInt32 id = session->GetResources().GetNextNamespaceRequestId();
        CORE::CDynamicBuffer body;
        CDRNWireWriter writer( body );
        CDRNNamespaceCodec::WriteStatRequestHeader( writer, id, share, count );
        for ( UInt32 i=0; i<count; ++i )
            CDRNNamespaceCodec::WriteStatRequestItem( writer, paths[ first + i ], first + i < known.size() ? known[ first + i ] : CORE::CString::Empty );
        CORE::CDynamicBuffer answer;
        CDRNNamespaceResult outcome = Roundtrip( DRN_RES_NS_STAT, id, body, timeoutInMs, answer, true );
        if ( !outcome.IsOk() )
            return outcome;
        CDRNWireReader reader( answer );
        UInt32 answered = 0;
        if ( !CDRNNamespaceCodec::ReadCount( reader, 1, count, answered ) || answered != count )
            return Malformed( "stat" );
        GUCEF::vector< CDRNNamespaceEntry > batch;
        batch.reserve( count );
        for ( UInt32 i=0; i<count; ++i )
        {
            CDRNNamespaceEntry entry;
            if ( !CDRNNamespaceCodec::ReadStatItem( reader, entry ) )
                return Malformed( "stat entry" );
            entry.name = paths[ first + i ];
            batch.push_back( entry );
        }
        results.insert( results.end(), batch.begin(), batch.end() );
        return outcome;
    }

    CDRNNamespaceResult Stat( const CORE::CString& share, const CORE::CString::StringVector& paths, const CORE::CString::StringVector& known,
                              GUCEF::vector< CDRNNamespaceEntry >& results, UInt32 timeoutInMs )
    {GUCEF_TRACE;

        results.clear();
        if ( paths.empty() )
            return CDRNNamespaceResult();
        CDRNNamespaceInfo negotiated;
        CDRNNamespaceResult outcome = EnsureInfo( negotiated, timeoutInMs );
        if ( !outcome.IsOk() )
            return outcome;
        UInt32 batch = negotiated.maxStatPaths;
        UInt32 first = 0;
        while ( first < paths.size() )
        {
            UInt32 count = (UInt32) paths.size() - first < batch ? (UInt32) paths.size() - first : batch;
            outcome = StatBatch( share, paths, known, first, count, results, timeoutInMs );
            if ( DRN_NSRESULT_TOO_LARGE == outcome.code && count > 1 )
            {
                // the share has a stricter limit than the member: smaller batches
                batch = count / 2;
                continue;
            }
            if ( !outcome.IsOk() )
            {
                results.clear();
                return outcome;
            }
            first += count;
        }
        return outcome;
    }

    CDRNNamespaceResult List( const CORE::CString& share, const CORE::CString& dir, const CORE::CString& cursor, UInt32 maxEntries,
                              const CORE::CString& ifRevisionNot, CDRNNamespaceListPage& page, UInt32 timeoutInMs )
    {GUCEF_TRACE;

        page.Clear();
        CDRNNamespaceInfo negotiated;
        CDRNNamespaceResult outcome = EnsureInfo( negotiated, timeoutInMs );
        if ( !outcome.IsOk() )
            return outcome;
        if ( maxEntries > negotiated.maxPageEntries )
            maxEntries = negotiated.maxPageEntries;
        for ( UInt32 attempt=0; attempt<2; ++attempt )
        {
            UInt32 id = session->GetResources().GetNextNamespaceRequestId();
            CORE::CDynamicBuffer body;
            CDRNWireWriter writer( body );
            CDRNNamespaceCodec::WriteListRequest( writer, id, share, dir, cursor, maxEntries, ifRevisionNot );
            CORE::CDynamicBuffer answer;
            outcome = Roundtrip( DRN_RES_NS_LIST, id, body, timeoutInMs, answer, true );
            if ( DRN_NSRESULT_TOO_LARGE == outcome.code && 0 != maxEntries )
            {
                maxEntries = 0;     // the page size of the share itself
                continue;
            }
            if ( !outcome.IsOk() )
                return outcome;
            CDRNWireReader reader( answer );
            if ( outcome.IsUnchanged() )
            {
                if ( !CDRNNamespaceCodec::ReadText( reader, page.dirRevision ) )
                    return Malformed( "unchanged" );
                page.unchanged = true;
                return outcome;
            }
            UInt32 count = 0;
            if ( !CDRNNamespaceCodec::ReadListHeader( reader, page.dirRevision, page.nextCursor, count ) )
                return Malformed( "list" );
            page.entries.reserve( count );
            for ( UInt32 i=0; i<count; ++i )
            {
                CDRNNamespaceEntry entry;
                if ( !CDRNNamespaceCodec::ReadListEntry( reader, entry ) )
                {
                    page.Clear();
                    return Malformed( "list entry" );
                }
                page.entries.push_back( entry );
            }
            return outcome;
        }
        return outcome;
    }

    CDRNNamespaceResult ListAll( const CORE::CString& share, const CORE::CString& dir, GUCEF::vector< CDRNNamespaceEntry >& entries, CORE::CString& dirRevision,
                                 bool& changedDuringListing, UInt32 maxTotalEntries, UInt32 pageEntries, UInt32 maxRestarts, UInt32 timeoutInMs )
    {GUCEF_TRACE;

        // a listing is under way from its first page to its last: the member announced how many it keeps for us. A listing
        // that waits for its turn waits no longer than a page may take
        CDRNNamespaceInfo negotiated;
        CDRNNamespaceResult ready = EnsureInfo( negotiated, timeoutInMs );
        if ( !ready.IsOk() )
            return ready;
        CListingTurn listingTurn( *this, timeoutInMs );
        if ( !listingTurn.IsHeld() )
            return TimedOut( "the member's listing budget" );

        UInt32 restarts = 0;
        while ( true )
        {
            entries.clear();
            dirRevision.Clear();
            changedDuringListing = false;
            CORE::CString cursor;
            bool first = true;
            bool restart = false;
            while ( !restart )
            {
                CDRNNamespaceListPage page;
                CDRNNamespaceResult outcome = List( share, dir, cursor, pageEntries, CORE::CString::Empty, page, timeoutInMs );
                if ( DRN_NSRESULT_CURSOR_EXPIRED == outcome.code && restarts < maxRestarts )
                {
                    ++restarts;
                    restart = true;
                    continue;
                }
                if ( !outcome.IsOk() )
                    return outcome;
                if ( first )
                {
                    dirRevision = page.dirRevision;
                    first = false;
                }
                else
                if ( page.dirRevision != dirRevision )
                {
                    changedDuringListing = true;
                }
                if ( entries.size() + page.entries.size() > maxTotalEntries )
                {
                    UInt32 room = (UInt32) ( maxTotalEntries - entries.size() );
                    entries.insert( entries.end(), page.entries.begin(), page.entries.begin() + room );
                    return CDRNNamespaceResult( DRN_NSRESULT_TOO_LARGE, DRN_RESSTATUS_TOO_LARGE, "the directory holds more entries than the bound of the call" );
                }
                entries.insert( entries.end(), page.entries.begin(), page.entries.end() );
                if ( page.IsLast() )
                    return CDRNNamespaceResult();
                if ( page.nextCursor == cursor )
                    return CDRNNamespaceResult( DRN_NSRESULT_INTEGRITY, DRN_RESSTATUS_PROTOCOL_ERROR, "the member gave the same cursor twice" );
                cursor = page.nextCursor;
            }
        }
    }
};

/*-------------------------------------------------------------------------*/

/** One call of the client, queued on the pool */
class CDRNNamespaceClient::CCallJob : public CIDRNResourceIoJob
{
    public:

    enum TOperation { OP_NEGOTIATE, OP_SHARES, OP_STAT, OP_LIST, OP_LISTALL };

    TStatePtr state;
    CDRNNamespaceResponsePtr response;
    TOperation operation;
    CORE::CString share;
    CORE::CString dir;
    CORE::CString cursor;
    CORE::CString ifRevisionNot;
    CORE::CString::StringVector paths;
    CORE::CString::StringVector known;
    UInt32 maxEntries;
    UInt32 maxTotalEntries;
    UInt32 maxRestarts;
    UInt32 timeoutInMs;

    CCallJob( const TStatePtr& s, TOperation op )
        : CIDRNResourceIoJob(), state( s ), response(), operation( op ), share(), dir(), cursor(), ifRevisionNot(), paths(), known()
        , maxEntries( 0 ), maxTotalEntries( 0 ), maxRestarts( 0 ), timeoutInMs( 0 )
    {GUCEF_TRACE;
    }

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CDRNNamespaceResponse& out = *response;
        switch ( operation )
        {
            case OP_NEGOTIATE:  { out.result = state->Negotiate( out.info, timeoutInMs ); break; }
            case OP_SHARES:     { out.result = state->Shares( out.shares, timeoutInMs ); break; }
            case OP_STAT:       { out.result = state->Stat( share, paths, known, out.entries, timeoutInMs ); break; }
            case OP_LIST:       { out.result = state->List( share, dir, cursor, maxEntries, ifRevisionNot, out.page, timeoutInMs ); break; }
            case OP_LISTALL:    { out.result = state->ListAll( share, dir, out.entries, out.dirRevision, out.changedDuringListing, maxTotalEntries, maxEntries, maxRestarts, timeoutInMs ); break; }
            default:            { break; }
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNNamespaceClient::DefaultThreadPoolName = "GUCEF.DRN.NamespaceClient";

/*-------------------------------------------------------------------------*/

CDRNNamespaceClient::CDRNNamespaceClient( CDRNSession& session, UInt32 peerId )
    : m_state( GUCEF_NEW CState( session, peerId ) )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNNamespaceClient::~CDRNNamespaceClient()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetPeerId( void ) const
{GUCEF_TRACE;

    return m_state->peerId;
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceClient::SetThreadPoolName( const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    m_state->threadPoolName = threadPoolName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNNamespaceClient::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    return m_state->threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceClient::SetDefaultTimeoutMs( UInt32 timeoutInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    m_state->defaultTimeoutMs = timeoutInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetDefaultTimeoutMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    return m_state->defaultTimeoutMs;
}

/*-------------------------------------------------------------------------*/

CDRNNamespaceResult
CDRNNamespaceClient::Negotiate( CDRNNamespaceInfo& info, UInt32 timeoutInMs )
{GUCEF_TRACE;

    return m_state->Negotiate( info, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceClient::GetNegotiatedInfo( CDRNNamespaceInfo& info ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    if ( !m_state->haveInfo )
        return false;
    info = m_state->info;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetRequestLimit( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    return m_state->haveInfo ? m_state->info.maxNamespaceRequests : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetListingLimit( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    return m_state->haveInfo ? m_state->info.maxNamespaceListings : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetOpenHandleLimit( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    return m_state->haveInfo ? m_state->info.maxOpenHandles : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetActiveReadLimit( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    return m_state->haveInfo ? m_state->info.maxActiveReads : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetUploadRateLimit( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    return m_state->haveInfo ? m_state->info.maxUploadBytesPerSecond : 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceClient::SetSelfThrottle( bool enabled )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    m_state->selfThrottle = enabled;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceClient::GetSelfThrottle( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_state->lock );
    return m_state->selfThrottle;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetRequestsInFlight( void ) const
{GUCEF_TRACE;

    return m_state->requestGate.GetInFlight();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetRequestsWaiting( void ) const
{GUCEF_TRACE;

    return m_state->requestGate.GetWaiting();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetPeakRequestsInFlight( void ) const
{GUCEF_TRACE;

    return m_state->requestGate.GetPeakInFlight();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetListingsInFlight( void ) const
{GUCEF_TRACE;

    return m_state->listingGate.GetInFlight();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetListingsWaiting( void ) const
{GUCEF_TRACE;

    return m_state->listingGate.GetWaiting();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNNamespaceClient::GetPeakListingsInFlight( void ) const
{GUCEF_TRACE;

    return m_state->listingGate.GetPeakInFlight();
}

/*-------------------------------------------------------------------------*/

CDRNNamespaceResult
CDRNNamespaceClient::Shares( GUCEF::vector< CDRNNamespaceShareInfo >& shares, UInt32 timeoutInMs )
{GUCEF_TRACE;

    return m_state->Shares( shares, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

CDRNNamespaceResult
CDRNNamespaceClient::Stat( const CORE::CString& share, const CORE::CString::StringVector& paths, const CORE::CString::StringVector& knownRevisions,
                           TEntryVector& results, UInt32 timeoutInMs )
{GUCEF_TRACE;

    return m_state->Stat( share, paths, knownRevisions, results, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

CDRNNamespaceResult
CDRNNamespaceClient::List( const CORE::CString& share, const CORE::CString& dir, const CORE::CString& cursor, UInt32 maxEntries,
                           const CORE::CString& ifRevisionNot, CDRNNamespaceListPage& page, UInt32 timeoutInMs )
{GUCEF_TRACE;

    return m_state->List( share, dir, cursor, maxEntries, ifRevisionNot, page, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

CDRNNamespaceResult
CDRNNamespaceClient::ListAll( const CORE::CString& share, const CORE::CString& dir, TEntryVector& entries, CORE::CString& dirRevision,
                              bool& changedDuringListing, UInt32 maxTotalEntries, UInt32 pageEntries, UInt32 maxRestarts, UInt32 timeoutInMs )
{GUCEF_TRACE;

    return m_state->ListAll( share, dir, entries, dirRevision, changedDuringListing, maxTotalEntries, pageEntries, maxRestarts, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNNamespaceClient::Queue( CCallJob* job, CDRNNamespaceResponsePtr& response )
{GUCEF_TRACE;

    response = CDRNNamespaceResponsePtr( GUCEF_NEW CDRNNamespaceResponse() );
    job->response = response;
    CIDRNResourceIoJobPtr jobPtr( job );
    CORE::CString poolName = GetThreadPoolName();
    if ( poolName.IsNULLOrEmpty() )
        poolName = DefaultThreadPoolName;
    if ( DefaultThreadPoolName == poolName )
    {
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( poolName );
        if ( !pool.IsNULL() && 0 == pool->GetDesiredMinNrOfWorkerThreads() )
            pool->SetDesiredMinNrOfWorkerThreads( 4 );
    }
    CORE::CFutureResult future = CDRNResourceIoTaskConsumer::Queue( poolName, jobPtr );
    if ( future.HasNoFuture() )
        response->result = CDRNNamespaceResult( DRN_NSRESULT_UNAVAILABLE, DRN_RESSTATUS_BUSY, "no thread pool to run the call on" );
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNNamespaceClient::NegotiateAsync( CDRNNamespaceResponsePtr& response, UInt32 timeoutInMs )
{GUCEF_TRACE;

    CCallJob* job = GUCEF_NEW CCallJob( m_state, CCallJob::OP_NEGOTIATE );
    job->timeoutInMs = timeoutInMs;
    return Queue( job, response );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNNamespaceClient::SharesAsync( CDRNNamespaceResponsePtr& response, UInt32 timeoutInMs )
{GUCEF_TRACE;

    CCallJob* job = GUCEF_NEW CCallJob( m_state, CCallJob::OP_SHARES );
    job->timeoutInMs = timeoutInMs;
    return Queue( job, response );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNNamespaceClient::StatAsync( const CORE::CString& share, const CORE::CString::StringVector& paths, const CORE::CString::StringVector& knownRevisions,
                                CDRNNamespaceResponsePtr& response, UInt32 timeoutInMs )
{GUCEF_TRACE;

    CCallJob* job = GUCEF_NEW CCallJob( m_state, CCallJob::OP_STAT );
    job->share = share;
    job->paths = paths;
    job->known = knownRevisions;
    job->timeoutInMs = timeoutInMs;
    return Queue( job, response );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNNamespaceClient::ListAsync( const CORE::CString& share, const CORE::CString& dir, const CORE::CString& cursor, UInt32 maxEntries,
                                const CORE::CString& ifRevisionNot, CDRNNamespaceResponsePtr& response, UInt32 timeoutInMs )
{GUCEF_TRACE;

    CCallJob* job = GUCEF_NEW CCallJob( m_state, CCallJob::OP_LIST );
    job->share = share;
    job->dir = dir;
    job->cursor = cursor;
    job->maxEntries = maxEntries;
    job->ifRevisionNot = ifRevisionNot;
    job->timeoutInMs = timeoutInMs;
    return Queue( job, response );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDRNNamespaceClient::ListAllAsync( const CORE::CString& share, const CORE::CString& dir, CDRNNamespaceResponsePtr& response,
                                   UInt32 maxTotalEntries, UInt32 pageEntries, UInt32 maxRestarts, UInt32 timeoutInMs )
{GUCEF_TRACE;

    CCallJob* job = GUCEF_NEW CCallJob( m_state, CCallJob::OP_LISTALL );
    job->share = share;
    job->dir = dir;
    job->maxEntries = pageEntries;
    job->maxTotalEntries = maxTotalEntries;
    job->maxRestarts = maxRestarts;
    job->timeoutInMs = timeoutInMs;
    return Queue( job, response );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
