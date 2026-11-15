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

#ifndef GUCEF_CORE_CJOBRETRYPOLICY_H
#define GUCEF_CORE_CJOBRETRYPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Retry of a failed run of a job: attempts, doubling backoff and jitter. Part of a CJobDefinition
 */
class GUCEF_CORE_PUBLIC_CPP CJobRetryPolicy
{
    public:

    CJobRetryPolicy( void );

    /** Attempts of one occurrence including the first; 1 = no retry */
    void SetMaxAttempts( UInt32 maxAttempts );
    UInt32 GetMaxAttempts( void ) const;

    /** Wait before the first retry */
    void SetBackoffInitialMs( UInt64 backoffInitialMs );
    UInt64 GetBackoffInitialMs( void ) const;

    /** Upper bound of the doubling backoff */
    void SetBackoffMaxMs( UInt64 backoffMaxMs );
    UInt64 GetBackoffMaxMs( void ) const;

    /** Random spread of each backoff, 0..50 percent */
    void SetJitterPercent( UInt32 jitterPercent );
    UInt32 GetJitterPercent( void ) const;
    bool operator==( const CJobRetryPolicy& other ) const;

    private:

    UInt32 m_maxAttempts;
    UInt64 m_backoffInitialMs;
    UInt64 m_backoffMaxMs;
    UInt32 m_jitterPercent;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBRETRYPOLICY_H ? */
