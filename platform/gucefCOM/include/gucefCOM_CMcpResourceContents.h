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

#ifndef GUCEF_COM_CMCPRESOURCECONTENTS_H
#define GUCEF_COM_CMCPRESOURCECONTENTS_H

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
 *  What reading a resource gives: one or more contents, each text or binary (written as base64), and optionally
 *  cache hints for this read that take the place of those of the resource (CIMcpResource::GetCacheHints()).
 *  A resource that does not exist is never answered with an empty list: the server refuses the read instead.
 */
class GUCEF_COM_PUBLIC_CPP CMcpResourceContents : public CORE::CICloneable
{
    public:

    struct GUCEF_COM_PUBLIC_CPP SItem
    {
        CORE::CString uri;
        CORE::CString mimeType;
        CORE::CString text;
        CORE::CDynamicBuffer blob;
        bool isBlob;
        CORE::CDataNode meta;           /**< the _meta of this item, an object node, written when it has members */

        SItem( void );
    };
    typedef GUCEF::vector< SItem > TItemVector;

    TItemVector items;
    bool hasCacheHints;
    UInt32 ttlMs;
    EMcpCacheScope cacheScope;
    CORE::CDataNode meta;               /**< the _meta of the result, an object node; merged with the keys of the server */

    CMcpResourceContents( void );

    CMcpResourceContents( const CMcpResourceContents& src );

    virtual ~CMcpResourceContents() GUCEF_VIRTUAL_OVERRIDE;

    CMcpResourceContents& operator=( const CMcpResourceContents& src );

    CMcpResourceContents& AddText( const CORE::CString& uri, const CORE::CString& mimeType, const CORE::CString& text );

    CMcpResourceContents& AddBlob( const CORE::CString& uri, const CORE::CString& mimeType, const CORE::CDynamicBuffer& bytes );

    CMcpResourceContents& SetCacheHints( UInt32 ttlInMs, EMcpCacheScope scope );

    void Clear( void );

    /** Writes "contents" and the _meta of the resource into 'result', an object node */
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

#endif /* GUCEF_COM_CMCPRESOURCECONTENTS_H ? */
