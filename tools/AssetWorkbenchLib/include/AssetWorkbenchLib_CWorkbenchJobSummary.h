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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBSUMMARY_H
#define ASSETWORKBENCHLIB_CWORKBENCHJOBSUMMARY_H

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

/** The counts of the jobs list that the status area shows: how many run, wait and failed, and how many are done and were cancelled */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchJobSummary
{
    public:

    UInt32 running;     /**< running, finishing and paused */
    UInt32 queued;
    UInt32 failed;
    UInt32 done;
    UInt32 cancelled;

    CWorkbenchJobSummary( void );

    /** Counts the jobs; a job that failed and was taken up again since still counts as failed until it is removed */
    static CWorkbenchJobSummary Count( const ASSETSTORE::TAssetJobEntryVector& jobs );

    UInt32 GetTotal( void ) const;

    /** "Jobs: 2 running, 1 queued, 0 failed" */
    CORE::CString ToText( void ) const;

    /** The short form for the status strip, only what there is: "Jobs: idle", "Jobs: 2 run, 1 failed" */
    CORE::CString ToShortText( void ) const;

    bool operator==( const CWorkbenchJobSummary& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBSUMMARY_H ? */
