/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefWEB_CHttpClientRequestRunner.h"
#include "gucefWEB_CHttpClientRequestTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CHttpClientRequestTaskConsumer::TaskType = "GUCEF::WEB::CHttpClientRequestTask";

const CORE::CEvent CHttpClientRequestTaskConsumer::ConnectingEvent = "GUCEF::WEB::CHttpClientRequestTaskConsumer::ConnectingEvent";
const CORE::CEvent CHttpClientRequestTaskConsumer::ConnectedEvent = "GUCEF::WEB::CHttpClientRequestTaskConsumer::ConnectedEvent";
const CORE::CEvent CHttpClientRequestTaskConsumer::ResponseHeadersEvent = "GUCEF::WEB::CHttpClientRequestTaskConsumer::ResponseHeadersEvent";
const CORE::CEvent CHttpClientRequestTaskConsumer::BodyDataEvent = "GUCEF::WEB::CHttpClientRequestTaskConsumer::BodyDataEvent";
const CORE::CEvent CHttpClientRequestTaskConsumer::DataSentEvent = "GUCEF::WEB::CHttpClientRequestTaskConsumer::DataSentEvent";
const CORE::CEvent CHttpClientRequestTaskConsumer::RedirectEvent = "GUCEF::WEB::CHttpClientRequestTaskConsumer::RedirectEvent";
const CORE::CEvent CHttpClientRequestTaskConsumer::RequestFinishedEvent = "GUCEF::WEB::CHttpClientRequestTaskConsumer::RequestFinishedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpClientRequestTaskData::CHttpClientRequestTaskData( void )
    : CORE::CICloneable()
    , request()
    , options()
    , response()
    , transferInfo()
    , backends()
    , reportProgress( false )
    , progressRequestId( 0 )
    , progressNotifier()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpClientRequestTaskData::CHttpClientRequestTaskData( const CHttpClientRequestTaskData& src )
    : CORE::CICloneable( src )
    , request( src.request )
    , options( src.options )
    , response( src.response )
    , transferInfo( src.transferInfo )
    , backends( src.backends )
    , reportProgress( src.reportProgress )
    , progressRequestId( src.progressRequestId )
    , progressNotifier( src.progressNotifier )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpClientRequestTaskData::~CHttpClientRequestTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CHttpClientRequestTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CHttpClientRequestTaskData( *this );
}

/*-------------------------------------------------------------------------*/

