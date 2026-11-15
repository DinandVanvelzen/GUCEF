/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CSCHEDULERWATCHSTREAM_H
#define GUCEF_DRNUTILS_CSCHEDULERWATCHSTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_COMCORE_CRPCPRINCIPAL_H
#include "gucefCOMCORE_CRpcPrincipal.h"
#define GUCEF_COMCORE_CRPCPRINCIPAL_H
#endif /* GUCEF_COMCORE_CRPCPRINCIPAL_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Module-private: one Scheduler/Watch call (design 6.5): its cursor, its pending changes (a map by kind and key,
 *  latest wins, bounded by the rows of the model), the messages built and not yet written (credit), and its message
 *  rate. Used on the host's notification thread only, so it has no lock.
 */
class CSchedulerWatchStream
{
    public:

    struct SPending
    {
        Int32 kind;
        CORE::CString key;
        UInt64 revision;
    };
    typedef GUCEF::map< CORE::CString, SPending > TPendingMap;
    typedef GUCEF::vector< COMCORE::CRpcMessage > TMessageVector;

    COMCORE::CRpcServerCallPtr call;
    COMCORE::CRpcPrincipal principal;
    CORE::CString sessionKey;           /**< the RPC session (hex), empty when the call has none */
    bool includeRuns;
    UInt32 maxMessagesPerSecond;        /**< 0 = no limit */
    UInt64 windowStartMs;
    UInt32 sentInWindow;
    bool needsSnapshot;
    bool throttled;                     /**< waits for the next second of its message rate */
    TPendingMap pending;
    TMessageVector outbox;
    UInt32 outboxPosition;
    UInt64 messagesWritten;

    CSchedulerWatchStream( void );

    void AddPending( Int32 kind, const CORE::CString& key, UInt64 revision );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERWATCHSTREAM_H ? */
