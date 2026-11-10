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

#ifndef GUCEF_DRN_TESTAPP_TESTDRNGRIDBENCHMARK_H
#define GUCEF_DRN_TESTAPP_TESTDRNGRIDBENCHMARK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#include "TestDRNGroup.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNTEST {

/**
 *  What a grid benchmark measured: nanoseconds per operation, the median of the repetitions. Wall time is what the
 *  operations took, CPU time (of the measuring thread) what they cost: a machine busy with other work stretches the
 *  first and leaves the second as it is.
 */
class CGridBenchmarkResult
{
    public:

    GUCEF::UInt32 keyCount;
    GUCEF::UInt64 setValueNs;               /**< SetValue( key, value ) on the publisher: key lookup, write */
    GUCEF::UInt64 setValueByIndexNs;        /**< SetValueByIndex on the publisher */
    GUCEF::UInt64 getValueByIndexNs;        /**< GetValueByIndex on the publisher */
    GUCEF::UInt64 replicaGetValueByIndexNs; /**< GetValueByIndex on the replica */
    GUCEF::UInt64 deltaNsPerKey;            /**< a changed key from the write on the publisher to the value on the replica, wall time: delta encoding, transport, decoding, hand over, paced by the ticks of the nodes */
    GUCEF::UInt64 deltaWorkNsPerKey;        /**< the same, the time spent inside the pulses of the nodes only: what the encoding, transport and decoding cost */
    GUCEF::UInt64 setValueCpuNs;            /**< CPU time of SetValue */
    GUCEF::UInt64 setValueByIndexCpuNs;     /**< CPU time of SetValueByIndex */
    GUCEF::UInt64 getValueByIndexCpuNs;     /**< CPU time of GetValueByIndex on the publisher */
    GUCEF::UInt64 replicaGetValueByIndexCpuNs; /**< CPU time of GetValueByIndex on the replica */
    GUCEF::UInt64 deltaWorkCpuNsPerKey;     /**< CPU time inside the pulses per changed key */
    GUCEF::UInt64 deltaPulses;              /**< pulses of a round: until the replica had the changed keys, at least BenchmarkDeltaMinPulses */

    CGridBenchmarkResult( void );

    /** One line with every number */
    GUCEF::CORE::CString ToString( void ) const;
};

/*-------------------------------------------------------------------------*/

/** CPU time of the calling thread, nanoseconds (the process's where the platform has no thread clock) */
GUCEF::UInt64 ThreadCpuTimeNs( void );

/*-------------------------------------------------------------------------*/

/** Work that runs next to the measured operations, for example a table scanning in the background */
class CGridBenchmarkLoad
{
    public:

    virtual ~CGridBenchmarkLoad() {}

    /** Called before a measured loop; the load runs until End() */
    virtual void Begin( void ) = 0;

