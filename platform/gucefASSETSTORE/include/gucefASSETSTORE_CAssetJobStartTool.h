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

#ifndef GUCEF_ASSETSTORE_CASSETJOBSTARTTOOL_H
#define GUCEF_ASSETSTORE_CASSETJOBSTARTTOOL_H

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
 *  The tools that start jobs (sections 6.4 and 7.2): convert_pack, convert_file, cook_pack, validate_pack,
 *  store_ingest, store_ingest_resume. A call:
 *
 *  1. checks its arguments and paths (store_ingest resolves its rooted path against the roots of the tool set, kinds
 *     source and staging), takes one job of the daily quota of the principal, and for the jobs on a converter pack asks
 *     licence_check first: a pack the licence refuses is licence-refused before anything is queued;
 *  2. starts a job of the job bridge (CMcpJobBridge::StartJob()) whose run drives the job of the service
 *     (CAssetToolJobTaskConsumer) on the lane of its kind (assets.conversion, assets.validation, assets.ingest); a requestKey
 *     makes the start safe to repeat, and is the command id of the service too;
 *  3. waits with CMcpJobBridge::Wait() (waitMs, progress as notifications/progress) and answers with the final result
 *     or the job object (CAssetToolJobResultBuilder).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetJobStartTool : public CAssetToolBase
{
    public:

    static const CORE::CString LaneConversion;      /**< "assets.conversion" */
    static const CORE::CString LaneValidation;      /**< "assets.validation" */
    static const CORE::CString LaneIngest;          /**< "assets.ingest" */

    CAssetJobStartTool( CAssetToolContext& context, const CORE::CString& name, const CORE::CString& taskTypeName );

    virtual ~CAssetJobStartTool() GUCEF_VIRTUAL_OVERRIDE;

    /** The lane of the jobs of a tool */
    static const CORE::CString& GetLaneOfKind( const CORE::CString& kind );

    protected:

    virtual COM::EMcpExecuteResult Compute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ,
                                            SCallState& state                     ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_taskTypeName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETJOBSTARTTOOL_H ? */
