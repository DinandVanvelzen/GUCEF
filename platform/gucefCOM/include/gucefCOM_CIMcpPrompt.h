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

#ifndef GUCEF_COM_CIMCPPROMPT_H
#define GUCEF_COM_CIMCPPROMPT_H

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

#ifndef GUCEF_COM_CMCPPROMPTDESCRIPTOR_H
#include "gucefCOM_CMcpPromptDescriptor.h"
#define GUCEF_COM_CMCPPROMPTDESCRIPTOR_H
#endif /* GUCEF_COM_CMCPPROMPTDESCRIPTOR_H ? */

#ifndef GUCEF_COM_CMCPPROMPTRESULT_H
#include "gucefCOM_CMcpPromptResult.h"
#define GUCEF_COM_CMCPPROMPTRESULT_H
#endif /* GUCEF_COM_CMCPPROMPTRESULT_H ? */

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

/** A prompt template. The members must be thread safe, see CIMcpTool */
class GUCEF_COM_PUBLIC_CPP CIMcpPrompt
{
    public:

    CIMcpPrompt( void ) {}

    virtual ~CIMcpPrompt() {}

    virtual bool GetDescriptor( CMcpPromptDescriptor& descriptor ) const = 0;

    virtual EMcpExecutionMode GetExecutionMode( void ) const = 0;

    /**
     *  Builds the prompt. Prompt arguments are strings on the wire, so they arrive as a value list keyed by name.
     *  Missing required arguments are refused before this is called.
     */
    virtual EMcpExecuteResult Get( CMcpRequestContext& context          ,
                                   const CORE::CValueList& arguments    ,
                                   CMcpPromptResult& result             ,
                                   CMcpInputRequired& inputRequired     ) = 0;
};

typedef CORE::CTSharedPtr< CIMcpPrompt, MT::CMutex > CIMcpPromptPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMCPPROMPT_H ? */
