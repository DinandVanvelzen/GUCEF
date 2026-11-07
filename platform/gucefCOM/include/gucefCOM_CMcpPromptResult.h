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

#ifndef GUCEF_COM_CMCPPROMPTRESULT_H
#define GUCEF_COM_CMCPPROMPTRESULT_H

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

/** What getting a prompt gives: an optional description and the messages of the prompt, each with a role */
class GUCEF_COM_PUBLIC_CPP CMcpPromptResult : public CORE::CICloneable
{
    public:

    enum ERole
    {
        MCP_ROLE_USER       ,
        MCP_ROLE_ASSISTANT
    };

    struct GUCEF_COM_PUBLIC_CPP SMessage
    {
        ERole role;
        CMcpContent content;

        SMessage( void );
        SMessage( ERole messageRole, const CMcpContent& messageContent );
    };
    typedef GUCEF::vector< SMessage > TMessageVector;

    CORE::CString description;
    TMessageVector messages;
    CORE::CDataNode meta;               /**< the _meta of the result, an object node; merged with the keys of the server */

    CMcpPromptResult( void );

    CMcpPromptResult( const CMcpPromptResult& src );

    virtual ~CMcpPromptResult() GUCEF_VIRTUAL_OVERRIDE;

    CMcpPromptResult& operator=( const CMcpPromptResult& src );

    CMcpPromptResult& AddMessage( ERole role, const CMcpContent& content );

    CMcpPromptResult& AddUserText( const CORE::CString& text );

    CMcpPromptResult& AddAssistantText( const CORE::CString& text );

    void Clear( void );

    /** Writes description, messages and the _meta of the prompt into 'result', an object node */
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

#endif /* GUCEF_COM_CMCPPROMPTRESULT_H ? */
