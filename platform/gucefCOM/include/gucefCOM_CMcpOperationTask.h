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

#ifndef GUCEF_COM_CMCPOPERATIONTASK_H
#define GUCEF_COM_CMCPOPERATIONTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COM_CMCPREQUESTCONTEXT_H
#include "gucefCOM_CMcpRequestContext.h"
#define GUCEF_COM_CMCPREQUESTCONTEXT_H
#endif /* GUCEF_COM_CMCPREQUESTCONTEXT_H ? */

#ifndef GUCEF_COM_CMCPTOOLREGISTRY_H
#include "gucefCOM_CMcpToolRegistry.h"
#define GUCEF_COM_CMCPTOOLREGISTRY_H
#endif /* GUCEF_COM_CMCPTOOLREGISTRY_H ? */

#ifndef GUCEF_COM_CMCPRESOURCEREGISTRY_H
#include "gucefCOM_CMcpResourceRegistry.h"
#define GUCEF_COM_CMCPRESOURCEREGISTRY_H
#endif /* GUCEF_COM_CMCPRESOURCEREGISTRY_H ? */

#ifndef GUCEF_COM_CMCPPROMPTREGISTRY_H
#include "gucefCOM_CMcpPromptRegistry.h"
#define GUCEF_COM_CMCPPROMPTREGISTRY_H
#endif /* GUCEF_COM_CMCPPROMPTREGISTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMcpServer;

/**
 *  One call of a handler: what to run and with what. The protocol thread of the server runs RunHandler() itself for
 *  handlers of MCP_EXECUTION_CALLING_THREAD; for MCP_EXECUTION_THREAD_POOL the same data is the data of a
 *  CMcpOperationTask and the pool thread runs the same RunHandler(). The outcome goes into the request context.
 *
 *  The in-process operations are the asynchronous forms of CMcpServer::CallTool() and the like: the pool thread runs
 *  the blocking function, and the outcome stays in the task data, which the future gives.
 */
class GUCEF_COM_PUBLIC_CPP CMcpOperationTaskData : public CORE::CICloneable
{
    public:

    enum EOperation
    {
        OPERATION_NONE                  ,
        OPERATION_TOOL_CALL             ,
        OPERATION_RESOURCE_READ         ,
        OPERATION_TEMPLATE_READ         ,
        OPERATION_PROMPT_GET            ,
        OPERATION_IN_PROCESS_TOOL_CALL  ,
        OPERATION_IN_PROCESS_RESOURCE_READ,
        OPERATION_IN_PROCESS_PROMPT_GET
    };

    EOperation operation;

    // the handler and its input
    CMcpRequestContextPtr context;
    CMcpToolEntryPtr toolEntry;
    CMcpResourceEntryPtr resourceEntry;
    CMcpResourceTemplateEntryPtr templateEntry;
    CMcpPromptEntryPtr promptEntry;
    CORE::CDataNode arguments;              /**< tool arguments, an object node */
    CORE::CString uri;                      /**< the URI to read */
    CORE::CValueList variables;             /**< the variables of a template match */
    CORE::CValueList promptArguments;

    // the in-process operations
    CMcpServer* server;
    CMcpPrincipal principal;
    CORE::CString name;                     /**< the tool or prompt name, for in-process operations */
    bool succeeded;
    CORE::CString error;
    CMcpToolResult toolResult;
    CMcpResourceContents resourceContents;
    CMcpPromptResult promptResult;

    // the pool task
    bool started;

    CMcpOperationTaskData( void );

    CMcpOperationTaskData( const CMcpOperationTaskData& src );

    virtual ~CMcpOperationTaskData() GUCEF_VIRTUAL_OVERRIDE;

    CMcpOperationTaskData& operator=( const CMcpOperationTaskData& src );

    /**
     *  Runs the handler on the calling thread and records a COMPLETE, INPUT_REQUIRED or PROTOCOL_ERROR outcome in
     *  the context; a DEFERRED handler completes the context itself later. An exception of the handler is a -32603.
     */
    EMcpExecuteResult RunHandler( void );

    /** Runs an in-process operation: the blocking function of the server on the calling thread */
    void RunInProcessOperation( void );

    bool IsInProcessOperation( void ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The task that runs a handler, or an in-process operation, on a thread of the pool of the server.
 *  A handler that answers DEFERRED keeps the task alive, the task delegator pulsing its own pulse generator (which
 *  CMcpRequestContext::GetPulseGenerator() gives), until the context completes or is cancelled.
 */
class GUCEF_COM_PUBLIC_CPP CMcpOperationTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;            /**< "GUCEF::COM::CMcpOperationTask" */

    CMcpOperationTaskConsumer( void );

    virtual ~CMcpOperationTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    /** The task data stays: it has the outcome of in-process operations */
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    CMcpOperationTaskConsumer( const CMcpOperationTaskConsumer& src );
    CMcpOperationTaskConsumer& operator=( const CMcpOperationTaskConsumer& src );
};

typedef CORE::CTFactory< CORE::CTaskConsumer, CMcpOperationTaskConsumer, MT::CMutex > TMcpOperationTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPOPERATIONTASK_H ? */
