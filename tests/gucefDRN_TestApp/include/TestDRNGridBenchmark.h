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

/** What a grid benchmark measured: nanoseconds per operation, the median of the repetitions */
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

    CGridBenchmarkResult( void );

    /** One line with every number */
    GUCEF::CORE::CString ToString( void ) const;
};

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
    bool Setup( GUCEF::UInt32 keyCount, const GUCEF::CORE::CString& name, bool declareTypes = false );

    GUCEF::DRN::CDRNSession& GetPublisherSession( void );
    GUCEF::DRN::CDRNSession& GetReplicaSession( void );
    GUCEF::DRN::CDRNDataGrid* GetPublisherGrid( void ) const;
    GUCEF::DRN::CDRNDataGrid* GetReplicaGrid( void ) const;

    /**
     *  Runs every measurement the given number of times and keeps the median. The load, when given, runs next to
     *  every measured loop.
     */
    bool Measure( CGridBenchmarkLoad* load, GUCEF::UInt32 repetitions, CGridBenchmarkResult& result );

    private:

    CGridBenchmark( const CGridBenchmark& src );
    CGridBenchmark& operator=( const CGridBenchmark& src );

    GUCEF::UInt64 MeasureSetValue( bool byIndex );
    GUCEF::UInt64 MeasureGetValue( GUCEF::DRN::CDRNDataGrid& grid );
    GUCEF::UInt64 MeasureDelta( GUCEF::UInt64& workNsPerKey );
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
