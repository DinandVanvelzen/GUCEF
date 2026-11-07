/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#include "gucefCOM_CMcpOperationTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMcpOperationTaskConsumer::TaskType = "GUCEF::COM::CMcpOperationTask";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpOperationTaskData::CMcpOperationTaskData( void )
    : CORE::CICloneable()
    , operation( OPERATION_NONE )
    , context()
    , toolEntry()
    , resourceEntry()
    , templateEntry()
    , promptEntry()
    , arguments( GUCEF_DATATYPE_OBJECT )
    , uri()
    , variables()
    , promptArguments()
    , server( GUCEF_NULL )
    , principal()
    , name()
    , succeeded( false )
    , error()
    , toolResult()
    , resourceContents()
    , promptResult()
    , started( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpOperationTaskData::CMcpOperationTaskData( const CMcpOperationTaskData& src )
    : CORE::CICloneable( src )
    , operation( src.operation )
    , context( src.context )
    , toolEntry( src.toolEntry )
    , resourceEntry( src.resourceEntry )
    , templateEntry( src.templateEntry )
    , promptEntry( src.promptEntry )
    , arguments( src.arguments )
    , uri( src.uri )
    , variables( src.variables )
    , promptArguments( src.promptArguments )
    , server( src.server )
    , principal( src.principal )
    , name( src.name )
    , succeeded( src.succeeded )
    , error( src.error )
    , toolResult( src.toolResult )
    , resourceContents( src.resourceContents )
    , promptResult( src.promptResult )
    , started( src.started )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpOperationTaskData::~CMcpOperationTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpOperationTaskData&
CMcpOperationTaskData::operator=( const CMcpOperationTaskData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        operation = src.operation;
        context = src.context;
        toolEntry = src.toolEntry;
        resourceEntry = src.resourceEntry;
        templateEntry = src.templateEntry;
        promptEntry = src.promptEntry;
        arguments = src.arguments;
        uri = src.uri;
        variables = src.variables;
        promptArguments = src.promptArguments;
        server = src.server;
        principal = src.principal;
        name = src.name;
        succeeded = src.succeeded;
        error = src.error;
        toolResult = src.toolResult;
        resourceContents = src.resourceContents;
        promptResult = src.promptResult;
        started = src.started;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMcpOperationTaskData::IsInProcessOperation( void ) const
{GUCEF_TRACE;

    return OPERATION_IN_PROCESS_TOOL_CALL == operation || OPERATION_IN_PROCESS_RESOURCE_READ == operation || OPERATION_IN_PROCESS_PROMPT_GET == operation;
}

/*-------------------------------------------------------------------------*/

EMcpExecuteResult
CMcpOperationTaskData::RunHandler( void )
{GUCEF_TRACE;

    if ( context.IsNULL() )
        return MCP_EXECUTE_PROTOCOL_ERROR;

    CMcpRequestContext& requestContext = *context;
    CMcpInputRequired inputRequired;
    EMcpExecuteResult outcome = MCP_EXECUTE_PROTOCOL_ERROR;
    try
    {
        switch ( operation )
        {
            case OPERATION_TOOL_CALL:
            {
                CMcpToolResult result;
                outcome = toolEntry->tool->Execute( requestContext, arguments, result, inputRequired );
                if ( MCP_EXECUTE_COMPLETE == outcome )
                    requestContext.CompleteWithToolResult( result );
                break;
            }
            case OPERATION_RESOURCE_READ:
            {
                CMcpResourceContents contents;
                outcome = resourceEntry->resource->Read( requestContext, contents, inputRequired );
                if ( MCP_EXECUTE_COMPLETE == outcome )
                    requestContext.CompleteWithResourceContents( contents );
                break;
            }
            case OPERATION_TEMPLATE_READ:
            {
                CMcpResourceContents contents;
                outcome = templateEntry->resourceTemplate->Read( requestContext, uri, variables, contents, inputRequired );
                if ( MCP_EXECUTE_COMPLETE == outcome )
                    requestContext.CompleteWithResourceContents( contents );
                break;
            }
            case OPERATION_PROMPT_GET:
            {
                CMcpPromptResult result;
                outcome = promptEntry->prompt->Get( requestContext, promptArguments, result, inputRequired );
                if ( MCP_EXECUTE_COMPLETE == outcome )
                    requestContext.CompleteWithPromptResult( result );
                break;
            }
            default:
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpOperationTask: No handler to run for operation " << (Int32) operation );
                requestContext.CompleteWithProtocolError( MCP_ERROR_INTERNAL, "Internal error" );
                return MCP_EXECUTE_PROTOCOL_ERROR;
            }
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "McpOperationTask: The handler of " << requestContext.GetMethod() << " threw: " << e.what() );
        requestContext.CompleteWithProtocolError( MCP_ERROR_INTERNAL, "Internal error" );
        return MCP_EXECUTE_PROTOCOL_ERROR;
    }
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "McpOperationTask: The handler of " << requestContext.GetMethod() << " threw" );
        requestContext.CompleteWithProtocolError( MCP_ERROR_INTERNAL, "Internal error" );
        return MCP_EXECUTE_PROTOCOL_ERROR;
    }

    switch ( outcome )
    {
        case MCP_EXECUTE_COMPLETE:
        case MCP_EXECUTE_DEFERRED:
            break;
        case MCP_EXECUTE_INPUT_REQUIRED:
            requestContext.CompleteWithInputRequired( inputRequired );
            break;
        case MCP_EXECUTE_PROTOCOL_ERROR:
        default:
            requestContext.CompleteWithRecordedProtocolError();
            outcome = MCP_EXECUTE_PROTOCOL_ERROR;
            break;
    }
    return outcome;
}

