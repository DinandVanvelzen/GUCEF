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

#ifndef GUCEF_COM_MCPTYPES_H
#define GUCEF_COM_MCPTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The two ways the Model Context Protocol is spoken.
 *  Modern (2026-07-28 and later): version, client identity and client capabilities travel in the _meta of every request.
 *  Legacy (2025-11-25): an initialize request opens a session that holds them.
 */
enum EMcpProtocolEra
{
    MCP_ERA_MODERN  ,
    MCP_ERA_LEGACY
};

/**
 *  The error codes the MCP server sends.
 *  -32020 to -32099 is reserved for the MCP specification: only the codes it defines are used from it.
 *  The codes of GUCEF itself lie outside the JSON-RPC reserved range of -32768 to -32000.
 */
enum EMcpErrorCode
{
    MCP_ERROR_PARSE                                 = -32700 ,
    MCP_ERROR_INVALID_REQUEST                       = -32600 ,
    MCP_ERROR_METHOD_NOT_FOUND                      = -32601 ,
    MCP_ERROR_INVALID_PARAMS                        = -32602 ,
    MCP_ERROR_INTERNAL                              = -32603 ,
    MCP_ERROR_HEADER_MISMATCH                       = -32020 ,
    MCP_ERROR_MISSING_REQUIRED_CLIENT_CAPABILITY    = -32021 ,
    MCP_ERROR_UNSUPPORTED_PROTOCOL_VERSION          = -32022 ,
    MCP_ERROR_LEGACY_RESOURCE_NOT_FOUND             = -32002 ,     /**< 2025-11-25 only: a modern request never gets it */

    MCP_ERROR_GUCEF_RATE_LIMITED                    = -31001 ,     /**< the tool call rate of the principal is exceeded */
    MCP_ERROR_GUCEF_BUSY                            = -31002 ,     /**< a concurrency or subscription limit is reached */
    MCP_ERROR_GUCEF_ACCESS_DENIED                   = -31003       /**< the access policy refuses a visible resource or prompt */
};

/** Who may keep a cached result: anyone, or only the same authorization context */
enum EMcpCacheScope
{
    MCP_CACHESCOPE_PUBLIC   ,
    MCP_CACHESCOPE_PRIVATE
};

/** What a tool does to the world. The tool annotations the client sees are generated from it */
enum EMcpToolEffect
{
    MCP_TOOLEFFECT_READ_ONLY    ,   /**< changes nothing */
    MCP_TOOLEFFECT_ADDITIVE     ,   /**< adds, never destroys */
    MCP_TOOLEFFECT_DESTRUCTIVE      /**< may change or remove what exists */
};

/** Where the handler of a request that comes in over a transport runs */
enum EMcpExecutionMode
{
    MCP_EXECUTION_CALLING_THREAD    ,   /**< on the protocol thread, inside the dispatch: quick work that never blocks */
    MCP_EXECUTION_THREAD_POOL           /**< as a task on the thread pool of the server */
};

/** What a handler did */
enum EMcpExecuteResult
{
    MCP_EXECUTE_COMPLETE        ,   /**< the result is filled in */
    MCP_EXECUTE_INPUT_REQUIRED  ,   /**< the CMcpInputRequired is filled in: the client has to provide input and retry */
    MCP_EXECUTE_DEFERRED        ,   /**< the answer comes later through one of the CompleteWith*() of the request context */
    MCP_EXECUTE_PROTOCOL_ERROR      /**< CMcpRequestContext::SetProtocolError() was called */
};

/** The answer of an access policy */
enum EMcpAccessDecision
{
    MCP_ACCESS_ALLOW    ,
    MCP_ACCESS_DENY     ,   /**< visible but refused */
    MCP_ACCESS_HIDE         /**< answered as if the object did not exist */
};

/** How a request ended, for events and audit */
enum EMcpRequestOutcome
{
    MCP_OUTCOME_COMPLETE        ,
    MCP_OUTCOME_INPUT_REQUIRED  ,
    MCP_OUTCOME_TOOL_ERROR      ,   /**< a complete tool result with isError */
    MCP_OUTCOME_PROTOCOL_ERROR  ,   /**< a JSON-RPC error response */
    MCP_OUTCOME_CANCELLED       ,
    MCP_OUTCOME_TIMED_OUT       ,
    MCP_OUTCOME_CLIENT_GONE     ,   /**< the result was ready but the channel was closed */
    MCP_OUTCOME_REFUSED             /**< refused by the transport before it was dispatched: authentication, Host, Origin */
};

GUCEF_COM_PUBLIC_CPP const char* McpRequestOutcomeToString( EMcpRequestOutcome outcome );

GUCEF_COM_PUBLIC_CPP const char* McpCacheScopeToString( EMcpCacheScope scope );

/*-------------------------------------------------------------------------*/

/** The protocol versions and which of them this implementation serves */
class GUCEF_COM_PUBLIC_CPP CMcpProtocolVersions
{
    public:

