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

#ifndef GUCEF_COM_CMCPTOOLRESULT_H
#define GUCEF_COM_CMCPTOOLRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_COM_CMCPCONTENT_H
#include "gucefCOM_CMcpContent.h"
#define GUCEF_COM_CMCPCONTENT_H
#endif /* GUCEF_COM_CMCPCONTENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a tool gives back: unstructured content blocks, optionally a structured JSON value, and whether the call
 *  failed in a way the model should be told about (a tool execution error, isError).
 *  The server adds resultType and its own _meta keys when it writes the result.
 */
class GUCEF_COM_PUBLIC_CPP CMcpToolResult : public CORE::CICloneable
{
    public:

    typedef GUCEF::vector< CMcpContent > TContentVector;

    TContentVector content;
    CORE::CDataNode structuredContent;      /**< any JSON value, see hasStructuredContent */
    bool hasStructuredContent;
    bool isError;
    CORE::CDataNode meta;                   /**< the _meta of the result, an object node; merged with the keys of the server */

    CMcpToolResult( void );

    CMcpToolResult( const CMcpToolResult& src );

    virtual ~CMcpToolResult() GUCEF_VIRTUAL_OVERRIDE;

    CMcpToolResult& operator=( const CMcpToolResult& src );

    CMcpToolResult& AddText( const CORE::CString& text );

    CMcpToolResult& AddContent( const CMcpContent& block );

    /**
     *  Sets the structured result. The tools page asks a tool that returns structured content to return the
     *  serialized JSON as a text block too, which alsoAsText does
     */
    CMcpToolResult& SetStructuredContent( const CORE::CDataNode& value, bool alsoAsText = true );

    /** Marks the result as a tool execution error with a text block that tells the model what went wrong */
    CMcpToolResult& SetError( const CORE::CString& messageForTheModel );

    void Clear( void );

    /** Writes content, structuredContent, isError and the _meta of the tool into 'result', an object node */
    bool ToDataNode( CORE::CDataNode& result ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPTOOLRESULT_H ? */
