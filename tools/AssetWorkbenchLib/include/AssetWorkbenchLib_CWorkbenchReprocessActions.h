/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHREPROCESSACTIONS_H
#define ASSETWORKBENCHLIB_CWORKBENCHREPROCESSACTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H
#include "gucefASSETSTORE_CAssetReprocessRequest.h"
#define GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The reprocessing of a pack of the primary store from the workbench (the console commands "redownload", "regen intermediate",
 *  "regen target", "reprocess" and the items of the Jobs menu): the original downloaded again and verified, the intermediate
 *  resources made again from the originals, the engine target resources made again from the intermediate, the three in a row, the
 *  rollback of a regeneration and the plan of one. Each is a pipeline job of the asset store service (CAssetReprocessRequest), so it
 *  runs on the server in client mode and shows in the jobs view with the counts before and after.
 *
 *  The options of the console: positional asset paths or globs (the assets; the asset form of the operation), --failed (only the
 *  assets that failed or were skipped), --statuses a,b, --kinds a,b, --targets ogre, --steps redownload,intermediate,target,
 *  --no-cook, --no-convert, --no-repair, --allow-large and --accept-errors.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchReprocessActions
{
    public:

    enum EReprocessAction
    {
        REPROCESS_REDOWNLOAD = 0    ,
        REPROCESS_INTERMEDIATE      ,
        REPROCESS_TARGET            ,
        REPROCESS_ALL               ,
        REPROCESS_ROLLBACK          ,
        REPROCESS_PLAN              ,
        REPROCESS_COUNT
    };

    struct SReprocessInfo
    {
        const char* id;                 /**< "intermediate": the action id suffix ("Reprocess:intermediate") */
        const char* menuText;           /**< "Regenerate intermediate" */
        const char* packOperation;      /**< the pipeline operation for a pack */
        const char* assetOperation;     /**< the pipeline operation for assets of a pack; GUCEF_NULL when the action is for a whole pack */
        const char* explanation;        /**< what it does, for the confirmation */
    };

    static const SReprocessInfo& GetInfo( EReprocessAction action );

    /** "redownload", "intermediate", "target", "reprocess" (or "all"), "rollback", "plan" */
    static bool Parse( const CORE::CString& id, EReprocessAction& action );

    /** The id of the menu item of the action ("Reprocess:intermediate") */
    static CORE::CString GetActionId( EReprocessAction action );
    static bool ParseActionId( const CORE::CString& actionId, EReprocessAction& action );

    /**
     *  The options of a console command: the words after the pack id. The first word that is no option and the words after it are
     *  asset paths or globs. False with the problem for an option that is not known or lacks its value
     */
    static bool ParseOptions( const CORE::CString::StringVector& words, ASSETSTORE::CAssetReprocessRequest& options, CORE::CString& problem );

    /** The request of the pipeline: the asset form of the operation when the options name assets, the pack form otherwise; reprocess_plan for the plan */
    static void MakePipelineRequest( EReprocessAction action, const CORE::CString& packId, const ASSETSTORE::CAssetReprocessRequest& options,
                                     ASSETSTORE::CAssetPipelineRequest& request );

    /** The words of the confirmation and of the log: what is done for which pack and scope */
    static CORE::CString MakeConfirmation( EReprocessAction action, const CORE::CString& packId, UInt32 assetCount );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHREPROCESSACTIONS_H ? */
