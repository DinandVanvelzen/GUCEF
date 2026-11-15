/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#include "gucefWEB_CWebViewEngineSelection.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebViewEngineSelection::SCandidate::SCandidate( void )
    : engineName()
    , outcome( OUTCOME_NOT_TRIED )
    , reason()
    , missingCapabilities( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewEngineSelection::CWebViewEngineSelection( void )
    : engine()
    , candidates()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineSelection::HasEngine( void ) const
{GUCEF_TRACE;

    return !engine.IsNULL();
}

/*-------------------------------------------------------------------------*/

const char*
CWebViewEngineSelection::OutcomeToString( TOutcome outcome )
{GUCEF_TRACE;

    switch ( outcome )
    {
        case OUTCOME_CHOSEN: return "chosen";
        case OUTCOME_NOT_LOADED: return "not loaded";
        case OUTCOME_NOT_AVAILABLE: return "not available";
        case OUTCOME_CAPABILITY_MISSING: return "capability missing";
        case OUTCOME_STALLED: return "stalled";
        default: return "not tried";
    }
}

/*-------------------------------------------------------------------------*/

CString
CWebViewEngineSelection::Describe( void ) const
{GUCEF_TRACE;

    CString text;
    for ( TCandidateVector::const_iterator i=candidates.begin(); i!=candidates.end(); ++i )
    {
        if ( !text.IsNULLOrEmpty() )
            text += "\n";
        text += (*i).engineName + ": " + OutcomeToString( (*i).outcome );
        if ( !(*i).reason.IsNULLOrEmpty() )
            text += ", " + (*i).reason;
    }
    if ( candidates.empty() )
        text = "No web view engine is loaded";
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
