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

#ifndef GUCEF_DRNUTILS_CDRNUTILSTABLEBINDER_H
#define GUCEF_DRNUTILS_CDRNUTILSTABLEBINDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERTABLESET_H
#include "gucefDRNUTILS_CSchedulerTableSet.h"
#define GUCEF_DRNUTILS_CSCHEDULERTABLESET_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERTABLESET_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERTABLESET_H
#include "gucefDRNUTILS_CTaskManagerTableSet.h"
#define GUCEF_DRNUTILS_CTASKMANAGERTABLESET_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERTABLESET_H ? */

#ifndef GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#include "gucefDRNUTILS_CISchedulerSource.h"
#define GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_CISCHEDULERSOURCE_H ? */

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
 *  Fills table sets for forms and tools: from the grids a host publishes in a DRN session (replicas, through the drn
 *  database client with format "published"), or locally from a scheduler source without DRN (CSchedulerSnapshotTable,
 *  raising RowsChangedEvent from the source's changes while observed).
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDrnUtilsTableBinder
{
    public:

    /** "<prefix>/<tableName>"; an empty prefix is "gucef.drnutils/<hostName>" */
    static CORE::CString GetGridName( const CORE::CString& gridPrefix, const CORE::CString& hostName, const CORE::CString& tableName );

    static bool OpenSchedulerTables( const CORE::CString& drnSessionName ,
                                     const CORE::CString& gridPrefix     ,
                                     CSchedulerTableSet& tables          ,
                                     CORE::CString& error                );

    static bool OpenLocalSchedulerTables( CISchedulerSource& source                         ,
                                          const CORE::PulseGeneratorPtr& eventPulseGenerator ,
                                          CSchedulerTableSet& tables                        ,
                                          CORE::CString& error                              );

    static bool OpenTaskManagerTables( const CORE::CString& drnSessionName ,
                                       const CORE::CString& gridPrefix     ,
                                       CTaskManagerTableSet& tables        ,
                                       CORE::CString& error                );

    private:

    CDrnUtilsTableBinder( void );       /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSTABLEBINDER_H ? */
