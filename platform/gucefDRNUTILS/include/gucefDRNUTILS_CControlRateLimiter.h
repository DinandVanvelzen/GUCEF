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

#ifndef GUCEF_DRNUTILS_CCONTROLRATELIMITER_H
#define GUCEF_DRNUTILS_CCONTROLRATELIMITER_H

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
 *  Module-private: at most a number of commands per subject in any minute (a sliding window, design 8.3). Bounded:
 *  a subject keeps at most the maximum of time stamps, subjects without commands in the window are dropped, and at
 *  most MaxSubjects subjects are tracked (beyond, the subject with the oldest last command goes). Leaf lock.
 */
class CControlRateLimiter
{
    public:

    static const UInt32 MaxSubjects = 4096;
    static const UInt64 WindowMs = 60000;

    CControlRateLimiter( void );

    /** 0 = no limit */
    void SetMaxPerMinute( UInt32 maxPerMinute );

    /** True when the command may go (it is counted), false when the subject reached the limit (not counted) */
    bool Admit( const CORE::CString& subject, UInt64 nowMs );

    UInt32 GetSubjectCount( void ) const;

    private:

    typedef GUCEF::vector< UInt64 > TTimeVector;
    typedef GUCEF::map< CORE::CString, TTimeVector > TSubjectMap;

    void Prune( UInt64 nowMs );

    mutable MT::CMutex m_lock;
    UInt32 m_maxPerMinute;
    TSubjectMap m_subjects;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CCONTROLRATELIMITER_H ? */
