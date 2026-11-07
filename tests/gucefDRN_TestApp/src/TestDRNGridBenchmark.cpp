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
{

}

/*-------------------------------------------------------------------------*/

CORE::CString
CGridBenchmarkResult::ToString( void ) const
{
    return CORE::UInt32ToString( keyCount ) + " keys: SetValue " + CORE::UInt64ToString( setValueNs ) + " ns, SetValueByIndex " + CORE::UInt64ToString( setValueByIndexNs ) +
           " ns, GetValueByIndex " + CORE::UInt64ToString( getValueByIndexNs ) + " ns, replica GetValueByIndex " + CORE::UInt64ToString( replicaGetValueByIndexNs ) +
           " ns, delta " + CORE::UInt64ToString( deltaNsPerKey ) + " ns per changed key (" + CORE::UInt64ToString( deltaWorkNsPerKey ) + " ns of work)";
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
CGridBenchmark::Setup( UInt32 keyCount, const CORE::CString& name, bool declareTypes )
{
    m_group = GUCEF_NEW CGroup( DRN::DRN_TOPOLOGY_CLIENT_SERVER, name, "mem" );
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

UInt64
CGridBenchmark::MeasureSetValue( bool byIndex )
{
    const UInt32 count = (UInt32) m_keys.size();
    const UInt32 operations = BenchmarkOperationCount;
    UInt64 start = DRN::CDRNClock::NowInMicroseconds();
    for ( UInt32 i=0; i<operations; ++i )
    {
        const UInt32 slot = i % count;
        const CORE::CVariant value( ++m_stamp );
        if ( byIndex )
            m_publisherGrid->SetValueByIndex( m_indices[ slot ], value );
        else
            m_publisherGrid->SetValue( m_keys[ slot ], value );
    }
    UInt64 elapsed = DRN::CDRNClock::NowInMicroseconds() - start;
    return elapsed * 1000 / operations;
}

/*-------------------------------------------------------------------------*/

UInt64
CGridBenchmark::MeasureGetValue( DRN::CDRNDataGrid& grid )
{
    const UInt32 count = (UInt32) m_indices.size();
    const UInt32 operations = BenchmarkOperationCount;
    CORE::CVariant value;
    UInt32 found = 0;
    UInt64 start = DRN::CDRNClock::NowInMicroseconds();
    for ( UInt32 i=0; i<operations; ++i )
    {
        if ( grid.GetValueByIndex( m_indices[ i % count ], value ) )
            ++found;
    }
    UInt64 elapsed = DRN::CDRNClock::NowInMicroseconds() - start;
    return found > 0 ? elapsed * 1000 / operations : 0;
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

UInt64
CGridBenchmark::MeasureDelta( UInt64& workNsPerKey )
{
    const UInt32 count = (UInt32) m_keys.size();
    const UInt32 changed = BenchmarkDeltaKeyCount < count ? BenchmarkDeltaKeyCount : count;
    GUCEF::vector< UInt64 > rounds;
    GUCEF::vector< UInt64 > works;
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    for ( UInt32 round=0; round<BenchmarkDeltaRounds; ++round )
    {
        const UInt32 stamp = ++m_stamp;
        for ( UInt32 i=0; i<changed; ++i )
            m_publisherGrid->SetValueByIndex( m_indices[ i ], CORE::CVariant( stamp ) );
        CReplicaConverged converged( *m_replicaGrid, m_keys[ changed-1 ], stamp );
        UInt64 start = DRN::CDRNClock::NowInMicroseconds();
        UInt64 inPulses = 0;
        while ( !converged.IsMet() )
        {
            if ( DRN::CDRNClock::NowInMicroseconds() - start > 60000000 )
                return 0;
            const UInt64 pulseStart = DRN::CDRNClock::NowInMicroseconds();
            pulseGenerator->ForceDirectPulse();
            inPulses += DRN::CDRNClock::NowInMicroseconds() - pulseStart;
            MT::ThreadDelay( 1 );
        }
        rounds.push_back( ( DRN::CDRNClock::NowInMicroseconds() - start ) * 1000 / changed );
        works.push_back( inPulses * 1000 / changed );
    }
    workNsPerKey = Median( works );
    return Median( rounds );
}

/*-------------------------------------------------------------------------*/

bool
CGridBenchmark::Measure( CGridBenchmarkLoad* load, UInt32 repetitions, CGridBenchmarkResult& result )
{
    result = CGridBenchmarkResult();
    result.keyCount = (UInt32) m_keys.size();
    GUCEF::vector< UInt64 > setValue, setValueByIndex, getValue, replicaGet, delta, deltaWork;
    for ( UInt32 r=0; r<repetitions; ++r )
    {
        if ( GUCEF_NULL != load ) load->Begin();
        setValue.push_back( MeasureSetValue( false ) );
        if ( GUCEF_NULL != load ) load->End();

        if ( GUCEF_NULL != load ) load->Begin();
        setValueByIndex.push_back( MeasureSetValue( true ) );
        if ( GUCEF_NULL != load ) load->End();

        if ( GUCEF_NULL != load ) load->Begin();
        getValue.push_back( MeasureGetValue( *m_publisherGrid ) );
        if ( GUCEF_NULL != load ) load->End();

        // the writes above are on their way to the replica: let them settle, the replica is then read
        if ( !SettleReplica() )
            return false;
        if ( GUCEF_NULL != load ) load->Begin();
        replicaGet.push_back( MeasureGetValue( *m_replicaGrid ) );
        if ( GUCEF_NULL != load ) load->End();

        if ( GUCEF_NULL != load ) load->Begin();
        UInt64 deltaWorkNs = 0;
        UInt64 deltaNs = MeasureDelta( deltaWorkNs );
        if ( GUCEF_NULL != load ) load->End();
        if ( 0 == deltaNs )
            return false;
        delta.push_back( deltaNs );
        deltaWork.push_back( deltaWorkNs );
    }
    result.setValueNs = Median( setValue );
    result.setValueByIndexNs = Median( setValueByIndex );
    result.getValueByIndexNs = Median( getValue );
    result.replicaGetValueByIndexNs = Median( replicaGet );
    result.deltaNsPerKey = Median( delta );
    result.deltaWorkNsPerKey = Median( deltaWork );
    return true;
}

}; /* namespace DRNTEST */

/*-------------------------------------------------------------------------*/
