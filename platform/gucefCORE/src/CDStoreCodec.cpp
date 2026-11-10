/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "CDStoreCodec.h"       /* definition of the class implemented here */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

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

CDStoreCodec::CDStoreCodec( const CDStoreCodec& src )
    : CICloneable( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDStoreCodec::CDStoreCodec( void )
    : CICloneable()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDStoreCodec::~CDStoreCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDStoreCodec&
CDStoreCodec::operator=( const CDStoreCodec& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodec::StoreDataTree( const CDataNode* tree   ,
                             const CString& filename )
{GUCEF_TRACE;

    CreatePathDirectories( filename );

    CFileAccess access( filename, "wb" );
    if ( access.IsValid() )
    {
        return StoreDataTree( tree, &access );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodec::BuildDataTree( CDataNode* treeroot     ,
                             const CString& filename )

{GUCEF_TRACE;

    CFileAccess access( filename );
    if ( access.IsValid() )
    {
        return BuildDataTree( treeroot, &access );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  Writer used for codecs which cannot write as they go:
 *  the document is gathered in memory and stored when the writer is closed
 */
class CDataTreeGatheringDocumentWriter : public CDocumentStreamWriter
{
    public:

    CDataTreeGatheringDocumentWriter( CDStoreCodec& codec, CIOAccess& output )
        : CDocumentStreamWriter()
        , m_codec( codec )
        , m_output( output )
        , m_document()
        , m_cursor( GUCEF_NULL )
    {GUCEF_TRACE;
    }

    virtual ~CDataTreeGatheringDocumentWriter() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        Close();
    }

    protected:

    virtual bool OnBeginNode( const CDataNode& node, bool willHaveChildren ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CDataNode* target = &m_document;
        if ( GUCEF_NULL != m_cursor )
            target = m_cursor->AddChild();

        target->SetName( node.GetName() );
        target->SetNodeType( node.GetNodeType() );
        if ( node.HasValue() )
            target->SetValue( node.GetValue() );
        UInt32 attCount = node.GetAttCount();
        for ( UInt32 i=0; i<attCount; ++i )
        {
            const CDataNode::TKeyValuePair* att = node.GetAttribute( i );
            target->SetAttribute( att->first, att->second );
        }

        m_cursor = target;
        return true;
    }

    virtual bool OnEndNode( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_cursor )
            m_cursor = m_cursor->GetParent();
        return true;
    }

    virtual bool OnWriteNode( const CDataNode& node ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_cursor )
            m_document.Copy( node );
        else
            m_cursor->AddChild( node );
        return true;
    }

    virtual bool OnClose( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool success = m_codec.StoreDataTree( &m_document, &m_output );
        m_document.Clear();
        return success;
    }

    private:

    CDStoreCodec& m_codec;
    CIOAccess& m_output;
    CDataNode m_document;
    CDataNode* m_cursor;
};

/*-------------------------------------------------------------------------*/

CDocumentStreamWriterPtr
CDStoreCodec::CreateDocumentWriter( CIOAccess& output )
{GUCEF_TRACE;

    return CDocumentStreamWriterPtr( GUCEF_NEW CDataTreeGatheringDocumentWriter( *this, output ) );
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodec::ParseDocument( CIOAccess& input                     ,
                             CIDocumentParserCallbacks& callbacks )
{GUCEF_TRACE;

    CDataNode document;
    if ( !BuildDataTree( &document, &input ) )
    {
        callbacks.OnParseError( 1, "Unable to read the document using codec of type " + GetTypeName() );
        return false;
    }

    ReportDataTree( document, callbacks );
    return true;
}

/*-------------------------------------------------------------------------*/

static void
ReportDataNode( const CDataNode& node, CIDocumentParserCallbacks& callbacks )
{GUCEF_TRACE;

    CVariant nodeId( node.GetName() );
    callbacks.OnNodeBegin( nodeId, node.GetNodeType() );

    UInt32 attCount = node.GetAttCount();
    for ( UInt32 i=0; i<attCount; ++i )
    {
        const CDataNode::TKeyValuePair* att = node.GetAttribute( i );
        callbacks.OnNodeAttribute( nodeId, CVariant( att->first ), att->second );
    }

    if ( node.HasValue() )
        callbacks.OnNodeValue( nodeId, node.GetValue() );

    if ( node.HasChildren() )
    {
        callbacks.OnNodeChildrenBegin( nodeId );
        CDataNode::const_iterator i = node.ConstBegin();
        while ( i != node.ConstEnd() )
        {
            ReportDataNode( *(*i), callbacks );
            ++i;
        }
        callbacks.OnNodeChildrenEnd( nodeId );
    }

    callbacks.OnNodeEnd( nodeId );
}

/*-------------------------------------------------------------------------*/

void
CDStoreCodec::ReportDataTree( const CDataNode& tree                ,
                              CIDocumentParserCallbacks& callbacks )
{GUCEF_TRACE;

    callbacks.OnDocumentBegin();
    ReportDataNode( tree, callbacks );
    callbacks.OnDocumentEnd();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
