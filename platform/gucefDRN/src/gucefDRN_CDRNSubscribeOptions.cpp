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

#include "gucefDRN_CDRNSubscribeOptions.h"

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

CDRNSubscribeOptions::CDRNSubscribeOptions( void )
    : fromSeq( 0 )
    , expectedEpoch( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSubscribeOptions::CDRNSubscribeOptions( UInt64 resumeFromSeq, UInt64 epoch )
    : fromSeq( resumeFromSeq )
    , expectedEpoch( epoch )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNSubscribeOptions::operator==( const CDRNSubscribeOptions& other ) const
{GUCEF_TRACE;

    return fromSeq == other.fromSeq && expectedEpoch == other.expectedEpoch;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSubscribeOptions::operator!=( const CDRNSubscribeOptions& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
