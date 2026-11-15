/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CJOBSCHEDULERSETTINGS_H
#define GUCEF_CORE_CJOBSCHEDULERSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Settings of one job scheduler. LoadConfig() / SaveConfig() use a CDataNode object with the fields below by their
 *  camel case names ("hostName", "dispatcherPoolName", "maxSleepMs", ...).
 */
class GUCEF_CORE_PUBLIC_CPP CJobSchedulerSettings : public CIConfigurable
{
    public:

    static const CString ClassTypeName;

    CJobSchedulerSettings( void );

    virtual ~CJobSchedulerSettings() GUCEF_VIRTUAL_OVERRIDE;

    /** Required: the key under which the task manager registers the scheduler; used in grid names and contract answers, so never user data */
    void SetHostName( const CString& hostName );
    const CString& GetHostName( void ) const;

    /** Pool of the dispatcher task (one thread) */
    void SetDispatcherPoolName( const CString& dispatcherPoolName );
    const CString& GetDispatcherPoolName( void ) const;

    /** Default pool of POOL runs */
    void SetRunPoolName( const CString& runPoolName );
    const CString& GetRunPoolName( void ) const;

    /** Pool without threads of its own that holds MAIN_THREAD runs until the main pulse generator's thread carries them out */
    void SetMainThreadPoolName( const CString& mainThreadPoolName );
    const CString& GetMainThreadPoolName( void ) const;

    void SetDefaultLaneMaxConcurrent( UInt32 defaultLaneMaxConcurrent );
    UInt32 GetDefaultLaneMaxConcurrent( void ) const;

    /** MAIN_THREAD runs carried out per pulse of the main pulse generator */
    void SetMaxMainThreadRunsPerPulse( UInt32 maxMainThreadRunsPerPulse );
    UInt32 GetMaxMainThreadRunsPerPulse( void ) const;

    /** Longest dispatcher sleep without a due instant (wall clock jump detection); 0 = no maximum */
    void SetMaxSleepMs( UInt64 maxSleepMs );
    UInt64 GetMaxSleepMs( void ) const;

    /** A difference between elapsed wall and monotonic time above this re-plans every job */
    void SetClockJumpThresholdMs( UInt64 clockJumpThresholdMs );
    UInt64 GetClockJumpThresholdMs( void ) const;

    /** Nothing is dispatched for this long after Start() */
    void SetStartDelayMs( UInt64 startDelayMs );
    UInt64 GetStartDelayMs( void ) const;

    /** Runs kept by the scheduler; 0 = no maximum */
    void SetMaxRunHistory( UInt32 maxRunHistory );
    UInt32 GetMaxRunHistory( void ) const;

    /** Last runs kept per job; 0 = no maximum */
    void SetPerJobHistory( UInt32 perJobHistory );
    UInt32 GetPerJobHistory( void ) const;

    /** Runs saved with the state */
    void SetPersistRunHistory( UInt32 persistRunHistory );
    UInt32 GetPersistRunHistory( void ) const;

    /** Monotonic */
    void SetStateSaveDebounceMs( UInt64 stateSaveDebounceMs );
    UInt64 GetStateSaveDebounceMs( void ) const;

    void SetStopTimeoutMs( UInt64 stopTimeoutMs );
    UInt64 GetStopTimeoutMs( void ) const;

    void SetProgressEventMinIntervalMs( UInt64 progressEventMinIntervalMs );
    UInt64 GetProgressEventMinIntervalMs( void ) const;

    /** Jobs submitted as data at once; 0 = no maximum */
    void SetMaxSubmittedJobs( UInt32 maxSubmittedJobs );
    UInt32 GetMaxSubmittedJobs( void ) const;

    /** Bound of a submitted payload in its serialized form; 0 = no maximum */
    void SetMaxSubmitPayloadBytes( UInt32 maxSubmitPayloadBytes );
    UInt32 GetMaxSubmitPayloadBytes( void ) const;
    virtual bool SaveConfig( CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CString m_hostName;
    CString m_dispatcherPoolName;
    CString m_runPoolName;
    CString m_mainThreadPoolName;
    UInt32 m_defaultLaneMaxConcurrent;
    UInt32 m_maxMainThreadRunsPerPulse;
    UInt64 m_maxSleepMs;
    UInt64 m_clockJumpThresholdMs;
    UInt64 m_startDelayMs;
    UInt32 m_maxRunHistory;
    UInt32 m_perJobHistory;
    UInt32 m_persistRunHistory;
    UInt64 m_stateSaveDebounceMs;
    UInt64 m_stopTimeoutMs;
    UInt64 m_progressEventMinIntervalMs;
    UInt32 m_maxSubmittedJobs;
    UInt32 m_maxSubmitPayloadBytes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBSCHEDULERSETTINGS_H ? */
