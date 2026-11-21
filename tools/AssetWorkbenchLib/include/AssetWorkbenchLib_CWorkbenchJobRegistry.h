/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H
#define ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_GUI_CACTIVITYSERVICE_H
#include "gucefGUI_CActivityService.h"
#define GUCEF_GUI_CACTIVITYSERVICE_H
#endif /* GUCEF_GUI_CACTIVITYSERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#include "gucefASSETSTORE_CAssetJobEntry.h"
#define GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETJOBENTRY_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The long operations of this application that no service runs for it: opening an asset, building a pack index, the presentation of a
 *  pack, an automatic thumbnail. Each one announces itself with Begin() and ends with End(); the registry keeps them as job entries
 *  (ASSETSTORE::CAssetJobEntry, ids "local:<n>") so that the jobs overview lists them next to the jobs of the asset store service.
 *
 *  The 300 ms rule: an operation is a job for the list only when it ran for the visible-after time (300 ms by default) or announced a
 *  longer expected duration, so the many short operations leave no trace; one that ended sooner is dropped. A job of an operation of the
 *  activity service is linked to it (LinkActivity()): the progress and the cancel button of the activity are those of the job, and
 *  FindByActivity() takes the indicator to its job.
 *
 *  The ended jobs are kept up to MaxEndedJobs, the oldest go first. Everything here is for the thread of the interface; the clock is
 *  replaceable for tests (milliseconds, monotonic).
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchJobRegistry : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent JobsChangedEvent;         /**< no data: a job began, changed, ended or was removed */
    static void RegisterEvents( void );

    static const CORE::CString IdPrefix;                /**< "local:": the ids of the jobs of the registry start with it */

    enum EDefaults
    {
        DefaultVisibleAfterMs = 300 ,
        MaxEndedJobs          = 200
    };

    /** A clock in milliseconds; the default is the precision timer */
    typedef UInt64 ( *TClockFunction )( void* userData );

    CWorkbenchJobRegistry( void );

    virtual ~CWorkbenchJobRegistry() GUCEF_VIRTUAL_OVERRIDE;

    void SetClock( TClockFunction clock, void* userData );

    void SetVisibleAfterMs( UInt32 visibleAfterMs );

    UInt32 GetVisibleAfterMs( void ) const;

    /** The activity service whose operations jobs may be linked to (not owned) */
    void SetActivityService( GUI::CActivityService* activity );

    /**
     *  An operation begins. 'kind' is "open", "index", "presentation", "thumbnail", ...; 'subKind' what it does within the kind
     *  ("analyse" of an index); 'scope' what it works on. Answers the number of the job, never 0
     */
    UInt32 Begin( const CORE::CString& kind, const CORE::CString& subKind, const CORE::CString& name, const CORE::CString& scope,
                  bool cancellable, UInt32 expectedDurationInMs = 0 );

    void LinkActivity( UInt32 number, UInt32 activityId );

    /** The job of an operation of the activity service, 0 when there is none (or it was never a job) */
    UInt32 FindByActivity( UInt32 activityId ) const;

    void SetProgress( UInt32 number, UInt32 done, UInt32 total );

    void SetNote( UInt32 number, const CORE::CString& note );

    /** The operation is over: ASSETJOB_STATE_DONE, FAILED or CANCELLED, with the outcome in words */
    void End( UInt32 number, ASSETSTORE::TAssetJobState state, const CORE::CString& resultText );

    /** Asks the operation to stop (its activity is asked); false when the job cannot be cancelled */
    bool RequestCancel( UInt32 number );

    bool IsCancelRequested( UInt32 number ) const;

    /** The jobs that are visible now, in the order they began */
    void GetJobs( ASSETSTORE::TAssetJobEntryVector& jobs ) const;

    bool GetJob( const CORE::CString& jobId, ASSETSTORE::CAssetJobEntry& job ) const;

    /** Takes an ended job out of the list */
    bool Remove( const CORE::CString& jobId );

    /** The job entry of a job id of the registry: "local:3" has the number 3; 0 when the id is not one of the registry */
    static UInt32 NumberOfId( const CORE::CString& jobId );

    static bool IsLocalId( const CORE::CString& jobId );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SJob
    {
        UInt32 number;
        UInt32 activityId;
        CORE::CString kind;
        CORE::CString subKind;
        CORE::CString name;
        CORE::CString scope;
        CORE::CString note;
        CORE::CString resultText;
        bool cancellable;
        bool cancelRequested;
        bool ended;
        bool visible;                   /**< was seen as a job: it ran long enough or announced that it would */
        bool announcedLong;
        ASSETSTORE::TAssetJobState state;
        UInt32 done;
        UInt32 total;
        UInt64 startedMs;               /**< the clock of the registry */
        UInt64 endedMs;
        Int64 startedWallMs;            /**< UTC milliseconds since the Unix epoch */
        SJob( void );
    };
    typedef GUCEF::vector< SJob > TJobVector;

    CWorkbenchJobRegistry( const CWorkbenchJobRegistry& src );
    CWorkbenchJobRegistry& operator=( const CWorkbenchJobRegistry& src );

    UInt64 Now( void ) const;
    SJob* Find( UInt32 number );
    const SJob* Find( UInt32 number ) const;
    bool IsVisible( const SJob& job ) const;
    void MakeEntry( const SJob& job, ASSETSTORE::CAssetJobEntry& entry ) const;
    void TrimEnded( void );

    private:

    TJobVector m_jobs;
    UInt32 m_nextNumber;
    UInt32 m_visibleAfterMs;
    TClockFunction m_clock;
    void* m_clockData;
    GUI::CActivityService* m_activity;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H ? */
