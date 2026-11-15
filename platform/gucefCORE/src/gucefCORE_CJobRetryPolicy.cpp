/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CJobRetryPolicy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CJobRetryPolicy::CJobRetryPolicy( void )
    : m_maxAttempts( 1 )
    , m_backoffInitialMs( 1000 )
    , m_backoffMaxMs( 60000 )
    , m_jitterPercent( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CJobRetryPolicy::operator==( const CJobRetryPolicy& other ) const
{GUCEF_TRACE;

    return m_maxAttempts == other.m_maxAttempts           &&
           m_backoffInitialMs == other.m_backoffInitialMs &&
           m_backoffMaxMs == other.m_backoffMaxMs         &&
           m_jitterPercent == other.m_jitterPercent;
}

/*-------------------------------------------------------------------------*/

void
CJobRetryPolicy::SetMaxAttempts( UInt32 maxAttempts )
{GUCEF_TRACE;

    m_maxAttempts = maxAttempts;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobRetryPolicy::GetMaxAttempts( void ) const
{GUCEF_TRACE;

    return m_maxAttempts;
}

/*-------------------------------------------------------------------------*/

void
CJobRetryPolicy::SetBackoffInitialMs( UInt64 backoffInitialMs )
{GUCEF_TRACE;

    m_backoffInitialMs = backoffInitialMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobRetryPolicy::GetBackoffInitialMs( void ) const
{GUCEF_TRACE;

    return m_backoffInitialMs;
}

/*-------------------------------------------------------------------------*/

void
CJobRetryPolicy::SetBackoffMaxMs( UInt64 backoffMaxMs )
{GUCEF_TRACE;

    m_backoffMaxMs = backoffMaxMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobRetryPolicy::GetBackoffMaxMs( void ) const
{GUCEF_TRACE;

    return m_backoffMaxMs;
}

/*-------------------------------------------------------------------------*/

void
CJobRetryPolicy::SetJitterPercent( UInt32 jitterPercent )
{GUCEF_TRACE;

    m_jitterPercent = jitterPercent;
}

/*-------------------------------------------------------------------------*/

UInt32
CJobRetryPolicy::GetJitterPercent( void ) const
{GUCEF_TRACE;

    return m_jitterPercent;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
