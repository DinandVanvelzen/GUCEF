/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H
#define GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_DRN_CIDRNRESOURCEIOJOB_H
#include "gucefDRN_CIDRNResourceIoJob.h"
#define GUCEF_DRN_CIDRNRESOURCEIOJOB_H
#endif /* GUCEF_DRN_CIDRNRESOURCEIOJOB_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Runs a job of the resource transfers (CIDRNResourceIoJob) on a thread of the pool, registered by CDRNGlobal */
class GUCEF_DRN_EXPORT_CPP CDRNResourceIoTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    /** Queues the job on the pool of that name (empty: the default pool); a future without a task when it could not be queued */
    static CORE::CFutureResult Queue( const CORE::CString& threadPoolName, const CIDRNResourceIoJobPtr& job );

    /**
     *  As above, and the node behind the pulse generator (CDRNNode::GetWorkPulseGenerator()) is woken when the job has
     *  ended and its future is ready, so that it takes the results at once. Nothing is woken for a null pulse generator
     */
    static CORE::CFutureResult Queue( const CORE::CString& threadPoolName, const CIDRNResourceIoJobPtr& job, const CORE::PulseGeneratorPtr& wakeOnCompletion );

    CDRNResourceIoTaskConsumer( void );
    virtual ~CDRNResourceIoTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNResourceIoTaskConsumer( const CDRNResourceIoTaskConsumer& src );               /**< not implemented */
    CDRNResourceIoTaskConsumer& operator=( const CDRNResourceIoTaskConsumer& src );    /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H ? */
