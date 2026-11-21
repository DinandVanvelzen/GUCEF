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

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFERSTATS_H
#define GUCEF_DRN_CDRNRESOURCETRANSFERSTATS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Counters of the resource transfers of a session, both roles, since it was made */
class GUCEF_DRN_EXPORT_CPP CDRNResourceTransferStats
{
    public:

    UInt64 opensServed;             /**< opens answered with the resource */
    UInt64 opensRefused;            /**< opens answered with another status */
    UInt64 readsServed;             /**< reads served to their end */
    UInt64 readsFailed;             /**< reads ended with an error */
    UInt64 chunksServed;            /**< chunks sent */
    UInt64 bytesServed;             /**< bytes of chunks sent */
    UInt64 treesBuilt;              /**< hash trees built */
    UInt64 treesLoaded;             /**< hash trees loaded from the tree cache directory */
    UInt64 handlesExpired;          /**< open resources closed for being idle */
    UInt64 serverBufferedBytes;     /**< bytes read and not yet sent, now */
    UInt64 peakServerBufferedBytes; /**< the most bytes read and not yet sent at once */
    UInt64 transfersStarted;        /**< transfers this side started */
    UInt64 transfersCompleted;      /**< transfers that completed */
    UInt64 transfersFailed;         /**< transfers that failed or were cancelled */
    UInt64 chunksReceived;          /**< chunks taken in */
    UInt64 bytesReceived;           /**< bytes of chunks taken in */
    UInt64 chunksVerified;          /**< chunks checked against a hash tree */
    UInt64 verificationFailures;    /**< chunks that failed their check */
    UInt64 namespaceRequestsServed; /**< namespace requests answered with their result (OK, UNCHANGED, NOT_FOUND ...) */
    UInt64 namespaceRequestsRefused;/**< namespace requests refused by a limit, the rules, the policy or because they were malformed */
    UInt64 namespaceMalformed;      /**< namespace messages that could not be read (answered PROTOCOL_ERROR when the request id could be) */
    UInt64 namespaceListingsOpened; /**< listings that were continued with a cursor, opened */
    UInt64 namespaceListingsExpired;/**< listings forgotten because their member did not continue them in time, or left */
    UInt64 namespaceEntriesServed;  /**< entries of listings and stats sent */
    UInt64 namespaceEntriesInvalidName; /**< entries of a listing left out because their name is not one the resource name rules allow */
    UInt64 namespaceEntriesHidden;  /**< entries of a listing left out by the share rules or the access policy */
    UInt64 namespaceRequestsMade;   /**< namespace requests this side sent */
    UInt64 namespaceAnswersReceived;/**< namespace answers taken in */
    UInt64 namespaceAnswersDropped; /**< namespace answers of no request this side waits for, or malformed */
    UInt64 refusedByServerLimit;    /**< serving: requests (opens, reads, namespace requests, new listings) refused BUSY by a server-wide limit */
    UInt64 refusedByMemberLimit;    /**< serving: requests refused BUSY by the limit of the member (the per-member value, as a CIDRNPeerLimitPolicy made it) */
    UInt64 droppedMessages;         /**< messages dropped: malformed, unknown, beyond credit, of ended reads */
    UInt64 clientBufferedBytes;     /**< bytes received and not yet written, now */
    UInt64 peakClientBufferedBytes; /**< the most bytes received and not yet written at once */

    CDRNResourceTransferStats( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCETRANSFERSTATS_H ? */
