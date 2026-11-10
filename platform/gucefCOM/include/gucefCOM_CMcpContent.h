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

#ifndef GUCEF_COM_CMCPCONTENT_H
#define GUCEF_COM_CMCPCONTENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

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
 *  The Annotations of the MCP schema: hints on who a resource or content block is for and how important it is.
 *  Shared by content blocks, resources and resource templates.
 */
class GUCEF_COM_PUBLIC_CPP CMcpAnnotations
{
    public:

    bool hasAudience;
    bool audienceUser;
    bool audienceAssistant;
    bool hasPriority;
    Float64 priority;                   /**< 0.0 (optional) to 1.0 (effectively required) */
    CORE::CString lastModified;         /**< ISO 8601, empty: not given */

    CMcpAnnotations( void );

    bool IsEmpty( void ) const;

    /** Writes the annotations object into 'node', which becomes an object node */
    void ToDataNode( CORE::CDataNode& node ) const;

    /** Adds the "annotations" member to an object when there is anything to say */
    void AddToObject( CORE::CDataNode& object ) const;
};

/*-------------------------------------------------------------------------*/

/** One block of "content": text, an image, audio, a link to a resource or an embedded resource */
class GUCEF_COM_PUBLIC_CPP CMcpContent
{
    public:

    enum EType
    {
        MCP_CONTENT_TEXT            ,
        MCP_CONTENT_IMAGE           ,
        MCP_CONTENT_AUDIO           ,
        MCP_CONTENT_RESOURCE_LINK   ,
        MCP_CONTENT_RESOURCE            /**< an embedded resource: uri, mime type and text or blob */
    };

    static CMcpContent Text( const CORE::CString& text );

    /** The bytes are written as base64 */
    static CMcpContent Image( const CORE::CDynamicBuffer& bytes, const CORE::CString& mimeType );

    static CMcpContent Audio( const CORE::CDynamicBuffer& bytes, const CORE::CString& mimeType );

    static CMcpContent ResourceLink( const CORE::CString& uri, const CORE::CString& name, const CORE::CString& mimeType );

    static CMcpContent EmbeddedText( const CORE::CString& uri, const CORE::CString& mimeType, const CORE::CString& text );

    static CMcpContent EmbeddedBlob( const CORE::CString& uri, const CORE::CString& mimeType, const CORE::CDynamicBuffer& bytes );

    CMcpContent( void );

    CMcpContent& SetAudience( bool user, bool assistant );

    CMcpContent& SetPriority( Float64 priority );

    CMcpContent& SetLastModified( const CORE::CString& iso8601 );

    /** Writes the content block into 'node', which becomes an object node */
    bool ToDataNode( CORE::CDataNode& node ) const;

    EType type;
    CORE::CString text;                 /**< text, or the text of an embedded text resource */
    CORE::CString mimeType;
    CORE::CDynamicBuffer data;          /**< image, audio, or the blob of an embedded resource */
    bool isBlob;                        /**< an embedded resource carries 'data' as its blob, not 'text' */
    CORE::CString uri;                  /**< resource link and embedded resource */
    CORE::CString name;                 /**< resource link */
    CORE::CString title;                /**< resource link, optional */
    CORE::CString description;          /**< resource link, optional */
    bool hasSize;
    UInt64 size;                        /**< resource link, optional */
    CMcpAnnotations annotations;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPCONTENT_H ? */
