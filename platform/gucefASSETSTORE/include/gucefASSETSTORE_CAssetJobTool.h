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

#ifndef GUCEF_ASSETSTORE_CASSETJOBTOOL_H
#define GUCEF_ASSETSTORE_CASSETJOBTOOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETTOOLBASE_H
#include "gucefASSETSTORE_CAssetToolBase.h"
#define GUCEF_ASSETSTORE_CASSETTOOLBASE_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLBASE_H ? */

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
 *  job_status, job_result, job_list and job_cancel (section 7.2) on the handles of the job bridge: a handle resolves
 *  for the principal that started the job only (job-unknown for anyone else), a finished handle expires after the
 *  result time to live (job-expired). job_result pages the sections failures, approximations and assets of the result
 *  document of the job of the service (GetJobResult()); summary is the bounded default. job_cancel asks the run to
 *  cancel, which cancels the job of the service; the group of the job's kind must be enabled (the access policy checks
 *  it before the call).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetJobTool : public CAssetToolBase
{
    public:

    CAssetJobTool( CAssetToolContext& context, const CORE::CString& name );

    virtual ~CAssetJobTool() GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual COM::EMcpExecuteResult Compute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ,
                                            SCallState& state                     ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool Lookup( COM::CMcpJobBridge& bridge, const COM::CMcpPrincipal& principal, const CORE::CString& jobId,
                 COM::CMcpJobInfo& job, COM::CMcpToolResult& result );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETJOBTOOL_H ? */
