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

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H */

#include "gucefWEB_CAsyncHttpServerResponseHandler.h"

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

const CORE::CString CAsyncHttpServerResponseHandler::TaskType = "AsyncHttpServerResponseHandler";
const CORE::CEvent CAsyncHttpServerResponseHandler::AsyncHttpServerResponseHandlingCompletedEvent = "GUCEF::COM::CAsyncHttpServerResponseHandler::AsyncHttpServerResponseHandlingCompletedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAsyncHttpResponseData::CAsyncHttpResponseData( CAsyncHttpRequestData* request )
    : CHttpResponseData()
    , httpServer( GUCEF_NULL )
    , clientConnectionId( 0 )
    , remoteClientAddress()
{GUCEF_TRACE;

    if ( GUCEF_NULL != request )
    {
        httpServer = request->httpServer;
        clientConnectionId = request->clientConnectionId;
        remoteClientAddress = request->remoteClientAddress;
        keepConnectionsAlive = request->keepConnectionsAlive;
    }
}

/*-------------------------------------------------------------------------*/

CAsyncHttpResponseData::CAsyncHttpResponseData( const CAsyncHttpResponseData& src )
    : CHttpResponseData( src )
    , httpServer( src.httpServer )
    , clientConnectionId( src.clientConnectionId )
    , remoteClientAddress( src.remoteClientAddress )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncHttpResponseData::~CAsyncHttpResponseData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CAsyncHttpResponseData::Clone( void ) const 
{GUCEF_TRACE;

    return GUCEF_NEW CAsyncHttpResponseData( *this );
}

/*-------------------------------------------------------------------------*/

void 
CAsyncHttpServerResponseHandler::RegisterEvents( void )
{GUCEF_TRACE;

    AsyncHttpServerResponseHandlingCompletedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAsyncHttpServerResponseHandler::CAsyncHttpServerResponseHandler( void )
    : CORE::CTaskConsumer() 
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAsyncHttpServerResponseHandler::CAsyncHttpServerResponseHandler( const CAsyncHttpServerResponseHandler& src )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncHttpServerResponseHandler::~CAsyncHttpServerResponseHandler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAsyncHttpServerResponseHandler::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncHttpServerResponseHandler::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncHttpServerResponseHandler(" + CORE::PointerToString( this ) + "):OnTaskStart" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncHttpServerResponseHandler::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncHttpServerResponseHandler(" + CORE::PointerToString( this ) + "):OnTaskCycle" );

    if ( !task.IsNULL() )
    {
        CORE::CICloneable* opaqueTaskData = task->GetTaskData();
        CAsyncHttpResponseData* httpResponseData = static_cast< CAsyncHttpResponseData* >( opaqueTaskData );
        if ( GUCEF_NULL == httpResponseData )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AsyncHttpServerResponseHandler(" + CORE::PointerToString( this ) + "):OnTaskCycle: No task data (http response) to operate upon" );
            return true;
        }

        CORE::CDynamicBuffer fullResponse;
        if ( httpResponseData->Serialize( fullResponse ) )
        {
            httpResponseData->httpServer->SendResponseASync( httpResponseData->clientConnectionId  ,
                                                             fullResponse                          ,
                                                             httpResponseData->remoteClientAddress );
        }        
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAsyncHttpServerResponseHandler::OnTaskEnding( CORE::CTaskPtr task ,
                                               bool willBeForced   )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAsyncHttpServerResponseHandler::OnTaskEnded( CORE::CTaskPtr task ,
                                              bool wasForced      )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AsyncHttpServerResponseHandler(" + CORE::PointerToString( this ) + "):OnTaskEnded" );
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
