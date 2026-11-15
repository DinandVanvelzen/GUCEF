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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefDRNUTILS_CSchedulerWatchStream.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSchedulerWatchStream::CSchedulerWatchStream( void )
    : call()
    , principal()
    , sessionKey()
    , includeRuns( false )
    , maxMessagesPerSecond( 0 )
    , windowStartMs( 0 )
    , sentInWindow( 0 )
    , needsSnapshot( false )
    , throttled( false )
    , pending()
    , outbox()
    , outboxPosition( 0 )
    , messagesWritten( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSchedulerWatchStream::AddPending( Int32 kind, const CORE::CString& key, UInt64 revision )
{GUCEF_TRACE;

    SPending& entry = pending[ CORE::ToString( kind ) + ":" + key ];
    entry.kind = kind;
    entry.key = key;
    entry.revision = revision;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
