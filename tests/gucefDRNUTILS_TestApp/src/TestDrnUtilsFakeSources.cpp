/*
 *  gucefDRNUTILS_TestApp: GUCEF test application for gucefDRNUTILS
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

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#include "TestDrnUtilsFakeSchedulerSource.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H ? */

namespace DRNUTILSTEST {

using namespace GUCEF;

/** Asks a host for its info on a pool thread */
class CHostProbeTask : public CORE::CTaskConsumer
{
    public:
    DRNUTILS::CSchedulerHost* host;
    MT::CTAtomic< UInt32 > done;
    CHostProbeTask( DRNUTILS::CSchedulerHost* h ) : CORE::CTaskConsumer(), host( h ), done( 0 ) {}
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "drnutils.test.probe"; }
    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        DRNUTILS::SSchedulerHostInfo info;
        COMCORE::CRpcPrincipal principal( "local", "local:probe", true );
        host->GetHostInfo( principal, info );
        done.Store( 1 );
        return true;
    }
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE {}
};

/*-------------------------------------------------------------------------*/

void
CFakeSchedulerSource::Probe( void )
{
    CHostProbeTask* probe = GUCEF_NEW CHostProbeTask( probeHost );
    CORE::CTSharedPtr< CHostProbeTask, MT::CMutex > owner( probe );
    CORE::CTaskConsumerPtr consumer;
    consumer = owner;
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "drnutils.test.probe", true );
    if ( pool.IsNULL() || pool->StartTaskWithConsumer( consumer ).HasNoFuture() )
    {
        MT::CScopeMutex scope( lock );
        ++probeFailed;
        return;
    }
    // a host lock held by the caller of Apply() would keep the probe from finishing
    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    while ( 0 == probe->done.Load() && ( MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) < 3000 )
        MT::PrecisionDelay( 1 );
    MT::CScopeMutex scope( lock );
    if ( 0 != probe->done.Load() )
        ++probeOk;
    else
        ++probeFailed;
    scope.EarlyUnlock();
    probe->WaitForTaskToFinish( 5000 );
}

}; /* namespace DRNUTILSTEST */

/*-------------------------------------------------------------------------*/
