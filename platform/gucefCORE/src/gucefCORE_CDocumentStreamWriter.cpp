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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CDocumentStreamWriter.h"

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

CDocumentStreamWriter::CDocumentStreamWriter( void )
    : m_openNodesWithChildren()
    , m_rootWritten( false )
    , m_closed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDocumentStreamWriter::~CDocumentStreamWriter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDocumentStreamWriter::CanAddNode( void ) const
{GUCEF_TRACE;

    if ( m_closed )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DocumentStreamWriter: Cannot write a node after the writer was closed" );
        return false;
    }
    if ( m_openNodesWithChildren.empty() )
    {
        if ( m_rootWritten )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DocumentStreamWriter: A document has a single root node, a second one cannot be written" );
            return false;
        }
        return true;
    }
    if ( !m_openNodesWithChildren.back() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DocumentStreamWriter: Cannot write a child for a node that was begun without children" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDocumentStreamWriter::BeginNode( const CDataNode& node, bool willHaveChildren )
{GUCEF_TRACE;

    if ( !CanAddNode() )
        return false;

    if ( !OnBeginNode( node, willHaveChildren ) )
        return false;

    m_rootWritten = true;
    m_openNodesWithChildren.push_back( willHaveChildren );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDocumentStreamWriter::EndNode( void )
{GUCEF_TRACE;

    if ( m_closed || m_openNodesWithChildren.empty() )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DocumentStreamWriter: EndNode() called without a matching BeginNode()" );
        return false;
    }

    m_openNodesWithChildren.pop_back();
    return OnEndNode();
}

/*-------------------------------------------------------------------------*/

bool
CDocumentStreamWriter::WriteNode( const CDataNode& node )
{GUCEF_TRACE;

    if ( !CanAddNode() )
        return false;

    m_rootWritten = true;
    return OnWriteNode( node );
}

/*-------------------------------------------------------------------------*/

bool
CDocumentStreamWriter::OnWriteNode( const CDataNode& node )
{GUCEF_TRACE;

    bool hasChildren = node.HasChildren();
    if ( !OnBeginNode( node, hasChildren ) )
        return false;

    bool success = true;
    CDataNode::const_iterator i = node.ConstBegin();
    while ( i != node.ConstEnd() )
    {
        success = OnWriteNode( *(*i) ) && success;
        ++i;
    }
    return OnEndNode() && success;
}

/*-------------------------------------------------------------------------*/

bool
CDocumentStreamWriter::Close( void )
{GUCEF_TRACE;

    if ( m_closed )
        return true;

    bool success = true;
    while ( !m_openNodesWithChildren.empty() )
    {
        m_openNodesWithChildren.pop_back();
        success = OnEndNode() && success;
    }

    m_closed = true;
    return OnClose() && success;
}

/*-------------------------------------------------------------------------*/

bool
CDocumentStreamWriter::IsOpen( void ) const
{GUCEF_TRACE;

    return !m_closed;
}

/*-------------------------------------------------------------------------*/

UInt32
CDocumentStreamWriter::GetDepth( void ) const
{GUCEF_TRACE;

    return (UInt32) m_openNodesWithChildren.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
