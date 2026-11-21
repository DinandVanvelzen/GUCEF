/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_ICiDeployer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

ICiDeployer::CState::CState( void )
    : image()
    , imagePullPolicy()
    , revision()
    , changeCause()
    , strategy()
    , replicas( 0 )
    , readyReplicas( 0 )
    , updatedReplicas( 0 )
    , availableReplicas( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
ICiDeployer::CState::GetDigest( void ) const
{GUCEF_TRACE;

    const Int32 at = image.HasChar( '@', false );
    return at > 0 ? image.CutChars( (UInt32) at + 1, true ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
ICiDeployer::CState::DescribeReplicas( void ) const
{GUCEF_TRACE;

    return CORE::UInt32ToString( readyReplicas ) + " of " + CORE::UInt32ToString( replicas ) + " ready, " + CORE::UInt32ToString( updatedReplicas ) +
           " updated, " + CORE::UInt32ToString( availableReplicas ) + " available";
}

/*-------------------------------------------------------------------------*/

ICiDeployer::~ICiDeployer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const char*
ICiDeployer::RolloutResultToString( ERolloutResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case ROLLOUT_COMPLETE: return "is complete";
        case ROLLOUT_FAILED: return "failed";
        case ROLLOUT_TIMEOUT: return "timed out";
        case ROLLOUT_CANCELLED: return "was no longer followed";
        case ROLLOUT_ERROR: return "could not be followed";
        default: return "is in an unknown state";
    }
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
