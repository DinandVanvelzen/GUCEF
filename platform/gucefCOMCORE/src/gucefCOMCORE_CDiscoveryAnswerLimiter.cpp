/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCOMCORE_CDiscoveryAnswerLimiter.h"

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

const UInt32 CDiscoveryAnswerLimiter::DefaultAnswersPerSourcePerMinute = 60;
const UInt32 CDiscoveryAnswerLimiter::DefaultAnswerBurstPerSource = 8;
const UInt32 CDiscoveryAnswerLimiter::DefaultAnswersPerSecond = 50;
const UInt32 CDiscoveryAnswerLimiter::DefaultAmplificationFactor = 1;
const UInt32 CDiscoveryAnswerLimiter::DefaultMaxTrackedSources = 256;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDiscoveryAnswerLimiter::SStats::SStats( void )
    : admitted( 0 )
    , refusedSource( 0 )
    , refusedSourceRate( 0 )
    , refusedGlobalRate( 0 )
    , answersWithheld( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDiscoveryAnswerLimiter::SBucket::SBucket( void )
    : milliTokens( 0 )
    , lastRefillInMs( 0 )
    , lastSeenInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDiscoveryAnswerLimiter::CDiscoveryAnswerLimiter( void )
    : CORE::CIConfigurable()
    , m_answersPerSourcePerMinute( DefaultAnswersPerSourcePerMinute )
    , m_answerBurstPerSource( DefaultAnswerBurstPerSource )
    , m_answersPerSecond( DefaultAnswersPerSecond )
    , m_amplificationFactor( DefaultAmplificationFactor )
    , m_maxTrackedSources( DefaultMaxTrackedSources )
    , m_sources()
    , m_global()
    , m_globalInitialized( false )
    , m_stats()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDiscoveryAnswerLimiter::CDiscoveryAnswerLimiter( const CDiscoveryAnswerLimiter& src )
    : CORE::CIConfigurable( src )
    , m_answersPerSourcePerMinute( src.m_answersPerSourcePerMinute )
    , m_answerBurstPerSource( src.m_answerBurstPerSource )
    , m_answersPerSecond( src.m_answersPerSecond )
    , m_amplificationFactor( src.m_amplificationFactor )
    , m_maxTrackedSources( src.m_maxTrackedSources )
    , m_sources()
    , m_global()
    , m_globalInitialized( false )
    , m_stats()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDiscoveryAnswerLimiter::~CDiscoveryAnswerLimiter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDiscoveryAnswerLimiter&
CDiscoveryAnswerLimiter::operator=( const CDiscoveryAnswerLimiter& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        // the settings travel, what was seen stays with the object that saw it
        UInt32 answersPerSourcePerMinute = src.GetAnswersPerSourcePerMinute();
        UInt32 answerBurstPerSource = src.GetAnswerBurstPerSource();
        UInt32 answersPerSecond = src.GetAnswersPerSecond();
        UInt32 amplificationFactor = src.GetAmplificationFactor();
        UInt32 maxTrackedSources = src.GetMaxTrackedSources();

        MT::CScopeMutex lock( m_lock );
        m_answersPerSourcePerMinute = answersPerSourcePerMinute;
        m_answerBurstPerSource = answerBurstPerSource;
        m_answersPerSecond = answersPerSecond;
        m_amplificationFactor = amplificationFactor;
        m_maxTrackedSources = maxTrackedSources;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryAnswerLimiter::SetAnswersPerSourcePerMinute( UInt32 answersPerMinute )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_answersPerSourcePerMinute = answersPerMinute;
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryAnswerLimiter::GetAnswersPerSourcePerMinute( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_answersPerSourcePerMinute;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryAnswerLimiter::SetAnswerBurstPerSource( UInt32 burst )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_answerBurstPerSource = burst;
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryAnswerLimiter::GetAnswerBurstPerSource( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_answerBurstPerSource;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryAnswerLimiter::SetAnswersPerSecond( UInt32 answersPerSecond )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_answersPerSecond = answersPerSecond;
    m_globalInitialized = false;
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryAnswerLimiter::GetAnswersPerSecond( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_answersPerSecond;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryAnswerLimiter::SetAmplificationFactor( UInt32 factor )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_amplificationFactor = factor;
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryAnswerLimiter::GetAmplificationFactor( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_amplificationFactor;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryAnswerLimiter::SetMaxTrackedSources( UInt32 maxSources )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxTrackedSources = maxSources;
    if ( m_maxTrackedSources > 0 )
    {
        while ( m_sources.size() > m_maxTrackedSources )
            MakeRoom();
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryAnswerLimiter::GetMaxTrackedSources( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxTrackedSources;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryAnswerLimiter::IsAcceptableSource( const CIPv4Address& source )
{GUCEF_TRACE;

    if ( 0 == source.GetPortInHostByteOrder() )
        return false;
    UInt32 address = source.GetAddressInHostByteOrder();
    if ( 0 == address || 0xFFFFFFFF == address )
        return false;
    // 224.0.0.0/4 multicast and 240.0.0.0/4 reserved: never the address of a single asker
    if ( ( address & 0xE0000000 ) == 0xE0000000 )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryAnswerLimiter::Refill( TBucket& bucket, UInt64 nowInMs, UInt64 tokensPerMinute, UInt64 capacityInTokens )
{GUCEF_TRACE;

    UInt64 capacity = capacityInTokens * 1000;
    if ( nowInMs > bucket.lastRefillInMs )
    {
        UInt64 elapsed = nowInMs - bucket.lastRefillInMs;
        // a long silence fills the bucket anyway: no need to multiply what would overflow
        UInt64 gain = elapsed >= 60000 ? capacity : ( elapsed * tokensPerMinute * 1000 ) / 60000;
        bucket.milliTokens = bucket.milliTokens + gain > capacity ? capacity : bucket.milliTokens + gain;
        if ( gain > 0 || bucket.milliTokens >= capacity )
            bucket.lastRefillInMs = nowInMs;
    }
    else
    if ( nowInMs < bucket.lastRefillInMs )
    {
        // a clock that went back: start again from now
        bucket.lastRefillInMs = nowInMs;
    }
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryAnswerLimiter::MakeRoom( void )
{GUCEF_TRACE;

    // the source heard longest ago goes
    TBucketMap::iterator oldest = m_sources.end();
    TBucketMap::iterator i = m_sources.begin();
    while ( i != m_sources.end() )
    {
        if ( oldest == m_sources.end() || (*i).second.lastSeenInMs < (*oldest).second.lastSeenInMs )
            oldest = i;
        ++i;
    }
    if ( oldest != m_sources.end() )
        m_sources.erase( oldest );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryAnswerLimiter::AdmitQuery( const CIPv4Address& source, UInt64 nowInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !IsAcceptableSource( source ) )
    {
        ++m_stats.refusedSource;
        return false;
    }

    // the process bucket first: when it is empty nothing is answered, and nobody's own bucket is touched
    if ( m_answersPerSecond > 0 )
    {
        if ( !m_globalInitialized )
        {
            m_global.milliTokens = (UInt64) m_answersPerSecond * 1000;
            m_global.lastRefillInMs = nowInMs;
            m_globalInitialized = true;
        }
        Refill( m_global, nowInMs, (UInt64) m_answersPerSecond * 60, m_answersPerSecond );
        if ( m_global.milliTokens < 1000 )
        {
            ++m_stats.refusedGlobalRate;
            return false;
        }
    }

    if ( m_answersPerSourcePerMinute > 0 && m_answerBurstPerSource > 0 )
    {
        UInt32 key = source.GetAddress();
        TBucketMap::iterator i = m_sources.find( key );
        if ( i == m_sources.end() )
        {
            if ( m_maxTrackedSources > 0 && m_sources.size() >= m_maxTrackedSources )
                MakeRoom();
            TBucket& bucket = m_sources[ key ];
            bucket.milliTokens = (UInt64) m_answerBurstPerSource * 1000;
            bucket.lastRefillInMs = nowInMs;
            i = m_sources.find( key );
        }
        TBucket& bucket = (*i).second;
        bucket.lastSeenInMs = nowInMs;
        Refill( bucket, nowInMs, m_answersPerSourcePerMinute, m_answerBurstPerSource );
        if ( bucket.milliTokens < 1000 )
        {
            ++m_stats.refusedSourceRate;
            return false;
        }
        bucket.milliTokens -= 1000;
    }

    if ( m_answersPerSecond > 0 )
        m_global.milliTokens -= 1000;
    ++m_stats.admitted;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryAnswerLimiter::GetAnswerBudget( UInt32 querySizeInBytes, bool sourceIsVerified ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( sourceIsVerified || 0 == m_amplificationFactor )
        return 0xFFFFFFFF;
    UInt64 budget = (UInt64) querySizeInBytes * m_amplificationFactor;
    return budget > 0xFFFFFFFF ? 0xFFFFFFFF : (UInt32) budget;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryAnswerLimiter::CountAnswerWithheld( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    ++m_stats.answersWithheld;
}

/*-------------------------------------------------------------------------*/

CDiscoveryAnswerLimiter::TStats
CDiscoveryAnswerLimiter::GetStats( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_stats;
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryAnswerLimiter::GetTrackedSourceCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_sources.size();
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryAnswerLimiter::Reset( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_sources.clear();
    m_globalInitialized = false;
    m_stats = TStats();
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryAnswerLimiter::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    config.SetAttribute( "answersPerSourcePerMinute", m_answersPerSourcePerMinute );
    config.SetAttribute( "answerBurstPerSource", m_answerBurstPerSource );
    config.SetAttribute( "answersPerSecond", m_answersPerSecond );
    config.SetAttribute( "amplificationFactor", m_amplificationFactor );
    config.SetAttribute( "maxTrackedSources", m_maxTrackedSources );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryAnswerLimiter::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    SetAnswersPerSourcePerMinute( config.GetAttributeValueOrChildValueByName( "answersPerSourcePerMinute" ).AsUInt32( GetAnswersPerSourcePerMinute(), true ) );
    SetAnswerBurstPerSource( config.GetAttributeValueOrChildValueByName( "answerBurstPerSource" ).AsUInt32( GetAnswerBurstPerSource(), true ) );
    SetAnswersPerSecond( config.GetAttributeValueOrChildValueByName( "answersPerSecond" ).AsUInt32( GetAnswersPerSecond(), true ) );
    SetAmplificationFactor( config.GetAttributeValueOrChildValueByName( "amplificationFactor" ).AsUInt32( GetAmplificationFactor(), true ) );
    SetMaxTrackedSources( config.GetAttributeValueOrChildValueByName( "maxTrackedSources" ).AsUInt32( GetMaxTrackedSources(), true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDiscoveryAnswerLimiter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CDiscoveryAnswerLimiter";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
