/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#include "gucefCORE_CDataNodeSubtreeCollector.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataNodeSubtreeCollector::CDataNodeSubtreeCollector( UInt32 subtreeDepth            ,
                                                      const CString& subtreeNodeName )
    : CIDocumentParserCallbacks()
    , m_subtreeDepth( subtreeDepth )
    , m_subtreeNodeName( subtreeNodeName )
    , m_depth( 0 )
    , m_ancestors()
    , m_subtree()
    , m_cursor( GUCEF_NULL )
    , m_skipping( false )
    , m_subtreeCount( 0 )
    , m_hadParseError( false )
    , m_parseErrorCode( 0 )
    , m_parseErrorDescription()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataNodeSubtreeCollector::~CDataNodeSubtreeCollector()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CDataNodeSubtreeCollector::TDataNodeVector&
CDataNodeSubtreeCollector::GetAncestors( void ) const
{GUCEF_TRACE;

    return m_ancestors;
}

/*-------------------------------------------------------------------------*/

UInt32
CDataNodeSubtreeCollector::GetSubtreeCount( void ) const
{GUCEF_TRACE;

    return m_subtreeCount;
}

/*-------------------------------------------------------------------------*/

bool
CDataNodeSubtreeCollector::HasParseError( void ) const
{GUCEF_TRACE;

    return m_hadParseError;
}

/*-------------------------------------------------------------------------*/

Int32
CDataNodeSubtreeCollector::GetParseErrorCode( void ) const
{GUCEF_TRACE;

    return m_parseErrorCode;
}

/*-------------------------------------------------------------------------*/

const CString&
CDataNodeSubtreeCollector::GetParseErrorDescription( void ) const
{GUCEF_TRACE;

    return m_parseErrorDescription;
}

/*-------------------------------------------------------------------------*/

void
CDataNodeSubtreeCollector::OnDocumentBegin( void )
{GUCEF_TRACE;

    m_depth = 0;
    m_ancestors.clear();
    m_subtree.Clear();
    m_cursor = GUCEF_NULL;
    m_skipping = false;
    m_subtreeCount = 0;
    m_hadParseError = false;
    m_parseErrorCode = 0;
    m_parseErrorDescription.Clear();
}

/*-------------------------------------------------------------------------*/

void
CDataNodeSubtreeCollector::OnDocumentEnd( void )
{GUCEF_TRACE;

    m_subtree.Clear();
    m_cursor = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDataNode*
CDataNodeSubtreeCollector::GetCurrentNode( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_cursor )
        return m_cursor;
    if ( m_skipping || m_ancestors.empty() )
        return GUCEF_NULL;
    return &m_ancestors.back();
}

/*-------------------------------------------------------------------------*/

void
CDataNodeSubtreeCollector::OnNodeBegin( const CVariant& nodeId, int nodeType )
{GUCEF_TRACE;

    UInt32 nodeDepth = m_depth;
    ++m_depth;

    if ( m_skipping )
        return;

    if ( GUCEF_NULL != m_cursor )
    {
        m_cursor = m_cursor->AddChild( nodeId.AsString(), nodeType );
        return;
    }

    if ( nodeDepth < m_subtreeDepth )
    {
        m_ancestors.push_back( CDataNode( nodeType ) );
        m_ancestors.back().SetName( nodeId.AsString() );
        return;
    }

    // a node at the depth of interest
    CString nodeName = nodeId.AsString();
    if ( !m_subtreeNodeName.IsNULLOrEmpty() && nodeName != m_subtreeNodeName )
    {
        m_skipping = true;
        return;
    }

    m_subtree.Clear();
    m_subtree.SetName( nodeName );
    m_subtree.SetNodeType( nodeType );
    m_cursor = &m_subtree;
}

/*-------------------------------------------------------------------------*/

void
CDataNodeSubtreeCollector::OnNodeEnd( const CVariant& nodeId )
{GUCEF_TRACE;

    if ( 0 == m_depth )
        return;
    --m_depth;
    UInt32 nodeDepth = m_depth;

    if ( nodeDepth < m_subtreeDepth )
    {
        if ( !m_ancestors.empty() )
            m_ancestors.pop_back();
        return;
    }

    if ( m_skipping )
    {
        if ( nodeDepth == m_subtreeDepth )
            m_skipping = false;
        return;
    }

    if ( GUCEF_NULL != m_cursor )
    {
        if ( nodeDepth == m_subtreeDepth )
        {
            m_cursor = GUCEF_NULL;
            ++m_subtreeCount;
            OnSubtree( m_subtree );
        }
        else
        {
            m_cursor = m_cursor->GetParent();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDataNodeSubtreeCollector::OnNodeAttribute( const CVariant& nodeId         ,
                                            const CVariant& attributeId    ,
                                            const CVariant& attributeValue )
{GUCEF_TRACE;

    CDataNode* node = GetCurrentNode();
    if ( GUCEF_NULL != node )
        node->SetAttribute( attributeId.AsString(), attributeValue );
}

/*-------------------------------------------------------------------------*/

void
CDataNodeSubtreeCollector::OnNodeValue( const CVariant& nodeId    ,
                                        const CVariant& nodeValue )
{GUCEF_TRACE;

    CDataNode* node = GetCurrentNode();
    if ( GUCEF_NULL != node )
    {
        int nodeType = node->GetNodeType();
        if ( GUCEF_DATATYPE_ARRAY == nodeType || GUCEF_DATATYPE_SET == nodeType )
            node->AddValueAsChild( nodeValue );
        else
            node->SetValue( nodeValue );
    }
}

/*-------------------------------------------------------------------------*/

void
CDataNodeSubtreeCollector::OnNodeChildrenBegin( const CVariant& nodeId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDataNodeSubtreeCollector::OnNodeChildrenEnd( const CVariant& nodeId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDataNodeSubtreeCollector::OnParseError( Int32 errorCode            ,
                                         const CString& description )
{GUCEF_TRACE;

    m_hadParseError = true;
    m_parseErrorCode = errorCode;
    m_parseErrorDescription = description;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
