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

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H
#include "gucefDRN_CDRNTablePublisherUpdateScope.h"
#define GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H
#endif /* GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H ? */

#ifndef GUCEF_DRNUTILS_CPOOLSTABLESCHEMA_H
#include "gucefDRNUTILS_CPoolsTableSchema.h"
#define GUCEF_DRNUTILS_CPOOLSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CPOOLSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CTHREADSTABLESCHEMA_H
#include "gucefDRNUTILS_CThreadsTableSchema.h"
#define GUCEF_DRNUTILS_CTHREADSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CTHREADSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CTASKSTABLESCHEMA_H
#include "gucefDRNUTILS_CTasksTableSchema.h"
#define GUCEF_DRNUTILS_CTASKSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CTASKSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULEDJOBSTABLESCHEMA_H
#include "gucefDRNUTILS_CScheduledJobsTableSchema.h"
#define GUCEF_DRNUTILS_CSCHEDULEDJOBSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CSCHEDULEDJOBSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSTABLEBINDER_H
#include "gucefDRNUTILS_CDrnUtilsTableBinder.h"
#define GUCEF_DRNUTILS_CDRNUTILSTABLEBINDER_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSTABLEBINDER_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSGRIDSUPPORT_H
#include "gucefDRNUTILS_CDrnUtilsGridSupport.h"
#define GUCEF_DRNUTILS_CDRNUTILSGRIDSUPPORT_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSGRIDSUPPORT_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERROWCODEC_H
#include "gucefDRNUTILS_CTaskManagerRowCodec.h"
#define GUCEF_DRNUTILS_CTASKMANAGERROWCODEC_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERROWCODEC_H ? */

#include "gucefDRNUTILS_CTaskManagerTablePublisher.h"

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

