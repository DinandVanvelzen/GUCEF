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

#ifndef GUCEF_DRNUTILS_CSCHEDULERCHANGEJOURNAL_H
#define GUCEF_DRNUTILS_CSCHEDULERCHANGEJOURNAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  Module-private: a bounded ring of the last change records of a scheduler host { revision, kind, key } without row
 *  copies (design 6.5): a Watch with afterRevision resumes from it when the journal still covers that revision.
 *  Not locked: the host guards it.
 */
class CSchedulerChangeJournal
{
    public:

    /** Kinds as the contract's ChangeKind numbers */
    enum EKind
    {
        KIND_JOB  = 1 ,
        KIND_LANE = 2 ,
        KIND_RUN  = 3
    };

    struct SRecord
    {
        UInt64 revision;
        Int32 kind;
        CORE::CString key;
    };
    typedef GUCEF::vector< SRecord > TRecordVector;

    explicit CSchedulerChangeJournal( UInt32 capacity = 4096 );

    /** Empties the journal: it covers everything after floorRevision from now on */
    void Reset( UInt32 capacity, UInt64 floorRevision );

    void Add( UInt64 revision, Int32 kind, const CORE::CString& key );

    /** False when the journal no longer covers afterRevision (a RESET and a snapshot are needed); else the records after it, oldest first */
    bool Collect( UInt64 afterRevision, TRecordVector& records ) const;

    UInt64 GetFloorRevision( void ) const;

    UInt32 GetCount( void ) const;

    private:

    TRecordVector m_ring;
    UInt32 m_capacity;
    UInt32 m_start;
    UInt32 m_count;
    UInt64 m_floor;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERCHANGEJOURNAL_H ? */