CHttpClientProgressNotifier::CHttpClientProgressNotifier( void )
    : CORE::CTSGNotifier()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpClientProgressNotifier::~CHttpClientProgressNotifier()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CHttpClientProgressNotifier::Report( const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    return NotifyObservers( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

const CString&
CHttpClientProgressNotifier::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CHttpClientProgressNotifier";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CHttpClientRequestProgressData::CHttpClientRequestProgressData( UInt64 id )
    : CORE::CICloneable()
    , requestId( id )
    , statusCode( 0 )
    , contentLength( 0 )
    , resumeSupported( false )
    , data()
    , byteCount( 0 )
    , url()
    , transferInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CHttpClientRequestProgressData::Clone( void ) const
{GUCEF_TRACE;

    // a linked data buffer becomes a copy, which the clone owns
    CHttpClientRequestProgressData* clone = GUCEF_NEW CHttpClientRequestProgressData( requestId );
    clone->statusCode = statusCode;
    clone->contentLength = contentLength;
    clone->resumeSupported = resumeSupported;
    if ( data.GetDataSize() > 0 )
        clone->data.CopyFrom( data.GetDataSize(), data.GetConstBufferPtr() );
    clone->byteCount = byteCount;
    clone->url = url;
    clone->transferInfo = transferInfo;
    return clone;
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestTaskConsumer::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectingEvent.Initialize();
    ConnectedEvent.Initialize();
    ResponseHeadersEvent.Initialize();
    BodyDataEvent.Initialize();
    DataSentEvent.Initialize();
    RedirectEvent.Initialize();
    RequestFinishedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CHttpClientRequestTaskConsumer::CHttpClientRequestTaskConsumer( void )
    : CORE::CTaskConsumer()
    , CIHttpClientBackendObserver()
    , m_runner( GUCEF_NULL )
    , m_reportProgress( false )
    , m_progressRequestId( 0 )
    , m_progressNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CHttpClientRequestTaskConsumer::~CHttpClientRequestTaskConsumer()
{GUCEF_TRACE;

    GUCEF_DELETE m_runner;
    m_runner = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHttpClientRequestTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientRequestTaskConsumer::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    CHttpClientRequestTaskData* taskData = static_cast< CHttpClientRequestTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == taskData )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpClientRequestTaskConsumer: Task without request data" );
        return false;
    }

    m_reportProgress = taskData->reportProgress;
    m_progressRequestId = taskData->progressRequestId;
    m_progressNotifier = taskData->progressNotifier;
    GUCEF_DELETE m_runner;
    m_runner = GUCEF_NEW CHttpClientRequestRunner( taskData->backends, taskData->request, taskData->options,
                                                   taskData->response, taskData->transferInfo, this );

    // The request runs on the pulses of the thread carrying out this task: the pulse generator of the task
    // delegator, which the consumer was given together with the delegator
    CORE::PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    if ( pulseGenerator.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpClientRequestTaskConsumer: No pulse generator available for the task thread" );
        return false;
    }

    if ( m_reportProgress )
    {
        CHttpClientRequestProgressData eventData( m_progressRequestId );
        Report( ConnectingEvent, &eventData );
    }

    // A request that fails to start is finished, reported by the first cycle
    m_runner->Start( pulseGenerator );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientRequestTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return GUCEF_NULL == m_runner || m_runner->Update();
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestTaskConsumer::OnTaskEnding( CORE::CTaskPtr task, bool willBeForced )
{GUCEF_TRACE;

    // Ending before the request is done means it was cancelled
    if ( GUCEF_NULL != m_runner && !m_runner->IsFinished() )
        m_runner->Cancel();
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_runner && !m_runner->IsFinished() )
        m_runner->Cancel();
    GUCEF_DELETE m_runner;
    m_runner = GUCEF_NULL;

    if ( m_reportProgress && !task.IsNULL() )
    {
        CHttpClientRequestTaskData* taskData = static_cast< CHttpClientRequestTaskData* >( task->GetTaskData() );
        if ( GUCEF_NULL != taskData )
        {
            CHttpClientRequestProgressData eventData( m_progressRequestId );
            eventData.transferInfo = taskData->transferInfo;
            Report( RequestFinishedEvent, &eventData );
        }
    }
    m_progressNotifier = CHttpClientProgressNotifierPtr();
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientRequestTaskConsumer::Report( const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CHttpClientProgressNotifierPtr progressNotifier = m_progressNotifier;
    if ( progressNotifier.IsNULL() )
        return true;
    return progressNotifier->Report( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestTaskConsumer::OnHttpClientConnected( void )
{GUCEF_TRACE;

    if ( m_reportProgress )
    {
        CHttpClientRequestProgressData eventData( m_progressRequestId );
        Report( ConnectedEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestTaskConsumer::OnHttpClientResponseHeadersComplete( const CHttpResponseData& response )
{GUCEF_TRACE;

    if ( m_reportProgress )
    {
        CHttpClientRequestProgressData eventData( m_progressRequestId );
        eventData.statusCode = response.statusCode;
        for ( CHttpResponseData::TStringMap::const_iterator i = response.otherHeaders.begin(); i != response.otherHeaders.end(); ++i )
        {
            if ( (*i).first.Equals( "Content-Length", false ) )
                eventData.contentLength = CORE::StringToUInt64( (*i).second );
            else
            if ( (*i).first.Equals( "Accept-Ranges", false ) )
                eventData.resumeSupported = (*i).second.Equals( "bytes", false );
        }
        Report( ResponseHeadersEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestTaskConsumer::OnHttpClientBodyData( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    if ( m_reportProgress )
    {
        // The data is linked here, it is only copied when the event is passed to another thread
        CHttpClientRequestProgressData eventData( m_progressRequestId );
        eventData.data.LinkTo( data, dataSize );
        Report( BodyDataEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestTaskConsumer::OnHttpClientDataSent( UInt32 bytesSent )
{GUCEF_TRACE;

    if ( m_reportProgress )
    {
        CHttpClientRequestProgressData eventData( m_progressRequestId );
        eventData.byteCount = bytesSent;
        Report( DataSentEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

void
CHttpClientRequestTaskConsumer::OnHttpClientRedirect( const CString& newUrl )
{GUCEF_TRACE;

    if ( m_reportProgress )
    {
        CHttpClientRequestProgressData eventData( m_progressRequestId );
        eventData.url = newUrl;
        Report( RedirectEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
