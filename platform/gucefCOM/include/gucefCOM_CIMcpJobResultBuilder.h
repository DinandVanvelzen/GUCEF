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

#ifndef GUCEF_COM_CIMCPJOBRESULTBUILDER_H
#define GUCEF_COM_CIMCPJOBRESULTBUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_COM_CMCPJOBINFO_H
#include "gucefCOM_CMcpJobInfo.h"
#define GUCEF_COM_CMCPJOBINFO_H
#endif /* GUCEF_COM_CMCPJOBINFO_H ? */

#ifndef GUCEF_COM_CMCPTOOLRESULT_H
#include "gucefCOM_CMcpToolResult.h"
#define GUCEF_COM_CMCPTOOLRESULT_H
#endif /* GUCEF_COM_CMCPTOOLRESULT_H ? */

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
 *  Makes the tool result of a job for CMcpJobBridge::Wait(): of a job that is still going when the wait ends (the job
 *  object and a hint to poll), or of a job that ended within the wait (the final result of the tool, for example the
 *  job object plus a summary read from the run's result document). Called on the thread of the request's pulse
 *  generator, without a lock of the bridge held. A tool with an output schema builds what the schema describes.
 */
class GUCEF_COM_PUBLIC_CPP CIMcpJobResultBuilder
{
    public:

    CIMcpJobResultBuilder( void );

    virtual ~CIMcpJobResultBuilder();

    virtual void BuildJobResult( const CMcpJobInfo& job, CMcpToolResult& result ) = 0;
};

typedef CORE::CTSharedPtr< CIMcpJobResultBuilder, MT::CMutex > CIMcpJobResultBuilderPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMCPJOBRESULTBUILDER_H ? */
