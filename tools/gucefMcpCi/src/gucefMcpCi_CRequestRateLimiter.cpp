/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CRequestRateLimiter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRequestRateLimiter::CBucket::CBucket( void )
    : level( 0.0 )
    , updatedInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRequestRateLimiter::CRequestRateLimiter( void )
    : m_perMinute( 600 )
    , m_burst( 120 )
    , m_buckets()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRequestRateLimiter::SetLimits( UInt32 perMinute, UInt32 burst )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_perMinute = perMinute;
    m_burst = burst > 0 ? burst : 1;
    m_buckets.clear();
}

/*-------------------------------------------------------------------------*/

void
CRequestRateLimiter::Prune( UInt64 nowInMs )
{GUCEF_TRACE;

    // a bucket that refilled completely is the same as no bucket
    const Float64 perMs = (Float64) m_perMinute / 60000.0;
    TBucketMap::iterator i = m_buckets.begin();
    while ( i != m_buckets.end() )
    {
        const CBucket& bucket = (*i).second;
        const Float64 refilled = bucket.level + ( nowInMs > bucket.updatedInMs ? (Float64) ( nowInMs - bucket.updatedInMs ) * perMs : 0.0 );
        if ( refilled >= (Float64) m_burst )
            m_buckets.erase( i++ );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRequestRateLimiter::Allow( const CORE::CString& key, UInt64 nowInMs, UInt32& retryAfterSecs )
{GUCEF_TRACE;

    retryAfterSecs = 0;
    MT::CScopeMutex lock( m_lock );
    if ( 0 == m_perMinute )
        return true;

    TBucketMap::iterator i = m_buckets.find( key );
    if ( i == m_buckets.end() )
    {
        if ( m_buckets.size() >= MaxKeys )
            Prune( nowInMs );
        if ( m_buckets.size() >= MaxKeys )
        {
            retryAfterSecs = 1;
            return false;
        }
        CBucket bucket;
        bucket.level = (Float64) m_burst;
        bucket.updatedInMs = nowInMs;
        i = m_buckets.insert( TBucketMap::value_type( key, bucket ) ).first;
    }

    CBucket& bucket = (*i).second;
    const Float64 perMs = (Float64) m_perMinute / 60000.0;
    if ( nowInMs > bucket.updatedInMs )
    {
        bucket.level += (Float64) ( nowInMs - bucket.updatedInMs ) * perMs;
        if ( bucket.level > (Float64) m_burst )
            bucket.level = (Float64) m_burst;
        bucket.updatedInMs = nowInMs;
    }
    if ( bucket.level >= 1.0 )
    {
        bucket.level -= 1.0;
        return true;
    }
    const Float64 waitMs = ( 1.0 - bucket.level ) / perMs;
    retryAfterSecs = (UInt32) ( waitMs / 1000.0 ) + 1;
    return false;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
