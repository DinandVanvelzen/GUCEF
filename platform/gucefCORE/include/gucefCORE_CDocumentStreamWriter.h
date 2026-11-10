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

#ifndef GUCEF_CORE_CDOCUMENTSTREAMWRITER_H
#define GUCEF_CORE_CDOCUMENTSTREAMWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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
 *  Writes a document node by node instead of from a complete CDataNode tree held in memory.
 *  This is the writing counterpart of CIDocumentParserCallbacks and allows documents of any size
 *  to be stored with a dstore codec. Obtain one from CDStoreCodec::CreateDocumentWriter().
 *
 *  A document has a single root node. A node is opened with BeginNode() which writes the node itself
 *  (its name, type, value and attributes but not its children) after which its children follow and
 *  EndNode() closes it again. A complete sub-tree can be written in one go with WriteNode(), which makes it
 *  easy to build one record at a time in memory, write it and discard it.
 *
 *  Codecs whose plugin supports it write straight to the output as you go. Other codecs gather
 *  the document in memory and store it when the writer is closed, so the result is the same but without
 *  the memory saving.
 *
 *  Misuse such as ending more nodes than were begun, writing children for a node that was begun without
 *  children or writing a second root node is logged as an error and the call returns false.
 *  Closing the writer ends any nodes that are still open. A writer is not thread-safe and the codec it came
 *  from must outlive it.
 */
class GUCEF_CORE_PUBLIC_CPP CDocumentStreamWriter
{
    public:

    virtual ~CDocumentStreamWriter();

    /**
     *  Writes the given node without its children and opens it so that children can follow
     *
     *  @param node the node to write: its name, type, value and attributes. Any children it has are ignored
     *  @param willHaveChildren whether children will be written before the node is ended.
     *         Some formats need to know this up front. Declaring children and then writing none is allowed.
     */
    bool BeginNode( const CDataNode& node, bool willHaveChildren );

    /**
     *  Ends the node most recently begun
     */
    bool EndNode( void );

    /**
     *  Writes the given node including its entire sub-tree
     */
    bool WriteNode( const CDataNode& node );

    /**
     *  Ends any nodes still open and finishes the document. Further writes fail.
     *  Calling it more than once is harmless.
     */
    bool Close( void );

    bool IsOpen( void ) const;

    /**
     *  @return the number of nodes currently begun but not yet ended
     */
    UInt32 GetDepth( void ) const;

    protected:

    CDocumentStreamWriter( void );

    virtual bool OnBeginNode( const CDataNode& node, bool willHaveChildren ) = 0;

    virtual bool OnEndNode( void ) = 0;

    /**
     *  Writes a complete sub-tree. The default implementation uses OnBeginNode() and OnEndNode()
     */
    virtual bool OnWriteNode( const CDataNode& node );

    /**
     *  Finishes the document, all nodes have been ended at this point.
     *  Derived classes have to call Close() from their destructor since it cannot be done from this base class
     */
    virtual bool OnClose( void ) = 0;

    private:

    CDocumentStreamWriter( const CDocumentStreamWriter& src );            /**< not supported */
    CDocumentStreamWriter& operator=( const CDocumentStreamWriter& src ); /**< not supported */

    bool CanAddNode( void ) const;

    std::vector< bool > m_openNodesWithChildren;
    bool m_rootWritten;
    bool m_closed;
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CDocumentStreamWriter, MT::CMutex > CDocumentStreamWriterPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDOCUMENTSTREAMWRITER_H ? */
