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

#ifndef GUCEF_DRNUTILS_CSCHEDULERHOSTSETTINGS_H
#define GUCEF_DRNUTILS_CSCHEDULERHOSTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#include "gucefCORE_JobSchedulerTypes.h"
#define GUCEF_CORE_JOBSCHEDULERTYPES_H
#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */

#ifndef GUCEF_DRNUTILS_DRNUTILSTYPES_H
#include "gucefDRNUTILS_DrnUtilsTypes.h"
#define GUCEF_DRNUTILS_DRNUTILSTYPES_H
#endif /* GUCEF_DRNUTILS_DRNUTILSTYPES_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSROLEMAP_H
#include "gucefDRNUTILS_CDrnUtilsRoleMap.h"
#define GUCEF_DRNUTILS_CDRNUTILSROLEMAP_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSROLEMAP_H ? */

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
 *  Settings of a CSchedulerHost: what is exposed, which actions and submissions are allowed, roles, limits and the grid
 *  prefix. Nothing is exposed or allowed by default.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CSchedulerHostSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    CSchedulerHostSettings( void );

    virtual ~CSchedulerHostSettings() GUCEF_VIRTUAL_OVERRIDE;

    /** What the host exposes; nothing by default */
    void SetExposure( TDrnUtilsExposure exposure );
    TDrnUtilsExposure GetExposure( void ) const;

    /** Host wide control actions allowed remotely (CORE::TJobControlAction values); none by default */
    void SetAllowedActions( const CORE::TJobControlActionSet& allowedActions );
    const CORE::TJobControlActionSet& GetAllowedActions( void ) const;

    /** Custom action names allowed remotely; none by default */
    void SetAllowedCustomActions( const CORE::CString::StringSet& allowedCustomActions );
    const CORE::CString::StringSet& GetAllowedCustomActions( void ) const;

    /** Task types remote principals may submit jobs for (in addition to the scheduler's own allowlist, both must allow); none by default */
    void SetSubmittableTaskTypes( const CORE::CString::StringSet& submittableTaskTypes );
    const CORE::CString::StringSet& GetSubmittableTaskTypes( void ) const;

    /** Bound of a submitted payload in its serialized form */
    void SetMaxSubmitPayloadBytes( UInt32 maxSubmitPayloadBytes );
    UInt32 GetMaxSubmitPayloadBytes( void ) const;

    void SetRoleMap( const CDrnUtilsRoleMap& roleMap );
    const CDrnUtilsRoleMap& GetRoleMap( void ) const;

    /** Grid name prefix; empty = "gucef.drnutils/<hostName>" */
    void SetTablePrefix( const CORE::CString& tablePrefix );
    const CORE::CString& GetTablePrefix( void ) const;

    void SetPublishIntervalMs( UInt32 publishIntervalMs );
    UInt32 GetPublishIntervalMs( void ) const;

    /** Ended runs kept in the runs grid besides the active ones */
    void SetPublishedRuns( UInt32 publishedRuns );
    UInt32 GetPublishedRuns( void ) const;

    /** Change records kept for Watch resumption */
    void SetJournalSize( UInt32 journalSize );
    UInt32 GetJournalSize( void ) const;

    /** Above this a watch stream gets RESET and a snapshot */
    void SetMaxPendingPerStream( UInt32 maxPendingPerStream );
    UInt32 GetMaxPendingPerStream( void ) const;

    void SetMaxWatchStreamsPerSession( UInt32 maxWatchStreamsPerSession );
    UInt32 GetMaxWatchStreamsPerSession( void ) const;

    void SetMaxWatchStreamsPerHost( UInt32 maxWatchStreamsPerHost );
    UInt32 GetMaxWatchStreamsPerHost( void ) const;

    void SetIdempotencyTtlMs( UInt64 idempotencyTtlMs );
    UInt64 GetIdempotencyTtlMs( void ) const;

    void SetMaxCommandsPerSubject( UInt32 maxCommandsPerSubject );
    UInt32 GetMaxCommandsPerSubject( void ) const;

    void SetMaxCommands( UInt32 maxCommands );
    UInt32 GetMaxCommands( void ) const;

    void SetMaxControlCommandsPerMinutePerSubject( UInt32 maxControlCommandsPerMinutePerSubject );
    UInt32 GetMaxControlCommandsPerMinutePerSubject( void ) const;

    /** Control without a secure transport (tests only) */
    void SetAllowInsecureControl( bool allowInsecureControl );
    bool GetAllowInsecureControl( void ) const;
    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    TDrnUtilsExposure m_exposure;
    CORE::TJobControlActionSet m_allowedActions;
    CORE::CString::StringSet m_allowedCustomActions;
    CORE::CString::StringSet m_submittableTaskTypes;
    UInt32 m_maxSubmitPayloadBytes;
    CDrnUtilsRoleMap m_roleMap;
    CORE::CString m_tablePrefix;
    UInt32 m_publishIntervalMs;
    UInt32 m_publishedRuns;
    UInt32 m_journalSize;
    UInt32 m_maxPendingPerStream;
    UInt32 m_maxWatchStreamsPerSession;
    UInt32 m_maxWatchStreamsPerHost;
    UInt64 m_idempotencyTtlMs;
    UInt32 m_maxCommandsPerSubject;
    UInt32 m_maxCommands;
    UInt32 m_maxControlCommandsPerMinutePerSubject;
    bool m_allowInsecureControl;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERHOSTSETTINGS_H ? */
