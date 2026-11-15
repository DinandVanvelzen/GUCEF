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

#ifndef GUCEF_DRNUTILS_CSCHEDULERTABLEPUBLISHER_H
#define GUCEF_DRNUTILS_CSCHEDULERTABLEPUBLISHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H
#include "gucefCORE_CJobSchedulerSnapshot.h"
#define GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H
#endif /* GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H ? */

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHER_H
#include "gucefDRN_CDRNTablePublisher.h"
#define GUCEF_DRN_CDRNTABLEPUBLISHER_H
#endif /* GUCEF_DRN_CDRNTABLEPUBLISHER_H ? */

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
 *  Module-private: the jobs, lanes and runs grids of a scheduler host in a DRN session (design 7.2). Publish() writes
 *  the rows of a model copy; rows that did not change are not sent (the publisher's equality check), removed rows go.
 *  The runs grid holds every active run and the last publishedRuns ended ones. Item keys of work sources are never
 *  part of a row (only the public item label). Called on the host's notification thread with no lock of the host held.
 */
class CSchedulerTablePublisher
{
    public:

    enum ETable { TABLE_JOBS = 0, TABLE_LANES = 1, TABLE_RUNS = 2, TABLE_COUNT = 3 };

    CSchedulerTablePublisher( void );
    ~CSchedulerTablePublisher();

    bool Start( const CORE::CString& sessionName, const CORE::CString& gridPrefix, const CORE::CString& hostName, CORE::CString& error );
    void Stop( void );
    bool IsStarted( void ) const;

    bool Publish( const CORE::CJobSchedulerSnapshot& model, UInt32 publishedRuns );

    const CORE::CString& GetGridName( ETable table ) const;
    const CORE::CString& GetSessionName( void ) const;

    private:

    CSchedulerTablePublisher( const CSchedulerTablePublisher& src );
    CSchedulerTablePublisher& operator=( const CSchedulerTablePublisher& src );

    CORE::CString m_sessionName;
    CORE::CString m_gridNames[ TABLE_COUNT ];
    DRN::CDRNDataGrid* m_grids[ TABLE_COUNT ];
    DRN::CDRNTablePublisher* m_publishers[ TABLE_COUNT ];
    CORE::CString::StringVector m_throttleColumns;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERTABLEPUBLISHER_H ? */
