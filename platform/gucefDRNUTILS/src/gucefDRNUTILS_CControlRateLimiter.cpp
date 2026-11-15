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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefDRNUTILS_CControlRateLimiter.h"

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

CControlRateLimiter::CControlRateLimiter( void )
    : m_lock()
    , m_maxPerMinute( 60 )
    , m_subjects()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CControlRateLimiter::SetMaxPerMinute( UInt32 maxPerMinute )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxPerMinute = maxPerMinute;
}

/*-------------------------------------------------------------------------*/

void
CControlRateLimiter::Prune( UInt64 nowMs )
{GUCEF_TRACE;

    UInt64 oldestAllowed = nowMs > WindowMs ? nowMs - WindowMs : 0;
    TSubjectMap::iterator i = m_subjects.begin();
    while ( i != m_subjects.end() )
    {
        TTimeVector& times = (*i).second;
        UInt32 drop = 0;
        while ( drop < times.size() && times[ drop ] <= oldestAllowed && nowMs >= WindowMs )
            ++drop;
        if ( drop > 0 )
            times.erase( times.begin(), times.begin() + drop );
        if ( times.empty() )
            GUCEF_ERASE_AND_ADVANCE( m_subjects, i );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CControlRateLimiter::Admit( const CORE::CString& subject, UInt64 nowMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( 0 == m_maxPerMinute )
        return true;
    Prune( nowMs );
    TSubjectMap::iterator found = m_subjects.find( subject );
    if ( found != m_subjects.end() && (*found).second.size() >= m_maxPerMinute )
        return false;
    if ( found == m_subjects.end() && m_subjects.size() >= MaxSubjects )
    {
        // the subject whose last command is the oldest goes
        TSubjectMap::iterator oldest = m_subjects.begin();
        TSubjectMap::iterator i = m_subjects.begin();
        while ( i != m_subjects.end() )
        {
            if ( (*i).second.back() < (*oldest).second.back() )
                oldest = i;
            ++i;
        }
        m_subjects.erase( oldest );
    }
    m_subjects[ subject ].push_back( nowMs );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CControlRateLimiter::GetSubjectCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_subjects.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