    /** Called after the measured loop; returns when the load has stopped */
    virtual void End( void ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  A publisher and a subscriber member with one grid of many keys, over the in-memory channel, and the measurements
 *  of the hot path operations of the grid on it: SetValue, SetValueByIndex, GetValueByIndex and the delta encoding
 *  (a changed key until its value is on the replica).
 */
class CGridBenchmark
{
    public:

    CGridBenchmark( void );
    ~CGridBenchmark();

    /**
     *  Makes the members and the grid with keyCount keys (UInt32 keys with UInt32 values); false when the replica did
     *  not get all of them. The grid declares the types of its keys and values when asked to, for tables over it.
     */
    bool Setup( GUCEF::UInt32 keyCount, const GUCEF::CORE::CString& name, bool declareTypes = false, bool allowPublicItems = false );

    GUCEF::DRN::CDRNSession& GetPublisherSession( void );
    GUCEF::DRN::CDRNSession& GetReplicaSession( void );
    GUCEF::DRN::CDRNDataGrid* GetPublisherGrid( void ) const;
    GUCEF::DRN::CDRNDataGrid* GetReplicaGrid( void ) const;

    /**
     *  Runs every measurement the given number of times and keeps the median. The load, when given, runs next to
     *  every measured loop.
     */
    bool Measure( CGridBenchmarkLoad* load, GUCEF::UInt32 repetitions, CGridBenchmarkResult& result );

    /**
     *  Measures every operation without and with the load right after each other, so that both runs of a pair see
     *  the same machine, the given number of times. Of the operations whose wall time the load may stretch by
     *  holding the node lock (the four single calls and the work of the delta inside the pulses):
     *  @param worstPercent the worst median of the pairs, wall time with the load in percent of without
     *  @param busyPercent the median of the runs without the load, wall time in percent of CPU time, of the operation
     *         of worstPercent: 100 when the machine gave the measuring thread a whole core, more when it was busy
     */
    bool MeasureAgainstReference( CGridBenchmarkLoad& load, GUCEF::UInt32 repetitions, CGridBenchmarkResult& reference,
                                  CGridBenchmarkResult& loaded, GUCEF::UInt32& worstPercent, GUCEF::UInt32& busyPercent );

    /**
     *  Measures every operation of two benchmarks in pairs, the given number of rounds: per round and operation the
     *  candidate right after the reference, or the other way round in every other round, so that both runs of a pair
     *  see the same machine. Work of other processes stretches or slows both runs of a pair alike, a burst of it lands
     *  in one pair, and the median of the pairs leaves such pairs out.
     *  @param worstCpuPercent per operation the median of the pairs (CPU time of the candidate in percent of the
     *         reference), the worst of those over the operations
     */
    static bool MeasurePaired( CGridBenchmark& reference, CGridBenchmark& candidate, GUCEF::UInt32 rounds,
                               CGridBenchmarkResult& referenceResult, CGridBenchmarkResult& candidateResult,
                               GUCEF::UInt32& worstCpuPercent );

    /**
     *  As MeasurePaired(), on this benchmark without and with the load (the load set up in Begin(), let go of in End(),
     *  for a state such as attached tables): the worst median of the pairs in CPU time, in percent of the runs without
     */
    bool MeasurePairedWithLoad( CGridBenchmarkLoad& load, GUCEF::UInt32 rounds, CGridBenchmarkResult& reference,
                                CGridBenchmarkResult& loaded, GUCEF::UInt32& worstCpuPercent );

    private:

    enum TOperation
    {
        OPERATION_SETVALUE              ,
        OPERATION_SETVALUE_BYINDEX      ,
        OPERATION_GETVALUE              ,
        OPERATION_REPLICA_GETVALUE      ,
        OPERATION_DELTA                 ,
        OPERATION_COUNT
    };

    /** wall and CPU time of one operation, per operation or per key; for the delta also its wall time per key and pulses */
    class CSample
    {
        public:
        GUCEF::UInt64 wallNs;
        GUCEF::UInt64 cpuNs;
        GUCEF::UInt64 deltaNs;
        GUCEF::UInt64 pulses;
        CSample( void ) : wallNs( 0 ), cpuNs( 0 ), deltaNs( 0 ), pulses( 0 ) {}
    };
    typedef GUCEF::vector< CSample > TSampleVector;

    bool MeasureOperation( TOperation operation, CGridBenchmarkLoad* load, CSample& sample );

    static bool MeasurePairs( CGridBenchmark& reference, CGridBenchmarkLoad* referenceLoad, CGridBenchmark& candidate,
                              CGridBenchmarkLoad* candidateLoad, GUCEF::UInt32 rounds, CGridBenchmarkResult& referenceResult,
                              CGridBenchmarkResult& candidateResult, GUCEF::UInt32& worstCpuPercent );
    static void SetMedians( TSampleVector* samples, CGridBenchmarkResult& result );

    CGridBenchmark( const CGridBenchmark& src );
    CGridBenchmark& operator=( const CGridBenchmark& src );

    void MeasureSetValue( bool byIndex, CSample& sample );
    void MeasureGetValue( GUCEF::DRN::CDRNDataGrid& grid, CSample& sample );
    bool MeasureDelta( CSample& sample );
    bool SettleReplica( void );

    CGroup* m_group;
    GUCEF::DRN::CDRNDataGrid* m_publisherGrid;
    GUCEF::DRN::CDRNDataGrid* m_replicaGrid;
    GUCEF::vector< GUCEF::UInt32 > m_indices;
    GUCEF::vector< GUCEF::CORE::CVariant > m_keys;
    GUCEF::UInt32 m_stamp;
};

}; /* namespace DRNTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_TESTAPP_TESTDRNGRIDBENCHMARK_H ? */
