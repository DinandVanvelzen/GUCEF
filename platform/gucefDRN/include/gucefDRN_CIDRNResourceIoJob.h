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

#ifndef GUCEF_DRN_CIDRNRESOURCEIOJOB_H
#define GUCEF_DRN_CIDRNRESOURCEIOJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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

/**
 *  A piece of work of the resource transfers that runs on a thread of the pool (provider reads, hash trees, sink
 *  writes, digests), never on the node's thread. The node's thread queues it (CDRNResourceIoTaskConsumer::Queue())
 *  and takes its results once its CFutureResult is ready; the job holds shared references to everything it uses, so
 *  a transfer or handle that went away meanwhile leaves it harmless.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNResourceIoJob
{
    public:

    CIDRNResourceIoJob( void );

    virtual ~CIDRNResourceIoJob();

    /** Does the work, on a thread of the pool */
    virtual void Execute( void ) = 0;

    /** Asks a long job (a hash tree) to stop early */
    void RequestStop( void );

    bool IsStopRequested( void ) const;

    /**
     *  The pulse generator of the node that waits for the job (CDRNNode::GetWorkPulseGenerator()), set before the job is
     *  queued. When the job has ended, as a future sees it, the pool thread asks that pulse generator for an immediate
     *  pulse through its driver (CPulseGenerator::RequestImmediatePulseThroughDriver()): the node works at once on the
     *  result instead of at its next cycle. The pulse generator is shared, so a job that outlives its node asks a pulse
     *  generator that has no driver any more, which does nothing.
     */
    void SetCompletionPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator );

    /** Called on the pool thread once the job has ended; wakes the node that was named by SetCompletionPulseGenerator() */
    void SignalCompleted( void );

    private:

    CIDRNResourceIoJob( const CIDRNResourceIoJob& src );                /**< not implemented */
    CIDRNResourceIoJob& operator=( const CIDRNResourceIoJob& src );     /**< not implemented */

    volatile bool m_stopRequested;
    CORE::PulseGeneratorPtr m_completionPulseGenerator;     /**< set before the job is queued, read by the pool thread once it ended */
};

typedef CORE::CTSharedPtr< CIDRNResourceIoJob, MT::CMutex > CIDRNResourceIoJobPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNRESOURCEIOJOB_H ? */
