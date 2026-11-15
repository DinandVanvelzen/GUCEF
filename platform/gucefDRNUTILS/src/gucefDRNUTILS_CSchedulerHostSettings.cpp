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

#include "gucefDRNUTILS_CSchedulerHostSettings.h"

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

const CORE::CString CSchedulerHostSettings::ClassTypeName = "GUCEF::DRNUTILS::CSchedulerHostSettings";

/*-------------------------------------------------------------------------*/

CSchedulerHostSettings::CSchedulerHostSettings( void )
    : CORE::CIConfigurable()
    , m_exposure( DRNUTILS_EXPOSURE_NONE )
    , m_allowedActions()
    , m_allowedCustomActions()
    , m_submittableTaskTypes()
    , m_maxSubmitPayloadBytes( 65536 )
    , m_roleMap()
    , m_tablePrefix()
    , m_publishIntervalMs( 1000 )
    , m_publishedRuns( 200 )
    , m_journalSize( 4096 )
    , m_maxPendingPerStream( 4096 )
    , m_maxWatchStreamsPerSession( 4 )
    , m_maxWatchStreamsPerHost( 64 )
    , m_idempotencyTtlMs( 86400000 )
    , m_maxCommandsPerSubject( 256 )
    , m_maxCommands( 4096 )
    , m_maxControlCommandsPerMinutePerSubject( 60 )
    , m_allowInsecureControl( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSchedulerHostSettings::~CSchedulerHostSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CSchedulerHostSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "exposure", CDrnUtilsConfigSupport::ExposureToString( m_exposure ) );
    CORE::CString::StringSet actions;
    CORE::TJobControlActionSet::const_iterator a = m_allowedActions.begin();
    for ( ; a != m_allowedActions.end(); ++a )
        actions.insert( CORE::JobControlActionToString( (CORE::TJobControlAction) (*a) ) );
    CDrnUtilsConfigSupport::SaveStringSet( config, "allowedActions", actions );
    CDrnUtilsConfigSupport::SaveStringSet( config, "allowedCustomActions", m_allowedCustomActions );
    CDrnUtilsConfigSupport::SaveStringSet( config, "submittableTaskTypes", m_submittableTaskTypes );
    config.SetAttribute( "maxSubmitPayloadBytes", m_maxSubmitPayloadBytes );
    CDrnUtilsConfigSupport::SaveRoleMap( config, m_roleMap );
    config.SetAttribute( "tablePrefix", m_tablePrefix );
    config.SetAttribute( "publishIntervalMs", m_publishIntervalMs );
    config.SetAttribute( "publishedRuns", m_publishedRuns );
    config.SetAttribute( "journalSize", m_journalSize );
    config.SetAttribute( "maxPendingPerStream", m_maxPendingPerStream );
    config.SetAttribute( "maxWatchStreamsPerSession", m_maxWatchStreamsPerSession );
    config.SetAttribute( "maxWatchStreamsPerHost", m_maxWatchStreamsPerHost );
    config.SetAttribute( "idempotencyTtlMs", m_idempotencyTtlMs );
    config.SetAttribute( "maxCommandsPerSubject", m_maxCommandsPerSubject );
    config.SetAttribute( "maxCommands", m_maxCommands );
    config.SetAttribute( "maxControlCommandsPerMinutePerSubject", m_maxControlCommandsPerMinutePerSubject );
    config.SetAttribute( "allowInsecureControl", m_allowInsecureControl );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerHostSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_exposure = CDrnUtilsConfigSupport::ExposureFromString( config.GetAttributeValueOrChildValueByName( "exposure" ).AsString(), m_exposure );
    CORE::CString::StringSet actions;
    if ( CDrnUtilsConfigSupport::LoadStringSet( config, "allowedActions", actions ) )
    {
        m_allowedActions.clear();
        CORE::CString::StringSet::const_iterator a = actions.begin();
        for ( ; a != actions.end(); ++a )
        {
            CORE::TJobControlAction action = CDrnUtilsConfigSupport::JobControlActionFromString( (*a) );
            if ( CORE::JOBCONTROLACTION_UNSPECIFIED != action )
                m_allowedActions.insert( (Int32) action );
            else
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "SchedulerHostSettings: unknown action \"" << (*a) << "\" in allowedActions is ignored" );
        }
    }
    CDrnUtilsConfigSupport::LoadStringSet( config, "allowedCustomActions", m_allowedCustomActions );
    CDrnUtilsConfigSupport::LoadStringSet( config, "submittableTaskTypes", m_submittableTaskTypes );
    m_maxSubmitPayloadBytes = config.GetAttributeValueOrChildValueByName( "maxSubmitPayloadBytes" ).AsUInt32( m_maxSubmitPayloadBytes, true );
    CDrnUtilsConfigSupport::LoadRoleMap( config, m_roleMap );
    m_tablePrefix = config.GetAttributeValueOrChildValueByName( "tablePrefix" ).AsString( m_tablePrefix, true );
    m_publishIntervalMs = config.GetAttributeValueOrChildValueByName( "publishIntervalMs" ).AsUInt32( m_publishIntervalMs, true );
    m_publishedRuns = config.GetAttributeValueOrChildValueByName( "publishedRuns" ).AsUInt32( m_publishedRuns, true );
    m_journalSize = config.GetAttributeValueOrChildValueByName( "journalSize" ).AsUInt32( m_journalSize, true );
    m_maxPendingPerStream = config.GetAttributeValueOrChildValueByName( "maxPendingPerStream" ).AsUInt32( m_maxPendingPerStream, true );
    m_maxWatchStreamsPerSession = config.GetAttributeValueOrChildValueByName( "maxWatchStreamsPerSession" ).AsUInt32( m_maxWatchStreamsPerSession, true );
    m_maxWatchStreamsPerHost = config.GetAttributeValueOrChildValueByName( "maxWatchStreamsPerHost" ).AsUInt32( m_maxWatchStreamsPerHost, true );
    m_idempotencyTtlMs = config.GetAttributeValueOrChildValueByName( "idempotencyTtlMs" ).AsUInt64( m_idempotencyTtlMs, true );
    m_maxCommandsPerSubject = config.GetAttributeValueOrChildValueByName( "maxCommandsPerSubject" ).AsUInt32( m_maxCommandsPerSubject, true );
    m_maxCommands = config.GetAttributeValueOrChildValueByName( "maxCommands" ).AsUInt32( m_maxCommands, true );
    m_maxControlCommandsPerMinutePerSubject = config.GetAttributeValueOrChildValueByName( "maxControlCommandsPerMinutePerSubject" ).AsUInt32( m_maxControlCommandsPerMinutePerSubject, true );
    m_allowInsecureControl = config.GetAttributeValueOrChildValueByName( "allowInsecureControl" ).AsBool( m_allowInsecureControl, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSchedulerHostSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetExposure( TDrnUtilsExposure exposure )
{GUCEF_TRACE;

    m_exposure = exposure;
}

/*-------------------------------------------------------------------------*/

TDrnUtilsExposure
CSchedulerHostSettings::GetExposure( void ) const
{GUCEF_TRACE;

    return m_exposure;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetAllowedActions( const CORE::TJobControlActionSet& allowedActions )
{GUCEF_TRACE;

    m_allowedActions = allowedActions;
}

/*-------------------------------------------------------------------------*/

const CORE::TJobControlActionSet&
CSchedulerHostSettings::GetAllowedActions( void ) const
{GUCEF_TRACE;

    return m_allowedActions;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetAllowedCustomActions( const CORE::CString::StringSet& allowedCustomActions )
{GUCEF_TRACE;

    m_allowedCustomActions = allowedCustomActions;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CSchedulerHostSettings::GetAllowedCustomActions( void ) const
{GUCEF_TRACE;

    return m_allowedCustomActions;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetSubmittableTaskTypes( const CORE::CString::StringSet& submittableTaskTypes )
{GUCEF_TRACE;

    m_submittableTaskTypes = submittableTaskTypes;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CSchedulerHostSettings::GetSubmittableTaskTypes( void ) const
{GUCEF_TRACE;

    return m_submittableTaskTypes;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetMaxSubmitPayloadBytes( UInt32 maxSubmitPayloadBytes )
{GUCEF_TRACE;

    m_maxSubmitPayloadBytes = maxSubmitPayloadBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerHostSettings::GetMaxSubmitPayloadBytes( void ) const
{GUCEF_TRACE;

    return m_maxSubmitPayloadBytes;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetRoleMap( const CDrnUtilsRoleMap& roleMap )
{GUCEF_TRACE;

    m_roleMap = roleMap;
}

/*-------------------------------------------------------------------------*/

const CDrnUtilsRoleMap&
CSchedulerHostSettings::GetRoleMap( void ) const
{GUCEF_TRACE;

    return m_roleMap;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetTablePrefix( const CORE::CString& tablePrefix )
{GUCEF_TRACE;

    m_tablePrefix = tablePrefix;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSchedulerHostSettings::GetTablePrefix( void ) const
{GUCEF_TRACE;

    return m_tablePrefix;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetPublishIntervalMs( UInt32 publishIntervalMs )
{GUCEF_TRACE;

    m_publishIntervalMs = publishIntervalMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerHostSettings::GetPublishIntervalMs( void ) const
{GUCEF_TRACE;

    return m_publishIntervalMs;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetPublishedRuns( UInt32 publishedRuns )
{GUCEF_TRACE;

    m_publishedRuns = publishedRuns;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerHostSettings::GetPublishedRuns( void ) const
{GUCEF_TRACE;

    return m_publishedRuns;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetJournalSize( UInt32 journalSize )
{GUCEF_TRACE;

    m_journalSize = journalSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerHostSettings::GetJournalSize( void ) const
{GUCEF_TRACE;

    return m_journalSize;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetMaxPendingPerStream( UInt32 maxPendingPerStream )
{GUCEF_TRACE;

    m_maxPendingPerStream = maxPendingPerStream;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerHostSettings::GetMaxPendingPerStream( void ) const
{GUCEF_TRACE;

    return m_maxPendingPerStream;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetMaxWatchStreamsPerSession( UInt32 maxWatchStreamsPerSession )
{GUCEF_TRACE;

    m_maxWatchStreamsPerSession = maxWatchStreamsPerSession;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerHostSettings::GetMaxWatchStreamsPerSession( void ) const
{GUCEF_TRACE;

    return m_maxWatchStreamsPerSession;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetMaxWatchStreamsPerHost( UInt32 maxWatchStreamsPerHost )
{GUCEF_TRACE;

    m_maxWatchStreamsPerHost = maxWatchStreamsPerHost;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerHostSettings::GetMaxWatchStreamsPerHost( void ) const
{GUCEF_TRACE;

    return m_maxWatchStreamsPerHost;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetIdempotencyTtlMs( UInt64 idempotencyTtlMs )
{GUCEF_TRACE;

    m_idempotencyTtlMs = idempotencyTtlMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CSchedulerHostSettings::GetIdempotencyTtlMs( void ) const
{GUCEF_TRACE;

    return m_idempotencyTtlMs;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetMaxCommandsPerSubject( UInt32 maxCommandsPerSubject )
{GUCEF_TRACE;

    m_maxCommandsPerSubject = maxCommandsPerSubject;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerHostSettings::GetMaxCommandsPerSubject( void ) const
{GUCEF_TRACE;

    return m_maxCommandsPerSubject;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetMaxCommands( UInt32 maxCommands )
{GUCEF_TRACE;

    m_maxCommands = maxCommands;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerHostSettings::GetMaxCommands( void ) const
{GUCEF_TRACE;

    return m_maxCommands;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetMaxControlCommandsPerMinutePerSubject( UInt32 maxControlCommandsPerMinutePerSubject )
{GUCEF_TRACE;

    m_maxControlCommandsPerMinutePerSubject = maxControlCommandsPerMinutePerSubject;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerHostSettings::GetMaxControlCommandsPerMinutePerSubject( void ) const
{GUCEF_TRACE;

    return m_maxControlCommandsPerMinutePerSubject;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHostSettings::SetAllowInsecureControl( bool allowInsecureControl )
{GUCEF_TRACE;

    m_allowInsecureControl = allowInsecureControl;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerHostSettings::GetAllowInsecureControl( void ) const
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
