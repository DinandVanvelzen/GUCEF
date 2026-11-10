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

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#include "gucefCOM_CMcpAuditRecord.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpAuditRecord::CMcpAuditRecord( void )
    : timestampUtc()
    , durationInMs( 0 )
    , transport()
    , channelId( 0 )
    , era( MCP_ERA_MODERN )
    , protocolVersion()
    , requestId()
    , subject()
    , isAuthenticated( false )
    , scopes()
    , clientName()
    , clientVersion()
    , method()
    , target()
    , hasDecision( false )
    , decision( MCP_ACCESS_ALLOW )
    , decisionReason()
    , outcome( MCP_OUTCOME_COMPLETE )
    , errorCode( 0 )
    , refusalReason()
    , hasArguments( false )
    , argumentsDigest()
    , argumentsPreview()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const char*
CMcpAuditRecord::DecisionToString( EMcpAccessDecision decision )
{GUCEF_TRACE;

    switch ( decision )
    {
        case MCP_ACCESS_ALLOW:  return "allow";
        case MCP_ACCESS_DENY:   return "deny";
        case MCP_ACCESS_HIDE:   return "hide";
        default:                return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

const char*
CMcpAuditRecord::EraToString( EMcpProtocolEra era )
{GUCEF_TRACE;

    return MCP_ERA_LEGACY == era ? "legacy" : "modern";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMcpAuditRecord::DigestToHex( const UInt8* digest )
{GUCEF_TRACE;

    return CORE::Base16Encode( digest, 32 ).Lowercase();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMcpAuditRecord::MakeArgumentsPreview( const CORE::CDataNode* arguments, const CORE::CString::StringSet& secretArguments )
{GUCEF_TRACE;

    if ( GUCEF_NULL == arguments )
        return CORE::CString::Empty;

    // the secrets first: by name, whatever their value looks like
    CORE::CDataNode masked( *arguments );
    if ( !secretArguments.empty() )
    {
        for ( CORE::CString::StringSet::const_iterator i = secretArguments.begin(); i != secretArguments.end(); ++i )
        {
            if ( masked.IsAttribute( *i ) )
            {
                masked.SetAttribute( *i, CORE::CVariant( CORE::CString( GUCEF_CORE_REDACTION_MARKER ) ) );
            }
            else
            {
                CORE::CDataNode* child = masked.FindChild( *i );
                if ( GUCEF_NULL != child )
                {
                    // a structured secret is replaced whole by a marker
                    masked.DelChild( *i );
                    masked.SetAttribute( *i, CORE::CVariant( CORE::CString( GUCEF_CORE_REDACTION_MARKER ) ) );
                }
            }
        }
    }

    CORE::CString jsonText;
    if ( !CJsonRpcEndpoint::ToJsonText( masked, jsonText ) )
        return CORE::CString::Empty;

    CORE::CRedactor redactor( CORE::CRedactor::ForLogLine() );
    redactor.SetMaxLength( MaxArgumentsPreviewBytes );
    return redactor.RedactText( jsonText );
}

/*-------------------------------------------------------------------------*/

bool
CMcpAuditRecord::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "time", CORE::CVariant( timestampUtc.ToIso8601DateTimeString( true, true ) ) );
    node.SetAttribute( "durationMs", durationInMs );
    node.SetAttribute( "transport", CORE::CVariant( transport ) );
    node.SetAttribute( "channelId", channelId );
    node.SetAttribute( "era", CORE::CVariant( CORE::CString( EraToString( era ) ) ) );
    node.SetAttribute( "protocolVersion", CORE::CVariant( protocolVersion ) );
    node.SetAttribute( "requestId", CORE::CVariant( requestId ) );
    node.SetAttribute( "subject", CORE::CVariant( subject ) );
    node.SetAttribute( "authenticated", isAuthenticated );
    node.SetAttribute( "clientName", CORE::CVariant( clientName ) );
    node.SetAttribute( "clientVersion", CORE::CVariant( clientVersion ) );
    node.SetAttribute( "clientInfoVerified", false );
    node.SetAttribute( "method", CORE::CVariant( method ) );
    node.SetAttribute( "target", CORE::CVariant( target ) );
    if ( hasDecision )
    {
        node.SetAttribute( "decision", CORE::CVariant( CORE::CString( DecisionToString( decision ) ) ) );
        node.SetAttribute( "decisionReason", CORE::CVariant( decisionReason ) );
    }
    node.SetAttribute( "outcome", CORE::CVariant( CORE::CString( McpRequestOutcomeToString( outcome ) ) ) );
    node.SetAttribute( "errorCode", (Int32) errorCode );
    if ( MCP_OUTCOME_REFUSED == outcome || !refusalReason.IsNULLOrEmpty() )
        node.SetAttribute( "reason", CORE::CVariant( refusalReason ) );
    if ( hasArguments )
    {
        node.SetAttribute( "argumentsDigest", CORE::CVariant( argumentsDigest ) );
        node.SetAttribute( "argumentsPreview", CORE::CVariant( argumentsPreview ) );
    }
    node.SetValuesOfChildByName( "scopes", scopes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpAuditRecord::ToJsonLine( CORE::CString& line ) const
{GUCEF_TRACE;

    CORE::CDataNode node;
    if ( !ToDataNode( node ) )
        return false;
    return CJsonRpcEndpoint::ToJsonText( node, line );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
