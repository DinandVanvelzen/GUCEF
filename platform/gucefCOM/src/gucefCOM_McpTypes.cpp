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

#include "gucefCOM_McpTypes.h"

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

const CORE::CString CMcpProtocolVersions::Version_2026_07_28 = "2026-07-28";
const CORE::CString CMcpProtocolVersions::Version_2025_11_25 = "2025-11-25";

const CORE::CString CMcpResultType::Complete = "complete";
const CORE::CString CMcpResultType::InputRequired = "input_required";

const CORE::CString CMcpMetaKeys::ProtocolVersion = "io.modelcontextprotocol/protocolVersion";
const CORE::CString CMcpMetaKeys::ClientCapabilities = "io.modelcontextprotocol/clientCapabilities";
const CORE::CString CMcpMetaKeys::ClientInfo = "io.modelcontextprotocol/clientInfo";
const CORE::CString CMcpMetaKeys::LogLevel = "io.modelcontextprotocol/logLevel";
const CORE::CString CMcpMetaKeys::ServerInfo = "io.modelcontextprotocol/serverInfo";
const CORE::CString CMcpMetaKeys::SubscriptionId = "io.modelcontextprotocol/subscriptionId";
const CORE::CString CMcpMetaKeys::ProgressToken = "progressToken";
const CORE::CString CMcpMetaKeys::TraceParent = "traceparent";
const CORE::CString CMcpMetaKeys::TraceState = "tracestate";
const CORE::CString CMcpMetaKeys::Baggage = "baggage";
const CORE::CString CMcpMetaKeys::GucefVendorPrefix = "com.vanvelzensoftware.gucef/";
const CORE::CString CMcpMetaKeys::GucefAuthToken = CMcpMetaKeys::GucefVendorPrefix + "authToken";

const CORE::CString CMcpMethods::ServerDiscover = "server/discover";
const CORE::CString CMcpMethods::Initialize = "initialize";
const CORE::CString CMcpMethods::Ping = "ping";
const CORE::CString CMcpMethods::ToolsList = "tools/list";
const CORE::CString CMcpMethods::ToolsCall = "tools/call";
const CORE::CString CMcpMethods::ResourcesList = "resources/list";
const CORE::CString CMcpMethods::ResourcesTemplatesList = "resources/templates/list";
const CORE::CString CMcpMethods::ResourcesRead = "resources/read";
const CORE::CString CMcpMethods::ResourcesSubscribe = "resources/subscribe";
const CORE::CString CMcpMethods::ResourcesUnsubscribe = "resources/unsubscribe";
const CORE::CString CMcpMethods::PromptsList = "prompts/list";
const CORE::CString CMcpMethods::PromptsGet = "prompts/get";
const CORE::CString CMcpMethods::SubscriptionsListen = "subscriptions/listen";
const CORE::CString CMcpMethods::NotificationInitialized = "notifications/initialized";
const CORE::CString CMcpMethods::NotificationCancelled = "notifications/cancelled";
const CORE::CString CMcpMethods::NotificationProgress = "notifications/progress";
const CORE::CString CMcpMethods::NotificationToolsListChanged = "notifications/tools/list_changed";
const CORE::CString CMcpMethods::NotificationPromptsListChanged = "notifications/prompts/list_changed";
const CORE::CString CMcpMethods::NotificationResourcesListChanged = "notifications/resources/list_changed";
const CORE::CString CMcpMethods::NotificationResourcesUpdated = "notifications/resources/updated";
const CORE::CString CMcpMethods::NotificationSubscriptionsAcknowledged = "notifications/subscriptions/acknowledged";
const CORE::CString CMcpMethods::ElicitationCreate = "elicitation/create";
const CORE::CString CMcpMethods::SamplingCreateMessage = "sampling/createMessage";
const CORE::CString CMcpMethods::RootsList = "roots/list";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
McpRequestOutcomeToString( EMcpRequestOutcome outcome )
{GUCEF_TRACE;

    switch ( outcome )
    {
        case MCP_OUTCOME_COMPLETE: return "complete";
        case MCP_OUTCOME_INPUT_REQUIRED: return "input_required";
        case MCP_OUTCOME_TOOL_ERROR: return "tool_error";
        case MCP_OUTCOME_PROTOCOL_ERROR: return "protocol_error";
        case MCP_OUTCOME_CANCELLED: return "cancelled";
        case MCP_OUTCOME_TIMED_OUT: return "timed_out";
        case MCP_OUTCOME_CLIENT_GONE: return "client_gone";
        case MCP_OUTCOME_REFUSED: return "refused";
        default: return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

const char*
McpCacheScopeToString( EMcpCacheScope scope )
{GUCEF_TRACE;

    return MCP_CACHESCOPE_PUBLIC == scope ? "public" : "private";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMcpProtocolVersions::GetModernVersions( CORE::CString::StringVector& versions )
{GUCEF_TRACE;

    versions.clear();
    versions.push_back( Version_2026_07_28 );
}

/*-------------------------------------------------------------------------*/

void
CMcpProtocolVersions::GetLegacyVersions( CORE::CString::StringVector& versions )
{GUCEF_TRACE;

    versions.clear();
    versions.push_back( Version_2025_11_25 );
}

/*-------------------------------------------------------------------------*/

bool
CMcpProtocolVersions::IsModernVersion( const CORE::CString& version )
{GUCEF_TRACE;

    return version == Version_2026_07_28;
}

/*-------------------------------------------------------------------------*/

bool
CMcpProtocolVersions::IsLegacyVersion( const CORE::CString& version )
{GUCEF_TRACE;

    return version == Version_2025_11_25;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
