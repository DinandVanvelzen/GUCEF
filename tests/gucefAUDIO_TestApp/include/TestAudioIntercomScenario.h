/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOINTERCOMSCENARIO_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOINTERCOMSCENARIO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestAudioIntercomUnit.h"
#include "TestAudioSimulatedVoiceLink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace VoiceLinkTest {

using namespace GUCEF;

class CTestIntercomScenario;

/** Called after every step of a scenario (measurements that follow the output as it is made) */
class ITestScenarioObserver
{
    public:

    virtual ~ITestScenarioObserver() {}

    virtual void OnStep( CTestIntercomScenario& scenario ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  Units of the intercom and the simulated links between them in one simulated clock (section 6.2 of the design).
 *  Every step is one playback period (of the simulated time; a unit whose crystal drifts renders that much more or
 *  less), in this order:
 *   1. every unit records the step: its capture device renders the period from the unit's signal and echo path; the
 *      capture path and the sender run as on the capture thread
 *   2. the frames the senders made go into the links of their unit, sent at the time each was complete
 *   3. the simulated time moves to the end of the step, the clocks of the units follow (with their drift)
 *   4. the links hand what has arrived to their receivers, with the arrival time in the receiving unit's clock
 *   5. every unit plays the step: its playback device renders the mix (the block plays during the next period), then
 *      the engine's Update() runs
 *   6. the observers
 *  The scenario owns its units and links.
 */
class CTestIntercomScenario
{
    public:

    struct SRoute
    {
        CTestIntercomUnit* from;
        CTestIntercomUnit* to;
        AUDIO::CVoiceReceiver* receiver;
        CTestSimulatedVoiceLink* link;
    };

    explicit CTestIntercomScenario( UInt32 period = 240, UInt32 sampleRate = 48000 )
        : m_period( period ), m_rate( sampleRate ), m_nowInUs( 0.0 ), m_steps( 0 ), units(), routes(), m_observers(), m_sent()
    {}

    ~CTestIntercomScenario()
    {
        for ( size_t i=0; i<routes.size(); ++i )
            GUCEF_DELETE routes[ i ].link;
        routes.clear();
        for ( size_t i=0; i<units.size(); ++i )
            GUCEF_DELETE units[ i ];
        units.clear();
    }

    /** A new unit, initialised; null when its engine could not be made */
    CTestIntercomUnit* AddUnit( SUnitConfig config )
    {
        config.sampleRate = m_rate;
        config.period = m_period;
        CTestIntercomUnit* unit = GUCEF_NEW CTestIntercomUnit( config );
        if ( !unit->Init() )
        {
            GUCEF_DELETE unit;
            return GUCEF_NULL;
        }
        unit->SetTime( m_nowInUs );
        units.push_back( unit );
        return unit;
    }

    /** A link from the sender of 'from' to 'receiver' (a receiver of 'to') */
    CTestSimulatedVoiceLink* Connect( CTestIntercomUnit* from, CTestIntercomUnit* to, AUDIO::CVoiceReceiver* receiver, const SLinkModel& model )
    {
        SRoute route;
        route.from = from;
        route.to = to;
        route.receiver = receiver;
        route.link = GUCEF_NEW CTestSimulatedVoiceLink( model );
        if ( GUCEF_NULL != receiver )
            route.link->SetRedPayloadType( receiver->GetConfig().GetRedPayloadType() );
        routes.push_back( route );
        return route.link;
    }

    void AddObserver( ITestScenarioObserver* observer ) { m_observers.push_back( observer ); }

    void Step( void )
    {
        for ( size_t u=0; u<units.size(); ++u )
            units[ u ]->Capture( m_period );

        for ( size_t u=0; u<units.size(); ++u )
        {
            m_sent.clear();
            units[ u ]->PopFrames( m_sent );
            for ( size_t r=0; r<routes.size(); ++r )
                if ( routes[ r ].from == units[ u ] )
                    for ( size_t f=0; f<m_sent.size(); ++f )
                        routes[ r ].link->Send( m_sent[ f ].frame, m_sent[ f ].sendTimeInUs );
        }

        ++m_steps;
        m_nowInUs = (Float64) m_steps * (Float64) m_period * 1000000.0 / (Float64) m_rate;
        for ( size_t u=0; u<units.size(); ++u )
            units[ u ]->SetTime( m_nowInUs );

        for ( size_t r=0; r<routes.size(); ++r )
            if ( GUCEF_NULL != routes[ r ].receiver )
                routes[ r ].link->Deliver( m_nowInUs, *routes[ r ].receiver, routes[ r ].to->LocalScale(), routes[ r ].to->LocalOffsetUs() );

        for ( size_t u=0; u<units.size(); ++u )
        {
            units[ u ]->Play( m_period );
            units[ u ]->engine->Update();
        }

        for ( size_t o=0; o<m_observers.size(); ++o )
            m_observers[ o ]->OnStep( *this );
    }

    void Run( Float64 seconds )
    {
        const UInt64 end = m_steps + (UInt64) ( seconds * (Float64) m_rate / (Float64) m_period + 0.5 );
        while ( m_steps < end )
            Step();
    }

    Float64 NowInUs( void ) const { return m_nowInUs; }
    Float64 NowInS( void ) const { return m_nowInUs / 1000000.0; }
    UInt64 GetStepCount( void ) const { return m_steps; }
    UInt32 GetPeriod( void ) const { return m_period; }
    UInt32 GetSampleRate( void ) const { return m_rate; }

    private:

    CTestIntercomScenario( const CTestIntercomScenario& );
    CTestIntercomScenario& operator=( const CTestIntercomScenario& );

    UInt32 m_period;
    UInt32 m_rate;
    Float64 m_nowInUs;
    UInt64 m_steps;

    public:

    GUCEF::vector< CTestIntercomUnit* > units;
    GUCEF::vector< SRoute > routes;

    private:

    GUCEF::vector< ITestScenarioObserver* > m_observers;
    TSentFrames m_sent;
};

/*-------------------------------------------------------------------------*/

}; /* namespace VoiceLinkTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOINTERCOMSCENARIO_H ? */
