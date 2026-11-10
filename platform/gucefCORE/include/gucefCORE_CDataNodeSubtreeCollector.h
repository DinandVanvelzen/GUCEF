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

#ifndef GUCEF_CORE_CDATANODESUBTREECOLLECTOR_H
#define GUCEF_CORE_CDATANODESUBTREECOLLECTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_CORE_CIDOCUMENTPARSERCALLBACKS_H
#include "gucefCORE_CIDocumentParserCallbacks.h"
#define GUCEF_CORE_CIDOCUMENTPARSERCALLBACKS_H
#endif /* GUCEF_CORE_CIDOCUMENTPARSERCALLBACKS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Document parser callbacks which hand over a document one sub-tree at a time.
 *  Each node at the given depth (the root node has depth 0) is gathered together with its sub-tree into a
 *  CDataNode and passed to OnSubtree() once it is complete, after which it is discarded.
 *  This allows reading documents far larger than would fit in memory as a single tree,
 *  as long as the individual sub-trees are of a reasonable size.
 *
 *  For example with depth 2 and node name "person" each <person> in <database><people><person>...
 *  is handed over in turn. The nodes enclosing the current sub-tree are available without their children
 *  via GetAncestors(), which gives access to their attributes.
 *
 *  Use with CDStoreCodec::ParseDocument()
 */
class GUCEF_CORE_PUBLIC_CPP CDataNodeSubtreeCollector : public CIDocumentParserCallbacks
{
    public:

    typedef std::vector< CDataNode > TDataNodeVector;

    /**
     *  @param subtreeDepth depth of the nodes to hand over, the root node has depth 0
     *  @param subtreeNodeName when not empty only nodes at the given depth with this name are handed over,
     *         others are skipped together with their sub-tree
     */
    CDataNodeSubtreeCollector( UInt32 subtreeDepth                                ,
                               const CString& subtreeNodeName = CString::Empty );

    virtual ~CDataNodeSubtreeCollector() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Called for each complete sub-tree. The node is reused afterwards so make a copy if you want to keep it
     */
    virtual void OnSubtree( const CDataNode& subtree ) = 0;

    /**
     *  @return the nodes enclosing the current position without their children, outermost first
     */
    const TDataNodeVector& GetAncestors( void ) const;

    UInt32 GetSubtreeCount( void ) const;

    bool HasParseError( void ) const;

    Int32 GetParseErrorCode( void ) const;

    const CString& GetParseErrorDescription( void ) const;

    virtual void OnDocumentBegin( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnDocumentEnd( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeBegin( const CVariant& nodeId, int nodeType ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeEnd( const CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeAttribute( const CVariant& nodeId, const CVariant& attributeId, const CVariant& attributeValue ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeValue( const CVariant& nodeId, const CVariant& nodeValue ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeChildrenBegin( const CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeChildrenEnd( const CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnParseError( Int32 errorCode, const CString& description ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDataNodeSubtreeCollector( const CDataNodeSubtreeCollector& src );            /**< not supported */
    CDataNodeSubtreeCollector& operator=( const CDataNodeSubtreeCollector& src ); /**< not supported */

    CDataNode* GetCurrentNode( void );

    UInt32 m_subtreeDepth;
    CString m_subtreeNodeName;
    UInt32 m_depth;
    TDataNodeVector m_ancestors;
    CDataNode m_subtree;
    CDataNode* m_cursor;
    bool m_skipping;
    UInt32 m_subtreeCount;
    bool m_hadParseError;
    Int32 m_parseErrorCode;
    CString m_parseErrorDescription;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATANODESUBTREECOLLECTOR_H ? */
