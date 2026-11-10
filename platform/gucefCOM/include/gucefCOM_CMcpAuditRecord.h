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

#ifndef GUCEF_COM_CMCPAUDITRECORD_H
#define GUCEF_COM_CMCPAUDITRECORD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

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

/**
 *  What the server tells its audit subscribers about one request, when the request has ended. A record never holds
 *  the arguments of a request: only their SHA-256 digest and a short preview in which secrets are masked and redacted.
 *  The server raises one record per request, so a denial is the record of the request that was denied, with the
 *  decision and the reason of the access policy in it.
 */
class GUCEF_COM_PUBLIC_CPP CMcpAuditRecord
{
    public:

    /** The largest number of bytes of an argument preview */
    static const UInt32 MaxArgumentsPreviewBytes = 512;

    // when
    CORE::CDateTime timestampUtc;               /**< when the request ended, UTC */
    UInt64 durationInMs;                        /**< from the monotonic clock */

    // where
    CORE::CString transport;                    /**< "stdio", "tcp", "streamable-http", "in-process" */
    UInt64 channelId;                           /**< 0 for in-process calls */
    EMcpProtocolEra era;
    CORE::CString protocolVersion;
    CORE::CString requestId;                    /**< as text; empty for in-process calls */

    // who
    CORE::CString subject;
    bool isAuthenticated;
    CORE::CString::StringVector scopes;
    CORE::CString clientName;                   /**< as the client reported it in _meta: not verified, never used for a decision */
    CORE::CString clientVersion;                /**< idem */

    // what
    CORE::CString method;
    CORE::CString target;                       /**< tool or prompt name, resource URI through CRedactor::ForUrl() */
    bool hasDecision;                           /**< the access policy was asked */
    EMcpAccessDecision decision;
    CORE::CString decisionReason;
    EMcpRequestOutcome outcome;
    Int32 errorCode;                            /**< for MCP_OUTCOME_PROTOCOL_ERROR, else 0 */
    CORE::CString refusalReason;                /**< for MCP_OUTCOME_REFUSED: why the transport refused the request, e.g. "HTTP 401: no valid credentials" */

    // arguments
    bool hasArguments;                          /**< the request had arguments (a tool call or a prompt get) */
    CORE::CString argumentsDigest;              /**< SHA-256 of the JSON text of the arguments, in hexadecimal; empty without arguments */
    CORE::CString argumentsPreview;             /**< the redacted text of the arguments, at most MaxArgumentsPreviewBytes bytes */

    CMcpAuditRecord( void );

    /** Writes the record as a JSON object node. The names of the members are those of the JSON lines of CMcpJsonLinesAuditLog */
    bool ToDataNode( CORE::CDataNode& node ) const;

    /** The record as one line of JSON text without a line end: control characters are escaped, so it never holds a '\n' */
    bool ToJsonLine( CORE::CString& line ) const;

    static const char* DecisionToString( EMcpAccessDecision decision );

    static const char* EraToString( EMcpProtocolEra era );

    /**
     *  Makes the text for the field argumentsPreview from the JSON text of the arguments. The names in 'secretArguments'
     *  are top level members of the arguments: their values are masked first. Then the text goes through
     *  CRedactor::ForLogLine() and is cut to MaxArgumentsPreviewBytes between characters.
     *  @param arguments    the arguments as a node, or GUCEF_NULL for none
     */
    static CORE::CString MakeArgumentsPreview( const CORE::CDataNode* arguments, const CORE::CString::StringSet& secretArguments );

    /** The SHA-256 of 32 bytes as 64 lower case hexadecimal digits */
    static CORE::CString DigestToHex( const UInt8* digest );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPAUDITRECORD_H ? */
