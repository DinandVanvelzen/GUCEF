/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CMCPCALLSCOPE_H
#include "gucefCOM_CMcpCallScope.h"
#define GUCEF_COM_CMCPCALLSCOPE_H
#endif /* GUCEF_COM_CMCPCALLSCOPE_H ? */

#ifndef GUCEF_COM_CMCPSCHEMABUILDER_H
#include "gucefCOM_CMcpSchemaBuilder.h"
#define GUCEF_COM_CMCPSCHEMABUILDER_H
#endif /* GUCEF_COM_CMCPSCHEMABUILDER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLCATALOGUE_H
#include "gucefASSETSTORE_CAssetToolCatalogue.h"
#define GUCEF_ASSETSTORE_CASSETTOOLCATALOGUE_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLCATALOGUE_H ? */

#include "gucefASSETSTORE_CAssetToolBase.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetToolBase::SCallState::SCallState( void )
    : tags()
    , finished( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolBase::CAssetToolBase( CAssetToolContext& context, const CORE::CString& name )
    : COM::CIMcpTool()
    , m_context( context )
    , m_name( name )
    , m_hasVocabulary( false )
    , m_vocabulary()
    , m_vocabularyLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolBase::~CAssetToolBase()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetToolBase::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolBase::SetVocabulary( const CAssetVocabularyInfo& vocabulary )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_vocabularyLock );
    m_vocabulary = vocabulary;
    m_hasVocabulary = true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolBase::GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_vocabularyLock );
    return CAssetToolCatalogue::BuildDescriptor( m_name, m_hasVocabulary ? &m_vocabulary : GUCEF_NULL, descriptor );
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecutionMode
CAssetToolBase::GetExecutionMode( void ) const
{GUCEF_TRACE;

    // the service may block (disk, database, network) and the VFS must not be used on the protocol thread
    return COM::MCP_EXECUTION_THREAD_POOL;
}

/*-------------------------------------------------------------------------*/

COM::EMcpExecuteResult
CAssetToolBase::Execute( COM::CMcpRequestContext& context      ,
                         const CORE::CDataNode& arguments      ,
                         COM::CMcpToolResult& result           ,
                         COM::CMcpInputRequired& inputRequired )
{GUCEF_TRACE;

    COM::CMcpCallScope scope( m_context.GetCallGate(), context );
    if ( !scope.IsEntered() )
    {
        m_context.SetError( result, "backend-unavailable", "The asset tools are being stopped; try again later", true );
        return COM::MCP_EXECUTE_COMPLETE;
    }
    SCallState state;
    COM::EMcpExecuteResult outcome = COM::MCP_EXECUTE_COMPLETE;
    try
    {
        outcome = Compute( context, arguments, result, inputRequired, state );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolBase: An exception ended the call of the tool " << m_name );
        m_context.SetError( result, "failed", "The tool failed unexpectedly", false );
        state.tags.clear();
        state.finished = false;
        outcome = COM::MCP_EXECUTE_COMPLETE;
    }
    if ( COM::MCP_EXECUTE_COMPLETE == outcome && !state.finished )
        m_context.FinishResult( result, state.tags );
    return outcome;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolBase::SetStructured( COM::CMcpToolResult& result, const CORE::CDataNode& document )
{GUCEF_TRACE;

    result.SetStructuredContent( document, false );
    if ( GUCEF_DATATYPE_OBJECT != result.structuredContent.GetNodeType() )
        result.structuredContent.SetNodeType( GUCEF_DATATYPE_OBJECT );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolBase::AddResourceLinks( const CORE::CDataNode& document, COM::CMcpToolResult& result )
{GUCEF_TRACE;

    const CORE::CDataNode* links = document.FindChild( "links" );
    if ( GUCEF_NULL == links )
        return;
    CORE::CDataNode::const_iterator i = links->ConstBegin();
    while ( i != links->ConstEnd() )
    {
        const CORE::CString uri = (*i)->GetAttributeValueOrChildValueByName( "uri" ).AsString( CORE::CString::Empty, true );
        const CORE::CString name = (*i)->GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true );
        const CORE::CString mimeType = (*i)->GetAttributeValueOrChildValueByName( "mimeType" ).AsString( CORE::CString::Empty, true );
        if ( !uri.IsNULLOrEmpty() )
            result.AddContent( COM::CMcpContent::ResourceLink( uri, name.IsNULLOrEmpty() ? uri : name, mimeType ) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode
CAssetToolBase::MakeUntrusted( const CORE::CString& source, const CORE::CString& text ) const
{GUCEF_TRACE;

    return COM::CMcpSchemaBuilder::MakeUntrustedTextValue( source, m_context.GetRedaction().ForResult( text ), m_context.GetUntrustedTextChars() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolBase::GetStringArgument( const CORE::CDataNode& arguments, const CORE::CString& name )
{GUCEF_TRACE;

    return arguments.GetAttributeValueOrChildValueByName( name ).AsString( CORE::CString::Empty, true );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolBase::GetBoolArgument( const CORE::CDataNode& arguments, const CORE::CString& name, bool defaultValue )
{GUCEF_TRACE;

    const CORE::CVariant& value = arguments.GetAttributeValueOrChildValueByName( name );
    return value.IsNULLOrEmpty() ? defaultValue : value.AsBool( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetToolBase::GetUInt32Argument( const CORE::CDataNode& arguments, const CORE::CString& name, UInt32 defaultValue )
{GUCEF_TRACE;

    const CORE::CVariant& value = arguments.GetAttributeValueOrChildValueByName( name );
    return value.IsNULLOrEmpty() ? defaultValue : value.AsUInt32( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

Float64
CAssetToolBase::GetFloat64Argument( const CORE::CDataNode& arguments, const CORE::CString& name, Float64 defaultValue )
{GUCEF_TRACE;

    const CORE::CVariant& value = arguments.GetAttributeValueOrChildValueByName( name );
    return value.IsNULLOrEmpty() ? defaultValue : value.AsFloat64( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolBase::GetStringListArgument( const CORE::CDataNode& arguments, const CORE::CString& name, CORE::CString::StringVector& values )
{GUCEF_TRACE;

    values.clear();
    arguments.GetValuesOfChildByName( name, values );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolBase::GetStringSetArgument( const CORE::CDataNode& arguments, const CORE::CString& name, CORE::CString::StringSet& values )
{GUCEF_TRACE;

    CORE::CString::StringVector list;
    GetStringListArgument( arguments, name, list );
    values.clear();
    for ( size_t i=0; i<list.size(); ++i )
        values.insert( list[ i ] );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolBase::HasArgument( const CORE::CDataNode& arguments, const CORE::CString& name )
{GUCEF_TRACE;

    return GUCEF_NULL != arguments.GetAttribute( name ) || GUCEF_NULL != arguments.FindChild( name );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolBase::SetNull( CORE::CDataNode& object, const CORE::CString& name )
{GUCEF_TRACE;

    object.SetAttribute( name, CORE::CVariant() );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolBase::SetTextOrNull( CORE::CDataNode& object, const CORE::CString& name, const CORE::CString& text )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        SetNull( object, name );
    else
        object.SetAttribute( name, text );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