    static const CORE::CString Version_2026_07_28;      /**< modern: served statelessly */
    static const CORE::CString Version_2025_11_25;      /**< legacy: served after initialize */

    /** The modern versions served statelessly, newest first: what server/discover lists */
    static void GetModernVersions( CORE::CString::StringVector& versions );

    /** The legacy versions accepted in initialize, newest first */
    static void GetLegacyVersions( CORE::CString::StringVector& versions );

    static bool IsModernVersion( const CORE::CString& version );

    static bool IsLegacyVersion( const CORE::CString& version );

    private:
    CMcpProtocolVersions( void );
};

/*-------------------------------------------------------------------------*/

/** The values of the resultType field of a modern result. A handler never writes it: the server does */
class GUCEF_COM_PUBLIC_CPP CMcpResultType
{
    public:

    static const CORE::CString Complete;            /**< "complete" */
    static const CORE::CString InputRequired;       /**< "input_required" */

    private:
    CMcpResultType( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The reserved _meta key names. They contain '/' and '.', so they are always matched as whole member names and
 *  never through a CDataNode path search.
 */
class GUCEF_COM_PUBLIC_CPP CMcpMetaKeys
{
    public:

    static const CORE::CString ProtocolVersion;         /**< "io.modelcontextprotocol/protocolVersion" */
    static const CORE::CString ClientCapabilities;      /**< "io.modelcontextprotocol/clientCapabilities" */
    static const CORE::CString ClientInfo;              /**< "io.modelcontextprotocol/clientInfo" */
    static const CORE::CString LogLevel;                /**< "io.modelcontextprotocol/logLevel" */
    static const CORE::CString ServerInfo;              /**< "io.modelcontextprotocol/serverInfo" */
    static const CORE::CString SubscriptionId;          /**< "io.modelcontextprotocol/subscriptionId" */
    static const CORE::CString ProgressToken;           /**< "progressToken" */
    static const CORE::CString TraceParent;             /**< "traceparent" */
    static const CORE::CString TraceState;              /**< "tracestate" */
    static const CORE::CString Baggage;                 /**< "baggage" */

    /**
     *  The vendor prefix of the _meta keys and extension identifiers of GUCEF itself, in the reverse DNS form the
     *  specification asks for: "com.vanvelzensoftware.gucef/". Every GUCEF key is built from this one constant.
     */
    static const CORE::CString GucefVendorPrefix;

    static const CORE::CString GucefAuthToken;          /**< GucefVendorPrefix + "authToken": the per request token of the TCP transport */

    private:
    CMcpMetaKeys( void );
};

/*-------------------------------------------------------------------------*/

/** The method names of the protocol */
class GUCEF_COM_PUBLIC_CPP CMcpMethods
{
    public:

    static const CORE::CString ServerDiscover;                  /**< "server/discover" */
    static const CORE::CString Initialize;                      /**< "initialize", legacy */
    static const CORE::CString Ping;                            /**< "ping", legacy */
    static const CORE::CString ToolsList;                       /**< "tools/list" */
    static const CORE::CString ToolsCall;                       /**< "tools/call" */
    static const CORE::CString ResourcesList;                   /**< "resources/list" */
    static const CORE::CString ResourcesTemplatesList;          /**< "resources/templates/list" */
    static const CORE::CString ResourcesRead;                   /**< "resources/read" */
    static const CORE::CString ResourcesSubscribe;              /**< "resources/subscribe", legacy */
    static const CORE::CString ResourcesUnsubscribe;            /**< "resources/unsubscribe", legacy */
    static const CORE::CString PromptsList;                     /**< "prompts/list" */
    static const CORE::CString PromptsGet;                      /**< "prompts/get" */
    static const CORE::CString SubscriptionsListen;             /**< "subscriptions/listen" */

    static const CORE::CString NotificationInitialized;         /**< "notifications/initialized", legacy */
    static const CORE::CString NotificationCancelled;           /**< "notifications/cancelled" */
    static const CORE::CString NotificationProgress;            /**< "notifications/progress" */
    static const CORE::CString NotificationToolsListChanged;    /**< "notifications/tools/list_changed" */
    static const CORE::CString NotificationPromptsListChanged;  /**< "notifications/prompts/list_changed" */
    static const CORE::CString NotificationResourcesListChanged;/**< "notifications/resources/list_changed" */
    static const CORE::CString NotificationResourcesUpdated;    /**< "notifications/resources/updated" */
    static const CORE::CString NotificationSubscriptionsAcknowledged; /**< "notifications/subscriptions/acknowledged" */

    static const CORE::CString ElicitationCreate;               /**< "elicitation/create", an input request of MRTR */
    static const CORE::CString SamplingCreateMessage;           /**< "sampling/createMessage", an input request of MRTR */
    static const CORE::CString RootsList;                       /**< "roots/list", an input request of MRTR */

    private:
    CMcpMethods( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_MCPTYPES_H ? */
