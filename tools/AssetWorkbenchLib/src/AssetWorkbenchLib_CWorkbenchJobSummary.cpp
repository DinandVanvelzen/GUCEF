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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#include "AssetWorkbenchLib_CWorkbenchJobFilter.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBSUMMARY_H
#include "AssetWorkbenchLib_CWorkbenchJobSummary.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBSUMMARY_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBSUMMARY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWorkbenchJobSummary::CWorkbenchJobSummary( void )
    : running( 0 )
    , queued( 0 )
    , failed( 0 )
    , done( 0 )
    , cancelled( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWorkbenchJobSummary
CWorkbenchJobSummary::Count( const ASSETSTORE::TAssetJobEntryVector& jobs )
{GUCEF_TRACE;

    CWorkbenchJobSummary summary;
    for ( size_t i=0; i<jobs.size(); ++i )
    {
        switch ( jobs[ i ].state )
        {
            case ASSETSTORE::ASSETJOB_STATE_QUEUED:    ++summary.queued; break;
            case ASSETSTORE::ASSETJOB_STATE_FAILED:    ++summary.failed; break;
            case ASSETSTORE::ASSETJOB_STATE_DONE:      ++( CWorkbenchJobFilter::IsFailed( jobs[ i ] ) ? summary.failed : summary.done ); break;
            case ASSETSTORE::ASSETJOB_STATE_CANCELLED: ++summary.cancelled; break;
            default:                                   ++summary.running; break;
        }
    }
    return summary;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchJobSummary::GetTotal( void ) const
{GUCEF_TRACE;

    return running + queued + failed + done + cancelled;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobSummary::ToText( void ) const
{GUCEF_TRACE;

    return "Jobs: " + CORE::UInt32ToString( running ) + " running, " + CORE::UInt32ToString( queued ) + " queued, " + CORE::UInt32ToString( failed ) + " failed";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchJobSummary::ToShortText( void ) const
{GUCEF_TRACE;

    // only what there is, so that the status strip has room for it: "Jobs: idle", "Jobs: 2 run, 1 failed"
    CORE::CString text;
    if ( running > 0 )
        text += CORE::UInt32ToString( running ) + " run";
    if ( queued > 0 )
        text += ( text.IsNULLOrEmpty() ? "" : ", " ) + CORE::UInt32ToString( queued ) + " wait";
    if ( failed > 0 )
        text += ( text.IsNULLOrEmpty() ? "" : ", " ) + CORE::UInt32ToString( failed ) + " failed";
    return "Jobs: " + ( text.IsNULLOrEmpty() ? CORE::CString( "idle" ) : text );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchJobSummary::operator==( const CWorkbenchJobSummary& other ) const
{GUCEF_TRACE;

    return running == other.running && queued == other.queued && failed == other.failed && done == other.done && cancelled == other.cancelled;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
