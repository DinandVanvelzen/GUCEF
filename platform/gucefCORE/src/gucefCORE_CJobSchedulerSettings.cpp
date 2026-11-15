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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCORE_CJobSchedulerSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CJobSchedulerSettings::ClassTypeName = "GUCEF::CORE::CJobSchedulerSettings";

/*-------------------------------------------------------------------------*/

CJobSchedulerSettings::CJobSchedulerSettings( void )
    : CIConfigurable()
    , m_hostName()
    , m_dispatcherPoolName( "gucef.scheduler" )
    , m_runPoolName( "gucef.scheduler.run" )
    , m_mainThreadPoolName( "gucef.scheduler.main" )
    , m_defaultLaneMaxConcurrent( 4 )
    , m_maxMainThreadRunsPerPulse( 1 )
    , m_maxSleepMs( 900000 )
    , m_clockJumpThresholdMs( 2000 )
    , m_startDelayMs( 0 )
    , m_maxRunHistory( 1000 )
    , m_perJobHistory( 20 )
    , m_persistRunHistory( 0 )
    , m_stateSaveDebounceMs( 5000 )
    , m_stopTimeoutMs( 10000 )
    , m_progressEventMinIntervalMs( 1000 )
    , m_maxSubmittedJobs( 256 )
    , m_maxSubmitPayloadBytes( 65536 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobSchedulerSettings::~CJobSchedulerSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CJobSchedulerSettings::SaveConfig( CDataNode& config ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    config.SetNodeType( GUCEF_DATATYPE_OBJECT );
    totalSuccess = config.SetAttribute( "hostName", m_hostName ) && totalSuccess;
    totalSuccess = config.SetAttribute( "dispatcherPoolName", m_dispatcherPoolName ) && totalSuccess;
    totalSuccess = config.SetAttribute( "runPoolName", m_runPoolName ) && totalSuccess;
    totalSuccess = config.SetAttribute( "mainThreadPoolName", m_mainThreadPoolName ) && totalSuccess;
    totalSuccess = config.SetAttribute( "defaultLaneMaxConcurrent", m_defaultLaneMaxConcurrent ) && totalSuccess;
    totalSuccess = config.SetAttribute( "maxMainThreadRunsPerPulse", m_maxMainThreadRunsPerPulse ) && totalSuccess;
    totalSuccess = config.SetAttribute( "maxSleepMs", m_maxSleepMs ) && totalSuccess;
    totalSuccess = config.SetAttribute( "clockJumpThresholdMs", m_clockJumpThresholdMs ) && totalSuccess;
    totalSuccess = config.SetAttribute( "startDelayMs", m_startDelayMs ) && totalSuccess;
    totalSuccess = config.SetAttribute( "maxRunHistory", m_maxRunHistory ) && totalSuccess;
    totalSuccess = config.SetAttribute( "perJobHistory", m_perJobHistory ) && totalSuccess;
    totalSuccess = config.SetAttribute( "persistRunHistory", m_persistRunHistory ) && totalSuccess;
    totalSuccess = config.SetAttribute( "stateSaveDebounceMs", m_stateSaveDebounceMs ) && totalSuccess;
    totalSuccess = config.SetAttribute( "stopTimeoutMs", m_stopTimeoutMs ) && totalSuccess;
    totalSuccess = config.SetAttribute( "progressEventMinIntervalMs", m_progressEventMinIntervalMs ) && totalSuccess;
    totalSuccess = config.SetAttribute( "maxSubmittedJobs", m_maxSubmittedJobs ) && totalSuccess;
    totalSuccess = config.SetAttribute( "maxSubmitPayloadBytes", m_maxSubmitPayloadBytes ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CJobSchedulerSettings::LoadConfig( const CDataNode& config )
{GUCEF_TRACE;

    // missing fields keep their current value
    m_hostName = config.GetAttributeValueOrChildValueByName( "hostName" ).AsString( m_hostName, true );
    m_dispatcherPoolName = config.GetAttributeValueOrChildValueByName( "dispatcherPoolName" ).AsString( m_dispatcherPoolName, true );
    m_runPoolName = config.GetAttributeValueOrChildValueByName( "runPoolName" ).AsString( m_runPoolName, true );
    m_mainThreadPoolName = config.GetAttributeValueOrChildValueByName( "mainThreadPoolName" ).AsString( m_mainThreadPoolName, true );
    m_defaultLaneMaxConcurrent = config.GetAttributeValueOrChildValueByName( "defaultLaneMaxConcurrent" ).AsUInt32( m_defaultLaneMaxConcurrent, true );
    m_maxMainThreadRunsPerPulse = config.GetAttributeValueOrChildValueByName( "maxMainThreadRunsPerPulse" ).AsUInt32( m_maxMainThreadRunsPerPulse, true );
    m_maxSleepMs = config.GetAttributeValueOrChildValueByName( "maxSleepMs" ).AsUInt64( m_maxSleepMs, true );
    m_clockJumpThresholdMs = config.GetAttributeValueOrChildValueByName( "clockJumpThresholdMs" ).AsUInt64( m_clockJumpThresholdMs, true );
    m_startDelayMs = config.GetAttributeValueOrChildValueByName( "startDelayMs" ).AsUInt64( m_startDelayMs, true );
    m_maxRunHistory = config.GetAttributeValueOrChildValueByName( "maxRunHistory" ).AsUInt32( m_maxRunHistory, true );
    m_perJobHistory = config.GetAttributeValueOrChildValueByName( "perJobHistory" ).AsUInt32( m_perJobHistory, true );
    m_persistRunHistory = config.GetAttributeValueOrChildValueByName( "persistRunHistory" ).AsUInt32( m_persistRunHistory, true );
    m_stateSaveDebounceMs = config.GetAttributeValueOrChildValueByName( "stateSaveDebounceMs" ).AsUInt64( m_stateSaveDebounceMs, true );
    m_stopTimeoutMs = config.GetAttributeValueOrChildValueByName( "stopTimeoutMs" ).AsUInt64( m_stopTimeoutMs, true );
    m_progressEventMinIntervalMs = config.GetAttributeValueOrChildValueByName( "progressEventMinIntervalMs" ).AsUInt64( m_progressEventMinIntervalMs, true );
    m_maxSubmittedJobs = config.GetAttributeValueOrChildValueByName( "maxSubmittedJobs" ).AsUInt32( m_maxSubmittedJobs, true );
    m_maxSubmitPayloadBytes = config.GetAttributeValueOrChildValueByName( "maxSubmitPayloadBytes" ).AsUInt32( m_maxSubmitPayloadBytes, true );
    return !m_hostName.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const CString&
CJobSchedulerSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetHostName( const CString& hostName )
{GUCEF_TRACE;

    m_hostName = hostName;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobSchedulerSettings::GetHostName( void ) const
{GUCEF_TRACE;

    return m_hostName;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetDispatcherPoolName( const CString& dispatcherPoolName )
{GUCEF_TRACE;

    m_dispatcherPoolName = dispatcherPoolName;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobSchedulerSettings::GetDispatcherPoolName( void ) const
{GUCEF_TRACE;

    return m_dispatcherPoolName;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetRunPoolName( const CString& runPoolName )
{GUCEF_TRACE;

    m_runPoolName = runPoolName;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobSchedulerSettings::GetRunPoolName( void ) const
{GUCEF_TRACE;

    return m_runPoolName;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetMainThreadPoolName( const CString& mainThreadPoolName )
{GUCEF_TRACE;

    m_mainThreadPoolName = mainThreadPoolName;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobSchedulerSettings::GetMainThreadPoolName( void ) const
{GUCEF_TRACE;

    return m_mainThreadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetDefaultLaneMaxConcurrent( UInt32 defaultLaneMaxConcurrent )
{GUCEF_TRACE;

    m_defaultLaneMaxConcurrent = defaultLaneMaxConcurrent;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobSchedulerSettings::GetDefaultLaneMaxConcurrent( void ) const
{GUCEF_TRACE;

    return m_defaultLaneMaxConcurrent;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetMaxMainThreadRunsPerPulse( UInt32 maxMainThreadRunsPerPulse )
{GUCEF_TRACE;

    m_maxMainThreadRunsPerPulse = maxMainThreadRunsPerPulse;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobSchedulerSettings::GetMaxMainThreadRunsPerPulse( void ) const
{GUCEF_TRACE;

    return m_maxMainThreadRunsPerPulse;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetMaxSleepMs( UInt64 maxSleepMs )
{GUCEF_TRACE;

    m_maxSleepMs = maxSleepMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSettings::GetMaxSleepMs( void ) const
{GUCEF_TRACE;

    return m_maxSleepMs;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetClockJumpThresholdMs( UInt64 clockJumpThresholdMs )
{GUCEF_TRACE;

    m_clockJumpThresholdMs = clockJumpThresholdMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSettings::GetClockJumpThresholdMs( void ) const
{GUCEF_TRACE;

    return m_clockJumpThresholdMs;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetStartDelayMs( UInt64 startDelayMs )
{GUCEF_TRACE;

    m_startDelayMs = startDelayMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSettings::GetStartDelayMs( void ) const
{GUCEF_TRACE;

    return m_startDelayMs;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetMaxRunHistory( UInt32 maxRunHistory )
{GUCEF_TRACE;

    m_maxRunHistory = maxRunHistory;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobSchedulerSettings::GetMaxRunHistory( void ) const
{GUCEF_TRACE;

    return m_maxRunHistory;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetPerJobHistory( UInt32 perJobHistory )
{GUCEF_TRACE;

    m_perJobHistory = perJobHistory;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobSchedulerSettings::GetPerJobHistory( void ) const
{GUCEF_TRACE;

    return m_perJobHistory;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetPersistRunHistory( UInt32 persistRunHistory )
{GUCEF_TRACE;

    m_persistRunHistory = persistRunHistory;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobSchedulerSettings::GetPersistRunHistory( void ) const
{GUCEF_TRACE;

    return m_persistRunHistory;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetStateSaveDebounceMs( UInt64 stateSaveDebounceMs )
{GUCEF_TRACE;

    m_stateSaveDebounceMs = stateSaveDebounceMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSettings::GetStateSaveDebounceMs( void ) const
{GUCEF_TRACE;

    return m_stateSaveDebounceMs;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetStopTimeoutMs( UInt64 stopTimeoutMs )
{GUCEF_TRACE;

    m_stopTimeoutMs = stopTimeoutMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSettings::GetStopTimeoutMs( void ) const
{GUCEF_TRACE;

    return m_stopTimeoutMs;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetProgressEventMinIntervalMs( UInt64 progressEventMinIntervalMs )
{GUCEF_TRACE;

    m_progressEventMinIntervalMs = progressEventMinIntervalMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSettings::GetProgressEventMinIntervalMs( void ) const
{GUCEF_TRACE;

    return m_progressEventMinIntervalMs;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetMaxSubmittedJobs( UInt32 maxSubmittedJobs )
{GUCEF_TRACE;

    m_maxSubmittedJobs = maxSubmittedJobs;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobSchedulerSettings::GetMaxSubmittedJobs( void ) const
{GUCEF_TRACE;

    return m_maxSubmittedJobs;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSettings::SetMaxSubmitPayloadBytes( UInt32 maxSubmitPayloadBytes )
{GUCEF_TRACE;

    m_maxSubmitPayloadBytes = maxSubmitPayloadBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobSchedulerSettings::GetMaxSubmitPayloadBytes( void ) const
{GUCEF_TRACE;

    return m_maxSubmitPayloadBytes;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
