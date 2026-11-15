/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H
#define GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMCPJOBRESULTBUILDER_H
#include "gucefCOM_CIMcpJobResultBuilder.h"
#define GUCEF_COM_CIMCPJOBRESULTBUILDER_H
#endif /* GUCEF_COM_CIMCPJOBRESULTBUILDER_H ? */

#ifndef GUCEF_COM_CMCPJOBINFO_H
#include "gucefCOM_CMcpJobInfo.h"
#define GUCEF_COM_CMCPJOBINFO_H
#endif /* GUCEF_COM_CMCPJOBINFO_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#include "gucefASSETSTORE_CAssetToolContext.h"
#define GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The job object of the asset tools (section 6.4) and the result of a job tool for CMcpJobBridge::Wait():
 *
 *  - the job object of the bridge (handle, kind, state, times, progress) with packId and resultUri;
 *  - the state of the job of the service when the run of the scheduler ended SUCCEEDED (completed): a job of the
 *    service that failed or was cancelled on its own is shown as failed or cancelled, with its message;
 *  - for a final job the summary of its result (GetJobResult( "summary" ) of the service, without the full lists,
 *    which job_result pages), else a hint to poll job_status.
 *
 *  The result is finished (redacted, bounded) here, since the bridge completes deferred calls with it directly.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolJobResultBuilder : public COM::CIMcpJobResultBuilder
{
    public:

    CAssetToolJobResultBuilder( CAssetToolContext& context, const CORE::CString& packId );

    virtual ~CAssetToolJobResultBuilder() GUCEF_VIRTUAL_OVERRIDE;

    virtual void BuildJobResult( const COM::CMcpJobInfo& job, COM::CMcpToolResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    /** The id of the job of the service the run names; empty when it names none. 'failure' gets "<code>:<message>" of a start that failed */
    static CORE::CString GetServiceJobId( const COM::CMcpJobInfo& job, CORE::CString& failure );

    /** The job object, see the class description; 'withSummary' adds the summary of a final job */
    static void BuildJobObject( CAssetToolContext& context, const COM::CMcpJobInfo& job, bool withSummary, CORE::CDataNode& object );

    /** The summary of a result document of the service: everything but the full lists and the links */
    static void MakeSummary( const CORE::CDataNode& document, CORE::CDataNode& summary );

    private:

    CAssetToolJobResultBuilder( const CAssetToolJobResultBuilder& src );              /**< not implemented */
    CAssetToolJobResultBuilder& operator=( const CAssetToolJobResultBuilder& src );   /**< not implemented */

    CAssetToolContext& m_context;
    CORE::CString m_packId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBRESULTBUILDER_H ? */
