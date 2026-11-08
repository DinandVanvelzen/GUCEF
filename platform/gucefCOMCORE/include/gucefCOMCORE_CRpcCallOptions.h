/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCCALLOPTIONS_H
#define GUCEF_COMCORE_CRPCCALLOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCMETADATA_H
#include "gucefCOMCORE_CRpcMetadata.h"
#define GUCEF_COMCORE_CRPCMETADATA_H
#endif /* GUCEF_COMCORE_CRPCMETADATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Options of one call (gucefCOMCORE RPC design 10.3, 6.8, 8.5, 8.6)
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcCallOptions
{
    public:

    UInt32 deadlineMs;                      /**< 0 = the method's default deadline (which may be none) */
    UInt32 resultTimeoutMs;                 /**< after ACCEPTED: how long to wait for RESULT; 0 = no limit */
    bool keepRunningAfterResultTimeout;     /**< when the result timeout expires: no CANCEL to the provider */
    bool waitForReady;                      /**< while the session is suspended the call waits for the resume */
    bool retryIdempotentOnNewSession;       /**< after RESUME_GAP / SESSION_UNKNOWN an idempotent call is issued again */
    bool hasResumeFromIndex;                /**< resumable streams: start after resumeFromIndex */
    UInt64 resumeFromIndex;
    CRpcMetadata metadata;

    CRpcCallOptions( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCCALLOPTIONS_H ? */
