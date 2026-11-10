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

#ifndef GUCEF_COM_CIMCPRESOURCE_H
#define GUCEF_COM_CIMCPRESOURCE_H

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

/** A fixed resource, found by its exact URI. The members must be thread safe, see CIMcpTool */
class GUCEF_COM_PUBLIC_CPP CIMcpResource
{
    public:

    CIMcpResource( void ) {}

    virtual ~CIMcpResource() {}

    virtual bool GetDescriptor( CMcpResourceDescriptor& descriptor ) const = 0;

    virtual EMcpExecutionMode GetExecutionMode( void ) const = 0;

    /** The results as for CIMcpTool::Execute(); a resource that turns out not to exist answers PROTOCOL_ERROR with -32602 */
    virtual EMcpExecuteResult Read( CMcpRequestContext& context, CMcpResourceContents& contents, CMcpInputRequired& inputRequired ) = 0;

    /** The cache hints of a complete read, unless the contents carry their own. The default: 0 ms, private */
    virtual void GetCacheHints( UInt32& ttlMs, EMcpCacheScope& scope ) const { ttlMs = 0; scope = MCP_CACHESCOPE_PRIVATE; }
};

typedef CORE::CTSharedPtr< CIMcpResource, MT::CMutex > CIMcpResourcePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMCPRESOURCE_H ? */
