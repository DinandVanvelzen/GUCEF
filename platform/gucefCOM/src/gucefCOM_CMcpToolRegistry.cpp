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

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#include "gucefCOM_CMcpToolRegistry.h"

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

const CORE::CEvent CMcpToolRegistry::ToolRegisteredEvent = "GUCEF::COM::CMcpToolRegistry::ToolRegisteredEvent";
const CORE::CEvent CMcpToolRegistry::ToolUnregisteredEvent = "GUCEF::COM::CMcpToolRegistry::ToolUnregisteredEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const HeaderKeyword = "x-mcp-header";

/** tchar of RFC 9110 section 5.6.2 */
bool
IsTokenChar( unsigned char c )
{GUCEF_TRACE;

    if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) )
        return true;
    switch ( c )
    {
        case '!': case '#': case '$': case '%': case '&': case '\'': case '*':
        case '+': case '-': case '.': case '^': case '_': case '`': case '|': case '~':
            return true;
        default:
            return false;
    }
}

bool
IsToken( const CORE::CString& text )
{GUCEF_TRACE;

    const char* data = text.C_String();
    UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    if ( 0 == size )
        return false;
    for ( UInt32 i=0; i<size; ++i )
        if ( !IsTokenChar( (unsigned char) data[ i ] ) )
            return false;
    return true;
}

/** Counts every x-mcp-header member anywhere in the schema */
UInt32
CountHeaderKeywords( const CORE::CDataNode& node )
{GUCEF_TRACE;

    UInt32 count = ( GUCEF_NULL != node.GetAttribute( HeaderKeyword ) || GUCEF_NULL != node.FindChild( HeaderKeyword ) ) ? 1 : 0;
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
        count += CountHeaderKeywords( *(*i) );
    return count;
}

/** Collects the annotations on the properties reachable through "properties" keys only */
bool
CollectReachableHeaders( const CORE::CDataNode& schema                          ,
                         CORE::CString::StringVector& path                      ,
                         CMcpToolEntry::THeaderParameterVector& parameters      ,
                         CORE::CString& error                                   )
{GUCEF_TRACE;

    const CORE::CDataNode* properties = schema.FindChild( "properties" );
    if ( GUCEF_NULL == properties || GUCEF_DATATYPE_OBJECT != properties->GetNodeType() )
        return true;

    for ( CORE::CDataNode::const_iterator i = properties->ConstBegin(); i != properties->ConstEnd(); ++i )
    {
        const CORE::CDataNode& property = *(*i);
        if ( GUCEF_DATATYPE_OBJECT != property.GetNodeType() )
            continue;

        path.push_back( property.GetName() );
        const CORE::CDataNode::TKeyValuePair* header = property.GetAttribute( HeaderKeyword );
        if ( GUCEF_NULL != header || GUCEF_NULL != property.FindChild( HeaderKeyword ) )
        {
            if ( GUCEF_NULL == header || !(*header).second.IsString() || !IsToken( (*header).second.AsString() ) )
            {
                error = "x-mcp-header of property \"" + property.GetName() + "\" must be a non-empty HTTP field-name token";
                return false;
            }
            const CORE::CDataNode::TKeyValuePair* type = property.GetAttribute( "type" );
            CORE::CString typeName = ( GUCEF_NULL != type && (*type).second.IsString() ) ? (*type).second.AsString() : CORE::CString::Empty;
            if ( typeName != "string" && typeName != "integer" && typeName != "boolean" )
            {
                error = "x-mcp-header of property \"" + property.GetName() + "\" is only allowed on a property of type string, integer or boolean";
                return false;
            }
            CORE::CString headerName = (*header).second.AsString();
            for ( size_t n=0; n<parameters.size(); ++n )
            {
                if ( parameters[ n ].headerName.Equals( headerName, false ) )
                {
                    error = "x-mcp-header \"" + headerName + "\" is used more than once (case-insensitively)";
                    return false;
                }
            }
            CMcpToolEntry::SHeaderParameter parameter;
            parameter.headerName = headerName;
            parameter.propertyPath = path;
            parameter.type = typeName;
            parameters.push_back( parameter );
        }
        if ( !CollectReachableHeaders( property, path, parameters, error ) )
            return false;
        path.pop_back();
    }
    return true;
}

bool
SchemaRequiresProperties( const CORE::CDataNode& schema )
{GUCEF_TRACE;

    const CORE::CDataNode* required = schema.FindChild( "required" );
    return GUCEF_NULL != required && GUCEF_DATATYPE_ARRAY == required->GetNodeType() && required->HasChildren();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpToolEntry::CMcpToolEntry( void )
    : tool()
    , descriptor()
    , inputSchema()
    , outputSchema()
    , hasOutputSchema( false )
    , inputSchemaRequiresProperties( false )
    , headerParameters()
    , descriptorNode()
    , generation( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMcpToolRegistry::RegisterEvents( void )
{GUCEF_TRACE;

    CMcpRegistryBase::RegisterEvents();
    ToolRegisteredEvent.Initialize();
    ToolUnregisteredEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMcpToolRegistry::CMcpToolRegistry( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CMcpRegistryBase( pulseGenerator )
    , m_entries()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CMcpToolRegistry::~CMcpToolRegistry()
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwLock );
    m_entries.clear();
}

/*-------------------------------------------------------------------------*/

bool
CMcpToolRegistry::IsValidToolName( const CORE::CString& name, CORE::CString& error )
{GUCEF_TRACE;

    const char* data = name.C_String();
    UInt32 size = name.ByteSize() > 0 ? name.ByteSize() - 1 : 0;
    if ( 0 == size || size > MaxToolNameLength )
    {
        error = "a tool name must be 1 to 128 characters long";
        return false;
    }
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = data[ i ];
        if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '_' == c || '-' == c || '.' == c ) )
        {
            error = "a tool name may only contain A-Z, a-z, 0-9, '_', '-' and '.'";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpToolRegistry::CheckHeaderAnnotations( const CORE::CDataNode& inputSchema                    ,
                                          CMcpToolEntry::THeaderParameterVector& parameters     ,
                                          CORE::CString& error                                  )
{GUCEF_TRACE;

    parameters.clear();
    CORE::CString::StringVector path;
    if ( !CollectReachableHeaders( inputSchema, path, parameters, error ) )
        return false;
    if ( CountHeaderKeywords( inputSchema ) != parameters.size() )
    {
        error = "x-mcp-header is only allowed on properties reachable from the root through \"properties\" keys";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpToolRegistry::Register( const CIMcpToolPtr& tool, CORE::CString& error )
{GUCEF_TRACE;

    error.Clear();
    if ( tool.IsNULL() )
    {
        error = "no tool";
        return false;
    }

    CMcpToolEntryPtr entry( GUCEF_NEW CMcpToolEntry() );
    entry->tool = tool;
    if ( !tool->GetDescriptor( entry->descriptor ) )
    {
        error = "the tool gave no descriptor";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpToolRegistry: A tool was refused: " << error );
        return false;
    }

    const CMcpToolDescriptor& descriptor = entry->descriptor;
    bool ok = IsValidToolName( descriptor.name, error );
    if ( ok )
    {
        const CORE::CDataNode::TKeyValuePair* type = descriptor.inputSchema.GetAttribute( "type" );
        if ( GUCEF_DATATYPE_OBJECT != descriptor.inputSchema.GetNodeType() || GUCEF_NULL == type || (*type).second.AsString() != "object" )
        {
            error = "the input schema must be an object schema with \"type\": \"object\"";
            ok = false;
        }
    }
    if ( ok )
    {
        CORE::CString schemaError;
        if ( !entry->inputSchema.Compile( descriptor.inputSchema, schemaError ) )
        {
            error = "the input schema does not compile: " + schemaError;
            ok = false;
        }
    }
    if ( ok && descriptor.hasOutputSchema )
    {
        CORE::CString schemaError;
        if ( !entry->outputSchema.Compile( descriptor.outputSchema, schemaError ) )
        {
            error = "the output schema does not compile: " + schemaError;
            ok = false;
        }
        entry->hasOutputSchema = ok;
    }
    if ( ok )
        ok = CheckHeaderAnnotations( descriptor.inputSchema, entry->headerParameters, error );
    if ( !ok )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpToolRegistry: The tool \"" << descriptor.name << "\" was refused: " << error );
        return false;
    }

    entry->inputSchemaRequiresProperties = SchemaRequiresProperties( descriptor.inputSchema );
    descriptor.ToDataNode( entry->descriptorNode );

    {
        MT::CScopeWriterLock lock( m_rwLock );
        TToolEntryMap::iterator existing = m_entries.find( descriptor.name );
        if ( existing != m_entries.end() && (*existing).second->tool != tool )
        {
            error = "another tool is registered with the name \"" + descriptor.name + "\"";
            lock.EarlyUnlock();
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpToolRegistry: The tool \"" << descriptor.name << "\" was refused: " << error );
            return false;
        }
        entry->generation = OnChangedWhileLocked();
        m_entries[ descriptor.name ] = entry;
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "McpToolRegistry: Registered the tool \"" << descriptor.name << "\"" );
    RaiseNameEvent( ToolRegisteredEvent, descriptor.name );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpToolRegistry::Unregister( const CORE::CString& name )
{GUCEF_TRACE;

    {
        MT::CScopeWriterLock lock( m_rwLock );
        TToolEntryMap::iterator i = m_entries.find( name );
        if ( i == m_entries.end() )
            return false;
        m_entries.erase( i );
        OnChangedWhileLocked();
    }

    RaiseNameEvent( ToolUnregisteredEvent, name );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpToolRegistry::UnregisterAll( void )
{GUCEF_TRACE;

    TToolEntryMap removed;
    {
        MT::CScopeWriterLock lock( m_rwLock );
        if ( m_entries.empty() )
            return;
        removed.swap( m_entries );
        OnChangedWhileLocked();
    }

    for ( TToolEntryMap::iterator i = removed.begin(); i != removed.end(); ++i )
        RaiseNameEvent( ToolUnregisteredEvent, (*i).first );
}

/*-------------------------------------------------------------------------*/

bool
CMcpToolRegistry::TryGet( const CORE::CString& name, CMcpToolEntryPtr& entry ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwLock );
    TToolEntryMap::const_iterator i = m_entries.find( name );
    if ( i == m_entries.end() )
        return false;
    entry = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpToolRegistry::GetEntries( TToolEntryVector& entries ) const
{GUCEF_TRACE;

    entries.clear();
    MT::CScopeReaderLock lock( m_rwLock );
    entries.reserve( m_entries.size() );
    for ( TToolEntryMap::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i )
        entries.push_back( (*i).second );
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpToolRegistry::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwLock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpToolRegistry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpToolRegistry";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
