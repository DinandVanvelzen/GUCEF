/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include "gucefDRN_CDRNStreamGapInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNStreamGapInfo::CDRNStreamGapInfo( void )
    : fromSeq( 0 )
    , toSeq( 0 )
    , reason( DRN_STREAMGAP_TRIMMED )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNStreamGapInfo::CDRNStreamGapInfo( UInt64 gapFromSeq, UInt64 gapToSeq, TDRNStreamGapReason gapReason )
    : fromSeq( gapFromSeq )
    , toSeq( gapToSeq )
    , reason( gapReason )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CDRNStreamGapInfo::GetEntryCount( void ) const
{GUCEF_TRACE;

    if ( 0 == toSeq || toSeq < fromSeq )
        return 0;
    return toSeq - fromSeq + 1;
}

/*-------------------------------------------------------------------------*/

const char*
CDRNStreamGapInfo::ReasonToString( TDRNStreamGapReason reason )
{GUCEF_TRACE;

    switch ( reason )
    {
        case DRN_STREAMGAP_TRIMMED: return "trimmed";
        case DRN_STREAMGAP_BEYOND_RETENTION: return "beyond retention";
        case DRN_STREAMGAP_EPOCH_CHANGED: return "epoch changed";
        default: return "unknown";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
