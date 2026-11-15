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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSCONFIGSUPPORT_H
#include "gucefDRNUTILS_CDrnUtilsConfigSupport.h"
#define GUCEF_DRNUTILS_CDRNUTILSCONFIGSUPPORT_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSCONFIGSUPPORT_H ? */

#include "gucefDRNUTILS_CTaskManagerHostSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CTaskManagerHostSettings::ClassTypeName = "GUCEF::DRNUTILS::CTaskManagerHostSettings";

/*-------------------------------------------------------------------------*/

CTaskManagerHostSettings::CTaskManagerHostSettings( void )
    : CORE::CIConfigurable()
    , m_exposure( DRNUTILS_EXPOSURE_NONE )
    , m_controllablePools()
    , m_protectedPools()
    , m_cancellableTaskTypes()
    , m_minAllowedWorkers( 0 )
    , m_maxAllowedThreads( 64 )
    , m_roleMap()
    , m_tablePrefix()
    , m_samplerPoolName( "gucef.drnutils.sampler" )
    , m_minSampleIntervalMs( 1000 )
    , m_sampleIntervalMs( 2000 )
    , m_taskSampleIntervalMs( 5000 )
    , m_maxTaskRows( 512 )
    , m_maxThreadRows( 512 )
    , m_finishedTaskRows( 64 )
    , m_viewingGraceMs( 30000 )
    , m_idempotencyTtlMs( 86400000 )
    , m_maxCommandsPerSubject( 256 )
    , m_maxCommands( 4096 )
    , m_maxControlCommandsPerMinutePerSubject( 60 )
    , m_allowInsecureControl( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTaskManagerHostSettings::~CTaskManagerHostSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerHostSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "exposure", CDrnUtilsConfigSupport::ExposureToString( m_exposure ) );
    CDrnUtilsConfigSupport::SaveStringSet( config, "controllablePools", m_controllablePools );
    CDrnUtilsConfigSupport::SaveStringSet( config, "protectedPools", m_protectedPools );
    CDrnUtilsConfigSupport::SaveStringSet( config, "cancellableTaskTypes", m_cancellableTaskTypes );
    config.SetAttribute( "minAllowedWorkers", m_minAllowedWorkers );
    config.SetAttribute( "maxAllowedThreads", m_maxAllowedThreads );
    CDrnUtilsConfigSupport::SaveRoleMap( config, m_roleMap );
    config.SetAttribute( "tablePrefix", m_tablePrefix );
    config.SetAttribute( "samplerPoolName", m_samplerPoolName );
    config.SetAttribute( "minSampleIntervalMs", m_minSampleIntervalMs );
    config.SetAttribute( "sampleIntervalMs", m_sampleIntervalMs );
    config.SetAttribute( "taskSampleIntervalMs", m_taskSampleIntervalMs );
    config.SetAttribute( "maxTaskRows", m_maxTaskRows );
    config.SetAttribute( "maxThreadRows", m_maxThreadRows );
    config.SetAttribute( "finishedTaskRows", m_finishedTaskRows );
    config.SetAttribute( "viewingGraceMs", m_viewingGraceMs );
    config.SetAttribute( "idempotencyTtlMs", m_idempotencyTtlMs );
    config.SetAttribute( "maxCommandsPerSubject", m_maxCommandsPerSubject );
    config.SetAttribute( "maxCommands", m_maxCommands );
    config.SetAttribute( "maxControlCommandsPerMinutePerSubject", m_maxControlCommandsPerMinutePerSubject );
    config.SetAttribute( "allowInsecureControl", m_allowInsecureControl );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerHostSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_exposure = CDrnUtilsConfigSupport::ExposureFromString( config.GetAttributeValueOrChildValueByName( "exposure" ).AsString(), m_exposure );
    CDrnUtilsConfigSupport::LoadStringSet( config, "controllablePools", m_controllablePools );
    CDrnUtilsConfigSupport::LoadStringSet( config, "protectedPools", m_protectedPools );
    CDrnUtilsConfigSupport::LoadStringSet( config, "cancellableTaskTypes", m_cancellableTaskTypes );
    m_minAllowedWorkers = config.GetAttributeValueOrChildValueByName( "minAllowedWorkers" ).AsUInt32( m_minAllowedWorkers, true );
    m_maxAllowedThreads = config.GetAttributeValueOrChildValueByName( "maxAllowedThreads" ).AsInt32( m_maxAllowedThreads, true );
    CDrnUtilsConfigSupport::LoadRoleMap( config, m_roleMap );
    m_tablePrefix = config.GetAttributeValueOrChildValueByName( "tablePrefix" ).AsString( m_tablePrefix, true );
    m_samplerPoolName = config.GetAttributeValueOrChildValueByName( "samplerPoolName" ).AsString( m_samplerPoolName, true );
    SetMinSampleIntervalMs( config.GetAttributeValueOrChildValueByName( "minSampleIntervalMs" ).AsUInt32( m_minSampleIntervalMs, true ) );
    SetSampleIntervalMs( config.GetAttributeValueOrChildValueByName( "sampleIntervalMs" ).AsUInt32( m_sampleIntervalMs, true ) );
    m_taskSampleIntervalMs = config.GetAttributeValueOrChildValueByName( "taskSampleIntervalMs" ).AsUInt32( m_taskSampleIntervalMs, true );
    m_maxTaskRows = config.GetAttributeValueOrChildValueByName( "maxTaskRows" ).AsUInt32( m_maxTaskRows, true );
    m_maxThreadRows = config.GetAttributeValueOrChildValueByName( "maxThreadRows" ).AsUInt32( m_maxThreadRows, true );
    m_finishedTaskRows = config.GetAttributeValueOrChildValueByName( "finishedTaskRows" ).AsUInt32( m_finishedTaskRows, true );
    m_viewingGraceMs = config.GetAttributeValueOrChildValueByName( "viewingGraceMs" ).AsUInt32( m_viewingGraceMs, true );
    m_idempotencyTtlMs = config.GetAttributeValueOrChildValueByName( "idempotencyTtlMs" ).AsUInt64( m_idempotencyTtlMs, true );
    m_maxCommandsPerSubject = config.GetAttributeValueOrChildValueByName( "maxCommandsPerSubject" ).AsUInt32( m_maxCommandsPerSubject, true );
    m_maxCommands = config.GetAttributeValueOrChildValueByName( "maxCommands" ).AsUInt32( m_maxCommands, true );
    m_maxControlCommandsPerMinutePerSubject = config.GetAttributeValueOrChildValueByName( "maxControlCommandsPerMinutePerSubject" ).AsUInt32( m_maxControlCommandsPerMinutePerSubject, true );
    m_allowInsecureControl = config.GetAttributeValueOrChildValueByName( "allowInsecureControl" ).AsBool( m_allowInsecureControl, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTaskManagerHostSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetExposure( TDrnUtilsExposure exposure )
{GUCEF_TRACE;

    m_exposure = exposure;
}

/*-------------------------------------------------------------------------*/

TDrnUtilsExposure
CTaskManagerHostSettings::GetExposure( void ) const
{GUCEF_TRACE;

    return m_exposure;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetControllablePools( const CORE::CString::StringSet& controllablePools )
{GUCEF_TRACE;

    m_controllablePools = controllablePools;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CTaskManagerHostSettings::GetControllablePools( void ) const
{GUCEF_TRACE;

    return m_controllablePools;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetProtectedPools( const CORE::CString::StringSet& protectedPools )
{GUCEF_TRACE;

    m_protectedPools = protectedPools;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CTaskManagerHostSettings::GetProtectedPools( void ) const
{GUCEF_TRACE;

    return m_protectedPools;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetCancellableTaskTypes( const CORE::CString::StringSet& cancellableTaskTypes )
{GUCEF_TRACE;

    m_cancellableTaskTypes = cancellableTaskTypes;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CTaskManagerHostSettings::GetCancellableTaskTypes( void ) const
{GUCEF_TRACE;

    return m_cancellableTaskTypes;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetMinAllowedWorkers( UInt32 minAllowedWorkers )
{GUCEF_TRACE;

    m_minAllowedWorkers = minAllowedWorkers;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetMinAllowedWorkers( void ) const
{GUCEF_TRACE;

    return m_minAllowedWorkers;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetMaxAllowedThreads( Int32 maxAllowedThreads )
{GUCEF_TRACE;

    m_maxAllowedThreads = maxAllowedThreads;
}

/*-------------------------------------------------------------------------*/

Int32
CTaskManagerHostSettings::GetMaxAllowedThreads( void ) const
{GUCEF_TRACE;

    return m_maxAllowedThreads;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetRoleMap( const CDrnUtilsRoleMap& roleMap )
{GUCEF_TRACE;

    m_roleMap = roleMap;
}

/*-------------------------------------------------------------------------*/

const CDrnUtilsRoleMap&
CTaskManagerHostSettings::GetRoleMap( void ) const
{GUCEF_TRACE;

    return m_roleMap;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetTablePrefix( const CORE::CString& tablePrefix )
{GUCEF_TRACE;

    m_tablePrefix = tablePrefix;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTaskManagerHostSettings::GetTablePrefix( void ) const
{GUCEF_TRACE;

    return m_tablePrefix;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetSamplerPoolName( const CORE::CString& samplerPoolName )
{GUCEF_TRACE;

    m_samplerPoolName = samplerPoolName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTaskManagerHostSettings::GetSamplerPoolName( void ) const
{GUCEF_TRACE;

    return m_samplerPoolName;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetMinSampleIntervalMs( UInt32 minSampleIntervalMs )
{GUCEF_TRACE;

    m_minSampleIntervalMs = minSampleIntervalMs < 1 ? 1 : minSampleIntervalMs;
    if ( m_sampleIntervalMs < m_minSampleIntervalMs )
        m_sampleIntervalMs = m_minSampleIntervalMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetMinSampleIntervalMs( void ) const
{GUCEF_TRACE;

    return m_minSampleIntervalMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetSampleIntervalMs( UInt32 sampleIntervalMs )
{GUCEF_TRACE;

    m_sampleIntervalMs = sampleIntervalMs < m_minSampleIntervalMs ? m_minSampleIntervalMs : sampleIntervalMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetSampleIntervalMs( void ) const
{GUCEF_TRACE;

    return m_sampleIntervalMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetTaskSampleIntervalMs( UInt32 taskSampleIntervalMs )
{GUCEF_TRACE;

    m_taskSampleIntervalMs = taskSampleIntervalMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetTaskSampleIntervalMs( void ) const
{GUCEF_TRACE;

    return m_taskSampleIntervalMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetMaxTaskRows( UInt32 maxTaskRows )
{GUCEF_TRACE;

    m_maxTaskRows = maxTaskRows;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetMaxTaskRows( void ) const
{GUCEF_TRACE;

    return m_maxTaskRows;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetMaxThreadRows( UInt32 maxThreadRows )
{GUCEF_TRACE;

    m_maxThreadRows = maxThreadRows;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetMaxThreadRows( void ) const
{GUCEF_TRACE;

    return m_maxThreadRows;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetFinishedTaskRows( UInt32 finishedTaskRows )
{GUCEF_TRACE;

    m_finishedTaskRows = finishedTaskRows;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetFinishedTaskRows( void ) const
{GUCEF_TRACE;

    return m_finishedTaskRows;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetViewingGraceMs( UInt32 viewingGraceMs )
{GUCEF_TRACE;

    m_viewingGraceMs = viewingGraceMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetViewingGraceMs( void ) const
{GUCEF_TRACE;

    return m_viewingGraceMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetIdempotencyTtlMs( UInt64 idempotencyTtlMs )
{GUCEF_TRACE;

    m_idempotencyTtlMs = idempotencyTtlMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CTaskManagerHostSettings::GetIdempotencyTtlMs( void ) const
{GUCEF_TRACE;

    return m_idempotencyTtlMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetMaxCommandsPerSubject( UInt32 maxCommandsPerSubject )
{GUCEF_TRACE;

    m_maxCommandsPerSubject = maxCommandsPerSubject;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetMaxCommandsPerSubject( void ) const
{GUCEF_TRACE;

    return m_maxCommandsPerSubject;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetMaxCommands( UInt32 maxCommands )
{GUCEF_TRACE;

    m_maxCommands = maxCommands;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetMaxCommands( void ) const
{GUCEF_TRACE;

    return m_maxCommands;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetMaxControlCommandsPerMinutePerSubject( UInt32 maxControlCommandsPerMinutePerSubject )
{GUCEF_TRACE;

    m_maxControlCommandsPerMinutePerSubject = maxControlCommandsPerMinutePerSubject;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerHostSettings::GetMaxControlCommandsPerMinutePerSubject( void ) const
{GUCEF_TRACE;

    return m_maxControlCommandsPerMinutePerSubject;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHostSettings::SetAllowInsecureControl( bool allowInsecureControl )
{GUCEF_TRACE;

    m_allowInsecureControl = allowInsecureControl;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerHostSettings::GetAllowInsecureControl( void ) const
{GUCEF_TRACE;

    return m_allowInsecureControl;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
