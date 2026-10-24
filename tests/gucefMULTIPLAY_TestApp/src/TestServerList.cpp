/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_COMCORE_CPING_H
#include "CPing.h"
#define GUCEF_COMCORE_CPING_H
#endif /* GUCEF_COMCORE_CPING_H ? */

#ifndef GUCEF_MULTIPLAY_CSERVERLIST_H
#include "gucefMULTIPLAY_CServerList.h"
#define GUCEF_MULTIPLAY_CSERVERLIST_H
#endif /* GUCEF_MULTIPLAY_CSERVERLIST_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestServerList.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Counts the answers of a ping */
class CPingAnswers : public CORE::CObservingNotifier
{
    public:

    UInt32 answers;
    UInt32 failures;

    CPingAnswers( COMCORE::CPing& ping )
        : CORE::CObservingNotifier(), answers( 0 ), failures( 0 )
    {
        SubscribeTo( &ping );
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == COMCORE::CPing::PingReponseEvent )
            ++answers;
        else
        if ( eventid == COMCORE::CPing::PingFailedEvent || eventid == COMCORE::CPing::PingTimeoutEvent )
            ++failures;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
Pump( UInt32 timeInMs )
{
    for ( UInt32 waited=0; waited<timeInMs; waited+=5 )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 5 );
    }
}

/*-------------------------------------------------------------------------*/

/** Whether this machine can ping at all: ICMP needs privileges on some platforms */
static bool
CanPingLoopback( void )
{
    COMCORE::CPing ping;
    CPingAnswers counter( ping );
    if ( !ping.Start( "127.0.0.1", 1, 32, 1000, 100 ) )
        return false;
    for ( UInt32 waited=0; waited<3000 && 0 == counter.answers + counter.failures; waited+=5 )
        Pump( 5 );
    ping.Stop();
    return counter.answers > 0;
}

/*-------------------------------------------------------------------------*/

static Int32
PingOf( const MULTIPLAY::CServerList& list, const CORE::CString& name )
{
    const MULTIPLAY::CServerList::TServerInfoList& entries = list.GetList();
    for ( size_t i=0; i<entries.size(); ++i )
        if ( name == entries[ i ]->serverInfo.name )
            return entries[ i ]->ping;
    return -2;
}

/*-------------------------------------------------------------------------*/

void
PerformServerListTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING server list TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "ServerList" );

    const bool canPing = CanPingLoopback();

    GUCEF_TESTFW_TESTCASE( "Test: Refreshing the pings of a server list gives each reachable server its ping" )
        GUCEF_TESTFW_SKIP_IF( !canPing, "ICMP ping is not available on this machine" );
        try
        {
            MULTIPLAY::CServerList list;
            MULTIPLAY::CServerList::TServerInfoList servers;
            const char* names[] = { "first", "second", "third", "fourth", "fifth", "sixth" };
            for ( UInt32 i=0; i<6; ++i )
            {
                MULTIPLAY::CServerList::TServerListEntry* entry = new MULTIPLAY::CServerList::TServerListEntry;
                entry->serverInfo.name = names[ i ];
                entry->serverInfo.hostAddress = COMCORE::CHostAddress( "127.0.0.1", 4000 + i );
                entry->ping = -1;
                servers.push_back( MULTIPLAY::CServerList::TServerListEntryPtr( entry ) );
            }
            list.SetList( servers );

            // more servers than parallel pings: every server is pinged in turn
            ASSERT_TRUE( list.SetParallelPingMax( 2 ) );
            list.RefreshPings();
            ASSERT_TRUE( list.IsBusyWithPing() );
            ASSERT_FALSE( list.SetParallelPingMax( 3 ) );     // not while pinging

            for ( UInt32 waited=0; waited<10000 && list.IsBusyWithPing(); waited+=5 )
                Pump( 5 );
            ASSERT_FALSE( list.IsBusyWithPing() );
            for ( UInt32 i=0; i<6; ++i )
                ASSERT_TRUE( PingOf( list, names[ i ] ) >= 0 );

            // a copy pings by itself too
            MULTIPLAY::CServerList copy( list );
            copy.RefreshPings();
            for ( UInt32 waited=0; waited<10000 && copy.IsBusyWithPing(); waited+=5 )
                Pump( 5 );
            ASSERT_FALSE( copy.IsBusyWithPing() );
            ASSERT_TRUE( PingOf( copy, "sixth" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A server list that is destroyed while it pings leaves nothing running" )
        GUCEF_TESTFW_SKIP_IF( !canPing, "ICMP ping is not available on this machine" );
        try
        {
            {
                MULTIPLAY::CServerList list;
                MULTIPLAY::CServerList::TServerInfoList servers;
                MULTIPLAY::CServerList::TServerListEntry* entry = new MULTIPLAY::CServerList::TServerListEntry;
                entry->serverInfo.name = "only";
                entry->serverInfo.hostAddress = COMCORE::CHostAddress( "127.0.0.1", 4000 );
                entry->ping = -1;
                servers.push_back( MULTIPLAY::CServerList::TServerListEntryPtr( entry ) );
                list.SetList( servers );
                list.RefreshPings();
            }
            // the pulses after the list is gone must not reach it
            Pump( 200 );
            ASSERT_TRUE( true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