CTaskManagerTablePublisher::CTaskManagerTablePublisher( void )
    : m_sessionName()
{GUCEF_TRACE;

    for ( UInt32 t=0; t<TABLE_COUNT; ++t )
    {
        m_grids[ t ] = GUCEF_NULL;
        m_publishers[ t ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CTaskManagerTablePublisher::~CTaskManagerTablePublisher()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerTablePublisher::IsStarted( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_publishers[ TABLE_POOLS ];
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTaskManagerTablePublisher::GetGridName( ETable table ) const
{GUCEF_TRACE;

    return m_gridNames[ table ];
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerTablePublisher::Start( const CORE::CString& sessionName, const CORE::CString& gridPrefix, const CORE::CString& hostName, CORE::CString& error )
{GUCEF_TRACE;

    Stop();
    m_gridNames[ TABLE_POOLS ] = CDrnUtilsTableBinder::GetGridName( gridPrefix, hostName, CPoolsTableSchema::TableName );
    m_gridNames[ TABLE_THREADS ] = CDrnUtilsTableBinder::GetGridName( gridPrefix, hostName, CThreadsTableSchema::TableName );
    m_gridNames[ TABLE_TASKS ] = CDrnUtilsTableBinder::GetGridName( gridPrefix, hostName, CTasksTableSchema::TableName );
    m_gridNames[ TABLE_SCHEDULEDJOBS ] = CDrnUtilsTableBinder::GetGridName( gridPrefix, hostName, CScheduledJobsTableSchema::TableName );
    m_sessionName = sessionName;
    for ( UInt32 t=0; t<TABLE_COUNT; ++t )
    {
        if ( !CDrnUtilsGridSupport::CreatePublishedGrid( sessionName, m_gridNames[ t ], m_grids[ t ], m_publishers[ t ], error ) )
        {
            Stop();
            return false;
        }
    }
    CORE::CString::StringVector none;
    CORE::CDataTableSchema schema;
    DRN::CDRNPublishedTableSchema published;
    CPoolsTableSchema::GetSchema( schema );
    CDrnUtilsGridSupport::MakePublishedSchema( schema, 0, true, none, published );
    bool ok = m_publishers[ TABLE_POOLS ]->SetSchema( published );
    CThreadsTableSchema::GetSchema( schema );
    CDrnUtilsGridSupport::MakePublishedSchema( schema, 2, true, none, published );
    ok = m_publishers[ TABLE_THREADS ]->SetSchema( published ) && ok;
    CTasksTableSchema::GetSchema( schema );
    CDrnUtilsGridSupport::MakePublishedSchema( schema, 0, true, none, published );
    ok = m_publishers[ TABLE_TASKS ]->SetSchema( published ) && ok;
    CScheduledJobsTableSchema::GetSchema( schema );
    CDrnUtilsGridSupport::MakePublishedSchema( schema, 0, true, none, published );
    ok = m_publishers[ TABLE_SCHEDULEDJOBS ]->SetSchema( published ) && ok;
    if ( !ok )
    {
        error = "the schemas of the task manager grids could not be published";
        Stop();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerTablePublisher::Stop( void )
{GUCEF_TRACE;

    for ( UInt32 t=0; t<TABLE_COUNT; ++t )
        CDrnUtilsGridSupport::DestroyPublishedGrid( m_sessionName, m_grids[ t ], m_publishers[ t ] );
    m_sessionName.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerTablePublisher::PublishTable( ETable table, const DRN::CDRNTablePublisher::TKeyedRows& rows, const CTaskManagerSample& sample,
                                          const CORE::CString& hostName, UInt64 hostInstanceId )
{GUCEF_TRACE;

    DRN::CDRNTablePublisher& publisher = *m_publishers[ table ];
    publisher.SetScalar( "hostName", CORE::CVariant( hostName ) );
    publisher.SetScalar( "hostInstanceId", CORE::CVariant( hostInstanceId ) );
    publisher.SetScalar( "truncated", CORE::CVariant( sample.truncated ) );
    DRN::CDRNTablePublisherUpdateScope update( publisher );
    return update.IsOpen() && publisher.PublishRows( rows );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerTablePublisher::Publish( const CTaskManagerSample& sample, const CORE::CString& hostName, UInt64 hostInstanceId )
{GUCEF_TRACE;

    if ( !IsStarted() )
        return false;
    bool ok = true;
    DRN::CDRNTablePublisher::TKeyedRows rows;
    for ( UInt32 i=0; i<sample.pools.size(); ++i )
    {
        DRN::CDRNTablePublisher::TKeyedRow row;
        row.first = CORE::CVariant( sample.pools[ i ].name );
        CTaskManagerRowCodec::PoolToCells( sample.pools[ i ], row.second );
        rows.push_back( row );
    }
    ok = PublishTable( TABLE_POOLS, rows, sample, hostName, hostInstanceId ) && ok;
    if ( sample.hasThreads )
    {
        rows.clear();
        for ( UInt32 i=0; i<sample.threads.size(); ++i )
        {
            DRN::CDRNTablePublisher::TKeyedRow row;
            row.first = CORE::CVariant( sample.threads[ i ].systemThreadId );
            CTaskManagerRowCodec::ThreadToCells( sample.threads[ i ], row.second );
            rows.push_back( row );
        }
        ok = PublishTable( TABLE_THREADS, rows, sample, hostName, hostInstanceId ) && ok;
    }
    if ( sample.hasTasks )
    {
        rows.clear();
        for ( UInt32 i=0; i<sample.tasks.size(); ++i )
        {
            DRN::CDRNTablePublisher::TKeyedRow row;
            row.first = CORE::CVariant( sample.tasks[ i ].taskId );
            CTaskManagerRowCodec::TaskToCells( sample.tasks[ i ], row.second );
            rows.push_back( row );
        }
        ok = PublishTable( TABLE_TASKS, rows, sample, hostName, hostInstanceId ) && ok;
    }
    if ( sample.hasScheduledJobs )
    {
        rows.clear();
        for ( UInt32 i=0; i<sample.scheduledJobs.size(); ++i )
        {
            DRN::CDRNTablePublisher::TKeyedRow row;
            row.first = CORE::CVariant( sample.scheduledJobs[ i ].jobKey );
            CTaskManagerRowCodec::ScheduledJobToCells( sample.scheduledJobs[ i ], row.second );
            rows.push_back( row );
        }
        ok = PublishTable( TABLE_SCHEDULEDJOBS, rows, sample, hostName, hostInstanceId ) && ok;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerTablePublisher::Clear( void )
{GUCEF_TRACE;

    if ( !IsStarted() )
        return false;
    bool ok = true;
    DRN::CDRNTablePublisher::TKeyedRows none;
    for ( UInt32 t=0; t<TABLE_COUNT; ++t )
    {
        DRN::CDRNTablePublisherUpdateScope update( *m_publishers[ t ] );
        ok = update.IsOpen() && m_publishers[ t ]->PublishRows( none ) && ok;
    }
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
