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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSAMPLERTASK_H
#define GUCEF_DRNUTILS_CTASKMANAGERSAMPLERTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H
#include "gucefDRNUTILS_CTaskManagerSample.h"
#define GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H ? */

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
 *  Module-private: the task of the sampler (design 5.3) on its own pool: takes a sample every interval (threads and
 *  tasks every task interval, only when asked), keeps the latest and raises SampleReadyEvent (mailed to the host's
 *  notification thread, never waited for). Sleeps on a semaphore between samples, so a stop request wakes it at once.
 *  Its lock is a leaf; it holds no lock while it calls the source.
 */
class CTaskManagerSamplerTask : public CORE::CTaskConsumer
{
    public:

    static const CORE::CEvent SampleReadyEvent;
    static const CORE::CString TaskType;

    static void RegisterEvents( void );

    struct SConfig
    {
        UInt32 intervalMs;
        UInt32 taskIntervalMs;
        UInt32 minIntervalMs;                   /**< the floor of intervalMs (CTaskManagerHostSettings::GetMinSampleIntervalMs()) */
        CTaskManagerSample::SOptions options;   /**< includeThreads / includeTasks: wanted at the task interval */
        SConfig( void );
    };

    explicit CTaskManagerSamplerTask( const CITaskManagerSource& source );
    virtual ~CTaskManagerSamplerTask() GUCEF_VIRTUAL_OVERRIDE;

    void SetConfig( const SConfig& config );

    /** Wakes the task (a stop request or a new configuration) */
    void Wake( void );

    /** The latest sample; false before the first one */
    bool GetLatest( CTaskManagerSample& sample ) const;

    UInt64 GetSampleCount( void ) const;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTaskManagerSamplerTask( const CTaskManagerSamplerTask& src );
    CTaskManagerSamplerTask& operator=( const CTaskManagerSamplerTask& src );

    const CITaskManagerSource* m_source;
    mutable MT::CMutex m_lock;
    MT::CSemaphore m_wake;
    SConfig m_config;
    bool m_first;
    UInt64 m_lastTaskSampleMs;
    UInt64 m_sampleCount;
    CTaskManagerSample m_previous;
    CTaskManagerSample m_previousWithTasks;
    CTaskManagerSample m_current;
    CTaskManagerSample m_latest;
    bool m_hasLatest;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CTASKMANAGERSAMPLERTASK_H ? */
