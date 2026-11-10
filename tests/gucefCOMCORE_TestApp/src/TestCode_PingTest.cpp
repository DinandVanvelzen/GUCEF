/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCode_Pump.h"

#include "TestCode_PingTest.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::COMCORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** records the events of a ping sequence */
class CPingRecorder : public CORE::CObserver
{
    public:

    CPingRecorder( void )
        : CORE::CObserver()
        , m_ping()
        , m_started( 0 )
        , m_responses( 0 )
        , m_timeouts( 0 )
        , m_failures( 0 )
        , m_stopped( 0 )
        , m_lastRoundTripTime( 0 )
    {GUCEF_TRACE;

        SubscribeTo( &m_ping );
    }

    virtual ~CPingRecorder() { UnsubscribeAllFromObserver(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CPingRecorder";
        return classTypeName;
    }

    CPing m_ping;
    CORE::UInt32 m_started;
    CORE::UInt32 m_responses;
    CORE::UInt32 m_timeouts;
    CORE::UInt32 m_failures;
    CORE::UInt32 m_stopped;
    CORE::UInt32 m_lastRoundTripTime;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( eventid == CPing::PingStartedEvent )
        {
            ++m_started;
        }
        else
        if ( eventid == CPing::PingReponseEvent )
        {
            ++m_responses;
            CPing::CPingEventData* data = static_cast< CPing::CPingEventData* >( eventdata );
            if ( GUCEF_NULL != data )
                m_lastRoundTripTime = data->GetRoundTripTime();
        }
        else
        if ( eventid == CPing::PingTimeoutEvent )
        {
            ++m_timeouts;
        }
        else
        if ( eventid == CPing::PingFailedEvent )
        {
            ++m_failures;
        }
        else
        if ( eventid == CPing::PingStoppedEvent )
        {
            ++m_stopped;
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Ping" );

    GUCEF_TESTFW_TESTCASE( "Test: The loopback address answers a ping" )
        try
        {
            CPingRecorder recorder;
            ASSERT_TRUE( recorder.m_ping.Start( "127.0.0.1", 1, 32, 1000 ) );

            ASSERT_TRUE( PumpUntil( [&recorder]() { return recorder.m_stopped > 0; }, 10000 ) );
            ASSERT_TRUE( 1 == recorder.m_started );
            ASSERT_TRUE( 1 == recorder.m_responses );
            ASSERT_TRUE( 0 == recorder.m_timeouts );
            ASSERT_TRUE( 0 == recorder.m_failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
