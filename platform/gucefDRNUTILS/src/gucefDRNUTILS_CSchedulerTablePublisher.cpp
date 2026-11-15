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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H
#include "gucefDRN_CDRNTablePublisherUpdateScope.h"
#define GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H
#endif /* GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H ? */

#ifndef GUCEF_DRNUTILS_CJOBSTABLESCHEMA_H
#include "gucefDRNUTILS_CJobsTableSchema.h"
#define GUCEF_DRNUTILS_CJOBSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CJOBSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CLANESTABLESCHEMA_H
#include "gucefDRNUTILS_CLanesTableSchema.h"
#define GUCEF_DRNUTILS_CLANESTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CLANESTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CRUNSTABLESCHEMA_H
#include "gucefDRNUTILS_CRunsTableSchema.h"
#define GUCEF_DRNUTILS_CRUNSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CRUNSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSTABLEBINDER_H
#include "gucefDRNUTILS_CDrnUtilsTableBinder.h"
#define GUCEF_DRNUTILS_CDRNUTILSTABLEBINDER_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSTABLEBINDER_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSGRIDSUPPORT_H
#include "gucefDRNUTILS_CDrnUtilsGridSupport.h"
#define GUCEF_DRNUTILS_CDRNUTILSGRIDSUPPORT_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSGRIDSUPPORT_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#include "gucefDRNUTILS_CSchedulerRowCodec.h"
#define GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H ? */

#include "gucefDRNUTILS_CSchedulerTablePublisher.h"

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

