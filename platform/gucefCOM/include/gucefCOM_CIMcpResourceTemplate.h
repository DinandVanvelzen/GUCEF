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

#ifndef GUCEF_COM_CIMCPRESOURCETEMPLATE_H
#define GUCEF_COM_CIMCPRESOURCETEMPLATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_COM_CMCPRESOURCEDESCRIPTOR_H
#include "gucefCOM_CMcpResourceDescriptor.h"
#define GUCEF_COM_CMCPRESOURCEDESCRIPTOR_H
#endif /* GUCEF_COM_CMCPRESOURCEDESCRIPTOR_H ? */

#ifndef GUCEF_COM_CMCPRESOURCECONTENTS_H
#include "gucefCOM_CMcpResourceContents.h"
#define GUCEF_COM_CMCPRESOURCECONTENTS_H
#endif /* GUCEF_COM_CMCPRESOURCECONTENTS_H ? */

#ifndef GUCEF_COM_CMCPINPUTREQUIRED_H
#include "gucefCOM_CMcpInputRequired.h"
#define GUCEF_COM_CMCPINPUTREQUIRED_H
#endif /* GUCEF_COM_CMCPINPUTREQUIRED_H ? */

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

class CMcpRequestContext;

/**
 *  A family of resources whose URIs match a URI template. What a template can produce is not listed.
 *  The members must be thread safe, see CIMcpTool.
 */
class GUCEF_COM_PUBLIC_CPP CIMcpResourceTemplate
{
    public:

    CIMcpResourceTemplate( void ) {}

    virtual ~CIMcpResourceTemplate() {}

    virtual bool GetDescriptor( CMcpResourceTemplateDescriptor& descriptor ) const = 0;

    virtual EMcpExecutionMode GetExecutionMode( void ) const = 0;

    /**
     *  Reads the resource with the given URI, which matched the template. 'variables' holds the value of every
     *  expression: {var} percent-decoded, {+var} as it is in the URI. A URI that matches but names nothing that
     *  exists answers PROTOCOL_ERROR with -32602.
     */
    virtual EMcpExecuteResult Read( CMcpRequestContext& context                 ,
                                    const CORE::CString& uri                    ,
                                    const CORE::CValueList& variables           ,
                                    CMcpResourceContents& contents              ,
                                    CMcpInputRequired& inputRequired            ) = 0;

    /** The cache hints of a complete read, unless the contents carry their own. The default: 0 ms, private */
    virtual void GetCacheHints( const CORE::CString& uri, UInt32& ttlMs, EMcpCacheScope& scope ) const { ttlMs = 0; scope = MCP_CACHESCOPE_PRIVATE; }
};

typedef CORE::CTSharedPtr< CIMcpResourceTemplate, MT::CMutex > CIMcpResourceTemplatePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMCPRESOURCETEMPLATE_H ? */
