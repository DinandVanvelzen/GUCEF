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

#ifndef GUCEF_CORE_CJOBTRIGGER_H
#define GUCEF_CORE_CJOBTRIGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CSCHEDULE_H
#include "gucefCORE_CSchedule.h"
#define GUCEF_CORE_CSCHEDULE_H
#endif /* GUCEF_CORE_CSCHEDULE_H ? */

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#include "gucefCORE_JobSchedulerTypes.h"
#define GUCEF_CORE_JOBSCHEDULERTYPES_H
#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */

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
 *  When a job becomes due. Built on the existing schedule classes: a SCHEDULE trigger holds a CSchedule
 *  (CCronSchedule and/or CDateTimeRange) which the scheduler evaluates with GetNextOccurrence() and
 *  GetPreviousOccurrence(), in UTC only. A trigger has no time zone: work that follows a local calendar is decided by a
 *  CIJobWorkSource, which hands the scheduler UTC instants.
 *
 *  Serialized form (a CDataNode object): "kind" (JobTriggerKindToString), "cron" (CCronSchedule::ToCronString()),
 *  "rangeStartUtc" / "rangeEndUtc" (ISO 8601 UTC, when the schedule has a range), "periodMs", "intervalFrom"
 *  ("START", "END"), "alignToUtcMs" (only when aligned), "onceAtUtcMs".
 */
class GUCEF_CORE_PUBLIC_CPP CJobTrigger : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;

    CJobTrigger( void );

    virtual ~CJobTrigger() GUCEF_VIRTUAL_OVERRIDE;

    static CJobTrigger Schedule( const CSchedule& schedule );
    static CJobTrigger Interval( UInt64 periodMs, TJobIntervalFrom intervalFrom = JOBINTERVALFROM_START );
    static CJobTrigger OnceAt( Int64 utcMs );
    static CJobTrigger Manual( void );
    static CJobTrigger WorkSource( void );

    void SetKind( TJobTriggerKind kind );
    TJobTriggerKind GetKind( void ) const;

    /** SCHEDULE: cron and/or date time range, always matched in UTC (no time zone field) */
    void SetSchedule( const CSchedule& schedule );
    const CSchedule& GetSchedule( void ) const;

    /** INTERVAL: the period */
    void SetPeriodMs( UInt64 periodMs );
    UInt64 GetPeriodMs( void ) const;

    /** INTERVAL: measured between run starts or run ends */
    void SetIntervalFrom( TJobIntervalFrom intervalFrom );
    TJobIntervalFrom GetIntervalFrom( void ) const;

    /** INTERVAL: whether due instants are aligned to AlignToUtcMs (a phase), else the period counts from Start() */
    void SetHasAlignment( bool hasAlignment );
    bool GetHasAlignment( void ) const;

    /** INTERVAL: phase of the due instants when HasAlignment */
    void SetAlignToUtcMs( Int64 alignToUtcMs );
    Int64 GetAlignToUtcMs( void ) const;

    /** ONCE_AT: the instant */
    void SetOnceAtUtcMs( Int64 onceAtUtcMs );
    Int64 GetOnceAtUtcMs( void ) const;
    /** Text for views: "cron 0 2 * * * UTC", "every 60000 ms", "once at 2026-11-15T10:00:00Z", "manual", "work source" */
    CString GetDescription( void ) const;

    /** Whether the fields needed by the kind are set and valid; the reason otherwise */
    bool IsValid( CString& error ) const;

    bool operator==( const CJobTrigger& other ) const;

    virtual bool Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    TJobTriggerKind m_kind;
    CSchedule m_schedule;
    UInt64 m_periodMs;
    TJobIntervalFrom m_intervalFrom;
    bool m_hasAlignment;
    Int64 m_alignToUtcMs;
    Int64 m_onceAtUtcMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBTRIGGER_H ? */
