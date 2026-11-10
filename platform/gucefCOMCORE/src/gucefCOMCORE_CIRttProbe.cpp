/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefCOMCORE_CIRttProbe.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIRttProbe::ProbeResultEvent = "GUCEF::COMCORE::CIRttProbe::ProbeResultEvent";
const CORE::CEvent CIRttProbe::ProbeTimeoutEvent = "GUCEF::COMCORE::CIRttProbe::ProbeTimeoutEvent";
const CORE::CEvent CIRttProbe::ProbeFailedEvent = "GUCEF::COMCORE::CIRttProbe::ProbeFailedEvent";
const CORE::CEvent CIRttProbe::ProbeFinishedEvent = "GUCEF::COMCORE::CIRttProbe::ProbeFinishedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRttProbeEventData::CRttProbeEventData( void )
    : CORE::CICloneable()
    , m_target()
    , m_rttInMicroSecs( 0 )
    , m_sequence( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRttProbeEventData::CRttProbeEventData( const CHostAddress& target ,
                                        UInt64 rttInMicroSecs      ,
                                        UInt32 sequence            )
    : CORE::CICloneable()
    , m_target( target )
    , m_rttInMicroSecs( rttInMicroSecs )
    , m_sequence( sequence )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRttProbeEventData::CRttProbeEventData( const CRttProbeEventData& src )
    : CORE::CICloneable( src )
    , m_target( src.m_target )
    , m_rttInMicroSecs( src.m_rttInMicroSecs )
    , m_sequence( src.m_sequence )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRttProbeEventData::~CRttProbeEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRttProbeEventData&
CRttProbeEventData::operator=( const CRttProbeEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_target = src.m_target;
        m_rttInMicroSecs = src.m_rttInMicroSecs;
        m_sequence = src.m_sequence;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRttProbeEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRttProbeEventData( *this );
}

/*-------------------------------------------------------------------------*/

const CHostAddress&
CRttProbeEventData::GetTarget( void ) const
{GUCEF_TRACE;

    return m_target;
}

/*-------------------------------------------------------------------------*/

UInt64
CRttProbeEventData::GetRttInMicroSecs( void ) const
{GUCEF_TRACE;

    return m_rttInMicroSecs;
}

/*-------------------------------------------------------------------------*/

UInt32
CRttProbeEventData::GetSequence( void ) const
{GUCEF_TRACE;

    return m_sequence;
}

/*-------------------------------------------------------------------------*/

void
CIRttProbe::RegisterEvents( void )
{GUCEF_TRACE;

    ProbeResultEvent.Initialize();
    ProbeTimeoutEvent.Initialize();
    ProbeFailedEvent.Initialize();
    ProbeFinishedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIRttProbe::CIRttProbe( void )
    : CORE::CObservingNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIRttProbe::~CIRttProbe()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CIRttProbe::GetTimeInMicroSecs( void )
{GUCEF_TRACE;

    UInt64 resolution = MT::PrecisionTimerResolution();
    UInt64 ticks = MT::PrecisionTickCount();
    if ( resolution >= 1000000 )
        return ticks / ( resolution / 1000000 );
    if ( 0 == resolution )
        return ticks;
    return ticks * ( 1000000 / resolution );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
