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

#ifndef GUCEF_COM_CIMCPTOOL_H
#define GUCEF_COM_CIMCPTOOL_H

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

#ifndef GUCEF_COM_CMCPTOOLDESCRIPTOR_H
#include "gucefCOM_CMcpToolDescriptor.h"
#define GUCEF_COM_CMCPTOOLDESCRIPTOR_H
#endif /* GUCEF_COM_CMCPTOOLDESCRIPTOR_H ? */

#ifndef GUCEF_COM_CMCPTOOLRESULT_H
#include "gucefCOM_CMcpToolResult.h"
#define GUCEF_COM_CMCPTOOLRESULT_H
#endif /* GUCEF_COM_CMCPTOOLRESULT_H ? */

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
 *  A tool an application exposes to MCP clients. The server does the protocol: it validates the arguments against
 *  the input schema, asks the access policy, runs Execute() where GetExecutionMode() says and writes the result.
 *
 *  Execute() can be called on several threads at once (several calls in flight, in-process callers), so it must be
 *  thread safe, as must GetDescriptor() and GetExecutionMode().
 */
class GUCEF_COM_PUBLIC_CPP CIMcpTool
{
    public:

    CIMcpTool( void ) {}

    virtual ~CIMcpTool() {}

    /** Asked once when the tool registers; register again to change it, which is a list change */
    virtual bool GetDescriptor( CMcpToolDescriptor& descriptor ) const = 0;

    /** Where Execute() runs when the call comes in over a transport. In-process blocking calls run it on the calling thread */
    virtual EMcpExecutionMode GetExecutionMode( void ) const = 0;

    /**
     *  Does the work. The arguments were validated against the input schema before this is called.
     *  COMPLETE: 'result' is filled in. INPUT_REQUIRED: 'inputRequired' is filled in.
     *  DEFERRED: the answer comes later through context.CompleteWith*(). PROTOCOL_ERROR: context.SetProtocolError() was called.
     *  A failure of the work itself is a COMPLETE result with result.SetError(), which tells the model what went wrong.
     */
    virtual EMcpExecuteResult Execute( CMcpRequestContext& context       ,
                                       const CORE::CDataNode& arguments  ,
                                       CMcpToolResult& result            ,
                                       CMcpInputRequired& inputRequired  ) = 0;
};

typedef CORE::CTSharedPtr< CIMcpTool, MT::CMutex > CIMcpToolPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMCPTOOL_H ? */
