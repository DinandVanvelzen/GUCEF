/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMULTIPLAY_CIMatchPackageVerifier.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
MatchPackageVerdictToString( TMatchPackageVerdict verdict )
{GUCEF_TRACE;

    switch ( verdict )
    {
        case MATCHPACKAGE_VERIFIED: return "VERIFIED";
        case MATCHPACKAGE_NO_BASE:  return "NO_BASE";
        default:                    return "REJECTED";
    }
}

/*-------------------------------------------------------------------------*/

CIMatchPackageVerifier::CIMatchPackageVerifier( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIMatchPackageVerifier::~CIMatchPackageVerifier()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CIMatchPackageVerifier::GetRefusalCode( void ) const
{GUCEF_TRACE;

    return "NOT_PLAYABLE";
}

/*-------------------------------------------------------------------------*/

UInt32
CIMatchPackageVerifier::GetDecisionEvidence( const CMatchInfo& info, const CMatchPackage& package, TEvidenceVector& evidence )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
