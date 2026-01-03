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

#ifndef GUCEF_CORE_CDATANODEDOCUMENTBUILDER_H
#define GUCEF_CORE_CDATANODEDOCUMENTBUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  Implementation which uses document parser callbacks to construct a DataNode class based tree
 */
class GUCEF_CORE_PUBLIC_CPP CDataNodeDocumentBuilder : public CIDocumentParserCallbacks
{
    public:

    CDataNodeDocumentBuilder( void );
    CDataNodeDocumentBuilder( const CDataNodeDocumentBuilder& src ); 
    virtual ~CDataNodeDocumentBuilder() GUCEF_VIRTUAL_OVERRIDE;
    CDataNodeDocumentBuilder& operator=( const CDataNodeDocumentBuilder& src );

    virtual void OnDocumentBegin( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnDocumentEnd( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeBegin( const CVariant& nodeId, int nodeType ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeEnd( const CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeAttribute( const CVariant& nodeId, const CVariant& attributeId, const CVariant& attributeValue ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeValue( const CVariant& nodeId, const CVariant& nodeValue ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeChildrenBegin( const CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeChildrenEnd( const CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnParseError( Int32 errorCode, const CString& description ) GUCEF_VIRTUAL_OVERRIDE;

    CDataNode document;

    private:

    CDataNode* m_nodeCursor;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATANODEDOCUMENTBUILDER_H ? */

/*-------------------------------------------------------------------------*/
