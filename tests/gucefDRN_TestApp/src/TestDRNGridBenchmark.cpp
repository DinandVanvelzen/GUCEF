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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>
#include <time.h>

#if defined( _WIN32 )
  #include <windows.h>
#endif

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNGridBenchmark.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNTEST {

using namespace GUCEF;

/** Operations per measured loop of the single call measurements */
static const UInt32 BenchmarkOperationCount = 100000;

/** Keys changed per round of the delta measurement, and rounds */
static const UInt32 BenchmarkDeltaKeyCount = 20000;
static const UInt32 BenchmarkDeltaRounds = 3;

/**
 *  Pulses of a delta round at the least. The two nodes pulse in the order of their addresses (the observers of a
 *  notifier), so the replica has the keys after the first pulse or after the second, by chance of the heap: a round
 *  that always takes this many pulses does the same work in either order, which keeps the CPU time of benchmarks
 *  of separate members comparable
 */
static const UInt32 BenchmarkDeltaMinPulses = 4;

/*-------------------------------------------------------------------------*/

static UInt64
Median( GUCEF::vector< UInt64 >& samples )
{
    if ( samples.empty() )
        return 0;
    std::sort( samples.begin(), samples.end() );
    return samples[ samples.size() / 2 ];
}

/*-------------------------------------------------------------------------*/

class CReplicaConverged : public CTestCondition
{
    public:

    CReplicaConverged( DRN::CDRNDataGrid& grid, const CORE::CVariant& key, UInt32 expected ) : m_grid( grid ), m_key( key ), m_expected( expected ) {}

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CVariant value;
        return m_grid.GetValue( m_key, value ) && value.AsUInt32() == m_expected;
    }

    private:

    DRN::CDRNDataGrid& m_grid;
    CORE::CVariant m_key;
    UInt32 m_expected;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridBenchmarkResult::CGridBenchmarkResult( void )
    : keyCount( 0 )
    , setValueNs( 0 )
    , setValueByIndexNs( 0 )
    , getValueByIndexNs( 0 )
    , replicaGetValueByIndexNs( 0 )
    , deltaNsPerKey( 0 )
    , deltaWorkNsPerKey( 0 )
    , setValueCpuNs( 0 )
    , setValueByIndexCpuNs( 0 )
    , getValueByIndexCpuNs( 0 )
    , replicaGetValueByIndexCpuNs( 0 )
    , deltaWorkCpuNsPerKey( 0 )
    , deltaPulses( 0 )
{

}

/*-------------------------------------------------------------------------*/

CORE::CString
CGridBenchmarkResult::ToString( void ) const
{
    return CORE::UInt32ToString( keyCount ) + " keys: SetValue " + CORE::UInt64ToString( setValueNs ) + " ns (CPU " + CORE::UInt64ToString( setValueCpuNs ) +
           "), SetValueByIndex " + CORE::UInt64ToString( setValueByIndexNs ) + " ns (CPU " + CORE::UInt64ToString( setValueByIndexCpuNs ) +
           "), GetValueByIndex " + CORE::UInt64ToString( getValueByIndexNs ) + " ns (CPU " + CORE::UInt64ToString( getValueByIndexCpuNs ) +
           "), replica GetValueByIndex " + CORE::UInt64ToString( replicaGetValueByIndexNs ) + " ns (CPU " + CORE::UInt64ToString( replicaGetValueByIndexCpuNs ) +
           "), delta " + CORE::UInt64ToString( deltaNsPerKey ) + " ns per changed key (" + CORE::UInt64ToString( deltaWorkNsPerKey ) + " ns of work, CPU " +
           CORE::UInt64ToString( deltaWorkCpuNsPerKey ) + ", " + CORE::UInt64ToString( deltaPulses ) + " pulses)";
}

/*-------------------------------------------------------------------------*/

UInt64
ThreadCpuTimeNs( void )
{
    #if defined( _WIN32 )

    FILETIME creationTime, exitTime, kernelTime, userTime;
    if ( 0 == ::GetThreadTimes( ::GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime ) )
        return 0;
    const UInt64 kernel = ( (UInt64) kernelTime.dwHighDateTime << 32 ) | kernelTime.dwLowDateTime;
    const UInt64 user = ( (UInt64) userTime.dwHighDateTime << 32 ) | userTime.dwLowDateTime;
    return ( kernel + user ) * 100;

    #elif defined( CLOCK_THREAD_CPUTIME_ID )

    struct timespec now;
    if ( 0 != clock_gettime( CLOCK_THREAD_CPUTIME_ID, &now ) )
        return 0;
    return (UInt64) now.tv_sec * 1000000000 + (UInt64) now.tv_nsec;

    #else

    return (UInt64) clock() * 1000000000 / CLOCKS_PER_SEC;

    #endif
}

/*-------------------------------------------------------------------------*/

CGridBenchmark::CGridBenchmark( void )
    : m_group( GUCEF_NULL )
    , m_publisherGrid( GUCEF_NULL )
    , m_replicaGrid( GUCEF_NULL )
    , m_indices()
    , m_keys()
    , m_stamp( 0 )
{

}

/*-------------------------------------------------------------------------*/

CGridBenchmark::~CGridBenchmark()
{
    GUCEF_DELETE m_group;
    m_group = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CGridBenchmark::Setup( UInt32 keyCount, const CORE::CString& name, bool declareTypes, bool allowPublicItems )
{
    m_group = GUCEF_NEW CGroup( DRN::DRN_TOPOLOGY_CLIENT_SERVER, name, "mem" );
    m_group->nodeSettings.securityPolicy.allowPublicItems = allowPublicItems;
    if ( !m_group->Start( 2 ) )
        return false;

    DRN::CDRNDataGridSettings settings;
    settings.name = name;
    settings.authority = DRN::DRN_AUTHORITY_OWNER;
    settings.sendRateHz = 0;
    settings.capacity = keyCount;
    if ( declareTypes )
    {
        settings.keyType = CORE::DATATABLE_TYPE_UINT32;
        settings.valueType = CORE::DATATABLE_TYPE_UINT32;
    }
    m_publisherGrid = (*m_group)[ 0 ].CreateGrid( settings );
    if ( GUCEF_NULL == m_publisherGrid )
        return false;

    m_indices.reserve( keyCount );
    m_keys.reserve( keyCount );
    for ( UInt32 i=0; i<keyCount; ++i )
    {
        m_keys.push_back( CORE::CVariant( i ) );
        m_indices.push_back( m_publisherGrid->GetOrAddKeyIndex( m_keys.back() ) );
        m_publisherGrid->SetValueByIndex( m_indices.back(), CORE::CVariant( (UInt32) 0 ) );
    }

    (*m_group)[ 1 ].Subscribe( name );
    UInt64 start = DRN::CDRNClock::NowInMicroseconds();
    while ( DRN::CDRNClock::NowInMicroseconds() - start < 120000000 )
    {
        PumpFor( 5 );
        m_replicaGrid = (*m_group)[ 1 ].FindGrid( name );
        if ( GUCEF_NULL != m_replicaGrid && m_replicaGrid->IsSnapshotComplete() && m_replicaGrid->GetKeyCount() == keyCount )
            break;
    }
    if ( GUCEF_NULL == m_replicaGrid || m_replicaGrid->GetKeyCount() != keyCount )
        return false;

    // the application polls: no event per key on the replica
    m_replicaGrid->SetNotifyMode( DRN::DRN_NOTIFY_POLL );
    return true;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSession&
CGridBenchmark::GetPublisherSession( void )
{
    return (*m_group)[ 0 ];
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSession&
CGridBenchmark::GetReplicaSession( void )
{
    return (*m_group)[ 1 ];
}

/*-------------------------------------------------------------------------*/

DRN::CDRNDataGrid*
CGridBenchmark::GetPublisherGrid( void ) const
{
    return m_publisherGrid;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNDataGrid*
CGridBenchmark::GetReplicaGrid( void ) const
{
    return m_replicaGrid;
}

/*-------------------------------------------------------------------------*/

void
CGridBenchmark::MeasureSetValue( bool byIndex, CSample& sample )
{
    const UInt32 count = (UInt32) m_keys.size();
    const UInt32 operations = BenchmarkOperationCount;
    UInt64 start = DRN::CDRNClock::NowInMicroseconds();
    UInt64 cpuStart = ThreadCpuTimeNs();
    for ( UInt32 i=0; i<operations; ++i )
    {
        const UInt32 slot = i % count;
        const CORE::CVariant value( ++m_stamp );
        if ( byIndex )
            m_publisherGrid->SetValueByIndex( m_indices[ slot ], value );
        else
            m_publisherGrid->SetValue( m_keys[ slot ], value );
    }
    sample.cpuNs = ( ThreadCpuTimeNs() - cpuStart ) / operations;
    sample.wallNs = ( DRN::CDRNClock::NowInMicroseconds() - start ) * 1000 / operations;
}

/*-------------------------------------------------------------------------*/

void
CGridBenchmark::MeasureGetValue( DRN::CDRNDataGrid& grid, CSample& sample )
{
    const UInt32 count = (UInt32) m_indices.size();
    const UInt32 operations = BenchmarkOperationCount;
    CORE::CVariant value;
    UInt32 found = 0;
    UInt64 start = DRN::CDRNClock::NowInMicroseconds();
    UInt64 cpuStart = ThreadCpuTimeNs();
    for ( UInt32 i=0; i<operations; ++i )
    {
        if ( grid.GetValueByIndex( m_indices[ i % count ], value ) )
            ++found;
    }
    sample.cpuNs = found > 0 ? ( ThreadCpuTimeNs() - cpuStart ) / operations : 0;
    sample.wallNs = found > 0 ? ( DRN::CDRNClock::NowInMicroseconds() - start ) * 1000 / operations : 0;
}

/*-------------------------------------------------------------------------*/

bool
CGridBenchmark::SettleReplica( void )
{
    // the replica has the value of the publisher for the keys that were written last
    const UInt32 count = (UInt32) m_keys.size();
    CORE::CVariant expected;
    const UInt32 sample = ( BenchmarkOperationCount - 1 ) % count;
    if ( !m_publisherGrid->GetValue( m_keys[ sample ], expected ) )
        return false;
    CReplicaConverged converged( *m_replicaGrid, m_keys[ sample ], expected.AsUInt32() );
    return PumpUntil( converged, 60000 );
}

/*-------------------------------------------------------------------------*/

bool
CGridBenchmark::MeasureDelta( CSample& sample )
{
    const UInt32 count = (UInt32) m_keys.size();
    const UInt32 changed = BenchmarkDeltaKeyCount < count ? BenchmarkDeltaKeyCount : count;
    GUCEF::vector< UInt64 > rounds;
    GUCEF::vector< UInt64 > works;
    GUCEF::vector< UInt64 > cpuWorks;
    GUCEF::vector< UInt64 > pulses;
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    for ( UInt32 round=0; round<BenchmarkDeltaRounds; ++round )
    {
        const UInt32 stamp = ++m_stamp;
        for ( UInt32 i=0; i<changed; ++i )
            m_publisherGrid->SetValueByIndex( m_indices[ i ], CORE::CVariant( stamp ) );
        CReplicaConverged converged( *m_replicaGrid, m_keys[ changed-1 ], stamp );
        UInt64 start = DRN::CDRNClock::NowInMicroseconds();
        UInt64 inPulses = 0;
        UInt64 cpuInPulses = 0;
        UInt64 pulseCount = 0;
        while ( !converged.IsMet() || pulseCount < BenchmarkDeltaMinPulses )
        {
            if ( DRN::CDRNClock::NowInMicroseconds() - start > 60000000 )
                return false;
            const UInt64 pulseStart = DRN::CDRNClock::NowInMicroseconds();
            const UInt64 pulseCpuStart = ThreadCpuTimeNs();
            pulseGenerator->ForceDirectPulse();
            cpuInPulses += ThreadCpuTimeNs() - pulseCpuStart;
            inPulses += DRN::CDRNClock::NowInMicroseconds() - pulseStart;
            ++pulseCount;
            MT::ThreadDelay( 1 );
        }
        rounds.push_back( ( DRN::CDRNClock::NowInMicroseconds() - start ) * 1000 / changed );
        works.push_back( inPulses * 1000 / changed );
        cpuWorks.push_back( cpuInPulses / changed );
        pulses.push_back( pulseCount );
    }
    sample.deltaNs = Median( rounds );
    sample.wallNs = Median( works );
    sample.cpuNs = Median( cpuWorks );
    sample.pulses = Median( pulses );
    return 0 != sample.deltaNs;
}

/*-------------------------------------------------------------------------*/

bool
CGridBenchmark::MeasureOperation( TOperation operation, CGridBenchmarkLoad* load, CSample& sample )
{
    // the writes of the operations before are on their way to the replica: let them settle before it is read
    if ( OPERATION_REPLICA_GETVALUE == operation && !SettleReplica() )
        return false;

    bool ok = true;
    if ( GUCEF_NULL != load )
        load->Begin();
    switch ( operation )
    {
        case OPERATION_SETVALUE: { MeasureSetValue( false, sample ); break; }
        case OPERATION_SETVALUE_BYINDEX: { MeasureSetValue( true, sample ); break; }
        case OPERATION_GETVALUE: { MeasureGetValue( *m_publisherGrid, sample ); break; }
        case OPERATION_REPLICA_GETVALUE: { MeasureGetValue( *m_replicaGrid, sample ); break; }
        case OPERATION_DELTA: { ok = MeasureDelta( sample ); break; }
        default: { ok = false; break; }
    }
    if ( GUCEF_NULL != load )
        load->End();
    return ok;
}

/*-------------------------------------------------------------------------*/

void
CGridBenchmark::SetMedians( TSampleVector* samples, CGridBenchmarkResult& result )
{
    GUCEF::vector< UInt64 > wall[ OPERATION_COUNT ];
    GUCEF::vector< UInt64 > cpu[ OPERATION_COUNT ];
    GUCEF::vector< UInt64 > delta;
    GUCEF::vector< UInt64 > pulses;
    for ( UInt32 o=0; o<OPERATION_COUNT; ++o )
    {
        for ( UInt32 r=0; r<samples[ o ].size(); ++r )
        {
            wall[ o ].push_back( samples[ o ][ r ].wallNs );
            cpu[ o ].push_back( samples[ o ][ r ].cpuNs );
            if ( OPERATION_DELTA == o )
            {
                delta.push_back( samples[ o ][ r ].deltaNs );
                pulses.push_back( samples[ o ][ r ].pulses );
            }
        }
    }
    result.setValueNs = Median( wall[ OPERATION_SETVALUE ] );
    result.setValueByIndexNs = Median( wall[ OPERATION_SETVALUE_BYINDEX ] );
    result.getValueByIndexNs = Median( wall[ OPERATION_GETVALUE ] );
    result.replicaGetValueByIndexNs = Median( wall[ OPERATION_REPLICA_GETVALUE ] );
    result.deltaWorkNsPerKey = Median( wall[ OPERATION_DELTA ] );
    result.setValueCpuNs = Median( cpu[ OPERATION_SETVALUE ] );
    result.setValueByIndexCpuNs = Median( cpu[ OPERATION_SETVALUE_BYINDEX ] );
    result.getValueByIndexCpuNs = Median( cpu[ OPERATION_GETVALUE ] );
    result.replicaGetValueByIndexCpuNs = Median( cpu[ OPERATION_REPLICA_GETVALUE ] );
    result.deltaWorkCpuNsPerKey = Median( cpu[ OPERATION_DELTA ] );
    result.deltaNsPerKey = Median( delta );
    result.deltaPulses = Median( pulses );
}

/*-------------------------------------------------------------------------*/

bool
CGridBenchmark::Measure( CGridBenchmarkLoad* load, UInt32 repetitions, CGridBenchmarkResult& result )
{
    result = CGridBenchmarkResult();
    result.keyCount = (UInt32) m_keys.size();
    TSampleVector samples[ OPERATION_COUNT ];
    for ( UInt32 r=0; r<repetitions; ++r )
    {
        for ( UInt32 o=0; o<OPERATION_COUNT; ++o )
        {
            CSample sample;
            if ( !MeasureOperation( (TOperation) o, load, sample ) )
                return false;
            samples[ o ].push_back( sample );
        }
    }
    SetMedians( samples, result );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridBenchmark::MeasurePaired( CGridBenchmark& reference, CGridBenchmark& candidate, UInt32 rounds,
                               CGridBenchmarkResult& referenceResult, CGridBenchmarkResult& candidateResult,
                               UInt32& worstCpuPercent )
{
    return MeasurePairs( reference, GUCEF_NULL, candidate, GUCEF_NULL, rounds, referenceResult, candidateResult, worstCpuPercent );
}

/*-------------------------------------------------------------------------*/

bool
CGridBenchmark::MeasurePairedWithLoad( CGridBenchmarkLoad& load, UInt32 rounds, CGridBenchmarkResult& reference,
                                       CGridBenchmarkResult& loaded, UInt32& worstCpuPercent )
{
    return MeasurePairs( *this, GUCEF_NULL, *this, &load, rounds, reference, loaded, worstCpuPercent );
}

/*-------------------------------------------------------------------------*/

bool
CGridBenchmark::MeasurePairs( CGridBenchmark& reference, CGridBenchmarkLoad* referenceLoad, CGridBenchmark& candidate,
                              CGridBenchmarkLoad* candidateLoad, UInt32 rounds, CGridBenchmarkResult& referenceResult,
                              CGridBenchmarkResult& candidateResult, UInt32& worstCpuPercent )
{
    referenceResult = CGridBenchmarkResult();
    referenceResult.keyCount = (UInt32) reference.m_keys.size();
    candidateResult = CGridBenchmarkResult();
    candidateResult.keyCount = (UInt32) candidate.m_keys.size();
    worstCpuPercent = 0;
    TSampleVector referenceSamples[ OPERATION_COUNT ];
    TSampleVector candidateSamples[ OPERATION_COUNT ];
    GUCEF::vector< UInt64 > percents[ OPERATION_COUNT ];
    for ( UInt32 r=0; r<rounds; ++r )
    {
        for ( UInt32 o=0; o<OPERATION_COUNT; ++o )
        {
            CSample referenceSample;
            CSample candidateSample;
            const bool referenceFirst = 0 == ( r % 2 );
            CGridBenchmark& first = referenceFirst ? reference : candidate;
            CGridBenchmark& second = referenceFirst ? candidate : reference;
            CGridBenchmarkLoad* firstLoad = referenceFirst ? referenceLoad : candidateLoad;
            CGridBenchmarkLoad* secondLoad = referenceFirst ? candidateLoad : referenceLoad;
            CSample& firstSample = referenceFirst ? referenceSample : candidateSample;
            CSample& secondSample = referenceFirst ? candidateSample : referenceSample;
            if ( !first.MeasureOperation( (TOperation) o, firstLoad, firstSample ) || !second.MeasureOperation( (TOperation) o, secondLoad, secondSample ) )
                return false;
            referenceSamples[ o ].push_back( referenceSample );
            candidateSamples[ o ].push_back( candidateSample );
            percents[ o ].push_back( referenceSample.cpuNs > 0 ? candidateSample.cpuNs * 100 / referenceSample.cpuNs : 0 );
        }
    }
    SetMedians( referenceSamples, referenceResult );
    SetMedians( candidateSamples, candidateResult );
    for ( UInt32 o=0; o<OPERATION_COUNT; ++o )
    {
        const UInt32 percent = (UInt32) Median( percents[ o ] );
        if ( percent > worstCpuPercent )
            worstCpuPercent = percent;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridBenchmark::MeasureAgainstReference( CGridBenchmarkLoad& load, UInt32 repetitions, CGridBenchmarkResult& reference,
                                         CGridBenchmarkResult& loaded, UInt32& worstPercent, UInt32& busyPercent )
{
    reference = CGridBenchmarkResult();
    reference.keyCount = (UInt32) m_keys.size();
    loaded = reference;
    worstPercent = 0;
    busyPercent = 0;
    TSampleVector referenceSamples[ OPERATION_COUNT ];
    TSampleVector loadedSamples[ OPERATION_COUNT ];
    GUCEF::vector< UInt64 > percents[ OPERATION_COUNT ];
    GUCEF::vector< UInt64 > busy[ OPERATION_COUNT ];
    for ( UInt32 r=0; r<repetitions; ++r )
    {
        for ( UInt32 o=0; o<OPERATION_COUNT; ++o )
        {
            CSample without;
            CSample with;
            if ( !MeasureOperation( (TOperation) o, GUCEF_NULL, without ) || !MeasureOperation( (TOperation) o, &load, with ) )
                return false;
            referenceSamples[ o ].push_back( without );
            loadedSamples[ o ].push_back( with );
            percents[ o ].push_back( without.wallNs > 0 ? with.wallNs * 100 / without.wallNs : 0 );
            busy[ o ].push_back( without.cpuNs > 0 ? without.wallNs * 100 / without.cpuNs : 100 );
        }
    }
    SetMedians( referenceSamples, reference );
    SetMedians( loadedSamples, loaded );
    for ( UInt32 o=0; o<OPERATION_COUNT; ++o )
    {
        const UInt32 percent = (UInt32) Median( percents[ o ] );
        if ( percent > worstPercent )
        {
            worstPercent = percent;
            busyPercent = (UInt32) Median( busy[ o ] );
        }
    }
    if ( busyPercent < 100 )
        busyPercent = 100;
    return true;
}

}; /* namespace DRNTEST */

/*-------------------------------------------------------------------------*/
