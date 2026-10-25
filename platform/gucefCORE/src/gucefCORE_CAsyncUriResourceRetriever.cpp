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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#include "gucefCORE_CAsyncUriRetrievalTask.h"
#include "gucefCORE_CAsyncUriResourceRetriever.h"

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

const CEvent CAsyncUriResourceRetriever::RetrievalStartedEvent = "GUCEF::CORE::CAsyncUriResourceRetriever::RetrievalStartedEvent";
const CEvent CAsyncUriResourceRetriever::RetrievalProgressEvent = "GUCEF::CORE::CAsyncUriResourceRetriever::RetrievalProgressEvent";
const CEvent CAsyncUriResourceRetriever::RetrievalCompletedEvent = "GUCEF::CORE::CAsyncUriResourceRetriever::RetrievalCompletedEvent";
const CEvent CAsyncUriResourceRetriever::RetrievalFailedEvent = "GUCEF::CORE::CAsyncUriResourceRetriever::RetrievalFailedEvent";
const CEvent CAsyncUriResourceRetriever::RetrievalCancelledEvent = "GUCEF::CORE::CAsyncUriResourceRetriever::RetrievalCancelledEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAsyncUriRetrievalEventData::CAsyncUriRetrievalEventData( void )
    : CICloneable()
    , retrievalId( 0 )
    , uri()
    , bytesTransferred( 0 )
    , totalBytes( -1 )
    , errorText()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncUriRetrievalEventData::CAsyncUriRetrievalEventData( const CAsyncUriRetrievalEventData& src )
    : CICloneable( src )
    , retrievalId( src.retrievalId )
    , uri( src.uri )
    , bytesTransferred( src.bytesTransferred )
    , totalBytes( src.totalBytes )
    , errorText( src.errorText )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncUriRetrievalEventData::~CAsyncUriRetrievalEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CICloneable*
CAsyncUriRetrievalEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAsyncUriRetrievalEventData( *this );
}

/*-------------------------------------------------------------------------*/

void
CAsyncUriResourceRetriever::RegisterEvents( void )
{GUCEF_TRACE;

    RetrievalStartedEvent.Initialize();
    RetrievalProgressEvent.Initialize();
    RetrievalCompletedEvent.Initialize();
    RetrievalFailedEvent.Initialize();
    RetrievalCancelledEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAsyncUriResourceRetriever::CAsyncUriResourceRetriever( void )
    : CTSGNotifier()
    , m_reporter( GUCEF_NEW CAsyncUriRetrievalReporter() )
    , m_threadPool()
    , m_activeThreadPool()
    , m_currentRetrieval( GUCEF_NULL )
    , m_currentRetrievalId( 0 )
    , m_lastRetrievalId( 0 )
    , m_busy( false )
    , m_queryTotalSize( false )
    , m_progressReportIntervalInBytes( 64 * 1024 )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CAsyncUriResourceRetriever::CAsyncUriResourceRetriever( PulseGeneratorPtr pulseGenerator )
    : CTSGNotifier( pulseGenerator )
    , m_reporter( GUCEF_NEW CAsyncUriRetrievalReporter() )
    , m_threadPool()
    , m_activeThreadPool()
    , m_currentRetrieval( GUCEF_NULL )
    , m_currentRetrievalId( 0 )
    , m_lastRetrievalId( 0 )
    , m_busy( false )
    , m_queryTotalSize( false )
    , m_progressReportIntervalInBytes( 64 * 1024 )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CAsyncUriResourceRetriever::Initialize( void )
{GUCEF_TRACE;

    RegisterEvents();

    // Progress of the retrieval arrives via the shared reporter, on our own thread
    m_reporter->Subscribe( &AsObserver() );
}

/*-------------------------------------------------------------------------*/

CAsyncUriResourceRetriever::~CAsyncUriResourceRetriever()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_reporter->Unsubscribe( &AsObserver() );

    // The destination must not be touched after we are gone
    if ( GUCEF_NULL != m_currentRetrieval && !m_currentRetrieval->IsReady() && !m_activeThreadPool.IsNULL() )
        m_activeThreadPool->RequestTaskCancellation( m_currentRetrieval->GetTaskId(), true, true );

    GUCEF_DELETE m_currentRetrieval;
    m_currentRetrieval = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CFutureResult
CAsyncUriResourceRetriever::QueueRetrieval( const CUri& uri            ,
                                            bool isPartial             ,
                                            UInt64 byteOffset          ,
                                            Int64 bytesToGet           ,
                                            CIOAccess* destination     ,
                                            CDynamicBuffer* buffer     )
{GUCEF_TRACE;

    if ( m_busy )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AsyncUriResourceRetriever: A retrieval is already busy, cannot start retrieving \"" << CRedactor::ForUrl().RedactUrl( uri.ToUriInStringForm() ) << "\"" );
        return CFutureResult( TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
    }

    ThreadPoolPtr threadPool = GetThreadPool();
    if ( threadPool.IsNULL() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AsyncUriResourceRetriever: No thread pool available to carry out the retrieval" );
        return CFutureResult( TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
    }

    CAsyncUriRetrievalTaskData* taskData = GUCEF_NEW CAsyncUriRetrievalTaskData();
    taskData->retrievalId = ++m_lastRetrievalId;
    taskData->uri = uri;
    taskData->isPartial = isPartial;
    taskData->byteOffset = byteOffset;
    taskData->bytesToGet = bytesToGet;
    taskData->destination = destination;
    taskData->destinationBuffer = buffer;
    taskData->queryTotalSize = m_queryTotalSize;
    taskData->reportIntervalInBytes = m_progressReportIntervalInBytes;
    taskData->reporter = m_reporter;

    CFutureResult future = threadPool->QueueTask( CAsyncUriRetrievalTaskConsumer::TaskType, taskData, GUCEF_NULL, true );
    if ( future.HasNoFuture() )
    {
        // Ownership of the task data is only transferred on success
        GUCEF_DELETE taskData;
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AsyncUriResourceRetriever: Failed to queue the retrieval of \"" << CRedactor::ForUrl().RedactUrl( uri.ToUriInStringForm() ) << "\"" );
        return future;
    }

    GUCEF_DELETE m_currentRetrieval;
    m_currentRetrieval = GUCEF_NEW CFutureResult( future );
    m_currentRetrievalId = taskData->retrievalId;
    m_activeThreadPool = threadPool;
    m_busy = true;
    return future;
}

