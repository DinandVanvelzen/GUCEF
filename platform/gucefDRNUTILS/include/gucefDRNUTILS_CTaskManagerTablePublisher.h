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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERTABLEPUBLISHER_H
#define GUCEF_DRNUTILS_CTASKMANAGERTABLEPUBLISHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHER_H
#include "gucefDRN_CDRNTablePublisher.h"
#define GUCEF_DRN_CDRNTABLEPUBLISHER_H
#endif /* GUCEF_DRN_CDRNTABLEPUBLISHER_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H
#include "gucefDRNUTILS_CTaskManagerSample.h"
#define GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H ? */

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
 *  Module-private: the pools, threads, tasks and scheduledjobs grids of a task manager host in a DRN session
 *  (design 7.2, 7.3), written from samples (so only while the host is in viewing mode) and emptied when viewing ends.
 *  Rows that did not change are not sent. Called on the host's notification thread with no lock of the host held.
 */
class CTaskManagerTablePublisher
{
    public:

    enum ETable { TABLE_POOLS = 0, TABLE_THREADS = 1, TABLE_TASKS = 2, TABLE_SCHEDULEDJOBS = 3, TABLE_COUNT = 4 };

    CTaskManagerTablePublisher( void );
    ~CTaskManagerTablePublisher();

    bool Start( const CORE::CString& sessionName, const CORE::CString& gridPrefix, const CORE::CString& hostName, CORE::CString& error );
    void Stop( void );
    bool IsStarted( void ) const;

    /** The parts the sample holds (threads and tasks only when it has them) */
    bool Publish( const CTaskManagerSample& sample, const CORE::CString& hostName, UInt64 hostInstanceId );

    /** Every table empty (viewing ended) */
    bool Clear( void );

    const CORE::CString& GetGridName( ETable table ) const;

    private:

    CTaskManagerTablePublisher( const CTaskManagerTablePublisher& src );
    CTaskManagerTablePublisher& operator=( const CTaskManagerTablePublisher& src );

    bool PublishTable( ETable table, const DRN::CDRNTablePublisher::TKeyedRows& rows, const CTaskManagerSample& sample,
                       const CORE::CString& hostName, UInt64 hostInstanceId );

    CORE::CString m_sessionName;
    CORE::CString m_gridNames[ TABLE_COUNT ];
    DRN::CDRNDataGrid* m_grids[ TABLE_COUNT ];
    DRN::CDRNTablePublisher* m_publishers[ TABLE_COUNT ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CTASKMANAGERTABLEPUBLISHER_H ? */
