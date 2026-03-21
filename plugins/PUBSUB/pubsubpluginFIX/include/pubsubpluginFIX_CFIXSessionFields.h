/*
 *  pubsubpluginFIX: Generic GUCEF COMCORE plugin for providing pubsub via FIX protocol
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef PUBSUBPLUGIN_FIX_CFIXSESSIONFIELDS_H
#define PUBSUBPLUGIN_FIX_CFIXSESSIONFIELDS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef PUBSUBPLUGIN_FIX_MACROS_H
#include "pubsubpluginFIX_macros.h"
#define PUBSUBPLUGIN_FIX_MACROS_H
#endif /* PUBSUBPLUGIN_FIX_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace FIX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Stack-allocated struct holding raw pointer views into the receive buffer.
 *  All pointers are only valid until ProcessReceiveBuffer compacts the buffer
 *  (i.e., the memmove that follows the dispatch loop).
 *
 *  IMPORTANT: Downstream consumers (topics, observers) MUST NOT hold references
 *  to these pointers beyond the synchronous notification callback.
 *  NotifyObservers is synchronous, so all observers finish before the buffer
 *  is compacted.
 */
struct PUBSUBPLUGIN_FIX_PLUGIN_PRIVATE_CPP CFIXSessionFields
{
    const char* beginStringStart; CORE::UInt32 beginStringLen;  // tag 8
    const char* msgTypeStart;     CORE::UInt32 msgTypeLen;      // tag 35
    const char* seqNumStart;      CORE::UInt32 seqNumLen;       // tag 34
    const char* senderStart;      CORE::UInt32 senderLen;       // tag 49
    const char* targetStart;      CORE::UInt32 targetLen;       // tag 56
    const char* hbIntStart;       CORE::UInt32 hbIntLen;        // tag 108
    const char* testReqIdStart;   CORE::UInt32 testReqIdLen;    // tag 112
    const char* resetFlagStart;   CORE::UInt32 resetFlagLen;    // tag 141
    const char* possDupFlagStart; CORE::UInt32 possDupFlagLen;  // tag 43
    const char* beginSeqNoStart;  CORE::UInt32 beginSeqNoLen;   // tag 7
    const char* newSeqNoStart;    CORE::UInt32 newSeqNoLen;     // tag 36
    CORE::UInt64 seqNumVal;                                      // tag 34 parsed inline - no allocation

    CFIXSessionFields( void )
    {
        ::memset( this, 0, sizeof( CFIXSessionFields ) );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_FIX_CFIXSESSIONFIELDS_H ? */
