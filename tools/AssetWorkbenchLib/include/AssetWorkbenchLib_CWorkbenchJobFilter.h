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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#define ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  Which jobs of the jobs list a view shows: by the state group (all, active, running, queued, failed, done, cancelled) and by the
 *  kind that the list shows for a job (DisplayKindOf(): the sub kind of a pipeline job, else its kind), empty for every kind.
 *  The state groups are fixed names, so that a combo box and a console command use the same words.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchJobFilter
{
    public:

    enum EStateGroup
    {
        STATES_ALL = 0  ,
        STATES_ACTIVE   ,   /**< queued, running, paused and finishing */
        STATES_RUNNING  ,
        STATES_QUEUED   ,
        STATES_FAILED   ,
        STATES_DONE     ,
        STATES_CANCELLED,
        STATES_FINISHED ,   /**< the history: done, failed and cancelled */
        STATES_COUNT
    };

    EStateGroup states;
    CORE::CString kind;         /**< a display kind; empty for all */

    CWorkbenchJobFilter( void );

    bool Matches( const ASSETSTORE::CAssetJobEntry& job ) const;

    /** The kind the list shows: "convert", "cook", "validate", "index", "analyse", "thumbnail", "open", "download", ... */
    static CORE::CString DisplayKindOf( const ASSETSTORE::CAssetJobEntry& job );

    /** The state in words as the list shows it ("running", "queued", "failed", the state text of the job when it has one) */
    /** Whether the list counts the job as failed: the service says failed, or it is done but some assets failed */
    static bool IsFailed( const ASSETSTORE::CAssetJobEntry& job );

    static CORE::CString StateTextOf( const ASSETSTORE::CAssetJobEntry& job );

    static const char* GetStateGroupName( EStateGroup group );
    static bool ParseStateGroup( const CORE::CString& name, EStateGroup& group );

    /** The kinds a job can have in the list, for the choice of the filter */
    static void GetKnownKinds( CORE::CString::StringVector& kinds );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H ? */
