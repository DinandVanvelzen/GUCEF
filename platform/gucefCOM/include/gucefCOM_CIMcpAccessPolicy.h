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

#ifndef GUCEF_COM_CIMCPACCESSPOLICY_H
#define GUCEF_COM_CIMCPACCESSPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPREQUESTCONTEXT_H
#include "gucefCOM_CMcpRequestContext.h"
#define GUCEF_COM_CMCPREQUESTCONTEXT_H
#endif /* GUCEF_COM_CMCPREQUESTCONTEXT_H ? */

#ifndef GUCEF_COM_CMCPTOOLREGISTRY_H
#include "gucefCOM_CMcpToolRegistry.h"
#define GUCEF_COM_CMCPTOOLREGISTRY_H
#endif /* GUCEF_COM_CMCPTOOLREGISTRY_H ? */

#ifndef GUCEF_COM_CMCPPROMPTREGISTRY_H
#include "gucefCOM_CMcpPromptRegistry.h"
#define GUCEF_COM_CMCPPROMPTREGISTRY_H
#endif /* GUCEF_COM_CMCPPROMPTREGISTRY_H ? */

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
 *  Decides what a principal may see and do. The server asks it for every request, from transports and in-process
 *  callers alike, from the protocol thread and from the threads of in-process callers: the members are const and
 *  must be thread safe.
 *
 *  MCP_ACCESS_HIDE answers as if the object did not exist. MCP_ACCESS_DENY on a visible tool is a tool execution
 *  error with the reason, so the model can say so; on a resource or prompt it is the GUCEF access denied error.
 *  An object a principal cannot see is always handled as hidden.
 */
class GUCEF_COM_PUBLIC_CPP CIMcpAccessPolicy
{
    public:

    CIMcpAccessPolicy( void ) {}

    virtual ~CIMcpAccessPolicy() {}

    /** True when what a list shows depends on the principal: lists are then cacheScope "private" */
    virtual bool IsVisibilityPrincipalDependent( void ) const = 0;

    virtual bool IsToolVisible( const CMcpPrincipal& principal, const CMcpToolEntry& tool ) const = 0;

    virtual EMcpAccessDecision AuthorizeToolCall( const CMcpRequestContext& context ,
                                                  const CMcpToolEntry& tool          ,
                                                  const CORE::CDataNode& arguments   ,
                                                  CORE::CString& reason              ) const = 0;

    virtual bool IsResourceVisible( const CMcpPrincipal& principal, const CORE::CString& uri ) const = 0;

    virtual EMcpAccessDecision AuthorizeResourceRead( const CMcpRequestContext& context, const CORE::CString& uri, CORE::CString& reason ) const = 0;

    virtual bool IsPromptVisible( const CMcpPrincipal& principal, const CMcpPromptEntry& prompt ) const = 0;

    virtual EMcpAccessDecision AuthorizePromptGet( const CMcpRequestContext& context, const CMcpPromptEntry& prompt, CORE::CString& reason ) const = 0;

    virtual EMcpAccessDecision AuthorizeSubscription( const CMcpRequestContext& context, CORE::CString& reason ) const = 0;
};

typedef CORE::CTSharedPtr< CIMcpAccessPolicy, MT::CMutex > CIMcpAccessPolicyPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMCPACCESSPOLICY_H ? */