/*-------------------------------------------------------------------------*/

CFutureResult
CAsyncUriResourceRetriever::Retrieve( const CUri& uri       ,
                                      CIOAccess& destination )
{GUCEF_TRACE;

    return QueueRetrieval( uri, false, 0, -1, &destination, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CAsyncUriResourceRetriever::Retrieve( const CUri& uri            ,
                                      CDynamicBuffer& destination )
{GUCEF_TRACE;

    return QueueRetrieval( uri, false, 0, -1, GUCEF_NULL, &destination );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CAsyncUriResourceRetriever::GetPartial( const CUri& uri        ,
                                        UInt64 byteOffset      ,
                                        Int64 bytesToGet       ,
                                        CIOAccess& destination )
{GUCEF_TRACE;

    return QueueRetrieval( uri, true, byteOffset, bytesToGet, &destination, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CAsyncUriResourceRetriever::GetPartial( const CUri& uri            ,
                                        UInt64 byteOffset          ,
                                        Int64 bytesToGet           ,
                                        CDynamicBuffer& destination )
{GUCEF_TRACE;

    return QueueRetrieval( uri, true, byteOffset, bytesToGet, GUCEF_NULL, &destination );
}

/*-------------------------------------------------------------------------*/

void
CAsyncUriResourceRetriever::Cancel( bool waitForEnd )
{GUCEF_TRACE;

    if ( !m_busy || GUCEF_NULL == m_currentRetrieval || m_activeThreadPool.IsNULL() )
        return;

    UInt32 retrievalId = m_currentRetrievalId;
    m_activeThreadPool->RequestTaskCancellation( m_currentRetrieval->GetTaskId(), waitForEnd, true );

    // A task that is ended as cancelled might never have run, in which case nothing else will report it.
    // Whatever the task did report before ending is dropped from here on.
    if ( m_currentRetrieval->IsReady() )
    {
        CTaskPtr task = m_currentRetrieval->GetResult( 0 );
        if ( !task.IsNULL() && TASKSTATUS_CANCELLED == task->GetTaskStatus() )
        {
            m_busy = false;
            CAsyncUriRetrievalEventData eventData;
            eventData.retrievalId = retrievalId;
            NotifyObservers( RetrievalCancelledEvent, &eventData );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CAsyncUriResourceRetriever::IsBusy( void ) const
{GUCEF_TRACE;

    return m_busy;
}

/*-------------------------------------------------------------------------*/

UInt32
CAsyncUriResourceRetriever::GetCurrentRetrievalId( void ) const
{GUCEF_TRACE;

    return m_currentRetrievalId;
}

/*-------------------------------------------------------------------------*/

void
CAsyncUriResourceRetriever::SetQueryTotalSize( bool queryTotalSize )
{GUCEF_TRACE;

    m_queryTotalSize = queryTotalSize;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncUriResourceRetriever::GetQueryTotalSize( void ) const
{GUCEF_TRACE;

    return m_queryTotalSize;
}

/*-------------------------------------------------------------------------*/

void
CAsyncUriResourceRetriever::SetProgressReportIntervalInBytes( UInt32 intervalInBytes )
{GUCEF_TRACE;

    m_progressReportIntervalInBytes = intervalInBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CAsyncUriResourceRetriever::GetProgressReportIntervalInBytes( void ) const
{GUCEF_TRACE;

    return m_progressReportIntervalInBytes;
}

/*-------------------------------------------------------------------------*/

void
CAsyncUriResourceRetriever::SetThreadPool( ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    m_threadPool = threadPool;
}

/*-------------------------------------------------------------------------*/

ThreadPoolPtr
CAsyncUriResourceRetriever::GetThreadPool( void ) const
{GUCEF_TRACE;

    if ( !m_threadPool.IsNULL() )
        return m_threadPool;
    return CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();
}

/*-------------------------------------------------------------------------*/

void
CAsyncUriResourceRetriever::OnPumpedNotify( CNotifier* notifier       ,
                                            const CEvent& eventid     ,
                                            CICloneable* eventdata    )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventdata )
        return;

    bool isFinal = eventid == RetrievalCompletedEvent || eventid == RetrievalFailedEvent || eventid == RetrievalCancelledEvent;
    if ( !isFinal && eventid != RetrievalStartedEvent && eventid != RetrievalProgressEvent )
        return;

    // Anything of a retrieval that is not the current one anymore (cancelled and replaced) is dropped
    CAsyncUriRetrievalEventData* data = static_cast< CAsyncUriRetrievalEventData* >( eventdata );
    if ( !m_busy || data->retrievalId != m_currentRetrievalId )
        return;

    if ( isFinal )
        m_busy = false;

    // Note: when NotifyObservers() returns false this object was destroyed and must not be touched anymore
    NotifyObservers( eventid, eventdata );
}

/*-------------------------------------------------------------------------*/

const CString&
CAsyncUriResourceRetriever::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CAsyncUriResourceRetriever";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
