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

#include "gucefDRN_CDRNResourceTransferStats.h"

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

CDRNResourceTransferStats::CDRNResourceTransferStats( void )
    : opensServed( 0 )
    , opensRefused( 0 )
    , readsServed( 0 )
    , readsFailed( 0 )
    , chunksServed( 0 )
    , bytesServed( 0 )
    , treesBuilt( 0 )
    , treesLoaded( 0 )
    , handlesExpired( 0 )
    , serverBufferedBytes( 0 )
    , peakServerBufferedBytes( 0 )
    , transfersStarted( 0 )
    , transfersCompleted( 0 )
    , transfersFailed( 0 )
    , chunksReceived( 0 )
    , bytesReceived( 0 )
    , chunksVerified( 0 )
    , verificationFailures( 0 )
    , namespaceRequestsServed( 0 )
    , namespaceRequestsRefused( 0 )
    , namespaceMalformed( 0 )
    , namespaceListingsOpened( 0 )
    , namespaceListingsExpired( 0 )
    , namespaceEntriesServed( 0 )
    , namespaceEntriesInvalidName( 0 )
    , namespaceEntriesHidden( 0 )
    , namespaceRequestsMade( 0 )
    , namespaceAnswersReceived( 0 )
    , namespaceAnswersDropped( 0 )
    , refusedByServerLimit( 0 )
    , refusedByMemberLimit( 0 )
    , droppedMessages( 0 )
    , clientBufferedBytes( 0 )
    , peakClientBufferedBytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
