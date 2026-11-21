/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_LOGINTHROTTLE_H
#define GUCEF_LABLINK_LOGINTHROTTLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Exponential back-off of failed logins, per key (an address, or a user name). The first freeFailures failures of a
 *  key cost nothing, each one after that doubles the wait before the next attempt, from baseDelay up to maxDelay: the
 *  wait is bounded, so there is no lockout that lasts. A key without failures for forgetAfter is forgotten. The table
 *  holds at most maxEntries keys; a new key takes the place of the one whose last failure is oldest. Times are passed
 *  in, monotonic milliseconds. Used on one thread, that of the website.
 */
class CLoginThrottle
{
    public:

    CLoginThrottle( GUCEF::UInt32 maxEntries, GUCEF::UInt32 freeFailures, GUCEF::UInt32 baseDelayInMs, GUCEF::UInt32 maxDelayInMs,
                    GUCEF::UInt32 forgetAfterInMs );

    /** How long the key has to wait before its next attempt; 0: it may try now */
    GUCEF::UInt32 GetWaitInMs( const GUCEF::CORE::CString& key, GUCEF::UInt64 nowInMs ) const;

    void OnFailure( const GUCEF::CORE::CString& key, GUCEF::UInt64 nowInMs );

    void OnSuccess( const GUCEF::CORE::CString& key );

    /** Forgets the keys that are quiet long enough */
    void Sweep( GUCEF::UInt64 nowInMs );

    GUCEF::UInt32 GetCount( void ) const;

    GUCEF::UInt32 GetFailures( const GUCEF::CORE::CString& key ) const;

    private:

    struct SEntry
    {
        GUCEF::UInt32 failures;
        GUCEF::UInt64 lastFailureInMs;
        GUCEF::UInt64 nextAttemptInMs;

        SEntry( void ) : failures( 0 ), lastFailureInMs( 0 ), nextAttemptInMs( 0 ) {}
    };
    typedef GUCEF::map< GUCEF::CORE::CString, SEntry > TEntryMap;

    TEntryMap m_entries;
    GUCEF::UInt32 m_maxEntries;
    GUCEF::UInt32 m_freeFailures;
    GUCEF::UInt32 m_baseDelayInMs;
    GUCEF::UInt32 m_maxDelayInMs;
    GUCEF::UInt32 m_forgetAfterInMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_LOGINTHROTTLE_H ? */
