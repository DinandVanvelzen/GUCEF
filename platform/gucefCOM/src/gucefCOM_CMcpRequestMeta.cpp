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

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#include "gucefCOM_CMcpRequestMeta.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Reads an optional string member; false when it is there but not a string */
bool
ReadOptionalString( const CORE::CDataNode& meta, const CORE::CString& key, CORE::CString& value, CORE::CString& error )
{GUCEF_TRACE;

    value.Clear();
    const CORE::CDataNode::TKeyValuePair* attribute = meta.GetAttribute( key );
    if ( GUCEF_NULL != attribute )
    {
        if ( !(*attribute).second.IsString() )
        {
            error = "_meta \"" + key + "\" must be a string";
            return false;
        }
        value = (*attribute).second.AsString();
        return true;
    }
    if ( GUCEF_NULL != meta.FindChild( key ) )
    {
        error = "_meta \"" + key + "\" must be a string";
        return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpRequestMeta::CMcpRequestMeta( void )
    : m_protocolVersion()
    , m_clientCapabilities( GUCEF_DATATYPE_OBJECT )
    , m_hasClientInfo( false )
    , m_clientInfo()
    , m_hasProgressToken( false )
    , m_progressToken()
    , m_logLevel()
    , m_traceParent()
    , m_traceState()
    , m_baggage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpRequestMeta::EParseResult
CMcpRequestMeta::Parse( const CORE::CDataNode* meta, CORE::CString& error )
{GUCEF_TRACE;

    *this = CMcpRequestMeta();
    error.Clear();

    if ( GUCEF_NULL == meta )
        return META_NO_PROTOCOL_VERSION;
    if ( GUCEF_DATATYPE_OBJECT != meta->GetNodeType() )
    {
        error = "_meta must be an object";
        return META_MALFORMED;
    }

    const CORE::CDataNode::TKeyValuePair* version = meta->GetAttribute( CMcpMetaKeys::ProtocolVersion );
    if ( GUCEF_NULL == version )
    {
        if ( GUCEF_NULL != meta->FindChild( CMcpMetaKeys::ProtocolVersion ) )
        {
            error = "_meta \"" + CMcpMetaKeys::ProtocolVersion + "\" must be a string";
            return META_MALFORMED;
        }
        return META_NO_PROTOCOL_VERSION;
    }
    if ( !(*version).second.IsString() )
    {
        error = "_meta \"" + CMcpMetaKeys::ProtocolVersion + "\" must be a string";
        return META_MALFORMED;
    }
    m_protocolVersion = (*version).second.AsString();

    const CORE::CDataNode* capabilities = meta->FindChild( CMcpMetaKeys::ClientCapabilities );
    if ( GUCEF_NULL == capabilities || GUCEF_DATATYPE_OBJECT != capabilities->GetNodeType() || capabilities->HasValue() )
    {
        error = ( GUCEF_NULL == capabilities && GUCEF_NULL == meta->GetAttribute( CMcpMetaKeys::ClientCapabilities ) ) ?
                    "_meta \"" + CMcpMetaKeys::ClientCapabilities + "\" is missing" :
                    "_meta \"" + CMcpMetaKeys::ClientCapabilities + "\" must be an object";
        return META_MALFORMED;
    }
    m_clientCapabilities = *capabilities;
    m_clientCapabilities.SetName( CORE::CString::Empty );

    const CORE::CDataNode* clientInfo = meta->FindChild( CMcpMetaKeys::ClientInfo );
    if ( GUCEF_NULL != clientInfo || GUCEF_NULL != meta->GetAttribute( CMcpMetaKeys::ClientInfo ) )
    {
        if ( GUCEF_NULL == clientInfo || !m_clientInfo.FromDataNode( *clientInfo ) )
        {
            error = "_meta \"" + CMcpMetaKeys::ClientInfo + "\" must be an Implementation object with a name and a version";
            return META_MALFORMED;
        }
        m_hasClientInfo = true;
    }

    const CORE::CDataNode::TKeyValuePair* token = meta->GetAttribute( CMcpMetaKeys::ProgressToken );
    if ( GUCEF_NULL != token )
    {
        if ( !(*token).second.IsString() && !(*token).second.IsInteger() )
        {
            error = "_meta \"progressToken\" must be a string or an integer";
            return META_MALFORMED;
        }
        m_progressToken = (*token).second;
        m_hasProgressToken = true;
    }
    else
    if ( GUCEF_NULL != meta->FindChild( CMcpMetaKeys::ProgressToken ) )
    {
        error = "_meta \"progressToken\" must be a string or an integer";
        return META_MALFORMED;
    }

    if ( !ReadOptionalString( *meta, CMcpMetaKeys::LogLevel, m_logLevel, error ) ||
         !ReadOptionalString( *meta, CMcpMetaKeys::TraceParent, m_traceParent, error ) ||
         !ReadOptionalString( *meta, CMcpMetaKeys::TraceState, m_traceState, error ) ||
         !ReadOptionalString( *meta, CMcpMetaKeys::Baggage, m_baggage, error ) )
        return META_MALFORMED;

    return META_OK;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestMeta::SetForLegacy( const CORE::CString& protocolVersion, const CORE::CDataNode& clientCapabilities, const CMcpImplementationInfo* clientInfo )
{GUCEF_TRACE;

    *this = CMcpRequestMeta();
    m_protocolVersion = protocolVersion;
    if ( GUCEF_DATATYPE_OBJECT == clientCapabilities.GetNodeType() )
    {
        m_clientCapabilities = clientCapabilities;
        m_clientCapabilities.SetName( CORE::CString::Empty );
    }
    if ( GUCEF_NULL != clientInfo )
    {
        m_clientInfo = *clientInfo;
        m_hasClientInfo = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestMeta::SetForInProcess( void )
{GUCEF_TRACE;

    *this = CMcpRequestMeta();
    m_protocolVersion = CMcpProtocolVersions::Version_2026_07_28;
}

/*-------------------------------------------------------------------------*/

void
CMcpRequestMeta::SetProgressToken( const CORE::CVariant& token )
{GUCEF_TRACE;

    m_hasProgressToken = token.IsString() || token.IsInteger();
    m_progressToken = m_hasProgressToken ? token : CORE::CVariant();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestMeta::GetProtocolVersion( void ) const
{GUCEF_TRACE;

    return m_protocolVersion;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CMcpRequestMeta::GetClientCapabilities( void ) const
{GUCEF_TRACE;

    return m_clientCapabilities;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestMeta::HasClientInfo( void ) const
{GUCEF_TRACE;

    return m_hasClientInfo;
}

/*-------------------------------------------------------------------------*/

const CMcpImplementationInfo&
CMcpRequestMeta::GetClientInfo( void ) const
{GUCEF_TRACE;

    return m_clientInfo;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestMeta::HasProgressToken( void ) const
{GUCEF_TRACE;

    return m_hasProgressToken;
}

/*-------------------------------------------------------------------------*/

const CORE::CVariant&
CMcpRequestMeta::GetProgressToken( void ) const
{GUCEF_TRACE;

    return m_progressToken;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestMeta::GetLogLevel( void ) const
{GUCEF_TRACE;

    return m_logLevel;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestMeta::GetTraceParent( void ) const
{GUCEF_TRACE;

    return m_traceParent;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestMeta::GetTraceState( void ) const
{GUCEF_TRACE;

    return m_traceState;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRequestMeta::GetBaggage( void ) const
{GUCEF_TRACE;

    return m_baggage;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestMeta::HasCapability( const CORE::CString& capabilityPath ) const
{GUCEF_TRACE;

    return HasCapability( m_clientCapabilities, capabilityPath );
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestMeta::HasCapability( const CORE::CDataNode& capabilities, const CORE::CString& capabilityPath )
{GUCEF_TRACE;

    CORE::CString::StringVector names = capabilityPath.ParseElements( '/', false );
    if ( names.empty() )
        return false;

    const CORE::CDataNode* node = &capabilities;
    for ( size_t i=0; i<names.size(); ++i )
    {
        const CORE::CDataNode* child = node->FindChild( names[ i ] );
        if ( GUCEF_NULL == child || GUCEF_DATATYPE_OBJECT != child->GetNodeType() )
        {
            // { "elicitation": {} } is the same as { "elicitation": { "form": {} } }
            if ( 1 == i && 2 == names.size() && names[ 0 ] == "elicitation" && names[ 1 ] == "form" &&
                 GUCEF_NULL == node->FindChild( "url" ) && GUCEF_NULL == node->GetAttribute( "url" ) &&
                 GUCEF_NULL == node->GetAttribute( "form" ) )
                return true;
            return false;
        }
        node = child;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRequestMeta::HasAllCapabilities( const CORE::CDataNode& capabilities, const CORE::CDataNode& required )
{GUCEF_TRACE;

    for ( CORE::CDataNode::const_iterator i = required.ConstBegin(); i != required.ConstEnd(); ++i )
    {
        const CORE::CDataNode* top = (*i);
        if ( !top->HasChildren() )
        {
            CORE::CString path = top->GetName();
            if ( path == "elicitation" )
                path = "elicitation/form";      // an empty elicitation object asks for form mode
            if ( !HasCapability( capabilities, path ) )
                return false;
            continue;
        }
        for ( CORE::CDataNode::const_iterator n = top->ConstBegin(); n != top->ConstEnd(); ++n )
        {
            if ( !HasCapability( capabilities, top->GetName() + "/" + (*n)->GetName() ) )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