/*-------------------------------------------------------------------------*/

void
CMcpOperationTaskData::RunInProcessOperation( void )
{GUCEF_TRACE;

    succeeded = false;
    if ( GUCEF_NULL == server )
    {
        error = "no server";
        return;
    }
    switch ( operation )
    {
        case OPERATION_IN_PROCESS_TOOL_CALL:
            succeeded = server->CallTool( name, arguments, principal, toolResult, error );
            break;
        case OPERATION_IN_PROCESS_RESOURCE_READ:
            succeeded = server->ReadResource( uri, principal, resourceContents, error );
            break;
        case OPERATION_IN_PROCESS_PROMPT_GET:
            succeeded = server->GetPrompt( name, promptArguments, principal, promptResult, error );
            break;
        default:
            error = "not an in-process operation";
            break;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMcpOperationTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMcpOperationTaskData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMcpOperationTaskData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMcpOperationTaskData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CMcpOperationTaskData";
}

/*-------------------------------------------------------------------------*/

CMcpOperationTaskConsumer::CMcpOperationTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpOperationTaskConsumer::~CMcpOperationTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CMcpOperationTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CMcpOperationTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CMcpOperationTaskData* data = CORE::CICloneable::CloneableAs< CMcpOperationTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpOperationTask: No task data to work on" );
        return true;
    }

    if ( data->IsInProcessOperation() )
    {
        if ( task->IsCancellationRequested() )
        {
            data->error = "cancelled";
            return true;
        }
        data->RunInProcessOperation();
        return true;
    }

    if ( data->context.IsNULL() )
        return true;

    if ( !data->started )
    {
        data->started = true;
        if ( task->IsCancellationRequested() || data->context->IsCancellationRequested() )
            return true;

        // event driven work of a deferred handler runs on the pulse generator of this thread: that of the task delegator
        data->context->SetPulseGenerator( GetPulseGenerator() );
        if ( MCP_EXECUTE_DEFERRED != data->RunHandler() )
            return true;
    }

    // a deferred handler: stay until the context completes or the request is cancelled
    return data->context->IsCompleted() || data->context->IsCancellationRequested() || task->IsCancellationRequested();
}

/*-------------------------------------------------------------------------*/

void
CMcpOperationTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
