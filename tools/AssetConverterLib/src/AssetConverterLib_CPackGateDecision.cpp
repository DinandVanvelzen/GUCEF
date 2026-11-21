/*
 *  AssetConverterLib: the answer of the work gate: whether a stored result may be used and why not
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

#include "AssetConverterLib_CPackGateDecision.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackGateDecision::CPackGateDecision( void )
    : outcome( OUTCOME_MISSING )
    , reason( ACTIVITYREASON_NEW )
    , detail()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackGateDecision::CPackGateDecision( EOutcome decided, TActivityReason why, const CORE::CString& why2 )
    : outcome( decided )
    , reason( why )
    , detail( why2 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackGateDecision::NeedsWork( void ) const
{GUCEF_TRACE;

    return OUTCOME_STALE == outcome || OUTCOME_MISSING == outcome || OUTCOME_FORCED == outcome;
}

/*-------------------------------------------------------------------------*/

bool
CPackGateDecision::IsFresh( void ) const
{GUCEF_TRACE;

    return OUTCOME_FRESH == outcome;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackGateDecision::GetOutcomeName( void ) const
{GUCEF_TRACE;

    switch ( outcome )
    {
        case OUTCOME_FRESH:     return "fresh";
        case OUTCOME_STALE:     return "stale";
        case OUTCOME_MISSING:   return "missing";
        case OUTCOME_FORCED:    return "forced";
        case OUTCOME_REFUSED:   return "refused";
    }
    return "unknown";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackGateDecision::ToString( void ) const
{GUCEF_TRACE;

    CORE::CString text = GetOutcomeName() + ": " + ASSETCONVERTER::ToString( reason );
    if ( !detail.IsNULLOrEmpty() )
        text += " (" + detail + ")";
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
