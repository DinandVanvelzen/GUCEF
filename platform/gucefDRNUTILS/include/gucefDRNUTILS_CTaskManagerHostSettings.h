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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERHOSTSETTINGS_H
#define GUCEF_DRNUTILS_CTASKMANAGERHOSTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

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
 *  Settings of a CTaskManagerHost: what is exposed, which pools and task types may be acted on, bounds, intervals,
 *  roles and the grid prefix. Nothing is exposed or controllable by default. Task data is never exposed.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CTaskManagerHostSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    CTaskManagerHostSettings( void );

    virtual ~CTaskManagerHostSettings() GUCEF_VIRTUAL_OVERRIDE;

    /** What the host exposes; nothing by default */
    void SetExposure( TDrnUtilsExposure exposure );
    TDrnUtilsExposure GetExposure( void ) const;

    /** Pools that may be resized and whose tasks may be cancelled or stopped; none by default */
    void SetControllablePools( const CORE::CString::StringSet& controllablePools );
    const CORE::CString::StringSet& GetControllablePools( void ) const;

    /** Never controllable, in addition to the built in protected pools (RPC protocol, DRN node, scheduler dispatcher, sampler) */
    void SetProtectedPools( const CORE::CString::StringSet& protectedPools );
    const CORE::CString::StringSet& GetProtectedPools( void ) const;

    /** Task types that may be cancelled or stopped remotely; none by default */
    void SetCancellableTaskTypes( const CORE::CString::StringSet& cancellableTaskTypes );
    const CORE::CString::StringSet& GetCancellableTaskTypes( void ) const;

    /** Lower bound of a remote resize */
    void SetMinAllowedWorkers( UInt32 minAllowedWorkers );
    UInt32 GetMinAllowedWorkers( void ) const;

    /** Upper bound of a remote resize */
    void SetMaxAllowedThreads( Int32 maxAllowedThreads );
    Int32 GetMaxAllowedThreads( void ) const;

    void SetRoleMap( const CDrnUtilsRoleMap& roleMap );
    const CDrnUtilsRoleMap& GetRoleMap( void ) const;

    /** Grid name prefix; empty = "gucef.drnutils/<hostName>" */
    void SetTablePrefix( const CORE::CString& tablePrefix );
    const CORE::CString& GetTablePrefix( void ) const;

    void SetSamplerPoolName( const CORE::CString& samplerPoolName );
    const CORE::CString& GetSamplerPoolName( void ) const;

    /**
     *  The floor of every sample interval (this setting and the intervals Watch requests ask for), default 1000 ms
     *  (design 5.3). Lower only for tests and special deployments; at least 1
     */
    void SetMinSampleIntervalMs( UInt32 minSampleIntervalMs );
    UInt32 GetMinSampleIntervalMs( void ) const;

    /** At least MinSampleIntervalMs */
    void SetSampleIntervalMs( UInt32 sampleIntervalMs );
    UInt32 GetSampleIntervalMs( void ) const;

    void SetTaskSampleIntervalMs( UInt32 taskSampleIntervalMs );
    UInt32 GetTaskSampleIntervalMs( void ) const;

    void SetMaxTaskRows( UInt32 maxTaskRows );
    UInt32 GetMaxTaskRows( void ) const;

    void SetMaxThreadRows( UInt32 maxThreadRows );
    UInt32 GetMaxThreadRows( void ) const;

    void SetFinishedTaskRows( UInt32 finishedTaskRows );
    UInt32 GetFinishedTaskRows( void ) const;

    /** Task tables are emptied this long after the last Watch stream closed */
    void SetViewingGraceMs( UInt32 viewingGraceMs );
    UInt32 GetViewingGraceMs( void ) const;

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
    CORE::CString::StringSet m_controllablePools;
    CORE::CString::StringSet m_protectedPools;
    CORE::CString::StringSet m_cancellableTaskTypes;
    UInt32 m_minAllowedWorkers;
    Int32 m_maxAllowedThreads;
    CDrnUtilsRoleMap m_roleMap;
    CORE::CString m_tablePrefix;
    CORE::CString m_samplerPoolName;
    UInt32 m_minSampleIntervalMs;
    UInt32 m_sampleIntervalMs;
    UInt32 m_taskSampleIntervalMs;
    UInt32 m_maxTaskRows;
    UInt32 m_maxThreadRows;
    UInt32 m_finishedTaskRows;
    UInt32 m_viewingGraceMs;
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

#endif /* GUCEF_DRNUTILS_CTASKMANAGERHOSTSETTINGS_H ? */
