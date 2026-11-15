/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSAMPLER_H
#define GUCEF_DRNUTILS_CTASKMANAGERSAMPLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSAMPLERTASK_H
#include "gucefDRNUTILS_CTaskManagerSamplerTask.h"
#define GUCEF_DRNUTILS_CTASKMANAGERSAMPLERTASK_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERSAMPLERTASK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Module-private: starts and stops the sampler task on its own pool (design 5.3). The host runs it only while it is in
 *  viewing mode. Start(), Update() and Stop() are called on the host's notification thread; Stop() waits for the
 *  sampler task, which never waits for that thread.
 */
class CTaskManagerSampler
{
    public:

    CTaskManagerSampler( const CITaskManagerSource& source, const CORE::CString& poolName );
    ~CTaskManagerSampler();

    /** Starts the task; the observer gets SampleReadyEvent of the task (from the sampler thread) */
    bool Start( const CTaskManagerSamplerTask::SConfig& config, CORE::CTSGNotifier& observer, CORE::CIEventHandlerFunctorBase& callback );
    void Update( const CTaskManagerSamplerTask::SConfig& config );
    void Stop( CORE::CTSGNotifier& observer );
    bool IsRunning( void ) const;

    bool GetLatest( CTaskManagerSample& sample ) const;

    /** Samples taken by every task this sampler started */
    UInt64 GetSampleCount( void ) const;

    private:

    CTaskManagerSampler( const CTaskManagerSampler& src );
    CTaskManagerSampler& operator=( const CTaskManagerSampler& src );

    const CITaskManagerSource* m_source;
    CORE::CString m_poolName;
    CTaskManagerSamplerTask* m_task;
    CORE::CTaskConsumerPtr m_taskPtr;
    UInt64 m_samplesOfStoppedTasks;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CTASKMANAGERSAMPLER_H ? */
