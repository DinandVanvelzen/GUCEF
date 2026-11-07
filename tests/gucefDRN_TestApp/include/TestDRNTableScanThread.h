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

#ifndef GUCEF_DRN_TESTAPP_TESTDRNTABLESCANTHREAD_H
#define GUCEF_DRN_TESTAPP_TESTDRNTABLESCANTHREAD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#include "TestDRNGridBenchmark.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNTEST {

/**
 *  Scans grid tables over and over on a thread of its own, as a tool that polls would. With checkAtomic the grid is
 *  one whose keys all get the same value in one change (the value column is UInt32): a scan that ran within one
 *  update tick must show one value for every key.
 */
class CTableScanThread : public GUCEF::MT::CActiveObject, public CGridBenchmarkLoad
{
    public:

    GUCEF::MT::CTAtomic< GUCEF::UInt32 > scans;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > scansWithinOneTick;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > rowsSeen;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > failures;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > torn;

    CTableScanThread( bool checkAtomic = false )
        : GUCEF::MT::CActiveObject(), scans(), scansWithinOneTick(), rowsSeen(), failures(), torn(), m_tables(), m_checkAtomic( checkAtomic ), m_stop()
    {
        m_stop.Store( 0 );
    }

    /** Before Start() */
    void AddTable( GUCEF::DRN::CDRNDataGridTable* table ) { m_tables.push_back( table ); }

    bool Start( void )
    {
        m_stop.Store( 0 );
        return Activate( GUCEF_NULL, 0, 0 );
    }

    void Stop( void )
    {
        m_stop.Store( 1 );
        Deactivate( false, true );
    }

    // work next to a benchmark loop
    virtual void Begin( void ) GUCEF_VIRTUAL_OVERRIDE { Start(); }
    virtual void End( void ) GUCEF_VIRTUAL_OVERRIDE { Stop(); }

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( 0 == m_stop.Load() )
        {
            for ( GUCEF::UInt32 i=0; i<(GUCEF::UInt32) m_tables.size() && 0 == m_stop.Load(); ++i )
                ScanOnce( *m_tables[ i ] );
        }
        return true;
    }

    virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    void ScanOnce( GUCEF::DRN::CDRNDataGridTable& table )
    {
        using namespace GUCEF;

        CORE::CDataTableQuery query;
        query.Select( "key" ).Select( "value" );
        CORE::CDataTableScanPtr scan;
        if ( CORE::DATATABLE_STATUS_OK != table.Scan( query, scan ) )
        {
            failures.FetchAdd( 1 );
            return;
        }
        UInt32 rows = 0;
        bool same = true;
        bool haveFirst = false;
        UInt32 first = 0;
        CORE::CDataTableBatchPtr batch;
        while ( CORE::DATATABLE_STATUS_OK == scan->NextBatch( batch ) )
        {
            const UInt32 selected = batch->GetSelectedRowCount();
            const UInt32* selection = batch->GetSelection();
            for ( UInt32 i=0; i<selected; ++i )
            {
                ++rows;
                if ( !m_checkAtomic )
                    continue;
                CORE::CVariant value;
                if ( !batch->GetValue( GUCEF_NULL == selection ? i : selection[ i ], 1, value, false ) )
                    continue;
                if ( !haveFirst )
                {
                    first = value.AsUInt32();
                    haveFirst = true;
                }
                else
                if ( value.AsUInt32() != first )
                    same = false;
            }
        }
        scans.FetchAdd( 1 );
        rowsSeen.FetchAdd( rows );

        UInt32 firstTick = 0;
        UInt32 lastTick = 0;
        if ( table.GetLastScanTicks( firstTick, lastTick ) && firstTick == lastTick )
        {
            scansWithinOneTick.FetchAdd( 1 );
            if ( m_checkAtomic && !same )
                torn.FetchAdd( 1 );
        }
    }

    GUCEF::vector< GUCEF::DRN::CDRNDataGridTable* > m_tables;
    bool m_checkAtomic;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > m_stop;
};

}; /* namespace DRNTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_TESTAPP_TESTDRNTABLESCANTHREAD_H ? */
