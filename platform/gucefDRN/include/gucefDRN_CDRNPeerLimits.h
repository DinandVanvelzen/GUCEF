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

#ifndef GUCEF_DRN_CDRNPEERLIMITS_H
#define GUCEF_DRN_CDRNPEERLIMITS_H

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

/**
 *  The limits that apply to one member of a session on the serving side of the resource transfers: the per-member
 *  values of CDRNResourceTransferSettings, as configured or as CIDRNPeerLimitPolicy made them for the member. 0 means
 *  no maximum for every value. They are independent of the server-wide limits (those of all members together): a
 *  request is refused when either is reached.
 */
class GUCEF_DRN_EXPORT_CPP CDRNPeerLimits
{
    public:

    UInt32 maxOpenHandles;              /**< resources the member holds open at once */
    UInt32 maxActiveReads;              /**< reads of the member served at once */
    UInt32 maxNamespaceRequests;        /**< namespace requests (STAT, LIST) of the member being worked on at once */
    UInt32 maxNamespaceListings;        /**< listings the member has under way */
    UInt32 maxUploadBytesPerSecond;     /**< bytes per second sent to the member by all its reads together */
    UInt32 maxBufferedBytes;            /**< bytes read for the member and not yet sent */

    CDRNPeerLimits( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNPEERLIMITS_H ? */
