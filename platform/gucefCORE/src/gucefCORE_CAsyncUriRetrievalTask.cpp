/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#include "gucefCORE_CAsyncUriRetrievalTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CAsyncUriRetrievalTaskConsumer::TaskType = "GUCEF::CORE::CAsyncUriRetrievalTask";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAsyncUriRetrievalReporter::CAsyncUriRetrievalReporter( void )
    : CTSGNotifier()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncUriRetrievalReporter::~CAsyncUriRetrievalReporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAsyncUriRetrievalReporter::Report( const CEvent& eventId, CICloneable* eventData )
{GUCEF_TRACE;

    return NotifyObservers( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

const CString&
CAsyncUriRetrievalReporter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CAsyncUriRetrievalReporter";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CAsyncUriRetrievalTaskData::CAsyncUriRetrievalTaskData( void )
    : CICloneable()
    , retrievalId( 0 )
    , uri()
    , isPartial( false )
    , byteOffset( 0 )
    , bytesToGet( -1 )
    , destination( GUCEF_NULL )
    , destinationBuffer( GUCEF_NULL )
    , queryTotalSize( false )
    , reportIntervalInBytes( 0 )
    , reporter()
    , success( false )
    , errorText()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncUriRetrievalTaskData::CAsyncUriRetrievalTaskData( const CAsyncUriRetrievalTaskData& src )
    : CICloneable( src )
    , retrievalId( src.retrievalId )
    , uri( src.uri )
    , isPartial( src.isPartial )
    , byteOffset( src.byteOffset )
    , bytesToGet( src.bytesToGet )
    , destination( src.destination )
    , destinationBuffer( src.destinationBuffer )
    , queryTotalSize( src.queryTotalSize )
    , reportIntervalInBytes( src.reportIntervalInBytes )
    , reporter( src.reporter )
    , success( src.success )
    , errorText( src.errorText )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncUriRetrievalTaskData::~CAsyncUriRetrievalTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CICloneable*
CAsyncUriRetrievalTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAsyncUriRetrievalTaskData( *this );
}

/*-------------------------------------------------------------------------*/

CAsyncUriRetrievalTaskConsumer::CAsyncUriRetrievalTaskConsumer( void )
    : CTaskConsumer()
    , CIIOProgressObserver()
    , m_taskData( GUCEF_NULL )
    , m_task( GUCEF_NULL )
    , m_totalBytes( -1 )
    , m_bytesTransferred( 0 )
    , m_lastReportedBytes( 0 )
    , m_stoppedEarly( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncUriRetrievalTaskConsumer::~CAsyncUriRetrievalTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CAsyncUriRetrievalTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

void
CAsyncUriRetrievalTaskConsumer::Report( const CEvent& eventId       ,
                                        UInt64 bytesTransferred     ,
                                        Int64 totalBytes            ,
                                        const CString& errorText    )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_taskData || m_taskData->reporter.IsNULL() )
        return;

    CAsyncUriRetrievalEventData eventData;
    eventData.retrievalId = m_taskData->retrievalId;
    eventData.uri = m_taskData->uri;
    eventData.bytesTransferred = bytesTransferred;
    eventData.totalBytes = totalBytes;
    eventData.errorText = errorText;
    m_taskData->reporter->Report( eventId, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncUriRetrievalTaskConsumer::OnTaskStart( CTaskPtr task )
{GUCEF_TRACE;

    m_taskData = static_cast< CAsyncUriRetrievalTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == m_taskData )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AsyncUriRetrievalTask: Task without retrieval data" );
        return false;
    }
    m_task = task.GetPointerAlways();
    m_totalBytes = -1;
    m_bytesTransferred = 0;
    m_lastReportedBytes = 0;
    m_stoppedEarly = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncUriRetrievalTaskConsumer::IsStopRequested( void ) const
{GUCEF_TRACE;

    return ( GUCEF_NULL != m_task && m_task->IsCancellationRequested() ) || IsDeactivationRequested();
}

/*-------------------------------------------------------------------------*/

bool
CAsyncUriRetrievalTaskConsumer::OnIOProgress( UInt64 totalBytesRead    ,
                                              UInt64 totalBytesWritten )
{GUCEF_TRACE;

    if ( IsStopRequested() )
    {
        m_stoppedEarly = true;
        return false;
    }

    // A retrieval delivers to the destination, which is what progress is about
    m_bytesTransferred = totalBytesWritten;
    m_lastReportedBytes = totalBytesWritten;
    Report( CAsyncUriResourceRetriever::RetrievalProgressEvent, m_bytesTransferred, m_totalBytes, CString() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncUriRetrievalTaskConsumer::OnTaskCycle( CTaskPtr task )
{GUCEF_TRACE;

    // Everything is done in this one cycle, which ends the task
    if ( GUCEF_NULL == m_taskData )
        return true;

    if ( IsStopRequested() )
    {
        m_stoppedEarly = true;
        return true;
    }

    CUriResourceAccessorFactory::TProductPtr accessor = CCoreGlobal::Instance()->GetUriResourceAccessorFactory().CreateAccessor( m_taskData->uri );
    if ( accessor.IsNULL() )
    {
        m_taskData->errorText = "No resource accessor available for scheme \"" + m_taskData->uri.GetScheme() + "\"";
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AsyncUriRetrievalTask: " + m_taskData->errorText );
        return true;
    }

    Report( CAsyncUriResourceRetriever::RetrievalStartedEvent, 0, -1, CString() );

    if ( m_taskData->queryTotalSize )
    {
        CResourceMetaData metaData;
        if ( accessor->GetResourceMetaData( m_taskData->uri, metaData ) && metaData.hasResourceSizeInBytes )
        {
            m_totalBytes = (Int64) metaData.resourceSizeInBytes;
            if ( m_taskData->isPartial )
            {
                // Only what is asked for will be delivered
                Int64 available = m_totalBytes - (Int64) m_taskData->byteOffset;
                if ( available < 0 )
                    available = 0;
                m_totalBytes = ( m_taskData->bytesToGet >= 0 && m_taskData->bytesToGet < available ) ? m_taskData->bytesToGet : available;
            }
        }
    }

    // The destination is wrapped to get progress and the ability to stop the blocking retrieval midway
    CDynamicBufferAccess bufferAccess;
    CIOAccess* destination = m_taskData->destination;
    if ( GUCEF_NULL == destination && GUCEF_NULL != m_taskData->destinationBuffer )
    {
        bufferAccess.LinkTo( m_taskData->destinationBuffer, false );
        destination = &bufferAccess;
    }
    if ( GUCEF_NULL == destination )
    {
        m_taskData->errorText = "No destination given";
        return true;
    }
    CProgressReportingIOAccess progressAccess( *destination, this, m_taskData->reportIntervalInBytes );

    bool success = false;
    try
    {
        if ( m_taskData->isPartial )
            success = accessor->GetPartialResource( m_taskData->uri, m_taskData->byteOffset, m_taskData->bytesToGet, progressAccess );
        else
            success = accessor->GetResource( m_taskData->uri, progressAccess );
    }
    catch ( ... )
    {
        m_taskData->errorText = "Unexpected exception while retrieving";
    }

    m_bytesTransferred = progressAccess.GetTotalBytesWritten();
    m_taskData->success = success && !progressAccess.IsAborted();

    // What was delivered since the last progress report, which for small resources is everything, is reported as well
    if ( m_taskData->success && m_bytesTransferred > m_lastReportedBytes )
        Report( CAsyncUriResourceRetriever::RetrievalProgressEvent, m_bytesTransferred, m_totalBytes, CString() );
    if ( !m_taskData->success && m_taskData->errorText.IsNULLOrEmpty() && !m_stoppedEarly )
        m_taskData->errorText = "The resource accessor could not retrieve \"" + CRedactor::ForUrl().RedactUrl( m_taskData->uri.ToUriInStringForm() ) + "\"";
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAsyncUriRetrievalTaskConsumer::OnTaskEnded( CTaskPtr task ,
                                             bool wasForced )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_taskData )
    {
        if ( m_stoppedEarly || wasForced || IsStopRequested() )
            Report( CAsyncUriResourceRetriever::RetrievalCancelledEvent, m_bytesTransferred, m_totalBytes, CString() );
        else
        if ( m_taskData->success )
            Report( CAsyncUriResourceRetriever::RetrievalCompletedEvent, m_bytesTransferred, m_bytesTransferred, CString() );
        else
            Report( CAsyncUriResourceRetriever::RetrievalFailedEvent, m_bytesTransferred, m_totalBytes, m_taskData->errorText );
    }

    // The reporter is shared with the retriever, the task data goes away with the task
    if ( GUCEF_NULL != m_taskData )
        m_taskData->reporter = CAsyncUriRetrievalReporterPtr();
    m_taskData = GUCEF_NULL;
    m_task = GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
