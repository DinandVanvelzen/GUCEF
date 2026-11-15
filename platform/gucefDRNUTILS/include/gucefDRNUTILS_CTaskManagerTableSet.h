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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERTABLESET_H
#define GUCEF_DRNUTILS_CTASKMANAGERTABLESET_H

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
 *  The tables of one task manager view as generic data tables (pools, threads, tasks and the scheduled jobs overview)
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CTaskManagerTableSet
{
    public:

    CTaskManagerTableSet( void );

    void SetPools( const CORE::CIDataTablePtr& pools );
    const CORE::CIDataTablePtr& GetPools( void ) const;

    void SetThreads( const CORE::CIDataTablePtr& threads );
    const CORE::CIDataTablePtr& GetThreads( void ) const;

    void SetTasks( const CORE::CIDataTablePtr& tasks );
    const CORE::CIDataTablePtr& GetTasks( void ) const;

    void SetScheduledJobs( const CORE::CIDataTablePtr& scheduledJobs );
    const CORE::CIDataTablePtr& GetScheduledJobs( void ) const;
    bool IsComplete( void ) const;

    void Clear( void );

    private:

    CORE::CIDataTablePtr m_pools;
    CORE::CIDataTablePtr m_threads;
    CORE::CIDataTablePtr m_tasks;
    CORE::CIDataTablePtr m_scheduledJobs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CTASKMANAGERTABLESET_H ? */
