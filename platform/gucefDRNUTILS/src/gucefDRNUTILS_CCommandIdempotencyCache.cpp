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

#include "gucefDRNUTILS_CCommandIdempotencyCache.h"

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

CCommandIdempotencyCache::SOutcome::SOutcome( void )
    : status( (TDrnUtilsCommandStatus) 0 )
    , result()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCommandIdempotencyCache::SOutcome::SOutcome( TDrnUtilsCommandStatus commandStatus, const CORE::SJobControlResult& commandResult )
    : status( commandStatus )
    , result( commandResult )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCommandIdempotencyCache::CCommandIdempotencyCache( void )
    : m_cache()
{GUCEF_TRACE;

    m_cache.SetBounds( 86400000, 256, 4096 );
}

/*-------------------------------------------------------------------------*/

void
CCommandIdempotencyCache::SetBounds( UInt64 ttlMs, UInt32 maxPerSubject, UInt32 maxTotal )
{GUCEF_TRACE;

    m_cache.SetBounds( ttlMs, maxPerSubject, maxTotal );
}

/*-------------------------------------------------------------------------*/

bool
CCommandIdempotencyCache::Find( const CORE::CString& subject, const CORE::CString& commandId, UInt64 nowMs,
                                TDrnUtilsCommandStatus& status, CORE::SJobControlResult& result )
{GUCEF_TRACE;

    SOutcome outcome;
    if ( !m_cache.Find( subject, commandId, nowMs, outcome ) )
        return false;
    status = outcome.status;
    result = outcome.result;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCommandIdempotencyCache::Store( const CORE::CString& subject, const CORE::CString& commandId, UInt64 nowMs,
                                 TDrnUtilsCommandStatus status, const CORE::SJobControlResult& result )
{GUCEF_TRACE;

    m_cache.Store( subject, commandId, nowMs, SOutcome( status, result ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CCommandIdempotencyCache::GetCount( void ) const
{GUCEF_TRACE;

    return m_cache.GetCount();
}

/*-------------------------------------------------------------------------*/

void
CCommandIdempotencyCache::Clear( void )
{GUCEF_TRACE;

    m_cache.Clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
