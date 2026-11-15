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

#ifndef GUCEF_DRNUTILS_CSCHEDULERTABLESET_H
#define GUCEF_DRNUTILS_CSCHEDULERTABLESET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

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
 *  The tables of one scheduler as generic data tables, whatever they come from: DRN replicas of the published grids,
 *  local tables on the publisher's own grids, CSchedulerSnapshotTable over a source, or CMemoryDataTable in tests.
 *  Forms bind to these, never to DRN objects (CDrnUtilsTableBinder fills them).
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CSchedulerTableSet
{
    public:

    CSchedulerTableSet( void );

    void SetJobs( const CORE::CIDataTablePtr& jobs );
    const CORE::CIDataTablePtr& GetJobs( void ) const;

    void SetLanes( const CORE::CIDataTablePtr& lanes );
    const CORE::CIDataTablePtr& GetLanes( void ) const;

    void SetRuns( const CORE::CIDataTablePtr& runs );
    const CORE::CIDataTablePtr& GetRuns( void ) const;
    /** Whether all tables are set */
    bool IsComplete( void ) const;

    void Clear( void );

    private:

    CORE::CIDataTablePtr m_jobs;
    CORE::CIDataTablePtr m_lanes;
    CORE::CIDataTablePtr m_runs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CSCHEDULERTABLESET_H ? */
