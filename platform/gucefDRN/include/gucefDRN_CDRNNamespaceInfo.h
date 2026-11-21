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

#ifndef GUCEF_DRN_CDRNNAMESPACEINFO_H
#define GUCEF_DRN_CDRNNAMESPACEINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

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

/**
 *  What a serving side tells in NS_HELLO: the version of the namespace protocol both sides speak and the limits of the
 *  serving side (the defaults of every share; a share may be stricter, CDRNNamespaceShareInfo). A requester stays within
 *  them, the serving side enforces them whatever the requester does.
 *
 *  The limits of the member that asked (the lower of the server-wide limit and the member's own, 0 = no maximum or not
 *  announced) are an optional extension of the answer: a serving side that does not announce them, or one that predates
 *  them, leaves 'limitsAnnounced' false and the requester treats the limits as unknown (it does not throttle itself). They
 *  are the caps that apply, not the capacity that is free at the moment: others may be using the server-wide budget.
 */
class GUCEF_DRN_EXPORT_CPP CDRNNamespaceInfo
{
    public:

    UInt32 version;                 /**< the lower of the versions both sides speak (DRN_NS_VERSION) */
    UInt32 capabilities;            /**< DRN_NS_CAP_*: what the serving side implements at all */
    UInt32 maxStatPaths;            /**< paths in one STAT, default 256 */
    UInt32 maxPageEntries;          /**< entries in one page of a listing; also what a page request that names no number gets, default 1000 */
    UInt32 maxAnswerBytes;          /**< the largest answer, which a page is cut to */
    UInt32 listingLifetimeMs;       /**< a listing the requester does not continue within this time is forgotten (CURSOR_EXPIRED) */
    bool limitsAnnounced;           /**< the answer carried the limits below; false: they are unknown (all 0) */
    UInt32 maxOpenHandles;          /**< resources the asking member may hold open at once, 0 = no maximum or unknown */
    UInt32 maxActiveReads;          /**< reads of the asking member served at once, 0 = no maximum or unknown */
    UInt32 maxNamespaceRequests;    /**< STAT and LIST requests of the asking member worked on at once, 0 = no maximum or unknown */
    UInt32 maxNamespaceListings;    /**< listings the asking member may have under way, 0 = no maximum or unknown */
    UInt32 maxUploadBytesPerSecond; /**< bytes per second sent to the asking member, 0 = no limit or unknown */

    CDRNNamespaceInfo( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNAMESPACEINFO_H ? */
