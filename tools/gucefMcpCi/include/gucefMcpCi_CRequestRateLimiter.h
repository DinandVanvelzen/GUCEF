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

#ifndef GUCEF_MCPCI_CREQUESTRATELIMITER_H
#define GUCEF_MCPCI_CREQUESTRATELIMITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A request rate per key (the machine of a token): a bucket of 'burst' requests per key that fills up at
 *  'perMinute' requests a minute. Keys that have been full for a while are forgotten, and at most MaxKeys are kept,
 *  so the memory stays bounded whatever the clients do.
 *
 *  Thread-safe.
 */
class CRequestRateLimiter
{
    public:

    static const GUCEF::UInt32 MaxKeys = 4096;

    CRequestRateLimiter( void );

    /** @param perMinute    0 for no limit */
    void SetLimits( GUCEF::UInt32 perMinute, GUCEF::UInt32 burst );

    /**
     *  Takes one request of the bucket of the key.
     *  @param retryAfterSecs   when refused: after how many seconds there is room again
     *  @return whether the request may be served
     */
    bool Allow( const GUCEF::CORE::CString& key, GUCEF::UInt64 nowInMs, GUCEF::UInt32& retryAfterSecs );

    private:

    class CBucket
    {
        public:

        GUCEF::Float64 level;
        GUCEF::UInt64 updatedInMs;

        CBucket( void );
    };
    typedef GUCEF::map< GUCEF::CORE::CString, CBucket > TBucketMap;

    void Prune( GUCEF::UInt64 nowInMs );

    GUCEF::UInt32 m_perMinute;
    GUCEF::UInt32 m_burst;
    TBucketMap m_buckets;
    GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CREQUESTRATELIMITER_H ? */
