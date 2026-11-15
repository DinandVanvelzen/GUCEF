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

#ifndef GUCEF_CORE_CJOBRUNHISTORY_H
#define GUCEF_CORE_CJOBRUNHISTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CJOBRUNRECORD_H
#include "gucefCORE_CJobRunRecord.h"
#define GUCEF_CORE_CJOBRUNRECORD_H
#endif /* GUCEF_CORE_CJOBRUNRECORD_H ? */

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
 *  A bounded history of ended runs of a job scheduler: a ring that keeps the newest runs, at most the maximum given
 *  (0 = no maximum). Once full, a new run takes the slot of the oldest one, so the history does not grow after it is
 *  warm. Not thread safe: the scheduler uses it under its own lock.
 */
class GUCEF_CORE_PUBLIC_CPP CJobRunHistory
{
    public:

    explicit CJobRunHistory( UInt32 maximum = 0 );

    /** Keeps the newest runs when the maximum shrinks */
    void SetMaximum( UInt32 maximum );
    UInt32 GetMaximum( void ) const;

    void Add( const SJobRunRecord& run );

    bool Find( UInt64 runId, SJobRunRecord& run ) const;

    UInt32 GetCount( void ) const;

    /** Appends the runs, newest first, whose revision is above changedAfterRevision; at most maxRuns (0 = all) */
    void GetNewestFirst( TJobRunRecordVector& runs, UInt64 changedAfterRevision = 0, UInt32 maxRuns = 0 ) const;

    void Clear( void );

    private:

    const SJobRunRecord& GetByAge( UInt32 newestFirstIndex ) const;

    private:

    TJobRunRecordVector m_ring;
    UInt32 m_next;          /**< slot of the next run once the ring is full */
    UInt32 m_count;
    UInt32 m_maximum;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBRUNHISTORY_H ? */