CSchedulerTablePublisher::CSchedulerTablePublisher( void )
    : m_sessionName()
    , m_throttleColumns()
{GUCEF_TRACE;

    for ( UInt32 t=0; t<TABLE_COUNT; ++t )
    {
        m_grids[ t ] = GUCEF_NULL;
        m_publishers[ t ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CSchedulerTablePublisher::~CSchedulerTablePublisher()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerTablePublisher::IsStarted( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_publishers[ TABLE_JOBS ];
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSchedulerTablePublisher::GetGridName( ETable table ) const
{GUCEF_TRACE;

    return m_gridNames[ table ];
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSchedulerTablePublisher::GetSessionName( void ) const
{GUCEF_TRACE;

    return m_sessionName;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerTablePublisher::Start( const CORE::CString& sessionName, const CORE::CString& gridPrefix, const CORE::CString& hostName, CORE::CString& error )
{GUCEF_TRACE;

    Stop();
    m_gridNames[ TABLE_JOBS ] = CDrnUtilsTableBinder::GetGridName( gridPrefix, hostName, CJobsTableSchema::TableName );
    m_gridNames[ TABLE_LANES ] = CDrnUtilsTableBinder::GetGridName( gridPrefix, hostName, CLanesTableSchema::TableName );
    m_gridNames[ TABLE_RUNS ] = CDrnUtilsTableBinder::GetGridName( gridPrefix, hostName, CRunsTableSchema::TableName );
    for ( UInt32 t=0; t<TABLE_COUNT; ++t )
    {
        if ( !CDrnUtilsGridSupport::CreatePublishedGrid( sessionName, m_gridNames[ t ], m_grids[ t ], m_publishers[ t ], error ) )
        {
            m_sessionName = sessionName;
            Stop();
            return false;
        }
    }
    m_sessionName = sessionName;

    CORE::CDataTableSchema schema;
    DRN::CDRNPublishedTableSchema published;
    CORE::CString::StringVector none;
    CJobsTableSchema::GetSchema( schema );
    CDrnUtilsGridSupport::MakePublishedSchema( schema, 0, true, none, published );
    bool ok = m_publishers[ TABLE_JOBS ]->SetSchema( published );
    CLanesTableSchema::GetSchema( schema );
    CDrnUtilsGridSupport::MakePublishedSchema( schema, 0, true, m_throttleColumns, published );
    ok = m_publishers[ TABLE_LANES ]->SetSchema( published ) && ok;
    CRunsTableSchema::GetSchema( schema );
    CDrnUtilsGridSupport::MakePublishedSchema( schema, 0, false, none, published );
    ok = m_publishers[ TABLE_RUNS ]->SetSchema( published ) && ok;
    if ( !ok )
    {
        error = "the schemas of the scheduler grids could not be published";
        Stop();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerTablePublisher::Stop( void )
{GUCEF_TRACE;

    for ( UInt32 t=0; t<TABLE_COUNT; ++t )
        CDrnUtilsGridSupport::DestroyPublishedGrid( m_sessionName, m_grids[ t ], m_publishers[ t ] );
    m_sessionName.Clear();
    m_throttleColumns.clear();
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerTablePublisher::Publish( const CORE::CJobSchedulerSnapshot& model, UInt32 publishedRuns )
{GUCEF_TRACE;

    if ( !IsStarted() )
        return false;

    // every row is built before an update opens: inside it only encoding and writing happen (publisher design 5.1)
    DRN::CDRNTablePublisher::TKeyedRows jobRows;
    const CORE::TJobStatusVector& jobs = model.GetJobs();
    for ( UInt32 i=0; i<jobs.size(); ++i )
    {
        DRN::CDRNTablePublisher::TKeyedRow row;
        row.first = CORE::CVariant( jobs[ i ].jobId );
        CSchedulerRowCodec::JobToCells( jobs[ i ], row.second );
        jobRows.push_back( row );
    }

    // the throttle status of the lanes as hidden columns: a new set of names is a new schema
    const CORE::TJobLaneStatusVector& lanes = model.GetLanes();
    GUCEF::vector< CORE::CString::StringMap > throttles( lanes.size() );
    CORE::CString::StringSet throttleNames;
    for ( UInt32 i=0; i<lanes.size(); ++i )
    {
        CSchedulerRowCodec::FlattenThrottle( lanes[ i ].throttleStatus, throttles[ i ] );
        CORE::CString::StringMap::const_iterator n = throttles[ i ].begin();
        while ( n != throttles[ i ].end() && throttleNames.size() < CSchedulerRowCodec::MaxThrottleColumns )
        {
            throttleNames.insert( (*n).first );
            ++n;
        }
    }
    CORE::CString::StringVector throttleColumns( throttleNames.begin(), throttleNames.end() );
    DRN::CDRNTablePublisher::TKeyedRows laneRows;
    for ( UInt32 i=0; i<lanes.size(); ++i )
    {
        DRN::CDRNTablePublisher::TKeyedRow row;
        row.first = CORE::CVariant( lanes[ i ].laneName );
        CSchedulerRowCodec::LaneToCells( lanes[ i ], row.second );
        for ( UInt32 c=0; c<throttleColumns.size(); ++c )
        {
            CORE::CString::StringMap::const_iterator v = throttles[ i ].find( throttleColumns[ c ] );
            row.second.push_back( v == throttles[ i ].end() ? CORE::CVariant() : CORE::CVariant( (*v).second ) );
        }
        laneRows.push_back( row );
    }

    // every active run and the last publishedRuns ended ones (the model lists the active runs first, then the ended ones, newest first)
    DRN::CDRNTablePublisher::TKeyedRows runRows;
    const CORE::TJobRunRecordVector& runs = model.GetRuns();
    UInt32 endedRuns = 0;
    for ( UInt32 i=0; i<runs.size(); ++i )
    {
        if ( CORE::JobRunStateIsAnEndState( runs[ i ].state ) )
        {
            if ( endedRuns >= publishedRuns )
                continue;
            ++endedRuns;
        }
        DRN::CDRNTablePublisher::TKeyedRow row;
        row.first = CORE::CVariant( runs[ i ].runId );
        CSchedulerRowCodec::RunToCells( runs[ i ], row.second );
        runRows.push_back( row );
    }

    bool ok = true;
    if ( throttleColumns != m_throttleColumns )
    {
        m_throttleColumns = throttleColumns;
        CORE::CDataTableSchema schema;
        DRN::CDRNPublishedTableSchema published;
        CLanesTableSchema::GetSchema( schema );
        CDrnUtilsGridSupport::MakePublishedSchema( schema, 0, true, m_throttleColumns, published );
        ok = m_publishers[ TABLE_LANES ]->SetSchema( published );
    }

    DRN::CDRNTablePublisher::TKeyedRows* rowSets[ TABLE_COUNT ] = { &jobRows, &laneRows, &runRows };
    for ( UInt32 t=0; t<TABLE_COUNT; ++t )
    {
        DRN::CDRNTablePublisher& publisher = *m_publishers[ t ];
        publisher.SetScalar( "hostName", CORE::CVariant( model.GetHostName() ) );
        publisher.SetScalar( "hostInstanceId", CORE::CVariant( model.GetHostInstanceId() ) );
        publisher.SetScalar( "revision", CORE::CVariant( model.GetRevision() ) );
        publisher.SetScalar( "truncated", CORE::CVariant( TABLE_RUNS == t && endedRuns >= publishedRuns && publishedRuns > 0 ) );
        publisher.SetSourceVersion( model.GetRevision() );
        DRN::CDRNTablePublisherUpdateScope update( publisher );
        ok = update.IsOpen() && publisher.PublishRows( *rowSets[ t ] ) && ok;
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
