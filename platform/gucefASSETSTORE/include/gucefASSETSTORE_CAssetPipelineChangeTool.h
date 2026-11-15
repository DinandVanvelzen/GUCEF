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

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINECHANGETOOL_H
#define GUCEF_ASSETSTORE_CASSETPIPELINECHANGETOOL_H

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
 *  The writes of the converter pipeline as tools (section 7.4): manifest_create, manifest_edit, override_write,
 *  override_remove. A call without apply is a preview (RunPipelineQuery(): the diff, the digests and a single-use
 *  change token bound to the principal, the target and the arguments, made and kept by the converter hook). A call with
 *  apply and the change token writes (ApplyPipelineChange()), after:
 *
 *  - the confirmation of confirmWrites: "auto" asks the owner through a form elicitation (MRTR input_required, one
 *    boolean "confirm", the sealed state with a single-use nonce) when the client can show one, and takes the change
 *    token and the client's own tool approval as the confirmation otherwise; "always" refuses an apply without the
 *    form; "never" takes the change token. A declined or cancelled form is a tool execution error (not-confirmed);
 *  - the request key: a repeat with the same key and the same arguments answers the first outcome (the command id of
 *    the service is made from the principal, the tool and the key, so a remote service replays it too); the key with
 *    other arguments is refused;
 *  - the hourly quota of writes of the principal (busy with the time of the next one).
 *
 *  After a write the resources of the pack that changed are announced (notifications/resources/updated).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPipelineChangeTool : public CAssetToolBase
{
    public:

    static const CORE::CString ConfirmationKey;     /**< "confirm" */

    CAssetPipelineChangeTool( CAssetToolContext& context, const CORE::CString& name );

    virtual ~CAssetPipelineChangeTool() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The confirmation step that every applying write tool shares: MCP_EXECUTE_COMPLETE with confirmed true to go on,
     *  with confirmed false and an error result when it is refused, or MCP_EXECUTE_INPUT_REQUIRED with the form
     */
    static COM::EMcpExecuteResult Confirm( CAssetToolContext& context              ,
                                           COM::CMcpRequestContext& request        ,
                                           const CORE::CString& toolName           ,
                                           const CORE::CString& what               ,
                                           COM::CMcpToolResult& result             ,
                                           COM::CMcpInputRequired& inputRequired   ,
                                           bool& confirmed                         );

    protected:

    virtual COM::EMcpExecuteResult Compute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ,
                                            SCallState& state                     ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPIPELINECHANGETOOL_H ? */
