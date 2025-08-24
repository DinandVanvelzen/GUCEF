/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include <string.h>
#include <time.h>

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H */

#ifndef GUCEF_WEB_CASYNCHTTPSERVERRESPONSEHANDLER_H
#include "gucefWEB_CAsyncHttpServerResponseHandler.h"
#define GUCEF_WEB_CASYNCHTTPSERVERRESPONSEHANDLER_H
#endif /* GUCEF_WEB_CASYNCHTTPSERVERRESPONSEHANDLER_H */

#include "gucefWEB_CAsyncHttpServerRequestHandler.h"

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

const CORE::CString CAsyncHttpServerRequestHandler::TaskType = "AsyncHttpServerRequestHandler";
const CORE::CEvent CAsyncHttpServerRequestHandler::AsyncHttpServerRequestHandlingCompletedEvent = "GUCEF::COM::CAsyncHttpServerRequestHandler::AsyncHttpServerRequestHandlingCompletedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAsyncHttpRequestData::CAsyncHttpRequestData( void )
    : CHttpRequestData()
    , httpServer( GUCEF_NULL )
    , clientConnectionId( 0 )
    , remoteClientAddress()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncHttpRequestData::CAsyncHttpRequestData( const CAsyncHttpRequestData& src )
    : CHttpRequestData( src )
    , httpServer( src.httpServer )
    , clientConnectionId( src.clientConnectionId )
    , remoteClientAddress( src.remoteClientAddress )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncHttpRequestData::~CAsyncHttpRequestData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAsyncHttpRequestData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAsyncHttpRequestData( *this );
}

/*-------------------------------------------------------------------------*/

void
CAsyncHttpServerRequestHandler::RegisterEvents( void )
{GUCEF_TRACE;

    AsyncHttpServerRequestHandlingCompletedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAsyncHttpServerRequestHandler::CAsyncHttpServerRequestHandler( void )
    : CORE::CTaskConsumer()
    , m_requestHandler()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAsyncHttpServerRequestHandler::CAsyncHttpServerRequestHandler( const CAsyncHttpServerRequestHandler& src )
    : CORE::CTaskConsumer()
    , m_requestHandler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncHttpServerRequestHandler::~CAsyncHttpServerRequestHandler()
{GUCEF_TRACE;

    m_requestHandler.Unlink();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAsyncHttpServerRequestHandler::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncHttpServerRequestHandler::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncHttpServerRequestHandler(" + CORE::PointerToString( this ) + "):OnTaskStart" );
    if ( !task.IsNULL() )
    {
        CORE::CICloneable* opaqueTaskData = task->GetTaskData();
        CAsyncHttpRequestData* httpRequestData = static_cast< CAsyncHttpRequestData* >( opaqueTaskData );
        if ( GUCEF_NULL != httpRequestData )
        {
            m_requestHandler = httpRequestData->httpServer->GetRequestHandlerFactory()->Create();
            if ( m_requestHandler.IsNULL() )
                return false;
        }
        return true;
    }
    return false; 
}

/*-------------------------------------------------------------------------*/

bool
CAsyncHttpServerRequestHandler::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true; // true means we are done

    time_t startTime = time( NULL );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncHttpServerRequestHandler(" + CORE::PointerToString( this ) + "):OnTaskCycle" );

    CORE::CICloneable* opaqueTaskData = task->GetTaskData();
    CAsyncHttpRequestData* httpRequestData = static_cast< CAsyncHttpRequestData* >( opaqueTaskData );
    if ( GUCEF_NULL == httpRequestData )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AsyncHttpServerRequestHandler(" + CORE::PointerToString( this ) + "):OnTaskCycle: No task data (http request) to operate upon" );
        return true;
    }

    if ( m_requestHandler.IsNULL() )
    {
        m_requestHandler = httpRequestData->httpServer->GetRequestHandlerFactory()->Create();
        if ( m_requestHandler.IsNULL() )
            return true; // true means we are done
    }

    CAsyncHttpResponseData* response = GUCEF_NEW CAsyncHttpResponseData( httpRequestData );
    m_requestHandler->OnRequest( *httpRequestData, *response );

    CORE::CFutureResult futureResult = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool()->QueueTask( CAsyncHttpServerResponseHandler::TaskType,
                                                                                                                   response,
                                                                                                                   GUCEF_NULL,
                                                                                                                   true );
    if ( futureResult.HasNoFuture() )
    {
        // Failed to queue task to send the response to the client
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AsyncHttpServerRequestHandler(" + CORE::ToString( this ) + "):OnTaskCycle: Failed to QueueTask to dispatch result to client after request processing" );

        // Use this thread instead as a fallback
        CORE::CDynamicBuffer fullResponse;
        if ( response->Serialize( fullResponse ) )
        {
            httpRequestData->httpServer->SendResponseASync( response->clientConnectionId  ,
                                                            fullResponse                  ,
                                                            response->remoteClientAddress );
        }
    }

    return true; // true means we are done
}

/*-------------------------------------------------------------------------*/

void
CAsyncHttpServerRequestHandler::OnTaskEnding( CORE::CTaskPtr task ,
                                              bool willBeForced   )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAsyncHttpServerRequestHandler::OnTaskEnded( CORE::CTaskPtr task ,
                                             bool wasForced      )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncHttpServerRequestHandler(" + CORE::PointerToString( this ) + "):OnTaskEnd" );

    m_requestHandler.Unlink();

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
